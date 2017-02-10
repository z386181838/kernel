/* dmares_lkm.c
 *
 * Linux Kernel-Mode implementation of the Driver Framework DMAResource API
 *
 */

/*****************************************************************************
* Copyright (c) 2010-2013 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://essoemsupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) some of the following interface(s):
 */

#include "dmares_mgmt.h"
#include "dmares_buf.h"
#include "dmares_rw.h"

// Internal API implemented here
#include "dmares_hwpal.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_dmares_lkm.h"

#include "dmares_gen.h"         // Helpers from Generic DMAResource API

#include "device_swap.h"        // Device_SwapEndian32
#include "device_mgmt.h"        // Device_GetReference

// Driver Framework C Run-Time Library API
#include "clib.h"               // memset

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t, NULL, inline, bool,
                                // IDENTIFIER_NOT_USED

// Logging API
//#undef LOG_SEVERITY_MAX
//#define LOG_SEVERITY_MAX  HWPAL_LOG_SEVERITY
#include "log.h"                // LOG_*

// Linux Kernel API
#include <linux/slab.h>         // kmalloc, kfree
#include <linux/dma-mapping.h>  // dma_sync_single_for_cpu, dma_alloc_coherent,
                                // dma_free_coherent
#include <linux/hardirq.h>      // in_atomic

#ifdef HWPAL_LOCK_SLEEPABLE
#include <linux/mutex.h>        // mutex_*
#else
#include <linux/spinlock.h>     // spinlock_*
#endif

#define MTK_EIP97_DRIVER
/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#ifdef HWPAL_64BIT_HOST
#ifdef HWPAL_DMARESOURCE_64BIT
#define HWPAL_DMA_FLAGS 0 // No special requirements for address.
#else
#define HWPAL_DMA_FLAGS GFP_DMA // 64-bit host, 32-bit memory addresses
#endif
#else
#ifdef MTK_EIP97_DRIVER
#include <linux/pci.h>
#define HWPAL_DMA_FLAGS (GFP_KERNEL | GFP_ATOMIC) // No special requirements for address.
#else
#define HWPAL_DMA_FLAGS 0 // No special requirements for address.
#endif
#endif

/*
 Requirements on the records:
  - pre-allocated array of records
  - Total size of this array may not be limited by kmalloc size.
  - valid between Create and Destroy
  - re-use on a least-recently-used basis to make sure accidental continued
    use after destroy does not cause crashes, allowing us to detect the
    situation instead of crashing quickly.

 Requirements on the handles:
  - one handle per record
  - valid between Create and Destroy
  - quickly find the ptr-to-record belonging to the handle
  - detect continued use of a handle after Destroy
  - caller-hidden admin/status, thus not inside the record
  - report leaking handles upon exit

 Solution:
  - handle cannot be a record number (no post-destroy use detection possible)
  - Handke is a pointer into the Handles_p array. Each entry contains a record
    index pair (or HWPAL_INVALID_INDEX if no record is associated with it).
  - Array of records is divided into chunks, each chunk contains as many
    records as fits into kmalloc buffer.
  - Pointers to these chunks in RecordChunkPtrs_p array. Each record is
    accessed via an index pair (chunk number and index within chunk).
  - list of free locations in Handles_p:  FreeHandles
  - list of free record index pairs     : FreeRecords
 */

typedef struct
{
    int ReadIndex;
    int WriteIndex;
    uint32_t * Nrs_p;
} HWPAL_FreeList_t;

typedef struct
{
    int CurIndex;
} DMAResourceLib_InUseHandles_Iterator_t;


/* Each chunk holds as many DMA resource records as will fit into a single
   kmalloc buffer, but not more than 65536 as we use a 16-bit index  */
#define MAX_RECORDS_PER_CHUNK \
    (KMALLOC_MAX_SIZE / sizeof(DMAResource_Record_t) > 65536 ? 65536 : \
     KMALLOC_MAX_SIZE / sizeof(DMAResource_Record_t))

/* Each DMA handle stores a 32-bit index pair consisting of two 16-bit
   fields to refer to the DMA resource record via RecordChunkPtrs_p.
   RecordChunkPtrs_p is an array of pointers to contiguous arrays
   (chunks) of DMA resource records.

   Those 16-bit fields (chunk number and record index) are combined into a
   single uint32_t
   Special value HWPAL_INVALID_INDEX (0xffffffff) represents destroyed records.
*/

#define COMPOSE_RECNR(chunk, recidx) (((chunk) << 16) | recidx)
#define CHUNK_OF(recnr) (((recnr) >> 16) & 0xffff)
#define RECIDX_OF(recnr) ((recnr) & 0xffff)
#define HWPAL_INVALID_INDEX 0xffffffff

// Note: dma_get_cache_alignment() can be used in place of L1_CACHE_BYTES
//       but it does not work on the 64-bit PowerPC Freescale P5020DS!
#ifndef HWPAL_DMARESOURCE_DCACHE_LINE_SIZE
#define HWPAL_DMARESOURCE_DCACHE_LINE_SIZE      L1_CACHE_BYTES
#endif


/*----------------------------------------------------------------------------
 * Local variables
 */

static int HandlesCount = 0; // remainder are valid only when this is != 0
static int ChunksCount;

static uint32_t * Handles_p;
static DMAResource_Record_t ** RecordChunkPtrs_p;

// array of pointers to arrays.
static HWPAL_FreeList_t FreeHandles;
static HWPAL_FreeList_t FreeRecords;

static void * HWPAL_Lock_p;


/*----------------------------------------------------------------------------
 * DMAResourceLib_IdxPair2RecordPtr
 *
 */
static inline DMAResource_Record_t *
DMAResourceLib_IdxPair2RecordPtr(uint32_t IdxPair)
{
    if (IdxPair != HWPAL_INVALID_INDEX &&
        CHUNK_OF(IdxPair) < ChunksCount &&
        RECIDX_OF(IdxPair) < MAX_RECORDS_PER_CHUNK &&
        CHUNK_OF(IdxPair) * MAX_RECORDS_PER_CHUNK +
        RECIDX_OF(IdxPair) < HandlesCount
        )
    {
        return RecordChunkPtrs_p[CHUNK_OF(IdxPair)] + RECIDX_OF(IdxPair);;
    }
    else
    {
        return NULL;
    }
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_FreeList_Get
 *
 * Gets the next entry from the freelist. Returns HWPAL_INVALID_INDEX
 *  when the list is empty.
 */
static inline uint32_t
DMAResourceLib_FreeList_Get(
        HWPAL_FreeList_t * const List_p)
{
    uint32_t Nr = HWPAL_INVALID_INDEX;
    int ReadIndex_Updated = List_p->ReadIndex + 1;

    if (ReadIndex_Updated >= HandlesCount)
        ReadIndex_Updated = 0;

    // if post-increment ReadIndex == WriteIndex, the list is empty
    if (ReadIndex_Updated != List_p->WriteIndex)
    {
        // grab the next number
        Nr = List_p->Nrs_p[List_p->ReadIndex];
        List_p->ReadIndex = ReadIndex_Updated;
    }

    return Nr;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_FreeList_Add
 *
 * Adds an entry to the freelist.
 */
static inline void
DMAResourceLib_FreeList_Add(
        HWPAL_FreeList_t * const List_p,
        uint32_t Nr)
{
    if (List_p->WriteIndex == List_p->ReadIndex)
    {
        LOG_WARN(
            "DMAResourceLib_FreeList_Add: "
            "Attempt to add value %u to full list\n",
            Nr);
        return;
    }

    if (Nr == HWPAL_INVALID_INDEX)
    {
        LOG_WARN(
            "DMAResourceLib_FreeList_Add: "
            "Attempt to put invalid value: %u\n",
            Nr);
        return;
    }

    {
        int WriteIndex_Updated = List_p->WriteIndex + 1;
        if (WriteIndex_Updated >= HandlesCount)
            WriteIndex_Updated = 0;

        // store the number
        List_p->Nrs_p[List_p->WriteIndex] = Nr;
        List_p->WriteIndex = WriteIndex_Updated;
    }
}

/*----------------------------------------------------------------------------
 * DMAResourceLib_InUseHandles_*
 *
 * Helper functions to iterate over all currently in-use handles.
 *
 * Usage:
 *     DMAResourceLib_InUseHandles_Iterator_t it;
 *     for (Handle = DMAResourceLib_InUseHandles_First(&it);
 *          Handle != NULL;
 *          Handle = DMAResourceLib_InUseHandles_Next(&it))
 *     { ...
 *
 */
static inline DMAResource_Record_t *
DMAResourceLib_InUseHandles_Get(
        DMAResourceLib_InUseHandles_Iterator_t * const it)
{
    DMAResource_Record_t * Rec_p;

    do
    {
        if (it->CurIndex >= HandlesCount)
            return NULL;

        Rec_p = DMAResourceLib_IdxPair2RecordPtr(Handles_p[it->CurIndex++]);

        if (Rec_p != NULL && Rec_p->Magic != DMARES_RECORD_MAGIC)
            Rec_p = NULL;
    }
    while(Rec_p == NULL);

    return Rec_p;
}


static inline DMAResource_Record_t *
DMAResourceLib_InUseHandles_First(
        DMAResourceLib_InUseHandles_Iterator_t * const it)
{
    it->CurIndex = 0;
    return DMAResourceLib_InUseHandles_Get(it);
}


static inline DMAResource_Record_t *
DMAResourceLib_InUseHandles_Next(
        DMAResourceLib_InUseHandles_Iterator_t * const it)
{
    return DMAResourceLib_InUseHandles_Get(it);
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_IsSubRangeOf
 *
 * Return true iff the address range defined by `AddrPair1' and `Size1' is
 * within the address range defined by `AddrPair2' and `Size2'.
 */
static bool
DMAResourceLib_IsSubRangeOf(
        const DMAResource_AddrPair_t * const AddrPair1,
        const unsigned int Size1,
        const DMAResource_AddrPair_t * const AddrPair2,
        const unsigned int Size2)
{
    if (AddrPair1->Domain == AddrPair2->Domain)
    {
        const uint8_t * Addr1 = AddrPair1->Address_p;
        const uint8_t * Addr2 = AddrPair2->Address_p;

        if ((Size1 <= Size2) &&
            (Addr2 <= Addr1) &&
            ((Addr1 + Size1) <= (Addr2 + Size2)))
        {
            return true;
        }
    }

    return false;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_Find_Matching_DMAResource
 *
 * Return a pointer to the DMAResource record for a currently allocated or
 * attached DMA buffer that matches the given `Properties' and `AddrPair'.
 * The match can be either exact or indicate that the buffer defined by
 * `Properties and `AddrPair' is a proper sub section of the allocated or
 * attached buffer.
 */
static DMAResource_Record_t *
DMAResourceLib_Find_Matching_DMAResource(
        const DMAResource_Properties_t * const Properties,
        const DMAResource_AddrPair_t AddrPair)
{
    DMAResourceLib_InUseHandles_Iterator_t it;
    DMAResource_AddrPair_t * Pair_p;
    DMAResource_Record_t * Rec_p;
    unsigned int Size;

    for (Rec_p = DMAResourceLib_InUseHandles_First(&it);
         Rec_p != NULL;
         Rec_p = DMAResourceLib_InUseHandles_Next(&it))
    {
        if (Rec_p->AllocatorRef == 'R' || Rec_p->AllocatorRef == 'N')
        {
            // skip registered buffers when looking for a match,
            // i.e. only consider allocated buffers.
            continue;
        }

        if (Properties->Bank != Rec_p->Props.Bank  ||
            Properties->Size > Rec_p->Props.Size ||
            Properties->Alignment > Rec_p->Props.Alignment)
        {
            // obvious mismatch in properties
            continue;
        }

        Size = Properties->Size;
        Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
        if (Pair_p != NULL &&
            DMAResourceLib_IsSubRangeOf(&AddrPair, Size, Pair_p, Rec_p->Props.Size))
        {
            return Rec_p;
        }

        Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_BUS);
        if (Pair_p != NULL &&
            DMAResourceLib_IsSubRangeOf(&AddrPair, Size, Pair_p, Rec_p->Props.Size))
        {
            return Rec_p;
        }
    } // for

    return NULL;
}



/*----------------------------------------------------------------------------
 * DMAResourceLib_Setup_Record
 *
 * Setup most fields of a given DMAResource record, except for the
 * AddrPairs array.
 */
static void
DMAResourceLib_Setup_Record(
        const DMAResource_Properties_t * const Props_p,
        const char AllocatorRef,
        DMAResource_Record_t * const Rec_p,
        const unsigned int AllocatedSize)
{
    Rec_p->Props = *Props_p;
    Rec_p->AllocatorRef = AllocatorRef;
    Rec_p->BufferSize = AllocatedSize;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_MaxAlignment_Get
 */
unsigned int
HWPAL_DMAResource_MaxAlignment_Get(void)
{
    return (1 * 1024 * 1024); // 1 MB
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_DCache_Alignment_Get
 */
unsigned int
HWPAL_DMAResource_DCache_Alignment_Get(void)
{
#ifdef HWPAL_ARCH_COHERENT
    unsigned int AlignTo = 1; // No cache line alignment required
#else
    unsigned int AlignTo = HWPAL_DMARESOURCE_DCACHE_LINE_SIZE;
#endif // HWPAL_ARCH_COHERENT

#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
    AlignTo = 1; // No cache line alignment required
#endif

    return AlignTo;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_MemAlloc
 */
void *
HWPAL_DMAResource_MemAlloc(
        size_t ByteCount)
{
    unsigned long flags = 0;

    if (in_atomic())
        flags |= GFP_ATOMIC;    // non-sleepable
    else
        flags |= GFP_KERNEL;    // sleepable

    return kmalloc(ByteCount, flags);
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_MemFree
 */
void
HWPAL_DMAResource_MemFree(
        void * Buf_p)
{
    kfree (Buf_p);
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Lock_Alloc
 */
void *
HWPAL_DMAResource_Lock_Alloc(void)
{
#ifdef HWPAL_LOCK_SLEEPABLE
    struct mutex * HWPAL_Lock = HWPAL_DMAResource_MemAlloc(sizeof(mutex));
    if (HWPAL_Lock == NULL)
        return NULL;

    LOG_INFO("HWPAL_DMAResource_Lock_Alloc: Lock = mutex\n");
    mutex_init(HWPAL_Lock);

    return HWPAL_Lock;
#else
    spinlock_t * HWPAL_SpinLock =
            HWPAL_DMAResource_MemAlloc(sizeof(spinlock_t));
    if (HWPAL_SpinLock == NULL)
        return NULL;

    LOG_INFO("HWPAL_DMAResource_Lock_Alloc: Lock = spinlock\n");
    spin_lock_init(HWPAL_SpinLock);

    return HWPAL_SpinLock;
#endif
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Lock_Free
 */
void
HWPAL_DMAResource_Lock_Free(void * Lock_p)
{
    HWPAL_DMAResource_MemFree(Lock_p);
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Lock_Acquire
 */
void
HWPAL_DMAResource_Lock_Acquire(
        void * Lock_p,
        unsigned long * Flags)
{
#ifdef HWPAL_LOCK_SLEEPABLE
    IDENTIFIER_NOT_USED(Flags);
    mutex_lock((struct mutex*)Lock_p);
#else
    spin_lock_irqsave((spinlock_t *)Lock_p, *Flags);
#endif
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Lock_Release
 */
void
HWPAL_DMAResource_Lock_Release(
        void * Lock_p,
        unsigned long * Flags)
{
#ifdef HWPAL_LOCK_SLEEPABLE
    IDENTIFIER_NOT_USED(Flags);
    mutex_unlock((struct mutex*)Lock_p);
#else
    spin_unlock_irqrestore((spinlock_t *)Lock_p, *Flags);
#endif
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Alloc
 */
int
HWPAL_DMAResource_Alloc(
        const DMAResource_Properties_t RequestedProperties,
        DMAResource_AddrPair_t * const AddrPair_p,
        DMAResource_Handle_t * const Handle_p)
{
    DMAResource_Properties_t ActualProperties;
    DMAResource_AddrPair_t * Pair_p;
    DMAResource_Handle_t Handle;
    DMAResource_Record_t * Rec_p = NULL;

    unsigned int AlignTo = HWPAL_DMAResource_DCache_Alignment_Get();

    ZEROINIT(ActualProperties);

    if ((NULL == AddrPair_p) || (NULL == Handle_p))
    {
        return -1;
    }

    if (!DMAResourceLib_IsSaneInput(NULL, &RequestedProperties))
    {
        return -1;
    }

    if (RequestedProperties.Alignment > AlignTo)
    {
        AlignTo = RequestedProperties.Alignment;
    }

    // Allocate record
    Handle = DMAResource_CreateRecord();
    if (NULL == Handle)
    {
        return -1;
    }

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (NULL == Rec_p)
    {
        DMAResource_DestroyRecord(Handle);
        return -1;
    }

    // allocate the memory
    {
        struct device * DMADevice_p;
        size_t n = 0;
        void * UnalignedAddr_p = NULL;
        void * AlignedAddr_p = NULL;
        dma_addr_t DMAAddr = 0;

        // Align if required
        n = DMAResourceLib_AlignForAddress(
               DMAResourceLib_AlignForSize(RequestedProperties.Size, AlignTo),
               AlignTo);

        // Get device reference for this resource
        DMADevice_p = Device_GetReference(NULL);

        // Step 1: Allocate a buffer
        {
            int flags = HWPAL_DMA_FLAGS;

            if (in_atomic())
                flags |= GFP_ATOMIC;    // non-sleepable
            else
                flags |= GFP_KERNEL;    // sleepable

#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
#ifdef MTK_EIP97_DRIVER
            UnalignedAddr_p = dma_alloc_coherent(
                                     NULL, n, &DMAAddr, flags);
#if (defined (MTK_EIP97_ICEPRN) || defined (MTK_EIP97_SIMPRN))
            printk("-- dma_alloc_coherent PA=0x%08X VA=0x%08X Len=%d--\n", DMAAddr, UnalignedAddr_p, n);
#endif
#else
            UnalignedAddr_p = dma_alloc_coherent(
                                     DMADevice_p, n, &DMAAddr, flags);
#endif
#else
            UnalignedAddr_p = kmalloc(n,flags);
#if (defined (MTK_EIP97_ICEPRN) || defined (MTK_EIP97_SIMPRN))
            printk("== kmalloc VA=0x%08X, Len=%d==\n",UnalignedAddr_p, n);
#endif            
#endif // HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT

            if (UnalignedAddr_p == NULL)
            {
                LOG_CRIT("DMAResource_Alloc: failed for handle 0x%p, size %d\n",
                         Handle,(int)n);
                DMAResource_DestroyRecord(Handle);
                return -1;
            }
        }

        ActualProperties.Alignment  = (uint8_t)AlignTo;
        ActualProperties.Bank       = RequestedProperties.Bank;

#ifdef HWPAL_ARCH_COHERENT
        ActualProperties.fCached    = false;
#else
        // This implementation does not allocate to non-cached resources
        ActualProperties.fCached    = true;
#endif // HWPAL_ARCH_COHERENT

#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
        ActualProperties.fCached    = false;
#endif // HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT

        // Hide the allocated size from the caller, since (s)he is not
        // supposed to access/use any space beyond what was requested
        ActualProperties.Size = RequestedProperties.Size;

        DMAResourceLib_Setup_Record(
                &ActualProperties,
                'A',
                Rec_p,
                n);

        // Step 2: Align the allocated buffer
        {
            unsigned long AlignmentOffset;
            unsigned long UnalignedAddress = ((unsigned long)UnalignedAddr_p);

            AlignmentOffset = UnalignedAddress % AlignTo;

            // Check if address needs to be aligned
            if( AlignmentOffset )
                AlignedAddr_p =
                        (void*)(UnalignedAddress + AlignTo - AlignmentOffset);
            else
                AlignedAddr_p = UnalignedAddr_p; // No alignment required
        }

        // Step 3: Get the physical address of the allocated buffer
#ifdef HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
#error "no need HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL"
        DMAAddr = virt_to_phys (AlignedAddr_p);
#else
#ifdef MTK_EIP97_DRIVER
#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
#else
				DMAAddr = dma_map_single(NULL, AlignedAddr_p, n,
                                 DMA_BIDIRECTIONAL);
#endif
#else
        DMAAddr = dma_map_single(DMADevice_p, AlignedAddr_p, n,
                                 DMA_BIDIRECTIONAL);
#endif
#endif // HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL

        if (0 == DMAAddr)
        {

#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
#ifdef MTK_EIP97_DRIVER
            dma_free_coherent(NULL, n, UnalignedAddr_p, DMAAddr);
#else
#error "NON MTK_EIP97_DRIVER"
            dma_free_coherent(DMADevice_p, n, UnalignedAddr_p, DMAAddr);
#endif            
#else
            kfree(UnalignedAddr_p);
#endif

            DMAResource_DestroyRecord(Handle);
            LOG_WARN(
                "DMAResource_Alloc: "
                "Failed to obtain DMA address for host address 0x%p, "
                "for handle 0x%p\n",
                AlignedAddr_p,
                Handle);

            return -1;
        }

        // put the bus address first, presumably being the most
        // frequently looked-up domain.
        Pair_p = Rec_p->AddrPairs;
        Pair_p->Address_p = (void *)DMAAddr;
        Pair_p->Domain = DMARES_DOMAIN_BUS;

        ++Pair_p;
        Pair_p->Address_p = AlignedAddr_p;
        Pair_p->Domain = DMARES_DOMAIN_HOST;

        // Return this address
        *AddrPair_p = *Pair_p;

        // This host address will be used for freeing the allocated buffer
        ++Pair_p;
        Pair_p->Address_p = UnalignedAddr_p;
        Pair_p->Domain = DMARES_DOMAIN_HOST_UNALIGNED;

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
        LOG_INFO("DMAResource_Alloc (1/2): handle = 0x%p, allocator='%c', "
                 "size allocated/requested=%d/%d, \n"
                 "DMAResource_Alloc (2/2): alignment/bank/cached=%d/%d/%d, "
                 "bus addr=0x%p, host addr un-/aligned=0x%p/0x%p\n",
                 Handle, Rec_p->AllocatorRef,
                 Rec_p->BufferSize, Rec_p->Props.Size,
                 Rec_p->Props.Alignment,Rec_p->Props.Bank,Rec_p->Props.fCached,
                 (void*)DMAAddr,UnalignedAddr_p,AlignedAddr_p);
#endif
    }

    // return results
    *Handle_p = Handle;

    return 0;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Release
 */
int
HWPAL_DMAResource_Release(
        const DMAResource_Handle_t Handle)
{
    DMAResource_Record_t * Rec_p;
    dma_addr_t DMAAddr = 0;

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
    void* UnalignedAddr_p = NULL;
#endif

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Release: "
            "Invalid handle %p\n",
            Handle);
        return -1;
    }

    // request the kernel to unmap the DMA resource
    if (Rec_p->AllocatorRef == 'A' || Rec_p->AllocatorRef == 'k' ||
        Rec_p->AllocatorRef == 'R')
    {
        DMAResource_AddrPair_t * Pair_p;
        struct device * DMADevice_p;

        Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_BUS);
        if (Pair_p == NULL)
        {
            LOG_WARN(
                "DMAResource_Release: "
                "No bus address found for Handle %p?\n",
                Handle);
            return -1;
        }

        DMAAddr = (dma_addr_t)Pair_p->Address_p;

        Pair_p = DMAResourceLib_LookupDomain(Rec_p,
                                             DMARES_DOMAIN_HOST_UNALIGNED);
        if (Pair_p == NULL)
        {
            LOG_WARN(
                "DMAResource_Release: "
                "No host address found for Handle %p?\n",
                Handle);
            return -1;
        }

        // Get device reference for this resource
        DMADevice_p = Device_GetReference(NULL);

#ifndef HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
        if(Rec_p->AllocatorRef != 'R')
        {
#ifdef MTK_EIP97_DRIVER
#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
#else
						dma_unmap_single(NULL, DMAAddr,
                             Rec_p->BufferSize, DMA_BIDIRECTIONAL);
#endif
#else
            dma_unmap_single(DMADevice_p, DMAAddr,
                             Rec_p->BufferSize, DMA_BIDIRECTIONAL);
#endif
        }
#endif // HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL

        if(Rec_p->AllocatorRef == 'A')
        {
#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
#ifdef MTK_EIP97_DRIVER
						dma_free_coherent(NULL, Rec_p->BufferSize,
                              Pair_p->Address_p, DMAAddr);
#else
            dma_free_coherent(DMADevice_p, Rec_p->BufferSize,
                              Pair_p->Address_p, DMAAddr);
#endif
#else
            kfree(Pair_p->Address_p);
#endif
        }

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
        UnalignedAddr_p = Pair_p->Address_p;
#endif
    }

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
    LOG_INFO("DMAResource_Release (1/2): "
             "handle = 0x%p, allocator='%c', "
             "size allocated/requested=%d/%d, \n"
             "DMAResource_Release (2/2): "
             "alignment/bank/cached=%d/%d/%d, "
             "bus addr=0x%p, unaligned host addr=0x%p\n",
             Handle, Rec_p->AllocatorRef,
             Rec_p->BufferSize, Rec_p->Props.Size,
             Rec_p->Props.Alignment, Rec_p->Props.Bank, Rec_p->Props.fCached,
             (void*)DMAAddr, UnalignedAddr_p);
#endif

    // free administration resources
    Rec_p->Magic = 0;
    DMAResource_DestroyRecord(Handle);

    return 0;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Init
 */
bool
HWPAL_DMAResource_Init(void)
{
    bool AllocFailed = false;
    unsigned int MaxHandles = HWPAL_DMA_NRESOURCES;

#ifdef HWPAL_DMARESOURCE_64BIT
    struct device * DMADevice_p;

    // Get device reference for this resource
    DMADevice_p = Device_GetReference(NULL);

    // Set DMA mask wider, so the DMA API will not try to bounce.
    dma_set_mask(DMADevice_p, 0xffffffffffL); // 40 bits, enough for current HW
#endif

    // already initialized?
    if (HandlesCount != 0)
        return false;

    HWPAL_Lock_p = HWPAL_DMAResource_Lock_Alloc();
    if (HWPAL_Lock_p == NULL)
    {
        LOG_CRIT("HWPAL_DMAResource_Init: record lock allocation failed\n");
        return false;
    }

    ChunksCount = (MaxHandles + MAX_RECORDS_PER_CHUNK - 1) /
        MAX_RECORDS_PER_CHUNK;

    if (ChunksCount > 65535 ||
        ChunksCount * sizeof(void *) > KMALLOC_MAX_SIZE)
    {
        LOG_CRIT(
            "HWPAL_DMAResource_Init: "
            "Too many chunks desired: %u\n",
            ChunksCount);
        return false;
    }

    LOG_INFO(
         "HWPAL_DMAResource_Init: "
         "Allocate %d records in %d chunks, "
         "DMA address size (bytes) %d, host address size  (bytes) %d\n",
         MaxHandles, ChunksCount,
         (int)sizeof(dma_addr_t),
         (int)sizeof(void*));

    LOG_INFO("HWPAL_DMAResource_Init: D-cache line size %d\n",
             HWPAL_DMARESOURCE_DCACHE_LINE_SIZE);

#ifdef HWPAL_64BIT_HOST
    if (sizeof(void*) != 8 || sizeof(long) < 8)
    {
        LOG_CRIT("\n\nHWPAL_DMAResource_Init: ERROR, 64-bit host specified, "
                 "but data sizes do not agree\n");
        return false;
    }
#else
    if (sizeof(void*) != 4)
    {
        LOG_CRIT("\n\nHWPAL_DMAResource_Init: ERROR, 32-bit host specified, "
                 "but data sizes do not agree\n");
        return false;
    }
#endif

    if (sizeof(dma_addr_t) > sizeof(void*))
    {
        LOG_WARN(
            "\n\nHWPAL_DMAResource_Init: WARNING, "
            "unsupported host DMA address size %d\n\n",
            (int)sizeof(dma_addr_t));
    }

    RecordChunkPtrs_p =
            HWPAL_DMAResource_MemAlloc(ChunksCount * sizeof(void *));
    if (RecordChunkPtrs_p)
    {
        // Allocate each of the chunks in the RecordChunkPtrs_p array,
        // all of size MAX_RECORDS_PER_CHUNK, except the last one.
        int RecordsLeft, RecordsThisChunk, i;

        // Pre-initialize with null, in case of early error exit
        memset(
            RecordChunkPtrs_p,
            0,
            ChunksCount * sizeof(DMAResource_Record_t*));

        i=0;
        RecordsLeft = MaxHandles;

        while ( RecordsLeft )
        {
            if (RecordsLeft > MAX_RECORDS_PER_CHUNK)
                RecordsThisChunk = MAX_RECORDS_PER_CHUNK;
            else
                RecordsThisChunk = RecordsLeft;

            RecordChunkPtrs_p[i] =
                    HWPAL_DMAResource_MemAlloc(RecordsThisChunk *
                                            sizeof(DMAResource_Record_t));
            if( RecordChunkPtrs_p[i] == NULL )
            {
                LOG_CRIT(
                    "HWPAL_DMAResource_Init:"
                    "Allocation failed chunk %d\n",i);
                AllocFailed = true;
                break;
            }

            RecordsLeft -= RecordsThisChunk;
            i++;
        }
        LOG_INFO(
            "HWPAL_DMAResource_Init:"
            "Allocated %d chunks last one=%d others=%d, total=%d\n",
            i,
            RecordsThisChunk,
            (int)MAX_RECORDS_PER_CHUNK,
            (int)((i-1) * MAX_RECORDS_PER_CHUNK + RecordsThisChunk));
    }

    if (MaxHandles * sizeof(uint32_t) > KMALLOC_MAX_SIZE)
    { // Too many handles for kmalloc, allocate with get_free_pages.
        int order = get_order(MaxHandles * sizeof(uint32_t));

        LOG_INFO(
             "HWPAL_DMAResource_Init: "
             "Handles & freelist allocated by get_free_pages order=%d\n",
             order);

        Handles_p = (void*) __get_free_pages(GFP_KERNEL, order);
        FreeHandles.Nrs_p = (void*) __get_free_pages(GFP_KERNEL, order);
        FreeRecords.Nrs_p = (void*) __get_free_pages(GFP_KERNEL, order);
    }
    else
    {
        Handles_p = HWPAL_DMAResource_MemAlloc(MaxHandles * sizeof(uint32_t));
        FreeHandles.Nrs_p =
                HWPAL_DMAResource_MemAlloc(MaxHandles * sizeof(uint32_t));
        FreeRecords.Nrs_p =
                HWPAL_DMAResource_MemAlloc(MaxHandles * sizeof(uint32_t));
    }

    // if any allocation failed, free the whole lot
    if (RecordChunkPtrs_p == NULL ||
        Handles_p == NULL ||
        FreeHandles.Nrs_p == NULL ||
        FreeRecords.Nrs_p == NULL ||
        AllocFailed)
    {
        LOG_CRIT(
            "HWPAL_DMAResource_Init: "
            "RP=%p HP=%p FH=%p FR=%p AF=%d\n",
            RecordChunkPtrs_p,
            Handles_p,
            FreeHandles.Nrs_p,
            FreeRecords.Nrs_p,
            AllocFailed);

        if (RecordChunkPtrs_p)
        {
            int i;
            for (i = 0; i < ChunksCount; i++)
            {
                if(RecordChunkPtrs_p[i])
                    HWPAL_DMAResource_MemFree(RecordChunkPtrs_p[i]);
            }
            HWPAL_DMAResource_MemFree(RecordChunkPtrs_p);
        }

        if (MaxHandles * sizeof(uint32_t) > KMALLOC_MAX_SIZE)
        { // Were allocated with get_free pages.
            int order = get_order(MaxHandles * sizeof(uint32_t));

            if (Handles_p)
                free_pages((unsigned long)Handles_p, order);

            if (FreeHandles.Nrs_p)
                free_pages((unsigned long)FreeHandles.Nrs_p, order);

            if (FreeRecords.Nrs_p)
                free_pages((unsigned long)FreeRecords.Nrs_p, order);
        }
        else
        {
            if (Handles_p)
                HWPAL_DMAResource_MemFree(Handles_p);

            if (FreeHandles.Nrs_p)
                HWPAL_DMAResource_MemFree(FreeHandles.Nrs_p);

            if (FreeRecords.Nrs_p)
                HWPAL_DMAResource_MemFree(FreeRecords.Nrs_p);
        }
        RecordChunkPtrs_p = NULL;
        Handles_p = NULL;
        FreeHandles.Nrs_p = NULL;
        FreeRecords.Nrs_p = NULL;

        if (HWPAL_Lock_p != NULL)
            HWPAL_DMAResource_Lock_Free(HWPAL_Lock_p);

        return false;
    }

    // initialize the record numbers freelist
    // initialize the handle numbers freelist
    // initialize the handles array
    {
        unsigned int i;
        unsigned int chunk=0;
        unsigned int recidx=0;

        for (i = 0; i < MaxHandles; i++)
        {
            Handles_p[i] = HWPAL_INVALID_INDEX;
            FreeHandles.Nrs_p[i] = MaxHandles - 1 - i;
            FreeRecords.Nrs_p[i] = COMPOSE_RECNR(chunk,recidx);
            recidx++;
            if(recidx == MAX_RECORDS_PER_CHUNK)
            {
                chunk++;
                recidx = 0;
            }
        }

        FreeHandles.ReadIndex = 0;
        FreeHandles.WriteIndex = 0;

        FreeRecords.ReadIndex = 0;
        FreeRecords.WriteIndex = 0;
    }

    HandlesCount = MaxHandles;

    return true;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_UnInit
 *
 * This function can be used to uninitialize the DMAResource administration.
 * The caller must make sure that handles will not be used after this function
 * returns.
 * If memory was allocated by HWPAL_DMAResource_Init, this function will free it.
 */
void
HWPAL_DMAResource_UnInit(void)
{
    // exit if not initialized
    if (HandlesCount == 0)
        return;

    // find resource leaks
#ifdef HWPAL_TRACE_DMARESOURCE_LEAKS
    {
        int i;
        bool fFirstPrint = true;

        for (i = 0; i < HandlesCount; i++)
        {
            uint32_t IdxPair = Handles_p[i];

            if (IdxPair != HWPAL_INVALID_INDEX)
            {
                if (fFirstPrint)
                {
                    fFirstPrint = false;
                    Log_FormattedMessage(
                            "HWPAL_DMAResource_UnInit found leaking handles:\n");
                }

                Log_FormattedMessage(
                    "Handle %p => "
                    "Record %u\n",
                    Handles_p + i,
                    IdxPair);

                {
                    DMAResource_AddrPair_t * Pair_p;
                    DMAResource_Record_t * Rec_p =
                        DMAResourceLib_IdxPair2RecordPtr(IdxPair);

                    if(Rec_p != 0)
                    {
                        Pair_p = DMAResourceLib_LookupDomain(Rec_p,
                                                          DMARES_DOMAIN_HOST);

                        Log_FormattedMessage(
                            "  AllocatedSize = %d\n"
                            "  Alignment = %d\n"
                            "  Host address = %p\n",
                            Rec_p->BufferSize,
                            Rec_p->Props.Alignment,
                            Pair_p->Address_p);
                    }
                    else
                    {
                        Log_FormattedMessage(" bad index pair\n");
                    }
                }
            } // if
        } // for

        if (fFirstPrint)
            Log_FormattedMessage(
                "HWPAL_DMAResource_UnInit: no leaks found\n");
    }
#endif /* HWPAL_TRACE_DMARESOURCE_LEAKS */

    {
        int i;

        for (i = 0; i < ChunksCount; i++)
            HWPAL_DMAResource_MemFree(RecordChunkPtrs_p[i]);
    }
    HWPAL_DMAResource_MemFree(RecordChunkPtrs_p);

    if (HandlesCount * sizeof(uint32_t) > KMALLOC_MAX_SIZE)
    { // Were allocated with get_free pages.
        int order = get_order(HandlesCount * sizeof(uint32_t));

        free_pages((unsigned long)Handles_p, order);
        free_pages((unsigned long)FreeHandles.Nrs_p, order);
        free_pages((unsigned long)FreeRecords.Nrs_p, order);
    }
    else
    {
        HWPAL_DMAResource_MemFree(FreeHandles.Nrs_p);
        HWPAL_DMAResource_MemFree(FreeRecords.Nrs_p);
        HWPAL_DMAResource_MemFree(Handles_p);
    }

    if (HWPAL_Lock_p != NULL)
        HWPAL_DMAResource_Lock_Free(HWPAL_Lock_p);

    FreeHandles.Nrs_p = NULL;
    FreeRecords.Nrs_p = NULL;
    Handles_p = NULL;
    RecordChunkPtrs_p = NULL;

    HandlesCount = 0;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_CheckAndRegister
 */
int
HWPAL_DMAResource_CheckAndRegister(
        const DMAResource_Properties_t RequestedProperties,
        const DMAResource_AddrPair_t AddrPair,
        const char AllocatorRef,
        DMAResource_Handle_t * const Handle_p)
{
    DMAResource_Properties_t ActualProperties = RequestedProperties;
    DMAResource_AddrPair_t * Pair_p;
    DMAResource_Record_t * Rec_p;
    DMAResource_Handle_t Handle;
    dma_addr_t DMAAddr = 0;

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
    void* HostAddr = NULL;
#endif

    if (NULL == Handle_p)
    {
        return -1;
    }

    if (!DMAResourceLib_IsSaneInput(&AddrPair, &ActualProperties))
    {
        return 1;
    }

    if (AddrPair.Domain != DMARES_DOMAIN_HOST)
    {
        LOG_WARN(
            "HWPAL_DMAResource_CheckAndRegister: "
            "Unsupported domain: %u\n",
            AddrPair.Domain);
        return 1;
    }

    if (AllocatorRef != 'k' && AllocatorRef != 'R' &&
        AllocatorRef != 'N' && AllocatorRef != 'C')
    {
        LOG_WARN(
            "HWPAL_DMAResource_CheckAndRegister: "
            "Unsupported AllocatorRef: %c\n",
            AllocatorRef);

        return 1;
    }

    // allocate record -> Handle & Rec_p
    Handle = DMAResource_CreateRecord();
    if (Handle == NULL)
    {
        return -1;
    }

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        return -1;
    }

#ifdef HWPAL_ARCH_COHERENT
    ActualProperties.fCached    = false;
#else
    // This implementation does not allocate to non-cached resources
    ActualProperties.fCached    = true;
#endif // HWPAL_ARCH_COHERENT

#ifdef HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
    ActualProperties.fCached    = false;
#endif // HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT

    DMAResourceLib_Setup_Record(
            &ActualProperties,
            AllocatorRef,
            Rec_p,
            ActualProperties.Size);

    Pair_p = Rec_p->AddrPairs;
    if (AllocatorRef == 'k' || AllocatorRef == 'R')
    {
        struct device * DMADevice_p;

        // Get device reference for this resource
        DMADevice_p = Device_GetReference(NULL);

#ifdef HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
        if (!is_kernel_addr ((unsigned long)AddrPair.Address_p))
        {
            DMAResource_DestroyRecord(Handle);
            LOG_WARN(
                "HWPAL_DMAResource_CheckAndRegister: "
                "Unsupported host address: 0x%p\n",
                AddrPair.Address_p);

            return -1;
        }

        DMAAddr = (dma_addr_t)virt_to_phys (AddrPair.Address_p);
#else
        if (AllocatorRef == 'k')
        {
            // Note: this function can create a bounce buffer!
#ifdef MTK_EIP97_DRIVER
						DMAAddr = dma_map_single(NULL,
                                     AddrPair.Address_p,
                                     ActualProperties.Size,
                                     DMA_BIDIRECTIONAL);
#else
            DMAAddr = dma_map_single(DMADevice_p,
                                     AddrPair.Address_p,
                                     ActualProperties.Size,
                                     DMA_BIDIRECTIONAL);
#endif
        }
        else if (AllocatorRef == 'R')
        {
            DMAResource_Record_t * ParentRec_p;
            DMAResource_AddrPair_t *ParentHostPair_p,*ParentBusPair_p;
            uint32_t SubsetOffset;

            ParentRec_p = DMAResourceLib_Find_Matching_DMAResource(
                &ActualProperties,
                AddrPair);
            if (ParentRec_p == NULL)
            {
                DMAAddr = 0;
            }
            else
            {
                ParentHostPair_p = DMAResourceLib_LookupDomain(
                    ParentRec_p,
                    DMARES_DOMAIN_HOST);
                ParentBusPair_p = DMAResourceLib_LookupDomain(
                    ParentRec_p,
                    DMARES_DOMAIN_BUS);
                if (ParentHostPair_p == NULL || ParentBusPair_p == NULL)
                {
                    DMAAddr = 0;
                }
                else
                {
                    SubsetOffset = (uint32_t)((uint8_t *)AddrPair.Address_p -
                                              (uint8_t *)ParentHostPair_p->Address_p);
                    DMAAddr = (dma_addr_t)ParentBusPair_p->Address_p + SubsetOffset;
                    LOG_INFO(
                        "HWPAL_DMAResource_CheckAndRegister: "
                        "Registered subset buffer, offset %u\n",
                        SubsetOffset);
                }
            }
            printk("===HWPAL_DMAResource_CheckAndRegister R VA=%08X DMAAddr=%08X===\n",AddrPair.Address_p,DMAAddr);
            //DMAAddr = (dma_addr_t)virt_to_phys(AddrPair.Address_p);
            //printk("!!! do virt_to_phys from %08X to %08X ===\n",AddrPair.Address_p,DMAAddr);
        }
#endif // HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL

        if (0 == DMAAddr)
        {
            DMAResource_DestroyRecord(Handle);
            LOG_CRIT(
                "HWPAL_DMAResource_CheckAndRegister: "
                "Failed to obtain DMA address for host address 0x%p, "
                "for handle 0x%p\n",
                AddrPair.Address_p,
                Handle);

            return -1;
        }

        Pair_p->Address_p = (void*)DMAAddr;
        Pair_p->Domain = DMARES_DOMAIN_BUS;

        ++Pair_p;
        Pair_p->Address_p = AddrPair.Address_p;
        Pair_p->Domain = DMARES_DOMAIN_HOST;

        // This host address will be used for freeing the allocated buffer
        ++Pair_p;
        Pair_p->Address_p = AddrPair.Address_p;
        Pair_p->Domain = DMARES_DOMAIN_HOST_UNALIGNED;

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
        HostAddr = AddrPair.Address_p;
#endif
    }
    else if (AllocatorRef == 'N' || AllocatorRef == 'C')
    {
        Pair_p->Address_p = AddrPair.Address_p;
        Pair_p->Domain = DMARES_DOMAIN_HOST;

        // This host address will be used for freeing the allocated buffer
        ++Pair_p;
        Pair_p->Address_p = AddrPair.Address_p;
        Pair_p->Domain = DMARES_DOMAIN_HOST_UNALIGNED;

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
        HostAddr = AddrPair.Address_p;
        DMAAddr = 0;
#endif
    }

#ifdef HWPAL_TRACE_DMARESOURCE_BUF
    LOG_INFO("HWPAL_DMAResource_CheckAndRegister (1/2): "
             "handle = 0x%p, allocator='%c', "
             "size allocated/requested=%d/%d, \n"
             "HWPAL_DMAResource_CheckAndRegister (2/2): "
             "alignment/bank/cached=%d/%d/%d, "
             "bus addr=0x%p, host addr=0x%p\n",
             Handle, Rec_p->AllocatorRef,
             Rec_p->BufferSize, Rec_p->Props.Size,
             Rec_p->Props.Alignment, Rec_p->Props.Bank, Rec_p->Props.fCached,
             (void*)DMAAddr, HostAddr);
#endif

    *Handle_p = Handle;
    return 0;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Record_Update
 */
int
HWPAL_DMAResource_Record_Update(
        const int Identifier,
        DMAResource_Record_t * const Rec_p)
{
    IDENTIFIER_NOT_USED(Identifier);
    IDENTIFIER_NOT_USED(Rec_p);

    return 0; // Success, empty implementation
}

/*----------------------------------------------------------------------------
 * DMAResource_CreateRecord
 *
 * This function can be used to create a record. The function returns a handle
 * for the record. Use DMAResource_Handle2RecordPtr to access the record.
 * Destroy the record when no longer required, see DMAResource_Destroy.
 * This function initializes the record to all zeros.
 *
 * Return Values
 *     Handle for the DMA Resource.
 *     NULL is returned when the creation failed.
 */
DMAResource_Handle_t
DMAResource_CreateRecord(void)
{
    unsigned long flags;
    uint32_t HandleNr;
    uint32_t IdxPair = 0;

    // return NULL when not initialized
    if (HandlesCount == 0)
        return NULL;

    HWPAL_DMAResource_Lock_Acquire(HWPAL_Lock_p, &flags);

    HandleNr = DMAResourceLib_FreeList_Get(&FreeHandles);
    if (HandleNr != HWPAL_INVALID_INDEX)
    {
        IdxPair = DMAResourceLib_FreeList_Get(&FreeRecords);
        if (IdxPair == HWPAL_INVALID_INDEX)
        {
            DMAResourceLib_FreeList_Add(&FreeHandles, HandleNr);
            HandleNr = HWPAL_INVALID_INDEX;
        }
    }

    HWPAL_DMAResource_Lock_Release(HWPAL_Lock_p, &flags);

    // return NULL when reservation failed
    if (HandleNr == HWPAL_INVALID_INDEX)
    {
        LOG_CRIT("DMAResource_Create_Record: "
                 "reservation failed, out of handles\n");
        return NULL;
    }

    // initialize the record
    {
        DMAResource_Record_t * Rec_p =
                DMAResourceLib_IdxPair2RecordPtr(IdxPair);
        if(Rec_p == NULL)
        {
            LOG_CRIT(
                "DMAResource_Create_Record: "
                "Bad index pair returned %u\n",IdxPair);
            return NULL;
        }
        memset(Rec_p, 0, sizeof(DMAResource_Record_t));
        Rec_p->Magic = DMARES_RECORD_MAGIC;
    }

    // initialize the handle
    Handles_p[HandleNr] = IdxPair;

    // fill in the handle position
    return Handles_p + HandleNr;
}


/*----------------------------------------------------------------------------
 * DMAResource_DestroyRecord
 *
 * This function invalidates the handle and the record instance.
 *
 * Handle
 *     A valid handle that was once returned by DMAResource_CreateRecord or
 *     one of the DMA Buffer Management functions (Alloc/Register/Attach).
 *
 * Return Values
 *     None
 */
void
DMAResource_DestroyRecord(
        const DMAResource_Handle_t Handle)
{
    if (DMAResource_IsValidHandle(Handle))
    {
        uint32_t * p = (uint32_t *)Handle;
        uint32_t IdxPair = *p;

        if (DMAResourceLib_IdxPair2RecordPtr(IdxPair) != NULL)
        {
            unsigned long flags;
            int HandleNr = p - Handles_p;

            // note handle is no longer value
            *p = HWPAL_INVALID_INDEX;

            HWPAL_DMAResource_Lock_Acquire(HWPAL_Lock_p, &flags);

            // add the HandleNr and IdxPair to respective LRU lists
            DMAResourceLib_FreeList_Add(&FreeHandles, HandleNr);
            DMAResourceLib_FreeList_Add(&FreeRecords, IdxPair);

            HWPAL_DMAResource_Lock_Release(HWPAL_Lock_p, &flags);
        }
        else
        {
            LOG_WARN(
                "DMAResource_Destroy: "
                "Handle %p was already destroyed\n",
                Handle);
        }
    }
    else
    {
        LOG_WARN(
            "DMAResource_Destroy: "
            "Invalid handle %p\n",
            Handle);
    }
}


/*----------------------------------------------------------------------------
 * DMAResource_IsValidHandle
 *
 * This function tells whether a handle is valid.
 *
 * Handle
 *     A valid handle that was once returned by DMAResource_CreateRecord or
 *     one of the DMA Buffer Management functions (Alloc/Register/Attach).
 *
 * Return Value
 *     true   The handle is valid
 *     false  The handle is NOT valid
 */
bool
DMAResource_IsValidHandle(
        const DMAResource_Handle_t Handle)
{
    uint32_t * p = (uint32_t *)Handle;

    if (p < Handles_p ||
        p >= Handles_p + HandlesCount)
    {
        return false;
    }

    // check that the handle has not been destroyed yet
    if (*p == HWPAL_INVALID_INDEX)
    {
        return false;
    }

    return true;
}


/*----------------------------------------------------------------------------
 * DMAResource_Handle2RecordPtr
 *
 * This function can be used to get a pointer to the DMA resource record
 * (DMAResource_Record_t) for the provided handle. The pointer is valid until
 * the record and handle are destroyed.
 *
 * Handle
 *     A valid handle that was once returned by DMAResource_CreateRecord or
 *     one of the DMA Buffer Management functions (Alloc/Register/Attach).
 *
 * Return Value
 *     Pointer to the DMAResource_Record_t memory for this handle.
 *     NULL is returned if the handle is invalid.
 */
DMAResource_Record_t *
DMAResource_Handle2RecordPtr(
        const DMAResource_Handle_t Handle)
{
    uint32_t * p = (uint32_t *)Handle;

    if(!DMAResource_IsValidHandle(Handle))
    {
        return NULL;
    }

    if (p != NULL)
    {
        uint32_t IdxPair = *p;

        DMAResource_Record_t* Rec_p =
                DMAResourceLib_IdxPair2RecordPtr(IdxPair);

        if(Rec_p != NULL && Rec_p->Magic == DMARES_RECORD_MAGIC)
        {
            return Rec_p; // ## RETURN ##
        }
        else
        {
            return NULL; // ## RETURN ##
        }
    }

    return NULL;
}


/*----------------------------------------------------------------------------
 * DMAResource_PreDMA
 */
void
DMAResource_PreDMA(
        const DMAResource_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
    DMAResource_Record_t *Rec_p;
    unsigned int NBytes = ByteCount;
    unsigned int Offset = ByteOffset;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_PreDMA: "
            "Invalid handle %p\n",
            Handle);
        return;
    }

    if (NBytes == 0)
    {
        // Prepare the whole resource for the DMA operation
        NBytes = Rec_p->Props.Size;
        Offset = 0;
    }

    if ((Offset >= Rec_p->Props.Size) ||
        (NBytes > Rec_p->Props.Size) ||
        (Offset + NBytes > Rec_p->Props.Size))
    {
        LOG_CRIT(
            "DMAResource_PreDMA: "
            "Invalid range 0x%08x-0x%08x (not in 0x0-0x%08x)\n",
            ByteOffset,
            ByteOffset + ByteCount,
            Rec_p->Props.Size);
        return;
    }

    if (Rec_p->Props.fCached &&
        (Rec_p->AllocatorRef == 'k' || Rec_p->AllocatorRef == 'A' ||
         Rec_p->AllocatorRef == 'R'))
    {
        DMAResource_AddrPair_t * Pair_p;

#ifdef HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
        dma_addr_t DMAAddr;

        Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
        if (Pair_p == NULL)
        {
            LOG_WARN(
                "DMAResource_PreDMA: "
                "No host address found for Handle %p?\n",
                Handle);

            return;
        }

        DMAAddr = (dma_addr_t)Pair_p->Address_p;
#else
        dma_addr_t DMAAddr;

        Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_BUS);
        if (Pair_p == NULL)
        {
            LOG_WARN(
                "DMAResource_PreDMA: "
                "No bus address found for Handle %p?\n",
                Handle);

            return;
        }

        DMAAddr = (dma_addr_t)Pair_p->Address_p;
#endif // HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL

#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
        Log_FormattedMessage(
            "DMAResource_PreDMA: "
            "Handle=%p, "
            "offset=%u, size=%u, "
            "allocator='%c', "
            "addr=0x%p\n",
            Handle,
            Offset, NBytes,
            Rec_p->AllocatorRef,
            (void*)DMAAddr);
#endif

#ifndef HWPAL_ARCH_COHERENT
        {
            struct device * DMADevice_p;
            size_t size = NBytes;

            // Get device reference for this resource
            DMADevice_p = Device_GetReference(NULL);

#ifdef HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
            {
                uint8_t* DMABuffer_p = (uint8_t*)DMAAddr;

                dma_cache_sync(DMADevice_p, DMABuffer_p + Offset,
                               size, DMA_TO_DEVICE);
            }
#else
#ifdef MTK_EIP97_DRIVER
						//dma_sync_single_range_for_device(NULL, DMAAddr,
            //        Offset, size, DMA_BIDIRECTIONAL);
            dma_sync_single_for_device(NULL, DMAAddr+Offset, size, DMA_TO_DEVICE);
#else 
           dma_sync_single_for_device(DMADevice_p, DMAAddr+Offset, size, DMA_BIDIRECTIONAL);
#endif
#endif // HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
        }
#endif
    }
}


/*----------------------------------------------------------------------------
 * DMAResource_PostDMA
 */
void
DMAResource_PostDMA(
        const DMAResource_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
    DMAResource_Record_t *Rec_p;
    unsigned int NBytes = ByteCount;
    unsigned int Offset = ByteOffset;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_PostDMA: "
            "Invalid handle %p\n",
            Handle);
        return;
    }

    if (NBytes == 0)
    {
        // Prepare the whole resource for the DMA operation
        NBytes = Rec_p->Props.Size;
        Offset = 0;
    }

    if ((ByteOffset >= Rec_p->Props.Size) ||
        (NBytes > Rec_p->Props.Size) ||
        (ByteOffset + NBytes > Rec_p->Props.Size))
    {
        LOG_CRIT(
            "DMAResource_PostDMA: "
            "Invalid range 0x%08x-0x%08x (not in 0x0-0x%08x)\n",
            ByteOffset,
            ByteOffset + NBytes,
            Rec_p->Props.Size);
        return;
    }

    if (Rec_p->Props.fCached &&
        (Rec_p->AllocatorRef == 'k' || Rec_p->AllocatorRef == 'A' ||
         Rec_p->AllocatorRef == 'R'))
    {
        DMAResource_AddrPair_t * Pair_p;
#ifdef HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
        dma_addr_t DMAAddr;

        Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
        if (Pair_p == NULL)
        {
            LOG_WARN(
                "DMAResource_PostDMA: "
                "No host address found for Handle %p?\n",
                Handle);

            return;
        }

        DMAAddr = (dma_addr_t)Pair_p->Address_p;
#else
        dma_addr_t DMAAddr;

        Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_BUS);
        if (Pair_p == NULL)
        {
            LOG_WARN(
                "DMAResource_PostDMA: "
                "No bus address found for Handle %p?\n",
                Handle);

            return;
        }

        DMAAddr = (dma_addr_t)Pair_p->Address_p;
#endif // HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL

#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
        Log_FormattedMessage(
            "DMAResource_PostDMA: "
            "Handle=%p, "
            "offset=%u, size=%u), "
            "allocator='%c', "
            "addr 0x%p\n",
            Handle,
            Offset, NBytes,
            Rec_p->AllocatorRef,
            (void*)DMAAddr);
#endif

#ifndef HWPAL_ARCH_COHERENT
        {
            struct device * DMADevice_p;
            size_t size = NBytes;

            // Get device reference for this resource
            DMADevice_p = Device_GetReference(NULL);

#ifdef HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
            {
                uint8_t* DMABuffer_p = (uint8_t*)DMAAddr;

                dma_cache_sync(DMADevice_p, DMABuffer_p + Offset,
                               size, DMA_FROM_DEVICE);
            }
#else
#ifdef MTK_EIP97_DRIVER
						//dma_sync_single_range_for_cpu(NULL, DMAAddr, Offset,
            //                              size, DMA_BIDIRECTIONAL);
						dma_sync_single_for_cpu(NULL, DMAAddr+Offset, size, DMA_FROM_DEVICE);
#else
            dma_sync_single_for_cpu(DMADevice_p, DMAAddr+Offset,
                                          size, DMA_BIDIRECTIONAL);
#endif
#endif // HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
        }
#endif
    }
}


/*----------------------------------------------------------------------------
 * DMAResource_Attach
 */
int
DMAResource_Attach(
        const DMAResource_Properties_t ActualProperties,
        const DMAResource_AddrPair_t AddrPair,
        DMAResource_Handle_t * const Handle_p)
{
    IDENTIFIER_NOT_USED(Handle_p);
    IDENTIFIER_NOT_USED(AddrPair.Address_p);
    IDENTIFIER_NOT_USED(ActualProperties.Alignment);

    return -1; // Not implemented
}


/* end of file dmares_lkm.c */

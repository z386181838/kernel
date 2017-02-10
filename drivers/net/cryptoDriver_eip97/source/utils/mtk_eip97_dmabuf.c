/* adapter_dmabuf.c
 *
 * Implementation of the DMA Buffer Allocation API.
 */

/*****************************************************************************
* Copyright (c) 2008-2013 INSIDE Secure B.V. All Rights Reserved.
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
 * This module implements (provides) the following interface(s):
 */

// DMABuf API
#include "api_dmabuf.h"

// Adapter DMABuf internal API
#include "adapter_dmabuf.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Logging API
#include "log.h"

// Driver Framework DMAResource API
#include "dmares_types.h"
#include "dmares_mgmt.h"
#include "dmares_buf.h"
#include "dmares_addr.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"

// Driver Framework C Run-Time Library API
#include "clib.h"               // memcmp


/*----------------------------------------------------------------------------
 * DMABuf_NULLHandle
 *
 */
const DMABuf_Handle_t DMABuf_NULLHandle = { 0 };

/*----------------------------------------------------------------------------
 * Adapter_DMABuf_Handle2DMAResourceHandle
 */
DMAResource_Handle_t
Adapter_DMABuf_Handle2DMAResourceHandle(
        DMABuf_Handle_t Handle)
{
    if (Handle.p == NULL)
    {
        return NULL;
    }
    else
    {
        return (DMAResource_Handle_t)Handle.p;
    }
}


/*----------------------------------------------------------------------------
 * Adapter_DMAResource_Handle2DMABufHandle
 */
DMABuf_Handle_t
Adapter_DMAResource_Handle2DMABufHandle(
        DMAResource_Handle_t Handle)
{
    DMABuf_Handle_t DMABuf_Handle;

    DMABuf_Handle.p = Handle;

    return DMABuf_Handle;
}


/*----------------------------------------------------------------------------
 * Adapter_DMAResource_IsForeignAllocated
 */
bool
Adapter_DMAResource_IsForeignAllocated(
        DMAResource_Handle_t Handle)
{
    DMAResource_Record_t * Rec_p;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);

    if(!Rec_p)
    {
        return false;
    }
    else
    {
        return Rec_p->AllocatorRef != 'A';
    }
}


/*----------------------------------------------------------------------------
 * Adapter_DMAResource_HostAddr
 */
void *
Adapter_DMAResource_HostAddr(
        DMAResource_Handle_t Handle)
{
    DMAResource_AddrPair_t HostAddr;

    DMAResource_Translate(Handle, DMARES_DOMAIN_HOST, &HostAddr);

    return HostAddr.Address_p;
}


/*----------------------------------------------------------------------------
 * Adapter_DMAResource_IsSubRangeOf
 *
 * Return true if the address range defined by Handle1 is
 * within the address range defined by Handle2.
 */
bool
Adapter_DMAResource_IsSubRangeOf(
        const DMAResource_Handle_t Handle1,
        const DMAResource_Handle_t Handle2)
{
    DMAResource_AddrPair_t AddrPair1, AddrPair2;

    DMAResource_Translate(Handle1, DMARES_DOMAIN_HOST, &AddrPair1);
    DMAResource_Translate(Handle2, DMARES_DOMAIN_HOST, &AddrPair2);

    if (AddrPair1.Domain == AddrPair2.Domain)
    {
        const uint8_t * Addr1 = AddrPair1.Address_p;
        const uint8_t * Addr2 = AddrPair2.Address_p;
        const DMAResource_Record_t * const Rec1_p =
                                DMAResource_Handle2RecordPtr(Handle1);
        const DMAResource_Record_t * const Rec2_p =
                                DMAResource_Handle2RecordPtr(Handle2);

        if ((Rec1_p->Props.Size <= Rec2_p->Props.Size) &&
            (Addr2 <= Addr1) &&
            ((Addr1 + Rec1_p->Props.Size) <= (Addr2 + Rec2_p->Props.Size)))
        {
            return true;
        }
    }

    return false;
}


/*----------------------------------------------------------------------------
 * DMABuf_Handle_IsSame
 */
bool
DMABuf_Handle_IsSame(
        const DMABuf_Handle_t * const Handle1_p,
        const DMABuf_Handle_t * const Handle2_p)
{
    if (memcmp(Handle1_p, Handle2_p, sizeof(DMABuf_Handle_t)) == 0)
    {
        return true;
    }

    return false;
}

/*----------------------------------------------------------------------------
 * DMABuf_Alloc
 */
DMABuf_Status_t
DMABuf_Alloc(
        const DMABuf_Properties_t RequestedProperties,
        DMABuf_HostAddress_t * const Buffer_p,
        DMABuf_Handle_t * const Handle_p)
{
    DMAResource_Handle_t DMAHandle;
    DMAResource_AddrPair_t AddrPair;
    DMAResource_Properties_t ActualProperties;

    ZEROINIT(AddrPair);
    ZEROINIT(ActualProperties);

    if (Handle_p == NULL ||
        Buffer_p == NULL)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    // initialize the output parameters
    Handle_p->p = NULL;
    Buffer_p->p = NULL;

    ActualProperties.Size       = RequestedProperties.Size;
    ActualProperties.Alignment  = RequestedProperties.Alignment;
    ActualProperties.Bank       = RequestedProperties.Bank;
    ActualProperties.fCached    = RequestedProperties.fCached;

    if( !DMAResource_Alloc(ActualProperties,&AddrPair,&DMAHandle) )
    {
        // set the output parameters
        Handle_p->p = (void*)DMAHandle;
        Buffer_p->p = AddrPair.Address_p;

        LOG_INFO("DMABuf_Alloc: allocated handle=0x%p, host addr=0x%p\n",
                 Handle_p->p,Buffer_p->p);

        return DMABUF_STATUS_OK;
    }
    else
    {
        return DMABUF_ERROR_OUT_OF_MEMORY;
    }
}


/*----------------------------------------------------------------------------
 * DMABuf_Register
 */
DMABuf_Status_t
DMABuf_Register(
        const DMABuf_Properties_t RequestedProperties,
        void * Buffer_p,
        void * Alternative_p,
        const char AllocatorRef,
        DMABuf_Handle_t * const Handle_p)
{
    DMAResource_Handle_t DMAHandle;
    char ActualAllocator;
    DMAResource_AddrPair_t AddrPair;
    DMAResource_Properties_t ActualProperties;

    ZEROINIT(AddrPair);
    ZEROINIT(ActualProperties);

    if (Handle_p == NULL ||
        Buffer_p == NULL)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    // initialize the output parameter
    Handle_p->p = NULL;

    ActualProperties.Size       = RequestedProperties.Size;
    ActualProperties.Alignment  = RequestedProperties.Alignment;
    ActualProperties.Bank       = RequestedProperties.Bank;
    ActualProperties.fCached    = RequestedProperties.fCached;

    ActualAllocator = AllocatorRef;

    if( AllocatorRef == 'k'  || AllocatorRef == 'N' || AllocatorRef == 'R'
        || AllocatorRef == 'C')
    {
        // 'N' is used to register buffers that do not need to be DMA-safe.
        // 'R' is used to register (subranges of) buffers that are already
        //     allocated with DMAResource_Alloc()/DMABuf_Alloc().
        // 'k' is supported for Linux kmalloc() allocator only,
        //     e.g. AllocatorRef = 'k' for streaming DMA mappings
        // 'C' is supported for coherent buffers.
        AddrPair.Domain     = DMARES_DOMAIN_HOST;
        AddrPair.Address_p  = Buffer_p;
    }
    else if( AllocatorRef == 0 )
    {
        // Linux kmalloc() allocator is used,
        // e.g. AllocatorRef = 'k' for streaming DMA mappings
        ActualAllocator = 'k';
        AddrPair.Domain     = DMARES_DOMAIN_HOST;
        AddrPair.Address_p  = Buffer_p;
    }
    else
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    if( DMAResource_CheckAndRegister(ActualProperties,AddrPair,
            ActualAllocator,&DMAHandle) == 0 )
    {
        if( ActualAllocator == 'C' )
        {
            // Add bus address for the resource for AllocatorRef = 'C'
            AddrPair.Domain     = DMARES_DOMAIN_BUS;
            AddrPair.Address_p  = Alternative_p;

            DMAResource_AddPair(DMAHandle,AddrPair);
        }

        // set the output parameters
        Handle_p->p = (void*)DMAHandle;

        LOG_INFO("DMABuf_Register: registered handle=0x%p, host addr=0x%p, "
                 "allocator=%d\n",
                 Handle_p->p, Buffer_p,
                 AllocatorRef);

        return DMABUF_STATUS_OK;
    }
    else
    {
        return DMABUF_ERROR_OUT_OF_MEMORY;
    }
}


/*----------------------------------------------------------------------------
 * DMABuf_Release
 */
DMABuf_Status_t
DMABuf_Release(
        DMABuf_Handle_t Handle)
{
    DMAResource_Handle_t DMAHandle =
            Adapter_DMABuf_Handle2DMAResourceHandle(Handle);

    LOG_INFO("DMABuf_Release: handle to release=0x%p\n",Handle.p);

    if( DMAResource_Release(DMAHandle) == 0 )
    {
        return DMABUF_STATUS_OK;
    }
    else
    {
        return DMABUF_ERROR_INVALID_HANDLE;
    }
}


/* end of file adapter_dmabuf.c */

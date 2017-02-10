/* eip202_rdr_init.c
 *
 * EIP-202 Ring Control Driver Library
 * RDR Init/Reset API implementation
 */

/*****************************************************************************
* Copyright (c) 2011-2013 INSIDE Secure B.V. All Rights Reserved.
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

#include "eip202_rdr.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// EIP-202 Ring Control Driver Library Types API
#include "eip202_ring_types.h"          // EIP202_Ring_* types

// EIP-202 Ring Control Driver Library Internal interfaces
#include "eip202_ring_internal.h"
#include "eip202_rdr_level0.h"          // EIP-202 Level 0 macros
#include "eip202_rdr_fsm.h"             // RDR State machine
#include "eip202_rdr_dscr.h"            // RingHelper callbacks
#include "eip202_rd_format.h"           // EIP-202 Result Descriptor

// RingHelper API
#include "ringhelper.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                // IDENTIFIER_NOT_USED, bool, uint32_t

// Driver Framework Device API
#include "device_types.h"              // Device_Handle_t

// Driver Framework DMA Resource API
#include "dmares_types.h"         // types of the DMA resource API
#include "dmares_rw.h"            // read/write of the DMA resource API.

// Driver Framework C Run-Time Library API
#include "clib.h"

#include "log.h"
/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202Lib_Detect
 *
 * Checks the presence of EIP-202 HIA hardware. Returns true when found.
 */
static bool
EIP202Lib_RDR_Detect(
        const Device_Handle_t Device)
{
    uint32_t Value;

    // read-write test one of the registers

    // Set MASK_31_BITS bits of the EIP202_RDR_RING_BASE_ADDR_LO register
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_RING_BASE_ADDR_LO,
                       MASK_31_BITS );

    Value = EIP202_RDR_Read32(Device, EIP202_RDR_RING_BASE_ADDR_LO);
    if ((Value & MASK_31_BITS) != MASK_31_BITS)
        return false;

    // Clear MASK_31_BITS bits of the EIP202_RDR_RING_BASE_ADDR_LO register
    EIP202_RDR_Write32(Device, EIP202_RDR_RING_BASE_ADDR_LO, 0);
    Value = EIP202_RDR_Read32(Device, EIP202_RDR_RING_BASE_ADDR_LO);
    if ((Value & MASK_31_BITS) != 0)
       return false;

    return true;
}


/*----------------------------------------------------------------------------
 * EIP202Lib_RDR_ClearAllDescriptors
 *
 * Clear all descriptors
 */
static inline void
EIP202Lib_RDR_ClearAllDescriptors(
        DMAResource_Handle_t Handle,
        const uint32_t DescriptorSpacingWordCount,
        const uint32_t DescriptorSizeWordCount,
        const uint32_t NumberOfDescriptors)
{
    static const uint32_t Words[EIP202_RD_MAX_WORD_COUNT]; // Will be set to 0
    unsigned int i;

    for(i = 0; i < NumberOfDescriptors; i++)
    {
        DMAResource_Write32Array(
                Handle,
                i * DescriptorSpacingWordCount,
                DescriptorSizeWordCount,
                Words);
    }
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Init
 *
 */
EIP202_Ring_Error_t
EIP202_RDR_Init(
        EIP202_Ring_IOArea_t * IOArea_p,
        const Device_Handle_t Device,
        const EIP202_RDR_Settings_t * const RDRSettings_p)
{
    uint16_t RDFIFOWordCount;
    EIP202_Ring_Error_t rv;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p = RDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);

    // Initialize the IO Area
    TrueIOArea_p->Device = Device;
    TrueIOArea_p->State = (unsigned int)EIP202_RDR_STATE_UNINITIALIZED;

    // Check if the CPU integer size is enough to store 32-bit value
    if(sizeof(unsigned int) < sizeof(uint32_t))
        return EIP202_RING_UNSUPPORTED_FEATURE_ERROR;

    // Detect presence of EIP-202 CDR hardware
    if(!EIP202Lib_RDR_Detect(Device))
        return EIP202_RING_UNSUPPORTED_FEATURE_ERROR;

    // Extension of 32-bit pointers to 64-bit addresses not supported.
    if(RDRSettings_p->Params.DMA_AddressMode == EIP202_RING_64BIT_DMA_EXT_ADDR)
        return EIP202_RING_UNSUPPORTED_FEATURE_ERROR;

    if(RDRSettings_p->Params.DscrOffsWordCount == 0 ||
       RDRSettings_p->Params.DscrOffsWordCount <
       RDRSettings_p->Params.DscrSizeWordCount)
        return EIP202_RING_ARGUMENT_ERROR;

    // Ring size cannot be smaller than one descriptor size or
    // larger than 4194303 (16MB / 4 - 1), in 32-bit words
    if(RDRSettings_p->Params.RingSizeWordCount <
       RDRSettings_p->Params.DscrOffsWordCount ||
       RDRSettings_p->Params.RingSizeWordCount > 4194303)
        return EIP202_RING_ARGUMENT_ERROR;

    // Read Result Descriptor FIFO size (in 32-bit words)
    EIP202_RDR_STAT_FIFO_SIZE_RD(Device, &RDFIFOWordCount);

    if(RDRSettings_p->Params.DscrSizeWordCount > EIP202_RD_MAX_WORD_COUNT)
        return EIP202_RING_ARGUMENT_ERROR;

    // ToDo: WA for the HIA_RDR_y_STAT reg rd_fifo_free_count field bug
    if( /* RDRSettings_p->Params.DscrThresholdWordCount > RDFIFOWordCount || */
        (RDRSettings_p->Params.DscrFetchSizeWordCount &
                (RDRSettings_p->Params.DscrOffsWordCount - 1)) != 0)
        return EIP202_RING_ARGUMENT_ERROR;

    if( RDRSettings_p->Params.IntThresholdDscrCount *
            RDRSettings_p->Params.DscrOffsWordCount >
          RDRSettings_p->Params.RingSizeWordCount )
        return EIP202_RING_ARGUMENT_ERROR;

    // Configure the Ring Helper
    TrueIOArea_p->RingHelperCallbacks.WriteFunc_p = &EIP202_RDR_WriteCB;
    TrueIOArea_p->RingHelperCallbacks.ReadFunc_p = &EIP202_RDR_ReadCB;
    TrueIOArea_p->RingHelperCallbacks.StatusFunc_p = &EIP202_RDR_StatusCB;
    TrueIOArea_p->RingHelperCallbacks.CallbackParam1_p = IOArea_p;
    TrueIOArea_p->RingHelperCallbacks.CallbackParam2 = 0;
    TrueIOArea_p->RingHandle = RDRSettings_p->Params.RingDMA_Handle;
    TrueIOArea_p->DescOffsWordCount = RDRSettings_p->Params.DscrOffsWordCount;
    TrueIOArea_p->RingSizeWordCount = RDRSettings_p->Params.RingSizeWordCount;

    // Initialize one RingHelper instance for one RDR instance
    if( RingHelper_Init(
         (volatile RingHelper_t*)&TrueIOArea_p->RingHelper,
         (volatile RingHelper_CallbackInterface_t*)&TrueIOArea_p->RingHelperCallbacks,
         false, // One RDR as combined rings
         (unsigned int)(RDRSettings_p->Params.RingSizeWordCount /
             RDRSettings_p->Params.DscrOffsWordCount),
         (unsigned int)(RDRSettings_p->Params.RingSizeWordCount /
             RDRSettings_p->Params.DscrOffsWordCount)) < 0)
        return EIP202_RING_ARGUMENT_ERROR;

    // Transit to a new state
    rv = EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                             EIP202_RDR_STATE_INITIALIZED);
    if(rv != EIP202_RING_NO_ERROR)
        return EIP202_RING_ILLEGAL_IN_STATE;

    // Prepare the RDR DMA buffer
    // Initialize all descriptors with zero for RDR
    EIP202Lib_RDR_ClearAllDescriptors(
            TrueIOArea_p->RingHandle,
            RDRSettings_p->Params.DscrOffsWordCount,
            RDRSettings_p->Params.DscrSizeWordCount,
            RDRSettings_p->Params.RingSizeWordCount /
                 RDRSettings_p->Params.DscrOffsWordCount);

    // Call PreDMA to make sure engine sees it
    DMAResource_PreDMA(TrueIOArea_p->RingHandle,
                       0,
                       (unsigned int)(TrueIOArea_p->RingSizeWordCount*4));

    EIP202_RDR_RING_BASE_ADDR_LO_WR(
                       Device,
                       RDRSettings_p->Params.RingDMA_Address.Addr);

    EIP202_RDR_RING_BASE_ADDR_HI_WR(
                       Device,
                       RDRSettings_p->Params.RingDMA_Address.UpperAddr);

    EIP202_RDR_RING_SIZE_WR(
                       Device,
                       RDRSettings_p->Params.RingSizeWordCount);

    EIP202_RDR_DESC_SIZE_WR(
                       Device,
                       RDRSettings_p->Params.DscrSizeWordCount,
                       RDRSettings_p->Params.DscrOffsWordCount,
                       RDRSettings_p->Params.DMA_AddressMode == EIP202_RING_64BIT_DMA_DSCR_PTR);

	LOG_INFO("\n\t\t\t %s [%d] DscrOffsWordCount=%d DscrSizeWordCount=%d\n",__func__,__LINE__,RDRSettings_p->Params.DscrOffsWordCount,RDRSettings_p->Params.DscrSizeWordCount);         

    EIP202_RDR_CFG_WR(
                       Device,
                       RDRSettings_p->Params.DscrFetchSizeWordCount,
                       RDRSettings_p->Params.DscrThresholdWordCount,
                       true); // Propagate Buffer and Descriptor Overflow
                              // interrupts to the RDR interrupt output

    // Disable Processed Descriptor threshold interrupt,
    // Disable Timeout interrupt and stop timeout counter for
    // reducing power consumption
    EIP202_RDR_THRESH_WR(
                       Device,
                       TrueIOArea_p->RingSizeWordCount,
                       0,  // Set descriptor processing mode
                       0); // Disable timeout

    EIP202_RDR_DMA_CFG_WR(
                       Device,
                       (uint8_t)RDRSettings_p->Params.ByteSwap_Descriptor_Mask,
                       (uint8_t)RDRSettings_p->Params.ByteSwap_Packet_Mask,
                       // Bufferability control
                       false, // Don't buffer Result Token DMA writes
                       false, //Qwert true,  // Don't buffer Descriptor Control DMA writes
                       false, //Qwert true,  // Don't buffer Ownership Word DMA writes
                       0,     // Write cache type control
                       0);    // Read cache type control

    return EIP202_RING_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Reset
 *
 */
EIP202_Ring_Error_t
EIP202_RDR_Reset(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const Device_Handle_t Device)
{
    EIP202_Ring_Error_t rv;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p = RDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);

    // Initialize the IO Area
    memset((void*)IOArea_p, 0, sizeof(*TrueIOArea_p));
    TrueIOArea_p->Device = Device;
    TrueIOArea_p->State = (unsigned int)EIP202_RDR_STATE_UNKNOWN;

    // Transit to a new state
    rv = EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                             EIP202_RDR_STATE_UNINITIALIZED);
    if(rv != EIP202_RING_NO_ERROR)
        return EIP202_RING_ILLEGAL_IN_STATE;

    // Clear RDR count
    EIP202_RDR_PREP_COUNT_WR(Device, 0, true);
    EIP202_RDR_PROC_COUNT_WR(Device, 0, 0, true);

    // Re-init RDR
    EIP202_RDR_PREP_PNTR_DEFAULT_WR(Device);
    EIP202_RDR_PROC_PNTR_DEFAULT_WR(Device);

    // Restore default register values
    EIP202_RDR_RING_BASE_ADDR_LO_DEFAULT_WR(Device);
    EIP202_RDR_RING_BASE_ADDR_HI_DEFAULT_WR(Device);
    EIP202_RDR_RING_SIZE_DEFAULT_WR(Device);
    EIP202_RDR_DESC_SIZE_DEFAULT_WR(Device);
    EIP202_RDR_CFG_DEFAULT_WR(Device);
    EIP202_RDR_DMA_CFG_DEFAULT_WR(Device);
    EIP202_RDR_THRESH_DEFAULT_WR(Device);

    // Clear and disable all RDR interrupts
    EIP202_RDR_STAT_CLEAR_ALL_IRQ_WR(Device);

    return EIP202_RING_NO_ERROR;
}


/* end of file eip202_rdr_init.c */



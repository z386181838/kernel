/* c_adapter_eip202.h
 *
 * Default Adapter EIP-202 configuration
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

#ifndef INCLUDE_GUARD_C_ADAPTER_EIP202_H
#define INCLUDE_GUARD_C_ADAPTER_EIP202_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"


// This parameter enables strict argument checking
//#define ADAPTER_EIP202_STRICT_ARGS

#ifndef ADAPTER_EIP202_GLOBAL_DEVICE_NAME
#define ADAPTER_EIP202_GLOBAL_DEVICE_NAME    "EIP202_GLOBAL"
#endif // ADAPTER_EIP202_GLOBAL_DEVICE_NAME

#ifndef ADAPTER_EIP202_DRIVER_NAME
#define ADAPTER_EIP202_DRIVER_NAME           "SafeXcel"
#endif

// This parameter enables the byte swap in 32-bit words
// for the EIP-202 CD Manager master interface
//#define ADAPTER_EIP202_CDR_BYTE_SWAP_ENABLE

// This parameter enables the byte swap in 32-bit words
// for the EIP-202 RD Manager master interface
//#define ADAPTER_EIP202_RDR_BYTE_SWAP_ENABLE

// This parameter enables the EIP-207 Record Cache interface
// for the record invalidation
//#define ADAPTER_EIP202_RC_SUPPORT

// This parameter enables the EIP-207 Record Cache interface
// for the record invalidation via the SHDevXS API.
// When not defined the Record Cache will be accessed directly
// via the EIP-207 Record Cache Driver Library API.
// This parameter must be defined together with ADAPTER_EIP202_RC_SUPPORT
// for the Record Cache access via the SHDevXS API.
//#define ADAPTER_EIP202_USE_SHDEVXS

// This parameter enables the EIP-202 64-bit DMA address support
//#define ADAPTER_EIP202_64BIT_DEVICE

// This parameter enables the DMA banks support
//#define ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

// This parameter enables the EIP-202 separate rings
// (CDR used separately from RDR)
//#define ADAPTER_EIP202_SEPARATE_RINGS

// This parameter enables the EIP-202 scatter-gather support
//#define ADAPTER_EIP202_ENABLE_SCATTERGATHER

// This parameter disables bounce buffers
//#define ADAPTER_EIP202_REMOVE_BOUNCEBUFFERS

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

// This parameter configures the maximum number of transform records
#ifndef ADAPTER_EIP202_TRANSFORM_RECORD_COUNT
#error "ADAPTER_EIP202_TRANSFORM_RECORD_COUNT is not defined"
#endif

// This parameter configures the maximum byte count of one transform record
#ifndef ADAPTER_EIP202_TRANSFORM_RECORD_BYTE_COUNT
#error "ADAPTER_EIP202_TRANSFORM_RECORD_BYTE_COUNT is not defined"
#endif

#endif // ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

// This parameter enables the EIP-202 interrupt support
//#define ADAPTER_EIP202_INTERRUPTS_ENABLE

#ifndef ADAPTER_EIP202_INTERRUPTS_TRACEFILTER
#define ADAPTER_EIP202_INTERRUPTS_TRACEFILTER   0
#endif

#ifndef ADAPTER_EIP202_PHY_CDR0_IRQ
#define ADAPTER_EIP202_PHY_CDR0_IRQ     0
#endif
#ifndef ADAPTER_EIP202_PHY_CDR1_IRQ
#define ADAPTER_EIP202_PHY_CDR1_IRQ     2
#endif
#ifndef ADAPTER_EIP202_PHY_CDR2_IRQ
#define ADAPTER_EIP202_PHY_CDR2_IRQ     4
#endif
#ifndef ADAPTER_EIP202_PHY_CDR3_IRQ
#define ADAPTER_EIP202_PHY_CDR3_IRQ     6
#endif

#ifndef ADAPTER_EIP202_CDR0_INT_NAME
#define ADAPTER_EIP202_CDR0_INT_NAME    "EIP202-CDR0"
#endif
#ifndef ADAPTER_EIP202_CDR1_INT_NAME
#define ADAPTER_EIP202_CDR1_INT_NAME    "EIP202-CDR1"
#endif
#ifndef ADAPTER_EIP202_CDR2_INT_NAME
#define ADAPTER_EIP202_CDR2_INT_NAME    "EIP202-CDR2"
#endif
#ifndef ADAPTER_EIP202_CDR3_INT_NAME
#define ADAPTER_EIP202_CDR3_INT_NAME    "EIP202-CDR3"
#endif

#ifndef ADAPTER_EIP202_PHY_RDR0_IRQ
#define ADAPTER_EIP202_PHY_RDR0_IRQ     1
#endif
#ifndef ADAPTER_EIP202_PHY_RDR1_IRQ
#define ADAPTER_EIP202_PHY_RDR1_IRQ     3
#endif
#ifndef ADAPTER_EIP202_PHY_RDR2_IRQ
#define ADAPTER_EIP202_PHY_RDR2_IRQ     5
#endif
#ifndef ADAPTER_EIP202_PHY_RDR3_IRQ
#define ADAPTER_EIP202_PHY_RDR3_IRQ     7
#endif

#ifndef ADAPTER_EIP202_RDR0_INT_NAME
#define ADAPTER_EIP202_RDR0_INT_NAME    "EIP202-RDR0"
#endif
#ifndef ADAPTER_EIP202_RDR1_INT_NAME
#define ADAPTER_EIP202_RDR1_INT_NAME    "EIP202-RDR1"
#endif
#ifndef ADAPTER_EIP202_RDR2_INT_NAME
#define ADAPTER_EIP202_RDR2_INT_NAME    "EIP202-RDR2"
#endif
#ifndef ADAPTER_EIP202_RDR3_INT_NAME
#define ADAPTER_EIP202_RDR3_INT_NAME    "EIP202-RDR3"
#endif


#ifndef ADAPTER_PHY_EIP202_DFE0_IRQ
#define ADAPTER_PHY_EIP202_DFE0_IRQ     0
#endif

#ifndef ADAPTER_EIP202_DFE0_INT_NAME
#define ADAPTER_EIP202_DFE0_INT_NAME    "EIP202-DFE0"
#endif

#ifndef ADAPTER_PHY_EIP202_DSE0_IRQ
#define ADAPTER_PHY_EIP202_DSE0_IRQ     1
#endif

#ifndef ADAPTER_EIP202_DSE0_INT_NAME
#define ADAPTER_EIP202_DSE0_INT_NAME    "EIP202-DSE0"
#endif

#ifndef ADAPTER_PHY_EIP202_RING0_IRQ
#define ADAPTER_PHY_EIP202_RING0_IRQ    16
#endif
#ifndef ADAPTER_PHY_EIP202_RING1_IRQ
#define ADAPTER_PHY_EIP202_RING1_IRQ    17
#endif
#ifndef ADAPTER_PHY_EIP202_RING2_IRQ
#define ADAPTER_PHY_EIP202_RING2_IRQ    18
#endif
#ifndef ADAPTER_PHY_EIP202_RING3_IRQ
#define ADAPTER_PHY_EIP202_RING3_IRQ    19
#endif

#ifndef ADAPTER_EIP202_RING0_INT_NAME
#define ADAPTER_EIP202_RING0_INT_NAME   "EIP202-RING0"
#endif
#ifndef ADAPTER_EIP202_RING1_INT_NAME
#define ADAPTER_EIP202_RING1_INT_NAME   "EIP202-RING1"
#endif
#ifndef ADAPTER_EIP202_RING2_INT_NAME
#define ADAPTER_EIP202_RING2_INT_NAME   "EIP202-RING2"
#endif
#ifndef ADAPTER_EIP202_RING3_INT_NAME
#define ADAPTER_EIP202_RING3_INT_NAME   "EIP202-RING3"
#endif


#ifndef ADAPTER_PHY_EIP202_PE0_IRQ
#define ADAPTER_PHY_EIP202_PE0_IRQ      24
#endif

#ifndef ADAPTER_EIP202_PE0_INT_NAME
#define ADAPTER_EIP202_PE0_INT_NAME     "EIP202-PE0"
#endif

#ifndef ADAPTER_EIP202_MAX_PACKETS
#define ADAPTER_EIP202_MAX_PACKETS      32
#endif

#ifndef ADAPTER_EIP202_MAX_LOGICDESCR
#define ADAPTER_EIP202_MAX_LOGICDESCR   32
#endif

#ifndef ADAPTER_EIP202_BANK_SA
#define ADAPTER_EIP202_BANK_SA          0
#endif

#ifndef ADAPTER_EIP202_BANK_RING
#define ADAPTER_EIP202_BANK_RING        0
#endif

// This parameter enables the endianness conversion by the Host CPU
// for the ring descriptors
//#define ADAPTER_EIP202_ARMRING_ENABLE_SWAP

#ifndef ADAPTER_EIP202_DESCRIPTORDONECOUNT
#define ADAPTER_EIP202_DESCRIPTORDONECOUNT      1
#endif

#ifndef ADAPTER_EIP202_DESCRIPTORDONETIMEOUT
#define ADAPTER_EIP202_DESCRIPTORDONETIMEOUT    0
#endif

#ifndef ADAPTER_EIP202_DEVICE_COUNT
#error "QW"
#define ADAPTER_EIP202_DEVICE_COUNT     1
#endif

#ifndef ADAPTER_EIP202_DEVICES
#error "ADAPTER_EIP202_DEVICES not defined"
#endif

#ifndef ADAPTER_EIP202_LOGICAL_INTERRUPTS
#error "ADAPTER_EIP202_LOGICAL_INTERRUPTS not defined"
#endif

//Set this if the IRQs for the second ring (Ring 1) are used.
#if (DDK_PEC_IF_ID > 1)
#define ADAPTER_EIP202_HAVE_RING1_IRQ
#endif
// Request the IRQ through the UMDevXS driver.
//#define ADAPTER_EIP202_USE_UMDEVXS_IRQ

// Provide manually CDR and RDR configuration parameters
#ifdef ADAPTER_EIP202_RING_MANUAL_CONFIGURE
// Host interface data width:
//   0 = 32 bits, 1 = 64 bits, 2 = 128 bits, 3 = 256 bits
#ifndef ADAPTER_EIP202_HOST_DATA_WIDTH
#define ADAPTER_EIP202_HOST_DATA_WIDTH      0
#endif

// Command Descriptor FIFO size, the actual size is 2^CF_Size 32-bit words
#ifndef ADAPTER_EIP202_CF_SIZE
#define ADAPTER_EIP202_CF_SIZE              5
#endif

// Result Descriptor FIFO size, the actual size is 2^RF_Size 32-bit words
#ifndef ADAPTER_EIP202_RF_SIZE
#define ADAPTER_EIP202_RF_SIZE              5
#endif

#endif // ADAPTER_EIP202_RING_CONFIGURE

// Define if the hardware uses extended command and result descriptors.
//#define ADAPTER_EIP202_USE_EXTENDED_DESCRIPTOR


#endif /* Include Guard */


/* end of file c_adapter_eip202.h */

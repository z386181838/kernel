/* cs_adapter.h
 *
 * Configuration Settings for the SLAD Adapter Combined module.
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
* In case you do not have an account for
* this system, please send an e-mail to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

// we accept a few settings from the top-level configuration file
#include "cs_driver.h"

// Adapter extensions
#include "cs_adapter_ext.h"


/****************************************************************************
 * Adapter Global configuration parameters
 */

// log level for the entire adapter (for now)
// choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#ifndef LOG_SEVERITY_MAX
#ifdef DRIVER_PERFORMANCE
#define LOG_SEVERITY_MAX  LOG_SEVERITY_CRITICAL
#else
#define LOG_SEVERITY_MAX  LOG_SEVERITY_WARN
#endif
#endif

//Qwert
//#undef LOG_SEVERITY_MAX
//#define LOG_SEVERITY_MAX  LOG_SEVERITY_INFO


#ifdef DRIVER_NAME
#define ADAPTER_DRIVER_NAME     DRIVER_NAME
#else
#define ADAPTER_DRIVER_NAME     "SafeXcel"
#endif

#ifdef DRIVER_LICENSE
#define ADAPTER_LICENSE         DRIVER_LICENSE
#else
#define ADAPTER_LICENSE         "Proprietary"
#endif

// PCI configuration value: Cache Line Size, in 32bit words
// Advised value: 1
#define ADAPTER_PCICONFIG_CACHELINESIZE   1

// PCI configuration value: Master Latency Timer, in PCI bus clocks
// Advised value: 0xf8
#define ADAPTER_PCICONFIG_MASTERLATENCYTIMER 0xf8

// filter for printing interrupts
#ifdef DRIVER_INTERRUPTS
#define ADAPTER_INTERRUPTS_TRACEFILTER 0x00
#else
#define ADAPTER_INTERRUPTS_TRACEFILTER 0x00
#endif

// Is host platform 64-bit?
#ifdef DRIVER_64BIT_HOST
#define ADAPTER_64BIT_HOST
// Is device 64-bit? Only makes sense on 64-bit host.
#ifdef DRIVER_64BIT_DEVICE
#define ADAPTER_64BIT_DEVICE
#endif  // DRIVER_64BIT_DEVICE
#endif  // DRIVER_64BIT_HOST

// Maximum number of descriptors that can be submitted in one go.
#ifdef DRIVER_MAX_PECLOGICDESCR
#define ADAPTER_MAX_PECLOGICDESCR       DRIVER_MAX_PECLOGICDESCR
#else
#define ADAPTER_MAX_PECLOGICDESCR       10
#endif

// switch to remove support for bounce buffers
#ifndef DRIVER_BOUNCEBUFFERS
#define ADAPTER_REMOVE_BOUNCEBUFFERS
#endif

#define ADAPTER_GLOBAL_DEVICE_NAME          "EIP97_GLOBAL"

// the number of packets in the result ring before the related interrupt
// is activated. This reduces the number of interrupts and allows the
// host to get many results in one call to PEC_Packet_Get.
#ifdef DRIVER_INTERRUPT_COALESCING
#define ADAPTER_DESCRIPTORDONECOUNT  5
#else
#define ADAPTER_DESCRIPTORDONECOUNT  1
#endif

// maximum delay until activating an interrupt
// after writing a result descriptor to the result ring
// desired maximum time: T in seconds
// calculate configuration value N as follows:
//   N = T (seconds) * Engine frequency (Hz) / 1024
// example: T = 100 microseconds,
//          Engine frequency = 100 MHz
// N = (1 / 10 000) sec * 100 000 000 Hz / 1024 ~= 10
#ifdef DRIVER_INTERRUPT_COALESCING
#define ADAPTER_DESCRIPTORDONETIMEOUT  10
#else
#define ADAPTER_DESCRIPTORDONETIMEOUT  0
#endif

#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE
#define ADAPTER_DMARESOURCE_BANKS_ENABLE
#endif


/****************************************************************************
 * Adapter PEC configuration parameters
 */

// enable debug checks
#ifndef DRIVER_PERFORMANCE
#define ADAPTER_PEC_DBG
#define ADAPTER_PEC_STRICT_ARGS
#endif

// Maximum number of supported Packet Engine devices
#ifdef DRIVER_MAX_NOF_RING_TO_USE
#define ADAPTER_PEC_DEVICE_COUNT       DRIVER_MAX_NOF_RING_TO_USE
#else
#error "Q1"
#define ADAPTER_PEC_DEVICE_COUNT       1
#endif

// the number of SA records the driver can register
// if State record or ARC4 State record are used together with SA record
// then this number should be increased by factor of 2 or 3 respectively
#ifndef DRIVER_PEC_MAX_SAS
#define ADAPTER_PEC_MAX_SAS         60
#else
#define ADAPTER_PEC_MAX_SAS         DRIVER_PEC_MAX_SAS
#endif

// the number of packets the driver can buffer either in its input buffer
// a.k.a. Packet Descriptor Ring (PDR) or its output buffer
// a.k.a. Result Descriptor Ring (RDR)
#ifndef DRIVER_PEC_MAX_PACKETS
#define ADAPTER_PEC_MAX_PACKETS     10
#else
#define ADAPTER_PEC_MAX_PACKETS     DRIVER_PEC_MAX_PACKETS
#endif

// one for commands, the other for results
#ifdef DRIVER_PE_ARM_SEPARATE
#define ADAPTER_PEC_SEPARATE_RINGS
#endif

#ifdef ADAPTER_MAX_PECLOGICDESCR
#define ADAPTER_PEC_MAX_LOGICDESCR  ADAPTER_MAX_PECLOGICDESCR
#endif

#if ADAPTER_PEC_MAX_LOGICDESCR > ADAPTER_PEC_MAX_PACKETS
#error "Error: ADAPTER_PEC_MAX_LOGICDESCR > ADAPTER_PEC_MAX_PACKETS"
#endif

#ifndef DRIVER_BOUNCEBUFFERS
#define ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
#endif

#ifdef DRIVER_SCATTERGATHER

#define ADAPTER_PEC_ENABLE_SCATTERGATHER

// the maximum number of fragments per packet
#define ADAPTER_PEC_MAX_FRAGMENTS_PER_PACKET    4

// the maximum number of descriptors in the gather and scatter rings
#define ADAPTER_PEC_SC_RING_MAX_DESCRIPTORS \
        (ADAPTER_PEC_MAX_FRAGMENTS_PER_PACKET * ADAPTER_PEC_MAX_PACKETS)

#endif /* scatter/gather */

#ifdef DRIVER_INTERRUPTS
#define ADAPTER_PEC_INTERRUPTS_ENABLE
#endif

// enable for big-endian CPU
#ifdef DRIVER_SWAPENDIAN
#define ADAPTER_PEC_ARMRING_ENABLE_SWAP
#endif //DRIVER_SWAPENDIAN

// DMA resource bank for SLAD PEC Adapter, used for bouncing SA buffers
#ifdef DRIVER_PEC_BANK_SA
#define ADAPTER_PEC_BANK_SA                     DRIVER_PEC_BANK_SA
#endif


/****************************************************************************
 * Adapter EIP-202 configuration parameters
 */

// enable debug checks
#ifndef DRIVER_PERFORMANCE
#define ADAPTER_EIP202_STRICT_ARGS
#endif

#ifdef ADAPTER_GLOBAL_DEVICE_NAME
#define ADAPTER_EIP202_GLOBAL_DEVICE_NAME    ADAPTER_GLOBAL_DEVICE_NAME
#endif

#ifdef DRIVER_64BIT_DEVICE
#define ADAPTER_EIP202_64BIT_DEVICE
#endif

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
#define ADAPTER_EIP202_ENABLE_SCATTERGATHER
#endif

#ifdef ADAPTER_PEC_SEPARATE_RINGS
#define ADAPTER_EIP202_SEPARATE_RINGS
#endif

#ifdef ADAPTER_PEC_MAX_PACKETS
#define ADAPTER_EIP202_MAX_PACKETS      ADAPTER_PEC_MAX_PACKETS
#endif

#ifndef ADAPTER_DESCRIPTORDONECOUNT
#define ADAPTER_EIP202_DESCRIPTORDONECOUNT  ADAPTER_DESCRIPTORDONECOUNT
#endif

#ifndef ADAPTER_DESCRIPTORDONETIMEOUT
#define ADAPTER_EIP202_DESCRIPTORDONETIMEOUT  ADAPTER_DESCRIPTORDONETIMEOUT
#endif

#ifdef ADAPTER_PEC_MAX_LOGICDESCR
#define ADAPTER_EIP202_MAX_LOGICDESCR   ADAPTER_PEC_MAX_LOGICDESCR
#endif

// switch to remove support for bounce buffers
#ifndef DRIVER_BOUNCEBUFFERS
#define ADAPTER_EIP202_REMOVE_BOUNCEBUFFERS
#endif

// descriptor spacing in words, allowing cache line alignment
// ring memory start alignment will use same value
#define ADAPTER_SYSTEM_DCACHE_LINE_SIZE_BYTES  64	//Qwert 32
#define ADAPTER_EIP97_DESCRIPTORSPACING_WORDS \
                (ADAPTER_SYSTEM_DCACHE_LINE_SIZE_BYTES / 4)

// Threshold for DMA input (read) and output (write)
// The transfer is initiated when the number of positions in
// the engine buffer are used (output) or free (input).
// It also affects the maximum length of the burst.
#define ADAPTER_EIP97_DMATHRESHOLD_INPUT   0x08
#define ADAPTER_EIP97_DMATHRESHOLD_OUTPUT  0x08

#ifdef DRIVER_ENABLE_SWAP_MASTER
// This parameter enables the byte swap in 32-bit words
// for the EIP-202 CD Manager master interface
#define ADAPTER_EIP202_CDR_BYTE_SWAP_ENABLE

// This parameter enables the byte swap in 32-bit words
// for the EIP-202 RD Manager master interface
#define ADAPTER_EIP202_RDR_BYTE_SWAP_ENABLE
#endif // DRIVER_ENABLE_SWAP_MASTER

#ifdef DRIVER_SWAPENDIAN
// This parameter enables the endianness conversion by the Host CPU
// for the ring descriptors
#define ADAPTER_EIP202_ARMRING_ENABLE_SWAP
#endif //DRIVER_SWAPENDIAN

#ifdef DRIVER_INTERRUPTS
#define ADAPTER_EIP202_INTERRUPTS_ENABLE
#endif

#ifdef ADAPTER_INTERRUPTS_TRACEFILTER
#define ADAPTER_EIP202_INTERRUPTS_TRACEFILTER   ADAPTER_INTERRUPTS_TRACEFILTER
#endif

#define ADAPTER_EIP202_PHY_CDR0_IRQ     0
#define ADAPTER_EIP202_CDR0_INT_NAME    "EIP202-CDR0"

#define ADAPTER_EIP202_PHY_RDR0_IRQ     1
#define ADAPTER_EIP202_RDR0_INT_NAME    "EIP202-RDR0"

#define ADAPTER_EIP202_PHY_DFE0_IRQ     0
#define ADAPTER_EIP202_DFE0_INT_NAME    "EIP202-DFE0"

#define ADAPTER_EIP202_PHY_DSE0_IRQ     1
#define ADAPTER_EIP202_DSE0_INT_NAME    "EIP202-DSE0"

#define ADAPTER_EIP202_PHY_RING0_IRQ    16
#define ADAPTER_EIP202_RING0_INT_NAME   "EIP202-RING0"

#define ADAPTER_EIP202_PHY_PE0_IRQ      24
#define ADAPTER_EIP202_PE0_INT_NAME     "EIP202-PE0"

#ifdef ADAPTER_PEC_BANK_RING
#define ADAPTER_EIP202_BANK_RING        ADAPTER_PEC_BANK_RING
#endif

// Maximum number of supported EIP-202 devices
// (one device includes CDR Manager and RDR Manager)
#ifdef DRIVER_MAX_NOF_RING_TO_USE
#define ADAPTER_EIP202_DEVICE_COUNT     DRIVER_MAX_NOF_RING_TO_USE
#else
#define ADAPTER_EIP202_DEVICE_COUNT     1
#endif

#define ADAPTER_EIP202_LOGICAL_INTERRUPTS                           \
        IRQ_DFE0 = 0,                                               \
        IRQ_DSE0 = 1,                                               \
        IRQ_PE0  = 2,                                               \
                                                                    \
        IRQ_CDR0 = 3,                                               \
        IRQ_RDR0 = 4,                                               \
		IRQ_CDR1 = 5,                                               \
		IRQ_RDR1 = 6,                                               \
		IRQ_CDR2 = 7,                                               \
		IRQ_RDR2 = 8,                                               \
		IRQ_CDR3 = 9,                                               \
		IRQ_RDR3 = 10,                                              \
																	\
        IRQ_LAST_LINE
#if (DDK_PEC_IF_ID>1)
#define ADAPTER_EIP202_DEVICES                                      \
		{ 3,                /* CDR IRQ */                            \
         0,                /* CDR IRQ flags */                      \
         "EIP202_CDR0",    /* CDR Device Name */                    \
         4,                /* RDR IRQ */                            \
         0,                /* RDR IRQ flags */                      \
		"EIP202_RDR0"     /* RDR Dev Name */                       \
		},\
		{\
		5,                /* CDR IRQ */                            \
		0,                /* CDR IRQ flags */                      \
		"EIP202_CDR1",     /* CDR Dev Name */                       \
		6,                /* RDR IRQ */                            \
		0,                /* RDR IRQ flags */                      \
		"EIP202_RDR1"     /* RDR Dev Name */ 						\
		},\
		{\
		7,                /* CDR IRQ */                            \
		0,                /* CDR IRQ flags */                      \
		"EIP202_CDR2",    /* CDR Device Name */                    \
		8,                /* RDR IRQ */                            \
		0,                /* RDR IRQ flags */                      \
		"EIP202_RDR2"     /* RDR Dev Name */                       \
		},\
		{\
		9,                /* CDR IRQ */                            \
		0,                /* CDR IRQ flags */                      \
		"EIP202_CDR3",    /* CDR Device Name */                    \
		10,                /* RDR IRQ */                            \
		0,                /* RDR IRQ flags */                      \
		"EIP202_RDR3"     /* RDR Dev Name */                       \
		}
#else
#define ADAPTER_EIP202_DEVICES                                      \
    {                                                               \
         3,                /* CDR IRQ */                            \
         0,                /* CDR IRQ flags */                      \
         "EIP202_CDR0",    /* CDR Device Name */                    \
         4,                /* RDR IRQ */                            \
         0,                /* RDR IRQ flags */                      \
         "EIP202_RDR0"     /* RDR Dev Name */                       \
    }
#endif
#if ADAPTER_PEC_DEVICE_COUNT != ADAPTER_EIP202_DEVICE_COUNT
#error "Adapter PEC device configuration is invalid"
#endif

#ifdef ADAPTER_PEC_BANK_SA
#define ADAPTER_EIP202_BANK_SA          ADAPTER_PEC_BANK_SA
#endif

#ifdef DRIVER_USE_EXTENDED_DESCRIPTOR
// Define if the hardware uses extended command and result descriptors.
#define ADAPTER_EIP202_USE_EXTENDED_DESCRIPTOR
#endif


/****************************************************************************
 * Adapter DMAResource handles maximum calculation
 */

// the number of buffers that can be tracked by the driver-internal
// administration: ADAPTER_MAX_DMARESOURCE_HANDLES
#ifndef ADAPTER_REMOVE_BOUNCEBUFFERS
#define ADAPTER_BOUNCE_FACTOR   2
#else
#define ADAPTER_BOUNCE_FACTOR   1
#endif

// internal DMA-safe buffers are used for rings, they are never bounced
#ifdef ADAPTER_PEC_SEPARATE_RINGS
#define ADAPTER_PEC_MAX_RINGS_DMARESOURCE_HANDLES   2
#else
#define ADAPTER_PEC_MAX_RINGS_DMARESOURCE_HANDLES   1
#endif

// SC buffers must be allocated DMA-safe by applications,
// they are never bounced
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
// 2 separate rings are used, one for gather and one for scatter
// 1 DMA resource is used for SG list and 1 is extra
#define ADAPTER_PEC_MAX_SC_DMARESOURCE_HANDLES   \
    (2+1+ADAPTER_PEC_SC_RING_MAX_DESCRIPTORS+1)
#else
#define ADAPTER_PEC_MAX_SC_DMARESOURCE_HANDLES   0
#endif


#define ADAPTER_MAX_DMARESOURCE_HANDLES \
    (ADAPTER_BOUNCE_FACTOR * \
     (ADAPTER_PEC_MAX_SAS + \
     ADAPTER_PEC_DEVICE_COUNT * ADAPTER_PEC_MAX_PACKETS) + \
     ADAPTER_PEC_MAX_RINGS_DMARESOURCE_HANDLES +         \
     ADAPTER_PEC_MAX_SC_DMARESOURCE_HANDLES)


/* end of file cs_adapter.h */

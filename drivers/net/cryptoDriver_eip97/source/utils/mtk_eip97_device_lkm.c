/* device_lkm.c
 *
 * This is is the Linux Kernel-mode Driver Framework v4 Device API
 * implementation for PCI. The implementation is device-agnostic and
 * receives configuration details from the c_device_lkm_pci.h file.
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
 * This module implements (provides) the following interface(s):
 */

#include "device_mgmt.h"            // API to implement
#include "device_rw.h"              // API to implement

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_device_lkm.h"

// Driver Framework Device API
#include "device_swap.h"            // Device_SwapEndian32

// Logging API
//#undef LOG_SEVERITY_MAX
//#define LOG_SEVERITY_MAX  HWPAL_LOG_SEVERITY
#include "log.h"                    // LOG_*

// Driver Framework C Run-Time Library API
#include "clib.h"                   // memcmp

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint32_t, NULL, inline, bool,
                                    // IDENTIFIER_NOT_USED
#ifdef HWPAL_USE_UMDEVXS_PCI_DEVICE
#include "umdevxs_pcidev.h"
#endif

// Linux Kernel API
#include <asm/io.h>                 // ioread32, iowrite32
#include <asm/system.h>             // smp_rmb, smp_wmb
#include <linux/version.h>          // LINUX_VERSION_CODE, KERNEL_VERSION
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>

#ifdef MTK_EIP97_DRIVER
#include <asm/rt2880/rt_mmap.h>
#endif

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define HWPAL_FLAG_READ     BIT_0   // 1
#define HWPAL_FLAG_WRITE    BIT_1   // 2
#define HWPAL_FLAG_SWAP     BIT_2   // 4
#define HWPAL_FLAG_HA       BIT_5   // 32

// Device administration structure
typedef struct
{
#ifdef HWPAL_DEVICE_MAGIC
    // Magic value for detecting valid handles
    unsigned int ValidHandle;
#endif

    // Name string used in Device_Find
    const char * DeviceName_p;

    // device offset range inside PCI device
    unsigned int StartByteOffset;
    unsigned int LastByteOffset;

    // Trace Read, Write flags,
    // Enable byte swap by the host processor flag,
    char Flags;
} HWPAL_Device_Administration_t;

// the c_device_lkm.h file defines a HWPAL_DEVICES that
// depends on the following HWPAL_DEVICE_ADD
#ifdef HWPAL_DEVICE_MAGIC
#define HWPAL_DEVICE_ADD(_name, _devrn, _start, _last, _flags) \
        { HWPAL_DEVICE_MAGIC, _name, _start, _last, _flags }
#else
#define HWPAL_DEVICE_ADD(_name, _devrn, _start, _last, _flags) \
        { _name, _start, _last, _flags }
#endif

// the c_device_lkm.h file defines a HWPAL_REMAP_ADDRESSES that
// depends on the following HWPAL_REMAP_ONE
#define HWPAL_REMAP_ONE(_old, _new) \
    case _old: \
        DeviceByteOffset = _new; \
        break;

// number of devices supported calculated on HWPAL_DEVICES defined
// in c_device_lkm.h
#define DEVICE_COUNT \
        (sizeof(HWPAL_Devices) \
         / sizeof(HWPAL_Device_Administration_t))

// checks that byte offset is in range
#define IS_INVALID_OFFSET(_ofs, _devp) \
    (((_devp)->StartByteOffset + (_ofs) > (_devp)->LastByteOffset) || \
     (((_ofs) & 3) != 0))

#ifdef HWPAL_DEVICE_MAGIC
// checks that device handle is valid
#define IS_INVALID_DEVICE(_devp) \
    ((_devp) < HWPAL_Devices || \
     (_devp) >= HWPAL_Devices + DEVICE_COUNT || \
     (_devp)->ValidHandle != HWPAL_DEVICE_MAGIC)
#endif /* HWPAL_DEVICE_MAGIC */

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif /* KERNEL_VERSION */

/* 3.1.10 is the highest version currently supported */
//Qwert #if LINUX_VERSION_CODE > KERNEL_VERSION(3,1,10)
//#error "Kernel versions after 3.1.10 are not supported"
//#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(3,1,10) */

//#define MTK_EIP97_ICEPRN	1
/*----------------------------------------------------------------------------
 * Forward declarations
 */

#ifndef HWPAL_USE_UMDEVXS_PCI_DEVICE
static int
HWPAL_Probe(
        struct pci_dev * PCI_Device_p,
        const struct pci_device_id * id);

static void
HWPAL_Remove(
        struct pci_dev * PCI_Device_p);
#endif

/*----------------------------------------------------------------------------
 * Local variables
 */

static const HWPAL_Device_Administration_t HWPAL_Devices[] =
{
    HWPAL_DEVICES
};

// declarations native to Linux kernel
static struct pci_dev * HWPAL_PCI_Device_p = NULL;

// virtual address returned by ioremap()
#ifdef MTK_EIP97_DRIVER
static uint32_t * HWPAL_MappedBaseAddr_p = ETHDMASYS_CRYPTO_ENGINE_BASE;
#else
static uint32_t * HWPAL_MappedBaseAddr_p = NULL;
#endif
#ifndef HWPAL_USE_UMDEVXS_PCI_DEVICE
const struct pci_device_id DeviceIDs[] =
{
    {PCI_DEVICE(HWPAL_VENDOR_ID, HWPAL_DEVICE_ID), },
    {0, }
};

static struct pci_driver HWPAL_PCI_Driver =
{
    .name = HWPAL_DRIVER_NAME,
    .id_table = DeviceIDs,
    .probe = HWPAL_Probe,
    .remove = HWPAL_Remove,
};
#endif

/*----------------------------------------------------------------------------
 * HWPAL_Hexdump
 *
 * This function hex-dumps an array of uint32_t.
 */
#if ((defined(HWPAL_TRACE_DEVICE_READ)) || (defined(HWPAL_TRACE_DEVICE_WRITE)))
static void
HWPAL_Hexdump(
        const char * ArrayName_p,
        const char * DeviceName_p,
        const unsigned int ByteOffset,
        const uint32_t * WordArray_p,
        const unsigned int WordCount,
        bool fSwapEndianness)
{
    unsigned int i;

    Log_FormattedMessage(
        "%s: "
        "byte offsets 0x%x - 0x%x"
        " (%s)\n"
        "  ",
        ArrayName_p,
        ByteOffset,
        ByteOffset + WordCount*4 -1,
        DeviceName_p);

    for (i = 1; i <= WordCount; i++)
    {
        uint32_t Value = WordArray_p[i - 1];

        if (fSwapEndianness)
            Value = Device_SwapEndian32(Value);

        Log_FormattedMessage(" 0x%08x", Value);

        if ((i & 7) == 0)
            Log_Message("\n  ");
    }

    if ((WordCount & 7) != 0)
        Log_Message("\n");
}
#endif


#ifndef HWPAL_USE_UMDEVXS_PCI_DEVICE
/*----------------------------------------------------------------------------
 * HWPAL_Probe
 */
static int
HWPAL_Probe(
        struct pci_dev * PCI_Device_p,
        const struct pci_device_id * id)
{
    int i;
    const int BAR_ID = 0;
    resource_size_t BaseAddrHwRd;

    {
        // enable the device
        // this also looks up the IRQ
        int res = pci_enable_device(PCI_Device_p);

        if (res)
        {
            LOG_CRIT(
                "HWPAL_Probe: "
                "Failed to enable PCI device %s\n",
                pci_name(PCI_Device_p));

            return res;
        }
    }

    // remember the device reference
    // we need when access the configuration space
    HWPAL_PCI_Device_p = PCI_Device_p;

    if (NULL == HWPAL_PCI_Device_p)
    {
         return 1;
    }

    // now map the chip into kernel memory
    // so we can access the EIP static resources
    BaseAddrHwRd = pci_resource_start(PCI_Device_p, BAR_ID);
    BaseAddrHwRd &= ~0xf; // Chop off the control bits

    // note: ioremap is uncached by default
    HWPAL_MappedBaseAddr_p = ioremap(
                                 BaseAddrHwRd,
                                 pci_resource_len(PCI_Device_p, BAR_ID));

    if (!HWPAL_MappedBaseAddr_p)
    {
        LOG_CRIT(
            "HWPAL_Probe: "
            "Failed to ioremap PCI device %s\n",
            pci_name(PCI_Device_p));

        return 1;
    }

    LOG_INFO(
        "HWPAL_Probe: "
        "Mapped base address is: %p, sizeof(resource_size_t)=%d\n"
        "  start=0x%x, end=0x%x, len=0x%x, flags=0x%x, irq=%d\n",
        HWPAL_MappedBaseAddr_p,
        (int)sizeof(resource_size_t),
        (unsigned int)pci_resource_start(PCI_Device_p, 0),
        (unsigned int)pci_resource_end(PCI_Device_p, 0),
        (unsigned int)pci_resource_len(PCI_Device_p, 0),
        (unsigned int)pci_resource_flags(PCI_Device_p, 0),
        PCI_Device_p->irq);

    for(i=0; i < DEVICE_COUNT; i++)
    {
            LOG_INFO("HWPAL_Probe: mapped device '%s', "
                     "virt base addr 0x%p, "
                     "start byte offset 0x%x, "
                     "last byte offset 0x%x\n",
                     HWPAL_Devices[i].DeviceName_p,
                     HWPAL_MappedBaseAddr_p,
                     HWPAL_Devices[i].StartByteOffset,
                     HWPAL_Devices[i].LastByteOffset);
    }

    IDENTIFIER_NOT_USED(id);

    // return 0 to indicate "we decided to take ownership"
    return 0;
}


/*----------------------------------------------------------------------------
 * HWPAL_Remove
 */
static void
HWPAL_Remove(
        struct pci_dev * PCI_Device_p)
{
    LOG_INFO(
        "HWPAL_Remove: "
        "HWPAL_MappedBaseAddr_p=%p\n",
        HWPAL_MappedBaseAddr_p);

    if (HWPAL_MappedBaseAddr_p)
    {
        iounmap(HWPAL_MappedBaseAddr_p);
        HWPAL_MappedBaseAddr_p = NULL;
    }

    pci_disable_device(PCI_Device_p);
}
#endif

/*----------------------------------------------------------------------------
 * Device_RemapDeviceAddress
 *
 * This function remaps certain device addresses (relative within the whole
 * device address map) to other addresses. This is needed when the integration
 * has remapped some EIP device registers to other addresses. The EIP Driver
 * Libraries assume the devices always have the same internal layout.
 */
static inline unsigned int
Device_RemapDeviceAddress(
        unsigned int DeviceByteOffset)
{
#ifdef HWPAL_REMAP_ADDRESSES
    switch(DeviceByteOffset)
    {
        // include the remap statements
        HWPAL_REMAP_ADDRESSES

        default:
            break;
    }
#endif

    return DeviceByteOffset;
}


/*------------------------------------------------------------------------------
 * device_mgmt API
 *
 * These functions support finding a device given its name.
 * A handle is returned that is needed in the device_rw API
 * to read or write the device
 */


/*------------------------------------------------------------------------------
 * Device_Initialize
 */
int
Device_Initialize(
        void * CustomInitData_p)
{
#ifndef MTK_EIP97_DRIVER
#ifndef HWPAL_USE_UMDEVXS_PCI_DEVICE
    int Status;

    Status = pci_register_driver(&HWPAL_PCI_Driver);
    if (Status < 0)
    {
        LOG_CRIT(
            "Device_Initialize: "
            "Failed to register the PCI device\n");

        return false;
    }

    if (NULL == HWPAL_PCI_Device_p)
    {
        LOG_CRIT(
        "Device_Initialize: "
        "Failed, no device detected\n");

        Device_UnInitialize();
        return false;
    }

    // if provided, CustomInitData_p points to an "int"
    // we return the "irq" number via this output parameter
    if (CustomInitData_p)
    {
        int * p = (int *)CustomInitData_p;
#ifdef HWPAL_USE_MSI
        pci_set_master(HWPAL_PCI_Device_p);
        pci_enable_msi(HWPAL_PCI_Device_p);
#endif
        *p = HWPAL_PCI_Device_p->irq;
    }
#else
    UMDevXS_PCIDev_Get(0, &HWPAL_PCI_Device_p, (void **)&HWPAL_MappedBaseAddr_p);
    if (HWPAL_PCI_Device_p == NULL || HWPAL_MappedBaseAddr_p == NULL)
    {
        LOG_CRIT(
        "Device_Initialize: "
        "Failed, no device detected\n");
        return false;
    }
    {
        int * p = (int *)CustomInitData_p;
        *p = HWPAL_PCI_Device_p->irq;
    }
#endif
#error "NO MTK_EIP97_DRIVER"
#endif /* MTK_EIP97_DRIVER */
    return true;
}


/*------------------------------------------------------------------------------
 * Device_UnInitialize
 */
void
Device_UnInitialize(void)
{
    LOG_INFO(
        "Device_Uninitialize: "
        "calling pci_unregister_driver\n");
#ifndef HWPAL_USE_UMDEVXS_PCI_DEVICE

#ifndef MTK_EIP97_DRIVER	
#ifdef HWPAL_USE_MSI
    if (HWPAL_PCI_Device_p)
    {
        pci_disable_msi(HWPAL_PCI_Device_p);
        pci_set_master(HWPAL_PCI_Device_p);
    }
#endif

    pci_unregister_driver(&HWPAL_PCI_Driver);
#else
	HWPAL_PCI_Device_p = NULL;
#endif	
#else
    HWPAL_PCI_Device_p = NULL;
//		HWPAL_MappedBaseAddr_p = NULL;
#endif
}


/*-----------------------------------------------------------------------------
 * Device_Find
 */
Device_Handle_t
Device_Find(
        const char * DeviceName_p)
{
    int i;
    unsigned int NameLen;

    if (DeviceName_p == NULL)
    {
        // not supported, thus not found
        return NULL;
    }

    // count the device name length, including the terminating zero
    NameLen = 0;
    while (DeviceName_p[NameLen++])
    {
        if (NameLen == HWPAL_MAX_DEVICE_NAME_LENGTH)
        {
            break;
        }
    }

    // walk through the defined devices and compare the name
    for (i = 0; i < DEVICE_COUNT; i++)
    {
        if (memcmp(
                DeviceName_p,
                HWPAL_Devices[i].DeviceName_p,
                NameLen) == 0)
        {
            // Return the device handle
            return (Device_Handle_t)(HWPAL_Devices + i);
        }
    }

    LOG_WARN("Device_Find: Could not find device '%s'", DeviceName_p);

    return NULL;
}


/*------------------------------------------------------------------------------
 * Device_GetReference
 */
Device_Reference_t
Device_GetReference(
        const Device_Handle_t Device)
{
    Device_Reference_t DevReference;

    // There exists only one reference for this implementation
    IDENTIFIER_NOT_USED(Device);

    // Return the PCI device reference
    // (pointer to the Linux device structure)
    DevReference = &HWPAL_PCI_Device_p->dev;

    return DevReference;
}


/*------------------------------------------------------------------------------
 * device_rw API
 *
 * These functions can be used to transfer a single 32bit word or an array of
 * 32bit words to or from a device.
 * Endianess swapping is performed on the fly based on the configuration for
 * this device.
 *
 */

/*------------------------------------------------------------------------------
 * Device_Read32
 */
uint32_t
Device_Read32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset)
{
    HWPAL_Device_Administration_t * Device_p;
    uint32_t Value = 0;

    Device_p = (HWPAL_Device_Administration_t *)Device;
    if (Device_p == NULL)
        return 0xEEEEEEEE;

#ifdef HWPAL_DEVICE_MAGIC
    if (IS_INVALID_DEVICE(Device_p))
    {
        LOG_WARN(
                "Device_Read32: "
                "Invalid device handle provided.\n");

        return 0xEEEEEEEE;
    }
#endif /* HWPAL_DEVICE_MAGIC */

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (IS_INVALID_OFFSET(ByteOffset, Device_p))
    {
        LOG_WARN(
                "Device_Read32: "
                "Invalid ByteOffset 0x%x (device %s)\n",
                ByteOffset,
                Device_p->DeviceName_p);

        return 0xEEEEEEEE;
    }
#endif /* HWPAL_STRICT_ARGS_CHECK */

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        if (ByteOffset == 0x3FC8)
        {
            Value = 0;
            goto HA_SKIP;
        }
    }
#endif

    if (Device_p->StartByteOffset == HWPAL_MAGIC_PCICONFIGSPACE)
    {
        pci_read_config_dword(HWPAL_PCI_Device_p, ByteOffset, &Value);
    }
    else
    {
        unsigned int DeviceByteOffset = Device_p->StartByteOffset + ByteOffset;

        DeviceByteOffset = Device_RemapDeviceAddress(DeviceByteOffset);

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
        Value = *(uint32_t *)(HWPAL_MappedBaseAddr_p +
                (DeviceByteOffset / 4));
#else
        Value = ioread32(HWPAL_MappedBaseAddr_p + (DeviceByteOffset / 4));
#ifdef MTK_EIP97_SIMPRN        
				printk("ReadWord(0x%X, 0x%08X, 0x%08X);\n", (unsigned int)(DeviceByteOffset), \
		                     Value, 0xFFFFFFFF);
#endif
#ifdef MTK_EIP97_ICEPRN		   	
		   printk("MReadI32 0x%08X 1; =0x%08X\n", (~(0x7<<29))&(unsigned int)(HWPAL_MappedBaseAddr_p+(DeviceByteOffset>>2)+0), \
				Value);
#endif
#endif

#ifdef HWPAL_DEVICE_ENABLE_SWAP
#error "QWERTSW"
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            Value = Device_SwapEndian32(Value);
#endif

        smp_rmb();
    }

#ifdef HWPAL_ENABLE_HA_SIMULATION
HA_SKIP:
#endif

#ifdef HWPAL_TRACE_DEVICE_READ
    if (Device_p->Flags & HWPAL_FLAG_READ)
    {
        unsigned int DeviceByteOffset = Device_p->StartByteOffset + ByteOffset;
        unsigned int DeviceByteOffset2 =
                Device_RemapDeviceAddress(DeviceByteOffset);
        if (DeviceByteOffset2 != DeviceByteOffset)
        {
            DeviceByteOffset2 -= Device_p->StartByteOffset;
            Log_FormattedMessage(
                    "Device_Read32: "
                    "0x%x(was 0x%x) = 0x%08x (%s)\n",
                    DeviceByteOffset2,
                    ByteOffset,
                    (unsigned int)Value,
                    Device_p->DeviceName_p);
        }
        else
        {
            Log_FormattedMessage(
                    "Device_Read32: "
                    "0x%x = 0x%08x (%s)\n",
                    ByteOffset,
                    (unsigned int)Value,
                    Device_p->DeviceName_p);
        }
    }
#endif /* HWPAL_TRACE_DEVICE_READ */

    return Value;
}


/*------------------------------------------------------------------------------
 * Device_Write32
 */
void
Device_Write32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset,
        const uint32_t ValueIn)
{
    HWPAL_Device_Administration_t * Device_p;
    uint32_t Value = ValueIn;

    Device_p = (HWPAL_Device_Administration_t *)Device;
    if (Device_p == NULL)
        return;

#ifdef HWPAL_DEVICE_MAGIC
    if (IS_INVALID_DEVICE(Device_p))
    {
        LOG_WARN(
                "Device_Write32 :"
                "Invalid device handle provided.\n");

        return;
    }
#endif /* HWPAL_DEVICE_MAGIC */

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (IS_INVALID_OFFSET(ByteOffset, Device_p))
    {
        LOG_WARN(
                "Device_Write32: "
                "Invalid ByteOffset 0x%x (device %s)\n",
                ByteOffset,
                Device_p->DeviceName_p);
        return;
    }
#endif /* HWPAL_STRICT_ARGS_CHECK */

#ifdef HWPAL_TRACE_DEVICE_WRITE
    if (Device_p->Flags & HWPAL_FLAG_WRITE)
    {
        Log_FormattedMessage(
                "Device_Write32: "
                "0x%x = 0x%08x (%s)\n",
                ByteOffset,
                (unsigned int)Value,
                Device_p->DeviceName_p);
    }
#endif /* HWPAL_TRACE_DEVICE_WRITE*/

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        if (ByteOffset == 0x3FC8)
        {
            LOG_CRIT(
                "Device_Write32: "
                "Unexpected write to PKA_MASTER_SEQ_CTRL\n");
            return;
        }
    }
#endif

    if (Device_p->StartByteOffset == HWPAL_MAGIC_PCICONFIGSPACE)
    {
        pci_write_config_dword(HWPAL_PCI_Device_p, ByteOffset, Value);
    }
    else
    {
        uint32_t DeviceByteOffset = Device_p->StartByteOffset + ByteOffset;

        DeviceByteOffset = Device_RemapDeviceAddress(DeviceByteOffset);

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            Value = Device_SwapEndian32(Value);
#endif

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
        *(uint32_t *)(HWPAL_MappedBaseAddr_p + (DeviceByteOffset / 4)) =
                    Value;
#else
        iowrite32(Value, HWPAL_MappedBaseAddr_p + (DeviceByteOffset / 4));
#ifdef MTK_EIP97_SIMPRN        
				printk("WriteWord(0x%X, 0x%08X, 0x%08X);\n", (unsigned int)(DeviceByteOffset), \
		                     Value, 0xFFFFFFFF);
#endif
#ifdef MTK_EIP97_ICEPRN		                     
		    printk("D.S SD:0x%08X %%LE %%LONG 0x%08X;\n", (~(0x7<<29))&(unsigned int)(HWPAL_MappedBaseAddr_p+(DeviceByteOffset>>2)+0), \
		                 Value);
#endif
#endif

        smp_wmb();
    }
}


/*------------------------------------------------------------------------------
 * Device_Read32Array
 *
 * Not supported for PCI Configuration space!
 */
void
Device_Read32Array(
        const Device_Handle_t Device,
        const unsigned int Offset,      // read starts here, +4 increments
        uint32_t * MemoryDst_p,         // writing starts here
        const int Count)                // number of uint32's to transfer
{
    HWPAL_Device_Administration_t * Device_p;
    unsigned int DeviceByteOffset;

    Device_p = (HWPAL_Device_Administration_t *)Device;

    if (Device_p == NULL ||
        MemoryDst_p == NULL ||
        Count <= 0)
    {
        return;
    }

    if (IS_INVALID_OFFSET(Offset, Device_p))
    {
        LOG_WARN("Device_Read32Array: "
               "Invalid ByteOffset 0x%x (device %s)\n",
               Offset,
               Device_p->DeviceName_p);
        return;
    }

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        return;
    }
#endif

    DeviceByteOffset = Device_p->StartByteOffset + Offset;

    {
        unsigned int RemappedOffset;
        uint32_t Value;
        int i;

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        bool fSwap = false;
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif
        for (i = 0; i < Count; i++)
        {
            RemappedOffset = Device_RemapDeviceAddress(DeviceByteOffset);

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
            Value = *(uint32_t*)(HWPAL_MappedBaseAddr_p + (RemappedOffset / 4));
#else
            Value = ioread32(HWPAL_MappedBaseAddr_p + (RemappedOffset / 4));
#endif

            smp_rmb();
#ifdef MTK_EIP97_SIMPRN            
						printk("ReadWord(0x%X, 0x%08X, 0x%08X);\n", (unsigned int)(RemappedOffset), \
		                     Value, 0xFFFFFFFF);
#endif
#ifdef MTK_EIP97_ICEPRN 	                     
				printk("MReadI32 0x%08X 1; =%08X\n", (~(0x7<<29))&(unsigned int)(HWPAL_MappedBaseAddr_p+(DeviceByteOffset>>2)+i), \
		                     Value);
#endif		                     
#ifdef HWPAL_DEVICE_ENABLE_SWAP
            // swap endianness if required
            if (fSwap)
                Value = Device_SwapEndian32(Value);
#endif

            MemoryDst_p[i] = Value;
            DeviceByteOffset +=  4;
        } // for
    }

#ifdef HWPAL_TRACE_DEVICE_READ
    if (Device_p->Flags & HWPAL_FLAG_READ)
    {
        HWPAL_Hexdump(
            "Device_Read32Array",
            Device_p->DeviceName_p,
            Device_p->StartByteOffset + Offset,
            MemoryDst_p,
            Count,
            false);     // already swapped during read above
    }
#endif /* HWPAL_TRACE_DEVICE_READ */
}


/*----------------------------------------------------------------------------
 * Device_Write32Array
 *
 * Not supported for PCI Configuration space!
 */
void
Device_Write32Array(
        const Device_Handle_t Device,
        const unsigned int Offset,      // write starts here, +4 increments
        const uint32_t * MemorySrc_p,   // reading starts here
        const int Count)                // number of uint32's to transfer
{
    HWPAL_Device_Administration_t * Device_p;
    unsigned int DeviceByteOffset;

    Device_p = (HWPAL_Device_Administration_t *)Device;

    if (Device_p == NULL ||
        MemorySrc_p == NULL ||
        Count <= 0)
    {
        return;     // ## RETURN ##
    }

    if (IS_INVALID_OFFSET(Offset, Device_p))
    {
        LOG_WARN(
            "Device_Write32Array: "
            "Invalid ByteOffset 0x%x (device %s)\n",
            Offset,
            Device_p->DeviceName_p);
        return;
    }

    DeviceByteOffset = Device_p->StartByteOffset + Offset;

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        return;
    }
#endif

#ifdef HWPAL_TRACE_DEVICE_WRITE
    if (Device_p->Flags & HWPAL_FLAG_WRITE)
    {
        bool fSwap = false;
#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif

        HWPAL_Hexdump(
            "Device_Write32Array",
            Device_p->DeviceName_p,
            DeviceByteOffset,
            MemorySrc_p,
            Count,
            fSwap);
    }
#endif /* HWPAL_TRACE_DEVICE_WRITE */

    {
        unsigned int RemappedOffset;
        uint32_t Value;
        int i;

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        bool fSwap = false;
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif

        for (i = 0; i < Count; i++)
        {
            RemappedOffset = Device_RemapDeviceAddress(DeviceByteOffset);
            Value = MemorySrc_p[i];
#ifdef HWPAL_DEVICE_ENABLE_SWAP
            if (fSwap)
                Value = Device_SwapEndian32(Value);
#endif

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
            *(uint32_t*)(HWPAL_MappedBaseAddr_p + (RemappedOffset / 4)) =
                                                       Value;
#else
            iowrite32(Value, HWPAL_MappedBaseAddr_p + (RemappedOffset / 4));
#endif
            smp_wmb();
#ifdef MTK_EIP97_SIMPRN
						printk("WriteWord(0x%X, 0x%08X, 0x%08X);\n", (unsigned int)(RemappedOffset), \
		                     Value, 0xFFFFFFFF);
#endif		                     
#ifdef MTK_EIP97_ICEPRN						
						printk("D.S SD:0x%08X %%LE %%LONG 0x%08X;\n", (~(0x7<<29))&(unsigned int)(HWPAL_MappedBaseAddr_p+(DeviceByteOffset>>2)+i), \
		                     Value);
#endif		                     
            DeviceByteOffset += 4;
        } // for
    }
}

#ifndef HWPAL_USE_UMDEVXS_PCI_DEVICE
MODULE_DEVICE_TABLE(pci, DeviceIDs);
#endif

/* end of file device_lkm.c */

/* adapter_init.c
 *
 * Adapter module responsible for adapter initialization tasks.
 *
 * This version is for the SafeXcel-IP-97 on an FPGA board.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "adapter_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "cs_adapter.h"

#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
#include "adapter_interrupts.h" // Adapter_Interrupts_Init,
                                // Adapter_Interrupts_UnInit
#endif
#ifndef MTK_EIP97_DRIVER
#include "adapter_pciconfig.h"  // PCICONFIG_*
#endif
// Logging API
#include "log.h"            // LOG_*

// Driver Framework Device API
#include "device_mgmt.h"    // Device_Initialize, Device_UnInitialize
#include "device_rw.h"      // Device_Read32, Device_Write32

// Driver Framework DMAResource API
#include "dmares_mgmt.h"    // DMAResource_Init, DMAResource_UnInit

// Driver Framework Basic Definitions API
#include "basic_defs.h"     // bool, true, false


#ifdef MTK_EIP97_DRIVER
#include <asm/rt2880/surfboardint.h>
#endif
/*----------------------------------------------------------------------------
 * Local variables
 */

static bool Adapter_IsInitialized = false;
static Device_Handle_t Adapter_Device_BOARDCTRL;
static Device_Handle_t Adapter_Device_PCIConfigSpace;

static int Device_IRQ;


/*----------------------------------------------------------------------------
 * Adapter_Init
 *
 * Return Value
 *     true   Success
 *     false  Failure (fatal!)
 */
bool
Adapter_Init(void)
{
    Device_IRQ = -1;

    if (Adapter_IsInitialized != false)
    {
        LOG_WARN("Adapter_Init: Already initialized\n");
        return true;
    }

    // trigger first-time initialization of the adapter
    if (Device_Initialize(&Device_IRQ) < 0)
        return false;
#ifndef MTK_EIP97_DRIVER
    Adapter_Device_PCIConfigSpace = Device_Find("PCI_CONFIG_SPACE");
    if (Adapter_Device_PCIConfigSpace == NULL)
    {
        LOG_CRIT("Adapter_Init: Failed to locate PCI_CONFIG_SPACE\n");
        return false;
    }

    Adapter_Device_BOARDCTRL = Device_Find("BOARD_CTRL");
    if (Adapter_Device_BOARDCTRL == NULL)
    {
        LOG_CRIT("Adapter_Init: Failed to locate BOARD_CTRL\n");
        return false;
    }	
#endif

    if (!DMAResource_Init())
    {
        Device_UnInitialize();		
        return false;
    }
#ifndef MTK_EIP97_DRIVER
    {
        uint32_t Value;
        int VendorID, DeviceID;

        Value = Device_Read32(
                    Adapter_Device_PCIConfigSpace,
                    PCICONFIG_REG_ID);

        VendorID = PCICONFIG_ID_EXTRACT_VENDOR(Value);
        DeviceID = PCICONFIG_ID_EXTRACT_DEVICE(Value);

        IDENTIFIER_NOT_USED(VendorID);
        IDENTIFIER_NOT_USED(DeviceID);

        LOG_INFO(
            "Adapter_Init: "
            "PCI device: "
            "Vendor=0x%X, "
            "Device=0x%X\n",
            VendorID,
            DeviceID);
    }

    // initialize the PCI device
    // command and status register - Writing value 0x146 to this register
    // is recommended before accessing SafeXcel-97 FPGA
    {
        uint32_t Value;

        Value = PCICONFIG_STATCMD_MEMORYACCESS_ENABLE +
                PCICONFIG_STATCMD_BUSMASTER_ENABLE +
                PCICONFIG_STATCMD_PARITYERR_ENABLE +
                PCICONFIG_STATCMD_SYSTEMERR_ENABLE;

        Device_Write32(
                Adapter_Device_PCIConfigSpace,
                PCICONFIG_REG_STATCMD,
                Value);
    }

    // Setting cache line size
    // maintain all other bits (set by BIOS or OS)
    {
        uint32_t OldValue, Value;

        Value = Device_Read32(
                    Adapter_Device_PCIConfigSpace,
                    PCICONFIG_REG_CONFIG);
        OldValue = Value;

        IDENTIFIER_NOT_USED(OldValue);

        Value = PCICONFIG_CONFIG_UPDATE_CACHELINESIZE(
                    Value,
                    ADAPTER_PCICONFIG_CACHELINESIZE);

#ifdef ADAPTER_PCICONFIG_MASTERLATENCYTIMER
        Value = PCICONFIG_CONFIG_UPDATE_MASTERLATENCYTIMER(
                    Value,
                    ADAPTER_PCICONFIG_MASTERLATENCYTIMER);
#endif

        Device_Write32(
                Adapter_Device_PCIConfigSpace,
                PCICONFIG_REG_CONFIG,
                Value);

        LOG_INFO(
            "Adapter_Init: "
            "Changed PCI_Config[0x0c] "
            "from 0x%08x "
            "to 0x%08x\n",
            OldValue,
            Value);
    }

    // FPGA board specific functionality
    {
        // Enable PLB access on the board
        Device_Write32(Adapter_Device_BOARDCTRL, 0x8, 0x00400000);

#ifdef ADAPTER_FPGA_HW_RESET_ENABLE
        // Perform HW Reset for the EIP-97 FPGA board
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0);
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0xFFFFFFFF);
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0);
#endif // ADAPTER_FPGA_HW_RESET_ENABLE
    }
#endif /* MTK_EIP97_DRIVER */
#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
#ifdef MTK_EIP97_DRIVER
	Device_IRQ = SURFBOARDINT_CRYPTO;
#endif
    if (!Adapter_Interrupts_Init(Device_IRQ))
    {
        LOG_CRIT("Adapter_Init: Adapter_Interrupts_Init failed\n");

        DMAResource_UnInit();
        Device_UnInitialize();
        return false;
    }
#endif

    Adapter_IsInitialized = true;

    return true;    // success
}


/*----------------------------------------------------------------------------
 * Adapter_UnInit
 */
void
Adapter_UnInit(void)
{
    if (!Adapter_IsInitialized)
    {
        LOG_WARN("Adapter_UnInit: Adapter is not initialized\n");
        return;
    }

    Adapter_IsInitialized = false;

    DMAResource_UnInit();

#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
    Adapter_Interrupts_UnInit(Device_IRQ);
#endif

    Device_UnInitialize();
}


/*----------------------------------------------------------------------------
 * Adapter_Report_Build_Params
 */
void
Adapter_Report_Build_Params(void)
{
//#ifdef LOG_INFO_ENABLED
#if 1
	int dummy;

    // This function is dependent on config file cs_adapter.h.
    // Please update this when Config file for Adapter is changed.
    Log_FormattedMessage("Adapter build configuration:\n");

#define REPORT_SET(_X) \
    Log_FormattedMessage("\t" #_X "\n")

#define REPORT_STR(_X) \
    Log_FormattedMessage("\t" #_X ": %s\n", _X)

#define REPORT_INT(_X) \
    dummy = _X; Log_FormattedMessage("\t" #_X ": %d\n", _X)

#define REPORT_HEX32(_X) \
    dummy = _X; Log_FormattedMessage("\t" #_X ": 0x%08X\n", _X)

#define REPORT_EQ(_X, _Y) \
    dummy = (_X + _Y); Log_FormattedMessage("\t" #_X " == " #_Y "\n")

#define REPORT_EXPL(_X, _Y) \
    Log_FormattedMessage("\t" #_X _Y "\n")

    // Adapter PEC
#ifdef ADAPTER_PEC_DBG
    REPORT_SET(ADAPTER_PEC_DBG);
#endif

#ifdef ADAPTER_PEC_STRICT_ARGS
    REPORT_SET(ADAPTER_PEC_STRICT_ARGS);
#endif

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    REPORT_SET(ADAPTER_PEC_ENABLE_SCATTERGATHER);
#endif

#ifdef ADAPTER_PEC_SEPARATE_RINGS
    REPORT_SET(ADAPTER_PEC_SEPARATE_RINGS);
#else
    REPORT_EXPL(ADAPTER_PEC_SEPARATE_RINGS, " is NOT set => Overlapping");
#endif

#ifdef ADAPTER_PEC_ARMRING_ENABLE_SWAP
    REPORT_SET(ADAPTER_PEC_ARMRING_ENABLE_SWAP);
#endif

    REPORT_INT(ADAPTER_PEC_DEVICE_COUNT);
    REPORT_INT(ADAPTER_PEC_MAX_PACKETS);
    REPORT_INT(ADAPTER_MAX_PECLOGICDESCR);
    REPORT_INT(ADAPTER_PEC_MAX_SAS);
    REPORT_INT(ADAPTER_DESCRIPTORDONETIMEOUT);
    REPORT_INT(ADAPTER_DESCRIPTORDONECOUNT);

#ifdef ADAPTER_REMOVE_BOUNCEBUFFERS
    REPORT_EXPL(ADAPTER_REMOVE_BOUNCEBUFFERS, " is SET => Bounce DISABLED");
#else
    REPORT_EXPL(ADAPTER_REMOVE_BOUNCEBUFFERS, " is NOT set => Bounce ENABLED");
#endif

#ifdef ADAPTER_EIP202_INTERRUPTS_ENABLE
    REPORT_EXPL(ADAPTER_EIP202_INTERRUPTS_ENABLE,
            " is SET => Interrupts ENABLED");
#else
    REPORT_EXPL(ADAPTER_EIP202_INTERRUPTS_ENABLE,
            " is NOT set => Interrupts DISABLED");
#endif

#ifdef ADAPTER_64BIT_HOST
    REPORT_EXPL(ADAPTER_64BIT_HOST,
                " is SET => addresses are 64-bit");
#else
    REPORT_EXPL(ADAPTER_64BIT_HOST,
                " is NOT set => addresses are 32-bit");
#endif

#ifdef ADAPTER_64BIT_DEVICE
    REPORT_EXPL(ADAPTER_64BIT_DEVICE,
                " is SET => full 64-bit DMA addresses usable");
#else
    REPORT_EXPL(ADAPTER_64BIT_DEVICE,
                " is NOT set => DMA addresses must be below 4GB");
#endif

#ifdef ADAPTER_DMARESOURCE_BANKS_ENABLE
    REPORT_SET(ADAPTER_DMARESOURCE_BANKS_ENABLE);
#endif

    // Log
    Log_FormattedMessage("Logging:\n");

#if (LOG_SEVERITY_MAX == LOG_SEVERITY_INFO)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_INFO);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_WARNING)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_WARNING);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_CRITICAL)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_CRITICAL);
#else
    REPORT_EXPL(LOG_SEVERITY_MAX, " - Unknown (not info/warn/crit)");
#endif

    // Adapter other
    Log_FormattedMessage("Other:\n");
    REPORT_STR(ADAPTER_DRIVER_NAME);
    REPORT_STR(ADAPTER_LICENSE);
    REPORT_HEX32(ADAPTER_INTERRUPTS_TRACEFILTER);

    IDENTIFIER_NOT_USED(dummy);

#endif //LOG_INFO_ENABLED
}


/* end of file adapter_init.c */

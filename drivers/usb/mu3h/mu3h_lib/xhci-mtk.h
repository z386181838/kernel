#ifndef _XHCI_MTK_H
#define _XHCI_MTK_H

#include <linux/version.h>
#include <linux/usb.h>
#include "../mu3h/mu3h_drv/xhci.h"
#include "xhci-mtk-scheduler.h"
#include "ssusb_sifslv_ippc.h"
#include "ssusb_usb3_mac_csr.h"
#include "ssusb_usb3_sys_csr.h"
#include "ssusb_usb2_csr.h"

/* U3H IP CONFIG: 
 * enable this according to the U3H IP num of the project
 */
#define  CFG_DEV_U3H0   1  //if the project has one or more U3H IP, enable this
#if defined (CONFIG_MULTIPLE_MU3H_PORT)
#define  CFG_DEV_U3H1   1  //if the project has two or more U3H IP, enable this
#else
#define  CFG_DEV_U3H1   0
#endif

#define FPGA_MODE       0   //if run in FPGA,enable this
#define OTG_SUPPORT     0   //if OTG support,enable this

/************* XHCI irq number ************/
/* U3H irq number for MT7623*/
#if CFG_DEV_U3H0
#define U3H_IRQ0 228
#endif

#if CFG_DEV_U3H1
#define U3H_IRQ1 229 
#endif

/************* Base address ***************/
/*U3H register bank*/
#if CFG_DEV_U3H0
//physical base address for U3H IP0
#define U3H_BASE0	    0x1A1C0000
#define IPPC_BASE0          0x1A1C4700
//#define U3H_BASE0	    0xFA1C0000
//#define IPPC_BASE0          0xFA1C4700
#endif

#if CFG_DEV_U3H1
//physical base address for U3H IP1
#define U3H_BASE1	    0x1A240000
#define IPPC_BASE1          0x1A244700
//#define U3H_BASE1	    0xFA240000
//#define IPPC_BASE1          0xFA244700
#endif

/* Clock source */
//Clock source may differ from project to project. Please check integrator
#define	U3_REF_CK_VAL	25			//MHz = value
#define	U3_SYS_CK_VAL	125			//MHz = value

/************ Defined for PHY init of FPGA MODE**********/
//#if FPGA_MODE
/*Defined for PHY init in FPGA MODE*/
//change this value according to U3 PHY calibration
#define U3_PHY_PIPE_PHASE_TIME_DELAY	0x0a 
//#endif

/*FIXME*/
//offset may differ from project to project. Please check integrator
#define SSUSB_USB3_CSR_OFFSET 0x00002400
#define SSUSB_USB2_CSR_OFFSET 0x00003400


/* FIXME */
#ifdef CONFIG_USB_MU3H_TEST_DRV 
#define SSUSB_U3_XHCI_BASE	0xFA1C0000
#define SSUSB_U3_SIFSLV_BASE    0xFA1C4000
#define SSUSB_USB3_MAC_CSR_OFFSET 0x00002400
#define SSUSB_SIFSLV_IPPC_OFFSET     0x00000700
#define SSUSB_U3_MAC_BASE       (SSUSB_U3_XHCI_BASE + SSUSB_USB3_MAC_CSR_OFFSET)
#define SSUSB_U2_SYS_BASE	(SSUSB_U3_XHCI_BASE+SSUSB_USB2_CSR_OFFSET)
#define SIFSLV_IPPC             (SSUSB_U3_SIFSLV_BASE + SSUSB_SIFSLV_IPPC_OFFSET)
#endif

#define MTK_U3H_SIZE	0x4000
#define MTK_IPPC_SIZE	0x100
#define U3_LTSSM_TIMING_PARAMETER3_VALUE        0x3E8012C
/*=========================================================================================*/


#define u3h_writelmsk(addr, data, msk) \
	{ writel(((readl(addr) & ~(msk)) | ((data) & (msk))), addr); \
	}

struct mtk_u3h_hw {
//	char u3_port_num;
//  	char u2_port_num;
    	unsigned char  *u3h_virtual_base;
	unsigned char  *ippc_virtual_base;
	struct sch_port u3h_sch_port[MAX_PORT_NUM];
};

extern struct mtk_u3h_hw u3h_hw;

void reinitIP(struct device *dev);
void setInitialReg(struct device *dev);
void dbg_prb_out(void);
int u3h_phy_init(void);
int get_xhci_u3_port_num(struct device *dev);
int get_xhci_u2_port_num(struct device *dev);


#if 0
/*
  mediatek probe out
*/
/************************************************************************************/

#define SW_PRB_OUT_ADDR	(SIFSLV_IPPC+0xc0)		//0xf00447c0
#define PRB_MODULE_SEL_ADDR	(SIFSLV_IPPC+0xbc)	//0xf00447bc

static inline void mtk_probe_init(const u32 byte){
	__u32 __iomem *ptr = (__u32 __iomem *) PRB_MODULE_SEL_ADDR;
	writel(byte, ptr);
}

static inline void mtk_probe_out(const u32 value){
	__u32 __iomem *ptr = (__u32 __iomem *) SW_PRB_OUT_ADDR;
	writel(value, ptr);
}

static inline u32 mtk_probe_value(void){
	__u32 __iomem *ptr = (__u32 __iomem *) SW_PRB_OUT_ADDR;

	return readl(ptr);
}

#endif
#endif

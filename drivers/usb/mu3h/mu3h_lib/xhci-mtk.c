#include "xhci-mtk.h"
#include "xhci-mtk-power.h"
#include "xhci-mtk-scheduler.h"
#include "mtk-phy.h"
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/module.h>


static const char hcd_name[] = "xhci-hcd";

/*defined for FPGA PHY daughter board init*/
//#if FPGA_MODE
int mtk_u3h_phy_init(void)
{
    	int PhyDrv;
	int TimeDelay;

	PhyDrv = 2;
	TimeDelay = U3_PHY_PIPE_PHASE_TIME_DELAY;

	printk("*** %s ***\n", __func__);	
	u3phy_init();
	printk(KERN_DEBUG "phy registers and operations initial done\n");
#ifdef CONFIG_PROJECT_7623
#if CFG_DEV_U3H0
	if(u3phy_ops->u2_slew_rate_calibration){
		u3phy_ops->u2_slew_rate_calibration(u3phy);
	}
	else{
		printk(KERN_ERR "WARN: PHY doesn't implement port0 u2 slew rate calibration function\n");
	}
#endif
#if CFG_DEV_U3H1
	if(u3phy_ops->u2_slew_rate_calibration_p1){
		u3phy_ops->u2_slew_rate_calibration_p1(u3phy);
	}
	else{
		printk(KERN_ERR "WARN: PHY doesn't implement port1 u2 slew rate calibration function\n");
	}
#endif
#else
	if(u3phy_ops->u2_slew_rate_calibration){
		u3phy_ops->u2_slew_rate_calibration(u3phy);
	}
	else{
		printk(KERN_ERR "WARN: PHY doesn't implement port0 u2 slew rate calibration function\n");
	}

#endif

	if(u3phy_ops->init(u3phy) != PHY_TRUE){
		printk(KERN_ERR "WARN: PHY INIT FAIL\n");
		return PHY_FALSE;
	}
	if((u3phy_ops->change_pipe_phase(u3phy, PhyDrv, TimeDelay)) != PHY_TRUE){
		printk(KERN_ERR "WARN: PHY change_pipe_phase FAIL\n");
		return PHY_FALSE;
	}

	return PHY_TRUE;
}
//#endif

int get_xhci_u3_port_num(struct device *dev){
	struct mtk_u3h_hw *u3h_hw;	
	__u32 __iomem *addr;	
	u32 data;
	int u3_port_num;

	u3h_hw = dev->platform_data;

	addr = (u32 __iomem *)(u3h_hw->ippc_virtual_base + U3H_SSUSB_IP_XHCI_CAP);
	data = readl(addr);
	u3_port_num = data & SSUSB_IP_XHCI_U3_PORT_NO;

	return u3_port_num;
}

int get_xhci_u2_port_num(struct device *dev){
	struct mtk_u3h_hw *u3h_hw;	
	__u32 __iomem *addr;	
	u32 data;
	int u2_port_num;
	
	u3h_hw = dev->platform_data;

	addr = (u32 __iomem *)(u3h_hw->ippc_virtual_base + U3H_SSUSB_IP_XHCI_CAP);
	data = readl(addr);
	
	u2_port_num = (data & SSUSB_IP_XHCI_U2_PORT_NO) >> 8;
	
	return u2_port_num;
}

void setInitialReg(struct device *dev){
	int i;
	struct mtk_u3h_hw *u3h_hw;
	__u32 __iomem *usb3_csr_base;
	__u32 __iomem *usb2_csr_base;
	
	__u32 __iomem *addr;	
	u32 data;

    	int u3_port_num,u2_port_num;

    	u3h_hw = dev->platform_data;
    	usb3_csr_base = (u32 __iomem *)(u3h_hw->u3h_virtual_base + SSUSB_USB3_CSR_OFFSET);
	usb2_csr_base = (u32 __iomem *)(u3h_hw->u3h_virtual_base + SSUSB_USB2_CSR_OFFSET);

	//get u3 & u2 port num
	u3_port_num = get_xhci_u3_port_num(dev);
	u2_port_num = get_xhci_u2_port_num(dev);

	for(i=0; i<u3_port_num; i++){
    	//set MAC reference clock speed
    	addr = usb3_csr_base + U3H_UX_EXIT_LFPS_TIMING_PARAMETER;
    	data = ((300*U3_REF_CK_VAL + (1000-1)) / 1000);
    	u3h_writelmsk(addr,data,RX_UX_EXIT_LFPS_REF);
    	
    	addr = usb3_csr_base + U3H_REF_CK_PARAMETER;
    	data = U3_REF_CK_VAL;
    	u3h_writelmsk(addr,data,REF_1000NS);
    
    	//set SYS_CK
    	addr = usb3_csr_base + U3H_TIMING_PULSE_CTRL;
    	data = U3_SYS_CK_VAL;
    	u3h_writelmsk(addr,data,CNT_1US_VALUE);
	}

	for(i=0; i<u2_port_num; i++){
    	addr = usb2_csr_base + U3H_USB20_TIMING_PARAMETER;
    	data = U3_SYS_CK_VAL;
    	u3h_writelmsk(addr,data,TIME_VALUE_1US);
	}	
}

void reinitIP(struct device *dev){
	enableAllClockPower(dev);
    	setInitialReg(dev);
	mtk_xhci_scheduler_init(dev);
}

#if CFG_DEV_U3H0
static struct resource mtk_resource_u3h0[] = {
	[0] = {
		    .start = U3H_IRQ0,
		    .end   = U3H_IRQ0,
		    .flags = IORESOURCE_IRQ,
	},
	[1] = {
            .name = "u3h",
			 /*physical address*/
		    .start = U3H_BASE0,
		    .end   = U3H_BASE0 + MTK_U3H_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},
	[2] = {
            .name = "ippc",
			 /*physical address*/
		    .start = IPPC_BASE0,
		    .end   = IPPC_BASE0 + MTK_IPPC_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},
};
#endif

#if CFG_DEV_U3H1
static struct resource mtk_resource_u3h1[] = {
	[0] = {
		    .start = U3H_IRQ1,
		    .end   = U3H_IRQ1,
		    .flags = IORESOURCE_IRQ,
	},
	[1] = {
            .name = "u3h",
			 /*physical address*/
		    .start = U3H_BASE1,
		    .end   = U3H_BASE1 + MTK_U3H_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},
	[2] = {
            .name = "ippc",
			 /*physical address*/
		    .start = IPPC_BASE1,
		    .end   = IPPC_BASE1 + MTK_IPPC_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},
	
};
#endif

#if CFG_DEV_U3H0
struct mtk_u3h_hw u3h_hw0;
#endif

#if CFG_DEV_U3H1
struct mtk_u3h_hw u3h_hw1;
#endif

static u64 mtk_u3h_dma_mask = 0xffffffffUL;

static struct platform_device mtk_device_u3h[] = {
#if CFG_DEV_U3H0
	{
	    .name          = hcd_name,
	    .id            = 0,
	    .resource      = mtk_resource_u3h0,
	    .num_resources = ARRAY_SIZE(mtk_resource_u3h0),
	    .dev           = { 
                            	.platform_data = &u3h_hw0,
			 	.dma_mask = &mtk_u3h_dma_mask,					
				.coherent_dma_mask = 0xffffffffUL,
                             },
     },
#endif
#if CFG_DEV_U3H1
	{
	    .name          = hcd_name,
	    .id            = 1,
	    .resource      = mtk_resource_u3h1,
	    .num_resources = ARRAY_SIZE(mtk_resource_u3h1),
	    .dev           = { 
                             	.platform_data = &u3h_hw1,
				.dma_mask = &mtk_u3h_dma_mask,					
				.coherent_dma_mask = 0xffffffffUL,
                             },
     },
#endif
};
	
static 	int __init mtk_u3h_init(void)
{
    	int ret;
	int i;
	int u3h_dev_num;

	u3h_dev_num = sizeof(mtk_device_u3h) / sizeof(mtk_device_u3h[0]);
    	for (i = 0; i < u3h_dev_num; i++){
		printk("***%s: port%d***\n", __func__, i);
        	ret = platform_device_register(&mtk_device_u3h[i]); 
        	if (ret != 0){
            		return ret;
        	}
    	}

    	mtk_u3h_phy_init();	
	
	return ret;
}

static void __exit mtk_u3h_cleanup(void)
{
	int u3h_dev_num;
	int i;

	u3h_dev_num = sizeof(mtk_device_u3h) / sizeof(mtk_device_u3h[0]);
    	for (i = 0; i < u3h_dev_num; i++){
       		platform_device_unregister(&mtk_device_u3h[i]); 
    	}
}
module_init(mtk_u3h_init);
module_exit(mtk_u3h_cleanup);
MODULE_LICENSE("GPL");

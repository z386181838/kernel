#ifndef __DISP_DRV_PLATFORM_H__
#define __DISP_DRV_PLATFORM_H__

#ifdef BUILD_UBOOT
#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <lcd.h>
#include <video_fb.h>
#include <mmc.h>
#include <part.h>
#include <fat.h>
#include <malloc.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/disp_drv.h>
#include <asm/arch/lcd_drv.h>
#include <asm/arch/dpi_drv.h>
#include <asm/arch/dsi_drv.h>
#include <asm/arch/lcd_reg.h>
#include <asm/arch/dpi_reg.h>
#include <asm/arch/dsi_reg.h>
#include <asm/arch/disp_assert_layer.h>
#include <asm/arch/disp_drv_log.h>
#include <asm/arch/mt65xx_disp_drv.h>
#include "lcm_drv.h"


#undef CONFIG_MTK_M4U_SUPPORT
#undef CONFIG_MTK_HDMI_SUPPORT
#define DEFINE_SEMAPHORE(x)  
#define down_interruptible(x) 0
#define up(x)                
#define DBG_OnTriggerLcd()   

#else
#include <linux/dma-mapping.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/m4u.h>
//#include <mach/mt6585_pwm.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>
//#include <mach/boot.h>
#include <board-custom.h>
#include <linux/disp_assert_layer.h>
#include "ddp_hal.h"
#include "ddp_drv.h"
#include "ddp_path.h"
#include "ddp_rdma.h"
#include "dsi_drv.h"
#endif

///LCD HW feature options for MT6575
#define MTK_LCD_HW_SIF_VERSION      2       ///for MT6575, we naming it is V2 because MT6516/73 is V1...
#define MTKFB_NO_M4U
#define MT65XX_NEW_DISP
//#define MTK_LCD_HW_3D_SUPPORT
#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))
#define MTK_FB_ALIGNMENT 16
#define MTK_FB_SYNC_SUPPORT
#ifndef MTK_OVERLAY_ENGINE_SUPPORT
#define MTK_OVL_DECOUPLE_SUPPORT
#endif

#if defined(MTK_ALPS_BOX_SUPPORT)
#define MTK_HDMI_MAIN_PATH 1 
#else
#define MTK_HDMI_MAIN_PATH 0
#endif


#if defined(MTK_ALPS_BOX_TVE_SUPPORT)
//#if defined(MTK_ALPS_BOX_TVE_CVBS_DITHERING)
#define MTK_FOR_CVBS_DITHERING
//#endif

#define MTK_CVBS_SUB_PATH_SUPPORT 1
#else
#define MTK_CVBS_SUB_PATH_SUPPORT 0
#endif

#define MTK_HDMI_MAIN_PATH_TEST 0
#define MTK_HDMI_MAIN_PATH_TEST_SIZE 0
#define MTK_DISABLE_HDMI_BUFFER_FROM_RDMA1 1


#define MTK_CVBS_FORMAT_YUV 1


#if MTK_CVBS_FORMAT_YUV
//YUV422
//#define RDMA1_INTPUT_FORMAT eYUYV

#define RDMA1_INTPUT_FORMAT eUYVY
#define RDMA1_SRC_FORMAT MTK_FB_FORMAT_YUV422
#define RDMA1_OUTPUT_FORMAT RDMA_OUTPUT_FORMAT_YUV444
#define RDMA1_SRC_BPP 2
#define TVE_DISPIF_FORMAT DISPIF_FORMAT_YUV422 
#define TVE_DISPIF_TYPE DISPIF_TYPE_CVBS
#define WDMA_INPUT_COLOR_FORMAT  WDMA_INPUT_FORMAT_YUV444
#else
 //  RGB888 
#define RDMA1_INTPUT_FORMAT eRGB888
#define RDMA1_SRC_FORMAT MTK_FB_FORMAT_RGB888
#define RDMA1_OUTPUT_FORMAT RDMA_OUTPUT_FORMAT_ARGB
#define RDMA1_SRC_BPP 3
#define TVE_DISPIF_FORMAT DISPIF_FORMAT_RGB888
#define TVE_DISPIF_TYPE HDMI
#define WDMA_INPUT_COLOR_FORMAT  WDMA_INPUT_FORMAT_ARGB
#endif



#define TVE_DISP_WIDTH 720
#define TVE_DISP_HEIGHT 576
#define RDMA1_FRAME_BUFFER_NUM 3

#define HDMI_DISP_WIDTH 1920
#define HDMI_DISP_HEIGHT 1080

#define HDMI_DEFAULT_RESOLUTION HDMI_VIDEO_1920x1080p_60Hz

/*for MTK_HDMI_MAIN_PATH*/
extern unsigned int ddp_dbg_level;
extern unsigned char *disp_module_name[];
#define DDP_FUNC_LOG (1 << 0)
#define DDP_FLOW_LOG (1 << 1)
#define DDP_COLOR_FORMAT_LOG (1 << 2)
#define DDP_FB_FLOW_LOG (1 << 3)
#define DDP_RESOLUTION_LOG (1 << 4)
#define DDP_OVL_FB_LOG (1 << 5)
#define DDP_FENCE_LOG (1 << 6)
#define DDP_TVE_FENCE_LOG (1 << 7)
#define DDP_FENCE1_LOG (1 << 8)
#define DDP_FENCE2_LOG (1 << 9)
#define DDP_CAPTURE_LOG (1 << 10)

#define DISP_PRINTF(level, string, args...) do{ \
	if(ddp_dbg_level & (level)) { \
		printk("[DDP] "string,##args); \
	} \
}while(0)

#define DISP_PRINTF_FUNC() do{ \
		if(ddp_dbg_level & DDP_FLOW_LOG) printk("[DDP] %s #%d\n",__func__,__LINE__); \
}while(0)

#endif //__DISP_DRV_PLATFORM_H__

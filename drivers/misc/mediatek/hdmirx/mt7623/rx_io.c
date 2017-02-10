/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/

 /******************************************************************************
*[File]             rx_io.c
*[Version]          v0.1
*[Revision Date]    2009-04-18
*[Author]           Kenny Hsieh
*[Description]
*    source file for hdmi general control routine
*
*
******************************************************************************/
#if 1//(DRV_SUPPORT_HDMI_RX)

#include <generated/autoconf.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/switch.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <asm/tlbflush.h>
#include <asm/page.h>
#include <cust_eint.h>
#include "cust_gpio_usage.h"
#include "mach/eint.h"
#include "mach/irqs.h"

#include <mach/devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_boot.h>

#include "rx_io.h"
#include "hal_io.h"
#include "hdmirx.h"
#include "typedef.h"


void Delay5MS(UINT32 count)
{
	UINT32 u4Index; 	
	for(u4Index=0;u4Index<count;u4Index++)
	{
		msleep(1);
	}
}

void vHalSetRxPort1HPDLevel(BOOL fgHighLevel)
{
	if(fgHighLevel)
	{
#ifdef GPIO_HDMI_HPD_CBUS_RX_PIN
		mt_set_gpio_mode(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_MODE_00);  
		mt_set_gpio_dir(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_OUT_ONE);
#endif
	}
	else
	{
#ifdef GPIO_HDMI_HPD_CBUS_RX_PIN
		mt_set_gpio_mode(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_MODE_00);  
		mt_set_gpio_dir(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_HDMI_HPD_CBUS_RX_PIN, GPIO_OUT_ZERO);
#endif
	}
}
#endif//#ifdef DRV_SUPPORT_HDMI_RX

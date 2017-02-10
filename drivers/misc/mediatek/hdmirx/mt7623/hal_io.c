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
*[File]             hal_io.c
*[Version]          v0.1
*[Revision Date]    2010-01-04
*[Author]           Kenny Hsieh
*[Description]
*    source file for global varabile and function in av_d directory
*
*
******************************************************************************/


#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/jiffies.h>

#include "hal_io.h"
#include "hdmirx.h"
#include "typedef.h"
//#include "x_os.h"

UINT16 u2HdmiRxIO32Read2B(UINT32 reg32)
{
    UINT32 addr=reg32&~3;
    switch(reg32&3)
    {
        default:
        case 0:
        case 2:
            return (*(volatile UINT16 *)(reg32));
        case 1:
            return  ((*(volatile UINT32 *)(addr))>>8)&0xffff;
        case 3:
            //ASSERT((reg32&3)<3);
            return  ((*(volatile UINT32 *)(addr))>>24)&0xff;
    }
}

void vHdmiRxIO32Write1BMsk(UINT32 reg32, UINT32 val8, UINT8 msk8)
{
    //CRIT_STATE_T csState;
    UINT32 u4Val, u4Msk;
    UINT8 bByte;

    bByte = reg32&3;
    reg32 &= ~3;
    val8 &= msk8;
    u4Msk = ~(UINT32)(msk8<<((UINT32)bByte<<3));

    //csState = x_crit_start();
    u4Val = (*(volatile UINT32 *)(reg32));
    u4Val = ((u4Val & u4Msk) | ((UINT32)val8 << (bByte<<3)));
    (*(volatile UINT32 *)(reg32)=(u4Val));
    //x_crit_end(csState);

}

void vHdmiRxIO32Write2BMsk(UINT32 reg32, UINT32 val16, UINT16 msk16)
{
    //CRIT_STATE_T csState;
    UINT32 u4Val, u4Msk;
    UINT8 bByte;

    bByte = reg32&3;
    //ASSERT(bByte<3);

    reg32 &= ~3;
    val16 &= msk16;
    u4Msk = ~(UINT32)(msk16<<((UINT32)bByte<<3));

    //csState = x_crit_start();
    u4Val = (*(volatile UINT32 *)(reg32));
    u4Val = ((u4Val & u4Msk) | ((UINT32)val16 << (bByte<<3)));
    (*(volatile UINT32 *)(reg32)=(u4Val));
    //x_crit_end(csState);

}

void vHdmiRxIO32Write4BMsk(UINT32 reg32, UINT32 val32, UINT32 msk32)
{
    //CRIT_STATE_T csState;

    //ASSERT((reg32&3)==0);

    val32 &=msk32;

    //csState = x_crit_start();
    (*(volatile UINT32 *)(reg32)=((*(volatile UINT32 *)(reg32))&~msk32)|val32);
    //x_crit_end(csState);
}


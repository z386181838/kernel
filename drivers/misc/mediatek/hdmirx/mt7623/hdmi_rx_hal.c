#include <linux/sched.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include "typedef.h"

#include <cust_eint.h>
#include "cust_gpio_usage.h"
#include "mach/eint.h"
#include "mach/irqs.h"


#include <mach/devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_boot.h>

#include "hdmirx.h"
#include "hdmi_rx_task.h"
#include "hdmi_rx_hw.h"
#include "hal_io.h"
#include "hdmi_rx_hal.h"
#include "hdmirx_utils.h"
#include "rx_io.h"
#include "hdmitable.h"

UINT32 _wHalHDMI_EQ_ZERO_VALUE;
UINT32 _wHalHDMI_EQ_BOOST_VALUE;
UINT32 _wHalHDMI_EQ_SEL_VALUE;
UINT32 _wHalHDMI_EQ_GAIN_VALUE;
UINT32 _wHalHDMI_HDCP_MASk1;
UINT32 _wHalHDMI_HDCP_MASk2;
UINT8 _CrcResult[3][3];
UINT32 _u4CKPDRDOLD = 0;

BYTE pdRepeaterEdid[] =
{
0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x4d,0xd9,0x00,0xf5,0x10,0x00,0x00,0x00,
0x00,0x12,0x01,0x03,0x80,0x00,0x00,0x78,0x0a,0xee,0x91,0xa3,0x54,0x4c,0x99,0x26,
0x0f,0x50,0x54,0x20,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x1d,0x00,0x72,0x51,0xd0,0x1e,0x20,0x6e,0x28,
0x55,0x00,0xc4,0x8e,0x21,0x00,0x00,0x1e,0x8c,0x0a,0xd0,0x90,0x20,0x40,0x31,0x20,
0x0c,0x40,0x55,0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x00,0x00,0x00,0xfc,0x00,0x4d,
0x54,0x38,0x35,0x35,0x35,0x20,0x20,0x41,0x56,0x52,0x0a,0x20,0x00,0x00,0x00,0xfd,
0x00,0x30,0x3e,0x17,0x2f,0x08,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0xd6,
0x02,0x03,0x35,0x71,0x50,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x90,0x11,0x12,0x13,
0x14,0x15,0x16,0x1f,0x20,0x2c,0x09,0x7f,0x07,0x0f,0x1f,0x07,0x17,0x07,0x50,0x3f,
0x07,0xc0,0x83,0x4f,0x00,0x00,0x6e,0x03,0x0c,0x00,0x20,0x00,0x80,0x2d,0x20,0x00,
0x00,0x00,0x00,0x80,0x00,0x8c,0x0a,0xd0,0x8a,0x20,0xe0,0x2d,0x10,0x10,0x3e,0x96,
0x00,0xc4,0x8e,0x21,0x00,0x00,0x18,0x01,0x1d,0x80,0x18,0x71,0x1c,0x16,0x20,0x58,
0x2c,0x25,0x00,0xc4,0x8e,0x21,0x00,0x00,0x9e,0x01,0x1d,0x00,0xbc,0x52,0xd0,0x1e,
0x20,0xb8,0x28,0x55,0x40,0xc4,0x8e,0x21,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x76,
};

void vHalSetEqZeroValueVar(UINT32 u4Data)
{
  _wHalHDMI_EQ_ZERO_VALUE = u4Data;
}

void vHalSetEqBoostValueVar(UINT32 u4Data)
{
   _wHalHDMI_EQ_BOOST_VALUE = u4Data;
}

void vHalSetEqSelValueVar(UINT32 u4Data)
{
  _wHalHDMI_EQ_SEL_VALUE = u4Data;
}

void vHalSetEqGainValueVar(UINT32 u4Data)
{
  _wHalHDMI_EQ_GAIN_VALUE = u4Data;
}

void vHalSetRxHdcpMask1Var(UINT32 u4Data)
{
  _wHalHDMI_HDCP_MASk1 = u4Data;
}

void vHalSetRxHdcpMask2Var(UINT32 u4Data)
{
  _wHalHDMI_HDCP_MASk2 = u4Data;
}

void vHalRxHDMITMDSCTRL(BOOL fgOn)
{
    // PD_TERM: Enable TMDS-PHY termination 50-ohm resistance
    if (fgOn)
    {
        /*turn on TMDS*/
		vRegWrite1B(PD_SYS_2,0xff);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,RG_HDMI_0_TERM_EN,RG_HDMI_0_TERM_EN);

    }
    else
    {
        /* turn off TMDS*/
		vRegWrite1B(PD_SYS_2,0xcf);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,0,RG_HDMI_0_TERM_EN);
    }

    return ;
}


void vHalSetRxHDMIHPDHigh(UINT8 u1HDMICurrSwitch)
{
    
    printk("[enter %s]u1HDMICurrSwitch = %d\n",__FUNCTION__,u1HDMICurrSwitch);
	switch(u1HDMICurrSwitch)
	{
	    case HDMI_SWITCH_1:
			printk("[enter %s]u1HDMICurrSwitch = %d\n",__FUNCTION__,u1HDMICurrSwitch);
			vHalSetRxPort1HPDLevel(TRUE);
		break;
			
		default:
			break;
	}
}

void vHalSetRxHDMIHPDLow(UINT8 u1HDMICurrSwitch)
{
	switch(u1HDMICurrSwitch)
	{
	    case HDMI_SWITCH_1:
			vHalSetRxPort1HPDLevel(FALSE);
		break;
			
		default:
			break;
	}
}

void vHalRxHwInit(void)
{
	UINT8 i;

	printk("HdmiRxPHYInit %s %d\n",__FUNCTION__,__LINE__);

	
	// ICG setting part
	vRxWriteReg(0xC000100,0x00000000);
	vRxWriteReg(0xC000110,0x00000000);
	vRxWriteReg(0xC000104,0x00000000);
	
	//HDMI Setting
	vRxWriteReg(0xC00001C,0xfb5aab4f);
	vRxWriteReg(0xC000020,0x00000000);
	vRxWriteReg(0xC000024,0x00000001);
	vRxWriteReg(0xC0062EC,0x00132000);
	vRxWriteReg(0xC0062EC,0x00102000);
	vRxWriteReg(0x0209164,0x0000000E);
	
	HDMIRX_WRITE32_MASK(SYS_CTRL_0,0x07<<1,0xff<<1);
	HDMIRX_WRITE32_MASK(REG_SYS_CTRL,0,RX_EN);

	/* power on all block */
	HDMIRX_WRITE32_MASK(REG_PD_SYS,0x01<<1,0xff<<1);
	HDMIRX_WRITE32_MASK(REG_PD_SYS,0xCf<<8,0xff<<8);
	HDMIRX_WRITE32_MASK(REG_PD_SYS,0xff<<16,0xff<<16);  // except Tri-state Audio/Video output
	HDMIRX_WRITE32_MASK(REG_PD_SYS,0xf3<<16,0xff<<16);	//solve analog PHY bandgap start-up
	HDMIRX_WRITE32_MASK(REG_PD_SYS,0xff<<16,0xff<<16); //solve analog PHY bandgap start-up

	HDMIRX_WRITE32(REG_TMDS_CTRL1,0x80200000);

	HDMIRX_WRITE32_MASK(REG_TMDS_CTRL1,_wHalHDMI_EQ_SEL_VALUE<<28, EQSEL);
	HDMIRX_WRITE32_MASK(REG_TMDS_CTRL1,_wHalHDMI_EQ_GAIN_VALUE<<20, EQ_GAIN);

	vHalRxHDMITMDSCTRL(FALSE);

	HDMIRX_WRITE32_MASK(REG_ACR_CTRL3,0x5C<<0,0xff<<1);

	HDMIRX_WRITE32_MASK(REG_LK_THRS_SVAL,0xffff<<0,0xffff<<0);
	/* I2S CTRL1
	bit 0 WS to SD shift First bit
	bit 1 SD data direction
	bit 2 SD Justification
	bit 3 UINT16 Select Left/right Polarity
	bit 4 MSB sign-extended
	bit 5 UINT16 size
	bit 6 sample clock edge
	bit 7 send invalid data enable
	*/
	HDMIRX_WRITE32_MASK(REG_I2S_CTRL,0x49<<16,0xff<<16);

	/*HDMI_CRIT1
	Preamble Criteria: selects the required number of repetitions
	for a valid preamble
	*/
	HDMIRX_WRITE32_MASK(REG_AUDP_STAT, 0x6<<8, PREAMBLE_CRI);
	//vRegWriteFldAlign(AUDP_STAT,0x0,PREAMBLE_CRI);
	/*HDMI_CRIT2
	Preamble Criteria: selects the required number of repetitions
	for a valid preamble
	*/
	HDMIRX_WRITE32_MASK(REG_TEST_CTRL,0xC<<0,0xff<<0);
	/*ACR_CFG
	ACR PLL configuration
	*/
	HDMIRX_WRITE32(REG_APLL0,0x21001680);

	/* int output polarity# */
	// vRegWrite1B(INTR_STATE0_0,0x2);//	vRegWrite1B(INTR_MASK1_1,0x2);

	/* set key mask to zero */
	HDMIRX_WRITE32_MASK(REG_EPST,0,KS_MASK_7_0);
	HDMIRX_WRITE32_MASK(REG_KS_MASK,0,0xff<<0);

	//Enable Audio SPDIF Output
	HDMIRX_WRITE32_MASK(REG_AUDRX_CTRL,0x1d<<8,0xff<<8);

	// KSCL_H
	HDMIRX_WRITE32_MASK(REG_VID_SET,0, KSCL_H);

	// HDCP Keymask
	HDMIRX_WRITE32_MASK(EPST_3,_wHalHDMI_HDCP_MASk1<<24,KS_MASK_7_0);		//vRegWrite1B(EPST_3,0xff);
	HDMIRX_WRITE32_MASK(REG_KS_MASK,_wHalHDMI_HDCP_MASk2<<0,0xff<<0);	//vRegWrite1B(KS_MASK_0,0xc3);

	// setting to avoid TMDS reset -> vHDMITMDSReset
	HDMIRX_WRITE32_MASK(REG_AUDP_STAT, 0 ,(BYP_SYNC|BYP_DVIFILT));

	// Clear Audio mute
	HDMIRX_WRITE32((REG_AUDP_STAT),(HDMIRX_READ32(REG_AUDP_STAT) & (~AUDIO_MUTE)));

	// increase the PCCNT resolution  from 128 to 1024 by adam
	HDMIRX_WRITE32_MASK(REG_VID_CRC_OUT, 0,XCLK_IN_PCLK_SEL);

	//decode di_DE and vi_DE by both preamble and guard-band
	HDMIRX_WRITE32_MASK(REG_MUTE_SET, 0x0, BYPASS_DI_GB);

	//modify 1080p twinkling dot
	//CKGEN_WRITE32(REG_RGBCLK_CFG, CKGEN_READ32(REG_RGBCLK_CFG) |RGB_INV_SEL );

	//deep color mode must refer output video clock not TMDS clock
	HDMIRX_WRITE32_MASK(REG_VID_CRC_OUT,XCLK_IN_DPCLK,XCLK_IN_DPCLK);
	HDMIRX_WRITE32(REG_CKDT,0x102b1b38);

	//enable  HW GAMUT packet decoder
	HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL,TT0_GAMUT_EN,TT0_GAMUT_EN);

	//enable  HW One Bit Audio packet decoder

	HDMIRX_WRITE32_MASK(REG_MUTE_SET,TDFIFO_SYNC_EN, TDFIFO_SYNC_EN);

	HDMIRX_WRITE32_MASK(REG_VID_VRES,VRES_MUTE_AUTO_CLR,VRES_MUTE_AUTO_CLR);

	HDMIRX_WRITE32_MASK(REG_VID_HRES,0x3,HCHG_CNT_THR);
	HDMIRX_WRITE32_MASK(REG_VID_HRES,0xf,HSTB_CNT_THR);
	HDMIRX_WRITE32_MASK(REG_VID_VRES,0x3,VSTB_CNT_THR);
	HDMIRX_WRITE32_MASK(REG_VID_SET,0x1,MUTE_CKDT);

	HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL1,TT88_0_NEW_GAMUT_ONLY, TT88_0_NEW_GAMUT_ONLY);
	HDMIRX_WRITE32_MASK(REG_INTR_MASK0,0,NEW_ACP_ONLY|NEW_AVI_ONLY|NEW_SPD_ONLY|NEW_AUD_ONLY|NEW_MPEG_ONLY|NEW_UNREC_ONLY);



	HDMIRX_WRITE32_MASK(REG_TMDS_CTRL1, MCK90SEL, MCK90SEL);
	
	HDMIRX_WRITE32(REG_TMDS_CTRL1,0x00000000);

    //HDMI RX ANALOG 
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON00,0x80000000);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON01,0x55A00E4A);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON02,0x2B217612);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON03,0x00B13F30);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON04,0x00000002);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON06,0x00000004);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON07,0x060A0000);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON06,0x00000802);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON02,0x3B217612);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON00,0x00000000);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON01,0x55A00A4A);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON02,0x3B217613);
	mdelay(1);
	HDMIRX_A_WRITE32(REG_HDMI_RX_CON01,0x55200A4A);
	mdelay(1);
	HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON00,0,RG_HDMI_CDR_RST);
	msleep(1);
	HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,0,RG_HDMI_EQ_RST);
	msleep(1);
	HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,RG_HDMI_0_DEEPCLRCLK_RSTN,RG_HDMI_0_DEEPCLRCLK_RSTN);
	msleep(1);
	HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,0,RG_HDMI_DATA_RST);
	msleep(1);
	
	HDMIRX_WRITE32(REG_VID_HRES,0x00002280);
	HDMIRX_WRITE32(REG_AUDRX_CTRL,0x00001FE4);
	HDMIRX_WRITE32(REG_I2S_CTRL,0xF1400000);
	HDMIRX_WRITE32(REG_PD_SYS,0xC0060001);
	HDMIRX_WRITE32(REG_AUDP_STAT,0x00020600);//zhiqiang modify for 720P 10BIT H ACTIVE ERROR
	HDMIRX_WRITE32(REG_SYS_CTRL,0x00001407);
	HDMIRX_WRITE32(REG_MUTE_SET,0x865E01EA);
	HDMIRX_WRITE32(HDCP_DEV,0x000003A0);
	HDMIRX_WRITE32(HDCP_ADDR,0x00000000);
	HDMIRX_WRITE32_MASK(REG_VID_CRC_OUT,(1<<22),(1<<22));
	HDMIRX_WRITE32_MASK(REG_VID_CRC_OUT,(1<<23),(1<<23));
	
	
	HDMIRX_WRITE32_MASK(REG_SYS_CTRL,(1<<12),(1<<12));
	HDMIRX_WRITE32_MASK(REG_MUTE_SET,(1<<31),(1<<31));
	vWriteHdmirxIoPadMsk(IO_HDMIRX_PAD_MODE,PAD_MODE_SCL_RX,PAD_MODE_SCL_RX_MASK);

	HDMIRX_WRITE32(REG_KS_MASK,0x00000000);
	HDMIRX_WRITE32(REG_EPST,0x0000ff00);
	HDMIRX_WRITE32(REG_VID_MODE,0x000a0000);
	HDMIRX_WRITE32_MASK(REG_APLL0,(1<<12)|(1<<13),(1<<12)|(1<<13));
	HDMIRX_WRITE32_MASK(REG_APLL0,0,(1<<14));
	HDMIRX_WRITE32_MASK(REG_HDMI_DIG_MACRO,(1<<10)|(1<<11)|(1<<12),(1<<10)|(1<<11)|(1<<12)|(1<<13)); //temp test
	
    // DDCCI PART INIT
	DDCCI_WRITE32_MASK(REG_DDC_MISC_CNTL0, (1<<26)|(1<<23),(1<<26)|(1<<23));//enable download mode
	for(i=0; i<64; i++)
	{
		DDCCI_WRITE32(REG_DDC_EDID_DOWNLOAD_PORT, pdRepeaterEdid[i*4]|(pdRepeaterEdid[i*4+1]<<8)|(pdRepeaterEdid[i*4+2]<<16)|(pdRepeaterEdid[i*4+3]<<24));//address 255 is invalid
	}
	DDCCI_WRITE32_MASK(REG_DDC_MISC_CNTL0, (1<<26),(1<<26)|(1<<23));	//disable download mode
	DDCCI_WRITE32_MASK(REG_DDC_HDMI0_CNTL1,(pdRepeaterEdid[255]<<16),(0xff<<16));//checksum
	DDCCI_WRITE32_MASK(REG_DDC_HDMI0_CNTL0, 0,(1<<16)|(1<<17));//disable ddcci

	vHalEnableINTR2_CKDT(FALSE);
}

void vHalResetHdmiRxTotalModule(void)
{
    HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,RG_HDMI_EQ_RST,RG_HDMI_EQ_RST);
    HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON00,RG_HDMI_CDR_RST,RG_HDMI_CDR_RST);
    HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,0,RG_HDMI_EQ_RST);
    HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON00,0,RG_HDMI_CDR_RST);
	
	msleep(10);
	HDMIRX_WRITE32_MASK(REG_CKDT_RST, 0, HDMI_MACRO_SW_RST);
	msleep(1);
	HDMIRX_WRITE32_MASK(REG_CKDT_RST, 0x1<<9, HDMI_MACRO_SW_RST);
	msleep(1);
	HDMIRX_WRITE32_MASK(REG_SRST, 0x1<<8,SW_RST);//Reset
	msleep(1);
	HDMIRX_WRITE32_MASK(REG_SRST, 0,  SW_RST);
}

void bHDMIPHYReset(UINT8 u1ResetSel)
{
    printk("[%s]\n",__FUNCTION__);
    if (u1ResetSel == HDMI_RST_ALL)
    {
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON00,RG_HDMI_CDR_RST,RG_HDMI_CDR_RST);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON00,0,RG_HDMI_CDR_STOP);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON00,0,RG_HDMI_CDR_RST);
		udelay(10);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,RG_HDMI_EQ_RST,RG_HDMI_EQ_RST);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,RG_HDMI_EQ_SWRSTSEL,RG_HDMI_EQ_SWRSTSEL);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,0,RG_HDMI_EQ_RST);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,0,RG_HDMI_EQ_SWRSTSEL);
	
		HDMIRX_WRITE32_MASK(REG_HDMI_DIG_MACRO,C_DATA_SYNC_AUTO,C_DATA_SYNC_AUTO);
		HDMIRX_WRITE32_MASK(REG_HDMI_DIG_MACRO,0,C_DATA_SYNC_AUTO);
	
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,0,RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,0,RG_HDMI_0_DEEPCLRCLK_RSTN);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,RG_HDMI_0_DEEPCLRCLK_PDB,RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,RG_HDMI_0_DEEPCLRCLK_RSTN,RG_HDMI_0_DEEPCLRCLK_RSTN);
    }
    if(u1ResetSel == HDMI_RST_EQ)
    {
        HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,RG_HDMI_EQ_RST,RG_HDMI_EQ_RST);
        HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,RG_HDMI_EQ_SWRSTSEL,RG_HDMI_EQ_SWRSTSEL);
        HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,0,RG_HDMI_EQ_RST);
        HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON01,0,RG_HDMI_EQ_SWRSTSEL);
    }
    if (u1ResetSel == HDMI_RST_DEEPCOLOR)
    {
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,0,RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,0,RG_HDMI_0_DEEPCLRCLK_RSTN);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,RG_HDMI_0_DEEPCLRCLK_PDB,RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMIRX_A_WRITE32_MASK(REG_HDMI_RX_CON02,RG_HDMI_0_DEEPCLRCLK_RSTN,RG_HDMI_0_DEEPCLRCLK_RSTN);
    }
    if (u1ResetSel == HDMI_RST_FIXEQ)
    {
        //reset FIXEQ
    }
    if (u1ResetSel == HDMI_RST_RTCK)
    {
        //reset RTCK
        HDMIRX_WRITE32_MASK(REG_HDMI_DIG_MACRO, 0x1<<15, C_DATA_SYNC_AUTO);
        HDMIRX_WRITE32_MASK(REG_HDMI_DIG_MACRO, 0x0, C_DATA_SYNC_AUTO);

    }

    return ;
}


//Reset TMDS-PHY sphtrack block
void vHalResetHdmiRxPhySPModule(void)
{

}

void vHalSwResetHdmiRxModule(void)
{
    HDMIRX_WRITE32_MASK(REG_SRST, 0x1<<8, SW_RST);
    HDMIRX_WRITE32_MASK(REG_SRST, 0x0, SW_RST);
    
    return ;
}

void vHalMuteHdmiRxAudioOut(void)
{
    //Mute audio channel, ch0~3
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x1<<16, CH0_MUTE);
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x1<<17, CH1_MUTE);
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x1<<18, CH2_MUTE);
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x1<<19, CH3_MUTE);

}

void vHalUnMuteHdmiRxAudioOut(void)
{
    //unMute audio channel, ch0~3
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x0, CH0_MUTE);
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x0, CH1_MUTE);
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x0, CH2_MUTE);
    HDMIRX_WRITE32_MASK(REG_CHST1, 0x0, CH3_MUTE);
}

void vHalOpenAPLL(void)
{
    HDMIRX_WRITE32_MASK(REG_PD_SYS, 0x1<<29, PD_APLL);
    HDMIRX_WRITE32_MASK(REG_PD_SYS, 0x1<<31, PD_AO);
}

void vHalSetRxI2sLRInv(UINT8 u1LRInv)
{
    if (u1LRInv)
    {
        HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x1<<19, WS);
    }
    else
    {
        HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x0, WS);
    }

}


void vHalSetRxI2sAudFormat(UINT8 u1fmt, UINT8 u1Cycle)
{
	if (u1fmt == FORMAT_RJ)
	{
		// Right-Justified
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x1<<28, JUSTIFY); 
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x1<<16, FIRST_BIT);
	}
	else if (u1fmt == FORMAT_LJ)
	{
		// Left Justified
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x0, JUSTIFY);
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x1<<16, FIRST_BIT);
	}
	else if (u1fmt == FORMAT_I2S)
	{
		// I2S
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x0, JUSTIFY);
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x0, FIRST_BIT);
	}

	if (u1Cycle == LRCK_CYC_16)
	{
		//16 cycle
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x1<<21, SIZE);
	}
	else if (u1Cycle == LRCK_CYC_32)
	{
		//32 cycle
		HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x0, SIZE);
	}
	
	return;
}


void vHalSetLRCKEdge(UINT8 u1EdgeFmt)
{
    if(u1EdgeFmt)
        HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x1<<22, CLK_EDGE);
    else
        HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x0, CLK_EDGE);

}

void vHalSetRxI2sMclk(UINT8 u1MclkType)
{
	if (u1MclkType == MCLK_128FS)
	{
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x0<<20, FM_VAL_SW);
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x0<<22, FM_IN_VAL_SW);
	}
	else if (u1MclkType == MCLK_256FS)
	{
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<20, FM_VAL_SW);
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<22, FM_IN_VAL_SW);
	}
	else if (u1MclkType == MCLK_384FS)
	{
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x2<<20, FM_VAL_SW);
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x2<<22, FM_IN_VAL_SW);
	}
	else if (u1MclkType == MCLK_512FS)
	{
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x3<<20, FM_VAL_SW);
		HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x3<<22, FM_IN_VAL_SW);
	}
	else
	{
		
	}

	return ;
}


 void vHalSetRxAudioFS(enum HDMI_RX_AUDIO_FS eFS)
 {
     switch (eFS)
     {
    case SW_44p1K:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x0<<16, FS_VAL_SW);
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<1, FS_HW_SW_SEL);
        break;

    case SW_88p2K:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x8<<16, FS_VAL_SW);
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<1, FS_HW_SW_SEL);
        break;

    case SW_176p4K:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0xc<<16, FS_VAL_SW);
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<1, FS_HW_SW_SEL);
        break;

    case SW_48K:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x2<<16, FS_VAL_SW);
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<1, FS_HW_SW_SEL);
        break;

    case SW_96K:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0xa<<16, FS_VAL_SW);
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<1, FS_HW_SW_SEL);
        break;

    case SW_192K:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0xe<<16, FS_VAL_SW);
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<1, FS_HW_SW_SEL);
        break;

    case SW_32K:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x3<<16, FS_VAL_SW);
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x1<<1, FS_HW_SW_SEL);
        break;

    case HW_FS:
        HDMIRX_WRITE32_MASK(REG_ACR_CTRL1, 0x0, FS_HW_SW_SEL);
        break;

    default:
        break;
    }
 }

 UINT8 u1HalGetRxI2sMclk(void)
 {
	 UINT32 u4Value = 0;
	 UINT8	u1MclkType=MCLK_128FS;
 
	 u4Value = (HDMIRX_READ32(REG_ACR_CTRL1) & FM_VAL_SW)>>20;
	 switch(u4Value)
	 {
	 case 0x0:
		 u1MclkType = MCLK_128FS;
		 break;
		 
	 case 0x1:
		 u1MclkType = MCLK_256FS;
		 break;
		 
	 case 0x2:
		 u1MclkType = MCLK_384FS;
		 break;
		 
	 case 0x3:
		 u1MclkType = MCLK_512FS;
		 break;
		 
	 default:
		 break;
	 }
 
	 return u1MclkType;
 }


void vHalEnableRxAudClk(void)
{
    HDMIRX_WRITE32_MASK(REG_AUDRX_CTRL, 0x1<<10, I2S_MODE);
    HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 
        SD2_EN | SD1_EN | SD0_EN | MCLK_EN, 
        SD2_EN | SD1_EN | SD0_EN | MCLK_EN);
    HDMIRX_WRITE32_MASK(REG_I2S_CTRL, 0x0, PCM);

    return ;
}

void vHalSetRxAudMuteCondition(void)
{
  // exception : audio fifo underrun/ audio fifo overrun / audio sampling freq. change
	HDMIRX_WRITE32_MASK(REG_AEC_CTRL, 0x16<<24, EXP_EN_15_8);
}

void vHalEnableRxAACToSd0Sd1Sd2Sd3(void)
{
    HDMIRX_WRITE32_MASK(REG_AEC_CTRL, 0x1<<13, AAC_OUT_OFF_EN);
    return ;
}

UINT8 u1HalChkDataEnableExist(void)//check Sync
{
    if(HDMIRX_READ32(REG_SRST) & SCDT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINT8 u1HalChkCKDTExist(void)//check Sync
{
    if(HDMIRX_READ32(REG_SRST) & CKDT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINT8 u1HalChkPwr5VExist(void)//check +5V
{

	UINT8 u1Exit = 0;
#ifdef GPIO_HDMI_RX_POWER_5V_STATUS_PIN
	mt_set_gpio_mode(GPIO_HDMI_RX_POWER_5V_STATUS_PIN, GPIO_MODE_00);  
    mt_set_gpio_dir(GPIO_HDMI_RX_POWER_5V_STATUS_PIN, GPIO_DIR_IN);
	u1Exit = mt_get_gpio_in(GPIO_HDMI_RX_POWER_5V_STATUS_PIN);
#endif
    return u1Exit;
}

void vHalRxSwitchPortSelect(BYTE bPort)
{
    if(bPort == 0)//standby,Both HPD1,2 same to HPD
    {
		vHalRxHDMITMDSCTRL(FALSE);
	}
	else
	{
		vHalRxHDMITMDSCTRL(TRUE);
	}
}

void vHalSelANABand(void)
{

	UINT32 u4CKPRD;
	
	
	// ICG setting part
	vRxWriteReg(0xC000100,0x00000000);
	vRxWriteReg(0xC000110,0x00000000);
	vRxWriteReg(0xC000104,0x00000000);

	//HDMI Setting
	vRxWriteReg(0xC00001C,0xfb5aab4f);
	vRxWriteReg(0xC000020,0x00000000);
	vRxWriteReg(0xC000024,0x00000001);
	vRxWriteReg(0xC0062EC,0x00132000);
	vRxWriteReg(0xC0062EC,0x00102000);
	vRxWriteReg(0x0209164,0x0000000E);
	u4CKPRD = (vRxReadReg(0xC00622C)>>12)&0xFF;
	
	if((26*64) <= (u4CKPRD*(27+3)))  // 27MHz
	{
        if(_u4CKPDRDOLD == 1)
            return;
        _u4CKPDRDOLD = 1;
		printk("[HDMI_RX] <=27MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x2B217612);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00B13F30);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00000002);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x060A0000);
		msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00000802);
		HDMIRX_A_WRITE32(0x158,0x3B217612);
		msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00A4A);
		HDMIRX_A_WRITE32(0x158,0x3B217613);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55200A4A);
		msleep(1);
	}
	else if((26*64) <= (u4CKPRD*(40+5)))
	{
        if(_u4CKPDRDOLD == 2)
            return;
        _u4CKPDRDOLD = 2;
		printk("[HDMI_RX] <=40MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x2B217612);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00914F30);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x0000000A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x060A0000);
		msleep(1);
		HDMIRX_A_WRITE32(0x160,0x0000080A);
		HDMIRX_A_WRITE32(0x158,0x3B217612);
		msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00A4A);
		HDMIRX_A_WRITE32(0x158,0x3B217613);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55200A4A);
		msleep(1);
	}
	else if((26*64) <= (u4CKPRD*60))
	{
        if(_u4CKPDRDOLD == 3)
            return;
        _u4CKPDRDOLD = 2;
		printk("[HDMI_RX] <=60MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x2B217412);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00914F30);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x0000000A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x060A0000);
		msleep(1);
		HDMIRX_A_WRITE32(0x160,0x0000080A);
		HDMIRX_A_WRITE32(0x158,0x3B217412);
		msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00A4A);
		HDMIRX_A_WRITE32(0x158,0x3B217413);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55200A4A);
		msleep(1);
	}
	else if((26*64) <= (u4CKPRD*(75+2)))
	{
        if(_u4CKPDRDOLD == 4)
            return;
        _u4CKPDRDOLD = 4;
		printk("[HDMI_RX] <=75MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x2B217412);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00814C30);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00000008);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x06020000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00000808);
		HDMIRX_A_WRITE32(0x158,0x3B217412);
		//msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00A4A);
		HDMIRX_A_WRITE32(0x158,0x3B217413);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55200A4A);
		//msleep(1);
	}
	else if((26*64) <= (u4CKPRD*(110+4)))
	{
        if(_u4CKPDRDOLD == 5)
            return;
        _u4CKPDRDOLD = 5;
		printk("[HDMI_RX] <=110MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x2B217412);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00868630);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00010008);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x06020000);
		msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00001808);
		HDMIRX_A_WRITE32(0x158,0x3B217412);
		msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55A00A4A);
		HDMIRX_A_WRITE32(0x158,0x3B217413);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0x55200A4A);
		msleep(1);
	}
	else if((26*64) <= (u4CKPRD*160))
	{
        if(_u4CKPDRDOLD == 6)
            return;
        _u4CKPDRDOLD = 6;
		printk("[HDMI_RX] <=160MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x2B217412);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00845630);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00010008);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x06120000);
		msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00001808);
		HDMIRX_A_WRITE32(0x158,0x3B217412);
		msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5A00A4A);
		HDMIRX_A_WRITE32(0x158,0x3B217413);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5200A4A);
		msleep(1);
	}
	else if((26*64) <= (u4CKPRD*(250)))
	{
        if(_u4CKPDRDOLD == 7)
            return;
        _u4CKPDRDOLD = 7;
		printk("[HDMI_RX] <=250MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x2B215012);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00822630);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00002009);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x06320000);
		msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00002809);
		HDMIRX_A_WRITE32(0x158,0x3B215012);
		msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5A00A4A);
		HDMIRX_A_WRITE32(0x158,0x3B215013);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5200A4A);
		msleep(1);
	}
	else
	{
        if(_u4CKPDRDOLD == 8)
            return;
        _u4CKPDRDOLD = 8;
		printk("[HDMI_RX] >250MHz \n");
		HDMIRX_A_WRITE32(0x150,0x80000000);
		//msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5A00E4A);
		//msleep(1);
		HDMIRX_A_WRITE32(0x158,0x29214012);
		//msleep(1);
		HDMIRX_A_WRITE32(0x15C,0x00822630);
		//msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00002009);
		//msleep(1);
		HDMIRX_A_WRITE32(0x168,0x00000004);
		//msleep(1);
		HDMIRX_A_WRITE32(0x16C,0x06320000);
		msleep(1);
		HDMIRX_A_WRITE32(0x160,0x00002809);
		HDMIRX_A_WRITE32(0x158,0x39214012);
		msleep(1);
		HDMIRX_A_WRITE32(0x150,0x00000000);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5A00A4A);
		HDMIRX_A_WRITE32(0x158,0x39214013);
		msleep(1);
		HDMIRX_A_WRITE32(0x154,0xD5200A4A);
		msleep(1);
	}

	//digital part
	vRxWriteReg(0xC006038,0x00002280);
	vRxWriteReg(0xC006128,0x00001FE4);
	vRxWriteReg(0xC006124,0xF1400000);
	vRxWriteReg(0xC00613C,0xC0060001);
	vRxWriteReg(0xC006134,0x00020600);
	vRxWriteReg(0xC006008,0x00001407);
	vRxWriteReg(0xC006014,0x865E01EA);
	vRxWriteReg(0xC006280,0x000003A0);
	vRxWriteReg(0xC006284,0x00000000);
	vRxWriteRegMsk(0xC00606C,(1<<22),(1<<22));
	vRxWriteRegMsk(0xC00606C,(1<<23),(1<<23));
	vRxWriteReg(0xC0060FC,0x00000000);
	vRxWriteReg(0xC0060F8,0x0000ff00);
	vRxWriteReg(0xC006048,0x000a0000);


	vRxWriteRegMsk(0xC006088,(1<<12)|(1<<13),(1<<12)|(1<<13)); //for audio unflow,overflow
	vRxWriteRegMsk(0xC006088,0,(1<<14));
	vRxWriteReg(0xC006100,0x00000000); //audio clk auto mode
	vRxWriteRegMsk(0xC006260,(1<<8),(1<<8));

}


UINT32 u4HalGetRxPixelClock(void)
{
    UINT32 u4XclkInPclk = 0;

    UINT32 u4Value_h = 0;
    UINT32 u4Value_l = 0;

    u4Value_h = (HDMIRX_READ32(REG_VID_CRC_OUT) & AAC_XCLK_IN_PCLK_10_8) >> 16;
    u4Value_l = (HDMIRX_READ32(REG_VID_CRC_OUT) & AAC_XCLK_IN_PCLK_7_0) >> 24;
    
    u4XclkInPclk = (u4Value_h<<8) | u4Value_l;
    u4XclkInPclk = u4XclkInPclk + 1;
    return u4XclkInPclk;
}

void vHalClearPclkChangedIntState(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, INTR2_CLK_CHG);
}

void vHalClearRxPclkChgStatus(void)//Clear Pixel clock change interrupt status bit
{
	HDMIRX_WRITE32(REG_INTR_STATE0,0x1<<16);
}


UINT8 vHalGetRxHdcpStatus(void)
{
    UINT32 u4Status = 0;

    u4Status = (HDMIRX_READ32(REG_HDCP_STAT) & (HDCP_DECRYPT|HDCP_AUTH))>>20;

    return (UINT8)u4Status;
}

BOOL fgHalCheckHdmiRXAuthDone(UINT8 u1Data)
{
    if((u1Data & 0x30)==0x30)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalCheckAviInforFrameExist(void)
{
    UINT32 u4Header = 0;
    u4Header = HDMIRX_READ32(REG_AVIRX0) & CEA_AVI_HEADER;
    
    if((u4Header & 0xff) == 0x82)
        return TRUE;
    else
        return FALSE;

}

UINT8 u1HalReadAviType(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX1) & 0x000000FF) >> 0;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAviVersion(void)
{
    UINT32 u4Version = 0;

    u4Version = HDMIRX_READ32(REG_AVIRX0) & CEA_AVI_HEADER;
    u4Version = (u4Version & 0xFF00) >> 8;

    return (UINT8)u4Version;

}

UINT8 u1HalReadAviLength(void)
{
    UINT32 u4Length = 0;
    u4Length = (HDMIRX_READ32(REG_AVIRX0) & CEA_AVI_LENGTH) >> 16;

    return (UINT8)u4Length;

}

UINT8 u1HalReadAviCheckSum(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX0) & CEA_AVI_CHECKSUM) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte1(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX1) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte2(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX1) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte3(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX1) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte4(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX1) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte5(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX2) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte6(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX2) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte7(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX2) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte8(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX2) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte9(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX3) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte10(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX3) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte11(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX3) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}
UINT8 u1HalReadAviByte12(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX3) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte13(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX4) & 0x000000FF) >> 0;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAviByte14(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX4) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAviByte15(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AVIRX4) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfType(void)
{
    UINT32 u4Value = 0;

    u4Value = (HDMIRX_READ32(REG_AUDRX0) & CEA_AUD_HEADER_7_0) >> 0;
    u4Value &= 0x00FF;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfVersion(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX0) & CEA_AUD_HEADER_15_8) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfLength(void)
{
    UINT32 u4Value = 0;

    u4Value = (HDMIRX_READ32(REG_AUDRX0) & CEA_AUD_LENGTH)>>16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfCheckSum(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX0) & CEA_AUD_CHECKSUM) >> 24;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAudioInfByte1(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX1) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}


UINT8 u1HalReadAudioInfByte2(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX1) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfByte3(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX1) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAudioInfByte4(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX1) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfByte5(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX2) & 0x000000FF) >> 0;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAudioInfByte6(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX2) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAudioInfByte7(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX2) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfByte8(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX2) & 0xFF000000) >> 24;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAudioInfByte9(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX3) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAudioInfByte10(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_AUDRX3) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAcpHb0Header(void)//ACP Packet Type =0x04
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX0) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadHb1HeaderAcpType(void)//HB1 ACP Type: 0:Geberic Audio 1:IEC 60958 Idetified 2:DVD audio, 3:DSD audio
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX0) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;
}


UINT8 u1HalReadAcpHb2Header(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX0) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}


UINT8 u1HalReadAcpPB0(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX0) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}


UINT8 u1HalReadAcpPB1(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX1) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}


UINT8 u1HalReadAcpPB2(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX1) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB3(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX1) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB4(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX1) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB5(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX2) & 0x000000FF) >> 0;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAcpPB6(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX2) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB7(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX2) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB8(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX2) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB9(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX3) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB10(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX3) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB11(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX3) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAcpPB12(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX3) & 0xFF000000) >> 24;

    return (UINT8)u4Value;
}

UINT8 u1HalReadAcpPB13(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX4) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB14(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX4) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadAcpPB15(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_ACPRX4) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}


UINT8 u1HalReadSPDType(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX0) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDVersion(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX0) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDLength(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX0) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}


UINT8 u1HalReadSPDByte1(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX0) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte2(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX1) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte3(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX1) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte4(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX1) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte5(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX1) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte6(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX2) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte7(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX2) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte8(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX2) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}


UINT8 u1HalReadSPDByte9(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX2) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte10(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX3) & 0x000000FF) >> 0;

    return (UINT8)u4Value;
}

UINT8 u1HalReadSPDByte11(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX3) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;
}

UINT8 u1HalReadSPDByte12(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX3) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte13(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX3) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte14(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX4) & 0x000000FF) >> 0;

    return (UINT8)u4Value;
}

UINT8 u1HalReadSPDByte15(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX4) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte16(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX4) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte17(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX4) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte18(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX5) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte19(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX5) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte20(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX5) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte21(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX5) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte22(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX6) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte23(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX6) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte24(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX6) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte25(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX6) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadSPDByte26(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_SPDRX7) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutHb0(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX0) & 0x000000FF) >> 0;

    return (UINT8)u4Value;
}

UINT8 u1HalReadGamutHb1(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX0) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutHb2(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX0) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}


UINT8 u1HalReadGamutPB0(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX0) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB1(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX1) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB2(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX1) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB3(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX1) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB4(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX1) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB5(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX2) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB6(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX2) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB7(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX2) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB8(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX2) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB9(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX3) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB10(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX3) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB11(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX3) & 0x00FF0000) >> 16;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB12(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX3) & 0xFF000000) >> 24;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB13(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX4) & 0x000000FF) >> 0;

    return (UINT8)u4Value;

}

UINT8 u1HalReadGamutPB14(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_GAMUTRX4) & 0x0000FF00) >> 8;

    return (UINT8)u4Value;

}

void vHalClearNewAviIntStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, INTR3_NEW_AVI);
}

void vHalClearNewAudIntStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, INTR3_NEW_AUD);
}

void vHalClearNewSpdIntStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, INTR3_NEW_SPD);
}

void vHalClearNewMpegIntStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, INTR3_NEW_MPEG);
}

void vHalClearNewUnRecIntStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE1, INTR6_NEW_ACP);
}

void vHalClearNewAcpIntStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE1, INTR6_NEW_ACP);
}

void vHalClearNewVSIntStatus(void)
{
    UINT32 u4Mask = 0;

    // set 1 to clear interrupt, but MASK value should not be changed.
    u4Mask = HDMIRX_READ32(REG_INTR_VS_ISRC1) |
        INTR_VSYNC_MASK| INTR_NO_VS_PKT_MASK| INTR_NEW_VS_PKT_MASK| INTR_NEW_ISRC1_PKT_MASK;

    HDMIRX_WRITE32(REG_INTR_VS_ISRC1, INTR_NEW_VS_PKT | u4Mask);
    return ;  
}

void vHalClearNewNOVSIntStatus(void)
{
    UINT32 u4Mask = 0;

    // set 1 to clear interrupt, but MASK value should not be changed.
    u4Mask = HDMIRX_READ32(REG_INTR_VS_ISRC1) |
        INTR_VSYNC_MASK| INTR_NO_VS_PKT_MASK| INTR_NEW_VS_PKT_MASK| INTR_NEW_ISRC1_PKT_MASK;

    HDMIRX_WRITE32(REG_INTR_VS_ISRC1, INTR_NO_VS_PKT | u4Mask);
    return ;  
}

void vHalClearNewISRC1IntStatus(void)
{
    UINT32 u4Mask = 0;

    // set 1 to clear interrupt, but MASK value should not be changed.
    u4Mask = HDMIRX_READ32(REG_INTR_VS_ISRC1) |
        INTR_VSYNC_MASK| INTR_NO_VS_PKT_MASK| INTR_NEW_VS_PKT_MASK| INTR_NEW_ISRC1_PKT_MASK;

    HDMIRX_WRITE32(REG_INTR_VS_ISRC1, INTR_NEW_ISRC1_PKT | u4Mask);
    return ;  
}

void vHalClearVSYNCIntStatus(void)
{
	UINT32 u4Mask;

	u4Mask = HDMIRX_READ32(REG_INTR_VS_ISRC1) |INTR_VSYNC_MASK| INTR_NO_VS_PKT_MASK| INTR_NEW_VS_PKT_MASK| INTR_NEW_ISRC1_PKT_MASK;
	HDMIRX_WRITE32(REG_INTR_VS_ISRC1, INTR_NEW_VS_PKT | u4Mask);

}

void vHalSetVSNewOnly(BOOL fgEnable)
{

	if(fgEnable)
		HDMIRX_WRITE32_MASK(REG_ISRC1RX0, 0x1<<1, REG_NEW_VS_ONLY); 
	else
		HDMIRX_WRITE32_MASK(REG_ISRC1RX0, 0x0, REG_NEW_VS_ONLY);

	return ;

}

void vHalSetISRC1NewOnly(BOOL fgEnable)
{

	if(fgEnable)
		HDMIRX_WRITE32_MASK(REG_ISRC1RX0, 0x1<<0, REG_NEW_ISRC1_ONLY);
	else
		HDMIRX_WRITE32_MASK(REG_ISRC1RX0, 0x0, REG_NEW_ISRC1_ONLY);

	return ;

}

BOOL vHalCheckIsPclkChanged(void)
{

  if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_CLK_CHG)
	  return TRUE;
  else
	  return FALSE;

}

BOOL vHalCheckRxIsHdmiMode(void)
{
    if(HDMIRX_READ32(REG_AUDP_STAT) & HDMI_MODE_DET)
        return TRUE; //hdmi mode
    else
        return FALSE; //dvi mode
}

BOOL vHalCheckGcpMuteEnable(void)
{
	if(HDMIRX_READ32(REG_AUDP_STAT) & HDMI_MUTE)
		return TRUE; 
	else
		return FALSE;

}

BOOL fgHalCheckAcpInforFrameExist(void)
{
	BOOL fgExit= FALSE;

	if((u1RegRead1B(HDMIRX_REG_BASE+REG_ACPRX0))==0x4)
		fgExit = TRUE;
	else
		fgExit = FALSE;

	return fgExit;

}

UINT8 u1HalRxGetAcpType(void)
{
  UINT8 u1Data=0;
  u1Data = u1RegRead1B(HDMIRX_REG_BASE+REG_ACPRX0);
  return u1Data;
}

BOOL fgHalCheckIsNewAcp(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_NEW_ACP)
        return TRUE; 
    else
        return FALSE; 


}

BYTE vHalGetAcpPacketHeader(void)
{
	return(u1RegRead1B(HDMIRX_REG_BASE+REG_ACPRX0));
}

void vHalGetAcpPacket(BYTE *bAcpPacketData)
{
	BYTE i;
	for(i=0; i<31; i++)
	{
		*(bAcpPacketData+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_ACPRX0)+i);
	}
}

void vSetSelectAcppacket(BYTE bHeader)
{
	HDMIRX_WRITE32_MASK(REG_ACPRX7,bHeader,ACP_DEC);
}


void fgHalClearAcpIntStatus(void)
{
  HDMIRX_WRITE32_MASK(REG_INTR_STATE1,0,INTR6_NEW_ACP);
}

void vHalGetAviInfoframe(BYTE *bAviinfoframe)
{
  BYTE i;
  for(i=0; i<19; i++)
  {
	*(bAviinfoframe+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_AVIRX0)+i);
  }
}

void vHalGetAudioInfoframe(BYTE *bAudioInfoframe)
{
	BYTE i;
	for(i=0; i<14; i++)
	{
		*(bAudioInfoframe+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_AUDRX0)+i);
	}
}

void vHalGetVSInfoframe(BYTE *bVSinfoframe)
{
    BYTE i;
    for(i=0; i<31; i++)
    {
        *(bVSinfoframe+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_VSRX0)+i);
    }
}

void vHalGetISRC1Infoframe(BYTE *bISRC1infoframe)
{
    BYTE i;
    for(i=0; i<18; i++)
    {
        *(bISRC1infoframe+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_ISRC1RX0)+i);
    }
}

void vHalGetMpegInfoframe(BYTE *bMpegInfoframeData)
{
	BYTE i;
	for(i=0; i<31; i++)
	{
		*(bMpegInfoframeData+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_MPEGRX0)+i);
	}
}

BYTE vHalGetMpegAddrHeader(void)
{
	return (u1RegRead1B(HDMIRX_REG_BASE+REG_MPEGRX0));
}

void vHalMpegAddrSetSelectPacket(BYTE bHeader)
{
    HDMIRX_WRITE32_MASK(REG_MPEGRX7, bHeader<<24, MPEG_DEC);
}

BYTE vHalMpegAddrGetSelectPacket(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_MPEGRX7) & MPEG_DEC) >> 24;

    return (UINT8)u4Value;
}


void vHalGetSpdInfoframe(BYTE *bSpdInfoframeData)
{
	BYTE i;
	for(i=0; i<31; i++)
	{
		*(bSpdInfoframeData+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_SPDRX0)+i);
	}
}

void vHalGetGamutPacket(BYTE *bGamutData)
{
	BYTE i;
	for(i=0; i<31; i++)
	{
		*(bGamutData+i) = u1RegRead1B((HDMIRX_REG_BASE+REG_GAMUTRX0)+i);
	}
}

UINT32 u4HalGetRxHwCTSValue(void)
{
    UINT32 u4Value  = 0;
    u4Value = (HDMIRX_READ32(REG_CTS_HVAL) & CTS_VAL_HW_19_0) >> 0;

    return u4Value ;
}

UINT32 u4HalGetRxHwNValue(void)
{
    UINT32 u4Value  = 0;
    u4Value = (((HDMIRX_READ32(REG_N_HVAL) & N_VAL_HW_19_16) >> 0) >> 16)  || 
        ((HDMIRX_READ32(REG_N_SVAL) & N_VAL_HW_15_0) >> 16);

    return u4Value ;

}

BOOL fgHalCheckIsNonDeepColorMode(void)
{
	UINT32 u4Mode = 0;

    u4Mode = (HDMIRX_READ32(REG_SRST) & DEEP_STA) >> 28;
    switch(u4Mode)
    {
    case 0: // 24bit mode
        return TRUE;
        break;
        
    case 1: //30bit mode
        return FALSE;
        break;
        
    case 2: //36bit mode
        return FALSE;
        break;

    case 3: //48bit mode
        return FALSE;
        break;
        
    default:
        break;
    }
    
    return FALSE;
}

UINT8 u4HalCheckDeepColorMode(void)
{
    UINT32 u4Mode = 0;
    u4Mode = (HDMIRX_READ32(REG_SRST) & DEEP_STA) >> 28;

    return u4Mode;
}

void vHalReInitAudioClock(void)
{

}

BOOL fgHalCheckIsNoAvi(void)
{
	if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_NO_AVI)
		return TRUE;
	else
		return FALSE;
}

void vHalClearVideoModeByte0(void)
{
    HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, 0xff<<0);
}

void vHalClearVideoModeByte1(void)
{
    HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, 0xff<<8);
}

void vHalClearVideoModeByte2(void)
{
	HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, 0xff<<16);
}

void vHalClearVideoModeByte3(void)
{
	HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, 0xFF<<24);
}

void vHalClearIntrState1Bit0_Bit7(void)
{
    UINT32 u4Value = 0;
    u4Value = HDMIRX_READ32(REG_INTR_STATE1) & 0x000000FF;
    HDMIRX_WRITE32(REG_INTR_STATE1, u4Value);

}

void vHalRxDisable656SyncMode(void)
{
    HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, ENSYNCCODES);
}

void vHalRxDisable422to444UpSample(void)
{
    HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, ENUPSAMPLE);
}


void vHalSetRxRGBBlankValue(UINT8 u1Blue , UINT8 u1Green, UINT8 u1Red)
{
    HDMIRX_WRITE32_MASK(REG_VID_MODE, u1Blue<<24, BLANKDATA1);
    HDMIRX_WRITE32_MASK(REG_VID_BLANK, u1Green<<0, BLANKDATA2);
    HDMIRX_WRITE32_MASK(REG_VID_BLANK, u1Red<<8, BLANKDATA3);

}

void vHalSetRxYCbCrBlankValue(UINT8 u1Cb , UINT8 u1Y, UINT8 u1Cr)
{
    HDMIRX_WRITE32_MASK(REG_VID_MODE, u1Cb<<24, BLANKDATA1);
    HDMIRX_WRITE32_MASK(REG_VID_BLANK, u1Y<<0, BLANKDATA2);
    HDMIRX_WRITE32_MASK(REG_VID_BLANK, u1Cr<<8, BLANKDATA3);

}

void vHalSetRxPclk(UINT8 u1Data)
{
	HDMIRX_WRITE32(REG_SYS_CTRL, u1Data);
}

BOOL fgHalCheckRxPclkIs2XRepeat(void)
{
    if((HDMIRX_READ32(REG_SYS_CTRL) & IDCK)>>4 == 0x1)
        return TRUE;
    else
        return FALSE;
}

void vHalWriteVideoChMap(UINT8 u1Data)
{
    HDMIRX_WRITE32_MASK(REG_VID_CH_MAP, u1Data<<16, CHANNEL_MAP);
}

BOOL fgHalCheckRxHResChg(void)
{
	if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_HRESCHG)
		return TRUE;
	else
		return FALSE;

}
void vHalRxResetTDFifoAutoRead(void)
{
    HDMIRX_WRITE32_MASK(REG_MUTE_SET, TDFIFO_SYNC_EN, TDFIFO_SYNC_EN);
    HDMIRX_WRITE32_MASK(REG_MUTE_SET, 0, TDFIFO_SYNC_EN);
    return ;
}

void vHalClearRxHresChgIntState(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE1, INTR5_HRESCHG);
}

void vHalRxEnableTDFifoAutoRead(void)
{
    HDMIRX_WRITE32_MASK(REG_MUTE_SET, 0x1<<20, TDFIFO_SYNC_EN);
}

void vHalRxDisableTDFifoAutoRead(void)
{
	HDMIRX_WRITE32_MASK(REG_MUTE_SET, 0x0, TDFIFO_SYNC_EN);
}

UINT32 HDMI_HalGetHFrontPorch(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_VID_HFP) & VID_HFRONTPORCH) >> 8;
    
    return u4Value;
}

UINT32 vHalRxGetActiveWidth(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_VID_BLANK) & VID_PIXELS) >> 16;

    return u4Value;
}

UINT32 HDMI_HalGetHSyncWidth(void)
{
    UINT32 u4ValueH = 0;
    UINT32 u4ValueL = 0;
    u4ValueH = (HDMIRX_READ32(REG_VID_AOF) & VID_HSACTIVEWIDTH_9_8) >> 0;
    u4ValueL = (HDMIRX_READ32(REG_VID_HFP) & VID_HSACTIVEWIDTH_7_0) >> 24;
    
    return ((u4ValueH << 8) | u4ValueL);
}

UINT32 vHalRxGetVFrontPorch(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_VID_STAT) & VFRONTPORCH) >> 24;

    return u4Value;
}

UINT32 vHalRxGetVBackPorch(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_VID_STAT) & V2ACTIVELINES) >> 16;

    return u4Value;
}

UINT32 vHalRxGetActiveHeight(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_VID_STAT) & VID_DELINES) >> 0;

    return u4Value;
}

UINT32 vHalRxGetHTotal(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_VID_HRES) & VID_HRES_12_0) >> 16;

    return u4Value;
}

UINT32 vHalRxGetVTotal(void)
{
    UINT32 u4Value = 0;
    u4Value = (HDMIRX_READ32(REG_VID_VRES) & VID_VRES_11_0) >> 0;

    return u4Value;
}

BOOL fgHalCheckRXHdcpDecrptOn(void)
{
    if(HDMIRX_READ32(REG_HDCP_STAT) & HDCP_DECRYPT)
        return TRUE;
    else
        return FALSE;
}

BOOL fgHalCheckRXH4096DetectOn(void)
{
  if(HDMIRX_READ32(REG_VID_STAT) & 0x1000)
	  return TRUE;
  else
	  return FALSE;
}

BOOL fgHalChkSCDTEnable(void)//check Sync
{
    if(HDMIRX_READ32(REG_SRST) & SCDT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void vHalClearModeChgIntState(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, 
        INTR2_HDMI_MODE| INTR2_CKDT |INTR2_SCDT |INTR2_CLK_CHG);
    HDMIRX_WRITE32(REG_INTR_STATE1, 
        INTR5_VRESCHG| INTR5_HRESCHG| INTR5_POLCHG| INTR5_INTERLACEOUT);
    
    return ;
}

BOOL fgHalCheckRxIsVResStable(void)
{
    if(HDMIRX_READ32(REG_VID_VRES) & VID_VRES_STB)
        return TRUE;
    else
        return FALSE;
}

BOOL fgHalCheckRxIsVResMute(void)
{
    if(HDMIRX_READ32(REG_VID_VRES) & VID_VRES_MUTE)
        return TRUE;
    else
        return FALSE;

}

void vHalSetRxVMute(void)
{
    HDMIRX_WRITE32_MASK(REG_VID_VRES, VRES_MUTE_CLR, VRES_MUTE_CLR);
}

void vHalClearRxVMute(void)
{
	HDMIRX_WRITE32_MASK(REG_VID_VRES, 0x0, VRES_MUTE_CLR);
}

void vHalDisableRxAvMute(void)
{
    HDMIRX_WRITE32_MASK(REG_VID_SET, AV_MUTE_CLR, AV_MUTE_CLR);
    return ;
}

void vHalRxEnableAvMuteRecv(void)
{
    HDMIRX_WRITE32_MASK(REG_VID_SET, 0x0, AV_MUTE_CLR);
    return ;

}

BOOL fgHalCheckInterlaceDetect(void)
{
    if(HDMIRX_READ32(REG_VID_CH_MAP) & INTERLACEDOUT)
        return TRUE;
    else
        return FALSE;

}

void vHalSetHDMIRXPowerOff(void)
{
	vRxWriteRegMsk(0x390, (1<<3),(1<<3));
}

//Int Status
BOOL fgHalIsINTR3_CEA_NEW_CP(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_CEA_NEW_CP)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR3_CP_SET_MUTE(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_CP_SET_MUTE)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR3_P_ERR(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_P_ERR)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR3_NEW_UNREC(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_NEW_UNREC)
        return TRUE;
    else
        return FALSE;

}



BOOL fgHalIsINTR3_NEW_MPEG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_NEW_MPEG)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR3_NEW_AUD(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_NEW_AUD)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR3_NEW_SPD(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_NEW_SPD)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR3_NEW_AVI(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR3_NEW_AVI)
        return TRUE;
    else
        return FALSE;

}
BOOL fgHalIsINTR_NEW_VS(void)
{
    if(HDMIRX_READ32(REG_INTR_VS_ISRC1) & INTR_NEW_VS_PKT)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR_VSYNC(void)
{
    if(HDMIRX_READ32(REG_INTR_VS_ISRC1) & INTR_VSYNC)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR_NO_VS(void)
{
    if(HDMIRX_READ32(REG_INTR_VS_ISRC1) & INTR_NO_VS_PKT)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR_NEW_ISRC1(void)
{
    if(HDMIRX_READ32(REG_INTR_VS_ISRC1) & INTR_NEW_ISRC1_PKT)
        return TRUE;
    else
        return FALSE;

}

void vHalHDMIRxEnableVsyncInt(BOOL fgEnable)
{
    if(fgEnable == TRUE)
        HDMIRX_WRITE32_MASK(REG_INTR_VS_ISRC1, 0x1<<3, INTR_VSYNC_MASK);
    else
        HDMIRX_WRITE32_MASK(REG_INTR_VS_ISRC1, 0, INTR_VSYNC_MASK);
}

void  vHalEnableINTR3_Send_AVMUTE(BOOL fgEnable)
{
	if(fgEnable == TRUE)
		HDMIRX_WRITE32_MASK(REG_INTR_MASK0, INTR3_CP_SET_MUTE, INTR3_CP_SET_MUTE);
	else
		HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0, INTR3_CP_SET_MUTE);
}


BOOL fgHalIsINTR2_HDMI_MODE(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_HDMI_MODE)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR2_VSYNC(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_VSYNC)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR2_SOFT_INTR_EN(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_SOFT_INTR_EN)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR2_CKDT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_CKDT)
        return TRUE;
    else
        return FALSE;

}
void  vHalClearINTR2_CKDT(void)
{
    //RegReadFldAlign(INTR_STATE0,INTR2_CKDT) ;
    HDMIRX_WRITE32_MASK(REG_INTR_STATE0, 0x1<<20, INTR2_CKDT);
}


void  vHalEnableINTR2_CKDT(BOOL fgenable)
{
    if(fgenable == TRUE)
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0,INTR2_CKDT, INTR2_CKDT);
    else
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0, INTR2_CKDT);
}


BOOL fgHalIsINTR2_SCDT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_SCDT)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR2_GOT_CTS(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_GOT_CTS)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR2_NEW_AUD_PKT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_NEW_AUD_PKT)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR2_CLK_CHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_CLK_CHG)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR1_HW_CTS_CHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_HW_CTS_CHG)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR1_HW_N_CHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_HW_N_CHG)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR1_PKT_ERR(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_PKT_ERR)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR1_PLL_UNLOCKED(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_PLL_UNLOCKED)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR1_FIFO_ERR(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_FIFO_ERR)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR1_BCH_PKT_ERR_ALERT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_BCH_PKT_ERR_ALERT)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsSOFT_INTR_EN(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & SOFT_INTR_EN)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR_OD(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR_OD)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR_POLARITY(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR_POLARITY)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR_STATE(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR_STATE)
        return TRUE;
    else
        return FALSE;

}

UINT32 u4HalReadINTR_STATE0(void)
{
    return HDMIRX_READ32(REG_INTR_STATE0);
}


UINT32 u4HalReadINTR_STATE1(void)
{
    return HDMIRX_READ32(REG_INTR_STATE1);
}

BOOL fgHalIsINTR7_RATIO_ERROR(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_RATIO_ERROR)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR7_AUD_CH_STAT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_AUD_CH_STAT)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR7_GCP_CD_CHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_GCP_CD_CHG)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR7_GAMUT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_GAMUT)
        return TRUE;
    else
        return FALSE;

}

void fgHalClearGamutIntStatus(void)
{
    HDMIRX_WRITE32_MASK(REG_INTR_STATE1, 0x0, INTR7_GAMUT);
}

BOOL fgHalIsINTR7_HBR(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_HBR)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR7_SACD(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_SACD)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR6_PRE_UNDERUN(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_PRE_UNDERUN)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR6_PRE_OVERUN(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_PRE_OVERUN)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR6_PWR5V_RX2(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_PWR5V_RX2)
        return TRUE;
    else
        return FALSE;
}


BOOL fgHalIsINTR6_PWR5V_RX1(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_PWR5V_RX1)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR6_NEW_ACP(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_NEW_ACP)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR6_P_ERR2(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_P_ERR2)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR6_PWR5V_RX0(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR6_PWR5V_RX0)
        return TRUE;
    else
        return FALSE;
}

BOOL fgHalIsINTR5_FN_CHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_FN_CHG)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR5_AUDIO_MUTE(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_AUDIO_MUTE)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR5_BCH_AUDIO_ALERT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_BCH_AUDIO_ALERT)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR5_VRESCHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_VRESCHG)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR5_HRESCHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_HRESCHG)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR5_POLCHG(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_POLCHG)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR5_INTERLACEOUT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_INTERLACEOUT)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR5_AUD_SAMPLE_F(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_AUD_SAMPLE_F)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR4_PKT_RECEIVED_ALERT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_PKT_RECEIVED_ALERT)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR4_HDCP_PKT_ERR_ALERT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_HDCP_PKT_ERR_ALERT)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR4_T4_PKT_ERR_ALERT(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_T4_PKT_ERR_ALERT)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR4_NO_AVI(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_NO_AVI)
        return TRUE;
    else
        return FALSE;


}

BOOL fgHalIsINTR4_CTS_DROPPED_ERR(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_CTS_DROPPED_ERR)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR4_CTS_REUSED_ERR(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_CTS_REUSED_ERR)
        return TRUE;
    else
        return FALSE;

}

BOOL fgHalIsINTR4_OVERRUN(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_OVERRUN)
        return TRUE;
    else
        return FALSE;

}


BOOL fgHalIsINTR4_UNDERRUN(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR4_UNDERRUN)
        return TRUE;
    else
        return FALSE;

}

void vHalSetIntOnNewAviOnlyEnable(BOOL fgEnable)
{
    if(fgEnable)
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x1<<0, NEW_AVI_ONLY);
    else
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_AVI_ONLY);   
}


void vHalSetIntOnNewAcpOnlyEnable(BOOL fgEnable)
{
    if(fgEnable)
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x1<<5, NEW_ACP_ONLY);
    else
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_ACP_ONLY);  
}


void vHalSetIntOnNewSpdOnlyEnable(BOOL fgEnable)
{
	if(fgEnable)
		HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x1<<1, NEW_SPD_ONLY);
	else
		HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_SPD_ONLY);  
}


void vHalSetIntOnNewAudioInfOnlyEnable(BOOL fgEnable)
{
    if(fgEnable)
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x1<<2, NEW_AUD_ONLY);
    else
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_AUD_ONLY);  
}

void vHalSetIntOnNewMpegInfOnlyEnable(BOOL fgEnable)
{
    if(fgEnable)
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x1<<3, NEW_MPEG_ONLY);
    else
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_MPEG_ONLY);  
}


void vHalSetIntOnNewUnrecInfOnlyEnable(BOOL fgEnable)
{
    if(fgEnable)
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x1<<4, NEW_UNREC_ONLY);
    else
        HDMIRX_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_UNREC_ONLY);  
}

BOOL vHalHdmiRxCrc(INT16 ntry)
{
    //to overwrite this function
    UINT8 idx;
	UINT32 crccnt = 0;
    UINT8 result[3][3];
    UINT8 tmp[3];

    idx = 0;
	crccnt ++;
    result[0][0] = 0;
    result[0][1] = 0;
    result[0][2] = 0;
    result[1][0] = 0;
    result[1][1] = 0;
    result[1][2] = 0;
    result[2][0] = 0;
    result[2][1] = 0;
    result[2][2] = 0;

    _CrcResult[0][0] =0;
    _CrcResult[0][1] =0;
    _CrcResult[0][2] =0;
    _CrcResult[1][0] =0;
    _CrcResult[1][1] =0;
    _CrcResult[1][2] =0;
    _CrcResult[2][0] =0;
    _CrcResult[2][1] =0;
    _CrcResult[2][2] =0;
    tmp[0]=0;
    tmp[1]=0;
    tmp[2]=0;
	
	printk( "fgHDMICRC: %d \r\n", ntry);

	HDMIRX_CLR_BIT(REG_VID_CRC_CHK, 23);
    
	printk( "fgHDMICRC1: %d \r\n", ntry);
    while (ntry > 0)
    {
        ntry--;
        //vUtDelay10ms(1); // NOTE: IT IS NECESSARY
        mdelay(10);
		
        if (idx > 2)
        {
            printk("CRC fail\n");
            _CrcResult[0][0] =result[0][0];
            _CrcResult[0][1] =result[0][1];
            _CrcResult[0][2] =result[0][2];
            _CrcResult[1][0] =result[1][0];
            _CrcResult[1][1] =result[1][1];
            _CrcResult[1][2] =result[1][2];
            _CrcResult[2][0] =tmp[0];
            _CrcResult[2][1] =tmp[1];
            _CrcResult[2][2] =tmp[2];
			printk( "FAIL: %d ,idx= %d\r\n", ntry,idx);
			printk("%x %x %x\r\n", result[0][0], result[0][1], result[0][2]);
			printk("%x %x %x\n", result[1][0], result[1][1], result[1][2]);
            return 0;
        }
        //vRegWrite1B(VID_CRC_CHK_2, 0x8c);// clr
        HDMIRX_WRITE32_MASK(REG_VID_CRC_CHK, (0x8c<<16), (0xff<<16));
		//PRINT_REG(0x22c68);

        while ((HDMIRX_READ32(REG_VID_CRC_CHK)&(1<<23)) != 0x0)
        {
            mdelay(1);

        }
        while (HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff<<24) != 0x00)
        {
            mdelay(1);
        }
        while (HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff)  != 0x00)
        {
            mdelay(1);
        }
        while (HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff<<8) != 0x00)
        {
            
            mdelay(1);
        }

        if (( (HDMIRX_READ32(REG_VID_CRC_CHK))&(0x81<<16)) ==0x0)
        {

            //vRegWrite1B(VID_CRC_CHK_2, 0x0d);// start trigger
            HDMIRX_WRITE32_MASK(REG_VID_CRC_CHK, (0x0d<<16), (0xff<<16));
            //while (u1RegRead1B(VID_CRC_CHK_2)  != 0x8d)
            while (!(HDMIRX_READ32(REG_VID_CRC_CHK)&0x00800000))
            {
                mdelay(1);
            }
			
            //vRegWrite1B(VID_CRC_CHK_2, 0x0c);
			HDMIRX_WRITE32_MASK(REG_VID_CRC_CHK, (0x0c<<16), (0xff<<16));
            //PRINT_REG(0x22c68);
            if (((HDMIRX_READ32(REG_VID_CRC_CHK))&(0x00800000)))
            {
                //HDMI_LOG(HDMI_LOG_DEBUG, "CRC ready\r\n");
                tmp[0] = ((HDMIRX_READ32(REG_VID_CRC_CHK))>>24)&(0xff);
                tmp[1] = HDMIRX_READ32(REG_VID_CRC_CHK)&(0xff);
                tmp[2] = ((HDMIRX_READ32(REG_VID_CRC_CHK))>>8)&(0xff);
				
				printk( "[TMP]%x %x %x\n", tmp[0],  tmp[1],  tmp[2]);
                // vUtDelay10ms(2);
                // compare and update result if necessary
                if ((tmp[0] == result[0][0]) && (tmp[1] == result[0][1]) && (tmp[2] == result[0][2]))
                {
                    continue;
                }
                if ((tmp[0] == result[1][0]) && (tmp[1] == result[1][1]) && (tmp[2] == result[1][2]))
                {
                    continue;
                }
                //ASSERT((idx<3));
                if(idx>=3) {
					printk("#################################################\r\n");
					printk("############    FAIL CRC ########################\r\n");
					printk("#################################################\r\n");
					return 0;
                }
           
                result[idx][0] = tmp[0];
                result[idx][1] = tmp[1];
                result[idx][2] = tmp[2];
                
                idx++;
                continue;
            }
            else
            {
                //PRINT_REG(0x22c68);
                printk("CRC is not ready\n");
                return 0;
            }
        }
        else
        {
            printk( "reset CRC fail");
            return 0;
        }
    }

    //if (u1RegRead1B(VID_CH_MAP_1) & 0x04)
    if(HDMIRX_READ32(REG_VID_CH_MAP)&(1<<10))
    {
        printk( "interlace signal\r\n");
    }
    else
    {
        printk( "progressive signal~\r\n");
    }

	printk( "idx = %d:\r\n",idx);
    #if 1
    if (idx == 1)
    {
        printk( "jiffies : %d\n", jiffies);
        printk( "assume progressive signal\r\n");
        printk( "CRC result:\r\n");
        printk("%x %x %x\r\n", result[0][0], result[0][1], result[0][2]);
    }
    else if (idx == 2)
    {
        printk( "assume interlaced signal\n");
        printk( "CRC result:\n");
        printk( "%x %x %x\n", result[0][0], result[0][1], result[0][2]);
        printk("%x %x %x\n", result[1][0], result[1][1], result[1][2]);
    }
	else
	{
		printk( "############################abort(idx>=3)#####################\n");
	}
	#endif//0

#if 0
	if( ((result[0][0] == 0xE5 && result[0][1] == 0x5F && result[0][1] == 0x7C)
		||(result[1][0] == 0xE5 && result[1][1] == 0x5F && result[0][2] == 0x7C)) )
	{
	    if(crccnt %= 50){
			//printk( "assume interlaced signal\n");
			printk( "crccnt = %d:\r\n",crccnt);
			printk( "CRC result:\n");
			printk( "%x %x %x\n", result[0][0], result[0][1], result[0][2]);
			printk("%x %x %x\n", result[1][0], result[1][1], result[1][2]);
    	}
	}
	else
	{
		printk( "[TEST] reset CRC fail");
		printk( "crccnt = %d:\r\n",crccnt);
		printk( "CRC result:\n");
		printk( "%x %x %x\n", result[0][0], result[0][1], result[0][2]);
		printk("%x %x %x\n", result[1][0], result[1][1], result[1][2]);
		return 0;
		
	}
	#endif
    _CrcResult[0][0] =result[0][0];
    _CrcResult[0][1] =result[0][1];
    _CrcResult[0][2] =result[0][2];
    _CrcResult[1][0] =result[1][0];
    _CrcResult[1][1] =result[1][1];
    _CrcResult[1][2] =result[1][2];

    return 1;

}

BOOL fgHalIsINTR8_AUDFMTCHG(void)
{
    if(HDMIRX_READ32(REG_INTR_MASK) & INTR_AUD_FMT_CHG_MASK)
    {
        //write "1" to clear
        HDMIRX_WRITE32(REG_INTR_MASK, INTR_AUD_FMT_CHG_MASK);
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

BOOL fgHalIsINTR8_AUDCHSTATUSCHG(void)
{
    if(HDMIRX_READ32(REG_INTR_MASK) & INTR_DE_EMPH_CHG_MASK)
    {
        //write "1" to clear
        HDMIRX_WRITE32(REG_INTR_MASK, INTR_DE_EMPH_CHG_MASK);
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

void vHDMIRxIntMask(BOOL fgOn)
{
    if(fgOn)
    {
        HDMIRX_WRITE32_MASK(REG_INTR_MASK, 0x1<<6, INTR_AUD_FMT_CHG_MASK);
        HDMIRX_WRITE32_MASK(REG_INTR_MASK, 0x1<<7, INTR_DE_EMPH_CHG_MASK);
        HDMIRX_WRITE32_MASK(REG_INTR_MASK1, 0x1<<0, INTR4_UNDERRUN);
        HDMIRX_WRITE32_MASK(REG_INTR_MASK1, 0x1<<1, INTR4_OVERRUN);
        HDMIRX_WRITE32_MASK(REG_INTR_MASK1, 0x1<<28, INTR7_AUD_CH_STAT);
        HDMIRX_WRITE32_MASK(REG_INTR_MASK1, 0x1<<14, INTR5_AUDIO_MUTE);
    }
    return ;
}

BOOL   fgHalHDMIRxHDAudio(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_HBR)
    {
        HDMIRX_WRITE32_MASK(REG_INTR_STATE1, 0x1<<25, INTR7_HBR);
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

BOOL   fgHalHDMIRxDSDAudio(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR7_SACD)
    {
        HDMIRX_WRITE32_MASK(REG_INTR_STATE1, 0x1<<24, INTR7_SACD);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
 
}

BOOL   fgHalHDMIRxAudioPkt(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR2_NEW_AUD_PKT)
    {
        HDMIRX_WRITE32_MASK(REG_INTR_STATE0, 0x1<<17, INTR2_NEW_AUD_PKT);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UINT8   u1HalHDMIRxAudioCHSTAT0(void)
{
    UINT8 u1AudChstat0 ;
    u1AudChstat0 = (HDMIRX_READ32(REG_AUDRX_CTRL) & 0x00FF0000) >> 16;
    return  u1AudChstat0 ;
}

UINT8   u1HalHDMIRxAudioCHSTAT1(void)
{
    UINT8 u1AudChstat1 ;
    u1AudChstat1 = (HDMIRX_READ32(REG_AUDRX_CTRL) & 0xFF000000) >> 24;
    return  u1AudChstat1 ;
}


UINT8   u1HalHDMIRxAudioCHSTAT2(void)
{
    UINT8 u1AudChstat2 ;
    u1AudChstat2 = (HDMIRX_READ32(REG_CHST0) & 0x000000FF) >> 0;
    return  u1AudChstat2 ;
}


UINT8   u1HalHDMIRxAudioCHSTAT3(void)
{
    UINT8 u1AudChstat3 ;
    u1AudChstat3 = (HDMIRX_READ32(REG_CHST1) & 0x00000FF0) >> 4;
    return  u1AudChstat3 ;
}

UINT8   u1HalHDMIRxAudioCHSTAT4(void)
{
    UINT8 u1AudChstat4 ;
    u1AudChstat4 = (HDMIRX_READ32(REG_CHST1) & 0x0000FF00) >> 8;
    return  u1AudChstat4 ;
}

BOOL fgHalHDMIRxMultiPCM(void)
{
    if(HDMIRX_READ32(REG_AUDP_STAT) & HDMI_LAYOUT)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UINT8  u1HalHDMIRxAudioChannelNum(void)
{
    UINT8 u1RxChNum ;
    u1RxChNum = (HDMIRX_READ32(REG_CHST0) & CH_NUM1) >> 4;
    return  u1RxChNum ; 
}

UINT8 u1HalHDMIRxAudFsGet(void)
{
    UINT8 u1AudFs ;
    u1AudFs = (HDMIRX_READ32(REG_LK_THRS_SVAL) & RHDMI_AUD_SAMPLE_F) >> 24;
    return  u1AudFs ;     
}

UINT8  u1HalHDMIRxAudValidCHGet(void)
{
    UINT8 u1AudValidCh ;
    u1AudValidCh = (HDMIRX_READ32(REG_AUDIO_INFO) & 0x000F00) >> 8;
    return  u1AudValidCh ;     
}

void  vHalHDMIRxSetAudValidCH(UINT8 u1ValidCh)
{
    HDMIRX_WRITE32_MASK(REG_I2S_CTRL, u1ValidCh<<28, 0xF<<28);
    return ;
}

void  vHalHDMIRxSetAudMuteCH(UINT8 u1MuteCh)
{
    HDMIRX_WRITE32_MASK(REG_CHST1, u1MuteCh<<16, 0xF<<16);
    return ;
}

UINT32  vHalHDMIRxAudErrorGet()
{
    UINT32 u4AudErrtst = 0;
    u4AudErrtst = HDMIRX_READ32(REG_INTR_STATE1);

    HDMIRX_WRITE32_MASK(REG_INTR_STATE1, 0x1<<0, INTR4_UNDERRUN);
    HDMIRX_WRITE32_MASK(REG_INTR_STATE1, 0x1<<1, INTR4_OVERRUN);
    HDMIRX_WRITE32_MASK(REG_INTR_STATE1, 0x1<<8, INTR5_AUD_SAMPLE_F);

    return (u4AudErrtst & HDMIRX_INT_STATUS_CHK);
    
}


void vHalHDMIRxAudResetMCLK(void)
{
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, 0x1<<27, REG_MCLK_RST);
    msleep(2);
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, 0x0, REG_MCLK_RST);
    return ;
}

void vHalHdmiRxAudResetAfifo(void)
{
	// Reset Audio Fifo
 	HDMIRX_WRITE32_MASK(REG_SRST, 0x1<<9, FIFO_RST);
 	msleep(1);
    // Set Audio Fifo normal operation
 	HDMIRX_WRITE32_MASK(REG_SRST, 0x0, FIFO_RST);
	msleep(1);
	// Reset  Interrupt status : under-run / over-run / terc4  / hdcp
  	HDMIRX_WRITE32(REG_INTR_STATE1, HDMIRX_INT_STATUS_CHK);
}

enum HDMI_RX_AUDIO_FS eGetACRFs(void)
{
#if 0
    //ACRFs = 27 * 1024 * 1000 / 0x1f06C[18:16][31:24]  /  CTS  *  N  /  128
    UINT32 u4Reg = HDMIRX_READ32(REG_VID_CRC_OUT);
    UINT32 u4Temp = (((u4Reg&(0x7<<16))>>16)<<8) | ((u4Reg&(0xFF<<24))>>24);

    UINT32 ACRFs = (UINT32)
        (
         ((UINT64)216000 * (UINT64)u4HalGetRxHwNValue())
       / ((UINT64)u4Temp * (UINT64)u4HalGetRxHwCTSValue())
        );
    printk("ACRFs=%u\n", ACRFs);

    if ((192-6) < ACRFs && ACRFs < (192+6))
    {
        printk("SW_192K\n");
        return SW_192K;
    }
    else if ((48-2) < ACRFs && ACRFs < (48+2))
    {
        printk("SW_48K\n");
        return SW_48K;
    }
    else if ((44-2) < ACRFs && ACRFs < (44+2))
    {
        printk("SW_44p1K\n");
        return SW_44p1K;
    }
    else if ((88-4) < ACRFs && ACRFs < (88+4))
    {
        printk("SW_88p2K\n");
        return SW_88p2K;
    }
    else if ((96-4) < ACRFs && ACRFs < (96+4))
    {
        printk("SW_96K\n");
        return SW_96K;
    }
    else if ((176-6) < ACRFs && ACRFs < (176+6))
    {
        printk("SW_176p4K\n");
        return SW_176p4K;
    }
    else
    {
        printk("default SW_192K\n");
        return SW_192K;
    }
	#endif
	
	return SW_48K;
}

void  vHalHDMIRxAudResetAudio(void)
{
}

BOOL  bHalGetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
{
	BYTE checksum;
	int i;
	
	if(pAudioInfoFrame==NULL)
		return ER_FAIL;
	
	pAudioInfoFrame->pktbyte.AUD_HB[0]= (HDMIRX_READ32(REG_AUDRX0) &CEA_AUD_HEADER_7_0) >> 0; 	 // AUDIO InfoFrame Type Code. Required 0x84 : HDMIRX_AUD_INFOFRAME_TYPE
	pAudioInfoFrame->pktbyte.AUD_HB[1]= (HDMIRX_READ32(REG_AUDRX0) &CEA_AUD_HEADER_15_8) >> 8;	// AUDIO InfoFrame Version Code. Required 0x01 : HDMIRX_AUD_INFOFRAME_VER
	pAudioInfoFrame->pktbyte.AUD_HB[2]= (HDMIRX_READ32(REG_AUDRX0) &CEA_AUD_LENGTH) >> 16;	   // AUDIO InfoFrame Length. Required 0x0A : HDMIRX_AUD_INFOFRAME_LEN
	checksum = (HDMIRX_READ32(REG_AUDRX0) &CEA_AUD_CHECKSUM) >> 24;	// AUDIO InfoFrame Checksum. : HDMIRX_AUD_INFOFRAME_CHKSUM

	//HDMIRX_AUD_INFOFRAME_DB1~BD10
	pAudioInfoFrame->pktbyte.AUD_DB[0] = (HDMIRX_READ32(REG_AUDRX1) &CEA_AUD_DBYTE1) >> 0;
	pAudioInfoFrame->pktbyte.AUD_DB[1] = (HDMIRX_READ32(REG_AUDRX1) &CEA_AUD_DBYTE2) >> 8;
	pAudioInfoFrame->pktbyte.AUD_DB[2] = (HDMIRX_READ32(REG_AUDRX1) &CEA_AUD_DBYTE3) >> 16;
	pAudioInfoFrame->pktbyte.AUD_DB[3] = (HDMIRX_READ32(REG_AUDRX1) &CEA_AUD_DBYTE4) >> 24;
	pAudioInfoFrame->pktbyte.AUD_DB[4] = (HDMIRX_READ32(REG_AUDRX2) &CEA_AUD_DBYTE5) >> 0;
	pAudioInfoFrame->pktbyte.AUD_DB[5] = (HDMIRX_READ32(REG_AUDRX2) &CEA_AUD_DBYTE6) >> 8;
	pAudioInfoFrame->pktbyte.AUD_DB[6] = (HDMIRX_READ32(REG_AUDRX2) &CEA_AUD_DBYTE7) >> 16;
	pAudioInfoFrame->pktbyte.AUD_DB[7] = (HDMIRX_READ32(REG_AUDRX2) &CEA_AUD_DBYTE8) >> 24;
	pAudioInfoFrame->pktbyte.AUD_DB[8] = (HDMIRX_READ32(REG_AUDRX3) &CEA_AUD_DBYTE9) >> 0;
	pAudioInfoFrame->pktbyte.AUD_DB[9] = (HDMIRX_READ32(REG_AUDRX3) &CEA_AUD_DBYTE10) >> 8;

	//config info value
	pAudioInfoFrame->info.Type= pAudioInfoFrame->pktbyte.AUD_HB[0];
	pAudioInfoFrame->info.Ver= pAudioInfoFrame->pktbyte.AUD_HB[1];
	pAudioInfoFrame->info.Len= pAudioInfoFrame->pktbyte.AUD_HB[2];
	pAudioInfoFrame->info.AudioChannelCount= pAudioInfoFrame->pktbyte.AUD_DB[0]& 0x07;

	pAudioInfoFrame->info.RSVD1= 0;
	pAudioInfoFrame->info.AudioCodingType= (pAudioInfoFrame->pktbyte.AUD_DB[0]>>4)& 0x0f;
	pAudioInfoFrame->info.SampleSize= pAudioInfoFrame->pktbyte.AUD_DB[1]& 0x03;
	pAudioInfoFrame->info.SampleFreq= (pAudioInfoFrame->pktbyte.AUD_DB[1] >>2)& 0x07;
	pAudioInfoFrame->info.Rsvd2=0;
	pAudioInfoFrame->info.FmtCoding= (pAudioInfoFrame->pktbyte.AUD_DB[0]>>4)& 0x0f;//actually spec does not have this
	pAudioInfoFrame->info.SpeakerPlacement= pAudioInfoFrame->pktbyte.AUD_DB[3];
	pAudioInfoFrame->info.Rsvd3= 0;
	pAudioInfoFrame->info.LevelShiftValue= (pAudioInfoFrame->pktbyte.AUD_DB[4]>>3)&0x0f;
	pAudioInfoFrame->info.DM_INH= (pAudioInfoFrame->pktbyte.AUD_DB[4]>> 7)&0x01;

	for(i=0; i<3; i++)
		checksum += pAudioInfoFrame->pktbyte.AUD_HB[i];
	for(i=0; i<5; i++) //only 5 bytes is valued
		checksum += pAudioInfoFrame->pktbyte.AUD_DB[i];
	
	if(checksum==0)
		return 0;
	else
		return 1;

}

BOOL fgHalCheckIsAAC(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE1) & INTR5_AUDIO_MUTE)
    {
        HDMIRX_WRITE32_MASK(REG_INTR_STATE1, 0x1<<14, INTR5_AUDIO_MUTE);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void vHalSetHDMIRxHBR(BOOL fgHBR)
{
    if (fgHBR)
    {
        enum HDMI_RX_AUDIO_FS eGetACRFs(void);
        printk("[HDMI RX AUD] HdmiRxHBR\n");
        vHalSetRxAudioFS(eGetACRFs());

        HDMIRX_WRITE32_MASK(REG_AUDRX_CTRL,I2S_MODE,I2S_MODE);
        HDMIRX_WRITE32_MASK(REG_AUDIO1,0,0x07<<17);
        HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL,(0x01<<9)|(0x01<<11)|(0x01<<16),(0x01<<9)|(0x01<<11)|(0x01<<16)|(0x01<<17));
        HDMIRX_WRITE32_MASK(REG_I2S_CTRL,0,(0x01<<16));
        HDMIRX_WRITE32_MASK(REG_AUDIO,0xB2<<24,(0xff<<24));

    }
    else
    {
        HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL, 0, TT1_8CH|TT0_HBR_8CH);
    }
}


void vHalHDMIRxEnableAudPktReceive(void)
{
    HDMIRX_WRITE32_MASK(0x84,(0x01<<8),(0x01<<8));
    HDMIRX_WRITE32_MASK(0x84,(0x01<<9),(0x01<<9));
}

void vHalHdmiRxAudBypass(BOOL fgBypass,BOOL fgBypassSPDIF2Tx)
{
#if 0
	 if (fgBypass)
	 {
        //Bypass HDMI Rx SPDIF to HDMI Rx(Internal)
        CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_RX_SEL_MASK)&(~HDMI_RX_SPDF2HDMITX_MASK)) | (HDMI_RX_SEL_INTERNAL|HDMI_RX_SPDF2HDMITX)));
        CKGEN_WRITE32(REG_RW_PAD_CFG_3, (CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_EXT_DATA_SEL_MASK)&(~HDMI_EXT_RX_SPDF_SEL_MASK)));
        CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_EXT_SPDF_SEL_MASK)&(~HDMI_EXT_ACK_SEL_MASK)&(~HDMI_EXT_BCK_SEL_MASK)&(~HDMI_EXT_LRCK_SEL_MASK)) | HDMI_SPDF_FROM_EXT_HDMIRX));


        //Bypass HDMI Rx I2S to HDMI Rx(Internal)
        //CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_RX_SEL_MASK)) | HDMI_RX_SEL_INTERNAL));
        CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_EXT_DATA_SEL_MASK)) | HDMI_I2S_FROM_EXT_HDMIRX));
        CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) &(~HDMI_EXT_ACK_SEL_MASK)&(~HDMI_EXT_BCK_SEL_MASK)&(~HDMI_EXT_LRCK_SEL_MASK)) | (HDMI_BCK_FROM_EXT_HDMIRX|HDMI_LRCK_FROM_EXT_HDMIRX)));
   	 }
	 else
	 {
	     // No Bypass
        CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_RX_SEL_MASK)&(~HDMI_RX_SPDF2HDMITX_MASK)) | (HDMI_RX_SEL_INTERNAL)));
		CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_EXT_DATA_SEL_MASK))));
		CKGEN_WRITE32(REG_RW_PAD_CFG_3, ((CKGEN_READ32(REG_RW_PAD_CFG_3) & (~HDMI_EXT_SPDF_SEL_MASK)&(~HDMI_EXT_ACK_SEL_MASK)&(~HDMI_EXT_BCK_SEL_MASK)&(~HDMI_EXT_LRCK_SEL_MASK))));
	 }
 #endif
}

void vHalSetHDMIRxI2S(void)
{

	    //LOG(9,"[HDMI RX AUD] HdmiRxHBR\n");

/*	    vRxWriteReg(0xd0,0x1F);
	    vRxWriteReg(0xdc,0x00);
	   vRxWriteReg(0xcc,0x10);
	 //Clock  mode
	  vRxWriteRegMsk(0x1F26c,0x02,0x0f);
*/
}

BOOL fgHalHDMIRxAPLLStatus(void)
{
    if(HDMIRX_READ32(REG_APLL1) & ACR_DPLL_LOCK)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void vHalSetHDMIRxDSD(BOOL fgDSD)
{
    if (fgDSD)
    {
        vHalSetRxAudioFS(SW_44p1K);
        HDMIRX_WRITE32_MASK(REG_AUDIO1,(DSD_SEL|REG_DSD_SCK_ON),(DSD_SEL|REG_DSD_SCK_ON));
        HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL,TT1_SPDIF_PICK_8CH,TT1_SPDIF_PICK_8CH); // for mutl-ch sacd nosie issue
    }
    else
    {
        HDMIRX_WRITE32_MASK(REG_AUDIO1,0,(DSD_SEL|REG_DSD_SCK_ON));
        HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL,0,TT1_SPDIF_PICK_8CH); // for mutl-ch sacd nosie issue
    }
}


BYTE vHalGetUnRecPacketHeader(void)
{
    UINT32 u4Value = 0;
    u4Value = HDMIRX_READ32(REG_UNRECRX0) & 0x000000FF;

    return (UINT8)u4Value;
}

void vHalGetUnRecPacket(BYTE *bUnRecPacketData)
{
	BYTE i;
	for(i=0; i<31; i++)
	{
		*(bUnRecPacketData+i) = HDMIRX_READ32(REG_UNRECRX0+i) & 0x000000FF;
	}
}

void vSetSelectUnRecpacket(BOOL fgEnable,BYTE bHeader)
{
    if(fgEnable)
    {
        HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL, TT2_EXT_PKT_EN, TT2_EXT_PKT_EN);
        HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL3, bHeader<<0, EXT_PKT_DEC);
    }
    else
    {
        HDMIRX_WRITE32_MASK(REG_N_HDMI_CTRL, 0, TT2_EXT_PKT_EN);
    }
    
    return ;
}

//For HDCP
void vHalRxHdcpReset(void)
{
    HDMIRX_WRITE32_MASK(REG_SRST, HDCP_RST, HDCP_RST);
    HDMIRX_WRITE32_MASK(REG_SRST, 0, HDCP_RST);

    return ;
}

void vHalDisableHDCPDDCPort(void)
{
    HDMIRX_WRITE32_MASK(REG_SYS_CTRL, 0x0, DDC_EN);
    return ;

}

void vHalEnableHDCPDDCPort(void)
{
    HDMIRX_WRITE32_MASK(REG_SYS_CTRL,DDC_EN, DDC_EN);
    return ;
}

void vHalSetKsvReadyBit(void)
{
    HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, 0, FIFO_RDY_WP);//pull low
    HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, REG_FIFO_RDY, REG_FIFO_RDY);
    HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, FIFO_RDY_WP, FIFO_RDY_WP);//pull high

    return ;
}

void  vHalClearKsvReadyBit(void)
{
    HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, 0, FIFO_RDY_WP);//pull low
    HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, 0, REG_FIFO_RDY);
    HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, FIFO_RDY_WP, FIFO_RDY_WP);//pull high

    return ;
}

void  vHalLoadHdcpKey(UINT8 *prHdcpKey)
{
	BYTE i;
	UINT32 u4Data;
	HDMIRX_WRITE32(HDCP_DEV,0x03A0);
	HDMIRX_WRITE32(HDCP_ADDR,0);
	for(i=0 ;i<73 ;i++)
	{
		msleep(1);
		u4Data = ((*(prHdcpKey+i*4+3))<<24)|((*(prHdcpKey+i*4+2))<<16)|((*(prHdcpKey+i*4+1))<<8)|((*(prHdcpKey+i*4)));
		HDMIRX_WRITE32(HDCP_DATA, u4Data);
	}
	msleep(1);
	HDMIRX_WRITE32_MASK(REG_EPST,OPT_DN_KSV|LD_KSV,OPT_DN_KSV|LD_KSV);
}

void  vHalSetRepeaterMode(BOOL fgRepeater)
{
	if(fgRepeater)
		HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, REPEATER, REPEATER);
	else
		HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, 0, REPEATER);
}

void  vHalSetHdmiCapable(BOOL fgHdmiCapable)
{
	HDMIRX_WRITE32_MASK(REG_SHD_BSTATUS, fgHdmiCapable<<23, HDMI_CAPABLE);
}

void  vHalWriteKsvList(BYTE *prKsvList,BYTE bCount)
{
	UINT8 bLength;
	UINT8 i;
	bLength = bCount*5;
	for(i=0; i<bLength; i++)
	{
		msleep(1);
		HDMIRX_WRITE32(KSV_DATA, *(prKsvList+i));
	}
	msleep(1);
}

UINT32  vHalGetKsvFifoAddr(void)
{
	UINT32 u4Addr;
	u4Addr = HDMIRX_READ32(KSV_DATA);
	u4Addr = (u4Addr>>16);
	u4Addr &= 0x3FF;
	return(u4Addr);
}

void  vHalTriggerSHA(void)
{
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, 0x1<<20, SHA_GO);
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, 0x0, SHA_GO);
    return ;
}

void  vHalRptStartAddrClr(void)
{
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, 0x1<<22, RPT_START_ADDR_CLR);
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, 0x0, RPT_START_ADDR_CLR);
    return ;
}

void  vHalSetSHALength(UINT32 u4Length)
{
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, u4Length<<8, SHA_LENGTH);
    return ;
}

void  vHalSetSHAAddr(UINT16 bAddr)
{
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, bAddr&0xFF, RPT_START_ADDR_7_0);
    HDMIRX_WRITE32_MASK(REG_SHA_LENGTH, ((bAddr>>8)&0xFF)<<18, RPT_START_ADDR_9_8);
    return ;
}

//AKSV is wrote
BOOL fgHalHdcpAuthenticationStart(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_AUTH_START)
        return TRUE;
    else
        return FALSE;
}


BOOL fgHalHdcpAuthenticationDone(void)
{
    if(HDMIRX_READ32(REG_INTR_STATE0) & INTR1_AUTH_DONE)
        return TRUE;
    else
        return FALSE;
}

BOOL fgHalHdcpHdmiMode(void)
{
    if(HDMIRX_READ32(REG_HDCP_STAT) & RHDMI_MODE_EN)
        return TRUE;
    else
        return FALSE;
}

void vHalClearHdcpAuthenticationStartStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, INTR1_AUTH_START);
    return ;
}

void vHalClearHdcpAuthenticationDoneStatus(void)
{
    HDMIRX_WRITE32(REG_INTR_STATE0, INTR1_AUTH_DONE);
    return ;
}

BOOL fgHalIsVReady(void)
{
	if ((HDMIRX_READ32(REG_INTR) & V_RDY_INTR) == 0x1 )
		return TRUE;
	else
		return FALSE;
}

void  vHalSetBstatus(UINT16 u2Bstatus)
{
	HDMIRX_WRITE32(REG_SHD_BSTATUS, u2Bstatus & 0xFF);
	HDMIRX_WRITE32(REG_HDCP_STAT, (u2Bstatus>>8) & 0xFF);
}


void  vHalSetKsvStop(BOOL fgRiscAccressEnable)
{
    HDMIRX_WRITE32_MASK(REPEATER_KSV,KSV_STOP,KSV_STOP);
    HDMIRX_WRITE32_MASK(REPEATER_KSV,0,KSV_STOP);
	if(fgRiscAccressEnable)
	HDMIRX_WRITE32_MASK(REPEATER_KSV,KSV_RISC_WR_SEL,KSV_RISC_WR_SEL);
	else
    HDMIRX_WRITE32_MASK(REPEATER_KSV,0,KSV_RISC_WR_SEL);

}

void vGetRxAKsv(BYTE *prRxAKSV)
{
    *(prRxAKSV+0) = HDMIRX_READ32(REG_SHD_AKSV) & HDCP_AKSV1;
    *(prRxAKSV+1) = HDMIRX_READ32(REG_SHD_AKSV) & HDCP_AKSV2;
    *(prRxAKSV+2) = HDMIRX_READ32(REG_SHD_AKSV) & HDCP_AKSV3;
    *(prRxAKSV+3) = HDMIRX_READ32(REG_SHD_AN0) & HDCP_AKSV4;
    *(prRxAKSV+4) = HDMIRX_READ32(REG_SHD_AN0) & HDCP_AKSV5;

    return ;
}

void  RxUse27M(void)
{
	//vRxWriteRegMsk(0x84,0,(1<<1)|(1<<2));
}

UINT16 vGetRxRi(void)
{
    UINT8 Ri_0,Ri_1;
    UINT16 Ri;
    Ri_0= (HDMIRX_READ32(REG_SHD_AKSV) & HDCP_RI0_7_0) >> 0;
    Ri_1= (HDMIRX_READ32(REG_SHD_BKSV1) & HDCP_RI0_15_8) >> 24;
    Ri = ((Ri_0)|(Ri_1<<8));
    
    return(Ri);
}

void vGetRxAn(BYTE *prRxAn)
{
    *(prRxAn+0)= (HDMIRX_READ32(REG_SHD_AN0) & HDCP_AN1) >> 16;
    *(prRxAn+1)= (HDMIRX_READ32(REG_SHD_AN0) & HDCP_AN2) >> 24;
    *(prRxAn+2)= (HDMIRX_READ32(REG_SHD_AN1) & HDCP_AN3) >> 0;
    *(prRxAn+3)= (HDMIRX_READ32(REG_SHD_AN1) & HDCP_AN4) >> 8;
    *(prRxAn+4)= (HDMIRX_READ32(REG_SHD_AN1) & HDCP_AN5) >> 16;
    *(prRxAn+5)= (HDMIRX_READ32(REG_SHD_AN1) & HDCP_AN6) >> 24;
    *(prRxAn+6)= (HDMIRX_READ32(REG_SHD_BSTATUS) & HDCP_AN7) >> 0;
    *(prRxAn+7)= (HDMIRX_READ32(REG_SHD_BSTATUS) & HDCP_AN8) >> 8;

    return ;
}

void vGetRxBKsv(BYTE *prRxBKSV)
{
    *(prRxBKSV+0)= (HDMIRX_READ32(REG_SHD_BKSV0) & HDCP_BKSV1) >> 16;
    *(prRxBKSV+1)= (HDMIRX_READ32(REG_SHD_BKSV0) & HDCP_BKSV2) >> 24;
    *(prRxBKSV+2)= (HDMIRX_READ32(REG_SHD_BKSV1) & HDCP_BKSV3) >> 0;
    *(prRxBKSV+3)= (HDMIRX_READ32(REG_SHD_BKSV1) & HDCP_BKSV4) >> 8;
    *(prRxBKSV+4)= (HDMIRX_READ32(REG_SHD_BKSV1) & HDCP_BKSV5) >> 16;

    return ;
}

UINT32  u4GetVblank(void)
{
	UINT32 u4Value = 0;
	u4Value = vHalRxGetVFrontPorch() + vHalRxGetVBackPorch();
	return u4Value;
}

UINT32 HDMI_HalGetPixelClockExt(void)
{
    UINT32 u4PclkFreq = 0;
    UINT32 u4PclkNum = 0;
    UINT32 u4XclkFreq = 26 *1000 *1000; // 27M crystal
    UINT32 u4XclkNum = 0;

    if(HDMIRX_READ32(REG_VID_CRC_OUT) & XCLK_IN_PCLK_SEL)
    {
        u4PclkNum = 1024;
    }
    else
    {
        u4PclkNum = 128;
    }

    u4XclkNum = HDMI_HalGetXclkCnt();

    // caculate pixel clock,  u4XclkNum * (1/u4XclkFreq) = u4PclkNum * (1/u4PclkFreq)
    if((u4XclkNum != 0) && (u4XclkNum != 1))
    {
        u4PclkFreq = u4PclkNum * (u4XclkFreq/1000) / u4XclkNum * 1000; // for range of 0xffffffff
    }
    else
    {
        u4PclkFreq = 0;
    }
    
    return u4PclkFreq;
}

UINT32 HDMI_HalGetXclkCnt(void)
{
    UINT32 u4XclkInPclk = 0;

    UINT32 u4Value_h = 0;
    UINT32 u4Value_l = 0;

    u4Value_h = (HDMIRX_READ32(REG_VID_CRC_OUT) & AAC_XCLK_IN_PCLK_10_8) >> 16;
    u4Value_l = (HDMIRX_READ32(REG_VID_CRC_OUT) & AAC_XCLK_IN_PCLK_7_0) >> 24;

    u4XclkInPclk = (u4Value_h<<8) | u4Value_l;
    u4XclkInPclk = u4XclkInPclk + 1;
    
    return u4XclkInPclk;
}

UINT32 HDMI_HalGetTmdsPeriod(void)
{
    UINT32 u4TmdsPeriod = 0;

    u4TmdsPeriod = (HDMIRX_READ32(REG_N_HDMI_CTRL4) & TMDS_CK_PERIOD) >> 12 ;
    
    return u4TmdsPeriod;
}

UINT32 HDMI_HalGetTmdsClockExt(void)
{
    UINT32 u4TmdsPeriod = 0;
    UINT32 u4TmdsClock = 0;

    u4TmdsPeriod = HDMI_HalGetTmdsPeriod();
    if(u4TmdsPeriod != 0)
    {
        u4TmdsClock = (27 * 1000 * 1000) * 64 / u4TmdsPeriod;
    }
    else
    {
        u4TmdsClock = 0;
    }

    return u4TmdsClock;
}
// hdmi rx part analog part power off
void hdmirxanapoweroff(void)
{
	HDMIRX_A_WRITE32(0x150,0xC0000000);
	HDMIRX_A_WRITE32(0x154,0x44E00E42);
	HDMIRX_A_WRITE32(0x158,0x0B005010);
	HDMIRX_A_WRITE32(0x15C,0x00000000);
	HDMIRX_A_WRITE32(0x160,0x00000100);
	HDMIRX_A_WRITE32(0x164,0x00000002);
	HDMIRX_A_WRITE32(0x168,0x00000009);
	HDMIRX_A_WRITE32(0x16c,0x00000000);
	HDMIRX_A_WRITE32(0x170,0x00000000);
	HDMIRX_A_WRITE32(0x174,0x00000000);
	HDMIRX_A_WRITE32(0x178,0x00000000);
	HDMIRX_A_WRITE32(0x17C,0x00000000);

}

/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "mt7623-dai.h"
#include "mt7623-afe.h"
#include <linux/delay.h>
#include "mt7623-private.h"
#include "mt7623-afe-reg.h"
#include "mt7623-hdmi-control.h"
extern kal_uint32 iec2_nsadr;
extern kal_uint32 iec_nsadr;

UINT32 iec_force_updata_size[20] = { 0, 0, 0, 0, 2, 3, 5, 0xa, 0, 0, 0, 0, 0, 0, 0, 3, 5,9,0,0xa};

UINT32 channel_status_pcm[20] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x03001900, 0x02001900, 0x0A001900, 0x0E001900,0x00000000,
                                 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00001900, 0x08001900,0x0C001900,0x00000000,0x09001900};

UINT32 channel_status_raw_pcm[20] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x03001902, 0x02001902, 0x0A001902, 0x0E001902,0x00000000,
                                     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00001902, 0x08001902,0x0C001902,0x00000000,0x09001902};

unsigned int  APll[2] = {98304000, 90316800};//48K 44.1K
/**
* value of connection register to set for certain input
*/
const unsigned int gHdmiConnectionValue[HDMI_NUM_INPUT] = {
	/* I_20 I_21 I_22 I_23 I_24 I_25 I_26 I_27 */
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7
};

/**
* mask of connection register to set for certain output
*/
const unsigned int gHdmiConnectionMask[HDMI_NUM_OUTPUT] = {
	/* O_20  O_21  O_22  O_23  O_24  O_25  O_26  O_27  O_28  O_29 */
	0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7
};

/**
* shift bits of connection register to set for certain output
*/
const char gHdmiConnectionShiftBits[HDMI_NUM_OUTPUT] = {
	/* O_20 O_21 O_22 O_23 O_24 O_25 O_26 O_27 O_28 O_29 */
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27
};
/**
* connection state of HDMI
*/
char gHdmiConnectionState[HDMI_NUM_INPUT][HDMI_NUM_OUTPUT] = {
	/* O_20 O_21 O_22 O_23 O_24 O_25 O_26 O_27 O_28 O_29 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* I_20 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* I_21 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* I_22 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* I_23 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* I_24 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* I_25 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* I_26 */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}	/* I_27 */
};

void SetHdmiInterConnection(HDMI_INTERCON_STATUS ConnectionState, unsigned int Input, unsigned int Output)
{
	unsigned int inputIndex;
	unsigned int outputIndex;

    pr_debug("%s() ConnectionState = %d,Input = %d,Output = %d\n", __func__, ConnectionState,Input,Output);
	/* check if connection request is valid */
	if (Input < HDMI_IN_BASE ||Input > HDMI_IN_MAX || Output < HDMI_OUT_BASE || Output > HDMI_OUT_MAX) {
	    pr_err("%s() error invaled ConnectionState = %d,Input = %d,Output = %d\n", __func__, ConnectionState,Input,Output);
	}
	inputIndex = Input - HDMI_IN_BASE;
	outputIndex = Output - HDMI_OUT_BASE;

	/* do connection */
	switch (ConnectionState) {
	case HDMI_Connection:
		{
			unsigned int regValue = gHdmiConnectionValue[inputIndex];
			unsigned int regMask = gHdmiConnectionMask[outputIndex];
			unsigned int regShiftBits = gHdmiConnectionShiftBits[outputIndex];
			afe_msk_write(AFE_HDMI_CONN0, regValue << regShiftBits,
				    regMask << regShiftBits);
			gHdmiConnectionState[inputIndex][outputIndex] = HDMI_Connection;
			break;
		}
	case HDMI_DisConnect:
		{
			unsigned int regValue = gHdmiConnectionValue[HDMI_IN_I27 - HDMI_IN_BASE];
			unsigned int regMask = gHdmiConnectionMask[outputIndex];
			unsigned int regShiftBits = gHdmiConnectionShiftBits[outputIndex];
			afe_msk_write(AFE_HDMI_CONN0, regValue << regShiftBits,
				    regMask << regShiftBits);
			gHdmiConnectionState[inputIndex][outputIndex] = HDMI_DisConnect;
			break;
		}
	default:
		pr_err("%s() invaled ConnectionState = %d,Input = %d,Output = %d\n", __func__, ConnectionState,Input,Output);
		break;
	}

}

UINT32 vGetSampleratefromindx(UINT32 fs)
{
    switch (fs) {
    case FS_8000HZ:   return 8000;
    case FS_12000HZ:  return 12000;
    case FS_16000HZ:  return 16000;
    case FS_24000HZ:  return 24000;
    case FS_32000HZ:  return 32000;
    case FS_48000HZ:  return 48000;
    case FS_96000HZ:  return 96000;
    case FS_192000HZ: return 192000;
    case FS_384000HZ: return 384000;
    case FS_7350HZ:   return 7350;
    case FS_11025HZ:  return 11025;
    case FS_14700HZ:  return 14700;
    case FS_22050HZ:  return 22050;
    case FS_29400HZ:  return 29400;
    case FS_44100HZ:  return 44100;
    case FS_88200HZ:  return 88200;
    case FS_176400HZ: return 176400;
    case FS_352800HZ: return 352800;
    default:          return 0;
    }
}

UINT32 vGetu2IecForceUpdataSize(UINT32 SampleRate)
{
	UINT32 u2IecForceUpdataSize = 0;
     switch(SampleRate)
     {
	 	case 0:
            u2IecForceUpdataSize = 2;
            break;
        case 1:
        case 2:
            u2IecForceUpdataSize = 3;
            break;
        case 3:
        case 4:
            u2IecForceUpdataSize = 5;
            break;
        case 5:
            u2IecForceUpdataSize = 9;
            break;
        case 6:
            u2IecForceUpdataSize = 0xa;
            break;
        default:
            u2IecForceUpdataSize = 0;
	        pr_err("%s() default invalid u2IecForceUpdataSize\n", __func__);
            break;
     }
	 return u2IecForceUpdataSize;

}

static UINT32 vSetDivClk(UINT32 CLK_DIV_2)
{
	UINT32 CLK_DIV = 0;
	switch(CLK_DIV_2)
	{
		case 0:
			CLK_DIV = 0;
			break;
		case 1:
			CLK_DIV = 1;
			break;
		case 4:
			CLK_DIV = 2;
		    break;
		case 8:
			CLK_DIV = 3;
			break;
		case 16:
			CLK_DIV = 4;
			break;
		case 24:
			CLK_DIV = 5;
			break;
		default:
			pr_err("%s() default invalid CLK_DIV\n", __func__);
			break;
	}
    return CLK_DIV;

}

static UINT32 vGetApllbySampleRate(UINT32 SampleRate)
{
    if ((SampleRate == 48000)|(SampleRate == 32000)|(SampleRate == 96000)|(SampleRate == 192000))// 98.304M 48K APLL1
    {
		afe_msk_write(CLK_AUDDIV_3, HDMISPDIF_48K_BASE_CLK, HDMISPDIF_CLK_SWITCH_MASK);
#if 1 //test I2S5
		afe_msk_write(CLK_AUDDIV_3, 0<<19, 1<<19);
		afe_msk_write(CLK_AUDDIV_2, 3<<0,  0xff<<0);
#endif
        return HDMI_APLL1;

    }
    else// 90.3168M  44.1K APLL2
    {
		afe_msk_write(CLK_AUDDIV_3, HDMISPDIF_44K_BASE_CLK, HDMISPDIF_CLK_SWITCH_MASK);
#if 1 //test I2S5
		afe_msk_write(CLK_AUDDIV_3, 1<<19, 1<<19);
		afe_msk_write(CLK_AUDDIV_2, 3<<0,  0xff<<0);
#endif
        return HDMI_APLL2;
    }
}
static void vSetRegisterForSpdif2(UINT32 eMuxSelect,UINT32 mclk_fs,UINT32 eSamplingRateType,UINT32 value)
{
	// spdif2 MCLK=CLK
    UINT32 OutFrequency_2 = 0;
    UINT32 Mclk_Div_2 = 0;
    UINT32 CLK_DIV_2 = 0;
	UINT32 Sample_Rate = 0;
	Sample_Rate = fs_integer(eSamplingRateType);
	// spdif2 use MCLK = mclk_fs*FS  = 128 * 48k
    CLK_DIV_2 = APll[eMuxSelect] / (Sample_Rate * mclk_fs); // APLL / (CLK=MCLK) 48 ---->16
	CLK_DIV_2 = vSetDivClk(CLK_DIV_2);
	Mclk_Div_2 = (Sample_Rate * mclk_fs / mclk_fs / Sample_Rate) - 1;//0

	afe_msk_write(CLK_CFG_5, CLK_DIV_2<<16, APLL_DIV_CLK_MASK);
    afe_msk_write(AUDIO_TOP_CON0,  value << 23, HDMISPDIF_POWER_UP_MASK);// 0 power up 1 power down
	afe_msk_write(AUDIO_TOP_CON2, Mclk_Div_2<<16,SPDIF2_CK_DIV_MASK);
	afe_msk_write(AUDIO_TOP_CON0, value<<22, SPDIF2_POWER_UP_MASK);//power up spdif1 1 power down
	OutFrequency_2 = Sample_Rate * mclk_fs / (Mclk_Div_2+1);// spdif_mclk

	pr_debug("%s():eMuxSelect = %d,OutFrequency_2 = %d,Sample_Rate = %d,mclk_fs = %d\n",__func__ ,eMuxSelect,OutFrequency_2,Sample_Rate,mclk_fs);
	pr_debug("%s():CLK_DIV_2 = %d,Mclk_Div_2 = %d\n",__func__ ,CLK_DIV_2,Mclk_Div_2);

}
void vSetRegisterForSpdif1(UINT32 eMuxSelect,UINT32 mclk_fs,UINT32 eSamplingRateType,UINT32 value)
{
	// spdif1  MCLK = CLK
    UINT32 OutFrequency_1 = 0;
    UINT32 Mclk_Div_1 = 1;
	UINT32 CLK_DIV_1 = 0;
	UINT32 Sample_Rate = 0;
	Sample_Rate = fs_integer(eSamplingRateType);
	// spdif1 use mclk_fs*FS  = 128 * 48k
    CLK_DIV_1 = APll[eMuxSelect] / (Sample_Rate * mclk_fs); // APLL / (CLK=MCLK) 48 ---->16
	CLK_DIV_1 = vSetDivClk(CLK_DIV_1);
	Mclk_Div_1 = (Sample_Rate * mclk_fs / mclk_fs / Sample_Rate) - 1;//0

    afe_msk_write(CLK_CFG_5, CLK_DIV_1<<16, APLL_DIV_CLK_MASK);
	afe_msk_write(AUDIO_TOP_CON0, value<<23, HDMISPDIF_POWER_UP_MASK);// 0 power up CLK 1 power down
    afe_msk_write(AUDIO_TOP_CON2, Mclk_Div_1<<0,SPDIF_CK_DIV_MASK);
    afe_msk_write(AUDIO_TOP_CON0, value<<21, SPDIF1_POWER_UP_MASK);//power up spdif1 1 power down

    OutFrequency_1 = Sample_Rate * mclk_fs /(Mclk_Div_1+1);
    pr_debug("%s():eMuxSelect = %d,OutFrequency_1 = %d,Sample_Rate = %d,mclk_fs = %d\n",__func__ ,eMuxSelect,OutFrequency_1,Sample_Rate,mclk_fs);
    pr_debug("%s():CLK_DIV_1 = %d,Mclk_Div_1 = %d\n",__func__ ,CLK_DIV_1,Mclk_Div_1);

}
void vSetRegisterForHdmi(UINT32 eMuxSelect,UINT32 mclk_fs,UINT32 eSamplingRateType,SPDIF_PCM_BITWIDTH eBitWidth,UINT32 DSDBCK,UINT32 value)
{
	//HDMI
    UINT32 OutFrequency = 0;
    UINT32 Mclk_Div = 0;
	UINT32 CLK_DIV = 0;
	UINT32 Bck_Div = 0;
	UINT32 Lrck_Div = 0;
	UINT32 Sample_Rate = 0;
	Sample_Rate = fs_integer(eSamplingRateType);
	//hdmi use
	// MCLK = mclk_fs * FS  = 128 * 48k
	// BCK = 16 * fs (8bit)
	// BCK = 32 *fs (16bit)
	// BCK = 64 * fs (24bit)
	// LRCLK = fs
	// DSD bck is 64 *fs
    switch(eBitWidth)
    {
		case PCM_OUTPUT_8BIT:
			if(DSDBCK == 1)
			{
			   Bck_Div = (((Sample_Rate * mclk_fs) / (64 * Sample_Rate))/2)-1; // for dsd  = 64 * fs
			}
		    else
		    {
			  Bck_Div = (((Sample_Rate * mclk_fs) / (16 * Sample_Rate))/2)-1; // 1
		    }
			break;
		case PCM_OUTPUT_16BIT:
			if(DSDBCK == 1)
			{
			   Bck_Div = (((Sample_Rate * mclk_fs) / (64 * Sample_Rate))/2)-1; // for dsd  = 64 * fs
			}
			else
			{
			  Bck_Div = (((Sample_Rate * mclk_fs) / (32 * Sample_Rate))/2)-1; // 1
			}
			break;
		case PCM_OUTPUT_24BIT:
		case PCM_OUTPUT_32BIT:
		      Bck_Div = (((Sample_Rate * mclk_fs) / (64 * Sample_Rate))/2)-1;// 0
		    break;
        default:
            pr_err("%s() default invalid Bck_Div\n", __func__);
			break;
    }
	CLK_DIV = APll[eMuxSelect] / (Sample_Rate * mclk_fs);// APLL/(clk = mclk)
	CLK_DIV = vSetDivClk(CLK_DIV);

	Mclk_Div = (Sample_Rate * mclk_fs / mclk_fs / Sample_Rate) - 1;//0   hdmi clk = mclk BCK=MCLK/(2*(BCK_DIV+1))
	Lrck_Div = ((Sample_Rate * mclk_fs) / (2*(Bck_Div+1)))/(2*Sample_Rate);//LRCLK = BCK/(2*LRCLK_DIV) 16 ==32k

    afe_msk_write(CLK_CFG_5, CLK_DIV<<16, APLL_DIV_CLK_MASK);//CLK = MCLK
	afe_msk_write(AUDIO_TOP_CON0, value << 23, HDMISPDIF_POWER_UP_MASK);//0 power up CLK 1 power down
    afe_msk_write(AUDIO_TOP_CON3, Bck_Div<<8,HDMI_CK_DIV_MASK);// BCK
    afe_msk_write(AUDIO_TOP_CON0, value << 20, HDMI_POWER_UP_MASK);//0 power up HDMI_BCK 1 power down

    OutFrequency =((Sample_Rate * mclk_fs) / (2*(Bck_Div+1)));

	pr_debug("%s():eMuxSelect = %d,OutFrequency = %d,Sample_Rate = %d,mclk_fs = %d\n",__func__ ,eMuxSelect,OutFrequency,Sample_Rate,mclk_fs);
    pr_debug("%s():CLK_DIV = %d,Mclk_Div = %d,Lrck_Div= %d,DSDBCK = %d\n",__func__ ,CLK_DIV,Mclk_Div,Lrck_Div,DSDBCK);
    
}


void vAudioClockSetting(UINT32 sample_rate_idx, UINT32 mclk_fs,UINT32 switch_clk,SPDIF_PCM_BITWIDTH eBitWidth,UINT32 DSDBCK,UINT32 value)
{

	enum afe_sampling_rate sample_idx = sample_rate_idx;
	APLL_DIV_IOMUX_SELECT_T eMuxSelect;
	UINT32  Sample_Rate = 0;


	pr_debug("%s() sample_rate_idx = %d,mclk_fs = %d,switch_clk = %d,eBitWidth = %d,DSDBCK = %d,value = %d\n",
		      __func__,sample_rate_idx,mclk_fs,switch_clk,eBitWidth,DSDBCK,value);

	Sample_Rate = fs_integer(sample_idx);;
	eMuxSelect = vGetApllbySampleRate(Sample_Rate);//// Select APLL1 or APLL2 -> apll1
	switch(switch_clk)
	{
		case APLL_SPDIF1_CK:// 0
			vSetRegisterForSpdif1(eMuxSelect,mclk_fs,sample_idx,value);
		    break;
		case APLL_SPDIF2_CK:// 1
			vSetRegisterForSpdif2(eMuxSelect,mclk_fs,sample_idx,value);
            break;
		case APLL_HDMI_CK:// 2
            vSetRegisterForHdmi(eMuxSelect,mclk_fs,sample_idx,eBitWidth,DSDBCK,value);
            break;
		default:
            pr_err("%s() default invalid switch_clk\n", __func__);
			break;
	}

}

void set_hdmi_out_control(unsigned int channels,HDMI_INPUT_DATA_BIT_WIDTH input_bit)
{
	unsigned int register_value = 0;
    pr_debug("%s() channels= %d\n", __func__,channels);
	register_value |= (channels << 4);
	register_value |= (input_bit << 1);// inputdata dram source
	afe_msk_write(AFE_HDMI_OUT_CON0, register_value, 0xffffffff);
}

void set_hdmi_out_control_enable(bool enable)
{
	unsigned int register_value = 0;
	if (enable)
		register_value |= 1;
	afe_msk_write(AFE_HDMI_OUT_CON0, register_value, 0x1);
}

void set_hdmi_i2s(void)
{
	unsigned int register_value = 0;
	register_value |= (PCM_OUTPUT_32BIT << 4);// 64cycle output data
	register_value |= (HDMI_I2S_FIRST_BIT_1T_DELAY << 3);
	register_value |= (HDMI_I2S_LRCK_INVERSE << 2);//inverse
	register_value |= (HDMI_I2S_BCLK_INVERSE << 1);
	afe_msk_write(AFE_8CH_I2S_OUT_CON, register_value, 0xffffffff);
	afe_msk_write(AUDIO_TOP_CON3, 0 << 6, SPEAKER_OUT_HDMI_SEL_MASK);
}

void set_hdmi_i2s_enable(bool enable)
{
	unsigned int register_value = 0;
	if (enable)
		register_value |= 1;
	afe_msk_write(AFE_8CH_I2S_OUT_CON, register_value, 0x1);
}
void set_hdmi_i2s_to_I2S5(void)
{
	unsigned int register_value = 0;
	register_value |= (SPEAKER_CLOCK_AND_DATA_FROM_HDMI);
	afe_msk_write(AUDIO_TOP_CON3, register_value, SPEAKER_CLOCK_AND_DATA_FROM_MASK);
}
void reset_hdmi_dma_buffer(enum mt_afe_mem_context mem_context,struct snd_pcm_runtime *runtime,struct mt_stream *s)
{
	
	if (likely(mem_context < MT_AFE_MEM_COUNT))
	{
		/*reset DMA buffer*/
	    memset(runtime->dma_area,0,runtime->dma_bytes);
            pr_debug("%s() mem_context =[%d]\n",__func__,mem_context);   
		switch(mem_context)
		{
			case MT_AFE_MEM_I2S:
                s->pointer = 0;
			    break;
			case MT_AFE_MEM_IEC1:
			    s->pointer = 0;
			    break;
            case MT_AFE_MEM_IEC2:
    			s->pointer = 0;
    			break;
			default:
				pr_err("%s() default unexpected mem_context = %d\n", __func__, mem_context);
			break;
		}
	}
	else
	{
		pr_err("%s() unexpected mem_context = %d\n", __func__, mem_context);
	}

}
void init_hdmi_dma_buffer(enum mt_afe_mem_context mem_context, struct snd_pcm_runtime *runtime,struct mt_stream *s)
{
	if((mem_context >= MT_AFE_MEM_COUNT)||(runtime == NULL))
	{
		pr_err("%s() unexpected mem_context = %d\n", __func__, mem_context);
		return;
	}
	switch(mem_context)
	{
		case MT_AFE_MEM_I2S:
            s->pointer = 0;
			afe_msk_write(AFE_HDMI_OUT_BASE, runtime->dma_addr, 0xffffffff);
			afe_msk_write(AFE_HDMI_OUT_END, runtime->dma_addr + (runtime->dma_bytes- 1), 0xffffffff);
		    break;
		case MT_AFE_MEM_IEC1:
            s->pointer = 0;
            iec_nsadr = runtime->dma_addr;
		    afe_msk_write(AFE_SPDIF_BASE, runtime->dma_addr, 0xffffffff);
        	afe_msk_write(AFE_SPDIF_END,runtime->dma_addr + runtime->dma_bytes, 0xffffffff);
		    break;
         case MT_AFE_MEM_IEC2:
            s->pointer = 0;
            iec2_nsadr = runtime->dma_addr;
		    afe_msk_write(AFE_SPDIF2_BASE, runtime->dma_addr, 0xffffffff);
        	afe_msk_write(AFE_SPDIF2_END,runtime->dma_addr + runtime->dma_bytes, 0xffffffff);
		    break;
		default:
			pr_err("%s() Default unexpected mem_context = %d\n", __func__, mem_context);
			break;
	}

}
void set_data_output_from_iec_enable(UINT32 sample_rate_idx,struct snd_pcm_runtime *runtime,int data_type)

{
    pr_debug("%s() sample_rate_idx = %d,channels = %d,sample_bits = %d\n",
             __func__, sample_rate_idx,runtime->channels,runtime->sample_bits);
	/* Pb buffer Size Settings*/
	afe_msk_write(AFE_MEMIF_PBUF_SIZE, AFE_MEMIF_IEC_PBUF_SIZE_128BYTES, AFE_MEMIF_IEC_PBUF_SIZE_MASK);
	/* SPDIF1 to HDMI Tx */
	afe_msk_write(AUDIO_TOP_CON3, HDMI_IEC_FROM_SEL_SPDIF, HDMI_IEC_FROM_SEL_MASK);
#if 1// test hdmi-iec1-output from spdif
    /* SPDIF1 to spdif */
    afe_msk_write(AUDIO_TOP_CON3, PAD_IEC_FROM_SEL_SPDIF, PAD_IEC_FROM_SEL_MASK);// hdmi spdif data to spdif (PAD)
#endif
	/*Set Spdif1 clock source*/
	afe_msk_write(AFE_HDMI_CONN0, SPDIF_LRCK_SRC_SEL_SPDIF, SPDIF_LRCK_SRC_SEL_MASK);
	/*set NSUM*/
	afe_msk_write(AFE_IEC_NSNUM, (((runtime->period_size / 2) << 16) | runtime->period_size), IEC_NEXT_SAM_NUM_MASK|IEC_INT_SAM_NUM_MASK);
    /* set IEC burst length (bits), assign period bytes */
	afe_msk_write(AFE_IEC_BURST_LEN, frames_to_bytes(runtime, runtime->period_size) * 8,IEC_BURST_LEN_MASK);
	/* set IEC burst info = 0 (PCM) */
	afe_msk_write(AFE_IEC_BURST_INFO, 0x0000, IEC_BURST_INFO_MASK);   
    /*set channelstatus */
	afe_msk_write(AFE_IEC_CHL_STAT0, channel_status_raw_pcm[sample_rate_idx], 0xffffffff);
    afe_msk_write(AFE_IEC_CHR_STAT0, channel_status_raw_pcm[sample_rate_idx], 0xffffffff);
    afe_msk_write(AFE_IEC_CHL_STAT1, 0x0, 0x0000ffff);
    afe_msk_write(AFE_IEC_CHR_STAT1, 0x0, 0x0000ffff);	
	/*Set Spdif1 memif and clock on*/
	afe_msk_write(AFE_SPDIF_OUT_CON0, SPDIF_OUT_CLOCK_ON|SPDIF_OUT_MEMIF_ON, SPDIF_OUT_CLOCK_ON_OFF_MASK|SPDIF_OUT_MEMIF_ON_OFF_MASK);
	/*Set isadr*/
	afe_msk_write(AFE_IEC_NSADR,runtime->dma_addr,0xffffffff);
	//Set IEC1 Ready Bit
	afe_msk_write(AFE_IEC_BURST_INFO, IEC_BURST_READY_NOT_READY, IEC_BURST_READY_MASK);
	/* delay for prefetch */
	udelay(2000);		/* 2 ms */
	// IEC CONFIG SETTING
    afe_msk_write(AFE_IEC_CFG,
                              IEC_RAW_24BIT_SWITCH_MODE_OFF|IEC_RAW_24BIT_MODE_OFF|(iec_force_updata_size[sample_rate_idx] << 24)|IEC_RAW_SEL_RAW|IEC_PCM_SEL_PCM|(iec_force_updata_size[sample_rate_idx] << 24)|IEC_FORCE_UPDATE_MASK|IEC_VALID_DATA_MASK|0x0|IEC_SW_RST_MASK,
                              IEC_RAW_24BIT_SWITCH_MODE_MASK|IEC_RAW_24BIT_MODE_MASK|IEC_FORCE_UPDATE_SIZE_MASK|IEC_RAW_SEL_MASK|IEC_PCM_SEL_MASK|IEC_FORCE_UPDATE_SIZE_MASK|IEC_FORCE_UPDATE_MASK|IEC_VALID_DATA_MASK|IEC_SW_RST_MASK|IEC_SW_RST_MASK);
	afe_msk_write(AFE_IEC_CFG, IEC_EN_MASK, IEC_EN_MASK);

}
void set_data_output_from_iec_disable(void)
{
	/* disable IEC */
	afe_msk_write(AFE_IEC_CFG, IEC_INVALID_DATA, IEC_VALID_DATA_MASK);
	afe_msk_write(AFE_IEC_CFG, IEC_RAW_SEL_COOK, IEC_RAW_SEL_MASK);
	while (1 == (afe_read(AFE_IEC_CFG) & 0x10000))
	{
		pr_err("%s() IEC_EN bit\n", __func__);
	}
	/* disable MemIF for SPDIF out */
	afe_msk_write(AFE_SPDIF_OUT_CON0, SPDIF_OUT_CLOCK_OFF|SPDIF_OUT_MEMIF_OFF, SPDIF_OUT_CLOCK_ON_OFF_MASK|SPDIF_OUT_MEMIF_ON_OFF_MASK);
}

void set_data_output_from_iec2_enable(UINT32 sample_rate_idx,struct snd_pcm_runtime *runtime,int data_type)
{
    pr_debug("%s() sample_rate_idx = %d,channels = %d,sample_bits = %d\n",
              __func__, sample_rate_idx,runtime->channels,runtime->sample_bits);

    /* set IEC2 prefetch buffer size : 128 bytes */
    afe_msk_write(AFE_MEMIF_PBUF_SIZE, AFE_MEMIF_IEC_PBUF_SIZE_128BYTES, AFE_MEMIF_IEC_PBUF_SIZE_MASK);
	/* use SPDIF2 LRCK */
    afe_msk_write(AFE_HDMI_CONN0, SPDIF2_LRCK_SRC_SEL_SPDIF, SPDIF2_LRCK_SRC_SEL_MASK);// inter hw
    /*set nsum */
    afe_msk_write(AFE_IEC2_NSNUM, (((runtime->period_size / 2) << 16) | runtime->period_size), 0xffffffff);
    // Set IEC burst length
    afe_msk_write(AFE_IEC2_BURST_LEN, frames_to_bytes(runtime, runtime->period_size) * 8, IEC2_BURST_LEN_MASK);
    // Set IEC burst info as pcm and rawpcm
    afe_msk_write(AFE_IEC2_BURST_INFO, 0x0000, IEC2_BURST_INFO_MASK);
    if(data_type == 0)//pcm mode
    {   
        //Set channel status
        afe_msk_write(AFE_IEC2_CHL_STAT0, channel_status_pcm[sample_rate_idx], 0xffffffff);
        afe_msk_write(AFE_IEC2_CHR_STAT0, channel_status_pcm[sample_rate_idx], 0xffffffff);
        if(runtime->sample_bits == 16)
        {
            afe_msk_write(AFE_IEC2_CHL_STAT1, 0x4, 0x0000ffff);
            afe_msk_write(AFE_IEC2_CHR_STAT1, 0x4, 0x0000ffff);
        }
        else if(runtime->sample_bits > 16)// 24bit
        {
            afe_msk_write(AFE_IEC2_CHL_STAT1, 0xB, 0x0000ffff);
            afe_msk_write(AFE_IEC2_CHR_STAT1, 0xB, 0x0000ffff);
        }       
    }else if(data_type == 1){//rawpcm mode    
        afe_msk_write(AFE_IEC2_CHL_STAT0, channel_status_raw_pcm[sample_rate_idx], 0xffffffff);
        afe_msk_write(AFE_IEC2_CHR_STAT0, channel_status_raw_pcm[sample_rate_idx], 0xffffffff);
        afe_msk_write(AFE_IEC2_CHL_STAT1, 0x0, 0x0000ffff);
        afe_msk_write(AFE_IEC2_CHR_STAT1, 0x0, 0x0000ffff);      
     }
    else{
        pr_err("%s() invaled data_type = %d\n", __func__, data_type);
    }
    /* SPDIF2 to PAD */
    afe_msk_write(AUDIO_TOP_CON3, PAD_IEC_FROM_SEL_SPDIF2, PAD_IEC_FROM_SEL_SPDIF);
    //Set Spdif2 memif and clock on
    afe_msk_write(AFE_SPDIF2_OUT_CON0, SPDIF2_OUT_CLOCK_ON|SPDIF2_OUT_MEMIF_ON, SPDIF2_OUT_CLOCK_ON_OFF_MASK|SPDIF2_OUT_MEMIF_ON_OFF_MASK);
    //Set AFE On
    afe_msk_write(AFE_DAC_CON0, AFE_ON, AFE_ON);
    /* set IEC NSADR (1st time) */
    afe_msk_write(AFE_IEC2_NSADR, runtime->dma_addr,0xffffffff);
    //Set IEC2 Ready Bit
    afe_msk_write(AFE_IEC2_BURST_INFO, IEC2_BURST_READY_NOT_READY, IEC2_BURST_READY_MASK);
	/* delay for prefetch */
	udelay(2000);		/* 2 ms */
    // Set IEC Config
    afe_msk_write(AFE_IEC2_CFG,
                              IEC2_RAW_24BIT_SWITCH_MODE_OFF|IEC2_RAW_24BIT_MODE_OFF|(iec_force_updata_size[sample_rate_idx] << 24)|IEC2_RAW_SEL_RAW|IEC2_REG_LOCK_EN_MASK|(iec_force_updata_size[sample_rate_idx] << 24)|IEC2_FORCE_UPDATE_MASK|IEC2_VALID_DATA_MASK|0x0|IEC2_SW_RST_MASK,
                              IEC2_RAW_24BIT_SWITCH_MODE_MASK|IEC2_RAW_24BIT_MODE_MASK|IEC2_FORCE_UPDATE_SIZE_MASK|IEC2_RAW_SEL_MASK|IEC2_REG_LOCK_EN_MASK|IEC2_FORCE_UPDATE_SIZE_MASK|IEC2_FORCE_UPDATE_MASK|IEC2_VALID_DATA_MASK|IEC2_SW_RST_MASK|IEC2_SW_RST_MASK);
    if(runtime->sample_bits == 24)
    {
         afe_msk_write(AFE_IEC2_CFG,
                       IEC2_RAW_24BIT_MODE_ON|IEC2_RAW_24BIT_SWITCH_MODE_ON,
                       IEC2_RAW_24BIT_MODE_MASK|IEC2_RAW_24BIT_SWITCH_MODE_MASK);
    }
    afe_msk_write(AFE_IEC2_CFG, IEC2_EN_MASK, IEC2_EN_MASK);
}
void set_data_output_from_iec2_disable(void)
{
	/* disable IEC */
	afe_msk_write(AFE_IEC2_CFG, IEC_INVALID_DATA, IEC_VALID_DATA_MASK);
	afe_msk_write(AFE_IEC2_CFG, IEC_RAW_SEL_COOK, IEC_RAW_SEL_MASK);
	while (1 == (afe_read(AFE_IEC2_CFG) & 0x10000))
	{
		pr_err("%s() iec2_en bit\n", __func__);
	}
	/* disable MemIF for SPDIF out */
	afe_msk_write(AFE_SPDIF2_OUT_CON0, SPDIF_OUT_CLOCK_OFF|SPDIF_OUT_MEMIF_OFF, SPDIF_OUT_CLOCK_ON_OFF_MASK|SPDIF_OUT_MEMIF_ON_OFF_MASK);
}



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



#ifndef _AUDIO_HDMI_CONTROL_H
#define _AUDIO_HDMI_CONTROL_H
#include <sound/soc.h>
#include "mt_typedefs.h"
#include"mt_clkmgr.h"
/*
#define  DEBUG_INFO
#ifdef DEBUG_INFO
#define  pr_debug(x...) printk("[mt7623_hdmi_pcm]::"x)
#define  pr_error(x...) printk("[mt7623_hdmi_pcm]::"x)
#define  pr_notice(x...) printk("[mt7623_hdmi_pcm]::"x)
#else
#define  pr_debug(x...) do{}while(0);
#define  pr_error(x...) do{}while(0);
#define  pr_notice(x...) do{}while(0);
#endif
*/
#define APLL_DIV_CLK_MASK	0x7 << 16
#define HDMI_POWER_UP_MASK  0x1 << 20
        #define HDMI_POWER_UP 0 << 20
		#define HDMI_POWER_DOWN 1 << 20

#define SPDIF1_POWER_UP_MASK  0x1 << 21
        #define SPDIF1_POWER_UP 0 << 21
		#define SPDIF1_POWER_DOWN 1 << 21

#define SPDIF2_POWER_UP_MASK  0x1 << 22
        #define SPDIF2_POWER_UP 0 << 22
		#define SPDIF2_POWER_DOWN 1 << 22

#define HDMISPDIF_POWER_UP_MASK  0x1 << 23
        #define HDMISPDIF_POWER_UP 0 << 23
		#define HDMISPDIF_POWER_DOWN 1 << 23
//HDMI PCM SETTING
#define HDMI_BUFFER_SIZE_MAX    (512*1024)
#define HDMI_PERIOD_SIZE_MIN    64
#define HDMI_PERIOD_SIZE_MAX    HDMI_BUFFER_SIZE_MAX
#define HDMI_FORMATS            (SNDRV_PCM_FMTBIT_S16_LE)
#define HDMI_RATES              (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000| \
									 SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 | \
									 SNDRV_PCM_RATE_192000)
#define HDMI_RATE_MIN           32000
#define HDMI_RATE_MAX           192000
#define HDMI_CHANNELS_MIN       2
#define HDMI_CHANNELS_MAX       8
#define HDMI_PERIODS_MIN        1
#define HDMI_PERIODS_MAX        1024
//HDMI RAW PCM SETTING
#define SPDIF_MAX_BUFFER_SIZE   (256*1024)
#define SPDIF_FORMATS            (SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_LE)
#define SPDIF_RATES              (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
				  SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | \
				  SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 | \
				  SNDRV_PCM_RATE_192000)
#define SPDIF_RATE_MIN           32000
#define SPDIF_RATE_MAX           192000
#define SPDIF_CHANNELS_MIN       2
#define SPDIF_CHANNELS_MAX       8

#define SOC_NORMAL_USE_RATE             (SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000)
#define SOC_NORMAL_USE_RATE_MIN         8000
#define SOC_NORMAL_USE_RATE_MAX         48000
#define SOC_NORMAL_USE_CHANNELS_MIN     1
#define SOC_NORMAL_USE_CHANNELS_MAX     2
#define SOC_NORMAL_USE_PERIODS_MIN      2
#define SOC_NORMAL_USE_PERIODS_MAX      256
#define SOC_NORMAL_USE_PERIOD_SIZE_MIN  (512)



enum mt_afe_mem_context {
	MT_AFE_MEM_I2S = 0,
	MT_AFE_MEM_IEC1,
	MT_AFE_MEM_IEC2,
	MT_AFE_MEM_COUNT
};

typedef enum
{
	PCM_OUTPUT_8BIT = 0,
	PCM_OUTPUT_16BIT = 1,
	PCM_OUTPUT_24BIT = 2,
	PCM_OUTPUT_32BIT = 3

}SPDIF_PCM_BITWIDTH;

typedef enum
{
    APLL_SPDIF1_CK = 0,
    APLL_SPDIF2_CK = 1,
    APLL_HDMI_CK = 2,
} APLL_OUTPUT_SELECT_T;

typedef enum
{
	HDMI_APLL1 = 0,//48K  98.304M
	HDMI_APLL2 = 1 //44.1K 90.3168M
} APLL_DIV_IOMUX_SELECT_T;

typedef enum
{
	HDMI_IN_I20 = 20,
	HDMI_IN_I21,
	HDMI_IN_I22,
	HDMI_IN_I23,
	HDMI_IN_I24,
	HDMI_IN_I25,
	HDMI_IN_I26,
	HDMI_IN_I27,
	HDMI_IN_BASE = HDMI_IN_I20,
	HDMI_IN_MAX = HDMI_IN_I27,
	HDMI_NUM_INPUT = (HDMI_IN_MAX - HDMI_IN_BASE + 1)
}HDMI_SPDIF_TRCON_IN_T;

typedef enum
{
	HDMI_OUT_O20 = 20,
	HDMI_OUT_O21,
	HDMI_OUT_O22,
	HDMI_OUT_O23,
	HDMI_OUT_O24,
	HDMI_OUT_O25,
	HDMI_OUT_O26,
	HDMI_OUT_O27,
	HDMI_OUT_O28,
	HDMI_OUT_O29,
	HDMI_OUT_BASE = HDMI_OUT_O20,
	HDMI_OUT_MAX = HDMI_OUT_O29,
	HDMI_NUM_OUTPUT = (HDMI_OUT_MAX - HDMI_OUT_BASE + 1)
}HDMI_SPDIF_ITRCON_OUT_T;


typedef enum
{
	HDMI_DisConnect = 0x0,
	HDMI_Connection = 0x1
}HDMI_INTERCON_STATUS;

typedef enum
{
	HDMI_INPUT_16BIT = 0,
	HDMI_INPUT_32BIT
}HDMI_INPUT_DATA_BIT_WIDTH;

typedef enum
{
	HDMI_I2S_8BIT = 0,
	HDMI_I2S_16BIT,
	HDMI_I2S_24BIT,
	HDMI_I2S_32BIT
}HDMI_I2S_WLEN;

typedef enum
{
	HDMI_I2S_NOT_DELAY = 0,
	HDMI_I2S_FIRST_BIT_1T_DELAY
}HDMI_I2S_DELAY_DATA;

typedef enum
 {
	HDMI_I2S_LRCK_NOT_INVERSE = 0,
	HDMI_I2S_LRCK_INVERSE
}HDMI_I2S_LRCK_INV;

typedef enum  {
	HDMI_I2S_BCLK_NOT_INVERSE = 0,
	HDMI_I2S_BCLK_INVERSE
}HDMI_I2S_BCLK_INV;

void SetHdmiInterConnection(unsigned int ConnectionState, unsigned int Input, unsigned int Output);

void set_hdmi_out_control(unsigned int channels,HDMI_INPUT_DATA_BIT_WIDTH input_bit);

void set_hdmi_out_control_enable(bool enable);

void set_hdmi_i2s(void);

void set_hdmi_i2s_enable(bool enable);

void set_hdmi_i2s_to_I2S5(void);

void vAudioClockSetting(UINT32 sample_rate_idx, UINT32 mclk_fs,UINT32 switch_clk,SPDIF_PCM_BITWIDTH eBitWidth,UINT32 DSDBCK,UINT32 value);

void init_hdmi_dma_buffer(enum mt_afe_mem_context mem_context, struct snd_pcm_runtime *runtime,struct mt_stream *s);

void reset_hdmi_dma_buffer(enum mt_afe_mem_context mem_context,struct snd_pcm_runtime *runtime,struct mt_stream *s);

void set_data_output_from_iec_enable(UINT32 sample_rate_idx,struct snd_pcm_runtime *runtime,int data_type);

void set_data_output_from_iec_disable(void);

void set_data_output_from_iec2_enable(UINT32 sample_rate_idx,struct snd_pcm_runtime *runtime,int data_type);

void set_data_output_from_iec2_disable(void);

#endif


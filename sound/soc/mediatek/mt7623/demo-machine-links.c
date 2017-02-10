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

#include <mach/upmu_hw.h>
#include <mach/mt_gpio.h>
#include <linux/delay.h>
#include "mt7623-dai.h"
#include "mt7623-private.h"
#include "wm8960.h"

static int pcm_master_data_rate_hw_params(
    struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    //codec slave, mt7623 master
    unsigned int fmt = SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_CBS_CFS|SND_SOC_DAIFMT_CONT| SND_SOC_DAIFMT_NB_NF;
    unsigned int mclk_rate;
    unsigned int rate = params_rate(params); /* data rate */
	unsigned int div_mclk_to_bck = rate > 192000 ? 2 : 4;
	unsigned int div_bck_to_lrck = 64;

    mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
    //codec mclk
	snd_soc_dai_set_clkdiv(codec_dai, WM8960_DACDIV, WM8960_DAC_DIV_1);   
    //codec slave
    snd_soc_dai_set_fmt(codec_dai, fmt);
    //mt7623 mclk
    snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
    //mt7623 bck
    snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
    //mt7623 lrck
    snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
    //mt7623 master
    snd_soc_dai_set_fmt(cpu_dai, fmt);
    return 0;
}

static int pcm_master_fixed_rate_hw_params(
    struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    unsigned int mclk_rate;
    unsigned int div_mclk_to_bck = 4;
    unsigned int div_bck_to_lrck = 64;
    unsigned int rate = 48000; /* fixed rate */
    //codec slave, mt7623 master
    unsigned int fmt = SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_CBS_CFS|SND_SOC_DAIFMT_CONT;
    mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
    //codec mclk
    snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
    //codec slave
    snd_soc_dai_set_fmt(codec_dai, fmt);
    //mt7623 mclk
    snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
    //mt7623 bck
    snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
    //mt7623 lrck
    snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
    //mt7623 master
    snd_soc_dai_set_fmt(cpu_dai, fmt);
    return 0;
}

static int pcm_slave_fixed_rate_hw_params(
    struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    unsigned int mclk_rate;
    unsigned int div_mclk_to_bck = 8;
    unsigned int div_bck_to_lrck = 64;
    unsigned int rate = 96000; /* connect OSC(49.152M) to PCM1861, set PCM1861 to master 512fs */
    //codec master, mt7623 slave
    unsigned int fmt = SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_CBM_CFM|SND_SOC_DAIFMT_CONT;
    mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
    //codec mclk
    snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
    //codec master
    snd_soc_dai_set_fmt(codec_dai, fmt);
    //mt7623 mclk
    snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
    //mt7623 bck
    snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
    //mt7623 lrck
    snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
    //mt7623 slave
    snd_soc_dai_set_fmt(cpu_dai, fmt|SLAVE_USE_ASRC_YES); /* slave PCM can use asrc */
    return 0;
}

static struct snd_soc_ops stream_pcm_master_data_rate_ops = {
    .hw_params = pcm_master_data_rate_hw_params
};

static struct snd_soc_ops stream_pcm_master_fixed_rate_ops = {
    .hw_params = pcm_master_fixed_rate_hw_params
};

static struct snd_soc_ops stream_pcm_slave_fixed_rate_ops = {
    .hw_params = pcm_slave_fixed_rate_hw_params
};

/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link demo_dai_links[] = {
    {
        .name = "demo-MT7623",
        .stream_name = "pcm test",
        .platform_name = "mt7623-audio",
        .cpu_dai_name = "mt7623-i2s1",
        .codec_dai_name = "wm8960-hifi",
        .codec_name = "wm8960.1-001a",
    	.dai_fmt = SND_SOC_DAIFMT_I2S
                 | SND_SOC_DAIFMT_CBS_CFS
                 | SND_SOC_DAIFMT_GATED,
        .ops = &stream_pcm_master_data_rate_ops
    },
#if 0
    {
		.name = "demo-hdmi-pcm-out",
		.stream_name = "hdmi-pcm-out",
		.platform_name = "mt7623-audio",
		.cpu_dai_name = "mt7623-hdmi-pcm-out",
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
#endif
    /* add other link here */
};



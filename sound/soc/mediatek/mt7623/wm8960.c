/*
 * wm8960.c  --  WM8960 ALSA SoC Audio driver
 *
 * Copyright 2007-11 Wolfson Microelectronics, plc
 *
 * Author: Liam Girdwood
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <sound/wm8960.h>
#include <mach/mt_gpio.h>
#include "wm8960.h"

/* R25 - Power 1 */
#define WM8960_VMID_MASK 0x180
#define WM8960_VREF      0x40

/* R26 - Power 2 */
#define WM8960_PWR2_LOUT1	0x40
#define WM8960_PWR2_ROUT1	0x20
#define WM8960_PWR2_OUT3	0x02

/* R28 - Anti-pop 1 */
#define WM8960_POBCTRL   0x80
#define WM8960_BUFDCOPEN 0x10
#define WM8960_BUFIOEN   0x08
#define WM8960_SOFT_ST   0x04
#define WM8960_HPSTBY    0x01

/* R29 - Anti-pop 2 */
#define WM8960_DISOP     0x40
#define WM8960_DRES_MASK 0x30
unsigned long wm_reg_data[56];

struct wm8960_priv {
	struct i2c_client	*client;
	struct device	    *dev;
	const char 			*name;
	int (*set_bias_level)(struct snd_soc_codec *,
			      enum snd_soc_bias_level level);
	struct snd_soc_dapm_widget *lout1;
	struct snd_soc_dapm_widget *rout1;
	struct snd_soc_dapm_widget *out3;
	bool deemph;
	int playback_fs;
};

struct wm8960_priv *wm_client;
static void wm8960_write(u32 reg, u32 data)
{
	int ret;
    struct i2c_msg msg;
	u8 buf[2]={0};
	unsigned int ext_flag = 0;
	
	ext_flag &= 0x7FFFFFFF;
	ext_flag |= I2C_POLLING_FLAG;
	
	wm_reg_data[reg] = data;
	
	buf[0]= (reg<<1)|(0x01&(data>>8));
	buf[1]= (data&0xFF);

	msg.addr = wm_client->client->addr;
    msg.flags = 0;
    msg.buf = (char *)buf;
	msg.len = 2;
	msg.timing = 100;
	msg.ext_flag = ext_flag & 0x7FFFFFFF;

    ret = i2c_transfer(wm_client->client->adapter, &msg, 1);// start transfer
	if (ret < 0)
        printk("%s i2c write error: %d\n", __func__, ret);
}

static int wm8960_update(u32 reg, u32 mask, u32 value)
{
	int change;
	u32 old, new;
	
	old = wm_reg_data[reg];
	new = (old & ~mask) | value;
	change = old != new;
	
	if (change)
		wm8960_write(reg, value);
	return change;
}

static int wm8960_codec_write(struct snd_soc_codec *codec, unsigned int reg, unsigned int data)
{
	wm8960_write(reg,data);
	return 0;
}

static int wm8960_codec_read(struct snd_soc_codec *codec, unsigned int reg)
{
	return wm_reg_data[reg];
}

static const int deemph_settings[] = { 0, 32000, 44100, 48000 };

static int wm8960_set_deemph(struct snd_soc_codec *codec)
{
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);
	int val, i, best;

	/* If we're using deemphasis select the nearest available sample
	 * rate.
	 */
	if (wm8960->deemph) {
		best = 1;
		for (i = 2; i < ARRAY_SIZE(deemph_settings); i++) {
			if (abs(deemph_settings[i] - wm8960->playback_fs) <
			    abs(deemph_settings[best] - wm8960->playback_fs))
				best = i;
		}

		val = best << 1;
	} else {
		val = 0;
	}

	dev_dbg(codec->dev, "Set deemphasis %d\n", val);

	return wm8960_update(WM8960_DACCTL1, 0x6, val);
}


static const DECLARE_TLV_DB_SCALE(adc_tlv, -9700, 50, 0);
static const DECLARE_TLV_DB_SCALE(dac_tlv, -12700, 50, 1);


static const struct snd_kcontrol_new wm8960_snd_controls[] = {
SOC_DOUBLE_R_TLV("Capture Volume", WM8960_LINVOL, WM8960_RINVOL,
		 0, 63, 0, adc_tlv),

SOC_DOUBLE_R_TLV("Playback Volume", WM8960_LDAC, WM8960_RDAC,
		 0, 255, 0, dac_tlv),

};

static int wm8960_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	u16 iface = 0;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface |= 0x0040;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= 0x0002;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface |= 0x0001;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface |= 0x0003;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface |= 0x0013;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface |= 0x0090;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface |= 0x0080;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		iface |= 0x0010;
		break;
	default:
		return -EINVAL;
	}
	
	/* set iface */
	wm8960_write(WM8960_IFACE1, iface);
	return 0;
}

static struct {
	int rate;
	unsigned int val;
} alc_rates[] = {
	{ 48000, 0 },
	{ 44100, 0 },
	{ 32000, 1 },
	{ 22050, 2 },
	{ 24000, 2 },
	{ 16000, 3 },
	{ 11250, 4 },
	{ 12000, 4 },
	{  8000, 5 },
};

static int wm8960_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);
	u32 iface = wm_reg_data[WM8960_IFACE1] & 0xfff3;
	snd_pcm_format_t format = params_format(params);
	int i;

	/* bit size */
	switch (format) {
	case SNDRV_PCM_FORMAT_S16_LE:
	case SNDRV_PCM_FORMAT_S16_BE:
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
	case SNDRV_PCM_FORMAT_S20_3BE:
		iface |= 0x0004;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S24_BE:
		iface |= 0x0008;
		break;
	default:
		dev_err(codec->dev, "unsupported format %i\n", format);
		return -EINVAL;
	}

	/* Update filters for the new rate */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		wm8960->playback_fs = params_rate(params);
		wm8960_set_deemph(codec);
	} else {
		for (i = 0; i < ARRAY_SIZE(alc_rates); i++)
			if (alc_rates[i].rate == params_rate(params))
				wm8960_update(WM8960_ADDCTL3, 0x7, alc_rates[i].val);
	}

	/* set iface */
	wm8960_write(WM8960_IFACE1, iface);
	return 0;
}

static int wm8960_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	u32 data = wm_reg_data[WM8960_DACCTL1] & 0xfff7;

	if (mute)
		wm8960_write(WM8960_DACCTL1, 0x8);
	else
		wm8960_write(WM8960_DACCTL1, data);
	return 0;
}

static int wm8960_set_bias_level_out3(struct snd_soc_codec *codec,
				      enum snd_soc_bias_level level)
{
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);
	u32 data;
	int i;

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:	
		break;
	case SND_SOC_BIAS_STANDBY:
	printk("%s SND_SOC_BIAS_STANDBY\n", __func__);
		wm8960_write(WM8960_RESET, 0);
		memset(wm_reg_data, 0 , sizeof(unsigned long)*55);		
		mdelay(500);	
		wm8960_write(WM8960_IFACE1, AINTFCE1_WL_24|AINTFCE1_FORMAT_I2S);//0x07

		// In
		data = wm_reg_data[WM8960_POWER1];
		wm8960_write(WM8960_POWER1, data|WM8960_PWR1_ADCL|WM8960_PWR1_ADCR|WM8960_PWR1_AINL |WM8960_PWR1_AINR|WM8960_PWR1_MICB);//0x19
		data = wm_reg_data[WM8960_ADDCTL1];
		wm8960_write(WM8960_ADDCTL1, data|ADDITIONAL1_DATSEL(0x01));//0x17
		wm8960_write(WM8960_LADC, LEFTGAIN_LDVU|LEFTGAIN_LDACVOL(0xc3));//0x15
		wm8960_write(WM8960_RADC, LEFTGAIN_LDVU|LEFTGAIN_LDACVOL(0xc3));//0x16
		wm8960_write(WM8960_LINPATH, 0x148);//0x20
		wm8960_write(WM8960_RINPATH, 0x148);//0x21
		wm8960_write(WM8960_POWER3, WM8960_PWR3_LMIC|WM8960_PWR3_RMIC);//0x2f
		
		// Out
		data = wm_reg_data[WM8960_POWER2];
		wm8960_write(WM8960_POWER2, data|WM8960_PWR2_DACL|WM8960_PWR2_DACR|WM8960_PWR2_LOUT1|WM8960_PWR2_ROUT1|WM8960_PWR2_SPKL|WM8960_PWR2_SPKR);//0x1a
		mdelay(10);	
		wm8960_write(WM8960_IFACE2, 0x40);
		wm8960_write(WM8960_LDAC, LEFTGAIN_LDVU|LEFTGAIN_LDACVOL(0xff));//0x0a
		wm8960_write(WM8960_RDAC, RIGHTGAIN_RDVU|RIGHTGAIN_RDACVOL(0xff));//0x0b
		wm8960_write(WM8960_LOUTMIX, 0x100);//0x22
		wm8960_write(WM8960_ROUTMIX, 0x100);//0x25

		data = wm_reg_data[WM8960_POWER3];
		wm8960_write(WM8960_POWER3, data|WM8960_PWR3_ROMIX|WM8960_PWR3_LOMIX);//0x2f

		wm8960_write(WM8960_CLASSD1, 0xf7);//0x31
		wm8960_write(WM8960_CLASSD3, 0xad);//0x33
		wm8960_write(WM8960_DACCTL1,  0x000);//0x05

		data = wm_reg_data[WM8960_POWER1];
		wm8960_write(WM8960_POWER1,  data|0x1c0);//0x19
		
		wm8960_write(WM8960_LOUT1, LOUT1_LO1VU|LOUT1_LO1ZC|LOUT1_LOUT1VOL(115));//0x02
		wm8960_write(WM8960_ROUT1, ROUT1_RO1VU|ROUT1_RO1ZC|ROUT1_ROUT1VOL(115));//0x03	
			
		wm8960_write(WM8960_LINVOL, LINV_IPVU|LINV_LINVOL(110)); //LINV(0x00)=>0x12b 
		wm8960_write(WM8960_RINVOL, RINV_IPVU|RINV_RINVOL(110)); //LINV(0x01)=>0x12b 
		break;
	case SND_SOC_BIAS_OFF:
		wm8960_write(WM8960_DACCTL1,0x8); //0x05->0x08 
		wm8960_write(WM8960_POWER1, 0x000); //0x19->0x000 
		mdelay(300);
		wm8960_write(WM8960_POWER2, 0x000); //0x1a->0x000 
		break;
	}

	codec->dapm.bias_level = level;

	return 0;
}

/* PLL divisors */
struct _pll_div {
	u32 pre_div:1;
	u32 n:4;
	u32 k:24;
};

/* The size in bits of the pll divide multiplied by 10
 * to allow rounding later */
#define FIXED_PLL_SIZE ((1 << 24) * 10)

static int pll_factors(unsigned int source, unsigned int target,
		       struct _pll_div *pll_div)
{
	unsigned long long Kpart;
	unsigned int K, Ndiv, Nmod;

	pr_debug("WM8960 PLL: setting %dHz->%dHz\n", source, target);

	/* Scale up target to PLL operating frequency */
	target *= 4;

	Ndiv = target / source;
	if (Ndiv < 6) {
		source >>= 1;
		pll_div->pre_div = 1;
		Ndiv = target / source;
	} else
		pll_div->pre_div = 0;

	if ((Ndiv < 6) || (Ndiv > 12)) {
		pr_err("WM8960 PLL: Unsupported N=%d\n", Ndiv);
		return -EINVAL;
	}

	pll_div->n = Ndiv;
	Nmod = target % source;
	Kpart = FIXED_PLL_SIZE * (long long)Nmod;

	do_div(Kpart, source);

	K = Kpart & 0xFFFFFFFF;

	/* Check if we need to round */
	if ((K % 10) >= 5)
		K += 5;

	/* Move down to proper range now rounding is done */
	K /= 10;

	pll_div->k = K;

	pr_debug("WM8960 PLL: N=%x K=%x pre_div=%d\n",
		 pll_div->n, pll_div->k, pll_div->pre_div);

	return 0;
}

static int wm8960_set_dai_pll(struct snd_soc_dai *codec_dai, int pll_id,
		int source, unsigned int freq_in, unsigned int freq_out)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 reg;
	static struct _pll_div pll_div;
	int ret;

	if (freq_in && freq_out) {
		ret = pll_factors(freq_in, freq_out, &pll_div);
		if (ret != 0)
			return ret;
	}

	/* Disable the PLL: even if we are changing the frequency the
	 * PLL needs to be disabled while we do so. */
	wm8960_update(WM8960_CLOCK1, 0x1, 0);
	wm8960_update(WM8960_POWER2, 0x1, 0);

	if (!freq_in || !freq_out)
		return 0;

	reg = snd_soc_read(codec, WM8960_PLL1) & ~0x3f;
	reg |= pll_div.pre_div << 4;
	reg |= pll_div.n;

	if (pll_div.k) {
		reg |= 0x20;

		wm8960_write(WM8960_PLL2, (pll_div.k >> 16) & 0xff);
		wm8960_write(WM8960_PLL3, (pll_div.k >> 8) & 0xff);
		wm8960_write(WM8960_PLL4, pll_div.k & 0xff);
	}
	wm8960_write(WM8960_PLL1, reg);

	/* Turn it on */
	wm8960_update(WM8960_POWER2, 0x1, 0x1);
	msleep(250);
	wm8960_update(WM8960_CLOCK1, 0x1, 0x1);

	return 0;
}

static int wm8960_set_dai_clkdiv(struct snd_soc_dai *codec_dai,
		int div_id, int div)
{
	u32 data;
	
	switch (div_id) {
	case WM8960_SYSCLKDIV:
		data = wm_reg_data[WM8960_CLOCK1] & 0x1f9;
		wm8960_write(WM8960_CLOCK1, data | div);
		break;
	case WM8960_DACDIV:
		data = wm_reg_data[WM8960_CLOCK1];
		wm8960_write(WM8960_CLOCK1, data | div);
		break;
	case WM8960_OPCLKDIV:
		data = wm_reg_data[WM8960_PLL1] & 0x03f;
		wm8960_write(WM8960_PLL1, data | div);
		break;
	case WM8960_DCLKDIV:
		data = wm_reg_data[WM8960_CLOCK2] & 0x03f;
		wm8960_write(WM8960_CLOCK2, data | div);
		break;
	case WM8960_TOCLKSEL:
		data = wm_reg_data[WM8960_ADDCTL1] & 0x1fd;
		wm8960_write(WM8960_ADDCTL1, data | div);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int wm8960_set_bias_level(struct snd_soc_codec *codec,
				 enum snd_soc_bias_level level)
{
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);

	return wm8960->set_bias_level(codec, level);
}

#define WM8960_RATES SNDRV_PCM_RATE_8000_48000

#define WM8960_FORMATS \
	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
	SNDRV_PCM_FMTBIT_S24_LE )

static const struct snd_soc_dai_ops wm8960_dai_ops = {
	.hw_params = wm8960_hw_params,
	.digital_mute = wm8960_mute,
	.set_fmt = wm8960_set_dai_fmt,
	.set_clkdiv = wm8960_set_dai_clkdiv,
	.set_pll = wm8960_set_dai_pll,
};

static struct snd_soc_dai_driver wm8960_dai = {
	.name = "wm8960-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8960_RATES,
		.formats = WM8960_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8960_RATES,
		.formats = WM8960_FORMATS,},
	.ops = &wm8960_dai_ops,
	.symmetric_rates = 1,
};

static int wm8960_suspend(struct snd_soc_codec *codec)
{
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);

	wm8960->set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int wm8960_resume(struct snd_soc_codec *codec)
{
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);

	wm8960->set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	return 0;
}

static int wm8960_probe(struct snd_soc_codec *codec)
{
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);

	int ret, i, j;
	u32 data;
	printk("%s wm8960_probe\n", __func__);
	wm8960->set_bias_level = wm8960_set_bias_level_out3;

	wm8960->set_bias_level(codec, SND_SOC_BIAS_STANDBY);

	snd_soc_add_codec_controls(codec, wm8960_snd_controls,
				     ARRAY_SIZE(wm8960_snd_controls));


	return 0;
}

/* power down chip */
static int wm8960_remove(struct snd_soc_codec *codec)
{
	struct wm8960_priv *wm8960 = snd_soc_codec_get_drvdata(codec);

	wm8960->set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_wm8960 = {
	.probe =	wm8960_probe,
	.remove =	wm8960_remove,
	.suspend =	wm8960_suspend,
	.resume =	wm8960_resume,
	.set_bias_level = wm8960_set_bias_level,
	.read = wm8960_codec_read,
	.write = wm8960_codec_write,
};

static int wm8960_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct wm8960_priv *wm8960;
	int ret;
	printk("%s wm8960_i2c_probe\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	return -EIO;

	wm8960 = devm_kzalloc(&client->dev, sizeof(struct wm8960_priv),
			      GFP_KERNEL);
	if (wm8960 == NULL)
		return -ENOMEM;
	
	wm8960->client = client;
	wm8960->dev = &client->dev;
	wm8960->name = id->name;
	
	i2c_set_clientdata(client, wm8960);
	wm_client = wm8960;
	
	ret = snd_soc_register_codec(&client->dev,
			&soc_codec_dev_wm8960, &wm8960_dai, 1);

	return ret;
}

static int wm8960_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_codec(&client->dev);
	return 0;
}

static struct i2c_board_info __initdata mt_audio_i2c_devs_info[] = {
    {
        I2C_BOARD_INFO("wm8960", (0x34>>1)),
    }
};

static const struct i2c_device_id wm8960_i2c_id[] = {
	{ "wm8960", 0 },
	{}
};

static struct i2c_driver wm8960_i2c_driver = {
	.driver = {
		.name = "wm8960",
		.owner = THIS_MODULE,
	},
	.probe =    wm8960_i2c_probe,
	.remove =   wm8960_i2c_remove,
	.id_table = wm8960_i2c_id,
};

static int __init wm8960_i2c_init(void)
{
	mt_set_gpio_mode(GPIO57, GPIO_MODE_00);
	mt_set_gpio_mode(GPIO58, GPIO_MODE_00);
	mt_set_gpio_mode(GPIO242, GPIO_MODE_04);
    mt_set_gpio_mode(GPIO243, GPIO_MODE_04);
	
	i2c_register_board_info(1, mt_audio_i2c_devs_info, ARRAY_SIZE(mt_audio_i2c_devs_info));
	
	return i2c_add_driver(&wm8960_i2c_driver);; 
}

static void __exit wm8960_i2c_exit(void)
{
	i2c_del_driver(&wm8960_i2c_driver);
}

module_init(wm8960_i2c_init);
module_exit(wm8960_i2c_exit);

MODULE_DESCRIPTION("ASoC WM8960 driver");

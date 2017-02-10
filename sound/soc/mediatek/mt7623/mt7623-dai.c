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

#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <sound/soc.h>
#include "mt7623-dai.h"
#include "mt7623-dai-private.h"


extern struct snd_soc_dai_ops mt7623_i2s_ops;


int mt7623_i2s_probe(struct snd_soc_dai *dai);
int mt7623_dmic_probe(struct snd_soc_dai *dai);

static struct snd_soc_component_driver mt7623_soc_component_driver = {
	.name = "mt7623-dai"
};

static struct snd_soc_dai_driver mt7623_soc_dai_drivers[] = {
    {
        .name = "mt7623-i2s1",
        .id = MT7623_DAI_I2S1_ID,
        .probe = mt7623_i2s_probe,
        .ops = &mt7623_i2s_ops,
        .playback = {
            .stream_name = "mt7623-i2s-out1",
            .channels_min = 1,
            .channels_max = 2,
            .rates = SNDRV_PCM_RATE_8000_384000,
            .formats = (SNDRV_PCM_FMTBIT_S16_LE
            | SNDRV_PCM_FMTBIT_S32_LE
            | SNDRV_PCM_FMTBIT_DSD_U8
            | SNDRV_PCM_FMTBIT_DSD_U16_LE)
        },
        .capture = {
            .stream_name = "mt7623-i2s-in1",
            .channels_min = 1,
            .channels_max = 2,
            .rates = SNDRV_PCM_RATE_8000_384000,
            .formats = (SNDRV_PCM_FMTBIT_S16_LE
            | SNDRV_PCM_FMTBIT_S32_LE
            | SNDRV_PCM_FMTBIT_DSD_U8
            | SNDRV_PCM_FMTBIT_DSD_U16_LE)
        }
    },
    {
	   .name =  "mt7623-hdmi-pcm-out",
	   .id = MT7623_DAI_HDMI_OUT_I2S_ID,
	   .ops = NULL,
	   .playback = {
		   .stream_name = "hdmi-pcm-out",
		   .channels_min = 1,
		   .channels_max = 8,
		   .rates = SNDRV_PCM_RATE_8000_384000,
		   .formats = (SNDRV_PCM_FMTBIT_S16_LE
			|SNDRV_PCM_FMTBIT_S24_LE
			|SNDRV_PCM_FMTBIT_S32_LE)
	   },
	 },
    /* add other dai here */
};

static int mt7623_dai_probe(struct platform_device *pdev)
{
	struct mt_dai_private *priv;

	pr_debug("%s()\n", __func__);
	priv = devm_kzalloc(&pdev->dev, sizeof(struct mt_dai_private), GFP_KERNEL);
	if (!priv) {
		dev_err(&pdev->dev, "%s() can't allocate memory\n", __func__);
		return -ENOMEM;
	}
	dev_set_drvdata(&pdev->dev, priv);
	return snd_soc_register_component(&pdev->dev
					 , &mt7623_soc_component_driver
					 , mt7623_soc_dai_drivers
					 , ARRAY_SIZE(mt7623_soc_dai_drivers));
}

static int mt7623_dai_remove(struct platform_device *pdev)
{
	struct mt_dai_private *priv;
	pr_debug("%s()\n", __func__);
	priv = dev_get_drvdata(&pdev->dev);
	devm_kfree(&pdev->dev, priv);
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static struct platform_driver mt7623_dai = {
	.driver = {
	.name = "mt7623-dai",
	.owner = THIS_MODULE,
	},
	.probe = mt7623_dai_probe,
	.remove = mt7623_dai_remove
};

module_platform_driver(mt7623_dai);

MODULE_DESCRIPTION("mt7623 dai driver");
MODULE_LICENSE("GPL");


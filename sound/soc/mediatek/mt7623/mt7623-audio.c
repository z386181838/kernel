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
#include <sound/pcm_params.h>
#include "mt7623-dai.h"
#include "mt7623-afe.h"
#include "mt7623-private.h"

extern struct snd_pcm_ops memif_ops;
extern struct snd_pcm_ops hdmi_pcm_ops;
void memif_isr(struct mt_stream *s);
void hdmi_pcm_isr(struct mt_stream *s);

struct hwgain_info {
	u32 hgctl_0;
	u32 hgctl_1;
};

struct hwgain_info hwgaininfo;

static int hwgain_control_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0x80000;
	return 0;
}

static int hwgain_control_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = hwgaininfo.hgctl_0;
	ucontrol->value.integer.value[1] = hwgaininfo.hgctl_1;
	return 0;
}

/*
   hgctl0
     bit0: hwgainID 0:AFE_HWGAIN_1, 1:AFE_HWGAIN_2
     bit1: hwgain: enable/disable
     bit2~bit9: hwgainsample per step (0~255)
     bit10~bit11: hwgainsetpdb: 0:0.125dB, 1:0.25dB, 2:0.5dB

   hgctl1
     hwgain: 0x0[-inf.dB]~0x80000[0dB]
*/
static int hwgain_control_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	u32 hgctl_0 = ucontrol->value.integer.value[0];
	u32 hgctl_1 = ucontrol->value.integer.value[1];
	enum afe_hwgain_id hgid = hgctl_0 & 0x1;
	int hgenable = (hgctl_0 & 0x2) >> 1;
	afe_hwgain_configurate(hgctl_0, hgctl_1);
	afe_hwgain_enable(hgid, hgenable);
	hwgaininfo.hgctl_0 = hgctl_0;
	hwgaininfo.hgctl_1 = hgctl_1;
	return 0;
}

static const struct snd_kcontrol_new mt7623_soc_controls[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "HWGAIN",
		.info = hwgain_control_info,
		.get = hwgain_control_get,
		.put = hwgain_control_put
	},
};

static int mt7623_pcm_open(struct snd_pcm_substream *substream)
{
    struct mt_stream *s;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct mt_private *priv = snd_soc_platform_get_drvdata(rtd->platform);
    pr_debug("%s() cpu_dai id %d, stream direction %d\n",
        __func__, rtd->cpu_dai->id, substream->stream);
    s = priv->dais[rtd->cpu_dai->id][substream->stream].s;
    substream->runtime->private_data = s;
    if (!s) {
        pr_err("%s() error: no mt stream for this dai\n", __func__);
        return -EINVAL;
    }
    pr_debug("%s() %s\n", __func__, s->name);
    if (s->occupied) {
        pr_warn("%s() warning: can't open %s because it has been occupied\n",
            __func__, s->name);
        return -EINVAL;
    }
    if (s->id >= MT_STREAM_DL1 && s->id <= MT_STREAM_DL5) {
        if (priv->streams[MT_STREAM_DLM].occupied) {
            pr_warn("%s() warning: can't open %s because MT_STREAM_DLM has been occupied\n",
                __func__, s->name);
            return -EINVAL;
        }
    }
    if (s->id == MT_STREAM_DLM) {
        enum mt_stream_id i;
        for (i = MT_STREAM_DL1; i <= MT_STREAM_DL5; ++i) {
            if (priv->streams[i].occupied) {
                pr_warn("%s() warning: can't open MT_STREAM_DLM because %s has been occupied\n",
                    __func__, priv->streams[i].name);
                return -EINVAL;
            }
        }
    }
    s->substream = substream;
    s->occupied = 1;
	snd_pcm_hw_constraint_step(substream->runtime, 0, SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 16);
    if (s->ops && s->ops->open)
        return s->ops->open(substream);
    return 0;
}

static int mt7623_pcm_close(struct snd_pcm_substream *substream)
{
    int ret = 0;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct mt_stream *s = substream->runtime->private_data;
    pr_debug("%s() cpu_dai id %d, stream direction %d\n",
        __func__, rtd->cpu_dai->id, substream->stream);
    if (!s) {
        pr_err("%s() error: no mt stream for this dai\n", __func__);
        return -EINVAL;;
    }
    pr_debug("%s() %s\n", __func__, s->name);
    if (s->ops && s->ops->close)
        ret = s->ops->close(substream);
    s->occupied = 0;
    s->substream = NULL;
    substream->runtime->private_data = NULL;
    return ret;
}

static int mt7623_pcm_hw_params(struct snd_pcm_substream *substream,
                                struct snd_pcm_hw_params *params)
{
    struct mt_stream *s = substream->runtime->private_data;
    pr_debug("%s()\n", __func__);
    if (s && s->ops && s->ops->hw_params)
        return s->ops->hw_params(substream, params);
    return 0;
}

static int mt7623_pcm_hw_free(struct snd_pcm_substream *substream)
{
    struct mt_stream *s = substream->runtime->private_data;
    pr_debug("%s()\n", __func__);
    if (s && s->ops && s->ops->hw_free)
        return s->ops->hw_free(substream);
    return 0;
}

static int mt7623_pcm_prepare(struct snd_pcm_substream *substream)
{
    struct mt_stream *s = substream->runtime->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;
    pr_debug("%s()\n", __func__);
    if (s && s->ops && s->ops->prepare)
        return s->ops->prepare(substream);
    return 0;
}

static int mt7623_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
    struct mt_stream *s = substream->runtime->private_data;
    pr_debug("%s()\n", __func__);
    if (s && s->ops && s->ops->trigger)
        return s->ops->trigger(substream, cmd);
    return 0;
}

static snd_pcm_uframes_t mt7623_pcm_pointer(struct snd_pcm_substream *substream)
{
    struct mt_stream *s = substream->runtime->private_data;
    if (s && s->ops && s->ops->pointer)
        return s->ops->pointer(substream);
    return 0;
}

static struct snd_pcm_ops mt7623_pcm_ops = {
    .open = mt7623_pcm_open,
    .close = mt7623_pcm_close,
    .ioctl = snd_pcm_lib_ioctl,
    .hw_params = mt7623_pcm_hw_params,
    .hw_free = mt7623_pcm_hw_free,
    .prepare = mt7623_pcm_prepare,
    .trigger = mt7623_pcm_trigger,
    .pointer = mt7623_pcm_pointer,
};

static int mt7623_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
    snd_pcm_lib_preallocate_pages_for_all(
        rtd->pcm,
        SNDRV_DMA_TYPE_DEV,
		NULL,
		1024*1024, 1024*1024//128*1024, 128*1024
    );

	return 0;
}

static void mt7623_pcm_free(struct snd_pcm *pcm)
{
    snd_pcm_lib_preallocate_free_for_all(pcm);
}

static inline void call_isr(struct mt_irq *irq)
{
    if (irq->isr) {
        irq->isr(irq->s);
    }
}

static irqreturn_t afe_isr(int irq, void *dev)
{
    enum audio_irq_id id;
    struct mt_private *priv;
    u32 status;
    status = afe_irq_status();
    afe_irq_clear(status);
    priv = dev;
    for (id = IRQ_AFE_IRQ1; id <= IRQ_AFE_DMA; ++id) {
        if (status & (0x1 << (id-IRQ_AFE_IRQ1))) { /* AFE_IRQ1_IRQ to AFE_DMA_IRQ */
            call_isr(&(priv->irqs[id]));
        }
    }
    return IRQ_HANDLED;
}

static irqreturn_t asys_isr(int irq, void *dev)
{
    enum audio_irq_id id;
    struct mt_private *priv;
    u32 status;
    status = asys_irq_status();
    asys_irq_clear(status);
    priv = dev;
    for (id = IRQ_ASYS_IRQ1; id <= IRQ_ASYS_IRQ16; ++id) {
        if (status & (0x1 << (id-IRQ_ASYS_IRQ1))) { /* ASYS_IRQ1_IRQ to ASYS_IRQ16_IRQ */
            call_isr(&(priv->irqs[id]));
        }
    }
    return IRQ_HANDLED;
}

static inline void link_dai_and_stream(struct mt_private *priv,
    int dai_id, int dir,
    enum mt_stream_id stream_id)
{
    priv->dais[dai_id][dir].s = &(priv->streams[stream_id]);
}

static inline void link_stream_and_irq(struct mt_private *priv,
    enum mt_stream_id stream_id,
    enum audio_irq_id irq_id,
    void (*isr)(struct mt_stream *))
{
    if (stream_id < MT_STREAM_NUM) {
        priv->streams[stream_id].irq = &(priv->irqs[irq_id]);
        priv->irqs[irq_id].s = &(priv->streams[stream_id]);
    } else {
        priv->irqs[irq_id].s = NULL;
    }
    priv->irqs[irq_id].isr = isr;
}

static inline void link_stream_and_ops(struct mt_private *priv,
    enum mt_stream_id stream_id,
    struct snd_pcm_ops *ops)
{
    priv->streams[stream_id].ops = ops;
}

static void init_mt_private(struct mt_private *priv)
{
	printk("%s()\n", __func__);
    enum mt_stream_id stream_id;
    enum audio_irq_id irq_id;
    static const char *names[MT_STREAM_NUM] = {
        /* playback streams */
        "MT_STREAM_DL1",
        "MT_STREAM_DL2",
        "MT_STREAM_DL3",
        "MT_STREAM_DL4",
        "MT_STREAM_DL5",
        "MT_STREAM_DL6",
        "MT_STREAM_DLM",
        "MT_STREAM_ARB1",
        "MT_STREAM_DSDR",
        "MT_STREAM_8CH_I2S_OUT",
        "MT_STREAM_IEC1",
        "MT_STREAM_IEC2",
        /* capture streams */
        "MT_STREAM_UL1",
        "MT_STREAM_UL2",
        "MT_STREAM_UL3",
        "MT_STREAM_UL4",
        "MT_STREAM_UL5",
        "MT_STREAM_UL6",
        "MT_STREAM_DAI",
        "MT_STREAM_MOD_PCM",
        "MT_STREAM_AWB",
        "MT_STREAM_AWB2",
        "MT_STREAM_DSDW",
        "MT_STREAM_MULTIIN",
    };
    for (stream_id = MT_STREAM_DL1; stream_id < MT_STREAM_NUM; ++stream_id) {
        priv->streams[stream_id].id = stream_id;
        priv->streams[stream_id].name = names[stream_id];
    }
    for (irq_id = IRQ_AFE_IRQ1; irq_id < IRQ_NUM; ++irq_id) {
        priv->irqs[irq_id].id = irq_id;
    }

    /* 1. stream <-> ops */
	link_stream_and_ops(priv, MT_STREAM_DL1, &memif_ops);
    link_stream_and_ops(priv, MT_STREAM_UL1, &memif_ops);
    link_stream_and_ops(priv, MT_STREAM_8CH_I2S_OUT, &hdmi_pcm_ops);


    /* 2. dai <-> stream */
	itrcon_connect(I12, O15, 1);
	itrcon_connect(I13, O16, 1);
    link_dai_and_stream(priv, MT7623_DAI_I2S1_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL1);
	
	itrcon_connect(I00, O00, 1);
    itrcon_connect(I01, O01, 1);
    link_dai_and_stream(priv, MT7623_DAI_I2S1_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL1);
	
    /* no itrcon */
    link_dai_and_stream(priv, MT7623_DAI_HDMI_OUT_I2S_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_8CH_I2S_OUT);

    /* 3. stream <-> irq */
	link_stream_and_irq(priv, MT_STREAM_DL1, IRQ_ASYS_IRQ1, memif_isr);
	link_stream_and_irq(priv, MT_STREAM_UL1, IRQ_ASYS_IRQ8, memif_isr);
    link_stream_and_irq(priv, MT_STREAM_8CH_I2S_OUT, IRQ_AFE_HDMI, hdmi_pcm_isr);
    
}


#define AFE_MCU_IRQ_ID  (136)
#define ASYS_MCU_IRQ_ID (164)

static int mt7623_pcm_probe(struct snd_soc_platform *platform)
{
    struct mt_private *priv;
    int ret;
    pr_debug("%s()\n", __func__);
    priv = devm_kzalloc(platform->dev, sizeof(struct mt_private), GFP_KERNEL);
    if (!priv) {
        dev_err(platform->dev, "%s() can't allocate memory\n", __func__);
        return -ENOMEM;
    }
    afe_enable(1);
    init_mt_private(priv);
    snd_soc_platform_set_drvdata(platform, priv);
    ret = request_irq(AFE_MCU_IRQ_ID, afe_isr, IRQF_TRIGGER_LOW, "afe-isr", priv);
    if (ret) {
        dev_err(platform->dev, "%s() can't register ISR for IRQ %u (ret=%i)\n",
            __func__, AFE_MCU_IRQ_ID, ret);
    }
    ret = request_irq(ASYS_MCU_IRQ_ID, asys_isr, IRQF_TRIGGER_LOW, "asys-isr", priv);
    if (ret) {
        dev_err(platform->dev, "%s() can't register ISR for IRQ %u (ret=%i)\n",
            __func__, ASYS_MCU_IRQ_ID, ret);
    }
    return 0;
}

static int mt7623_pcm_remove(struct snd_soc_platform *platform)
{
    struct mt_private *priv;
    priv = snd_soc_platform_get_drvdata(platform);
    free_irq(AFE_MCU_IRQ_ID, priv);
    free_irq(ASYS_MCU_IRQ_ID, priv);
    devm_kfree(platform->dev, priv);
    itrcon_disconnectall();
    afe_enable(0);
    return 0;
}

static struct snd_soc_platform_driver mt7623_soc_platform_driver = {
    .probe = mt7623_pcm_probe,
	.remove = mt7623_pcm_remove,
	.pcm_new = mt7623_pcm_new,
	.pcm_free = mt7623_pcm_free,
	.ops = &mt7623_pcm_ops,
	//.controls = mt7623_soc_controls,
	//.num_controls = ARRAY_SIZE(mt7623_soc_controls),
};

static int mt7623_audio_probe(struct platform_device *pdev)
{
    pr_debug("%s()\n", __func__);
    return snd_soc_register_platform(&pdev->dev, &mt7623_soc_platform_driver);
}

static int mt7623_audio_remove(struct platform_device *pdev)
{
    pr_debug("%s()\n", __func__);
    snd_soc_unregister_platform(&pdev->dev);
    return 0;
}

static struct platform_driver mt7623_audio = {
    .driver = {
        .name = "mt7623-audio",
        .owner = THIS_MODULE,
    },
    .probe = mt7623_audio_probe,
    .remove = mt7623_audio_remove
};

module_platform_driver(mt7623_audio);

MODULE_DESCRIPTION("mt7623 audio driver");
MODULE_LICENSE("GPL");

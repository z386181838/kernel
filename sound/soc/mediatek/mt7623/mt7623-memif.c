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
#include "mt7623-private.h"

static const struct snd_pcm_hardware memif_hardware = {
    .info =(SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_RESUME |
	     SNDRV_PCM_INFO_MMAP_VALID),
    .formats          = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE,
    .period_bytes_min = 1024,//3072,
    /* 384k 2ch 32bit 1ms
                                    384k 2ch 16bit 2ms
                                    384k 1ch 32bit 2ms
                                    384k 1ch 16bit 4ms
                                    192k 2ch 32bit 2ms
                                    192k 2ch 16bit 4ms
                                    192k 1ch 32bit 4ms
                                    192k 1ch 16bit 8ms */
    .period_bytes_max = 1024 * 256,//3072 * 10,
    .periods_min      = 4,
    .periods_max      = 1024,
    .buffer_bytes_max = 1024 * 1024,//128 * 1024,  
    .fifo_size = 0,
};

static enum afe_mem_interface get_memif(struct mt_stream *s)
{
    if (!s) {
        return AFE_MEM_NONE;
    }
    switch (s->id) {
    case MT_STREAM_DL1:     return AFE_MEM_DL1;
    case MT_STREAM_DL2:     return AFE_MEM_DL2;
    case MT_STREAM_DL3:     return AFE_MEM_DL3;
    case MT_STREAM_DL4:     return AFE_MEM_DL4;
    case MT_STREAM_DL5:     return AFE_MEM_DL5;
    case MT_STREAM_DL6:     return AFE_MEM_DL6;
    case MT_STREAM_DLM:     return AFE_MEM_DLMCH;
    case MT_STREAM_UL1:     return AFE_MEM_UL1;
    case MT_STREAM_UL2:     return AFE_MEM_UL2;
    case MT_STREAM_UL3:     return AFE_MEM_UL3;
    case MT_STREAM_UL4:     return AFE_MEM_UL4;
    case MT_STREAM_UL5:     return AFE_MEM_UL5;
    case MT_STREAM_UL6:     return AFE_MEM_UL6;
    case MT_STREAM_ARB1:    return AFE_MEM_ARB1;
    case MT_STREAM_DSDR:    return AFE_MEM_DSDR;
    case MT_STREAM_DAI:     return AFE_MEM_DAI;
    case MT_STREAM_MOD_PCM: return AFE_MEM_MOD_PCM;
    case MT_STREAM_AWB:     return AFE_MEM_AWB;
    case MT_STREAM_AWB2:    return AFE_MEM_AWB2;
    case MT_STREAM_DSDW:    return AFE_MEM_DSDW;
    default:                return AFE_MEM_NONE;
    }
}

static int memif_open(struct snd_pcm_substream *substream)
{
    snd_soc_set_runtime_hwparams(substream, &memif_hardware);
    return 0;
}

static int memif_close(struct snd_pcm_substream *substream)
{
    return 0;
}

static int memif_hw_params(struct snd_pcm_substream *substream,
                                struct snd_pcm_hw_params *params)
{
    int ret;
    pr_debug("%s()\n", __func__);
	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
	if (ret < 0) {
		pr_err("%s() error: allocation of memory failed\n", __func__);
		return ret;
	}
    return 0;
}

static int memif_hw_free(struct snd_pcm_substream *substream)
{
    pr_debug("%s()\n", __func__);
    return snd_pcm_lib_free_pages(substream);
}

static int memif_prepare(struct snd_pcm_substream *substream)
{
    int ret;
    struct mt_stream *s = substream->runtime->private_data;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    enum afe_mem_interface memif = get_memif(s);
    enum afe_sampling_rate fs;
    enum afe_channel_mode channel = STEREO;
    enum afe_dlmch_ch_num dlmch_ch_num = DLMCH_0CH;
    enum afe_dsd_width dsd_width = DSD_WIDTH_32BIT;
    int hd_audio = 1;
    struct snd_pcm_runtime *runtime = substream->runtime;
    int dai_id = rtd->cpu_dai->id;
    if (s->use_i2s_slave_clock) {
        if (dai_id >= MT7623_DAI_I2S1_ID && dai_id <= MT7623_DAI_I2S6_ID) {
            fs = FS_I2S1 + (dai_id - MT7623_DAI_I2S1_ID);
        } else {
            fs = FS_I2S1;
			afe_hwgain_gainmode_set(AFE_HWGAIN_1, fs);
        }
    } else {
        fs = fs_enum(runtime->rate);
    }

	switch (runtime->channels) {
	case 1:
        channel = MONO;
        dlmch_ch_num = DLMCH_1CH;
        break;
    case 2:
        dlmch_ch_num = DLMCH_2CH;
        break;
    case 3:
        dlmch_ch_num = DLMCH_3CH;
        break;
    case 4:
        dlmch_ch_num = DLMCH_4CH;
        break;
    case 5:
        dlmch_ch_num = DLMCH_5CH;
        break;
    case 6:
        dlmch_ch_num = DLMCH_6CH;
        break;
    case 7:
        dlmch_ch_num = DLMCH_7CH;
        break;
    case 8:
        dlmch_ch_num = DLMCH_8CH;
        break;
    case 9:
        dlmch_ch_num = DLMCH_9CH;
        break;
    case 10:
        dlmch_ch_num = DLMCH_10CH;
        break;
    default:
        pr_err("%s() error: unsupported channel\n", __func__);
        return -EINVAL;
	}

	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S16_LE:
        hd_audio = 0;
        break;
    case SNDRV_PCM_FORMAT_S32_LE:
        hd_audio = 1;
        break;
    case SNDRV_PCM_FORMAT_DSD_U8:
        hd_audio = 1;
        dsd_width = DSD_WIDTH_8BIT;
        break;
    case SNDRV_PCM_FORMAT_DSD_U16_LE:
        hd_audio = 1;
        dsd_width = DSD_WIDTH_16BIT;
        break;
    default:
        pr_err("%s() error: unsupported format\n", __func__);
        return -EINVAL;
	}

    /* configurate memory interface */
    {
        struct afe_memif_config config = {
            .fs = fs,
            .hd_audio = hd_audio,
            .dsd_width = dsd_width,
            .first_bit = MSB_FIRST,
            .daimod_fs = DAIMOD_8000HZ,
            .channel = channel,
            .dlmch_ch_num = dlmch_ch_num,
            .mono_sel = MONO_USE_L,
            .dup_write = DUP_WR_DISABLE,
            .buffer = {
                .base = runtime->dma_addr,
                .size = frames_to_bytes(runtime, runtime->buffer_size)
            }
        };
        printk("%s() fs = %d,hd_audio = %d,dsd_width = %d,chanael= %d,runtime->periods=%d, runtime->period_size =%d, runtime->buffer_size=%d\n",__func__
                  ,config.fs,config.hd_audio,config.dsd_width,config.channel,runtime->periods, runtime->period_size, runtime->buffer_size);
        
        ret = afe_memif_configurate(memif, &config);
        if (ret < 0) {
            pr_err("%s() error: afe_memif_configurate return %d\n", __func__, ret);
            return ret;
        }
    }

    /* configurate irq */
    if (s->irq) {
        struct audio_irq_config config = {
            .mode = fs,
            .init_val = runtime->period_size
        };
        audio_irq_configurate(s->irq->id , &config);
    }

    return 0;
}

static int memif_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct mt_stream *s = substream->runtime->private_data;
	enum afe_mem_interface memif = get_memif(s);
	struct mt_irq *irq = s->irq;
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		afe_memif_enable(memif, 1);
		afe_hwgain_enable(AFE_HWGAIN_1, 1);
		if (irq)
			audio_irq_enable(irq->id, 1);
		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		if (irq)
			audio_irq_enable(irq->id, 0);
		afe_memif_enable(memif, 0);
		afe_hwgain_enable(AFE_HWGAIN_1, 0);
		return 0;
	default:
		return -EINVAL;
	}
}

static snd_pcm_uframes_t memif_pointer(struct snd_pcm_substream *substream)
{
    snd_pcm_uframes_t offset;
    struct mt_stream *s = substream->runtime->private_data;
    struct snd_pcm_runtime *runtime = substream->runtime;
    offset = bytes_to_frames(runtime, s->pointer);
	if (unlikely(offset >= runtime->buffer_size))
		offset = 0;
    return offset;
}

struct snd_pcm_ops memif_ops = {
    .open = memif_open,
    .close = memif_close,
    .ioctl = snd_pcm_lib_ioctl,
    .hw_params = memif_hw_params,
    .hw_free = memif_hw_free,
    .prepare = memif_prepare,
    .trigger = memif_trigger,
    .pointer = memif_pointer,
};

void memif_isr(struct mt_stream *s)
{
    if (s) {
        u32 base, cur;
        enum afe_mem_interface memif = get_memif(s);
        afe_memif_base(memif, &base);
        afe_memif_pointer(memif, &cur);
        s->pointer = cur - base;
        snd_pcm_period_elapsed(s->substream);
    }
}


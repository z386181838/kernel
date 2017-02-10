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
#include "mt7623-afe-reg.h"
#include "mt7623-hdmi-control.h"

kal_uint32 iec2_nsadr = 0;

static const struct snd_pcm_hardware spdifout_hardware = {
    .info =(SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_RESUME |
	     SNDRV_PCM_INFO_MMAP_VALID),
    .formats = SPDIF_FORMATS,
    .rates = SPDIF_RATES,
    .rate_min = SPDIF_RATE_MIN,
    .rate_max = SPDIF_RATE_MAX,
    .channels_min = SPDIF_CHANNELS_MIN,
    .channels_max = SPDIF_CHANNELS_MAX,
    .buffer_bytes_max = SPDIF_MAX_BUFFER_SIZE,
    .period_bytes_min = SOC_NORMAL_USE_PERIOD_SIZE_MIN,
    .period_bytes_max = (SPDIF_MAX_BUFFER_SIZE / SOC_NORMAL_USE_PERIODS_MIN),
    .periods_min = SOC_NORMAL_USE_PERIODS_MIN,
    .periods_max = SOC_NORMAL_USE_PERIODS_MAX,
    .fifo_size = 0,
};

void spdifout_isr(struct mt_stream *s)
{
    kal_uint32 hw_consume_bytes = 0;
    kal_uint32 hw_mem_idex = 0;
    kal_uint32 hw_cur_idex = 0;
    kal_uint32 burst_len = 0;
    if ((s == NULL) || (afe_read(AFE_IEC2_BURST_INFO) & 0x000010000)) {
        pr_err("%s() invalid values\n", __func__);
        return;
    }
    hw_cur_idex = afe_read(AFE_SPDIF2_CUR);
    if (hw_cur_idex == 0) {
        pr_debug("%s() hw_cur_idex = 0\n", __func__);
        hw_cur_idex = s->substream->runtime->dma_addr;
    }
    hw_mem_idex = (hw_cur_idex - s->substream->runtime->dma_addr);
    /* get hw consume bytes */
    if (hw_mem_idex > s->pointer) {
        hw_consume_bytes = hw_mem_idex - s->pointer;
    } else {
        hw_consume_bytes = s->substream->runtime->dma_bytes + hw_mem_idex - s->pointer;
    }
    pr_debug("%s() hw_cur_idex = 0x%x hw_mem_idex = 0x%x PhysicalAddr = 0x%x,hw_consume_bytes = 0x%x\n",
             __func__, hw_cur_idex, hw_mem_idex, s->substream->runtime->dma_addr, hw_consume_bytes);
    s->pointer += hw_consume_bytes;
    s->pointer %= s->substream->runtime->dma_bytes;
    burst_len = (afe_read(AFE_IEC2_BURST_LEN) & 0x0007ffff) >> 3;
    iec2_nsadr += burst_len;
    if (iec2_nsadr >= afe_read(AFE_SPDIF2_END)) {
        iec2_nsadr = afe_read(AFE_SPDIF2_BASE);
    }
    /* set NSADR for next period */
    afe_msk_write(AFE_IEC2_NSADR, iec2_nsadr, 0xffffffff);
    //Set IEC2 Ready Bit
    afe_msk_write(AFE_IEC2_BURST_INFO, IEC2_BURST_READY_NOT_READY, IEC2_BURST_READY_MASK);
    pr_debug("%s() burst_info = 0x%x iec2_nsadr = 0x%x\n",
             __func__, afe_read(AFE_IEC2_BURST_INFO), afe_read(AFE_IEC2_NSADR));
    snd_pcm_period_elapsed(s->substream);
}

static int mt7623_spdifout_open(struct snd_pcm_substream *substream)
{
    int ret = 0;
    struct snd_pcm_runtime *runtime = substream->runtime;
    snd_soc_set_runtime_hwparams(substream, &spdifout_hardware);
    /* Ensure that buffer size is a multiple of period size */
    ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
    if (ret < 0)
        pr_err("%s() error: snd_pcm_hw_constraint_integer fail %d\n", __func__, ret);
    pr_debug("%s() substream->pcm->device = %d\n", __func__, substream->pcm->device);
    return ret;
}

static int mt7623_spdifout_close(struct snd_pcm_substream *substream)
{
    int i = 0;
    pr_debug("%s() substream->pcm->device = %d\n", __func__, substream->pcm->device);
    return i;
}


static int mt7623_spdifout_params(struct snd_pcm_substream *substream,
                                  struct snd_pcm_hw_params *hw_params)
{
    int ret = 0;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct snd_dma_buffer *dma_buf = &substream->dma_buffer;
    dma_buf->dev.type = SNDRV_DMA_TYPE_DEV;
    dma_buf->dev.dev = substream->pcm->card->dev;
    ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
    if (ret < 0)
        pr_err("%s() error: snd_pcm_lib_malloc_pages fail %d\n", __func__, ret);
    pr_debug("%s() dma_bytes = 0x%x dma_area = %p dma_addr = 0x%llx\n",
             __func__, runtime->dma_bytes, runtime->dma_area, (unsigned long long)runtime->dma_addr);
    return ret;
}

static int mt7623_spdifout_free(struct snd_pcm_substream *substream)
{
    pr_debug("%s()\n", __func__);
    return snd_pcm_lib_free_pages(substream);
}

static int mt7623_spdifout_prepare(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    pr_debug("%s() rate = %u  channels = %u period_size = %lu\n",
             __func__, runtime->rate, runtime->channels, runtime->period_size);
    return 0;
}

static int mt7623_spdifout_start(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct mt_stream *s = runtime->private_data;
    enum afe_sampling_rate sample_rate_idx = fs_enum(runtime->rate);
    pr_debug("%s() period_size = %lu,channels = %d,sample_rate = 0x%x,sample_bits = %d\n",
             __func__, runtime->period_size, runtime->channels, sample_rate_idx, runtime->sample_bits);
    pr_debug("%s() format = %d\n", __func__, runtime->format);
    /*hdmi clock setting*/
    vAudioClockSetting(sample_rate_idx, 128, APLL_SPDIF2_CK, PCM_OUTPUT_32BIT, 0, 0); //PCM_OUTPUT_16BIT
    /*hdmi dram setting*/
    init_hdmi_dma_buffer(MT_AFE_MEM_IEC2, runtime, s);
    /*enable irq*/
    audio_irq_enable(s->irq->id, 1);
    /* interconnection  0 ->pcm  1->rawpcm */
    set_data_output_from_iec2_enable(sample_rate_idx, runtime, 0);
    afe_enable(1);
    return 0;
}

static int mt7623_spdifout_stop(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct mt_stream *s = runtime->private_data;
    enum afe_sampling_rate sample_idx = fs_enum(runtime->rate);
    pr_debug("%s()\n", __func__);
    audio_irq_enable(s->irq->id, 0);
    afe_enable(0);
    set_data_output_from_iec2_disable();
    reset_hdmi_dma_buffer(MT_AFE_MEM_IEC2, runtime, s);
    vAudioClockSetting(sample_idx, 128, APLL_SPDIF2_CK, PCM_OUTPUT_32BIT, 0, 1); //power down clk
    return 0;
}

static int mt7623_spdifout_trigger(struct snd_pcm_substream *substream, int cmd)
{
    pr_debug("%s() cmd = %d\n", __func__, cmd);
    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
        return mt7623_spdifout_start(substream);
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
        return mt7623_spdifout_stop(substream);
    default:
        pr_err("%s() cmd = %d not valid\n", __func__, cmd);
        break;
    }
    return -EINVAL;
}

static snd_pcm_uframes_t mt7623_spdifout_pointer(struct snd_pcm_substream *substream)
{
    int offset = 0;
    struct mt_stream *s = substream->runtime->private_data;
    offset = bytes_to_frames(substream->runtime, s->pointer);
    if (unlikely(offset >= substream->runtime->buffer_size))
        offset = 0;
    return offset;
}

struct snd_pcm_ops spdif_iec2_ops = {
    .open = mt7623_spdifout_open,
    .close = mt7623_spdifout_close,
    .ioctl = snd_pcm_lib_ioctl,
    .hw_params = mt7623_spdifout_params,
    .hw_free = mt7623_spdifout_free,
    .prepare = mt7623_spdifout_prepare,
    .trigger = mt7623_spdifout_trigger,
    .pointer = mt7623_spdifout_pointer,
};


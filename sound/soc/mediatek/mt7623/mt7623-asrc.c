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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <sound/memalloc.h>
#include <linux/ioctl.h>
#include "mt7623-afe.h"

#define ASRC_CMD_TRIGGER         _IOW(0, 0, int)
#define ASRC_CMD_CHANNELS        _IOW(0, 1, int)
#define ASRC_CMD_INPUT_FREQ      _IOW(0, 2, int)
#define ASRC_CMD_INPUT_BITWIDTH  _IOW(0, 3, int)
#define ASRC_CMD_OUTPUT_FREQ     _IOW(0, 4, int)
#define ASRC_CMD_OUTPUT_BITWIDTH _IOW(0, 5, int)
#define ASRC_CMD_IS_DRAIN        _IOR(0, 6, int)

struct asrc_private {
    enum afe_mem_asrc_id id;
    int running;
    int stereo;
    struct snd_dma_buffer input_dmab;
    u32 input_freq;
    u32 input_bitwidth;
    struct snd_dma_buffer output_dmab;
    u32 output_freq;
    u32 output_bitwidth;
};

static enum afe_mem_asrc_id get_asrc_id(const struct file *file);

static int asrc_open(struct inode *node, struct file *file)
{
    struct asrc_private *priv;
    enum afe_mem_asrc_id id = get_asrc_id(file);
    pr_debug("%s()\n", __func__);
    if (id >= MEM_ASRC_NUM) {
        pr_err("%s() error: invalid id %u\n", __func__, id);
        goto open_error;
    }
    priv = kzalloc(sizeof(struct asrc_private), GFP_KERNEL);
    if (!priv) {
        pr_err("%s() error: kzalloc asrc_private failed\n", __func__);
        goto open_error;
    }
	if (snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, 0,
			128*1024, &priv->input_dmab) < 0) {
        pr_err("%s() error: snd_dma_alloc_pages failed for input buffer\n", __func__);
        goto dma_alloc_error;
	}
	if (snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, 0,
			128*1024, &priv->output_dmab) < 0) {
        pr_err("%s() error: snd_dma_alloc_pages failed for output buffer\n", __func__);
        goto dma_alloc_error;
	}
    priv->id = id;
    file->private_data = priv;
    afe_power_on_mem_asrc(id, 1);
    return 0;

dma_alloc_error:
    kzfree(priv);
open_error:
    return -EINVAL;
}

static int asrc_release(struct inode *node, struct file *file)
{
    struct asrc_private *priv = file->private_data;
    pr_debug("%s()\n", __func__);
    if (!priv) {
        return -ENOMEM;
    }
    afe_mem_asrc_enable(priv->id, 0);
    afe_power_on_mem_asrc(priv->id, 0);
    snd_dma_free_pages(&priv->input_dmab);
    snd_dma_free_pages(&priv->output_dmab);
    kzfree(priv);
    file->private_data = NULL;
    return 0;
}

static int is_ibuf_empty(enum afe_mem_asrc_id id)
{
    u32 wp, rp;
    wp = afe_mem_asrc_get_ibuf_wp(id);
    rp = afe_mem_asrc_get_ibuf_rp(id);
    return wp == rp ? 1 : 0;
}

static int is_obuf_empty(enum afe_mem_asrc_id id, size_t bytes)
{
    u32 wp, rp;
    wp = afe_mem_asrc_get_obuf_wp(id);
    rp = afe_mem_asrc_get_obuf_rp(id);
    if (wp > rp) {
        return (wp - rp == 16) ? 1 : 0;
    } else {
        return (wp + bytes - rp == 16) ? 1 : 0;
    }
}

static long asrc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct asrc_private *priv = file->private_data;
    enum afe_mem_asrc_id id;
    pr_debug("%s() cmd=0x%08x, arg=0x%lx\n", __func__, cmd, arg);
    if (!priv) {
        return -ENOMEM;
    }
    id = priv->id;
    if (id >= MEM_ASRC_NUM) {
        pr_err("%s() error: invalid id %u\n", __func__, id);
        return -EINVAL;
    }
    switch (cmd) {
    case ASRC_CMD_CHANNELS:
        priv->stereo = (arg == 1) ? 0 : 1;
        break;
    case ASRC_CMD_INPUT_FREQ:
        priv->input_freq = arg;
        afe_mem_asrc_set_ibuf_freq(id, arg);
        break;
    case ASRC_CMD_INPUT_BITWIDTH:
        priv->input_bitwidth = arg;
        break;
    case ASRC_CMD_OUTPUT_FREQ:
        priv->output_freq = arg;
        afe_mem_asrc_set_obuf_freq(id, arg);
        break;
    case ASRC_CMD_OUTPUT_BITWIDTH:
        priv->output_bitwidth = arg;
        break;
    case ASRC_CMD_TRIGGER:
        if (arg) {
            struct afe_mem_asrc_config config = {
                .input_buffer = {
                    .base = priv->input_dmab.addr,
                    .size = priv->input_dmab.bytes,
                    .freq = priv->input_freq,
                    .bitwidth = priv->input_bitwidth
                },
                .output_buffer = {
                    .base = priv->output_dmab.addr,
                    .size = priv->output_dmab.bytes,
                    .freq = priv->output_freq,
                    .bitwidth = priv->output_bitwidth
                },
                .stereo = priv->stereo,
                .tracking_mode = MEM_ASRC_NO_TRACKING
            };
            afe_mem_asrc_configurate(id, &config);
            afe_mem_asrc_enable(id, 1);
            priv->running = 1;
        } else {
            priv->running = 0;
            afe_mem_asrc_enable(id, 0);
        }
        break;
    case ASRC_CMD_IS_DRAIN: {
        int is_drain = 1;
        if (!arg) return -EFAULT;
        if (priv->running) {
            is_drain = is_ibuf_empty(id) && is_obuf_empty(id, priv->output_dmab.bytes)
                     ? 1 : 0;
        } else {
            is_drain = 1;
        }
        if (copy_to_user((void __user *)(arg), (void *)(&is_drain), sizeof(is_drain)) != 0)
            return -EFAULT;
        break;
    }
    default:
        pr_err("%s() error: unknown asrc cmd 0x%08x\n", __func__, cmd);
        return -EINVAL;
    }
    return 0;
}

static ssize_t asrc_read(struct file *file, char __user *buf, size_t count,
		loff_t *offset)
{
    struct asrc_private *priv = file->private_data;
    enum afe_mem_asrc_id id;
    struct snd_dma_buffer *dmab;
    u32 wp, rp, copy;
    unsigned char *copy_start;
    pr_debug("%s()\n", __func__);
    if (!priv) {
        return 0;
    }
    id = priv->id;
    if (unlikely(id >= MEM_ASRC_NUM)) {
        pr_err("%s() error: invalid id %u\n", __func__, id);
        return 0;
    }
    if (!priv->running) {
        pr_warn("%s() warning: asrc[%u] is not running\n", __func__, id);
        return 0;
    }
    dmab = &priv->output_dmab;
    wp = afe_mem_asrc_get_obuf_wp(id);
    rp = afe_mem_asrc_get_obuf_rp(id);
    copy_start = dmab->area + (rp - dmab->addr);
    count = count / 16 * 16;
    //printk("%s() wp=0x%08x, rp=0x%08x\n", __func__, wp, rp);
    if (rp < wp) {
        u32 delta = wp - rp;
        copy = delta - 16 < count ? delta - 16 : count;
        if (copy == 0) return 0;
        if (copy_to_user(buf, copy_start, copy) == 0) {
            afe_mem_asrc_set_obuf_rp(id, rp + copy);
            return copy;
        } else {
            pr_err("%s() error: L%d copy_to_user\n", __func__, __LINE__);
            return 0;
        }
    } else { /* rp >= wp */
        u32 delta = wp + dmab->bytes - rp;
        copy = delta - 16 < count ? delta - 16 : count;
        if (copy == 0) return 0;
        if (rp + copy < dmab->addr + dmab->bytes) {
            if (copy_to_user(buf, copy_start, copy) == 0) {
                afe_mem_asrc_set_obuf_rp(id, rp + copy);
                return copy;
            } else {
                pr_err("%s() error: L%d copy_to_user\n", __func__, __LINE__);
                return 0;
            }
        } else {
            u32 s1 = dmab->addr + dmab->bytes - rp;
            u32 s2 = copy - s1;
            if (copy_to_user(buf, copy_start, s1) == 0) {
            } else {
                pr_err("%s() error: L%d copy_to_user\n", __func__, __LINE__);
                return 0;
            }
            if (s2) {
                if (copy_to_user(buf + s1, dmab->area, s2) == 0) {
                } else {
                    pr_err("%s() error: L%d copy_to_user\n", __func__, __LINE__);
                    afe_mem_asrc_set_obuf_rp(id, dmab->addr);
                    return s1;
                }
            }
            afe_mem_asrc_set_obuf_rp(id, dmab->addr + s2);
            return copy;
        }
    }
}

static ssize_t asrc_write(struct file *file, const char __user *buf,
		size_t count, loff_t *offset)
{
    struct asrc_private *priv = file->private_data;
    enum afe_mem_asrc_id id;
    struct snd_dma_buffer *dmab;
    u32 wp, rp, copy;
    unsigned char *copy_start;
    pr_debug("%s()\n", __func__);
    if (!priv) {
        return 0;
    }
    id = priv->id;
    if (unlikely(id >= MEM_ASRC_NUM)) {
        pr_err("%s() error: invalid id %u\n", __func__, id);
        return 0;
    }
    if (!priv->running) {
        pr_warn("%s() warning: asrc[%u] is not running\n", __func__, id);
        return 0;
    }
    dmab = &priv->input_dmab;
    wp = afe_mem_asrc_get_ibuf_wp(id);
    rp = afe_mem_asrc_get_ibuf_rp(id);
    //printk("%s() wp=0x%08x, rp=0x%08x\n", __func__, wp, rp);
    copy_start = dmab->area + (wp - dmab->addr);
    count = count / 16 * 16;
    if (wp < rp) {
        u32 delta = rp - wp;
        copy = delta - 16 < count ? delta - 16 : count;
        if (copy == 0) return 0;
        if (copy_from_user(copy_start, buf, copy) == 0) {
            afe_mem_asrc_set_ibuf_wp(id, wp + copy);
            return copy;
        } else {
            pr_err("%s() error: L%d copy_from_user\n", __func__, __LINE__);
            return 0;
        }
    } else { /* wp >= rp */
        u32 delta = rp + dmab->bytes - wp;
        copy = delta - 16 < count ? delta - 16 : count;
        if (copy == 0) return 0;
        if (wp + copy < dmab->addr + dmab->bytes) {
            if (copy_from_user(copy_start, buf, copy) == 0) {
                afe_mem_asrc_set_ibuf_wp(id, wp + copy);
                return copy;
            } else {
                pr_err("%s() error: L%d copy_from_user\n", __func__, __LINE__);
                return 0;
            }
        } else {
            u32 s1 = dmab->addr + dmab->bytes - wp;
            u32 s2 = copy - s1;
            if (copy_from_user(copy_start, buf, s1) == 0) {
            } else {
                pr_err("%s() error: L%d copy_from_user\n", __func__, __LINE__);
                return 0;
            }
            if (s2) {
                if (copy_from_user(dmab->area, buf + s1, s2) == 0) {
                } else {
                    pr_err("%s() error: L%d copy_from_user\n", __func__, __LINE__);
                    afe_mem_asrc_set_ibuf_wp(id, dmab->addr);
                    return s1;
                }
            }
            afe_mem_asrc_set_ibuf_wp(id, dmab->addr + s2);
            return copy;
        }
    }
}

static struct file_operations asrc_fops[MEM_ASRC_NUM] = {
    [MEM_ASRC_1 ... MEM_ASRC_5] = {
        .owner          = THIS_MODULE,
        .open           = asrc_open,
        .release        = asrc_release,
        .unlocked_ioctl = asrc_ioctl,
        .write          = asrc_write,
        .read           = asrc_read,
        .flush          = NULL,
        .fasync         = NULL,
        .mmap           = NULL
    }
};

static enum afe_mem_asrc_id get_asrc_id(const struct file *file)
{
    if (file) {
        enum afe_mem_asrc_id id;
        for (id = MEM_ASRC_1; id < MEM_ASRC_NUM; ++id) {
            if (file->f_op == &asrc_fops[id]) {
                return id;
            }
        }
    }
    return MEM_ASRC_NUM;
}

static struct miscdevice asrc_devices[MEM_ASRC_NUM] = {
    [MEM_ASRC_1] = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "asrc1",
        .fops = &asrc_fops[MEM_ASRC_1],
    },
    [MEM_ASRC_2] = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "asrc2",
        .fops = &asrc_fops[MEM_ASRC_2],
    },
    [MEM_ASRC_3] = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "asrc3",
        .fops = &asrc_fops[MEM_ASRC_3],
    },
    [MEM_ASRC_4] = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "asrc4",
        .fops = &asrc_fops[MEM_ASRC_4],
    },
    [MEM_ASRC_5] = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "asrc5",
        .fops = &asrc_fops[MEM_ASRC_5],
    }
};

static int asrc_mod_init(void)
{
    int ret;
    enum afe_mem_asrc_id id;
    pr_debug("%s()\n", __func__);
    for (id = MEM_ASRC_1; id < MEM_ASRC_NUM; ++id) {
        ret = misc_register(&asrc_devices[id]);
        if (ret) {
            pr_err("%s() error: misc_register asrc[%u] failed %d\n", __func__, id, ret);
        }
    }
    return 0;
}

static void asrc_mod_exit(void)
{
    int ret;
    enum afe_mem_asrc_id id;
    pr_debug("%s()\n", __func__);
    for (id = MEM_ASRC_1; id < MEM_ASRC_NUM; ++id) {
        ret = misc_deregister(&asrc_devices[id]);
        if (ret) {
            pr_err("%s() error: misc_deregister asrc[%u] failed %d\n", __func__, id, ret);
        }
    }
}

module_init(asrc_mod_init);
module_exit(asrc_mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("memory asrc driver");


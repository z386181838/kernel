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


#ifndef _MT7623_DAI_H_
#define _MT7623_DAI_H_

#define MT7623_DAI_NONE             (0)
#define MT7623_DAI_I2S1_ID          (1)
#define MT7623_DAI_I2S2_ID          (2)
#define MT7623_DAI_I2S3_ID          (3)
#define MT7623_DAI_I2S4_ID          (4)
#define MT7623_DAI_I2S5_ID          (5)
#define MT7623_DAI_I2S6_ID          (6)
#define MT7623_DAI_I2SM_ID          (7)
#define MT7623_DAI_SPDIF_OUT_ID     (8)
#define MT7623_DAI_SPDIF_IN_ID      (9)
#define MT7623_DAI_HDMI_OUT_I2S_ID (10)
#define MT7623_DAI_HDMI_OUT_IEC_ID (11)
#define MT7623_DAI_HDMI_IN_ID      (12)
#define MT7623_DAI_BTPCM_ID        (13)
#define MT7623_DAI_DSDENC_ID       (14)
#define MT7623_DAI_DMIC1_ID        (15)
#define MT7623_DAI_DMIC2_ID        (16)
#define MT7623_DAI_MRGIF_I2S_ID    (17)
#define MT7623_DAI_MRGIF_BT_ID     (18)
#define MT7623_DAI_DSDENC_RECORD_ID (19)
#define MT7623_DAI_NUM              (20)

#define DIV_ID_MCLK_TO_BCK  (0)
#define DIV_ID_BCK_TO_LRCK  (1)

#define SLAVE_USE_ASRC_MASK  (1U<<31)
#define SLAVE_USE_ASRC_YES   (1U<<31)
#define SLAVE_USE_ASRC_NO    (0U<<31)

#endif


/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include "partition_define.h"
static const struct excel_info PartInfo_Private[PART_NUM]={			{"preloader",262144,0, EMMC, 0,EMMC_PART_BOOT1},
			{"mbr",524288,0x0, EMMC, 0,EMMC_PART_USER},
			{"ebr1",524288,0x80000, EMMC, 1,EMMC_PART_USER},
			{"pro_info",3145728,0x100000, EMMC, 0,EMMC_PART_USER},
			{"nvram",5242880,0x400000, EMMC, 0,EMMC_PART_USER},
			{"protect_f",10485760,0x900000, EMMC, 2,EMMC_PART_USER},
			{"protect_s",10485760,0x1300000, EMMC, 3,EMMC_PART_USER},
			{"seccfg",131072,0x1d00000, EMMC, 0,EMMC_PART_USER},
			{"uboot",393216,0x1d20000, EMMC, 0,EMMC_PART_USER},
			{"bootimg",16777216,0x1d80000, EMMC, 0,EMMC_PART_USER},
			{"recovery",16777216,0x2d80000, EMMC, 0,EMMC_PART_USER},
			{"sec_ro",6291456,0x3d80000, EMMC, 4,EMMC_PART_USER},
			{"misc",524288,0x4380000, EMMC, 0,EMMC_PART_USER},
			{"logo",3145728,0x4400000, EMMC, 0,EMMC_PART_USER},
			{"ebr2",524288,0x4700000, EMMC, 0,EMMC_PART_USER},
			{"expdb",10485760,0x4780000, EMMC, 0,EMMC_PART_USER},
			{"tee1",5242880,0x5180000, EMMC, 0,EMMC_PART_USER},
			{"tee2",5242880,0x5680000, EMMC, 0,EMMC_PART_USER},
			{"kb",1048576,0x5b80000, EMMC, 0,EMMC_PART_USER},
			{"dkb",1048576,0x5c80000, EMMC, 0,EMMC_PART_USER},
			{"android",1073741824,0x5d80000, EMMC, 5,EMMC_PART_USER},
			{"cache",132120576,0x45d80000, EMMC, 6,EMMC_PART_USER},
			{"usrdata",2147483648,0x4db80000, EMMC, 7,EMMC_PART_USER},
			{"fat",0,0xcdb80000, EMMC, 8,EMMC_PART_USER},
			{"bmtpool",22020096,0xFFFF00a8, EMMC, 0,EMMC_PART_USER},
 };

#ifdef  CONFIG_MTK_EMMC_SUPPORT
struct MBR_EBR_struct MBR_EBR_px[MBR_COUNT]={
	{"mbr", {1, 2, 3, 4, }},
	{"ebr1", {5, 6, 7, }},
	{"ebr2", {8, }},
};

EXPORT_SYMBOL(MBR_EBR_px);
#endif


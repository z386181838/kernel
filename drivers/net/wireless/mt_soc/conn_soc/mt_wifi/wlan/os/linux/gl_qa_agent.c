/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	gl_ate_agent.c
*/
/*******************************************************************************
 *						C O M P I L E R	 F L A G S
 ********************************************************************************
 */
 
/*******************************************************************************
 *						E X T E R N A L	R E F E R E N C E S
 ********************************************************************************
 */
#include "precomp.h"
#if (CFG_SUPPORT_APSOC_MP_TOOL == 1)
#include "gl_wext.h"
#include "gl_cfg80211.h"
#include "gl_ate_agent.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
#include <uapi/linux/nl80211.h>
#endif
/*******************************************************************************
*						C O N S T A N T S
********************************************************************************
*/
#if 0
INT32 priv_qa_agent(
	PRTMP_ADAPTER	pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	RTMP_STRING *wrq_name)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	HQA_CMD_FRAME *HqaCmdFrame;
	UINT32 ATEMagicNum;

	os_alloc_mem_suspend(pAd, (UCHAR **)&HqaCmdFrame, sizeof(*HqaCmdFrame));

	if (!HqaCmdFrame)
	{
		Status = -ENOMEM;
		goto ERROR0;
	}

	NdisZeroMemory(HqaCmdFrame, sizeof(*HqaCmdFrame));

	Status = copy_from_user((PUCHAR)HqaCmdFrame, WRQ->u.data.pointer,
													WRQ->u.data.length);

	if (Status)
	{
		Status = -EFAULT;
		goto ERROR1;
	}

	ATEMagicNum = OS_NTOHL(HqaCmdFrame->MagicNo);

	switch(ATEMagicNum)
	{
		case HQA_CMD_MAGIC_NO:
			Status = HQA_CMDHandler(pAd, WRQ, HqaCmdFrame);
			break;
		default:
			Status = NDIS_STATUS_FAILURE;
			DBGPRINT_ERR(("Unknown magic number of HQA command = %x\n", ATEMagicNum));
			break;
	}

 ERROR1:
	os_free_mem(NULL, HqaCmdFrame);
 ERROR0:
	return Status;
}
#endif
#endif
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
typedef struct _ATE_PRIV_CMD {
	UINT_8 *name;
	int (*set_proc)(struct net_device *prNetDev, UINT_8 *arg);
} ATE_PRIV_CMD, *P_ATE_PRIV_CMD;

ATE_PRIV_CMD rAtePrivCmdTable[] = {
#if 0
	{"ATE",	SetATE},
	{"ATEDA", SetATEDa},
	{"ATESA", SetATESa},
	{"ADCDump", SetADCDump},
	{"ATEBSSID", SetATEBssid},
#endif	
	{"ATECHANNEL", SetATEChannel},
	{NULL,}
};
static UINT_8 aucAteBuf[1024] = {0};
/*******************************************************************************
*						F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
int SetATEChannel(struct net_device *prNetDev, UINT_8 *arg)
{
    P_NDIS_TRANSPORT_STRUCT     prNdisReq;
    UINT_32                     u4BufLen = 0;
    P_PARAM_MTK_WIFI_TEST_STRUC_T  prRfATInfo;
    int i4SetChan = 0;
    int i4SetFreq = 0;

	printk(KERN_ERR "ATE_CMD: SetChannel\n");
    if (sscanf(arg, "%d", &i4SetChan) == 1) {
        i4SetFreq = nicChannelNum2Freq(i4SetChan);
        printk(KERN_ERR "ATE_CMD: SetChannel=%d, Freq=%d \n", i4SetChan, i4SetFreq);
        if ( i4SetFreq == 0)
            return -EINVAL;            
    } else {
        return -EINVAL;
    }		
    prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucAteBuf[0];
    prNdisReq->ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST;
    prNdisReq->inNdisOidlength = 8;
    prNdisReq->outNdisOidLength = 8;
    //prNdisReq->inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);    

    prRfATInfo = (P_PARAM_MTK_WIFI_TEST_STRUC_T)prNdisReq->ndisOidContent;
    prRfATInfo->u4FuncIndex = RF_AT_FUNCID_CHNL_FREQ;
    //prRfATInfo->u4FuncData = 2412000;
    prRfATInfo->u4FuncData = i4SetFreq;

    /* Execute this OID */
    ate_set_ndis(prNetDev, prNdisReq, &u4BufLen);
                          
    return 0;
}
/*----------------------------------------------------------------------------*/
/*! \brief  This routine is called to send a command to TDLS module.
*
* \param[in] prGlueInfo		Pointer to the Adapter structure
* \param[in] prInBuf		A pointer to the command string buffer
* \param[in] u4InBufLen	The length of the buffer
* \param[out] None
*
* \retval None
*/
/*----------------------------------------------------------------------------*/
int
AteCmdSetHandle(
	struct net_device                   *prNetDev,
	UINT_8								*prInBuf,
	UINT_32 							u4InBufLen
	)
{
	UINT_8 *this_char, *value;
	P_ATE_PRIV_CMD prAtePrivCmd;
	int Status = 0;

	while ((this_char = strsep((char **)&prInBuf, ",")) != NULL)
	{
		if (!*this_char)
			 continue;

		if ((value = strchr(this_char, '=')) != NULL)
			*value++ = 0;
		printk(KERN_ERR "ATE_CMD: cmd=%s, value=%s\n", this_char, value);	

		for (prAtePrivCmd = rAtePrivCmdTable; prAtePrivCmd->name; prAtePrivCmd++)
		{
			if (!strcmp(this_char, prAtePrivCmd->name))
			{
				if(prAtePrivCmd->set_proc(prNetDev, value) != 0)
				{   /*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}
				break;  /*Exit for loop. */
			}
		}

		if(prAtePrivCmd->name == NULL)
		{  /*Not found argument */
			Status = -EINVAL;
			break;
		}
	}
	return Status; 
}
#endif
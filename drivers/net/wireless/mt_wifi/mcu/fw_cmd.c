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
	fw_cmd.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

static VOID EventExtCmdResult(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
                                        (struct _EVENT_EXT_CMD_RESULT_T *)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->ucExTenCID));
#ifdef RT_BIG_ENDIAN
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
#endif
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s: EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
}

INT32 CmdInitAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg = {0};
    struct _CMD_ATTRIBUTE attr = {0};

	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s: address = %x, data = %x\n", __FUNCTION__, address, data));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _INIT_CMD_ACCESS_REG));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, INIT_CMD_ACCESS_REG);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);

    os_zero_mem(&access_reg, sizeof(access_reg));

	access_reg.ucSetQuery = 1;
	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdInitAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _INIT_EVENT_ACCESS_REG *access_reg =
                                        (struct _INIT_EVENT_ACCESS_REG *)data;
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &access_reg->u4Data, len - 4);
    *((UINT32 *)(msg->attr.rsp.wb_buf_in_calbk)) =
                        le2cpu32(*((UINT32 *)msg->attr.rsp.wb_buf_in_calbk));
}


INT32 CmdInitAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg = {0};
    struct _CMD_ATTRIBUTE attr = {0};

	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s: address = %x\n", __FUNCTION__, address));

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _INIT_CMD_ACCESS_REG));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, INIT_CMD_ACCESS_REG);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, data);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdInitAccessRegReadCb);

    AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));

	access_reg.ucSetQuery = 0;
	access_reg.u4Address = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}



#ifdef MT_PS

static VOID CmdPsRetrieveStartRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
	{
	MAC_TABLE_ENTRY *pEntry;

	P_EXT_EVENT_AP_PS_RETRIEVE_T EvtPsRetrieveStart;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	UINT32 WlanIdx;
	STA_TR_ENTRY *tr_entry;
	NDIS_STATUS token_status;
	unsigned char q_idx;

	EvtPsRetrieveStart = (struct P_EXT_EVENT_AP_PS_RETRIEVE_T *)Data;
	WlanIdx = le2cpu32(EvtPsRetrieveStart->u4Param1);

	if (!(VALID_TR_WCID(WlanIdx)))
    {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("---->%s INVALID_TR_WCID(WlanIndex)\n", __FUNCTION__));
        return;
    }

	pEntry = &pAd->MacTab.Content[WlanIdx];
	tr_entry = &pAd->MacTab.tr_entry[WlanIdx];
	if (IS_ENTRY_NONE(pEntry))
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("---->%s Entry(wcid=%d) left.\n", __FUNCTION__, WlanIdx));
		return;
	}

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("---->%s: Start to send TOKEN frames.\n", __FUNCTION__));

    tr_entry->ps_state = APPS_RETRIEVE_GOING;

    tr_entry->ps_qbitmap = 0;

	for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
	{
		UINT16 IsEmpty = IS_TXRING_EMPTY(pAd, q_idx);

		if (!IsEmpty)
		{
		    token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]),
                                                    0, WlanIdx, 0, q_idx);
    		if (!token_status)
            {
                tr_entry->ps_qbitmap |= (1 << q_idx);
            }
            else
            {
                MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                        ("%s(%d) Fail: Send TOKEN Frame, AC=%d\n",
                                __FUNCTION__, __LINE__, q_idx));
            }
	    }
	}

	if (tr_entry->ps_qbitmap == 0)
	{
		q_idx = QID_AC_VO;
		token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]),
                                            0, WlanIdx, 0, q_idx);

		if (!token_status)
	    {
			tr_entry->ps_qbitmap |= (1 << q_idx);
        }
	}


	if (tr_entry->ps_qbitmap == 0)
	{
		tr_entry->ps_state = APPS_RETRIEVE_WAIT_EVENT;
		tr_entry->token_enq_all_fail = TRUE;
		RTMP_PS_RETRIVE_CLEAR(pAd,WlanIdx);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
            ("%s(%d): (ps_state = %d) token_enq_all_fail!!"
                    " ==> send CMDTHREAD_PS_CLEAR cmd.\n",
                        			__FUNCTION__, __LINE__,
                        			tr_entry->ps_state));
	}
	else
	{
		tr_entry->token_enq_all_fail = FALSE;
	}
}


static VOID CmdPsClearRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{

    MAC_TABLE_ENTRY *pEntry;
	P_CMD_AP_PS_CLEAR_STRUC_T EvtPsClear;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	STA_TR_ENTRY *tr_entry;
	struct wtbl_entry tb_entry;
	UINT32 WlanIndex;
	unsigned char q_idx;

	EvtPsClear = (struct _CMD_AP_PS_CLEAR_STRUC_T *)Data;
	WlanIndex = le2cpu32(EvtPsClear->u4WlanIdx);
        pEntry = &ad->MacTab.Content[WlanIndex];
	tr_entry = &ad->MacTab.tr_entry[WlanIndex];


    if (ad->MacTab.tr_entry[WlanIndex].PsMode == PWR_ACTIVE)
    {
        tr_entry->ps_state = APPS_RETRIEVE_IDLE;
    }
    else
    {
        tr_entry->ps_state = APPS_RETRIEVE_DONE;
    }
    MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("wcid=%d, Receive Event of CmdPsClear "
                        "tr_entry->ps_state = %d\n",
                        WlanIndex,tr_entry->ps_state));

	if(tr_entry->token_enq_all_fail)
	{
		tr_entry->token_enq_all_fail = FALSE;

		if (tr_entry->ps_queue.Number)
		{
		    MtEnqTxSwqFromPsQueue(ad, q_idx, tr_entry);
        }
		for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
		{
		    tr_entry->TokenCount[q_idx] = tr_entry->tx_queue[q_idx].Number;
        }
	}

#ifdef RTMP_MAC_PCI
#ifdef DOT11_N_SUPPORT
    SendRefreshBAR(ad, pEntry);
#endif /* DOT11_N_SUPPORT */
#endif /* RTMP_MAC_PCI */
    if (tr_entry->ps_state == APPS_RETRIEVE_IDLE)
    {
        MtHandleRxPsPoll(ad, &pEntry->Addr[0], WlanIndex, TRUE);
    }
#ifdef UAPSD_SUPPORT
	else
	{
		if (tr_entry->bEospNullSnd)
		{
			UINT32	AcQueId;

			tr_entry->bEospNullSnd = FALSE;
			/* sanity Check for UAPSD condition */
			if (tr_entry->EospNullUp >= 8)
            {
                tr_entry->EospNullUp = 1; /* shout not be here */
            }
			/* get the AC ID of incoming packet */
			AcQueId = WMM_UP2AC_MAP[tr_entry->EospNullUp];

			/* bQosNull = bEOSP = TRUE = 1 */

			/*
				Use management queue to tx QoS Null frame to avoid delay so
				us_of_frame is not used.
			*/
            RtmpEnqueueNullFrame(ad, pEntry->Addr, pEntry->CurrTxRate,
                                    pEntry->Aid, pEntry->func_tb_idx,
                                    TRUE, TRUE, tr_entry->EospNullUp);

#ifdef UAPSD_DEBUG
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			    ("%s: send a EOSP QoS Null frame!\n", __FUNCTION__));
#endif /* UAPSD_DEBUG */
		}
		else if (pEntry->UAPSDTxNum != 0)
		{
			RTMPDeQueuePacket(ad, TRUE, WMM_NUM_OF_AC,
                    pEntry->wcid, pEntry->UAPSDTxNum);
		}
	}
#endif /* UAPSD_SUPPORT */
}
#endif /* MT_PS */


#ifdef MT_PS
INT32 CmdPsRetrieveStartReq(RTMP_ADAPTER *ad, UINT32 WlanIdx)
{
    struct cmd_msg *msg;
    struct _EXT_CMD_AP_PWS_START_T CmdApPwsStart;
    int ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

    /*how to handle memory allocate failure? */
    msg = AndesAllocCmdMsg(ad, sizeof(struct _EXT_CMD_AP_PWS_START_T));
    if (!msg)
    {
        ret = NDIS_STATUS_RESOURCES;
        MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s:(ret = %d)\n", __FUNCTION__, ret));
        goto error;
    }

    MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s(%d): RTEnqueueInternalCmd comming!! WlanIdx: %x\n",
                            __FUNCTION__, __LINE__,WlanIdx));

   SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
   SET_CMD_ATTR_TYPE(attr, EXT_CID);
   SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_PS_RETRIEVE_START);
   SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
   SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
   SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(struct _EXT_CMD_AP_PWS_START_T));
   SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
   SET_CMD_ATTR_RSP_HANDLER(attr, CmdPsRetrieveStartRsp);

   AndesInitCmdMsg(msg, attr);

   os_zero_mem(&CmdApPwsStart, sizeof(CmdApPwsStart));

   CmdApPwsStart.u4WlanIdx = cpu2le32(WlanIdx);

   AndesAppendCmdMsg(msg, (char *)&CmdApPwsStart, sizeof(CmdApPwsStart));
   msg->wcid = WlanIdx;
   ret = AndesSendCmdMsg(ad, msg);

error:
   	return ret;
}



INT32 CmdPsClearReq(RTMP_ADAPTER *ad, UINT32 wlanidx, BOOLEAN p_wait)
{
	struct cmd_msg *msg;
	struct _CMD_AP_PS_CLEAR_STRUC_T CmdPsClear;
    int ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_AP_PS_CLEAR_STRUC_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
    	MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
		goto error;
	}

	MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s(%d): RTEnqueueInternalCmd comming!! WlanIdx: %x\n",
                            __FUNCTION__, __LINE__,wlanidx));

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_PWR_SAVING);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(struct _CMD_AP_PS_CLEAR_STRUC_T));
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdPsClearRsp);

    AndesInitCmdMsg(msg, attr);

	os_zero_mem(&CmdPsClear, sizeof(CmdPsClear));

	CmdPsClear.u4WlanIdx = cpu2le32(wlanidx);
	CmdPsClear.u4Status = 0;

	AndesAppendCmdMsg(msg, (char *)&CmdPsClear, sizeof(CmdPsClear));
	msg->wcid = wlanidx;
	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}
#endif /* MT_PS */




static VOID CmdHIFLoopbackRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	printk("HIF Loopback status=%d\n", Status);
	switch (Status)
	{
		case TARGET_ADDRESS_LEN_SUCCESS:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                ("%s: Request target address and length success\n",
                                                __FUNCTION__));
			break;
		default:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s: Unknow Status(%d)\n", __FUNCTION__, Status));
			break;
	}
}


INT32 CmdHIFLoopbackReq(RTMP_ADAPTER *ad, UINT32 enable, UINT32 qidx)
{
	struct cmd_msg *msg;
	UINT32 value;
	int ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(ad, 4);
	if (!msg)
    {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, MT_HIF_LOOPBACK);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdHIFLoopbackRsp);

    AndesInitCmdMsg(msg, attr);
	/* start enable */
	enable = (qidx << 16) | (enable & 0xffff);
	value = cpu2le32(enable);

	printk("loopback value=0x%x\n", value);

	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}





INT32 CmdChPrivilege(RTMP_ADAPTER *ad, UINT8 Action, UINT8 control_chl,
            UINT8 central_chl, UINT8 BW, UINT8 TXStream, UINT8 RXStream)
{
	struct cmd_msg *msg;
	struct _CMD_CH_PRIVILEGE_T ch_privilege = {0};
	INT32 ret = 0;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;
    struct _CMD_ATTRIBUTE attr = {0};

	if (central_chl == 0)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s: central channel = 0 is invalid\n", __FUNCTION__));
		return -1;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s: control_chl = %d, central_chl = %d, "
            "BW = %d, TXStream = %d, RXStream = %d\n",
                __FUNCTION__, control_chl, central_chl,
                BW, TXStream, RXStream));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_CH_PRIVILEGE_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, CMD_CH_PRIVILEGE);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);
	os_zero_mem(&ch_privilege, sizeof(ch_privilege));

	ch_privilege.ucAction = Action;
	ch_privilege.ucPrimaryChannel = control_chl;

	if (BW == BAND_WIDTH_20)
	{
		ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCN;
	}
	else if (BW == BAND_WIDTH_40)
	{
		if (control_chl < central_chl)
		{
		    ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCA;
		}
        else
        {
			ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCB;

        }
	}
    else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                        ("unknown bandwidth = %d\n", BW));
	}

	if (central_chl > 14)
	{
	    ch_privilege.ucRfBand =  CMD_CH_PRIV_BAND_A;
	}
    else
	{
	    ch_privilege.ucRfBand = CMD_CH_PRIV_BAND_G;
    }
	ch_privilege.ucRfChannelWidth = CMD_CH_PRIV_CH_WIDTH_20_40;

	ch_privilege.ucReqType = CMD_CH_PRIV_REQ_JOIN;

	AndesAppendCmdMsg(msg, (char *)&ch_privilege, sizeof(ch_privilege));

	if (IS_MT7603(ad) || IS_MT7628(ad) || IS_MT76x6(ad) || IS_MT7637(ad))
	{
		UINT32 Value;
		RTMP_IO_READ32(ad, RMAC_RMCR, &Value);

		if (Value & RMAC_RMCR_RX_STREAM_0)
        {
            Ctl->RxStream0 = 1;
        }
        if (Value & RMAC_RMCR_RX_STREAM_1)
        {
            Ctl->RxStream1 = 1;
        }
		Value |= RMAC_RMCR_RX_STREAM_0;
		Value |= RMAC_RMCR_RX_STREAM_1;
		RTMP_IO_WRITE32(ad, RMAC_RMCR, Value);
	}

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg = {0};
	INT32 ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s: address = %x, data = %x\n", __FUNCTION__, address, data));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_ACCESS_REG_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, CMD_ACCESS_REG);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));

	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _CMD_ACCESS_REG_T *access_reg = (struct _CMD_ACCESS_REG_T *)data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __FUNCTION__));
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &access_reg->u4Data, len - 4);
	*((UINT32 *)(msg->attr.rsp.wb_buf_in_calbk)) =
                        le2cpu32(*((UINT32 *)msg->attr.rsp.wb_buf_in_calbk));
}


INT32 CmdAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg = {0};
	INT32 ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_ACCESS_REG_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, CMD_ACCESS_REG);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, data);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdAccessRegReadCb);

    AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));

	access_reg.u4Address = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(pAd, msg);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                    ("%s: address = %x, value = %x\n",
                    __FUNCTION__, address, *data));
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


VOID CmdIOWrite32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value)
{
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	RTMP_REG_PAIR RegPair;

	if (Ctl->Stage == FW_RUN_TIME)
	{
		RegPair.Register = Offset;
		RegPair.Value = Value;
		MtCmdMultipleMacRegAccessWrite(pAd, &RegPair, 1);
	}
	else
	{
		CmdInitAccessRegWrite(pAd, Offset, Value);
	}
}


VOID CmdIORead32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value)
{

	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	RTMP_REG_PAIR RegPair;

	if (Ctl->Stage == FW_RUN_TIME)
	{
		RegPair.Register = Offset;
		MtCmdMultipleMacRegAccessRead(pAd, &RegPair, 1);
		*Value = RegPair.Value;
	}
	else
	{
		CmdInitAccessRegRead(pAd, Offset, Value);
	}
}


static VOID EventExtNicCapability(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_NIC_CAPABILITY *ExtEventNicCapability =
                                            (EXT_EVENT_NIC_CAPABILITY *)Data;
	UINT32 Loop;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    ("The data code of firmware:"));

	for (Loop = 0; Loop < 16; Loop++)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%c", ExtEventNicCapability->aucDateCode[Loop]));
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\nThe version code of firmware:"));

	for (Loop = 0; Loop < 12; Loop++)
	{
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%c", ExtEventNicCapability->aucVersionCode[Loop]));
	}
}


INT32 CmdNicCapability(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, 0);
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NIC_CAPABILITY);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 28);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, EventExtNicCapability);

    AndesInitCmdMsg(msg, attr);
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

#ifdef CONFIG_ATE
INT32 CmdTxContinous(RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW,
                UINT32 PriCh, UINT32 Mcs, UINT32 WFSel, UCHAR onoff)
{
	struct cmd_msg *msg;
	struct _CMD_TEST_CTRL_T ContiTXParam = {0};
	INT32 ret = 0;
	UCHAR TXDRate = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s, mode:0x%x, bw:0x%x, prich(Control CH):0x%x, "
        "mcs:0x%x, wfsel:0x%x, on/off:0x%x\n", __FUNCTION__,
                PhyMode, BW, PriCh, Mcs, WFSel, onoff));

    msg = AndesAllocCmdMsg(pAd, sizeof(ContiTXParam));

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RF_TEST);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);

    AndesInitCmdMsg(msg, attr);

    os_zero_mem(&ContiTXParam, sizeof(ContiTXParam));

    ContiTXParam.ucAction = ACTION_IN_RFTEST;
	if (onoff == 0)
	{
	    ContiTXParam.u.rRfATInfo.u4FuncIndex = CONTINUOUS_TX_STOP;
	}
    else
	{
        ContiTXParam.u.rRfATInfo.u4FuncIndex = CONTINUOUS_TX_START;
    }
	/* 0: All 1:TX0 2:TX1 */
    ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucCentralCh = PriCh;
	if (BW_40 == BW || BW_80 == BW)
	{
		ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucCtrlCh = (PriCh + 2);
	}
	else
	{
		ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucCtrlCh = PriCh;
	}
	ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucAntIndex = WFSel;
	if (0 == PhyMode) //CCK
    {
        switch(Mcs)
        {
            //long preamble
            case 0:
                TXDRate = 0;
                break;
            case 1:
                TXDRate = 1;
                break;
            case 2:
                TXDRate = 2;
                break;
            case 3:
                TXDRate = 3;
                break;
            //short preamble
            case 9:
                TXDRate = 5;
                break;
            case 10:
                TXDRate = 6;
                break;
            case 11:
                TXDRate = 7;
                break;
        }
    }
    else if (1 == PhyMode) //OFDM
    {
         switch(Mcs)
         {
            case 0:
                TXDRate = 11;
                break;
            case 1:
                TXDRate = 15;
                break;
            case 2:
                TXDRate = 10;
                break;
            case 3:
                TXDRate = 14;
                break;
            case 4:
                TXDRate = 9;
                break;
            case 5:
                TXDRate = 13;
                break;
            case 6:
                TXDRate = 8;
                break;
            case 7:
                TXDRate = 12;
                break;

        }
  	}
    else if (2 == PhyMode || 3 == PhyMode || 4 == PhyMode)
    {
		/* 2. MODULATION_SYSTEM_HT20 ||3.MODULATION_SYSTEM_HT40 || 4. VHT*/
        TXDRate = Mcs;
    }

	ContiTXParam.u.rRfATInfo.Data.rConTxParam.u2RateCode = Mcs << 6 | TXDRate;
#ifdef RT_BIG_ENDIAN
	ContiTXParam.u.rRfATInfo.u4FuncIndex = cpu2le32(ContiTXParam.u.rRfATInfo.u4FuncIndex);
	ContiTXParam.u.rRfATInfo.Data.rConTxParam.u2RateCode = 
							cpu2le32(ContiTXParam.u.rRfATInfo.Data.rConTxParam.u2RateCode);
#endif
    AndesAppendCmdMsg(msg, (char *)&ContiTXParam, sizeof(ContiTXParam));
	ret = AndesSendCmdMsg(pAd, msg);

	return ret;
}

INT32 CmdTxTonePower(RTMP_ADAPTER *pAd, INT32 type, INT32 dec)
{
	struct cmd_msg *msg;
	struct _CMD_TEST_CTRL_T TestCtrl;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(TestCtrl));

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s, type:%d, dec:%d\n", __FUNCTION__, type, dec));

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RF_TEST);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);

    AndesInitCmdMsg(msg, attr);
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));

    TestCtrl.ucAction =ACTION_IN_RFTEST;
    TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(type);
	/* 0: All 1:TX0 2:TX1 */
	switch (ATECtrl->TxAntennaSel)
    {
	case 0:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = 0;
		break;
	case 1:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = 1;
		break;
	case 2:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = 2;
		break;
	default:
        //for future more than 3*3 ant
        TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex =
                                        ATECtrl->TxAntennaSel - 1;
		break;
	}
	TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucTonePowerGain = dec;
    AndesAppendCmdMsg(msg, (char *)&TestCtrl, sizeof(TestCtrl));
    ret = AndesSendCmdMsg(pAd, msg);

	return ret;
}
#endif /* CONFIG_ATE */


#ifdef MT_MAC_BTCOEX
INT AndesCoexOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Status)
{
    struct cmd_msg *msg;
    struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_WIFI_STATUS_UPDATE_T coex_status;
	UINT32 SetBtWlanStatus;
	ret = 0;

	os_zero_mem(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	os_zero_mem(&coex_status, sizeof(COEX_WIFI_STATUS_UPDATE_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_BT_COEX);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);

	coext_t.ucSubOpCode= COEX_WIFI_STATUS_UPDATE;
	coex_status.u4WIFIStatus = 0x00;

	SetBtWlanStatus = pAd->BtWlanStatus;
	switch (Status)
	{
		case STATUS_RADIO_ON:
			SetBtWlanStatus |= COEX_STATUS_RADIO_ON;
			break;
		case STATUS_RADIO_OFF:
			SetBtWlanStatus &= ~( COEX_STATUS_RADIO_ON);
			break;
		case STATUS_SCAN_G_BAND:
			SetBtWlanStatus |= COEX_STATUS_SCAN_G_BAND;
			break;
		case STATUS_SCAN_G_BAND_END:
			SetBtWlanStatus &= ~( COEX_STATUS_SCAN_G_BAND);
			break;
		case STATUS_SCAN_A_BAND:
			SetBtWlanStatus |= COEX_STATUS_SCAN_A_BAND;
			break;
		case STATUS_SCAN_A_BAND_END:
			SetBtWlanStatus &= ~( COEX_STATUS_SCAN_A_BAND);
			break;
		case STATUS_LINK_UP:
			SetBtWlanStatus |= COEX_STATUS_LINK_UP;
			break;
		case STATUS_LINK_DOWN:
			SetBtWlanStatus &= ~( COEX_STATUS_LINK_UP);
			break;
		case STATUS_BT_OVER_WIFI:
			SetBtWlanStatus |= COEX_STATUS_BT_OVER_WIFI;
			break;
		default: /* fatal error */
			break;
	} /* End of switch */

	if (SetBtWlanStatus == pAd->BtWlanStatus)
	{
	    goto error;
	}
    else
	{
	    pAd->BtWlanStatus = SetBtWlanStatus;
    }
	coex_status.u4WIFIStatus = pAd->BtWlanStatus;
	/* Parameter */
#ifdef RT_BIG_ENDIAN
	coex_status.u4WIFIStatus = cpu2le32(coex_status.u4WIFIStatus);
#endif
	os_move_mem(coext_t.aucData, &coex_status,
	        sizeof(COEX_WIFI_STATUS_UPDATE_T));

	hex_dump("AndesBtCoex: ", (UCHAR *)&coext_t,
                    sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t,
                sizeof(EXT_CMD_COEXISTENCE_T));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

INT AndesCoexProtectionFrameOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Mode,
	IN UCHAR Rate)
{
	struct cmd_msg *msg;
    struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_SET_PROTECTION_FRAME_T coex_proction;
	ret = 0;


	os_zero_mem(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	os_zero_mem(&coex_proction, sizeof(COEX_SET_PROTECTION_FRAME_T));

    msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_BT_COEX);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);
	coext_t.ucSubOpCode = COEX_SET_PROTECTION_FRAME;
	coex_proction.ucProFrameMode = Mode;
	coex_proction.ucProFrameRate = Rate;

	os_move_mem(coext_t.aucData, &coex_proction,
            sizeof(COEX_SET_PROTECTION_FRAME_T));

	hex_dump("AndesBtCoexProtection: ",
        (UCHAR *)&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t,sizeof(EXT_CMD_COEXISTENCE_T));


	ret = AndesSendCmdMsg(pAd, msg);


error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT AndesCoexBSSInfo(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN Enable,
	IN UCHAR bQoS)
{
	struct cmd_msg *msg;
    struct _CMD_ATTRIBUTE attr = {0};

	INT32 ret;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_UPDATE_BSS_INFO_T coex_bss_info;
	ret = 0;

	os_zero_mem(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	os_zero_mem(&coex_bss_info, sizeof(COEX_UPDATE_BSS_INFO_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_BT_COEX);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);
	coext_t.ucSubOpCode= COEX_UPDATE_BSS_INFO;

	if (Enable)
	{
		coex_bss_info.u4BSSPresence[0] = 0x1;
		coex_bss_info.u4IsQBSS[0] = bQoS;
	}
	else
	{
		coex_bss_info.u4BSSPresence[0] = 0x0;
		coex_bss_info.u4IsQBSS[0] = 0;
	}
	os_move_mem(coext_t.aucData, &coex_bss_info, sizeof(COEX_UPDATE_BSS_INFO_T));

	hex_dump("AndesBtCoexProtection: ", (UCHAR *)&coext_t,
                        sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));


	ret = AndesSendCmdMsg(pAd, msg);


error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}
#endif


/* WTBL manipulation function*/
PNDIS_PACKET WtblTlvBufferAlloc(RTMP_ADAPTER *pAd,  UINT32 u4AllocateSize)
 {
	PNDIS_PACKET net_pkt = NULL;

	net_pkt = RTMP_AllocateFragPacketBuffer(pAd, u4AllocateSize);

	if (net_pkt == NULL)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::Can not allocate net_pkt\n", __FUNCTION__));
		return NULL;
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::Allocate(%p)\n", __FUNCTION__, net_pkt));
		return net_pkt;
	}
 }

VOID WtblTlvBufferAppend(PNDIS_PACKET pWtblTlvBuffer, UINT16 u2Type,
                        UINT16 u2Length, PUCHAR pNextWtblTlvBuffer)
 {
	if ((pNextWtblTlvBuffer != NULL) && (u2Length != 0))
	{
		P_CMD_WTBL_GENERIC_TLV_T pWtblGenericTlv =
                        (P_CMD_WTBL_GENERIC_TLV_T)pNextWtblTlvBuffer;
		pWtblGenericTlv->u2Tag = u2Type;
		pWtblGenericTlv->u2Length = u2Length;
		os_move_mem(OS_PKT_TAIL_BUF_EXTEND(pWtblTlvBuffer, u2Length),
                                    pNextWtblTlvBuffer, u2Length);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::T(%d), L(%d), V(%p)\n", __FUNCTION__,
            pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length,
                                    pNextWtblTlvBuffer));
	}
	else
	{
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::Can not append WTBL TLV\n", __FUNCTION__));
	}
 }

 /* WTBL TLV buffer free*/
VOID WtblTlvBufferFree(RTMP_ADAPTER *pAd, PNDIS_PACKET pWtblTlvBuffer)
{
	if (pWtblTlvBuffer != NULL)
	{
		RTMPFreeNdisPacket(pAd, pWtblTlvBuffer);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::Free buffer(%p)\n", __FUNCTION__, pWtblTlvBuffer));
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::Fail to free buffer(%p)\n", __FUNCTION__, pWtblTlvBuffer));
	}
}

 VOID *pTlvAppend(VOID *pTlvBuffer, UINT16 u2Type, UINT16 u2Length,
        VOID *pNextTlvBuffer, UINT32 *pu4TotalTlvLen, UCHAR *pucTotalTlvNumber)
 {
	if ((pNextTlvBuffer != NULL) && (u2Length != 0))
	{
		P_CMD_WTBL_GENERIC_TLV_T pWtblGenericTlv =
                                    (P_CMD_WTBL_GENERIC_TLV_T)pNextTlvBuffer;
		pWtblGenericTlv->u2Tag = u2Type;
		pWtblGenericTlv->u2Length = u2Length;
		*pu4TotalTlvLen += u2Length;
		*pucTotalTlvNumber += 1;
		os_move_mem((PUCHAR)pTlvBuffer, (PUCHAR)pWtblGenericTlv, u2Length);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s::T(%d), L(%d), V(%p)\n", __FUNCTION__,
            pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length,
                                        pNextTlvBuffer));
		return (pTlvBuffer + u2Length);
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::Can not append WTBL TLV\n", __FUNCTION__));
		return NULL;
	}
 }

 static VOID CmdExtTlvUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
                                    (struct _EVENT_EXT_CMD_RESULT_T *)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
        __FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s: EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
}


  INT32 CmdExtTlvBufferSend(
	RTMP_ADAPTER *pAd,
	UINT8 ExtCmdType,
	VOID *pTlvBuffer,
	UINT32 u4TlvLength)
{
	struct cmd_msg	*msg = NULL;
	INT32			Ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	/* Allocte CMD msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_STAREC_COMMON_T));
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, ExtCmdType);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtTlvUpdateRsp);

    AndesInitCmdMsg(msg, attr);
	/* Copy TLV buffer to CMD msg */
	AndesAppendCmdMsg(msg, (char *)pTlvBuffer, u4TlvLength);

	/* Send out CMD msg */
	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdExtWtblUpdateCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
                                    (struct _EVENT_EXT_CMD_RESULT_T *)data;

	UINT32	u4Len = len - 20 - sizeof(CMD_WTBL_UPDATE_T);
	PUCHAR	pData = (PCHAR)(data + 20 + sizeof(CMD_WTBL_UPDATE_T));
	PUCHAR	pRspPayload = (PUCHAR)msg->attr.rsp.wb_buf_in_calbk;
	if (pRspPayload == NULL)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->ucExTenCID));
		EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s: EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s: Copy query result to buffer\n",__FUNCTION__));
		os_move_mem(pRspPayload, pData, u4Len);
	}
}

INT32 CmdExtWtblUpdate(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucOperation,
                                        VOID *pBuffer, UINT32 u4BufferLen)
{
	struct cmd_msg 			*msg = NULL;
	CMD_WTBL_UPDATE_T	CmdWtblUpdate = {0};
	INT32					Ret = 0;
	UINT32					u4EnableFeature = 0;
	UINT16					ucTLVNumber = 0;
	UINT16					u2Len = 0;
	UINT32					*pRspPayload = NULL;
	P_CMD_WTBL_GENERIC_TLV_T	pWtblGenericTlv = NULL;
	UINT8					ucRemainingTLVNumber = 0;
	UINT32					u4RemainingTLVBufLen = 0;
	PUCHAR					TempBuffer = (PUCHAR)pBuffer;
    struct _CMD_ATTRIBUTE attr = {0};


	msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_WTBL_INFO + 100);
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	/* Get TVL number from TLV buffer*/
	u4RemainingTLVBufLen = u4BufferLen;
	while (u4RemainingTLVBufLen > 0)
	{
		pWtblGenericTlv = (P_CMD_WTBL_GENERIC_TLV_T)TempBuffer;

		if (pWtblGenericTlv == NULL)
		{
		    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		    		    ("%s, pWtblGenericTlv is NULL\n", __FUNCTION__));
			Ret = NDIS_STATUS_INVALID_DATA;
			break;
		}
		else if (pWtblGenericTlv->u2Length == 0)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s, fail to handle T(%d), L(%d)\n", __FUNCTION__,
                pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length));
			Ret = NDIS_STATUS_INVALID_DATA;
			break;
		}

		u4EnableFeature |= (1 << (pWtblGenericTlv->u2Tag));
		TempBuffer += pWtblGenericTlv->u2Length;
		u4RemainingTLVBufLen -= pWtblGenericTlv->u2Length;
		ucTLVNumber++;
	}

	if (Ret != 0)
	{
		goto Error1;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s, ucTLVNumber = %d\n", __FUNCTION__, ucTLVNumber));

	/* Assign vlaue to related parameters */
	TempBuffer = (PUCHAR)pBuffer;
	ucRemainingTLVNumber = ucTLVNumber;
	u4RemainingTLVBufLen = u4BufferLen;

	/* Set correct length and RspPayload buffer with different operation */
	if (ucOperation == RESET_WTBL_AND_SET)
	{
		if (pBuffer != NULL)
		{
			/* Reset a specific WCID and set WTBL TLV */
			u2Len = 8;
			pRspPayload = NULL;
		}
		else if(u4RemainingTLVBufLen == 0) /* NULL */
		{
			/* Reset a specific WCID only */
			u2Len = 8;
			pRspPayload = NULL;
		}
		else
		{
		    return NDIS_STATUS_FAILURE;
		}
	}
	else if (ucOperation == SET_WTBL)
	{
		if (pBuffer != NULL)
		{
			u2Len = 8;
			pRspPayload = NULL;
		}
		else
		{
			return NDIS_STATUS_FAILURE;
		}
	}
	else if (ucOperation == QUERY_WTBL)
	{
		if (pBuffer != NULL)
		{
			u2Len = u4BufferLen + 20 + sizeof(CMD_WTBL_UPDATE_T);
			pRspPayload = (UINT32 *)pBuffer;
		}
		else
		{
			return NDIS_STATUS_FAILURE;
		}
	}
	else if (ucOperation == RESET_ALL_WTBL)
	{
		if (pBuffer == NULL && u4RemainingTLVBufLen == 0)
		{
			u2Len = 8;
			pRspPayload = NULL;
		}
		else
		{
			return NDIS_STATUS_FAILURE;
		}
	}
	else
	{
		/* Error */
		return NDIS_STATUS_FAILURE;
	}
    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_WTBL_UPDATE);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, u2Len);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pRspPayload);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtWtblUpdateCb);

    AndesInitCmdMsg(msg, attr);
	CmdWtblUpdate.ucWlanIdx = ucWlanIdx;
	CmdWtblUpdate.ucOperation = ucOperation;
	CmdWtblUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	AndesAppendCmdMsg(msg, (char *)&CmdWtblUpdate, sizeof(CMD_WTBL_UPDATE_T));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s, ucWlanIdx = %d, ucOperation = %d, u4EnableFeature = 0x%x\n",
                __FUNCTION__, ucWlanIdx, ucOperation, u4EnableFeature));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s, ucRemainingTLVNumber = %d, u2RemainingTLVBufLen = %d\n",
                __FUNCTION__, ucRemainingTLVNumber, u4RemainingTLVBufLen));

	/* Handle TVL request here */
	while ((ucRemainingTLVNumber > 0) && (u4RemainingTLVBufLen > 0))
	{
		/* Get current TLV */
		pWtblGenericTlv = (P_CMD_WTBL_GENERIC_TLV_T)TempBuffer;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                        ("%s, TLV(%d, %d)\n", __FUNCTION__,
            pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length));

		/* Handle this TLV */
		switch (pWtblGenericTlv->u2Tag)
		{
			/* Tag = 0 */
			case WTBL_GENERIC:
			{
				P_CMD_WTBL_GENERIC_T pCmdWtblGeneric =
                                        (P_CMD_WTBL_GENERIC_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				    ("%s(WTBL_GENERIC), ucMUARIndex = %d, ucSkipTx = %d,"
				        "ucCfAck = %d, ucQos = %d, ucQos = %d, ucAdm = %d,"
            				        "PartialAID = %d, aucPeerAddress"
            				        "(%02x:%02x:%02x:%02x:%02x:%02x)\n",
                								__FUNCTION__,
                								pCmdWtblGeneric->ucMUARIndex,
                								pCmdWtblGeneric->ucSkipTx,
                								pCmdWtblGeneric->ucCfAck,
                								pCmdWtblGeneric->ucQos,
                								pCmdWtblGeneric->ucMesh,
                								pCmdWtblGeneric->ucAdm,
                								pCmdWtblGeneric->u2PartialAID,
								PRINT_MAC(pCmdWtblGeneric->aucPeerAddress)));
#ifdef RT_BIG_ENDIAN
					pCmdWtblGeneric->u2PartialAID = cpu2le16(pCmdWtblGeneric->u2PartialAID);
#endif
				}
				else if (ucOperation == QUERY_WTBL)
				{
					/* No need to fill real parameters when query and
					        just append all zero data buffer to msg */
				}

				break;
			}

			/* Tag = 1 */
			case WTBL_RX:
			{
				P_CMD_WTBL_RX_T 	pCmdWtblRx = (P_CMD_WTBL_RX_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        					("%s(WTBL_RX), ucRcid = %d, ucRca1 = %d,"
        					            " ucRca2 = %d, ucRv = %d\n",
        										__FUNCTION__,
        										pCmdWtblRx->ucRcid,
        										pCmdWtblRx->ucRca1,
        										pCmdWtblRx->ucRca2,
        										pCmdWtblRx->ucRv));
				}

				break;
			}

			/* Tag = 2 */
			case WTBL_HT:
			{
				P_CMD_WTBL_HT_T pCmdWtblHt = (P_CMD_WTBL_HT_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        					("%s(WTBL_HT), ucHt = %d, ucLdpc = %d,"
        					            " ucAf = %d, ucMm = %d\n",
        										__FUNCTION__,
        										pCmdWtblHt->ucHt,
        										pCmdWtblHt->ucLdpc,
        										pCmdWtblHt->ucAf,
        										pCmdWtblHt->ucMm));
				}

				break;
			}

			/* Tag = 3 */
			case WTBL_VHT:
			{
				P_CMD_WTBL_VHT_T pCmdWtblVht = (P_CMD_WTBL_VHT_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					    ("%s(WTBL_VHT), ucLdpcVht = %d, ucDynBw= %d,"
					                " ucVht = %d, ucTxopPsCap = %d\n",
    											__FUNCTION__,
    											pCmdWtblVht->ucLdpcVht,
    											pCmdWtblVht->ucDynBw,
    											pCmdWtblVht->ucVht,
    											pCmdWtblVht->ucTxopPsCap));
				}

				break;
			}

			/* Tag = 4 */
			case WTBL_PEER_PS:
			{
				P_CMD_WTBL_PEER_PS_T pCmdWtblPerPs =
                                            (P_CMD_WTBL_PEER_PS_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					    ("%s(WTBL_PEER_PS), ucDuIPsm = %d, ucIPsm = %d\n",
												__FUNCTION__,
												pCmdWtblPerPs->ucDuIPsm,
												pCmdWtblPerPs->ucIPsm));
				}

				break;
			}

			/* Tag = 5 */
			case WTBL_TX_PS:
			{
				P_CMD_WTBL_TX_PS_T	pCmdWtbTxPs = (P_CMD_WTBL_TX_PS_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					                ("%s(WTBL_TX_PS), ucTxPs = %d\n",
											__FUNCTION__,
											pCmdWtbTxPs->ucTxPs));
				}

				break;
			}

			/* Tag = 6 */
			case WTBL_HDR_TRANS:
			{
				P_CMD_WTBL_HDR_TRANS_T	pCmdWtblHdrTrans =
                                        (P_CMD_WTBL_HDR_TRANS_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					    ("%s(WTBL_HDR_TRANS), ucTd = %d, ucFd = %d, ucDisRhtr =%d\n",
												__FUNCTION__,
												pCmdWtblHdrTrans->ucTd,
												pCmdWtblHdrTrans->ucFd,
												pCmdWtblHdrTrans->ucDisRhtr));
				}

				break;
			}

			/* Tag = 7 */
			case WTBL_SECURITY_KEY:
			{
				P_CMD_WTBL_SECURITY_KEY_T pCmdWtblSecurityKey =
                                        (P_CMD_WTBL_SECURITY_KEY_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					    ("%s(WTBL_SECURITY_KEY), ucAddRemove = %d, ucRkv = %d, "
					    "ucIkv =%d, ucAlgorithmId = %d, ucKeyId = %d, ucKeyLen = %d\n",
											__FUNCTION__,
											pCmdWtblSecurityKey->ucAddRemove,
											pCmdWtblSecurityKey->ucRkv,
											pCmdWtblSecurityKey->ucIkv,
											pCmdWtblSecurityKey->ucAlgorithmId,
											pCmdWtblSecurityKey->ucKeyId,
											pCmdWtblSecurityKey->ucKeyLen));
					}

				break;
			}

			/* Tag = 8 */
			case WTBL_BA:
			{
				P_CMD_WTBL_BA_T pCmdWtblBa = (P_CMD_WTBL_BA_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					if (pCmdWtblBa->ucBaSessionType == BA_SESSION_RECP)
					{
						/* Recipient */
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					    ("%s(WTBL_BA, Recipient), ucTid(%d), ucBaSessionType(%d), "
    					    "ucRstBaTid(%d), ucRstBaSel(%d), ucStartRstBaSb(%d), "
    					    "aucPeerAddress(%02x:%02x:%02x:%02x:%02x:%02x)\n",
										__FUNCTION__,
										pCmdWtblBa->ucTid,
										pCmdWtblBa->ucBaSessionType,
										pCmdWtblBa->ucRstBaTid,
										pCmdWtblBa->ucRstBaSel,
										pCmdWtblBa->ucStartRstBaSb,
										PRINT_MAC(pCmdWtblBa->aucPeerAddress)));
					}
					else
					{
						/* Originator */
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						    ("%s(WTBL_BA, Originator), ucTid(%d), "
						    "u2Sn(%d), ucBaEn(%d), ucBaSize(%d)\n",
    											__FUNCTION__,
    											pCmdWtblBa->ucTid,
    											pCmdWtblBa->u2Sn,
    											pCmdWtblBa->ucBaEn,
    											pCmdWtblBa->ucBaSize));
                    }

				}

				break;
			}

			/* Tag = 9 */
			case WTBL_RDG:
			{
				P_CMD_WTBL_RDG_T 	pCmdWtblRdg = (P_CMD_WTBL_RDG_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					        ("%s(WTBL_RDG), ucRdgBa = %d, ucR = %d\n",
												__FUNCTION__,
												pCmdWtblRdg->ucRdgBa,
												pCmdWtblRdg->ucR));
				}

				break;
			}

			/* Tag = 10 */
			case WTBL_PROTECTION:
			{
				P_CMD_WTBL_PROTECTION_T	pCmdWtblProtection =
                                            (P_CMD_WTBL_PROTECTION_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					            ("%s(WTBL_PROTECTION), ucRts = %d\n",
											__FUNCTION__,
											pCmdWtblProtection->ucRts));
				}

				break;
			}

			/* Tag = 11 */
			case WTBL_CLEAR:
			{
				/* bit 0: Clear PSM (WF_WTBLON: 0x60322300, Bit 31 set 1 then set 0) */
				/* bit 1: Clear BA (WTBL2.DW15) */
				/* bit 2: Clear Rx Counter (6019_00002, bit 14) */
				/* bit 3: Clear Tx Counter (6019_0000, bit 15) */
				/* bit 4: Clear ADM Counter (6019_0000, bit 12) */
				/* bit 5: Clear Cipher key (WTBL3)*/
				P_CMD_WTBL_CLEAR_T	pCmdWtblClear = (P_CMD_WTBL_CLEAR_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					                ("%s(WTBL_CLEAR), ucClear = %x\n",
											__FUNCTION__,
											pCmdWtblClear->ucClear));
				}

				break;
			}

			/* Tag = 12 */
			case WTBL_BF:
			{
				P_CMD_WTBL_BF_T		pCmdWtblBf = (P_CMD_WTBL_BF_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(WTBL_BF), ucTiBf = %d, ucTeBf = %d, ucTibfVh = %d, ucTebfVht = %d, ucGid = %d, ucPfmuIdx = %d\n",
																			__FUNCTION__,
																			pCmdWtblBf->ucTiBf,
																			pCmdWtblBf->ucTeBf,
																			pCmdWtblBf->ucTibfVht,
																			pCmdWtblBf->ucTebfVht,
																			pCmdWtblBf->ucGid,
																			pCmdWtblBf->ucPFMUIdx));
				}

				break;
			}

			/* Tag = 13 */
			case WTBL_SMPS:
			{
				P_CMD_WTBL_SMPS_T pCmdWtblSmPs = (P_CMD_WTBL_SMPS_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					                ("%s(WTBL_SMPS), ucSmPs = %d\n",
												__FUNCTION__,
												pCmdWtblSmPs->ucSmPs));
				}

				break;
			}

			/* Tag = 14 */
			case WTBL_RAW_DATA_RW:
			{
				P_CMD_WTBL_RAW_DATA_RW_T pCmdWtblRawDataRw =
                                        (P_CMD_WTBL_RAW_DATA_RW_T)TempBuffer;

				/* Print argumrnts */
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				    ("%s(WTBL_RAW_DATA_RW), ucWtblIdx = %d, ucWhichDW = %d,"
				                    " u4DwMask = 0x%x, u4DwValue = 0x%x\n",
											__FUNCTION__,
											pCmdWtblRawDataRw->ucWtblIdx,
											pCmdWtblRawDataRw->ucWhichDW,
											pCmdWtblRawDataRw->u4DwMask,
											pCmdWtblRawDataRw->u4DwValue));
#ifdef RT_BIG_ENDIAN
				pCmdWtblRawDataRw->u4DwMask = cpu2le32(pCmdWtblRawDataRw->u4DwMask);
				pCmdWtblRawDataRw->u4DwValue = cpu2le32(pCmdWtblRawDataRw->u4DwValue);
#endif

				break;
			}

			/* Tag = 15 */
			case WTBL_DUMP:
			{
				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					        ("%s(WTBL_DUMP), WTBL_BUFFER_SIZE = %d\n",
													__FUNCTION__,
													WTBL_BUFFER_SIZE));
					/* Fill value  format */
					/* No need to fill this argument */
				}

				break;
			}

			/* Tag = 16 */
			case WTBL_SPE:
			{
				P_CMD_WTBL_SPE_T		pCmdWtblSpe = (P_CMD_WTBL_SPE_T)TempBuffer;

				if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL))
				{
					/* Print argumrnts */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(WTBL_BF), ucSpeIdx = %d\n",
																			__FUNCTION__,
																			pCmdWtblSpe->ucSpeIdx));
				}

				break;
			}

			default:
			{
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                            ("%s, Unknown WTBL TLV Tag(%d)\n",
                            __FUNCTION__, pWtblGenericTlv->u2Tag));
				break;
			}
		}

		/* Advance to next TLV */
		TempBuffer += pWtblGenericTlv->u2Length;
		u4RemainingTLVBufLen -= pWtblGenericTlv->u2Length;
		ucRemainingTLVNumber--;
#ifdef RT_BIG_ENDIAN
		pWtblGenericTlv->u2Length = cpu2le16(pWtblGenericTlv->u2Length);
		pWtblGenericTlv->u2Tag = cpu2le16(pWtblGenericTlv->u2Tag);
#endif
		
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s in while loop, ucRemainingTLVNumber = %d,"
                " u2RemainingTLVBufLen = %d\n", __FUNCTION__,
                ucRemainingTLVNumber, u4RemainingTLVBufLen));
	}

	AndesAppendCmdMsg(msg, (PUCHAR)pBuffer, u4BufferLen);

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);

	goto Success;

Error1:
	if (msg)
	{
		AndesFreeCmdMsg(msg);
	}
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;

Success:
Error0:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

UINT32 WtblDwQuery(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx,
                    UINT8 ucWtbl1234, UINT8 ucWhichDW)
{
	CMD_WTBL_RAW_DATA_RW_T	rWtblRawDataRwWtblDw = {0};

	rWtblRawDataRwWtblDw.u2Tag = WTBL_RAW_DATA_RW;
	rWtblRawDataRwWtblDw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);
	rWtblRawDataRwWtblDw.ucWtblIdx = ucWtbl1234;
	rWtblRawDataRwWtblDw.ucWhichDW = ucWhichDW;
	CmdExtWtblUpdate(pAd, ucWlanIdx, QUERY_WTBL, &rWtblRawDataRwWtblDw,
                                    rWtblRawDataRwWtblDw.u2Length);

	return rWtblRawDataRwWtblDw.u4DwValue;
}

INT32 WtblDwSet(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucWtbl1234,
            UINT8 ucWhichDW, UINT32 u4DwMask, UINT32 u4DwValue)
{
    INT32 ret;
	CMD_WTBL_RAW_DATA_RW_T	rWtblRawDataRwWtblDw = {0};

	rWtblRawDataRwWtblDw.u2Tag = WTBL_RAW_DATA_RW;
	rWtblRawDataRwWtblDw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);
	rWtblRawDataRwWtblDw.ucWtblIdx = ucWtbl1234;
	rWtblRawDataRwWtblDw.ucWhichDW = ucWhichDW;
	rWtblRawDataRwWtblDw.u4DwMask = u4DwMask;
	rWtblRawDataRwWtblDw.u4DwValue = u4DwValue;
	ret = CmdExtWtblUpdate(pAd, ucWlanIdx, SET_WTBL, &rWtblRawDataRwWtblDw,
                                    rWtblRawDataRwWtblDw.u2Length);

    return ret;
}


INT32 WtblResetAndDWsSet(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucWtbl1234,
            INT dw_cnt, struct cmd_wtbl_dw_mask_set *dw_set)
{
    INT32 ret = 0;
    CMD_WTBL_RAW_DATA_RW_T  rWtblRawDataRwWtblDw;
    INT cmd_cnt;
    UINT8 cmd_op;
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s \n", __FUNCTION__));


    for (cmd_cnt = 0; cmd_cnt < dw_cnt; cmd_cnt++) {
        NdisZeroMemory((UCHAR *)&rWtblRawDataRwWtblDw, sizeof(CMD_WTBL_RAW_DATA_RW_T));

        if (cmd_cnt == 0)
            cmd_op = RESET_WTBL_AND_SET;
        else
            cmd_op = SET_WTBL;

        rWtblRawDataRwWtblDw.u2Tag = WTBL_RAW_DATA_RW;
        rWtblRawDataRwWtblDw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);
        rWtblRawDataRwWtblDw.ucWtblIdx = ucWtbl1234;
        rWtblRawDataRwWtblDw.ucWhichDW = dw_set[cmd_cnt].ucWhichDW;
        rWtblRawDataRwWtblDw.u4DwMask = dw_set[cmd_cnt].u4DwMask;
        rWtblRawDataRwWtblDw.u4DwValue = dw_set[cmd_cnt].u4DwValue;
        ret = CmdExtWtblUpdate(pAd, ucWlanIdx, cmd_op, &rWtblRawDataRwWtblDw,
                                        rWtblRawDataRwWtblDw.u2Length);

        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("%s: cmd_cnt/WlanIdx/Tag/Length/WtblIdx/WhichDW/DwMask/DwValue/ret=%d/%d/%d/%d/%d/%d/0x%x/0x%x/%d \n",
                __FUNCTION__, cmd_cnt, ucWlanIdx, rWtblRawDataRwWtblDw.u2Tag, rWtblRawDataRwWtblDw.u2Length,
                rWtblRawDataRwWtblDw.ucWtblIdx, rWtblRawDataRwWtblDw.ucWhichDW,
                rWtblRawDataRwWtblDw.u4DwMask, rWtblRawDataRwWtblDw.u4DwValue, ret));
    }

    return ret;
}


/**
 * @addtogroup bss_dev_sta_info
 * @{
 * @name bss info/device info/sta record firmware command
 * @{
 */
static VOID CmdExtDevInfoUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_DEVINFO_UPDATE_T EventExtCmdResult = (P_EVENT_DEVINFO_UPDATE_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
        __FUNCTION__, EventExtCmdResult->ucExtenCID));

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	if (EventExtCmdResult->u4Status != 0)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::BUG::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::ucOwnMacIdx(%d), u2TotalElementNum(%d)\n",
						__FUNCTION__,
						EventExtCmdResult->ucOwnMacIdx,
						EventExtCmdResult->u2TotalElementNum));
	}
}

static VOID CmdExtStaRecUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_STAREC_UPDATE_T EventExtCmdResult = (P_EVENT_STAREC_UPDATE_T)(Data);
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
    UINT16 u2Tag = 0;
    CMD_STAREC_COMMON_T *pStaRecCommon = NULL;
    MAC_TABLE_ENTRY *pEntry = NULL;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s::EventExtCmdResult.ucExTenCID = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->ucExtenCID));

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	// We can consider move this to caller
	msg->cmd_return_status = EventExtCmdResult->u4Status;

	if (EventExtCmdResult->u4Status != 0)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::ERROR::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
		
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s::ucBssInfoIdx(%d), ucWlanIdx(%d), u2TotalElementNum(%d)\n",
						__FUNCTION__,
						EventExtCmdResult->ucBssInfoIdx,
						EventExtCmdResult->ucWlanIdx,
						le2cpu16(EventExtCmdResult->u2TotalElementNum)));

        if (le2cpu16(EventExtCmdResult->u2TotalElementNum > 0))
        {
            os_move_mem(&u2Tag, EventExtCmdResult->aucBuffer, sizeof(UINT16));

            if (le2cpu16(u2Tag) == STA_REC_BASIC_STA_RECORD)
            {
                /*check if host just sent STA_REC_UPDATE with DISCONNECT state. */
                //os_move_mem(&StaRecCommon, EventExtCmdResult->aucBuffer, sizeof(CMD_STAREC_COMMON_T));
                pStaRecCommon = (CMD_STAREC_COMMON_T *)EventExtCmdResult->aucBuffer;
                if (pStaRecCommon->ucConnectionState == STATE_DISCONNECT)
                {

                    NdisAcquireSpinLock(&pAd->MacTabLock);
                    pEntry = &pAd->MacTab.Content[EventExtCmdResult->ucWlanIdx];
                    if (IS_ENTRY_NONE(pEntry) &&
                        (GET_ENTRY_STATE(pEntry) == ENTRY_STATE_INUSE))
                    {
                        SET_ENTRY_STATE_IDLE(pEntry);
                    }
                    NdisReleaseSpinLock(&pAd->MacTabLock);
                }
            }
        }
	}
}

static VOID CmdExtBssInfoUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_BSSINFO_UPDATE_T EventExtCmdResult = (P_EVENT_BSSINFO_UPDATE_T)Data;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
        __FUNCTION__, EventExtCmdResult->ucExtenCID));

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	if (EventExtCmdResult->u4Status != 0)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::BUG::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
	}
	else
	{
		/* Need to check Rsp called by Add or Delete cmd */
		if (WifiSysGetBssInfoState(pAd, EventExtCmdResult->ucBssInfoIdx) >= BSS_ACTIVE)
			WifiSysUpdateBssInfoState(pAd, EventExtCmdResult->ucBssInfoIdx, BSS_READY);
		else
			WifiSysUpdateBssInfoState(pAd, EventExtCmdResult->ucBssInfoIdx, BSS_INIT);

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::ucBssInfoIdx(%d), u2TotalElementNum(%d)\n",
						__FUNCTION__,
						EventExtCmdResult->ucBssInfoIdx,
						EventExtCmdResult->u2TotalElementNum));
	}
}

/* WTBL TLV update */
INT32 CmdExtDevInfoUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 u4EnableFeature)
{
	struct cmd_msg 			*msg = NULL;
	CMD_DEVINFO_UPDATE_T	CmdDeviceInfoUpdate = {0};
	INT32					Ret = 0;
	UINT8					i = 0;
	UINT16					ucTLVNumber = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	/* Allocate buffer */
	msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_DEVICEINFO);
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	for (i = 0; i < DEVINFO_MAX_NUM; i++)
	{
		if (u4EnableFeature & (1 << i))
		{
			ucTLVNumber++;
		}
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DEVINFO_UPDATE);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_DEVINFO_UPDATE_T));
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtDevInfoUpdateRsp);

    AndesInitCmdMsg(msg, attr);
	/* Fill Devive info parameter here*/
	CmdDeviceInfoUpdate.ucOwnMacIdx = OwnMacIdx;
	CmdDeviceInfoUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdDeviceInfoUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdDeviceInfoUpdate,
                            sizeof(CMD_DEVINFO_UPDATE_T));

	if (u4EnableFeature & DEVINFO_ACTIVE_FEATURE)
	{
		CMD_DEVINFO_ACTIVE_T DevInfoBasic = {0};

        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                ("%s: if (u4EnableFeature & DEVINFO_ACTIVE_FEATURE): BandIdx = %d\n", __FUNCTION__, BandIdx));

		/* Fill TLV format */
		DevInfoBasic.u2Tag = DEVINFO_ACTIVE;
		DevInfoBasic.u2Length = sizeof(CMD_DEVINFO_ACTIVE_T);
		DevInfoBasic.ucActive = Active;
		DevInfoBasic.ucDbdcIdx = BandIdx;
#ifdef RT_BIG_ENDIAN
		DevInfoBasic.u2Tag = cpu2le16(DevInfoBasic.u2Tag);
		DevInfoBasic.u2Length = cpu2le16(DevInfoBasic.u2Length);
#endif
		os_move_mem(DevInfoBasic.aucOwnMAC, OwnMacAddr, MAC_ADDR_LEN);

        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                    ("%s, DevInfoBasic.ucDbdcIdx = %d \n",__FUNCTION__, DevInfoBasic.ucDbdcIdx));

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s(CMD_DEVINFO_BASIC_T), OwnMacIdx = %d, ucActive = %d,"
            " aucOwnMAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__,
						OwnMacIdx,
						DevInfoBasic.ucActive,
						PRINT_MAC(DevInfoBasic.aucOwnMAC)));

		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&DevInfoBasic,
		                sizeof(CMD_DEVINFO_ACTIVE_T));
	}

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

/*Start STARec Handler*/

static INT32 StaRecUpdateBasic(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	/* Fill STA Rec Common */
	CMD_STAREC_COMMON_T StaRecCommon = {0};


	/* Fill TLV format */
	StaRecCommon.u2Tag = STA_REC_BASIC_STA_RECORD;
	StaRecCommon.u2Length = sizeof(CMD_STAREC_COMMON_T);
	StaRecCommon.u4ConnectionType = cpu2le32(pStaRecCfg->ConnectionType);
	StaRecCommon.ucConnectionState = pStaRecCfg->ConnectionState;
	//New info to indicate this is new way to update STAREC
	StaRecCommon.u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2;
	if (pStaRecCfg->IsNewSTARec)
		StaRecCommon.u2ExtraInfo |= STAREC_COMMON_EXTRAINFO_NEWSTAREC;
	StaRecCommon.u2ExtraInfo = StaRecCommon.u2ExtraInfo;
#ifdef CONFIG_AP_SUPPORT
	if(pEntry)
	{
		StaRecCommon.ucIsQBSS =
			CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE) ?
														  TRUE : FALSE;
		StaRecCommon.u2AID = cpu2le16(pEntry->Aid);
	}
#endif /* CONFIG_AP_SUPPORT */


	if (pEntry)
	{
		os_move_mem(StaRecCommon.aucPeerMacAddr,
					pEntry->Addr, MAC_ADDR_LEN);
	}
	else
	{
		os_move_mem(StaRecCommon.aucPeerMacAddr,
					BROADCAST_ADDR, MAC_ADDR_LEN);
	}
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(CMD_STAREC_COMMON_T), u4ConnectionType = %d, "
		"ucConnectionState = %d, ucIsQBSS = %d, u2AID = %d,"
		" aucPeerMacAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
					__FUNCTION__,
					le2cpu32(StaRecCommon.u4ConnectionType),
					StaRecCommon.ucConnectionState,
					StaRecCommon.ucIsQBSS,
					le2cpu16(StaRecCommon.u2AID),
					PRINT_MAC(StaRecCommon.aucPeerMacAddr)));
	/* Append this feature */
#ifdef RT_BIG_ENDIAN
	StaRecCommon.u2Tag = cpu2le16(StaRecCommon.u2Tag);
	StaRecCommon.u2Length = cpu2le16(StaRecCommon.u2Length);
	StaRecCommon.u2ExtraInfo = cpu2le16(StaRecCommon.u2ExtraInfo);
#endif
	AndesAppendCmdMsg(msg, (char *)&StaRecCommon, sizeof(CMD_STAREC_COMMON_T));
	return 0;
}

static INT32 StaRecUpdateRa(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{
#if !defined(WIFI_BUILD_RAM) || (CFG_WIFI_DRIVER_OFFLOAD_RATE_CTRL == 1)
#ifndef WIFI_BUILD_RAM
#if defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622)
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	CMD_STAREC_AUTO_RATE_T CmdStaRecAutoRate = {0};

	if(pEntry)
	{
		os_zero_mem(&CmdStaRecAutoRate, sizeof(CmdStaRecAutoRate));

		StaRecAutoRateParamSet(&pEntry->RaEntry, &CmdStaRecAutoRate);

		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAutoRate,
						sizeof(CMD_STAREC_AUTO_RATE_T));
	return 0;
	}
#endif /* defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) */
#endif /* WIFI_BUILD_RAM */
#endif
	return -1;
}

static INT32 StaRecUpdateRaInfo(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{

#if !defined(WIFI_BUILD_RAM) || (CFG_WIFI_DRIVER_OFFLOAD_RATE_CTRL == 1)
#ifndef WIFI_BUILD_RAM
#if defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622)
	CMD_STAREC_AUTO_RATE_CFG_T CmdStaRecAutoRateCfg = {0};
	RA_COMMON_INFO_T RaCfg;
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if(pEntry)
	{
		os_zero_mem(&RaCfg, sizeof(RaCfg));
		os_zero_mem(&CmdStaRecAutoRateCfg, sizeof(CmdStaRecAutoRateCfg));
		raWrapperConfigSet(pAd, &RaCfg);
		StaRecAutoRateCommCfgSet(&RaCfg, &CmdStaRecAutoRateCfg);

		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAutoRateCfg,
						sizeof(CMD_STAREC_AUTO_RATE_CFG_T));
		return 0;
	}
#endif /* defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) */
#endif /* WIFI_BUILD_RAM */
#endif
	return -1;
}

static INT32 StaRecUpdateRaUpdate(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{

#if !defined(WIFI_BUILD_RAM) || (CFG_WIFI_DRIVER_OFFLOAD_RATE_CTRL == 1)
#ifndef WIFI_BUILD_RAM
#if defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622)
	CMD_STAREC_AUTO_RATE_UPDATE_T CmdStaRecAutoRateUpdate = {0};
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if(pEntry)
	{
		os_zero_mem(&CmdStaRecAutoRateUpdate, sizeof(CmdStaRecAutoRateUpdate));

		StaRecAutoRateUpdate(&pEntry->RaEntry, &pEntry->RaInternal,
			pStaRecCfg->pRaParam, &CmdStaRecAutoRateUpdate);

		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAutoRateUpdate,
						sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
		return 0;
	}
#endif /* defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) */
#endif /* WIFI_BUILD_RAM */
#endif
	return -1;
}

#ifdef TXBF_SUPPORT
static INT32 StaRecUpdateBf(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	CMD_STAREC_BF CmdStaRecBf;

	if(pEntry)
	{

        os_zero_mem(&CmdStaRecBf, sizeof(CMD_STAREC_BF));
        StaRecBfUpdate(pEntry, &CmdStaRecBf);
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecBf, sizeof(CMD_STAREC_BF));
		return 0;
	}
	return -1;
}
#endif /*TXBF_SUPPORT*/

static INT32 StaRecUpdateAmsdu(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{

	CMD_STAREC_AMSDU_T CmdStaRecAmsdu = {0};
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if(pEntry)
	{
		os_zero_mem(&CmdStaRecAmsdu, sizeof(CMD_STAREC_AMSDU_T));
		CmdStaRecAmsdu.u2Tag = STA_REC_AMSDU; // Tag = 0x05
		CmdStaRecAmsdu.u2Length = sizeof(CMD_STAREC_AMSDU_T);
		CmdStaRecAmsdu.ucMaxMpduSize = pEntry->AMsduSize;
		CmdStaRecAmsdu.ucMaxAmsduNum = 0;

#ifdef TX_AGG_ADJUST_WKR
		if (tx_check_for_agg_adjust(pAd, pEntry) == TRUE) {
			CmdStaRecAmsdu.ucMaxMpduSize = 0;
			if (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_APCLI(pEntry)) {
				CmdStaRecAmsdu.ucMaxAmsduNum = 3;
			}
		}
#endif /* TX_AGG_ADJUST_WKR */
#ifdef RT_BIG_ENDIAN
		CmdStaRecAmsdu.u2Tag = cpu2le16(CmdStaRecAmsdu.u2Tag);
		CmdStaRecAmsdu.u2Length = cpu2le16(CmdStaRecAmsdu.u2Length);
#endif
		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAmsdu,
		                    sizeof(CMD_STAREC_AMSDU_T));
		return 0;
	}
	return -1;
}

static INT32 StaRecUpdateTxProc(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{
	CMD_STAREC_TX_PROC_T CmdStaRecTxProc = {0};

	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	os_zero_mem(&CmdStaRecTxProc, sizeof(CMD_STAREC_TX_PROC_T));

	CmdStaRecTxProc.u2Tag = STA_REC_TX_PROC; // Tag = 0x07
	CmdStaRecTxProc.u2Length = sizeof(CMD_STAREC_TX_PROC_T);
	CmdStaRecTxProc.u4TxProcFlag = RVLAN;

	if (pEntry && ((pEntry->wdev->wdev_type == WDEV_TYPE_APCLI)
			|| (pEntry->wdev->wdev_type == WDEV_TYPE_REPEATER)
			|| (pEntry->wdev->wdev_type == WDEV_TYPE_STA)))
	{
		CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_EN;

		if (pEntry->bACMBit[0] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_BK;

		if (pEntry->bACMBit[1] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_BE;

		if (pEntry->bACMBit[2] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_VI;

		if (pEntry->bACMBit[3] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_VO;
	}
#ifdef RT_BIG_ENDIAN
	CmdStaRecTxProc.u4TxProcFlag = cpu2le32(CmdStaRecTxProc.u4TxProcFlag);
	CmdStaRecTxProc.u2Tag = cpu2le16(CmdStaRecTxProc.u2Tag);
	CmdStaRecTxProc.u2Length = cpu2le16(CmdStaRecTxProc.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecTxProc,
	                sizeof(CMD_STAREC_TX_PROC_T));
	return 0;
}

#if defined(MT7615) || defined(MT7622)
#ifdef DOT11_N_SUPPORT
static INT32 StaRecUpdateHtInfo(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{

	CMD_STAREC_HT_INFO_T CmdStaRecHtInfo;
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if(pEntry)
	{
		os_zero_mem(&CmdStaRecHtInfo, sizeof(CMD_STAREC_HT_INFO_T));

		CmdStaRecHtInfo.u2Tag = STA_REC_BASIC_HT_INFO;
		CmdStaRecHtInfo.u2Length = sizeof(CMD_STAREC_HT_INFO_T);

		/* FIXME: may need separate function to compose the payload */
		os_move_mem(&CmdStaRecHtInfo.u2HtCap, &(pEntry->HTCapability.HtCapInfo),
		                                    sizeof(CmdStaRecHtInfo.u2HtCap));
#ifdef RT_BIG_ENDIAN
		CmdStaRecHtInfo.u2HtCap = cpu2le16(CmdStaRecHtInfo.u2HtCap);
		CmdStaRecHtInfo.u2Tag = cpu2le16(CmdStaRecHtInfo.u2Tag);
		CmdStaRecHtInfo.u2Length = cpu2le16(CmdStaRecHtInfo.u2Length);
#endif
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecHtInfo,
	                    sizeof(CMD_STAREC_HT_INFO_T));
		return 0;
	}
	return -1;
}

#ifdef DOT11_VHT_AC
static INT32 StaRecUpdateVhtInfo(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{
	CMD_STAREC_VHT_INFO_T CmdStaRecVHtInfo;
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if(pEntry)
	{
		os_zero_mem(&CmdStaRecVHtInfo, sizeof(CMD_STAREC_VHT_INFO_T));

		CmdStaRecVHtInfo.u2Tag = STA_REC_BASIC_VHT_INFO;
		CmdStaRecVHtInfo.u2Length = sizeof(CMD_STAREC_VHT_INFO_T);

		/* FIXME: may need separate function to compose the payload */
		os_move_mem(&CmdStaRecVHtInfo.u4VhtCap,
	            &(pEntry->vht_cap_ie.vht_cap),
	            sizeof(CmdStaRecVHtInfo.u4VhtCap));
		os_move_mem(&CmdStaRecVHtInfo.u2VhtRxMcsMap,
	            &(pEntry->vht_cap_ie.mcs_set.rx_mcs_map),
	            sizeof(CmdStaRecVHtInfo.u2VhtRxMcsMap));
		os_move_mem(&CmdStaRecVHtInfo.u2VhtTxMcsMap,
	            &(pEntry->vht_cap_ie.mcs_set.tx_mcs_map),
	            sizeof(CmdStaRecVHtInfo.u2VhtTxMcsMap));
#ifdef RT_BIG_ENDIAN
		CmdStaRecVHtInfo.u2Tag = cpu2le16(CmdStaRecVHtInfo.u2Tag);
		CmdStaRecVHtInfo.u2Length = cpu2le16(CmdStaRecVHtInfo.u2Length);
		CmdStaRecVHtInfo.u4VhtCap = cpu2le32(CmdStaRecVHtInfo.u4VhtCap);
		CmdStaRecVHtInfo.u2VhtRxMcsMap = cpu2le16(CmdStaRecVHtInfo.u2VhtRxMcsMap);
		CmdStaRecVHtInfo.u2VhtTxMcsMap = cpu2le16(CmdStaRecVHtInfo.u2VhtTxMcsMap);
#endif
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecVHtInfo,
	                    sizeof(CMD_STAREC_VHT_INFO_T));
		return 0;
	}
	return -1;
}
#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
#endif /* defined(MT7615) || defined(MT7622) */


static INT32 StaRecUpdateApPs(RTMP_ADAPTER *pAd,struct cmd_msg *msg,VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T*)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	CMD_STAREC_PS_T CmdPsInfo = {0};
	UINT8 IdApsd;
	UINT8 ACTriSet = 0;
	UINT8 ACDelSet = 0;

	if(pEntry)
	{
		/* Fill TLV format */
		CmdPsInfo.u2Tag = STA_REC_AP_PS;
		CmdPsInfo.u2Length = sizeof(CMD_STAREC_PS_T);
		// Find Triggerable AC
		// Find Deliverable AC
		ACTriSet = 0;
		ACDelSet = 0;
		for(IdApsd = 0; IdApsd < 4; IdApsd++)
		{
			if (pEntry->bAPSDCapablePerAC[IdApsd])
			{
				ACTriSet |= 1 << IdApsd;
			}
			if (pEntry->bAPSDDeliverEnabledPerAC[IdApsd])
			{
				ACDelSet |= 1 << IdApsd;
			}
		}
		CmdPsInfo.ucStaBmpTriggerAC = ACTriSet;
		CmdPsInfo.ucStaBmpDeliveryAC = ACDelSet;

		CmdPsInfo.ucStaMaxSPLength = pStaRecCfg->pEntry->MaxSPLength;
		CmdPsInfo.u2StaListenInterval = 0; //TODO:

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	        ("%s(STA_REC_AP_PS), Delv=%x Trig=%x SP=%d LInt=%d",
	                        __FUNCTION__,
	                        CmdPsInfo.ucStaBmpDeliveryAC,
	                        CmdPsInfo.ucStaBmpTriggerAC,
	                        CmdPsInfo.ucStaMaxSPLength,
	                        CmdPsInfo.u2StaListenInterval));
#ifdef RT_BIG_ENDIAN
		CmdPsInfo.u2Tag =cpu2le16(CmdPsInfo.u2Tag);
		CmdPsInfo.u2Length = cpu2le16(CmdPsInfo.u2Length);
		CmdPsInfo.u2StaListenInterval = cpu2le16(CmdPsInfo.u2StaListenInterval);
#endif
		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdPsInfo, sizeof(CMD_STAREC_PS_T));
		return 0;
	}
	return -1;
}


static STAREC_HANDLE_T StaRecHandle[] =
{
    {STA_REC_BASIC_STA_RECORD,(UINT32)sizeof(CMD_STAREC_COMMON_T),StaRecUpdateBasic},
    {STA_REC_RA,(UINT32)sizeof(CMD_STAREC_AUTO_RATE_T),StaRecUpdateRa},
    {STA_REC_RA_COMMON_INFO,(UINT32)sizeof(CMD_STAREC_AUTO_RATE_CFG_T),StaRecUpdateRaInfo},
    {STA_REC_RA_UPDATE,(UINT32)sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T),StaRecUpdateRaUpdate},
#ifdef TXBF_SUPPORT
    {STA_REC_BF,(UINT32)sizeof(CMD_STAREC_BF),StaRecUpdateBf},
#endif
    {STA_REC_AMSDU,(UINT32)sizeof(CMD_STAREC_AMSDU_T),StaRecUpdateAmsdu},
#if defined(MT7615) || defined(MT7622)
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
    /* This tag is used in CmdExtStaRecBaUpdate alone, not in CmdExtStaRecUpdate like most tag used. */
    {STA_REC_BA,(UINT32)sizeof(CMD_STAREC_BA_T),NULL},
#endif /*DOT11_N_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/
#endif /* defined(MT7615) || defined(MT7622) */
    {STA_REC_RED,(UINT32)sizeof(CMD_STAREC_RED_T),NULL},
    {STA_REC_TX_PROC,(UINT32)sizeof(CMD_STAREC_TX_PROC_T),StaRecUpdateTxProc},
#if defined(MT7615) || defined(MT7622)
#ifdef DOT11_N_SUPPORT
    {STA_REC_BASIC_HT_INFO,(UINT32)sizeof(CMD_STAREC_HT_INFO_T),StaRecUpdateHtInfo},
#ifdef DOT11_VHT_AC
    {STA_REC_BASIC_VHT_INFO,(UINT32)sizeof(CMD_STAREC_VHT_INFO_T),StaRecUpdateVhtInfo},
#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
#endif /* defined(MT7615) || defined(MT7622) */
    {STA_REC_AP_PS,(UINT32)sizeof(CMD_STAREC_PS_T),StaRecUpdateApPs},
};

typedef UINT32 WLAN_STATUS, *P_WLAN_STATUS;

#define WLAN_STATUS_SUCCESS                     ((WLAN_STATUS)0x00000000L)
#define WLAN_STATUS_PENDING                     ((WLAN_STATUS)0x00000103L)
#define WLAN_STATUS_NOT_ACCEPTED                ((WLAN_STATUS)0x00010003L)

#define STAREC_RETRY 3

static INT32 CmdExtStaRecUpdate_ReSyncDelete(
	RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg)
{
	struct cmd_msg		*msg = NULL;
	CMD_STAREC_UPDATE_T	CmdStaRecUpdate = {0};
	INT32			Ret = 0;
	UINT16			u2TLVNumber = 0;
	UINT32			size;
	struct _CMD_ATTRIBUTE	attr = {0};
	STA_REC_CFG_T 		StaRecCfgForDel = {0};

	StaRecCfgForDel.ucBssIndex = pStaRecCfg->ucBssIndex;
	StaRecCfgForDel.ucWlanIdx = pStaRecCfg->ucWlanIdx;
	StaRecCfgForDel.ConnectionState = STATE_DISCONNECT;
	StaRecCfgForDel.MuarIdx = pStaRecCfg->MuarIdx;
	StaRecCfgForDel.ConnectionType = pStaRecCfg->ConnectionType;
	StaRecCfgForDel.u4EnableFeature = STA_REC_BASIC_STA_RECORD;
	StaRecCfgForDel.pEntry = pStaRecCfg->pEntry;

	size = sizeof(CMD_STAREC_UPDATE_T);

	size += StaRecHandle[STA_REC_BASIC_STA_RECORD].StaRecTagLen;
	
	msg = AndesAllocCmdMsg(pAd,size);
	
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);
	
	AndesInitCmdMsg(msg, attr);

	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = StaRecCfgForDel.ucBssIndex;
	CmdStaRecUpdate.ucWlanIdx = StaRecCfgForDel.ucWlanIdx;
	CmdStaRecUpdate.ucMuarIdx = StaRecCfgForDel.MuarIdx;

	Ret = StaRecHandle[STA_REC_BASIC_STA_RECORD].StaRecTagHandler(pAd,msg,&StaRecCfgForDel);
	if(Ret == NDIS_STATUS_SUCCESS)
	{
		u2TLVNumber++;
	}

	/*insert to head*/
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(u2TLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
#ifdef RT_BIG_ENDIAN
	StaRecCfgForDel.u4EnableFeature = cpu2le32(StaRecCfgForDel.u4EnableFeature);
#endif
	AndesAppendHeadCmdMsg(msg, (char *)&CmdStaRecUpdate,
			    sizeof(CMD_STAREC_UPDATE_T));
	
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	return Ret;
}

INT32 CmdExtStaRecUpdate(
	RTMP_ADAPTER *pAd,
	STA_REC_CFG_T StaRecCfg)
{
	struct cmd_msg 			*msg = NULL;
	CMD_STAREC_UPDATE_T	    CmdStaRecUpdate = {0};
	INT32					Ret = 0;
	UINT8					i = 0;
	UINT16					u2TLVNumber;
	UINT32					size;
	UCHAR StaRecSupprotNum = sizeof(StaRecHandle)/sizeof(STAREC_HANDLE_T);
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 retry = STAREC_RETRY;

CmdExtStaRecUpdate_restart:

	u2TLVNumber = 0;

	size = sizeof(CMD_STAREC_UPDATE_T);
	/* Get number of TLV*/
	for (i = 0; i < StaRecSupprotNum; i++)
	{
		if (StaRecCfg.u4EnableFeature & (1 << StaRecHandle[i].StaRecTag))
		{
			size+=StaRecHandle[i].StaRecTagLen;
		}
	}

	msg = AndesAllocCmdMsg(pAd,size);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);

    AndesInitCmdMsg(msg, attr);
	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = StaRecCfg.ucBssIndex;
	CmdStaRecUpdate.ucWlanIdx = StaRecCfg.ucWlanIdx;
	CmdStaRecUpdate.ucMuarIdx = StaRecCfg.MuarIdx;

	/* Fill RA related parameters */

    for (i = 0; i < StaRecSupprotNum; i++)
    {
        if ((StaRecCfg.u4EnableFeature & (1 << StaRecHandle[i].StaRecTag)))
        {
            if (StaRecHandle[i].StaRecTagHandler!= NULL)
            {
                Ret = StaRecHandle[i].StaRecTagHandler(pAd,msg,&StaRecCfg);
				if(Ret==0)
				{
					u2TLVNumber++;
				}
            }
            else
            {
                MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                    ("%s: StaRecTag = %d no corresponding function handler.\n",
                        __FUNCTION__, StaRecHandle[i].StaRecTag));
            }
        }
    }

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
        ("%s: u2TLVNumber(%d)\n", __FUNCTION__, u2TLVNumber));

	/*insert to head*/
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(u2TLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendHeadCmdMsg(msg, (char *)&CmdStaRecUpdate,
                        sizeof(CMD_STAREC_UPDATE_T));

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);

	while ((Ret != WLAN_STATUS_SUCCESS) && (retry))
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		    ("%s:(FW Ret = 0x%x retry=%d)\n", __FUNCTION__, Ret, retry));
	
		retry--;
		
		// something wrong with STAREC update
		Ret = CmdExtStaRecUpdate_ReSyncDelete(pAd, &StaRecCfg);
		if (Ret != WLAN_STATUS_SUCCESS)
			continue;

		goto CmdExtStaRecUpdate_restart;
	}

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}



INT32 CmdExtStaRecBaUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg)
{
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	CMD_STAREC_UPDATE_T CmdStaRecUpdate = {0};
	CMD_STAREC_BA_T CmdStaRecBa;
	UINT32 MsgLen;
	INT32 Ret = 0;
	UINT16 ucTLVNumber = 0;

	MsgLen = sizeof(CMD_STAREC_UPDATE_T) + sizeof(CMD_STAREC_BA_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	ucTLVNumber = 1;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: ucTLVNumber(%d)\n", __FUNCTION__, ucTLVNumber));

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);

	AndesInitCmdMsg(msg, attr);

	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = StaRecBaCfg.BssIdx;
	CmdStaRecUpdate.ucWlanIdx = StaRecBaCfg.WlanIdx;
	CmdStaRecUpdate.ucMuarIdx = StaRecBaCfg.MuarIdx;


	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, sizeof(CMD_STAREC_UPDATE_T));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: StaRecUpdate:\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: ucBssIndex=%d, ucWlanIdx=%d, ucMuarIdx=%d, u2TotalElementNum=%d\n",
			 __FUNCTION__, CmdStaRecUpdate.ucBssIndex, CmdStaRecUpdate.ucWlanIdx,CmdStaRecUpdate.ucMuarIdx,CmdStaRecUpdate.u2TotalElementNum));

	/* Fill BA related parameters */
	NdisZeroMemory(&CmdStaRecBa, sizeof(CMD_STAREC_BA_T));
	if (StaRecBaCfg.baDirection == ORI_BA)
	{
		BA_ORI_ENTRY *pBAEntry = (BA_ORI_ENTRY*)StaRecBaCfg.BaEntry;

		CmdStaRecBa.ucAmsduCap = pBAEntry->amsdu_cap;
		CmdStaRecBa.u2BaStartSeq = cpu2le16(pBAEntry->Sequence);
		CmdStaRecBa.u2BaWinSize = cpu2le16(pBAEntry->BAWinSize);
	}
	else if (StaRecBaCfg.baDirection== RCV_BA)
	{
		BA_REC_ENTRY *pBAEntry = (BA_REC_ENTRY*)StaRecBaCfg.BaEntry;
		CmdStaRecBa.u2BaStartSeq = cpu2le16(pBAEntry->LastIndSeq);
		CmdStaRecBa.u2BaWinSize = cpu2le16(pBAEntry->BAWinSize);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:Invalid BaDirection (baDirection = %d)\n", __FUNCTION__, StaRecBaCfg.baDirection));
		return Ret;
	}
	CmdStaRecBa.ucBaEenable = StaRecBaCfg.BaEnable;

	CmdStaRecBa.u2Tag = STA_REC_BA;
	CmdStaRecBa.u2Length = sizeof(CMD_STAREC_BA_T);
	CmdStaRecBa.ucTid = StaRecBaCfg.tid;
	CmdStaRecBa.ucBaDirection = StaRecBaCfg.baDirection;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: BaInfo:\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: u2Tag=%d, u2Length=%d, ucTid=%d, u2BaDirectin=%d, BaEnable=%d, u2BaStartSeq=%d, u2BaWinSize=%d, ucAmsduCap=%d\n",
			 __FUNCTION__,	CmdStaRecBa.u2Tag, CmdStaRecBa.u2Length, CmdStaRecBa.ucTid, CmdStaRecBa.ucBaDirection,
			 CmdStaRecBa.ucBaEenable, le2cpu16(CmdStaRecBa.u2BaStartSeq), le2cpu16(CmdStaRecBa.u2BaWinSize),CmdStaRecBa.ucAmsduCap));
#ifdef RT_BIG_ENDIAN
	CmdStaRecBa.u2Tag = cpu2le16(CmdStaRecBa.u2Tag);
	CmdStaRecBa.u2Length = cpu2le16(CmdStaRecBa.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecBa, sizeof(CMD_STAREC_BA_T));

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID bssConnectOwnDev(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
    CMD_BSSINFO_CONNECT_OWN_DEV_T CmdBssiinfoConnectOwnDev = {0};

    CmdBssiinfoConnectOwnDev.u2Tag = BSS_INFO_OWN_MAC;
    CmdBssiinfoConnectOwnDev.u2Length =
                        sizeof(CMD_BSSINFO_CONNECT_OWN_DEV_T);
    CmdBssiinfoConnectOwnDev.ucHwBSSIndex =
		(bss_info_argument.OwnMacIdx > HW_BSSID_MAX) ? 
		 HW_BSSID_0 : bss_info_argument.OwnMacIdx;
	
    CmdBssiinfoConnectOwnDev.ucOwnMacIdx = bss_info_argument.OwnMacIdx;
    CmdBssiinfoConnectOwnDev.u4ConnectionType =
                        cpu2le32(bss_info_argument.u4ConnectionType);

    /* Fill TLV format */
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s, ucHwBSSIndex = %d, ucOwnMacIdx = %d,"
                " u4ConnectionType = %x\n", __FUNCTION__,
                CmdBssiinfoConnectOwnDev.ucHwBSSIndex,
                CmdBssiinfoConnectOwnDev.ucOwnMacIdx,
                bss_info_argument.u4ConnectionType));
#ifdef RT_BIG_ENDIAN
	CmdBssiinfoConnectOwnDev.u2Tag = cpu2le16(CmdBssiinfoConnectOwnDev.u2Tag);
	CmdBssiinfoConnectOwnDev.u2Length = cpu2le16(CmdBssiinfoConnectOwnDev.u2Length);
#endif
    /* Append this feature */
    AndesAppendCmdMsg(msg, (char *)&CmdBssiinfoConnectOwnDev,
                    sizeof(CMD_BSSINFO_CONNECT_OWN_DEV_T));
}

static VOID bssUpdateBssInfoBasic(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
    CMD_BSSINFO_BASIC_T CmdBssInfoBasic = {0};

    /* Fill TLV format */
    CmdBssInfoBasic.u2Tag = BSS_INFO_BASIC;
    CmdBssInfoBasic.u2Length = sizeof(CMD_BSSINFO_BASIC_T);
    CmdBssInfoBasic.u4NetworkType = cpu2le32(bss_info_argument.NetworkType);
	if (bss_info_argument.bss_state >= BSS_ACTIVE)
		CmdBssInfoBasic.ucActive = TRUE;
	else
		CmdBssInfoBasic.ucActive = FALSE;
#ifdef CONFIG_AP_SUPPORT
    CmdBssInfoBasic.u2BcnInterval = cpu2le16(pAd->CommonCfg.BeaconPeriod);
    CmdBssInfoBasic.ucDtimPeriod = pAd->ApCfg.DtimPeriod;
#endif /* CONFIG_AP_SUPPORT */
    os_move_mem(CmdBssInfoBasic.aucBSSID, bss_info_argument.Bssid, MAC_ADDR_LEN);
    CmdBssInfoBasic.ucWmmIdx = bss_info_argument.WmmIdx;
    CmdBssInfoBasic.ucBcMcWlanidx = (bss_info_argument.u4ConnectionType == CONNECTION_INFRA_STA) ?
		bss_info_argument.ucPeerWlanIdx:bss_info_argument.ucBcMcWlanIdx;

    CmdBssInfoBasic.ucCipherSuit= bss_info_argument.CipherSuit;

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s, u4NetworkType = %d, ucActive = %d, u2BcnInterval = %d, \
                    ucWmmIdx = %d, ucDtimPeriod = %d, ucBcMcWlanidx = %d, \
                    ucCipherSuit=%d, BSSID = %02x:%02x:%02x:%02x:%02x:%02x\n",
                                        __FUNCTION__,
                                        bss_info_argument.NetworkType,
                                        CmdBssInfoBasic.ucActive,
                                        le2cpu16(CmdBssInfoBasic.u2BcnInterval),
                                        CmdBssInfoBasic.ucWmmIdx,
                                        CmdBssInfoBasic.ucDtimPeriod,
                                        CmdBssInfoBasic.ucBcMcWlanidx,
                                        CmdBssInfoBasic.ucCipherSuit,
                                        PRINT_MAC(CmdBssInfoBasic.aucBSSID)));
#ifdef RT_BIG_ENDIAN
	CmdBssInfoBasic.u2Tag = cpu2le16(CmdBssInfoBasic.u2Tag);
	CmdBssInfoBasic.u2Length = cpu2le16(CmdBssInfoBasic.u2Length);
#endif
	/* Append this feature */

    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoBasic, sizeof(CMD_BSSINFO_BASIC_T));
}

static VOID bssUpdateChannel(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
    CMD_BSSINFO_RF_CH_T CmdBssInfoRfCh = {0};
    struct wifi_dev *wdev = NULL;

    wdev = WdevSearchByOmacIdx(pAd, bss_info_argument.OwnMacIdx);

    if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): Error! Cannot find a valid wdev.\n", __FUNCTION__));
		return;
    }

    /* Fill TLV format */
    CmdBssInfoRfCh.u2Tag = BSS_INFO_RF_CH;
    CmdBssInfoRfCh.u2Length = sizeof(CMD_BSSINFO_RF_CH_T);
    CmdBssInfoRfCh.ucPrimaryChannel = wdev->channel;
    CmdBssInfoRfCh.ucCenterChannelSeg0 = wdev->CentralChannel;
    CmdBssInfoRfCh.ucCenterChannelSeg1 = wdev->CentralChannel;
	CmdBssInfoRfCh.ucBandwidth = GetCfgBw2RawBw(wlan_operate_get_bw(wdev));
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                ("%s(BSS_INFO_RF_CH), ucPrimCh = %d, ucCentChSeg0 = %d,"
                            " ucCentChSeg1 = %d, BW=%d\n", __FUNCTION__,
                                    CmdBssInfoRfCh.ucPrimaryChannel,
                                    CmdBssInfoRfCh.ucCenterChannelSeg0,
                                    CmdBssInfoRfCh.ucCenterChannelSeg1,
                                    CmdBssInfoRfCh.ucBandwidth));
#ifdef RT_BIG_ENDIAN
	CmdBssInfoRfCh.u2Tag = cpu2le16(CmdBssInfoRfCh.u2Tag);
	CmdBssInfoRfCh.u2Length = cpu2le16(CmdBssInfoRfCh.u2Length);
#endif

    /* Append this feature */
    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoRfCh, sizeof(CMD_BSSINFO_RF_CH_T));
}

static VOID pmUpdateBssInfoPM(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
}

static VOID pmUpdateBssUapsd(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{

    CMD_BSSINFO_UAPSD_T CmdBssInfoUapsd = {0};

    /* Fill TLV format */
    CmdBssInfoUapsd.u2Tag = BSS_INFO_UAPSD;
    CmdBssInfoUapsd.u2Length = sizeof(CMD_BSSINFO_UAPSD_T);

        CmdBssInfoUapsd.ucIsUapsdSupported = FALSE;

        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                ("%s, ucIsUapsdSupported = %d, ucUapsdTriggerAC = 0x%x,"
                    "ucUapsdTriggerAC = 0x%x, u2UapsdServicePeriodTO = 0x%x\n",
                                __FUNCTION__,
                                CmdBssInfoUapsd.ucIsUapsdSupported,
                                CmdBssInfoUapsd.ucUapsdTriggerAC,
                                CmdBssInfoUapsd.ucUapsdDeliveryAC,
                                CmdBssInfoUapsd.u2UapsdServicePeriodTO));
#ifdef RT_BIG_ENDIAN
	CmdBssInfoUapsd.u2Tag = cpu2le16(CmdBssInfoUapsd.u2Tag);
	CmdBssInfoUapsd.u2Length = cpu2le16(CmdBssInfoUapsd.u2Length);
	CmdBssInfoUapsd.u2UapsdServicePeriodTO = cpu2le16(CmdBssInfoUapsd.u2UapsdServicePeriodTO);
#endif
        /* Append this feature */
        AndesAppendCmdMsg(msg, (char *)&CmdBssInfoUapsd, sizeof(CMD_BSSINFO_UAPSD_T));
}

static VOID bssUpdateRssiRmDetParams(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
    CMD_BSSINFO_RSSI_RM_DET_T CmdBssInfoRoamDetection = {0};

    /* Fill TLV format */
    CmdBssInfoRoamDetection.u2Tag = BSS_INFO_ROAM_DETECTION;
    CmdBssInfoRoamDetection.u2Length = sizeof(CMD_BSSINFO_RSSI_RM_DET_T);

#ifdef RT_BIG_ENDIAN
	CmdBssInfoRoamDetection.u2Tag = cpu2le16(CmdBssInfoRoamDetection.u2Tag);
	CmdBssInfoRoamDetection.u2Length = cpu2le16(CmdBssInfoRoamDetection.u2Length);
#endif
    /* Append this feature */
    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoRoamDetection, sizeof(CMD_BSSINFO_RSSI_RM_DET_T));
}

static VOID bssUpdateExtBssInfo(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
    UCHAR ExtBssidIdx = 1;
    CMD_BSSINFO_EXT_BSS_INFO_T CmdBssInfoExtBssInfo = {0};

    //this feature is only for Omac 0x11~0x1f
    ASSERT(bss_info_argument.OwnMacIdx > HW_BSSID_MAX);

    ExtBssidIdx = (bss_info_argument.OwnMacIdx & 0xf);
    CmdBssInfoExtBssInfo.u2Tag = BSS_INFO_EXT_BSS;
    CmdBssInfoExtBssInfo.u2Length = sizeof(CMD_BSSINFO_EXT_BSS_INFO_T);
    CmdBssInfoExtBssInfo.ucMbssTsfOffset = ExtBssidIdx * BCN_TRANSMIT_ESTIMATE_TIME;

    /* Fill TLV format */
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s (BSSINFO_EXT_BSS_INFO), \
                ExtBssidIdx = %d, \
                ucMbssTsfOffset = %d\n",
                __FUNCTION__,
                ExtBssidIdx,
                CmdBssInfoExtBssInfo.ucMbssTsfOffset));
#ifdef RT_BIG_ENDIAN
	CmdBssInfoExtBssInfo.u2Tag = cpu2le16(CmdBssInfoExtBssInfo.u2Tag);
	CmdBssInfoExtBssInfo.u2Length = cpu2le16(CmdBssInfoExtBssInfo.u2Length);
	CmdBssInfoExtBssInfo.ucMbssTsfOffset = cpu2le32(CmdBssInfoExtBssInfo.ucMbssTsfOffset);
#endif
    /* Append this feature */
    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoExtBssInfo, sizeof(CMD_BSSINFO_EXT_BSS_INFO_T));
}


static VOID bssUpdateBmcMngRate(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
    CMD_BSSINFO_BMC_RATE_T CmdBssInfoBmcRate = {0};

    if (bss_info_argument.bss_state >= BSS_ACTIVE)
    {
        CmdBssInfoBmcRate.u2BcTransmit = cpu2le16((UINT16)(bss_info_argument.BcTransmit.word));
        CmdBssInfoBmcRate.u2McTransmit = cpu2le16((UINT16)(bss_info_argument.McTransmit.word));
        CmdBssInfoBmcRate.ucPreambleMode =
            OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
    }

    CmdBssInfoBmcRate.u2Tag = BSS_INFO_BROADCAST_INFO;
    CmdBssInfoBmcRate.u2Length = sizeof(CMD_BSSINFO_BMC_RATE_T);

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s (BSS_INFO_BROADCAST_INFO), \
                CmdBssInfoBmcRate.u2BcTransmit= %d, \
                CmdBssInfoBmcRate.u2McTransmit = %d\n",
                __FUNCTION__,
                le2cpu16(CmdBssInfoBmcRate.u2BcTransmit),
                le2cpu16(CmdBssInfoBmcRate.u2McTransmit)));
#ifdef RT_BIG_ENDIAN
	CmdBssInfoBmcRate.u2Tag = cpu2le16(CmdBssInfoBmcRate.u2Tag);
	CmdBssInfoBmcRate.u2Length = cpu2le16(CmdBssInfoBmcRate.u2Length);
#endif
    /* Append this feature */
    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoBmcRate, sizeof(CMD_BSSINFO_BMC_RATE_T));
}

static VOID bssUpdateSyncModeCtrl(
        struct _RTMP_ADAPTER *pAd,
        BSS_INFO_ARGUMENT_T bss_info_argument,
        struct cmd_msg *msg)
{
    CMD_BSSINFO_SYNC_MODE_CTRL_T CmdBssInfoSyncModeCtrl = {0};

    if (bss_info_argument.bss_state >= BSS_ACTIVE)
    {
    	CmdBssInfoSyncModeCtrl.fgIsEnableSync = TRUE;
#ifdef CONFIG_AP_SUPPORT
        CmdBssInfoSyncModeCtrl.u2BcnInterval = cpu2le16(pAd->CommonCfg.BeaconPeriod);
        CmdBssInfoSyncModeCtrl.ucDtimPeriod = pAd->ApCfg.DtimPeriod;
#endif /* CONFIG_AP_SUPPORT */
    }
	else
		CmdBssInfoSyncModeCtrl.fgIsEnableSync = FALSE;

    CmdBssInfoSyncModeCtrl.u2Tag = BSS_INFO_SYNC_MODE;
    CmdBssInfoSyncModeCtrl.u2Length = sizeof(CMD_BSSINFO_SYNC_MODE_CTRL_T);
#ifdef RT_BIG_ENDIAN
	CmdBssInfoSyncModeCtrl.u2Tag = cpu2le16(CmdBssInfoSyncModeCtrl.u2Tag);
	CmdBssInfoSyncModeCtrl.u2Length = cpu2le16(CmdBssInfoSyncModeCtrl.u2Length);
#endif
    /* Append this feature */
    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoSyncModeCtrl, sizeof(CMD_BSSINFO_SYNC_MODE_CTRL_T));
}

/* BSSinfo tag handle */
static BSS_INFO_HANDLE_T apfBssInfoTagHandle[] =
{
    {BSS_INFO_OWN_MAC_FEATURE,          bssConnectOwnDev},
    {BSS_INFO_BASIC_FEATURE,            bssUpdateBssInfoBasic},
    {BSS_INFO_RF_CH_FEATURE,            bssUpdateChannel},
    {BSS_INFO_PM_FEATURE,               pmUpdateBssInfoPM},
    {BSS_INFO_UAPSD_FEATURE,            pmUpdateBssUapsd},
    {BSS_INFO_ROAM_DETECTION_FEATURE,   bssUpdateRssiRmDetParams},
    {BSS_INFO_LQ_RM_FEATURE,            NULL},
    {BSS_INFO_EXT_BSS_FEATURE,          bssUpdateExtBssInfo},
    {BSS_INFO_BROADCAST_INFO_FEATURE,   bssUpdateBmcMngRate},
    {BSS_INFO_SYNC_MODE_FEATURE,        bssUpdateSyncModeCtrl},
    {BSS_INFO_MAX_NUM_FEATURE,          NULL},
};

INT32 CmdSetSyncModeByBssInfoUpdate(
    RTMP_ADAPTER *pAd,
    BSS_INFO_ARGUMENT_T bss_info_argument)
{
    struct cmd_msg          *msg = NULL;
    CMD_BSSINFO_UPDATE_T    CmdBssInfoUpdate = {0};
    INT32                   Ret = 0;
    UINT16                   ucTLVNumber = 1;
    struct _CMD_ATTRIBUTE attr = {0};

    msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_BSS_INFO + 100);
    if (!msg)
    {
        Ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BSSINFO_UPDATE);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtBssInfoUpdateRsp);

    AndesInitCmdMsg(msg, attr);
    /* Tag = 0, Fill WLAN related header here */
    CmdBssInfoUpdate.ucBssIndex = bss_info_argument.OwnMacIdx;
    CmdBssInfoUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
    CmdBssInfoUpdate.ucAppendCmdTLV = TRUE;
    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoUpdate,
                        sizeof(CMD_BSSINFO_UPDATE_T));

    bssUpdateSyncModeCtrl(pAd, bss_info_argument, msg);

    /* Send out CMD */
    Ret = AndesSendCmdMsg(pAd, msg);

error:
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
    return Ret;
}

INT32 CmdExtBssInfoUpdate(
    RTMP_ADAPTER *pAd,
    BSS_INFO_ARGUMENT_T bss_info_argument)
{
    struct cmd_msg          *msg = NULL;
    CMD_BSSINFO_UPDATE_T    CmdBssInfoUpdate = {0};
    INT32                   Ret = 0;
    UINT8                   i = 0;
    UINT16                   ucTLVNumber = 0;
    struct _CMD_ATTRIBUTE attr = {0};

    msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_BSS_INFO);
    if (!msg)
    {
        Ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

    /* Get number of TLV*/
    for (i = 0; i < BSS_INFO_MAX_NUM; i++)
    {
        if (bss_info_argument.u4BssInfoFeature & (1 << i))
        {
            ucTLVNumber++;
        }
    }

    if ((bss_info_argument.bssinfo_type == WDS) || (bss_info_argument.bssinfo_type == REPT))
    {
        SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	    SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
    }
    else
    {
        SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_BSSINFO_UPDATE_T));
	    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtBssInfoUpdateRsp);
    }
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BSSINFO_UPDATE);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);

    AndesInitCmdMsg(msg, attr);

    /* Tag = 0, Fill WLAN related header here */
    CmdBssInfoUpdate.ucBssIndex = bss_info_argument.ucBssIndex;
    CmdBssInfoUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
    CmdBssInfoUpdate.ucAppendCmdTLV = TRUE;
    AndesAppendCmdMsg(msg, (char *)&CmdBssInfoUpdate, sizeof(CMD_BSSINFO_UPDATE_T));

    for (i = 0; i < BSS_INFO_MAX_NUM; i++)
    {
        if (bss_info_argument.u4BssInfoFeature & apfBssInfoTagHandle[i].BssInfoTag)
        {
            if (apfBssInfoTagHandle[i].BssInfoTagHandler != NULL)
            {
                apfBssInfoTagHandle[i].BssInfoTagHandler(
                                            pAd,
                                            bss_info_argument,
                                            msg);
            }
            else
            {
                MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    ("%s: BssInfoTag = %d no corresponding function handler.\n",
                        __FUNCTION__, apfBssInfoTagHandle[i].BssInfoTag));
            }
        }
    }

    /* Send out CMD */
    Ret = AndesSendCmdMsg(pAd, msg);

error:
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
    return Ret;
}

INT32 CmdExtSetTmrCR(
            struct _RTMP_ADAPTER *pAd,
            UCHAR enable,
            UCHAR BandIdx)
{
    struct cmd_msg          *msg = NULL;
    CMD_TMR_CTRL_T    CmdTmrCtrl = {0};
    TMR_CTRL_SET_TMR_EN_T TmrCtrlSetTmr = {0};
    struct _CMD_ATTRIBUTE attr = {0};
    INT32                   Ret = 0;

    msg = AndesAllocCmdMsg(pAd, sizeof(CMD_TMR_CTRL_T));
    if (!msg)
    {
        Ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TMR_CTRL);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
    AndesInitCmdMsg(msg, attr);

    CmdTmrCtrl.ucTmrCtrlType = SET_TMR_ENABLE;
    CmdTmrCtrl.ucTmrVer = pAd->chipCap.TmrHwVer;
    AndesAppendCmdMsg(msg, (char *)&CmdTmrCtrl, sizeof(CMD_TMR_CTRL_T));

    if (enable) {
        TmrCtrlSetTmr.ucEnable = TRUE;
        if (enable == TMR_INITIATOR)
            TmrCtrlSetTmr.ucRole = 0;
        else if (enable == TMR_RESPONDER)
        {
            TmrCtrlSetTmr.ucRole = 1;
            //TODO:Carter, fix ugly hard code.
            TmrCtrlSetTmr.aucType_Subtype[0] = 0x34;//bit0:1_0x2, bit2:5_0xd
            TmrCtrlSetTmr.aucType_Subtype[1] = 0xff;//unused.
            TmrCtrlSetTmr.aucType_Subtype[2] = 0xff;//unused.
            TmrCtrlSetTmr.aucType_Subtype[3] = 0xff;//unused.
        }
    }
    else
    {
        TmrCtrlSetTmr.ucEnable = FALSE;
    }

    TmrCtrlSetTmr.ucDbdcIdx = BandIdx;

    AndesAppendCmdMsg(msg, (char *)&TmrCtrlSetTmr,
                    sizeof(TMR_CTRL_SET_TMR_EN_T));

    /* Send out CMD */
    Ret = AndesSendCmdMsg(pAd, msg);

error:
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
    return Ret;
}

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
INT32 CmdP2pNoaOffloadCtrl(RTMP_ADAPTER *ad, UINT8 enable)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_NOA_CTRL_T extCmdNoaCtrl;
	int ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(ad, sizeof(struct _EXT_CMD_NOA_CTRL_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_NOA_OFFLOAD_CTRL);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(struct _EXT_CMD_NOA_CTRL_T));
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);

    AndesInitCmdMsg(msg, attr);

	os_zero_mem(&extCmdNoaCtrl, sizeof(extCmdNoaCtrl));

	extCmdNoaCtrl.ucMode1 = enable;

	AndesAppendCmdMsg(msg, (char *)&extCmdNoaCtrl, sizeof(extCmdNoaCtrl));
	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}
#endif /* defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) */


static VOID CmdRxHdrTransUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
                                        (struct _EVENT_EXT_CMD_RESULT_T *)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
    	("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
    	    __FUNCTION__, EventExtCmdResult->ucExTenCID));
#ifdef RT_BIG_ENDIAN
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
#endif
	if (EventExtCmdResult->u4Status != 0)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::BUG::EventExtCmdResult.u4Status = 0x%x\n",
                __FUNCTION__, EventExtCmdResult->u4Status));
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
	}

}


INT32 CmdRxHdrTransUpdate(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid,
                            BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP)
{
	struct cmd_msg 			*msg = NULL;
	INT32					Ret = 0;
	EXT_RX_HEADER_TRANSLATE_T	ExtRxHdrTrans = {0};
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd,  sizeof(EXT_RX_HEADER_TRANSLATE_T));
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_HDR_TRANS);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdRxHdrTransUpdateRsp);

    AndesInitCmdMsg(msg, attr);

	ExtRxHdrTrans.ucOperation = RXHDR_TRANS;
	ExtRxHdrTrans.ucEnable = En;
	ExtRxHdrTrans.ucCheckBssid = ChkBssid;
	ExtRxHdrTrans.ucInsertVlan = InSVlan;
	ExtRxHdrTrans.ucRemoveVlan = RmVlan;
	ExtRxHdrTrans.ucUserQosTid = !SwPcP;
	ExtRxHdrTrans.ucTranslationMode = 0;

	AndesAppendCmdMsg(msg, (char *)&ExtRxHdrTrans,
            sizeof(EXT_RX_HEADER_TRANSLATE_T));

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdRxHdrTransBLUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{

	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
                                        (struct _EVENT_EXT_CMD_RESULT_T *)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
	        __FUNCTION__, EventExtCmdResult->ucExTenCID));
#ifdef RT_BIG_ENDIAN
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
#endif
	if (EventExtCmdResult->u4Status != 0)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::BUG::EventExtCmdResult.u4Status = 0x%x\n",
                __FUNCTION__, EventExtCmdResult->u4Status));
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("%s::EventExtCmdResult.u4Status = 0x%x\n",
            __FUNCTION__, EventExtCmdResult->u4Status));
	}
}


#ifdef BA_TRIGGER_OFFLOAD
INT32 CmdAutoBATrigger(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_AUTO_BA_T ExtAutoBa = {0};

    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_AUTO_BA_T));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_AUTO_BA);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);

	ExtAutoBa.ucAutoBaEnable = Enable;
	ExtAutoBa.ucTarget = 0;
	ExtAutoBa.u4Timeout = cpu2le32(Timeout);

	AndesAppendCmdMsg(msg, (char *)&ExtAutoBa,
            sizeof(EXT_CMD_ID_AUTO_BA_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}
#endif


#ifdef IGMP_SNOOP_SUPPORT
INT32 CmdMcastCloneEnable(RTMP_ADAPTER *pAd, BOOLEAN Enable)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MCAST_CLONE_T ExtMcastClone = {0};

    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MCAST_CLONE_T));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MCAST_CLONE);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);

	ExtMcastClone.ucMcastCloneEnable = Enable;

	AndesAppendCmdMsg(msg, (char *)&ExtMcastClone,
            sizeof(EXT_CMD_ID_MCAST_CLONE_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


BOOLEAN CmdMcastEntryInsert(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T ExtMcastEntryInsert;

    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MULTICAST_ENTRY_INSERT);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);

	NdisMoveMemory(&ExtMcastEntryInsert.aucGroupId[0], (UCHAR *)GrpAddr, 6);
	ExtMcastEntryInsert.ucBssInfoIdx = BssIdx;
	ExtMcastEntryInsert.ucMcastEntryType = Type;
	ExtMcastEntryInsert.ucMemberNum = 1;
	NdisMoveMemory(&ExtMcastEntryInsert.aucMemberAddr[0], (UCHAR *)MemberAddr, 6);
	ExtMcastEntryInsert.ucIndex = WlanIndex;

	AndesAppendCmdMsg(msg, (char *)&ExtMcastEntryInsert,
            sizeof(EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


BOOLEAN CmdMcastEntryDelete(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T ExtMcastEntryDelete;

    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MULTICAST_ENTRY_DELETE);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

    AndesInitCmdMsg(msg, attr);

	NdisMoveMemory(&ExtMcastEntryDelete.aucGroupId[0], (UCHAR *)GrpAddr, 6);
	ExtMcastEntryDelete.ucBssInfoIdx = BssIdx;

	if (MemberAddr)
	{
		ExtMcastEntryDelete.ucMemberNum = 1;
		NdisMoveMemory(&ExtMcastEntryDelete.aucMemberAddr[0], (UCHAR *)MemberAddr, 6);
		ExtMcastEntryDelete.ucIndex = WlanIndex;
	}
	else
	{
		ExtMcastEntryDelete.ucMemberNum = 0;
	}

	AndesAppendCmdMsg(msg, (char *)&ExtMcastEntryDelete,
            sizeof(EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}
#endif


INT32 CmdRxHdrTransBLUpdate(RTMP_ADAPTER *pAd, UINT8 Index, UINT8 En, UINT16 EthType)
{
	struct cmd_msg 			*msg = NULL;
	INT32					Ret = 0;
	EXT_RX_HEADER_TRANSLATE_BL_T	ExtRxHdrTransBL = {0};
    struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd,  sizeof(EXT_RX_HEADER_TRANSLATE_BL_T));
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_HDR_TRANS);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdRxHdrTransBLUpdateRsp);

    AndesInitCmdMsg(msg, attr);

	ExtRxHdrTransBL.ucOperation = RXHDR_BL;
	ExtRxHdrTransBL.ucCount = 1;
	ExtRxHdrTransBL.ucBlackListIndex = Index;
	ExtRxHdrTransBL.ucEnable = En;
	ExtRxHdrTransBL.usEtherType = cpu2le16(EthType);
	AndesAppendCmdMsg(msg, (char *)&ExtRxHdrTransBL,
            sizeof(EXT_RX_HEADER_TRANSLATE_BL_T));
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdExtGeneralTestRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	printk("General Test status=%d\n", Status);
	switch (Status)
	{
		case TARGET_ADDRESS_LEN_SUCCESS:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                ("%s: General Test success\n", __FUNCTION__));
			break;
		default:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s: General Test success Unknow Status(%d)\n",
                __FUNCTION__, Status));
			break;
	}
}

#ifdef ERR_RECOVERY

INT32 CmdExtGeneralTestOn(
    struct _RTMP_ADAPTER *pAd,
    BOOLEAN enable)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_GENERAL_TEST_T CmdGeneralTest = {0};
    UINT32 MsgLen;
	INT32 Ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

    MsgLen = sizeof(CMD_STAREC_UPDATE_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GENERAL_TEST);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtGeneralTestRsp);

    AndesInitCmdMsg(msg, attr);

	/* Fill command related header here*/
    CmdGeneralTest.ucCategory = GENERAL_TEST_CATEGORY_SIM_ERROR_DETECTION;
    CmdGeneralTest.ucAction = GENERAL_TEST_ACTION_SWITCH_ON_OFF;
    CmdGeneralTest.Data.rGeneralTestSimErrorSwitchOnOff.ucSwitchMode = enable;

	AndesAppendCmdMsg(msg, (char *)&CmdGeneralTest, sizeof(EXT_CMD_GENERAL_TEST_T));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s: CmdExtGeneralTest:\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s: ucCategory=%d, ucAction=%d, ucSwitchMode=%d\n",
            __FUNCTION__, CmdGeneralTest.ucCategory, CmdGeneralTest.ucAction,
            CmdGeneralTest.Data.rGeneralTestSimErrorSwitchOnOff.ucSwitchMode));

    /* Send out CMD */
    Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

INT32 CmdExtGeneralTestMode(
    struct _RTMP_ADAPTER *pAd,
    UINT8 mode,
    UINT8 submode)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_GENERAL_TEST_T CmdGeneralTest = {0};
    UINT32 MsgLen;
	INT32 Ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

    MsgLen = sizeof(CMD_STAREC_UPDATE_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GENERAL_TEST);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtGeneralTestRsp);

    AndesInitCmdMsg(msg, attr);

	/* Fill command related header here*/
    CmdGeneralTest.ucCategory = GENERAL_TEST_CATEGORY_SIM_ERROR_DETECTION;
    CmdGeneralTest.ucAction = GENERAL_TEST_ACTION_RECOVERY;
    CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucModule = mode;
    CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucSubModule = submode;

	AndesAppendCmdMsg(msg, (char *)&CmdGeneralTest, sizeof(EXT_CMD_GENERAL_TEST_T));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s: CmdExtGeneralTest:\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s: ucCategory=%d, ucAction=%d, ucModule=%d ucSubModule=%d\n",
            __FUNCTION__, CmdGeneralTest.ucCategory, CmdGeneralTest.ucAction,
            CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucModule,
            CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucSubModule));

    /* Send out CMD */
    Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}
#endif /* ERR_RECOVERY */

INT32 CmdExtGeneralTestAPPWS(
    struct _RTMP_ADAPTER *pAd,
    UINT action)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_GENERAL_TEST_T CmdGeneralTest = {0};
    UINT32 MsgLen;
	INT32 Ret = 0;
    struct _CMD_ATTRIBUTE attr = {0};

    MsgLen = sizeof(CMD_STAREC_UPDATE_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);
	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
    SET_CMD_ATTR_TYPE(attr, EXT_CID);
    SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GENERAL_TEST);
    SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
    SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
    SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
    SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
    SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtGeneralTestRsp);

    AndesInitCmdMsg(msg, attr);

	/* Fill command related header here*/
    CmdGeneralTest.ucCategory = GENERAL_TEST_CATEGORY_APPWS;
    CmdGeneralTest.ucAction = action;

	AndesAppendCmdMsg(msg, (char *)&CmdGeneralTest, sizeof(EXT_CMD_GENERAL_TEST_T));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("%s: ucCategory=%d, ucAction=%d\n",
            __FUNCTION__, CmdGeneralTest.ucCategory, CmdGeneralTest.ucAction));

    /* Send out CMD */
    Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

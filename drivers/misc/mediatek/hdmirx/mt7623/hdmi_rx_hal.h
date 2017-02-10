#ifndef _HDMI_RX_HAL_H_
#define _HDMI_RX_HAL_H_

#include "hdmirx.h"
#include "typedef.h"

enum eHDMI_SWITCH_NO {
	HDMI_SWITCH_INIT = 0,
	HDMI_SWITCH_1,
	HDMI_SWITCH_2,
	HDMI_SWITCH_3,
	HDMI_SWITCH_4,
	HDMI_SWITCH_5,
	HDMI_SWITCH_6
};

typedef enum
{
  RX_NON_DEEP= 0x00,
  RX_30BITS_DEEP_COLOR =0x01,
  RX_36BITS_DEEP_COLOR =0x02,
  RX_48BITS_DEEP_COLOR =0x03
}RX_DEEP_COLOR_MODE;

typedef enum
{
    HDMI_ANA_BAND_NULL = 0,
    HDMI_ANA_BAND_10_27M, // 10~27M
    HDMI_ANA_BAND_27_40M, // 27~40M
    HDMI_ANA_BAND_40_160M, //40 ~160M
    HDMI_ANA_BAND_160_250M,
    HDMI_ANA_BAND_250_MAX,
}HDMI_ANA_BAND;

typedef enum
{
  RX_CH_MAP_RGB=0,
  RX_CH_MAP_RBG=1,
  RX_CH_MAP_GRB=2,
  RX_CH_MAP_BRG=3,
  RX_CH_MAP_GBR=4,
  RX_CH_MAP_BGR=5,
  RX_CH_MAP_RGB1=6,
  RX_CH_MAP_RGB2=7,

}RX_VIDEO_CH_MAP_TYPE;

enum eHDMI_Rx_PHY_RESET {
	HDMI_RST_ALL = 1,
	HDMI_RST_EQ,
	HDMI_RST_DEEPCOLOR,
	HDMI_RST_FIXEQ,
	HDMI_RST_RTCK
};

typedef enum
{
    FORMAT_RJ = 0,
    FORMAT_LJ = 1,
    FORMAT_I2S = 2
} RX_AUD_DATA_FORMAT_T;

typedef enum LRCK_CYC_T {
    LRCK_CYC_16,
    LRCK_CYC_24,
    LRCK_CYC_32
}LRCK_CYC_T;

void vHalSetRxPclk(UINT8 u1Data);
UINT32 u4HalGetRxPixelClock(void);
UINT8 u1HalChkDataEnableExist(void);//check Sync
void vHalClearRxPclkChgStatus(void);
UINT8 vHalGetRxHdcpStatus(void);
BOOL fgHalCheckHdmiRXAuthDone(UINT8 u1Data);
UINT8 u1HalChkCKDTExist(void);//check Sync
UINT8 u1HalChkCKDTExist(void);//check Sync
void bHDMIPHYReset(UINT8 resettype);
void vHalResetHdmiRxPhySPModule(void);
void vHalSwResetHdmiRxModule(void);
void vHalMuteHdmiRxAudioOut(void);
void vHalUnMuteHdmiRxAudioOut(void);
void vHalOpenAPLL(void);
void vHalSetRxI2sLRInv(UINT8 u1LRInv);
void vHalSetRxI2sAudFormat(UINT8 u1fmt, UINT8 u1Cycle);
void vHalSetLRCKEdge(UINT8 u1EdgeFmt);
void vHalSetRxI2sMclk(UINT8 u1MclkType);
UINT8 u1HalChkPwr5VExist(void);

enum HDMI_RX_AUDIO_FS {
    SW_44p1K,
    SW_88p2K,
    SW_176p4K,
    SW_48K,
    SW_96K,
    SW_192K,
    SW_32K,
    HW_FS
};
void vHalEnableRxAACToSd0Sd1Sd2Sd3(void);
void vHalSetRxAudioFS(enum HDMI_RX_AUDIO_FS eFS);
void vHalEnableRxAudClk(void);
void vHalSetRxAudMuteCondition(void);
void vHalSelANABand(void);
void vHalRxHwInit(void);
void vHalRxHDMITMDSCTRL(UINT8 bOnOff);
void vHalSetEqZeroValueVar(UINT32 u4Data);
void vHalSetEqBoostValueVar(UINT32 u4Data);
void vHalSetEqSelValueVar(UINT32 u4Data);
void vHalSetEqGainValueVar(UINT32 u4Data);
void vHalSetLBWValueVar(UINT32 u4Data);
void vHalSetRxHdcpMask1Var(UINT32 u4Data);
void vHalSetRxHdcpMask2Var(UINT32 u4Data);

UINT8 u1HalReadGamutHb0(void);
UINT8 u1HalReadGamutHb1(void);
UINT8 u1HalReadGamutHb2(void);
UINT8 u1HalReadGamutPB0(void);
UINT8 u1HalReadGamutPB1(void);
UINT8 u1HalReadGamutPB2(void);
UINT8 u1HalReadGamutPB3(void);
UINT8 u1HalReadGamutPB4(void);
UINT8 u1HalReadGamutPB5(void);
UINT8 u1HalReadGamutPB6(void);
UINT8 u1HalReadGamutPB7(void);
UINT8 u1HalReadGamutPB8(void);
UINT8 u1HalReadGamutPB9(void);
UINT8 u1HalReadGamutPB10(void);
UINT8 u1HalReadGamutPB11(void);
UINT8 u1HalReadGamutPB12(void);
UINT8 u1HalReadGamutPB13(void);
UINT8 u1HalReadGamutPB14(void);
void fgHalClearGamutIntStatus(void);
void vHalClearNewVSIntStatus(void);
void vHalClearNewNOVSIntStatus(void);
void vHalClearNewISRC1IntStatus(void);
void vHalSetVSNewOnly(BOOL fgEnable);
void vHalSetISRC1NewOnly(BOOL fgEnable);
void vHalClearVSYNCIntStatus(void);
void vHalGetUnRecPacket(BYTE *bUnknowPacketData);
void vHalClearNewAviIntStatus(void);
void vHalClearNewAudIntStatus(void);
void vHalClearNewSpdIntStatus(void);
void vHalClearNewMpegIntStatus(void);
void vHalClearNewUnRecIntStatus(void);
void vHalClearNewAcpIntStatus(void);
void vSetSelectUnRecpacket(BOOL fgEnable,BYTE bHeader);
BOOL vHalCheckIsPclkChanged(void);
void vHalClearPclkChangedIntState(void);
BOOL vHalCheckGcpMuteEnable(void);
BYTE vHalGetAcpPacketHeader(void);
void vHalGetAcpPacket(BYTE *bAcpPacketData);
void vSetSelectAcppacket(BYTE bHeader);
BOOL fgHalCheckAcpInforFrameExist(void);
UINT8 u1HalRxGetAcpType(void);
BOOL fgHalCheckIsNewAcp(void);
void fgHalClearAcpIntStatus(void);
void vHalGetAviInfoframe(BYTE *bAviinfoframe);
void vHalGetAudioInfoframe(BYTE *bAudioInfoframe);
void vHalGetVSInfoframe(BYTE *bVSinfoframe);
void vHalGetISRC1Infoframe(BYTE *bISRC1infoframe);
BYTE vHalGetMpegAddrHeader(void);
void vHalMpegAddrSetSelectPacket(BYTE bHeader);
BYTE vHalMpegAddrGetSelectPacket(void);
void vHalGetMpegInfoframe(BYTE *bMpegInfoframeData);
void vHalGetSpdInfoframe(BYTE *bSpdInfoframeData);
void vHalGetGamutPacket(BYTE *bGamutData);
UINT8 u4HalCheckDeepColorMode(void);
void vHalReInitAudioClock(void);
BOOL fgHalCheckIsNoAvi(void);
void vHalClearVideoModeByte0(void);
void vHalClearVideoModeByte1(void);
void vHalClearVideoModeByte2(void);
void vHalClearVideoModeByte3(void);
void vHalClearIntrState1Bit0_Bit7(void);
void vHalRxDisable656SyncMode(void);
void vHalRxDisable422to444UpSample(void);
void vHalSetRxRGBBlankValue(UINT8 u1Blue , UINT8 u1Green, UINT8 u1Red);
void vHalSetRxYCbCrBlankValue(UINT8 u1Cb , UINT8 u1Y, UINT8 u1Cr);
void vHalWriteVideoChMap(UINT8 u1Data);
void vHalClearRxHresChgIntState(void);
void vHalRxEnableTDFifoAutoRead(void);
void vHalRxDisableTDFifoAutoRead(void);
BOOL fgHalCheckRXHdcpDecrptOn(void);
BOOL fgHalCheckRXH4096DetectOn(void);
void vHalClearModeChgIntState(void);
BOOL fgHalCheckRxIsVResStable(void);
BOOL fgHalCheckRxIsVResMute(void);
void vHalSetRxVMute(void);
void vHalClearRxVMute(void);
void vHalDisableRxAvMute(void);
void HDMI_HalEnableAvMuteRecv(void);
void vHalRxEnableAvMuteRecv(void);
BOOL fgHalCheckInterlaceDetect(void);
void vHalSetHDMIRXPowerOff(void);
BOOL fgHalIsINTR3_CEA_NEW_CP(void);
void vHalHDMIRxEnableVsyncInt(BOOL fgEnable);
BOOL fgHalIsINTR3_CP_SET_MUTE(void);
BOOL fgHalIsINTR3_P_ERR(void);
BOOL fgHalIsINTR3_NEW_UNREC(void);
BOOL fgHalIsINTR3_NEW_MPEG(void);
BOOL fgHalIsINTR3_NEW_AUD(void);
BOOL fgHalIsINTR3_NEW_SPD(void);
BOOL fgHalIsINTR3_NEW_AVI(void);
void  vHalEnableINTR3_Send_AVMUTE(BOOL fgEnable);
BOOL fgHalIsINTR_NEW_VS(void);
BOOL fgHalIsINTR_VSYNC(void);
BOOL fgHalIsINTR_NO_VS(void);
BOOL fgHalIsINTR_NEW_ISRC1(void);
void vHalHDMIRxEnableVsyncInt(BOOL fgEnable);
BOOL fgHalIsINTR2_HDMI_MODE(void);
BOOL fgHalIsINTR2_VSYNC(void);
BOOL fgHalIsINTR2_SOFT_INTR_EN(void);
BOOL fgHalIsINTR2_CKDT(void);
void  vHalClearINTR2_CKDT(void);
void  vHalEnableINTR2_CKDT(BOOL fgenable);
BOOL fgHalIsINTR2_SCDT(void);
BOOL fgHalIsINTR2_GOT_CTS(void);
BOOL fgHalIsINTR2_NEW_AUD_PKT(void);
BOOL fgHalIsINTR2_CLK_CHG(void);
BOOL fgHalIsINTR1_HW_CTS_CHG(void);
BOOL fgHalIsINTR1_HW_N_CHG(void);
BOOL fgHalIsINTR1_FIFO_ERR(void);
BOOL fgHalIsSOFT_INTR_EN(void);
BOOL fgHalIsINTR_OD(void);
BOOL fgHalIsINTR_POLARITY(void);
BOOL fgHalIsINTR_STATE(void);
UINT32 u4HalReadINTR_STATE0(void);
UINT32 u4HalReadINTR_STATE1(void);
BOOL fgHalIsINTR7_RATIO_ERROR(void);
BOOL fgHalIsINTR7_AUD_CH_STAT(void);
BOOL fgHalIsINTR7_GCP_CD_CHG(void);
BOOL fgHalIsINTR7_GAMUT(void);
BOOL fgHalIsINTR7_HBR(void);
BOOL fgHalIsINTR7_SACD(void);
BOOL fgHalIsINTR6_PRE_UNDERUN(void);
BOOL fgHalIsINTR6_PRE_OVERUN(void);
BOOL fgHalIsINTR6_PWR5V_RX2(void);
BOOL fgHalIsINTR6_PWR5V_RX1(void);
BOOL fgHalIsINTR6_NEW_ACP(void);
BOOL fgHalIsINTR6_P_ERR2(void);
BOOL fgHalIsINTR6_PWR5V_RX0(void);
BOOL fgHalIsINTR5_FN_CHG(void);
BOOL fgHalIsINTR5_AUDIO_MUTE(void);
BOOL fgHalIsINTR5_BCH_AUDIO_ALERT(void);
BOOL fgHalIsINTR5_VRESCHG(void);
BOOL fgHalIsINTR5_HRESCHG(void);
BOOL fgHalIsINTR5_POLCHG(void);
BOOL fgHalIsINTR5_INTERLACEOUT(void);
BOOL fgHalIsINTR5_AUD_SAMPLE_F(void);
BOOL fgHalIsINTR4_PKT_RECEIVED_ALERT(void);
BOOL fgHalIsINTR4_HDCP_PKT_ERR_ALERT(void);
BOOL fgHalIsINTR4_T4_PKT_ERR_ALERT(void);
BOOL fgHalIsINTR4_NO_AVI(void);
BOOL fgHalIsINTR4_CTS_DROPPED_ERR(void);
BOOL fgHalIsINTR4_CTS_REUSED_ERR(void);
BOOL fgHalIsINTR4_OVERRUN(void);
BOOL fgHalIsINTR4_UNDERRUN(void);
void vHalSetIntOnNewAviOnlyEnable(BOOL fgEnable);
void vHalSetIntOnNewAcpOnlyEnable(BOOL fgEnable);
void vHalSetIntOnNewSpdOnlyEnable(BOOL fgEnable);
void vHalSetIntOnNewAudioInfOnlyEnable(BOOL fgEnable);
void vHalSetIntOnNewMpegInfOnlyEnable(BOOL fgEnable);
void vHalSetIntOnNewUnrecInfOnlyEnable(BOOL fgEnable);
BOOL vHalHdmiRxCrc(INT16 ntry);
BOOL fgHalIsINTR8_AUDFMTCHG(void);
BOOL fgHalIsINTR8_AUDCHSTATUSCHG(void);
void vHDMIRxIntMask(BOOL fgOn);
BOOL   fgHalHDMIRxHDAudio(void);
BOOL   fgHalHDMIRxDSDAudio(void);
BOOL   fgHalHDMIRxAudioPkt(void);
UINT8   u1HalHDMIRxAudioCHSTAT0(void);
UINT8   u1HalHDMIRxAudioCHSTAT1(void);
UINT8   u1HalHDMIRxAudioCHSTAT2(void);
UINT8   u1HalHDMIRxAudioCHSTAT3(void);
UINT8   u1HalHDMIRxAudioCHSTAT4(void);
BOOL fgHalHDMIRxMultiPCM(void);
UINT8  u1HalHDMIRxAudioChannelNum(void);
UINT8 u1HalHDMIRxAudFsGet(void);
UINT8  u1HalHDMIRxAudValidCHGet(void);
void  vHalHDMIRxSetAudValidCH(UINT8 u1ValidCh);
void  vHalHDMIRxSetAudMuteCH(UINT8 u1MuteCh);
UINT32  vHalHDMIRxAudErrorGet(void);
void  vHalHDMIRxAudResetAudio(void);
void  vHalHDMIRxAudResetAudioFifo(void);
void vHalHDMIRxAudResetMCLK(void);
void HalHdmiRxAudioReset(void);
BOOL fgHalCheckIsAAC(void);
void vHalSetHDMIRxHBR(BOOL fgHBR);
void vHalSetHDMIRxI2S(void);
BOOL fgHalHDMIRxAPLLStatus(void);
void vHalSetHDMIRxDSD(BOOL fgDSD);
void vHalHdmiRxAudBypass(BOOL fgBypass,BOOL fgBypassSPDIF2Tx);
void vHalSetRxAudioFS(enum HDMI_RX_AUDIO_FS eFS);
void vHalSetHDMIRxI2S(void);
void vHalSetRxHDMIHPDHigh(UINT8 u1HDMICurrSwitch);
void vHalSetRxHDMIHPDLow(UINT8 u1HDMICurrSwitch);
UINT32 HDMI_HalGetXclkCnt(void);
UINT32 HDMI_HalGetTmdsPeriod(void);
UINT32 HDMI_HalGetTmdsClockExt(void);
void vHalHDMIRxEnableVsyncInt(BOOL fgEnable);
BOOL fgHalIsINTR2_CKDT(void);
void  vHalEnableINTR2_CKDT(BOOL fgenable);
void  vHalClearINTR2_CKDT(void);
unsigned char u1HalChkCKDTExist(void);//check Sync
void vHalClearVSYNCIntStatus(void);
BOOL fgHalIsINTR_VSYNC(void);
BOOL fgHalChkSCDTEnable(void);//check Sync
UINT32 vHalRxGetActiveWidth(void);
UINT32 vHalRxGetVBackPorch(void);
UINT32 vHalRxGetActiveHeight(void);
UINT32 vHalRxGetHTotal(void);
UINT32 vHalRxGetVTotal(void);
BOOL fgHalCheckRxPclkIs2XRepeat(void);
UINT32 HDMI_HalGetPixelClockExt(void);
UINT32 vHalRxGetVFrontPorch(void);
void vHalRxResetTDFifoAutoRead(void);
BOOL vHalCheckRxIsHdmiMode(void);
BOOL fgHalCheckAviInforFrameExist(void);
UINT8 u1HalReadAviType(void);
UINT8 u1HalReadAviVersion(void);
UINT8 u1HalReadAviLength(void);
UINT8 u1HalReadAviCheckSum(void);
UINT8 u1HalReadAviByte1(void);
UINT8 u1HalReadAviByte2(void);
UINT8 u1HalReadAviByte3(void);
UINT8 u1HalReadAviByte4(void);
UINT8 u1HalReadAviByte5(void);
UINT8 u1HalReadAviByte6(void);
UINT8 u1HalReadAviByte7(void);
UINT8 u1HalReadAviByte8(void);
UINT8 u1HalReadAviByte9(void);
UINT8 u1HalReadAviByte10(void);
UINT8 u1HalReadAviByte11(void);
UINT8 u1HalReadAviByte12(void);
UINT8 u1HalReadAviByte13(void);
UINT8 u1HalReadAviByte14(void);
UINT8 u1HalReadAviByte15(void);
UINT8 u1HalReadAudioInfType(void);
UINT8 u1HalReadAudioInfVersion(void);
UINT8 u1HalReadAudioInfLength(void);
UINT8 u1HalReadAudioInfCheckSum(void);
UINT8 u1HalReadAudioInfByte1(void);
UINT8 u1HalReadAudioInfByte2(void);
UINT8 u1HalReadAudioInfByte3(void);
UINT8 u1HalReadAudioInfByte4(void);
UINT8 u1HalReadAudioInfByte5(void);
UINT8 u1HalReadAudioInfByte6(void);
UINT8 u1HalReadAudioInfByte7(void);
UINT8 u1HalReadAudioInfByte8(void);
UINT8 u1HalReadAudioInfByte9(void);
UINT8 u1HalReadAudioInfByte10(void);
UINT8 u1HalReadHb1HeaderAcpType(void);
UINT8 u1HalReadAcpHb2Header(void);
UINT8 u1HalReadAcpPB0(void);
UINT8 u1HalReadAcpPB1(void);
UINT8 u1HalReadAcpPB2(void);
UINT8 u1HalReadAcpPB3(void);
UINT8 u1HalReadAcpPB4(void);
UINT8 u1HalReadAcpPB5(void);
UINT8 u1HalReadAcpPB6(void);
UINT8 u1HalReadAcpPB7(void);
UINT8 u1HalReadAcpPB8(void);
UINT8 u1HalReadAcpPB9(void);
UINT8 u1HalReadAcpPB10(void);
UINT8 u1HalReadAcpPB11(void);
UINT8 u1HalReadAcpPB12(void);
UINT8 u1HalReadAcpPB13(void);
UINT8 u1HalReadAcpPB14(void);
UINT8 u1HalReadAcpPB15(void);
UINT8 u1HalReadSPDType(void);
UINT8 u1HalReadSPDVersion(void);
UINT8 u1HalReadSPDLength(void);
UINT8 u1HalReadSPDByte1(void);
UINT8 u1HalReadSPDByte2(void);
UINT8 u1HalReadSPDByte3(void);
UINT8 u1HalReadSPDByte4(void);
UINT8 u1HalReadSPDByte5(void);
UINT8 u1HalReadSPDByte6(void);
UINT8 u1HalReadSPDByte7(void);
UINT8 u1HalReadSPDByte8(void);
UINT8 u1HalReadSPDByte9(void);
UINT8 u1HalReadSPDByte10(void);
UINT8 u1HalReadSPDByte11(void);
UINT8 u1HalReadSPDByte12(void);
UINT8 u1HalReadSPDByte13(void);
UINT8 u1HalReadSPDByte14(void);
UINT8 u1HalReadSPDByte15(void);
UINT8 u1HalReadSPDByte16(void);
UINT8 u1HalReadSPDByte17(void);
UINT8 u1HalReadSPDByte18(void);
UINT8 u1HalReadSPDByte19(void);
UINT8 u1HalReadSPDByte20(void);
UINT8 u1HalReadSPDByte21(void);
UINT8 u1HalReadSPDByte22(void);
UINT8 u1HalReadSPDByte23(void);
UINT8 u1HalReadSPDByte24(void);
UINT8 u1HalReadSPDByte25(void);
UINT8 u1HalReadSPDByte26(void);
void vHalResetHdmiRxTotalModule(void);
void vHalRxHDMITMDSCTRL(BOOL fgOn);
UINT32 u4HalGetRxHwCTSValue(void);
UINT32 u4HalGetRxHwNValue(void);
BOOL fgHalCheckIsNonDeepColorMode(void);
void  vHalRxHdcpReset(void);
void  vHalDisableHDCPDDCPort(void);
void  vHalEnableHDCPDDCPort(void);
void  vHalSetKsvReadyBit(void);
void  vHalClearKsvReadyBit(void);
void  vHalLoadHdcpKey(UINT8 *prHdcpKey);
void  vHalSetRepeaterMode(BOOL fgRepeater);
void  vHalSetHdmiCapable(BOOL fgHdmiCapable);
void  vHalWriteKsvList(BYTE *prKsvList,BYTE bCount);
UINT32  vHalGetKsvFifoAddr(void);
void  vHalTriggerSHA(void);
void  vHalRptStartAddrClr(void);
BOOL  fgHalHdcpAuthenticationStart(void);
BOOL fgHalHdcpAuthenticationDone(void);
void  vHalClearHdcpAuthenticationStartStatus(void);
void  vHalClearHdcpAuthenticationDoneStatus(void);
BOOL  fgHalHdcpHdmiMode(void);
void  vHalSetBstatus(UINT16 u2Bstatus);
void  vHalSetSHALength(UINT32 u4Length);
void  vHalSetSHAAddr(UINT16 bAddr);
BOOL fgHalCheckRxHResChg(void);
void  vHalSetKsvStop(BOOL fgRiscAccressEnable);
void  RxUse27M(void);
BOOL  fgHalIsVReady(void);
void  vGetRxAKsv(BYTE *prRxAKSV);
UINT16 vGetRxRi(void);
void  vGetRxAn(BYTE *prRxAn);
void  vGetRxBKsv(BYTE *prRxBKSV);
UINT32  u4GetVblank(void);
void hdmirxanapoweroff(void);

#endif





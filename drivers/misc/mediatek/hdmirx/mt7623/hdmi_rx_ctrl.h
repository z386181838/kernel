#ifndef _HDMI_RX_CTRL_H_
#define _HDMI_RX_CTRL_H_

#include "video_in_if.h"
#include "hdmi_rx_hal.h"


// HDMI Event Flag
extern UINT8   _bHdmiFlag;
#define vSetHdmiFlg(arg) (_bHdmiFlag |= (arg))
#define vClrHdmiFlg(arg) (_bHdmiFlag &= (~(arg)))
#define fgIsHdmiFlgSet(arg) ((_bHdmiFlag & (arg)) > 0)
#define HDMI_CHECK (1<<0)
#define HDMI_OCLKDIV2 (1<<1)
#define HDMI_AUDIO_ON (1<<2)
#define HDMI_MODE_CHG (1<<3)

//----------------------------------------------------------------------------
// HDMI option flag
//----------------------------------------------------------------------------

#define HDMI_SUPPORT_EXT_SWITCH (0)
#define HDMI_BYPASS_INITIAL_FLOW (1)
#define HDMI_OFFON_MUTE_COUNT (100)	//ms
#define DVI_WAIT_STABLE_COUNT (120)
#define DVI_WAIT_NOSIGNAL_COUNT (12)	//second
#define HDMI_WAIT_SCDT_STABLE_COUNT (1)	//second
#define HDMI_TMDS_EQ_ZERO_VALUE (0x1)
#define HDMI_TMDS_EQ_BOOST_VALUE (0xd)
#define HDMI_TMDS_EQ_SEL_VALUE (0xd) //default
#define HDMI_TMDS_EQ_GAIN_VALUE (0x2)
#define HDMI_HDCP_Mask1 (0xff)
#define HDMI_HDCP_Mask2 (0xc3)

//HDCP Bstatus
#define DEVICE_COUNT		 0x7F
#define MAX_DEVS_EXCEEDED	 (0x01<<7)
#define DEVICE_DEPTH		 (0x07<<8)
#define MAX_CASCADE_EXCEEDED (0x1<<11)
#define HDMI_MODE			 (0x1<<12)

#define TX_MAX_KSV_COUNT 9
#define RX_MAX_KSV_COUNT 10
#define HDCP_RECEIVER   0
#define HDCP_REPEATER   1

enum
{
	HDMI_3D_Structure_FramePacking=0,
	HDMI_3D_Structure_FieldAlternative,
	HDMI_3D_Structure_LineAlternative,
	HDMI_3D_Structure_SideBySideFull,
	HDMI_3D_Structure_LDepth,
	HDMI_3D_Structure_LDepthGraph,
	HDMI_3D_Structure_TopBottom,
	HDMI_3D_Structure_SideBySideHalf=8,
	HDMI_3D_Structure_Unknow
};

typedef struct
{
	UINT8 HDMI_3D_Enable;
	UINT8 HDMI_3D_Video_Format;
	UINT8 HDMI_3D_Structure;
	UINT8 HDMI_3D_EXTDATA;
}HDMI_3D_INFOFRAME;

typedef enum _RxHDCP_State_Type 
{
	RxHDCP_UnAuthenticated=0,
	RxHDCP_Computations,
	RxHDCP_WaitforDownstream,
	RxHDCP_AssembleKSVList,
	RxHDCP_WaitVReady,
	RxHDCP_Authenticated
} RxHDCPStateType;


#define RETNULL(cond)       if ((cond)){HDMIRX_LOG("return in %d\n",__LINE__);return;}

#define ACR_PACKET_HEADER             0x01
#define AUDIO_SAMPLE_PACKET_HEADER    0x02
#define GENERAL_CONTROL_PACKET_HEADER 0x03
#define ACP_PACKET_HEADER             0x04
#define ISRC1_PACKET_HEADER           0x05
#define ISRC2_PACKET_HEADER           0x06
#define DSD_PACKET_HEADER             0x07
#define DST_PACKET_HEADER             0x08
#define HBR_PACKET_HEADER             0x09
#define GAMUT_PACKET_HEADER           0x0A

#define VS_INFOFRAME_HEADER    0x81
#define AVI_INFOFRAME_HEADER   0x82
#define SPD_INFOFRAME_HEADER   0x83
#define AUDIO_INFOFRAME_HEADER 0x84
#define MPEG_INFOFRAME_HEADER  0x85

//VS  infoframe
#define VS_INFOFRAME_HB0  0
#define VS_INFOFRAME_HB1  1
#define VS_INFOFRAME_HB2  2
#define VS_INFOFRAME_PB0       3
#define VS_INFOFRAME_PB1       4
#define VS_INFOFRAME_PB2       5
#define VS_INFOFRAME_PB3       6
#define VS_INFOFRAME_PB4       7
  #define HDMI_VIDEO_FORMAT_MSK  (0x07<<5)
    #define NO_ADDITIONAL_HDMI_VIDEO_FORMAT_PRESENT 0x00
	#define EXTENDED_RESOLUTION_FORMAT_PRESENT      0x01//4kx2k
	#define _3D_FORMAT_PRESENT                       0x02
#define VS_INFOFRAME_PB5          8
  #define HDMI_VIC               (0xFF)
  #define _3D_STRUCTURE_MSK       (0xF<<4)

#define VS_INFOFRAME_PB6          9
  #define _3D_Ext_Data            (0xF<<4)

enum
{
	HDMI_InfoFrame_AVI,
	HDMI_InfoFrame_SPD,
	HDMI_InfoFrame_AUDIO,
	HDMI_InfoFrame_MPEG,
	HDMI_InfoFrame_UNREC,
	HDMI_InfoFrame_GAMUT,
	HDMI_InfoFrame_ACP
};

enum
{
	HDMI_STATE_NOTREADY,
	HDMI_STATE_INIT,
	HDMI_STATE_PWOFF,
	HDMI_STATE_PWON,
	HDMI_STATE_PWON2,
	HDMI_STATE_SCDT,
	HDMI_STATE_AUTH
};

typedef enum
{	
   HDMIRX_STATUS_OK = 0,
   HDMIRX_STATUS_ERROR,
} HDMIRX_STATUS;


extern UINT8 _bHDMIState;

BOOL fgHdmiRepeaterIsBypassMode(void);
BOOL fgHdmiRepeaterIsDramMode(void);
void vHdmiRepeaterMode(HDMI_RX_STATUS_MODE bHdmiMode);
void vHDMIInterRxInit(void) ;
UINT16 wHDMIVTotal(void);
UINT32 wHDMIHTotal(void);
UINT8 bHDMI422Input(void);
BOOL fgHDMIHsyncAct(void);
void RxHDCPSetReceiver(void);
void vTxSetRxReceiverMode(void);
void vTxSetRxRepeaterMode(void);
void vShowHDMIRxAudioChannelStatus(void);
void vShowRxDeepColorStatus(void);
void vShowAVMuteStatus(void);
void vShowAviInforFrame(void);
void vShowAudioInforFrame(void);
void vShowACPInforFrame(void);
void vShowSPDInforFrame(void);
void vShowGamutInforFrame(void);
void vShowVENDInforFrame(void);
void vShowGCPInforFrame(void);
void vHDMIRxHpdLoop(void);
void vHDMIVideoOutOn(void);
void vHDMIVideoOutOff(void);
void vHDMISetColorRalated(void);
UINT16 wHDMIResoWidth(void);
UINT16 wHDMIResoHeight(void);
void vHdmiRxPwrStatus(UINT32 *prData);
void vHdmiRxGetPacketData(void);
void vHdmiGet3DInfo(BOOL fgPrintLog);
void vEdidUpdataChk(void);
void vBdModeChk(void);
void vHDMIHPDLow(UINT8 u1HDMICurrSwitch);
void vGETHDMIRxEDID(HDMI_RX_EDID_T *prData);
void vUserSetHDMIRxEDIDToDrv(unsigned char *prData);
void vShowHDMIRxStatus(void);
UINT32 dwHDMILineFreq(void);
UINT8 bHDMIInputType(void);
void vHDMIRXColorSpaceConveter(void);
BOOL fgHDMICRC(INT16 ntry);
void vShowAllIntStatus(void);
void vShowRxHDCPBstatus(void);
void vModifyBstatusDepth(UINT8 u1Depth);
void vRecoverBstatusDepth(void);
void bHDMIEQTwoGear(UINT8);
UINT8 bHDMIITCFlag(void);
UINT8 bHDMIITCContent(void);
void vHDMISetEQRsel(UINT8);
void vHDMIPowerOnOff(UINT8);
void  vSetRxHdcpStatus(RxHDCPStateType bStatus);
void RxHDCPSetRepeater(void);
UINT8 bHDMIHDCPKeyCheck(void);
UINT8 bHDMIHDCPStatusGet(void);
BOOL fgDVICRC(INT16 ntry);
void vHDMIBypassVdo(void);
void vHDMIHDCPKey(void);
void vHDMIReloadHDCPKey(void);
void vHDMIHDCPSelfBist(UINT8 mode);
UINT8 bHDMIDeepColorStatus(void);
BOOL fgHDMIinterlaced(void);
void vCheckPwr5vStatus(void);
void vHDMIHPDHigh(UINT8 u1HDMICurrSwitch);
void vHdmiRxPacketDataInit(void);
UINT8 bHDMIScanInfo(void);
UINT8 bHDMIAspectRatio(void);
UINT8 bHDMIAFD(void);
UINT8 bHDMIRefreshRate(void);
extern void vHDMIHandleAudFmtChange(void);
void Linux_HAL_GetTime(unsigned long *prTime);
BOOL Linux_HAL_GetDeltaTime(unsigned long *u4OverTime, unsigned long *prStartT, unsigned long *prCurrentT );
void _HdmiRxPHYInit(void);
void MMSYS_BDP_POWER_ON(void);
void _GetTimingInfomationNoreset(void);
INT32 _HdmiRxCrcCheck(INT16 ntry);
void vLoadEdidFromCODE(void);
void vHDMIRxLoadHdcpKey(void);
void vHDMIRxHdcpService(void);
void RxHdcpMode(UINT8 u1Mode);
void vHdmiRxSetTxPacket(void);
void vShowHDMIRxInfo(void);
HDMIRX_STATUS HDMIRX_DIG_PowerOn(void);
HDMIRX_STATUS HDMIRX_DIG_PowerOff(void);


#endif

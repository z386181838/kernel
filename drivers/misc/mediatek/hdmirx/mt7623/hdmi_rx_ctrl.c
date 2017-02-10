#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/jiffies.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_spm_mtcmos.h>

#include "hdmi_rx_task.h"
#include "hdmi_rx_hal.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_rx_dvi.h"
#include "rx_io.h"
#include "edid_data.h"
#include "dgi_if.h"
#include "hdmiedid.h"
#include "hdmitable.h"
#include "hdmihdcp.h"

static UINT32 u4PreXpcCnt;
static UINT32 u4CurXpcCnt;
static UINT8 _bXpcStableCnt;
static UINT8 _u8SkipHpd = 0xff;
static UINT8 u1HdmiStateOld = 0;
UINT8   _bHDMIState;      // hdmi state
static UINT8 bInitHDCP = 0;
UINT8   _bHdmiMode; // 0 - DVI, 1 - HDMI
extern UINT8 _bDviModeChged;
extern UINT8 _u1RxSysState;

//TX info
BYTE	_TxDownStreamCount;
UINT16	_TxBStatus;
BYTE	_TxBKsv[5];
BYTE	_TxKsvList[TX_MAX_KSV_COUNT*5];	// 5 bytes * 9 Group = 35 bytes
BOOL    _fgTxHDCPAuthDone = FALSE;
BOOL    _fgTxVMatch = FALSE;
//RX info
CHAR RxHdcpKey[292];
RxHDCPStateType  _RxHDCPState;
BYTE 	_RxDownStreamCount;
BYTE	_RxKsvList[RX_MAX_KSV_COUNT*5];	//5 byte * 8 group = 40 bytes
UINT16 	_RxBstatus;
BYTE    _bHDCPMode = HDCP_RECEIVER;
UINT16  _wRxWaitTxKsvListCount = 0;
UINT8   _bHDMIRxHDCPStatus = 0;
BYTE _HdmiAvMuteShalow = FALSE;


BYTE _RxAKSVShadow[5];
BYTE _RxBKSVShadow[5];
BYTE _RxAnShadow[8];
UINT16 _u2RxRiShadow;
UINT16 _u2RxRiShadowOld;


UINT8 _bSCDTTMonitor = 0;
UINT8 TMDS_delay;

UINT32 g_u4HdmiState = HDMI_STATE_NOTREADY;

// Constant define
#define HDMI_AUD_OK               0
#define HDMI_AUD_NG               1
#define HDMI_AUD_UNSTABLE_LVL_1   1
#define HDMI_AUD_UNSTABLE_LVL_2   2
#define HDMI_AUD_UNSTABLE_LVL_3   3

static BOOL _fgAudMute = FALSE;
#define IS_AUD_MUTE()   _fgAudMute
UINT8   _bAudHdmiFlg;
UINT8   _bSmpFrq = AUD_FS_44K;
static UINT8 _bHdmiAudFreq;
BOOL _fgVideoOn = FALSE;
BOOL _fgAudOutMute = FALSE;

//RX_AUD_AIN_CFG_T _rAudCfg;
//extern HDMI_RX_IN_AUDIO_INFO_T _hdmi_rx_aud;
unsigned long _rHdmiPlugWaitTime;
unsigned long _rHdmiLowPlugWaitTime;
unsigned long _rHdmiUnplugTime;

HDMI_3D_INFOFRAME _3DInfo;
HDMI_3D_INFOFRAME _3DInfoOld;

unsigned char _u1TxEdidReadyOld = HDMI_PLUG_OUT;
extern UINT8 _u1TxEdidReady;

UINT32 _wHDMI_EQ_ZERO_VALUE;
UINT32 _wHDMI_EQ_BOOST_VALUE;
UINT32 _wHDMI_EQ_SEL_VALUE;
UINT32 _wHDMI_EQ_GAIN_VALUE;

UINT32 _wHDMI_HDCP_MASk1;
UINT32 _wHDMI_HDCP_MASk2;

// plug event
UINT8 _bPreHPDLowDelay;
UINT8 _bUnplugFlag;
UINT8 _bUnplugCount;
UINT8 _bForceHPDLow;
UINT8 _bHDMIColorSpace;
UINT8   _bEQFlag;
UINT8 _bSCDTdelay;
UINT8   _bHdmiMD;
UINT8 _bACPCount;
UINT8 _bAVIInfo_tmp;
UINT8   _bHDMIScanInfo;
UINT8 _bHDMIAspectRatio;
UINT8 _bHDMIAFD;
UINT8 _bHDMIHDCPStatus;
UINT8 _bHDMI422Input;
UINT8 _bHDMIITCFlag;
UINT8 _bHDMIITCContent;
UINT8 _bIntr_CK_CHG;
UINT8 _bNEW_AVI_Info;
UINT8 _bAVIInfo_tmp;
UINT8 _bHPD_Indep_Ctrl;	//  1 is 5v detect , 0 is CKDT detect
UINT32 _wHDMI_OFFON_MUTE_COUNT;
UINT32 _wDVI_WAIT_STABLE_COUNT;
UINT32 _wHDMIBypassFlag;
UINT32 _wDVI_WAIT_NOSIGNAL_COUNT;
UINT32 _wHDMI_WAIT_SCDT_STABLE_COUNT;
BOOL _fgUseModifiedDepth = FALSE;
UINT8 _u1ModifiedDepth=0;

#define HDMI_XPC_STABLE_CNT 30 // unit, per Vsync, xclk in pclk stable count
#define SAVE_WEAK_IC 0
#define ATC_720p_fix 1
#define DATA_CNT_MAX 2

UINT8   _bHdmiFlag;
UINT8   _bHdmiPwOnDelay;
UINT16   _bHPDdelay;
UINT16  _bCKDTcnt;
UINT8  _bHdmiPlug ;
UINT8  _bHdmiLowPlug ;
extern BYTE _bHdmiRepeaterMode ;
extern BYTE _bAppHdmiRepeaterMode;
UINT8   _bHDMICurrSwitch = HDMI_SWITCH_INIT;
UINT8   _bAppHDMICurrSwitch = HDMI_SWITCH_INIT;
BOOL    _fgBDPModeChgRes = FALSE;
UINT32 _u4DebugRxMessageType = 0;//HDMI_RX_DEBUG_EDID | HDMI_RX_DEBUG_HOT_PLUG;//| HDMI_RX_DEBUG_INFOFRAME;
UINT8 _bRxHDCPMode = 0;
BYTE _bHdmiRepeaterModeDelayCount = 15;
UINT8 _bHdmiAudioOutputMode = 0;//0: SPEAKER,SPEAKER+HDMI; 1 :HDMI

UINT8 _u1Force3DType = 0;

extern UINT8 u1HDMIIN1EDID[256];
extern UINT8 u1HDMIIN2EDID[256];
extern UINT8 u1HDMIINEDID[512];
extern UINT16 _u2EDID0PA;
extern UINT8 _u1EDID0PAOFF;
HDMI_RX_PACKET_INFO _RxPacket[MAX_PACKET];
HDMI_RX_EDID_T _Usersetedidtodrv;

const CHAR* cRxHdcpStatus[6] =
{
    "RxHDCP_UnAuthenticated",
    "RxHDCP_Computations",
    "RxHDCP_WaitforDownstream",
    "RxHDCP_AssembleKSVList",
    "RxHDCP_WaitVReady",
    "RxHDCP_Authenticated",
};

const CHAR* cHDMIState[7] =
{
    "HDMI_STATE_NOTREADY",
    "HDMI_STATE_INIT",
    "HDMI_STATE_PWOFF",
    "HDMI_STATE_PWON",
    "HDMI_STATE_PWON2",
    "HDMI_STATE_SCDT",
    "HDMI_STATE_AUTH",
};

const CHAR* cHDMIPacketName[10] =
{
    "AVI",
    "AUDIO",
    "ACP",
    "ISRC1",
    "ISRC2",
    "GAMUT",
    "VENDOR",
    "SPD",
    "MPEG",
    "GEN",
};

const CHAR* c3DStructure[10] =
{
	"HDMI_3D_Structure_FramePacking",
	"HDMI_3D_Structure_FieldAlternative",
	"HDMI_3D_Structure_LineAlternative",
	"HDMI_3D_Structure_SideBySideFull",
	"HDMI_3D_Structure_LDepth",
	"HDMI_3D_Structure_LDepthGraph",
	"HDMI_3D_Structure_TopBottom",
	"HDMI_3D_Structure_RSV_For_Future_use",
	"HDMI_3D_Structure_SideBySideHalf",
	"HDMI_3D_Structure_Unknow",
};

static CHAR * _aszHdmiState[] =
{
    "HDMI_STATE_NOTREADY",
    "HDMI_STATE_INIT",
    "HDMI_STATE_PWOFF",
    "HDMI_STATE_PWON",
    "HDMI_STATE_PWON2",
    "HDMI_STATE_SCDT",
    "HDMI_STATE_AUTH"
};

UINT8 pdInternalRxHdcpKey[292] =
{
    0x00 ,0x14 ,0x7c ,0xE6 ,0x2C ,0x37, 0xFF ,0xFF ,//0X00,KSV,0Xff,0xff
    0x00 ,0x18 ,0x8b ,0x78 ,0x94, //0x00,HDCP KEY
    0xc8 ,0xef ,0xb3 ,0x00 ,0xd5 ,0x89 ,0xdb ,0x9f ,0xf7 ,0xb8 ,0x51 ,0x7d ,0x41 ,0x07 ,0x0a ,0x5f,
    0x92 ,0xf1 ,0xc3 ,0x46 ,0x20 ,0x92 ,0x7c ,0x2c ,0x2a ,0x4f ,0xd5 ,0xbd ,0x86 ,0x74 ,0xe5 ,0x29,
    0xca ,0x74 ,0xf0 ,0xc5 ,0x9c ,0x68 ,0x42 ,0x0f ,0x8e ,0x7f ,0x89 ,0x4b ,0xa1 ,0xa5 ,0xb1 ,0xe1,
    0xb6 ,0x03 ,0x1e ,0x34 ,0x83 ,0x1a ,0xe8 ,0x51 ,0x0d ,0xbb ,0xe2 ,0x46 ,0x0e ,0x05 ,0x38 ,0xad,
    0x1d ,0xb1 ,0x99 ,0xee ,0x36 ,0x5a ,0x7d ,0x35 ,0xd7 ,0xa4 ,0xc9 ,0xdd ,0xb0 ,0x83 ,0xec ,0x0d,
    0xe1 ,0x45 ,0x62 ,0xed ,0xa4 ,0x16 ,0xea ,0x08 ,0x5b ,0x5b ,0x02 ,0x32 ,0xe2 ,0x4c ,0xfd ,0x73,
    0x76 ,0x68 ,0x4f ,0x9a ,0xb0 ,0x9d ,0x94 ,0x81 ,0xa8 ,0xb8 ,0x67 ,0x00 ,0x24 ,0x0e ,0x79 ,0xb2,
    0x3b ,0x7d ,0x2d ,0x36 ,0x25 ,0x13 ,0x3d ,0x89 ,0x57 ,0xec ,0xef ,0xf0 ,0x53 ,0xa8 ,0x6b ,0xf6,
    0xc8 ,0x47 ,0xc0 ,0xf6 ,0xc6 ,0xa9 ,0x91 ,0x61 ,0xc6 ,0x6a ,0xd0 ,0x4e ,0x88 ,0x01 ,0xcb ,0x18,
    0x5a ,0x7d ,0x3e ,0xcb ,0x66 ,0x3b ,0xd5 ,0x99 ,0xa8 ,0x95 ,0xe7 ,0x4e ,0x5d ,0x8c ,0xc3 ,0x3a,
    0x9a ,0xc3 ,0x8c ,0x72 ,0x59 ,0xa3 ,0xd1 ,0xf1 ,0x02 ,0xd9 ,0x44 ,0x37 ,0x30 ,0x73 ,0x2b ,0x73,
    0x97 ,0x99 ,0x9e ,0xc6 ,0x13 ,0xbc ,0x6d ,0x3a ,0x53 ,0x3c ,0xd1 ,0x81 ,0x45 ,0xfb ,0x00 ,0x2b,
    0x0f ,0x7e ,0xb6 ,0xf5 ,0xc1 ,0x50 ,0x33 ,0xff ,0x4a ,0x04 ,0x22 ,0x0f ,0x53 ,0xa3 ,0x78 ,0xef,
    0xde ,0x93 ,0x2b ,0x54 ,0xc6 ,0xfc ,0xb4 ,0x4b ,0xf0 ,0x3a ,0x35 ,0x83 ,0x9d ,0x69 ,0x29 ,0xcd,
    0x99 ,0x12 ,0x36 ,0xfa ,0xa6 ,0x44 ,0xd9 ,0x92 ,0xd3 ,0x4f ,0x36 ,0xb5 ,0x52 ,0xfa ,0x27 ,0xac,
    0x3a ,0x2f ,0x1e ,0xb8 ,0xe8 ,0x29 ,0x9a ,0x90 ,0x54 ,0x8d ,0xd3 ,0x9e ,0x9e ,0xb4 ,0xa4 ,0x13,
    0x92 ,0x26 ,0x34 ,0xff ,0x97 ,0x07 ,0xe1 ,0xed ,0x6d ,0x3e ,0x60 ,0x53 ,0x94 ,0xc6 ,0x62 ,0x8d,
    0x66 ,0x5e ,0x13 ,0xaa ,0x00 ,0x00, 0x00//HDCP KEY 0x00,0x00,0x00
};

static const char* cHdmiAudFsStr[7] =
{
  "HDMI_FS_32K",
  "HDMI_FS_44K",
  "HDMI_FS_48K",
  "HDMI_FS_88K",
  "HDMI_FS_96K",
  "HDMI_FS_176K",
  "HDMI_FS_192K"

};

static const char* cAudCodingTypeStr[16] =
{
  "Refer to Stream Header",
  "PCM",
  "AC3",
  "MPEG1",
  "MP3",
  "MPEG2",
  "AAC",
  "DTS",
  "ATRAC",
  "ONE Bit Audio",
  "Dolby Digital+",
  "DTS-HD",
  "MAT(MLP)",
  "DST",
  "WMA Pro",
  "Reserved",
};

static const char* cAudChCountStr[8] =
{
  "Refer to Stream Header",
  "2ch",
  "3ch",
  "4ch",
  "5ch",
  "6ch",
  "7ch",
  "8ch",

};

static const char* cAudFsStr[8] =
{
  "Refer to Stream Header",
  "32 khz",
  "44.1 khz",
  "48 khz",
  "88.2 khz",
  "96 khz",
  "176.4 khz",
  "192 khz"

};


static const char* cAudChMapStr[32] =
{
  "FR,FL",
  "LFE,FR,FL",
  "FC,FR,FL",
  "FC,LFE,FR,FL",
  "RC,FR,FL",
  "RC,LFE,FR,FL",
  "RC,FC,FR,FL",
  "RC,FC,LFE,FR,FL",
  "RR,RL,FR,FL",
  "RR,RL,LFE,FR,FL",
  "RR,RL,FC,FR,FL",
  "RR,RL,FC,LFE,FR,FL",
  "RC,RR,RL,FR,FL",
  "RC,RR,RL,LFE,FR,FL",
  "RC,RR,RL,FC,FR,FL",
  "RC,RR,RL,FC,LFE,FR,FL",
  "RRC,RLC,RR,RL,FR,FL",
  "RRC,RLC,RR,RL,LFE,FR,FL",
  "RRC,RLC,RR,RL,FC,FR,FL",
  "RRC,RLC,RR,RL,FC,LFE,FR,FL",
  "FRC,FLC,FR,FL",
  "FRC,FLC,LFE,FR,FL",
  "FRC,FLC,FC,FR,FL",
  "FRC,FLC,FC,LFE,FR,FL",
  "FRC,FLC,RC,FR,FL",
  "FRC,FLC,RC,LFE,FR,FL",
  "FRC,FLC,RC,FC,FR,FL",
  "FRC,FLC,RC,FC,LFE,FR,FL",
  "FRC,FLC,RR,RL,FR,FL",
  "FRC,FLC,RR,RL,LFE,FR,FL",
  "FRC,FLC,RR,RL,FC,FR,FL",
  "FRC,FLC,RR,RL,FC,LFE,FR,FL",
};

static const char* cAudDMINHStr[2] =
{
 "Permiited down mixed stereo or no information",
 "Prohibited down mixed stereo"
};

static const char* cAudSampleSizeStr[4] =
{
  "Refer to Stream Header",
  "16 bit",
  "20 bit",
  "24 bit"
};


static const char* cAviRgbYcbcrStr[4] =
{
  "RGB",
  "YCbCr 4:2:2",
  "YCbCr 4:4:4",
  "Future"
};

static const char* cAviActivePresentStr[2] =
{
  "No data",
  "Actuve Format(R0..R3) Valid",

};

static const char* cAviBarStr[4] =
{
  "Bar data not valid",
  "Vert. Bar info valid",
  "Horiz. Bar info valid",
  "Vert. and Horiz Bar info valid",
};
static const char* cAviScanStr[4] =
{
  "No data",
  "Overscanned display",
  "underscanned display",
  "Future",
};

static const char* cAviColorimetryStr[4] =
{
  "no data",
  "ITU601",
  "ITU709",
  "Extended Colorimetry infor valid",
};

static const char* cAviAspectStr[4] =
{
  "No data",
  "4:3",
  "16:9",
  "Future",
};


static const char* cAviActiveStr[16] =
{
  "reserved",
  "reserved",
  "box 16:9(top)",
  "box 14:9(top)",
  "box > 16:9(center)",
  "reserved",
  "reserved",
  "reserved",
  "Same as picture aspect ratio",
  "4:3(Center)",
  "16:9(Center)",
  "14:9(Center)",
  "reserved",
  "4:3(with shoot & protect 14:9 center)",
  "16:9(with shoot & protect 14:9 center)",
  "16:3(with shoot & protect 4:3 center)"
};

static const char* cAviItContentStr[2] =
{
 "no data",
 "IT Content"

};

static const char* cAviExtColorimetryStr[2] =
{
 "xvYCC601",
 "xvYCC709",
};

static const char* cAviRGBRangeStr[4] =
{
 "depends on video format",
 "Limit range",
 "FULL range",
 "Reserved",
};

static const char* cAviScaleStr[4] =
{
 "Unkown non-uniform scaling",
 "Picture has been scaled horizontally",
 "Picture has been scaled vertically",
 "Picture has been scaled horizontally and vertically",
};



static const char* cSPDDeviceStr[16] =
{
 "unknown",
 "Digital STB",
 "DVD Player",
 "D-VHS",
 "HDD Videorecorder",
 "DVC",
 "DSC",
 "Video CD",
 "Game",
 "PC General",
 "Blu-Ray Disc",
 "Super Audio CD",
 "reserved",
 "reserved",
 "reserved",
 "reserved",
};  

extern UINT32 _u4CKPDRDOLD;
extern BYTE bCheckPordHotPlug(BYTE bMode);
static UINT8 bHDMIPort5VStatus(UINT8 u1Switch);
static void vHDMIMuteAudio(void);
static void vHDMIUnMuteAudio(void);
static void vXpcStableCount(void);
static UINT32 wHDMIXPCCNT(void);
static void vHDMIHandleAudFifoFault(void);
static UINT32 dwHDMIPixelFreq(void);
static void vHDMIVideoHdmiSetting(void);
static void vHDMIHDCPRst(void);
static void vLogHdmiStateChange(UINT8 u1HdmiState);
void vHDMITMDSCTRL(UINT8 bOnOff);
UINT8 u1ConvertChLayoutToChNumIndex(UINT32 u4ChLayout);
UINT8 u1ConvertFsToFsStrIndex(UINT32 u4Fs);

void Linux_HAL_GetTime(unsigned long *prTime)
{
    *prTime = jiffies;
}

BOOL Linux_HAL_GetDeltaTime(unsigned long *u4OverTime, unsigned long *prStartT, unsigned long *prCurrentT )
{
    unsigned long u4DeltaTime;

    u4DeltaTime = *prStartT+ (*u4OverTime)*HZ/1000;
    if(time_after(*prCurrentT, u4DeltaTime)) {
        return 1;
    }
    return 0;
}

void vHDMIRxHpdLoop(void)
{
	UINT32 dReadData;
	static UINT8 dReadData1=0xFF,dReadHDCPChangFlag=0;

	unsigned long rDelta;
	unsigned long  rTimeTmp;

	vCheckPwr5vStatus();

	if (bInitHDCP == 0)
	{
		bInitHDCP++;
		return;
	}
	else if (bInitHDCP == 1)
	{
		bInitHDCP++;
		vHDMIHDCPRst();
        printk(" **************only internel hdcp test******************* \n");
        vHDMIRxLoadHdcpKey();
		return;
	}

	vHalWriteVideoChMap(RX_CH_MAP_RGB);//RGB
	vBdModeChk();
	vEdidUpdataChk();
	//read state
	//	bit0 SCDT
	//	bit1 CKDT
	//	bit2 VSYNC
	//	bit3 PWR5V

	//-----------------------------------5V detect ----------------------------------------
	if (((_bHDMICurrSwitch==HDMI_SWITCH_1) && !(bHDMIPort5VStatus(HDMI_SWITCH_1))) ||
		(_bHDMICurrSwitch==HDMI_SWITCH_INIT) )
	{

		if (_bHDMIState == HDMI_STATE_PWOFF)
		{
			if (_u8SkipHpd == 0)
			{
				Linux_HAL_GetTime(&rTimeTmp);
				rDelta = 1000; //1000ms
				if(Linux_HAL_GetDeltaTime(&rDelta, &_rHdmiUnplugTime, &rTimeTmp))
					_u8SkipHpd = 1;


			}
			return;
		}
		else
		{
			printk( "[HDMI RX]HDMI_STATE_INIT #1\n");
			_bHDMIState = HDMI_STATE_INIT;
			_bUnplugFlag=1;

			vHDMIHPDHigh(_bHDMICurrSwitch);
			if(fgIsHdmiRepeater())
			{
				hdmi_tmdsonoff(0);
			}
			
			//vHDMIRxUnPlugNotifyAud();
			printk("[HDMI RX] vHDMIRxUnPlugNotifyAud.\n") ;
		}

	}
	else
	{
		if (_bHDMIState == HDMI_STATE_PWOFF)
		{
			printk( "[HDMI RX]HDMI_STATE_PWON\n");
			printk( "[HDMI RX]5V Detected \n");
			_bHDMIState= HDMI_STATE_PWON;
		}
	}
	if(u1HalChkCKDTExist())
	{
		{
		   vHalSelANABand();
		}
	}
	else
	{
		   _u4CKPDRDOLD = 0;
	}

	/*---------CKDT monitor----------------*/
	if (!u1HalChkDataEnableExist())
	{
		_bSCDTTMonitor++;
		
		if (_bSCDTTMonitor >= 3)
		{
			_bSCDTTMonitor = 3;
		}
	}
	else
	{
		if (_bSCDTTMonitor == 3)
		{
            bHDMIPHYReset(HDMI_RST_ALL);
			vHalResetHdmiRxTotalModule();
			printk("HDMI Analog SCDT ........\n");
		}
		_bSCDTTMonitor = 0;
	}
	//--------------------------------------------------------------------------

	vXpcStableCount();
	vLogHdmiStateChange(_bHDMIState);

	switch (_bHDMIState)
	{
	case HDMI_STATE_INIT:
		vHalResetHdmiRxTotalModule();
		vHDMIVideoOutOff();
		_bHdmiPwOnDelay = 0;
		_bHdmiPlug = 0;
		_bHdmiLowPlug =0;
		dReadHDCPChangFlag=0;
		_bPreHPDLowDelay=0;
		vHDMIMuteAudio();

		_u8SkipHpd = 0;
		Linux_HAL_GetTime(&_rHdmiUnplugTime);
		_bHDMIState = HDMI_STATE_PWOFF;
		printk( "[HDMI RX]HDMI_STATE_PWOFF\n");
		break;

	case HDMI_STATE_PWOFF:
		break;

	case HDMI_STATE_PWON:
		if(_bPreHPDLowDelay++>5)
		{
			unsigned long rCurHPDLowTime;

			Linux_HAL_GetTime(&rCurHPDLowTime);

			if (!_bHdmiLowPlug)
			{

				_bHdmiLowPlug = 1;
				Linux_HAL_GetTime(&_rHdmiLowPlugWaitTime);

			}

			rDelta = 100;//330 ms

			if(Linux_HAL_GetDeltaTime(&rDelta, &_rHdmiLowPlugWaitTime, &rCurHPDLowTime))
			{
				{
					vHDMITMDSCTRL(TRUE);
				}
					printk( "[HDMI RX]RSEN = TURE\n");
			}

			rDelta = _wHDMI_OFFON_MUTE_COUNT;//800 ms

			if((Linux_HAL_GetDeltaTime(&rDelta, &_rHdmiLowPlugWaitTime, &rCurHPDLowTime)== TRUE))
			{
				vHalEnableHDCPDDCPort();// enable DDCDLY DDC_EN RX_EN
				printk( "[HDMI RX]Enable DDC Channel \n");
				vHDMITMDSCTRL(TRUE);
				_bHdmiPwOnDelay = 0;
				_bHPDdelay=0;
				_bHDMIState = HDMI_STATE_PWON2;
			}

		}
		printk( "[HDMI RX]HDMI_STATE_PWON\n");
		vHDMIMuteAudio();
		break;

	case HDMI_STATE_PWON2:

		vHDMIHPDHigh(_bHDMICurrSwitch);

		_bEQFlag=0;

		_bHDMIState = HDMI_STATE_SCDT;

			printk( "[HDMI RX]HDMI_STATE_PWON2\n");
			printk( "[HDMI RX]HPD = HIGH \n");
		break;

	case HDMI_STATE_SCDT:
		if (u1HalChkDataEnableExist())
		{
			unsigned long rCurTime;

			Linux_HAL_GetTime(&rCurTime);

			vHalClearRxPclkChgStatus();//Clear Pixel clock change interrupt status bit
			if (!_bHdmiPlug)
			{

				_bHdmiPlug = 1;
				Linux_HAL_GetTime(&_rHdmiPlugWaitTime);
			}


			dReadData = vHalGetRxHdcpStatus(); //00: No HDCP; 01: Authenticating; 11: AUTH Done

			if(dReadData1!=dReadData)
			{
				dReadData1=dReadData;
				Linux_HAL_GetTime(&_rHdmiPlugWaitTime);

			}

			if(fgHalCheckHdmiRXAuthDone(dReadData))
			{
				dReadHDCPChangFlag++;
			}

			rDelta = _wHDMI_WAIT_SCDT_STABLE_COUNT;
			if((Linux_HAL_GetDeltaTime(&rDelta, &_rHdmiPlugWaitTime, &rCurTime)== TRUE)|| (dReadHDCPChangFlag==30))
			{
				printk( "[HDMI RX]HDMI_STATE_AUTH\n");
				_bSCDTdelay=0;
				vHdmiRxPacketDataInit();
				_bHDMIState = HDMI_STATE_AUTH;

				//when 3x3 matrix is RGB but player output is Ycbcr, AVI infofrmae has changed	to Ycbcr but FW doesn't change
				if(fgHalCheckAviInforFrameExist())
				{
					_bAVIInfo_tmp= u1HalReadAviByte1();
					_bNEW_AVI_Info=1;
				}
			}
		}
		break;

	case HDMI_STATE_AUTH:
		// solve HD2600 VGA Card
		if(vHalCheckIsPclkChanged())//Pixel clock change
		{
			msleep(2);//vUtDelay2us(1);
		}
		// solve HD2600 VGA Card
		TMDS_delay=0;

		if (!(u1HalChkDataEnableExist()))
		{
			if(_bSCDTdelay++>30)
			{
				printk( "[HDMI RX]SCDT =%x\n",vHalCheckIsPclkChanged());
				printk( "[HDMI RX]HDMI_STATE_SCDT #2\n");
				dReadHDCPChangFlag=0;
				vHdmiRxPacketDataInit();
				vHalHDMIRxEnableVsyncInt(FALSE);
				_bHDMIState = HDMI_STATE_SCDT;
			}
			break;
		}
		else
		{
			_bSCDTdelay=0;
		}
		_bEQFlag=0;

		vHDMIVideoOutOn();
		// Check HDMI Mode
		_bHdmiMode = vHalCheckRxIsHdmiMode();
		if((!_bHdmiMode) && vHalCheckGcpMuteEnable())//DVI mode but receive GCP MUTE
		{
			vHalSwResetHdmiRxModule();//SW reset
		}
		vHDMISetColorRalated();
		vHdmiRxGetPacketData();
		break;

	default:
		_bHDMIState = HDMI_STATE_INIT;
		break;
	}//switch

}

BOOL fgHDMICRC(INT16 ntry)
{
    BOOL fgResult= 0;

    fgResult = vHalHdmiRxCrc(ntry);

    return  fgResult;
}


void vShowAllIntStatus(void)
{
    UINT32 u4Data;

    u4Data = u4HalReadINTR_STATE0();

    printk("[HDMI RX]INTR_STATE0 = 0x%x\n", u4Data);

    u4Data = u4HalReadINTR_STATE1();

    printk("[HDMI RX]INTR_STATE1 = 0x%x\n", u4Data);

    if(fgHalIsINTR3_CEA_NEW_CP())
        printk("[HDMI RX]INT INTR3_CEA_NEW_CP\n");

    if(fgHalIsINTR3_CP_SET_MUTE())
        printk("[HDMI RX]INT INTR3_CP_SET_MUTE\n");

    if(fgHalIsINTR3_P_ERR())
        printk("[HDMI RX]INT INTR3_P_ERR\n");

    if(fgHalIsINTR3_NEW_UNREC())
        printk("[HDMI RX]INT INTR3_NEW_UNREC\n");

    if(fgHalIsINTR3_NEW_MPEG())
        printk("[HDMI RX]INT INTR3_NEW_MPEG\n");

    if(fgHalIsINTR3_NEW_AUD())
        printk("[HDMI RX]INT INTR3_NEW_AUD\n");

    if(fgHalIsINTR3_NEW_SPD())
        printk("[HDMI RX]INT INTR3_NEW_SPD\n");

    if(fgHalIsINTR3_NEW_AVI())
        printk("[HDMI RX]INT INTR3_NEW_AVI\n");

    if(fgHalIsINTR2_HDMI_MODE())
        printk("[HDMI RX]INT INTR2_HDMI_MODE\n");

    if(fgHalIsINTR2_VSYNC())
        printk("[HDMI RX]INT INTR2_VSYNC\n");


    if(fgHalIsINTR2_SOFT_INTR_EN())
        printk("[HDMI RX]INT INTR2_SOFT_INTR_EN\n");

    if(fgHalIsINTR2_CKDT())
        printk("[HDMI RX]INT INTR2_CKDT\n");

    if(fgHalIsINTR2_SCDT())
        printk("[HDMI RX]INT INTR2_SCDT\n");
    if(fgHalIsINTR2_GOT_CTS())
        printk("[HDMI RX]INT INTR2_GOT_CTS\n");
    if(fgHalIsINTR2_NEW_AUD_PKT())
        printk("[HDMI RX]INT INTR2_NEW_AUD_PKT\n");

    if(fgHalIsINTR2_CLK_CHG())
        printk("[HDMI RX]INT INTR2_CLK_CHG\n");

    if(fgHalIsINTR1_HW_CTS_CHG())
        printk("[HDMI RX]INT INTR1_HW_CTS_CHG\n");

    if(fgHalIsINTR1_HW_N_CHG())
        printk("[HDMI RX]INT INTR1_HW_N_CHG\n");

    if(fgHalIsINTR1_FIFO_ERR())
        printk("[HDMI RX]INT INTR1_FIFO_ERR\n");

    if(fgHalHdcpAuthenticationStart())
        printk("[HDMI RX]INT INTR1_AUTH_START\n");

    if(fgHalHdcpAuthenticationDone())
        printk("[HDMI RX]INT INTR1_AUTH_DONE\n");
    if(fgHalIsSOFT_INTR_EN())
        printk("[HDMI RX]INT SOFT_INTR_EN\n");

    if(fgHalIsINTR_OD())
        printk("[HDMI RX]INT INTR_OD\n");

    if(fgHalIsINTR_POLARITY())
        printk("[HDMI RX]INT INTR_POLARITY\n");

    if(fgHalIsINTR_STATE())
        printk("[HDMI RX]INT INTR_STATE\n");

    if(fgHalIsINTR7_RATIO_ERROR())
        printk("[HDMI RX]INT INTR7_RATIO_ERROR \n");

    if(fgHalIsINTR7_AUD_CH_STAT())
        printk("[HDMI RX]INT INTR7_AUD_CH_STAT \n");

    if(fgHalIsINTR7_GCP_CD_CHG())
        printk("[HDMI RX]INT INTR7_GCP_CD_CHG \n");

    if(fgHalIsINTR7_GAMUT())
        printk("[HDMI RX]INT INTR7_GAMUT \n");

    if(fgHalIsINTR7_HBR())
        printk("[HDMI RX]INT INTR7_HBR \n");

    if(fgHalIsINTR7_SACD())
        printk("[HDMI RX]INT INTR7_SACD \n");

    if(fgHalIsINTR6_PRE_UNDERUN())
        printk("[HDMI RX]INT INTR6_PRE_UNDERUN \n");

    if(fgHalIsINTR6_PRE_OVERUN())
        printk("[HDMI RX]INT INTR6_PRE_OVERUN \n");

    if(fgHalIsINTR6_PWR5V_RX2())
        printk("[HDMI RX]INT INTR6_PWR5V_RX2 \n");

    if(fgHalIsINTR6_PWR5V_RX1())
        printk("[HDMI RX]INT INTR6_PWR5V_RX1 \n");

    if(fgHalIsINTR6_NEW_ACP())
        printk("[HDMI RX]INT INTR6_NEW_ACP \n");

    if(fgHalIsINTR6_P_ERR2())
        printk("[HDMI RX]INT INTR6_P_ERR2 \n");

    if(fgHalIsINTR6_PWR5V_RX0())
        printk("[HDMI RX]INT INTR6_PWR5V_RX0 \n");

    if(fgHalIsINTR5_FN_CHG())
        printk("[HDMI RX]INT INTR5_FN_CHG \n");

    if(fgHalIsINTR5_AUDIO_MUTE())
        printk("[HDMI RX]INT INTR5_AUDIO_MUTE \n");

    if(fgHalIsINTR5_BCH_AUDIO_ALERT())
        printk("[HDMI RX]INT INTR5_BCH_AUDIO_ALERT \n");

    if(fgHalIsINTR5_VRESCHG())
        printk("[HDMI RX]INT INTR5_VRESCHG \n");

    if(fgHalIsINTR5_HRESCHG())
        printk("[HDMI RX]INT INTR5_HRESCHG \n");

    if(fgHalIsINTR5_POLCHG())
        printk("[HDMI RX]INT INTR5_POLCHG \n");

    if(fgHalIsINTR5_INTERLACEOUT())
        printk("[HDMI RX]INT INTR5_INTERLACEOUT \n");

    if(fgHalIsINTR5_AUD_SAMPLE_F())
        printk("[HDMI RX]INT INTR5_AUD_SAMPLE_F \n");

    if(fgHalIsINTR4_PKT_RECEIVED_ALERT())
        printk("[HDMI RX]INT INTR4_PKT_RECEIVED_ALERT \n");

    if(fgHalIsINTR4_HDCP_PKT_ERR_ALERT())
        printk("[HDMI RX]INT INTR4_HDCP_PKT_ERR_ALERT \n");

    if(fgHalIsINTR4_T4_PKT_ERR_ALERT())
        printk("[HDMI RX]INT INTR4_T4_PKT_ERR_ALERT \n");

    if(fgHalIsINTR4_NO_AVI())
        printk("[HDMI RX]INT INTR4_NO_AVI \n");

    if(fgHalIsINTR4_CTS_DROPPED_ERR())
        printk("[HDMI RX]INT INTR4_CTS_DROPPED_ERR \n");

    if(fgHalIsINTR4_CTS_REUSED_ERR())
        printk("[HDMI RX]INT INTR4_CTS_REUSED_ERR \n");

    if(fgHalIsINTR4_OVERRUN())
        printk("[HDMI RX]INT INTR4_OVERRUN \n");

    if(fgHalIsINTR4_UNDERRUN())
        printk("[HDMI RX]INT INTR4_UNDERRUN \n");
}
HDMIRX_STATUS HDMIRX_DIG_PowerOn(void)
{
//digtal part power on 
    int ret = 0; 
	spm_mtcmos_ctrl_bdpsys(STA_POWER_ON);
	ret += enable_clock(MT_CG_AUDIO_HDMIRX, "hdmirx");
	ret += enable_clock(MT_MUX_HDMIRX_BIST, "hdmirx");
	ret += enable_clock(MT_MUX_HDMIRX_26M_24M, "hdmirx");
#if 0
	ret += enable_clock(MT_CG_RX_FCLK, "hdmirx");
	ret += enable_clock(MT_CG_RX_XCLK, "hdmirx");
	ret += enable_clock(MT_CG_RXPDTCLK, "hdmirx");
	ret += enable_clock(MT_CG_RX_CSCL_N, "hdmirx");
	ret += enable_clock(MT_CG_RX_CSCL, "hdmirx");
	ret += enable_clock(MT_CG_RX_DDCSCL_N, "hdmirx");
	ret += enable_clock(MT_CG_RX_DDCSCL, "hdmirx");
	ret += enable_clock(MT_CG_RX_VCOCLK, "hdmirx");
	ret += enable_clock(MT_CG_RX_DPCLK, "hdmirx");
	ret += enable_clock(MT_CG_RX_PCLK, "hdmirx");
	ret += enable_clock(MT_CG_RX_MCLK, "hdmirx");
	ret += enable_clock(MT_CG_RX_PLLCLK, "hdmirx");
#endif    
    return HDMIRX_STATUS_OK;
}

HDMIRX_STATUS HDMIRX_DIG_PowerOff(void)
{
// digital part power off
    int ret = 0; 
	
	spm_mtcmos_ctrl_bdpsys(STA_POWER_DOWN);
	ret += disable_clock(MT_CG_AUDIO_HDMIRX, "hdmirx");
	ret += disable_clock(MT_MUX_HDMIRX_BIST, "hdmirx");
	ret += disable_clock(MT_MUX_HDMIRX_26M_24M, "hdmirx");
	ret += disable_clock(MT_CG_RX_FCLK, "hdmirx");
	ret += disable_clock(MT_CG_RX_XCLK, "hdmirx");
	ret += disable_clock(MT_CG_RXPDTCLK, "hdmirx");
	ret += disable_clock(MT_CG_RX_CSCL_N, "hdmirx");
	ret += disable_clock(MT_CG_RX_CSCL, "hdmirx");
	ret += disable_clock(MT_CG_RX_DDCSCL_N, "hdmirx");
	ret += disable_clock(MT_CG_RX_DDCSCL, "hdmirx");
	ret += disable_clock(MT_CG_RX_VCOCLK, "hdmirx");
	ret += disable_clock(MT_CG_RX_DPCLK, "hdmirx");
	ret += disable_clock(MT_CG_RX_PCLK, "hdmirx");
	ret += disable_clock(MT_CG_RX_MCLK, "hdmirx");
	ret += disable_clock(MT_CG_RX_PLLCLK, "hdmirx");
    
    return HDMIRX_STATUS_OK;
}

void vHDMIInterRxInit(void)
{
   
    HDMIRX_DIG_PowerOn();
	_bHDMIState = HDMI_STATE_PWOFF;
	
    // 2. Software Initial
    _bHPD_Indep_Ctrl=0;
    _bForceHPDLow=1;	//when TV power on, no resistance between GPIO and power5v, HPD always High, Pioneer 969 neglect termination disapper and

    _bHdmiFlag = 0; //for audio info 
    _bHdmiMode = 0; //hdmi mode is hdmi or dvi
    _bHdmiMD=0;     //same as _bHdmiMode
    _bHDMIScanInfo=bHDMIScanInfo();
    _bHDMIAspectRatio=bHDMIAspectRatio();
    _bHDMIAFD=bHDMIAFD();
    _bHDMIHDCPStatus= bHDMIHDCPStatusGet();
    _bHDMI422Input=0;
    _bHDMIITCFlag=0;  //AVI info
    _bHDMIITCContent=0;
    _bIntr_CK_CHG =0;
    _bNEW_AVI_Info=0;
    _bACPCount=0;
    _bUnplugFlag=0;
    _bUnplugCount=0;
    _bHDMIColorSpace=0;

    u4PreXpcCnt = 0;
    u4CurXpcCnt = 0;
    _bXpcStableCnt = 0;

    _bHdmiAudFreq = AUD_FS_44K;  // 44.1 k

    _bHDMICurrSwitch = (UINT8) HDMI_SWITCH_INIT;
    _bAppHDMICurrSwitch = (UINT8) HDMI_SWITCH_INIT;
    if (_bHDMIState == HDMI_STATE_NOTREADY)
    {
        return;
    }

    _wHDMI_EQ_ZERO_VALUE = HDMI_TMDS_EQ_ZERO_VALUE;
    _wHDMI_EQ_BOOST_VALUE = HDMI_TMDS_EQ_BOOST_VALUE;
    _wHDMI_EQ_SEL_VALUE = HDMI_TMDS_EQ_SEL_VALUE;
    _wHDMI_EQ_GAIN_VALUE = HDMI_TMDS_EQ_GAIN_VALUE;
    _wHDMI_HDCP_MASk1 = HDMI_HDCP_Mask1;
    _wHDMI_HDCP_MASk2 = HDMI_HDCP_Mask2;
    _wHDMI_OFFON_MUTE_COUNT = HDMI_OFFON_MUTE_COUNT;
    _wDVI_WAIT_STABLE_COUNT = DVI_WAIT_STABLE_COUNT;
    _wHDMIBypassFlag = HDMI_BYPASS_INITIAL_FLOW;
    _wDVI_WAIT_NOSIGNAL_COUNT = DVI_WAIT_NOSIGNAL_COUNT;
    _wHDMI_WAIT_SCDT_STABLE_COUNT = HDMI_WAIT_SCDT_STABLE_COUNT;

    //THe following Initial Variable Function need to set before vHalRxHwInit()
    vHalSetEqZeroValueVar(HDMI_TMDS_EQ_ZERO_VALUE);
    vHalSetEqBoostValueVar(HDMI_TMDS_EQ_BOOST_VALUE);
    vHalSetEqSelValueVar(HDMI_TMDS_EQ_SEL_VALUE);
    vHalSetEqGainValueVar(HDMI_TMDS_EQ_GAIN_VALUE);
    vHalSetRxHdcpMask1Var(_wHDMI_HDCP_MASk1);
    vHalSetRxHdcpMask2Var(_wHDMI_HDCP_MASk2);

      _bCKDTcnt=0;

	//THe following is Hardware init
	vHalRxHwInit();
	vHdmiRxPacketDataInit();
}

void vHDMIVideoOutOff(void)
{
    if (_bHDMIState == HDMI_STATE_NOTREADY)
    {
        return;
    }
    _fgVideoOn = FALSE;
}

void vHDMIVideoOutOn(void)
{

    if (_bHDMIState == HDMI_STATE_NOTREADY)
    {
        return;
    }

    _fgVideoOn = TRUE;
}


void vHDMIHPDLow(UINT8 u1HDMICurrSwitch)
{
    printk("[enter %s]u1HDMICurrSwitch = %d\n",__FUNCTION__,u1HDMICurrSwitch);
	vHalSetRxHDMIHPDLow(u1HDMICurrSwitch);
}

void vHDMIHPDHigh(UINT8 u1HDMICurrSwitch)
{ 
    printk("[enter %s]u1HDMICurrSwitch = %d\n",__FUNCTION__,u1HDMICurrSwitch);
    vHalSetRxHDMIHPDHigh(u1HDMICurrSwitch);
    return ;
}

void vHDMIAudioOutOn(void)
{
    if (_bHDMIState == HDMI_STATE_NOTREADY)
    {
        return;
    }
    vHalOpenAPLL();
    vSetHdmiFlg(HDMI_AUDIO_ON);
}

UINT8 bHDMIDeepColorStatus(void)
{
    return u4HalCheckDeepColorMode();
}

void vHDMISetColorRalated(void)
{
    UINT8 bWriteData;
    UINT8 bReadData;

    /* VIDEO - check hdmi mode */
    if (_bHdmiMode)
    {
        /* HDMI mode */

        /* Video Setting */
        vHDMIVideoHdmiSetting();

        /* 2x pixel clock setting*/
        bReadData = u1HalReadAviByte5();
        if ((bReadData & 0x0F) != 0)
        {
            bWriteData = 0x17;
        }
        else
        {
            bWriteData = 0x07;
        }
        vHalSetRxPclk(bWriteData);
    }
    else
    {
        vHalSetRxPclk(0x07);// ICLK , no repeat
        // ToDo - use VRes to detect HDTV input.
        vHalClearVideoModeByte1();
        // RGB to YCBCr Space Convert
     // always YCbCr
       vHalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);//Cb:0x80, Y:0x10, Cr:0x80	            vHalRxDisable656SyncMode();
	}
}

UINT8 bHDMIInputType(void)
{//check color space

    UINT8 bReadData;
    if (vHalCheckRxIsHdmiMode())
    {
        if (fgHalCheckAviInforFrameExist()== FALSE)
        {
            return 1;
        }

        _bNEW_AVI_Info=1;
        bReadData = u1HalReadAviType();
        //vHalClearNewAviIntStatus();
        _bAVIInfo_tmp=bReadData;

        if ((bReadData & 0x60)== 0x00) // RGB
            return 1;
        else //Ycbcr
            return 0;
    }
    else
    {
        return 1;
    }

}

UINT8 bHDMIAVIPixelCount(void)
{
    return (u1HalReadAviByte5() & 0x0f);
}

UINT16 wHDMIResoWidth(void)
{
    UINT32 tmp;

    tmp = vHalRxGetActiveWidth();
    if(fgHalCheckRxPclkIs2XRepeat()) // ICLK x2
    {
        tmp <<= 1;
    }

    return tmp;
}

UINT16 wHDMIResoHeight(void)
{
    UINT16 tmp;

    tmp = vHalRxGetActiveHeight();

    return tmp;
}

UINT32 wHDMIHTotal(void)
{
    UINT32 tmp;

    tmp = vHalRxGetHTotal();
    if(fgHalCheckRxPclkIs2XRepeat()) // ICLK
    {
        tmp <<= 1;
    }

    return tmp;
}

UINT16 wHDMIVTotal(void)
{
    UINT16 tmp;

    tmp = vHalRxGetVTotal();

    return tmp;
}


BOOL fgHDMIHsyncAct(void)
{
	if (fgHalChkSCDTEnable())//modify by ciwu
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UINT8 bHDMIScanInfo(void)
{
    if ((fgHalCheckAviInforFrameExist()== FALSE) ||(!_bHdmiMode))
    {
        return 0;
    }
    else
    {
        return (u1HalReadAviByte1()  & 0x03);
    }

}

UINT8 bHDMIAspectRatio(void)
{
    if ((fgHalCheckAviInforFrameExist()== FALSE) ||(!_bHdmiMode))
    {
        return 0;
    }
    else
    {
        return ((u1HalReadAviByte2()  & 0x30)>>4);
    }

}

UINT8 bHDMIAFD(void)
{
    if ((fgHalCheckAviInforFrameExist()== FALSE) ||(!_bHdmiMode))
    {
        return 0;
    }
    else
    {
        return (u1HalReadAviByte2()  & 0xf);
    }

}


UINT8 bHDMI422Input(void)
{

    if ((fgHalCheckAviInforFrameExist()== FALSE) ||(!vHalCheckRxIsHdmiMode()))
    {
        return 0;
    }
    else
    {
        if ((u1HalReadAviType() &0x60) == 0x20 )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

UINT8 bHDMIITCFlag(void)
{
    if ((fgHalCheckAviInforFrameExist()== FALSE) ||(!_bHdmiMode))
    {
        return 0;
    }
    else
    {
        if ((u1HalReadAviByte3()&0x80) == 0x80 )
        {
            return 1;
        }
        else
        {
            return 0;
        }

    }

}

UINT8 bHDMIITCContent(void)
{
    if ((fgHalCheckAviInforFrameExist()== FALSE) ||(!_bHdmiMode))
    {
        return 0;
    }
    else
    {
        return (u1HalReadAviByte5()&0x30)>>4 ;

    }

}
/*
   full range: 0 ~ 255.
   limited range: 16 ~ 235.
00: Default, depend on video format.
PC timing: full range.
Video timing: limited range.
01: limited range.
10: full range.
11: Reserved.
*/
UINT8 bHDMIRgbRange(void)
{
    if ((fgHalCheckAviInforFrameExist()== FALSE) ||(!_bHdmiMode))
    {
        return 0;
    }
    else
    {
        return ((u1HalReadAviByte3() & 0x0c) >> 2);
    }

}

UINT8 bHDMIHDCPStatusGet(void)
{
    if(fgHalCheckRXHdcpDecrptOn())
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINT8 _bIsXpcStable(void)
{
    return ((_bXpcStableCnt > HDMI_XPC_STABLE_CNT) ? 1 : 0);
}

BOOL fgHDMIinterlaced(void)
{
    if (fgHalCheckInterlaceDetect())
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINT8 bHDMIRefreshRate(void)
{
    UINT32 pfreq;
    UINT8 rate;
    UINT32 dwtmp;

	UINT8 _u1RxGetColorspace = 0;
	_u1RxGetColorspace = (_RxPacket[AVI_INFOFRAME].PacketData[4]>>5)&0x03;
	
	if(_u1RxGetColorspace == 3 )
	{
		dwtmp =((wHDMIHTotal())/2) * wHDMIVTotal();
	}
	else
	{
	    dwtmp = wHDMIHTotal() * wHDMIVTotal();
		//printk("[HDMI RX](HTOTAL *VTOTAL) dwtmp = %d\n ",dwtmp);
	}
    if (dwtmp == 0) // avoid divide by zero
    {
        return 1;
    }
    pfreq = dwHDMIPixelFreq();
	
    //rate= ((pfreq*1000)+(dwtmp-1)) / dwtmp; //modify by ciwu
    rate = pfreq /dwtmp;
	
    HDMIRX_LOG("[HDMI RX] pfreq = %x,rate = %d\n",pfreq,rate);
    if ((rate <= 51) && (rate >= 49))
    {
        rate = 50;
    }
    else if ((rate <= 57) && (rate >= 55))
    {
        rate = 56;
    }
    else if ((rate <= 61) && (rate >= 59))
    {
        rate = 60;
    }
    else if ((rate <= 68) && (rate >= 65))
    {
        rate = 67;
    }
    else if ((rate <= 71) && (rate >= 69))
    {
        rate = 70;
    }
    else if ((rate <= 73) && (rate >= 71))
    {
        rate = 72;
    }
    else if ((rate <= 76) && (rate >= 74))
    {
        rate= 75;
    }
    else if ((rate <= 86) && (rate >= 84))
    {
        rate = 85;
    }

    HDMIRX_LOG("[HDMI RX] rate = %d\n",rate);
    return rate;
}

UINT32 dwHDMILineFreq(void)
{
    UINT32 ret;
    UINT16 wDiv;

    wDiv = wHDMIHTotal();
    if (wDiv == 0)
    {
        return 1;
    }

    ret = ((dwHDMIPixelFreq() /100) / (wDiv));

    return ret;//return freq_line*10/1000
}

void vShowAviInforFrame(void)
{
    if(_RxPacket[AVI_INFOFRAME].fgValid)
    {

        printk("*****AVI Inforframe START********************\n");
        printk("    AVI TYPE = 0x%x \n", _RxPacket[AVI_INFOFRAME].PacketData[0]);
        printk(" AVI Version = 0x%x \n", _RxPacket[AVI_INFOFRAME].PacketData[1]);
        printk("  AVI Length = 0x%x \n", _RxPacket[AVI_INFOFRAME].PacketData[2]);
        printk("AVI CheckSum = 0x%x \n", _RxPacket[AVI_INFOFRAME].PacketData[3]);

        // PB0 - PB13
        printk("   AVI BYTE0~7 = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
               _RxPacket[AVI_INFOFRAME].PacketData[3],_RxPacket[AVI_INFOFRAME].PacketData[4],_RxPacket[AVI_INFOFRAME].PacketData[5],
               _RxPacket[AVI_INFOFRAME].PacketData[6],_RxPacket[AVI_INFOFRAME].PacketData[7],_RxPacket[AVI_INFOFRAME].PacketData[8],
               _RxPacket[AVI_INFOFRAME].PacketData[9],_RxPacket[AVI_INFOFRAME].PacketData[10]);
        printk("   AVI BYTE8~13 = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x \n",
               _RxPacket[AVI_INFOFRAME].PacketData[11],_RxPacket[AVI_INFOFRAME].PacketData[12],_RxPacket[AVI_INFOFRAME].PacketData[13],
               _RxPacket[AVI_INFOFRAME].PacketData[14],_RxPacket[AVI_INFOFRAME].PacketData[15],_RxPacket[AVI_INFOFRAME].PacketData[16]);

        //_RxPacket[AVI_INFOFRAME].PacketData[4] is the PB1 of the AVI INFO
        printk("S1,S0: 0x%x, %s \n", _RxPacket[AVI_INFOFRAME].PacketData[4]&0x03, cAviScanStr[_RxPacket[AVI_INFOFRAME].PacketData[4]&0x03]);
        printk("B1,S0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[4]>>2)&0x03, cAviBarStr[(_RxPacket[AVI_INFOFRAME].PacketData[4]>>2)&0x03]);
        printk("A0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[4]>>4)&0x01, cAviActivePresentStr[(_RxPacket[AVI_INFOFRAME].PacketData[4]>>4)&0x01]);
        printk("Y1,Y0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[4]>>5)&0x03, cAviRgbYcbcrStr[(_RxPacket[AVI_INFOFRAME].PacketData[4]>>5)&0x03]);
        printk("R3~R0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[5])&0x0f, cAviActiveStr[(_RxPacket[AVI_INFOFRAME].PacketData[5])&0x0f]);
        printk("M1,M0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[5]>>4)&0x03, cAviAspectStr[(_RxPacket[AVI_INFOFRAME].PacketData[5]>>4)&0x03]);
        printk("C1,C0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[5]>>6)&0x03, cAviColorimetryStr[(_RxPacket[AVI_INFOFRAME].PacketData[5]>>6)&0x03]);
        printk("SC1,SC0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[6])&0x03, cAviScaleStr[(_RxPacket[AVI_INFOFRAME].PacketData[6])&0x03]);
        printk("Q1,Q0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[6]>>2)&0x03, cAviRGBRangeStr[(_RxPacket[AVI_INFOFRAME].PacketData[6]>>2)&0x03]);
        if(((_RxPacket[AVI_INFOFRAME].PacketData[6]>>4)&0x07)<=1)
            printk("EC2~EC0: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[6]>>4)&0x07, cAviExtColorimetryStr[(_RxPacket[AVI_INFOFRAME].PacketData[6]>>4)&0x07]);
        else
            printk("EC2~EC0: resevered\n");

        printk("ITC: 0x%x, %s \n", (_RxPacket[AVI_INFOFRAME].PacketData[6]>>7)&0x01, cAviItContentStr[(_RxPacket[AVI_INFOFRAME].PacketData[6]>>7)&0x01]);
        printk("*****AVI Inforframe END**********************\n");

    }
}

void vShowAudioInforFrame(void)
{
    if(_RxPacket[AUDIO_INFOFRAME].fgValid)
    {
        printk("*****Audio Inforframe START********************\n");
        printk("    AUD TYPE = 0x%x \n", _RxPacket[AUDIO_INFOFRAME].PacketData[0]);
        printk(" AUD Version = 0x%x \n", _RxPacket[AUDIO_INFOFRAME].PacketData[1]);
        printk("  AUD Length = 0x%x \n", _RxPacket[AUDIO_INFOFRAME].PacketData[2]);
        printk("AUD CheckSum = 0x%x \n", _RxPacket[AUDIO_INFOFRAME].PacketData[3]);

        // PB0 - PB10
        printk("   AUD BYTE0~7 = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
               _RxPacket[AUDIO_INFOFRAME].PacketData[3],_RxPacket[AUDIO_INFOFRAME].PacketData[4],_RxPacket[AUDIO_INFOFRAME].PacketData[5],
               _RxPacket[AUDIO_INFOFRAME].PacketData[6],_RxPacket[AUDIO_INFOFRAME].PacketData[7],_RxPacket[AUDIO_INFOFRAME].PacketData[8],
               _RxPacket[AUDIO_INFOFRAME].PacketData[9],_RxPacket[AUDIO_INFOFRAME].PacketData[10]);
        printk("   AUD BYTE8~10 = 0x%x,0x%x,0x%x \n",
               _RxPacket[AUDIO_INFOFRAME].PacketData[11],_RxPacket[AUDIO_INFOFRAME].PacketData[12],_RxPacket[AUDIO_INFOFRAME].PacketData[13]);


        printk("CC2~ CC0: 0x%x, %s \n", (_RxPacket[AUDIO_INFOFRAME].PacketData[4])&0x07, cAudChCountStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[4])&0x07]);
		printk("CT3~ CT0: 0x%x, %s \n", (_RxPacket[AUDIO_INFOFRAME].PacketData[4]>>4)&0x0f, cAudCodingTypeStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[4]>>4)&0x0f]);
        printk("SS1, SS0: 0x%x, %s \n", (_RxPacket[AUDIO_INFOFRAME].PacketData[5])&0x03, cAudSampleSizeStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[5])&0x03]);
        printk("SF2~ SF0: 0x%x, %s \n", ((_RxPacket[AUDIO_INFOFRAME].PacketData[5])>>2)&0x07, cAudFsStr[((_RxPacket[AUDIO_INFOFRAME].PacketData[5])>>2)&0x07]);
        printk("CA7~ CA0: 0x%x, %s \n", (_RxPacket[AUDIO_INFOFRAME].PacketData[7])&0xff, cAudChMapStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[7])&0xff]);
        printk("LSV3~LSV0: %d db \n", ((_RxPacket[AUDIO_INFOFRAME].PacketData[8])>>3)&0x0f);
        printk("DM_INH: 0x%x , \n", ((_RxPacket[AUDIO_INFOFRAME].PacketData[8])>>7)&0x01, cAudDMINHStr[((_RxPacket[AUDIO_INFOFRAME].PacketData[8])>>7)&0x01]);


        printk("*****Audio Inforframe END**********************\n");
    }
}


void vShowACPInforFrame(void)
{
    if(_RxPacket[ACP_PACKET].fgValid)
    {
        printk("*****ACP Inforframe START********************\n");
        printk(" ACP HB0 = 0x%x \n", _RxPacket[ACP_PACKET].PacketData[0]);
        printk("ACP TYPE = 0x%x \n", _RxPacket[ACP_PACKET].PacketData[1]);
        printk(" ACP HB2 = 0x%x \n", _RxPacket[ACP_PACKET].PacketData[2]);


        if(_RxPacket[ACP_PACKET].PacketData[1] ==0)
        {
            printk("Generic Audio\n");


            printk("  Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
                   _RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4],_RxPacket[ACP_PACKET].PacketData[5],
                   _RxPacket[ACP_PACKET].PacketData[6],_RxPacket[ACP_PACKET].PacketData[7],_RxPacket[ACP_PACKET].PacketData[8],
                   _RxPacket[ACP_PACKET].PacketData[9],_RxPacket[ACP_PACKET].PacketData[10]);
            printk("  Data Byte (8~15)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
                   _RxPacket[ACP_PACKET].PacketData[11],_RxPacket[ACP_PACKET].PacketData[12],_RxPacket[ACP_PACKET].PacketData[13],
                   _RxPacket[ACP_PACKET].PacketData[14],_RxPacket[ACP_PACKET].PacketData[15],_RxPacket[ACP_PACKET].PacketData[17],
                   _RxPacket[ACP_PACKET].PacketData[18],_RxPacket[ACP_PACKET].PacketData[19]);
        }
        else if(_RxPacket[ACP_PACKET].PacketData[1] ==1)
        {
            printk("IEC 60958-Identified Audio\n");
            printk("  Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
                   _RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4],_RxPacket[ACP_PACKET].PacketData[5],
                   _RxPacket[ACP_PACKET].PacketData[6],_RxPacket[ACP_PACKET].PacketData[7],_RxPacket[ACP_PACKET].PacketData[8],
                   _RxPacket[ACP_PACKET].PacketData[9],_RxPacket[ACP_PACKET].PacketData[10]);
            printk("  Data Byte (8~15)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
                   _RxPacket[ACP_PACKET].PacketData[11],_RxPacket[ACP_PACKET].PacketData[12],_RxPacket[ACP_PACKET].PacketData[13],
                   _RxPacket[ACP_PACKET].PacketData[14],_RxPacket[ACP_PACKET].PacketData[15],_RxPacket[ACP_PACKET].PacketData[17],
                   _RxPacket[ACP_PACKET].PacketData[18],_RxPacket[ACP_PACKET].PacketData[19]);
        }
        else if(_RxPacket[ACP_PACKET].PacketData[1] ==2)
        {
            printk("DVD Audio\n");
            printk("   Data Byte (0~1)= 0x%x,0x%x \n",
                   _RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4]);

            printk("DVD-AUdio_TYPE_Dependent Generation = 0x%x \n", _RxPacket[ACP_PACKET].PacketData[3]);
            printk("Copy Permission = 0x%x \n", ((_RxPacket[ACP_PACKET].PacketData[4])>>6)&0x03);
            printk("Copy Number = 0x%x \n", ((_RxPacket[ACP_PACKET].PacketData[4])>>3)&0x07);
            printk("Quality = 0x%x \n", ((_RxPacket[ACP_PACKET].PacketData[4])>>1)&0x03);
            printk("Transaction = 0x%x \n", (_RxPacket[ACP_PACKET].PacketData[4])&0x01);

        }
        else if(_RxPacket[ACP_PACKET].PacketData[1] ==3)
        {
            printk("SuperAudio CD\n");

            printk("  CCI_1 (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
                   _RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4],_RxPacket[ACP_PACKET].PacketData[5],
                   _RxPacket[ACP_PACKET].PacketData[6],_RxPacket[ACP_PACKET].PacketData[7],_RxPacket[ACP_PACKET].PacketData[8],
                   _RxPacket[ACP_PACKET].PacketData[9],_RxPacket[ACP_PACKET].PacketData[10]);
            printk("  CCI_1 (8~15)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
                   _RxPacket[ACP_PACKET].PacketData[11],_RxPacket[ACP_PACKET].PacketData[12],_RxPacket[ACP_PACKET].PacketData[13],
                   _RxPacket[ACP_PACKET].PacketData[14],_RxPacket[ACP_PACKET].PacketData[15],_RxPacket[ACP_PACKET].PacketData[17],
                   _RxPacket[ACP_PACKET].PacketData[18],_RxPacket[ACP_PACKET].PacketData[19]);
        }


        printk("*****ACP Inforframe END**********************\n");
    }
}


void vShowSPDInforFrame(void)
{
    if(_RxPacket[SPD_INFOFRAME].fgValid)
    {
        printk("*****SPD Inforframe START********************\n");
        printk("    SPD TYPE = 0x%x \n", u1HalReadSPDType());
        printk(" SPD Version = 0x%x \n", u1HalReadSPDVersion());
        printk("  SPD Length = 0x%x \n", u1HalReadSPDLength());


        printk("	 Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
               _RxPacket[SPD_INFOFRAME].PacketData[3],_RxPacket[SPD_INFOFRAME].PacketData[4],_RxPacket[SPD_INFOFRAME].PacketData[5],
               _RxPacket[SPD_INFOFRAME].PacketData[6],_RxPacket[SPD_INFOFRAME].PacketData[7],_RxPacket[SPD_INFOFRAME].PacketData[8],
               _RxPacket[SPD_INFOFRAME].PacketData[9],_RxPacket[SPD_INFOFRAME].PacketData[10]);
        printk("	Data Byte (8~15) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
               _RxPacket[SPD_INFOFRAME].PacketData[11],_RxPacket[SPD_INFOFRAME].PacketData[12],_RxPacket[SPD_INFOFRAME].PacketData[13],
               _RxPacket[SPD_INFOFRAME].PacketData[14],_RxPacket[SPD_INFOFRAME].PacketData[15],_RxPacket[SPD_INFOFRAME].PacketData[16],
               _RxPacket[SPD_INFOFRAME].PacketData[17],_RxPacket[SPD_INFOFRAME].PacketData[18]);
        printk("	Data Byte (16~23)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
               _RxPacket[SPD_INFOFRAME].PacketData[19],_RxPacket[SPD_INFOFRAME].PacketData[20],_RxPacket[SPD_INFOFRAME].PacketData[21],
               _RxPacket[SPD_INFOFRAME].PacketData[22],_RxPacket[SPD_INFOFRAME].PacketData[23],_RxPacket[SPD_INFOFRAME].PacketData[24],
               _RxPacket[SPD_INFOFRAME].PacketData[25],_RxPacket[SPD_INFOFRAME].PacketData[26]);
        printk("	Data Byte (24~25)= 0x%x,0x%x \n",
               _RxPacket[SPD_INFOFRAME].PacketData[27],_RxPacket[SPD_INFOFRAME].PacketData[28]);

        printk("Source Device information is %s \n", cSPDDeviceStr[_RxPacket[SPD_INFOFRAME].PacketData[28]]);


        printk("*****SPD Inforframe END**********************\n");
    }
}

void vShowGamutInforFrame(void)
{
    if(_RxPacket[GAMUT_PACKET].fgValid)
    {
        printk("*****GAMUT Inforframe START********************\n");
        printk("   GAMUT HB0 = 0x%x \n", _RxPacket[GAMUT_PACKET].PacketData[0]);
        printk("   GAMUT HB1 = 0x%x \n", _RxPacket[GAMUT_PACKET].PacketData[1]);
        printk("   GAMUT HB2 = 0x%x \n", _RxPacket[GAMUT_PACKET].PacketData[2]);

        printk("	 Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
               _RxPacket[GAMUT_PACKET].PacketData[3],_RxPacket[GAMUT_PACKET].PacketData[4],_RxPacket[GAMUT_PACKET].PacketData[5],
               _RxPacket[GAMUT_PACKET].PacketData[6],_RxPacket[GAMUT_PACKET].PacketData[7],_RxPacket[GAMUT_PACKET].PacketData[8],
               _RxPacket[GAMUT_PACKET].PacketData[9],_RxPacket[GAMUT_PACKET].PacketData[10]);
        printk("	Data Byte (8~14) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x \n",
               _RxPacket[GAMUT_PACKET].PacketData[11],_RxPacket[GAMUT_PACKET].PacketData[12],_RxPacket[GAMUT_PACKET].PacketData[13],
               _RxPacket[GAMUT_PACKET].PacketData[14],_RxPacket[GAMUT_PACKET].PacketData[15],_RxPacket[GAMUT_PACKET].PacketData[16],
               _RxPacket[GAMUT_PACKET].PacketData[17]);
        printk("*****GAMUT Inforframe END**********************\n");
    }
}


void vShowMPEGInforFrame(void)
{
    if(_RxPacket[MPEG_INFOFRAME].fgValid)
    {

        printk("====================MPEG inforFrame Start ====================================\n\r");

        printk("	MPEG TYPE = 0x%x \n", _RxPacket[MPEG_INFOFRAME].PacketData[0]);
        printk(" MPEG Version = 0x%x \n", _RxPacket[MPEG_INFOFRAME].PacketData[1]);
        printk("  MPEG Length = 0x%x \n", _RxPacket[MPEG_INFOFRAME].PacketData[2]);

        printk("Data Byte (0~7) = 0x%x	0x%x  0x%x	0x%x  0x%x	0x%x  0x%x	0x%x\n",
                    _RxPacket[MPEG_INFOFRAME].PacketData[3],_RxPacket[MPEG_INFOFRAME].PacketData[4],_RxPacket[MPEG_INFOFRAME].PacketData[5],
                    _RxPacket[MPEG_INFOFRAME].PacketData[6],_RxPacket[MPEG_INFOFRAME].PacketData[7],_RxPacket[MPEG_INFOFRAME].PacketData[8],
                    _RxPacket[MPEG_INFOFRAME].PacketData[8],_RxPacket[MPEG_INFOFRAME].PacketData[10]);
        printk("Data Byte (8~15)= 0x%x	0x%x  0x%x	0x%x  0x%x	0x%x  0x%x	0x%x\n",
                    _RxPacket[MPEG_INFOFRAME].PacketData[11],_RxPacket[MPEG_INFOFRAME].PacketData[12],_RxPacket[MPEG_INFOFRAME].PacketData[13],
                    _RxPacket[MPEG_INFOFRAME].PacketData[14],_RxPacket[MPEG_INFOFRAME].PacketData[15],_RxPacket[MPEG_INFOFRAME].PacketData[16],
                    _RxPacket[MPEG_INFOFRAME].PacketData[17],_RxPacket[MPEG_INFOFRAME].PacketData[18]);
        printk("====================MPEG inforFrame End ======================================\n\r");
    }


}


void vShowISRC1InforFrame(void)
{
    if(_RxPacket[ISRC1_PACKET].fgValid)
    {

        printk("====================ISRC1 inforFrame Start ====================================\n\r");


        printk(" ISRC1 TYPE = 0x%x \n", _RxPacket[ISRC1_PACKET].PacketData[0]);
        printk("  ISRC1 HB1 = 0x%x \n", _RxPacket[ISRC1_PACKET].PacketData[1]);
	    printk("  ISRC1 HB2 = 0x%x \n", _RxPacket[ISRC1_PACKET].PacketData[2]);

        printk("Data Byte (0~7) = 0x%x	0x%x  0x%x	0x%x  0x%x	0x%x  0x%x	0x%x\n",
                    _RxPacket[ISRC1_PACKET].PacketData[3],_RxPacket[ISRC1_PACKET].PacketData[4],_RxPacket[ISRC1_PACKET].PacketData[5],
                    _RxPacket[ISRC1_PACKET].PacketData[6],_RxPacket[ISRC1_PACKET].PacketData[7],_RxPacket[ISRC1_PACKET].PacketData[8],
                    _RxPacket[ISRC1_PACKET].PacketData[8],_RxPacket[ISRC1_PACKET].PacketData[10]);
        printk("Data Byte (8~15)= 0x%x	0x%x  0x%x	0x%x  0x%x	0x%x  0x%x	0x%x\n",
                    _RxPacket[ISRC1_PACKET].PacketData[11],_RxPacket[ISRC1_PACKET].PacketData[12],_RxPacket[ISRC1_PACKET].PacketData[13],
                    _RxPacket[ISRC1_PACKET].PacketData[14],_RxPacket[ISRC1_PACKET].PacketData[15],_RxPacket[ISRC1_PACKET].PacketData[16],
                    _RxPacket[ISRC1_PACKET].PacketData[17],_RxPacket[ISRC1_PACKET].PacketData[18]);

        printk("====================ISRC1 inforFrame End ======================================\n\r");
    }


}

void vShowISRC2InforFrame(void)
{
    if(_RxPacket[ISRC2_PACKET].fgValid)
    {

        printk("====================ISRC2 inforFrame Start ====================================\n\r");

        printk("  ISRC2 HB0 = 0x%x \n", _RxPacket[ISRC2_PACKET].PacketData[0]);
        printk("  ISRC2 HB1 = 0x%x \n", _RxPacket[ISRC2_PACKET].PacketData[1]);
		printk("  ISRC2 HB2 = 0x%x \n", _RxPacket[ISRC2_PACKET].PacketData[2]);

        printk("Data Byte (0~7)= 0x%x	0x%x  0x%x	0x%x  0x%x	0x%x  0x%x	0x%x\n",
                    _RxPacket[ISRC2_PACKET].PacketData[3],_RxPacket[ISRC2_PACKET].PacketData[4],_RxPacket[ISRC2_PACKET].PacketData[5],
                    _RxPacket[ISRC2_PACKET].PacketData[6],_RxPacket[ISRC2_PACKET].PacketData[7],_RxPacket[ISRC2_PACKET].PacketData[8],
                    _RxPacket[ISRC2_PACKET].PacketData[8],_RxPacket[ISRC2_PACKET].PacketData[10]);
        printk("Data Byte (8~15)= 0x%x	0x%x  0x%x	0x%x  0x%x	0x%x  0x%x	0x%x\n",
                    _RxPacket[ISRC2_PACKET].PacketData[11],_RxPacket[ISRC2_PACKET].PacketData[12],_RxPacket[ISRC2_PACKET].PacketData[13],
                    _RxPacket[ISRC2_PACKET].PacketData[14],_RxPacket[ISRC2_PACKET].PacketData[15],_RxPacket[ISRC2_PACKET].PacketData[16],
                    _RxPacket[ISRC2_PACKET].PacketData[17],_RxPacket[ISRC2_PACKET].PacketData[18]);

        printk("====================ISRC2 inforFrame End ======================================\n\r");
    }
    else
        printk("[HDMI RX]ISRC2 is not Valid \n");


}

void vShowVENDInforFrame(void)
{
    if(_RxPacket[VENDOR_INFOFRAME].fgValid)
    {

        printk("	VEND TYPE = 0x%x \n", _RxPacket[VENDOR_INFOFRAME].PacketData[0]);
        printk(" VEND Version = 0x%x \n", _RxPacket[VENDOR_INFOFRAME].PacketData[1]);
        printk("  VEND Length = 0x%x \n", _RxPacket[VENDOR_INFOFRAME].PacketData[2]);

        printk("====================Vendor Spec inforFrame Start ====================================\n\r");

        printk("Data Byte (0~6) = 0x%x	0x%x  0x%x	0x%x  0x%x	0x%x  0x%x	\n",
                    _RxPacket[VENDOR_INFOFRAME].PacketData[3],_RxPacket[VENDOR_INFOFRAME].PacketData[4],_RxPacket[VENDOR_INFOFRAME].PacketData[5],
                    _RxPacket[VENDOR_INFOFRAME].PacketData[6],_RxPacket[VENDOR_INFOFRAME].PacketData[7],_RxPacket[VENDOR_INFOFRAME].PacketData[8],
                    _RxPacket[VENDOR_INFOFRAME].PacketData[8]);

        printk("====================Vendor Spec inforFrame End ======================================\n\r");
    }


}

void vShowGCPInforFrame(void)
{
    UINT8 u1Tmp = 0;
	
	printk("====================General Control Packet Start ====================================\n\r");

	printk("  General Control Packet HB0 = 0x03 \n");
	printk("  General Control Packet HB1 = 0x00 \n");
	printk("  General Control Packet HB2 = 0x00 \n");

	printk("Data Byte (0~6) = \n");
	if(vHalCheckGcpMuteEnable())
		printk("0x01 ");
	else
		printk("0x10 ");

	u1Tmp = bHDMIDeepColorStatus() | (1<<2);
	
	printk("0x%x  0x00  0x00  0x00  0x00  0x00\n",u1Tmp);

       vShowAVMuteStatus();
	   vShowRxDeepColorStatus();

	printk("====================General Control Packet End ======================================\n\r");

}

void vShowSACDInforFrame(void)
{
	printk("====================One Bit Audio Packet Start ====================================\n\r");
    if(fgHalIsINTR7_SACD())
        printk("[HDMI RX] One Bit Audio Packet Send \n");
	else
		printk("[HDMI RX] One Bit Audio Packet Not Send \n");

	printk("====================One Bit Audio Packet End ======================================\n\r");
}

static UINT8 bHDMIPort5VStatus(UINT8 u1Switch)
{
    UINT8 u15VDetect= FALSE;
	
    u15VDetect = u1HalChkPwr5VExist();
    return u15VDetect;

}

static void vHDMIMuteAudio(void)
{
    if (!IS_AUD_MUTE())
    {
        vHalMuteHdmiRxAudioOut();
        _fgAudMute = TRUE;
    }
}

static void vHDMIUnMuteAudio(void)
{
    if (IS_AUD_MUTE())
    {
        vHalUnMuteHdmiRxAudioOut();
        _fgAudMute = FALSE;
    }
}

static void vXpcStableCount(void)
{
    if (u1HalChkDataEnableExist() && \
        ((_bHDMIState == HDMI_STATE_SCDT) || (_bHDMIState == HDMI_STATE_AUTH))) {

        u4CurXpcCnt = wHDMIXPCCNT();

        // CKDT stable counting
        if (RANGE_CHECKING(u4CurXpcCnt, u4PreXpcCnt, 3)) {

            if (_bXpcStableCnt < 255) _bXpcStableCnt++;
        }
        else {

            if (_bXpcStableCnt == 255) {
                _bXpcStableCnt--;
            }
            else {
                _bXpcStableCnt = 0;
            }
        }

        u4PreXpcCnt = u4CurXpcCnt;
    }
}

static UINT32 wHDMIXPCCNT(void)
{
    UINT32 u4XclkInPclk = 0;
#if 0
    UINT32 datacnt[DATA_CNT_MAX];
    UINT32 idx, i,tmp;

    for (i = 0 ; i < DATA_CNT_MAX ; i++)
    {
        datacnt[i] = 0;
    }

    idx = 0;
    // TODO
    for (i=0; i<10; i++)
    {

        tmp = u4HalGetRxPixelClock();
        printk("[HDMI RX] pixel clk tmp = %d\n",tmp);
        if (datacnt[0] == tmp)
        {
            continue;
        }
        if (datacnt[1] == tmp)
        {
            continue;
        }

        if (idx < DATA_CNT_MAX)
        {
            datacnt[idx] = tmp;
        }

        idx++;
        if (idx == 2)
        {
            break;
        }
    }

    if (idx == 2)
    {
        printk("[HDMI RX] idx = %d, ((datacnt[0]+datacnt[1])*(100/2))= %d\n",idx,(datacnt[0]+datacnt[1])*(100/2));
        return (datacnt[0]+datacnt[1])*(100/2);
    }
    else
    {
        printk("[HDMI RX] idx = %d, datacnt[0] = %d\n",idx,datacnt[0]*(100));
        return ((datacnt[0])*(100));
    }
#endif
	u4XclkInPclk = u4HalGetRxPixelClock();
	HDMIRX_LOG("[HDMI RX 0] u4XclkInPclk = %x\n",u4XclkInPclk);
	return u4XclkInPclk;
}

static void vLogHdmiStateChange(UINT8 u1HdmiState)
{

    if (u1HdmiState <= HDMI_STATE_AUTH)
    {
        if (u1HdmiStateOld != u1HdmiState)
        {
            printk( "[HDMI RX]_bHDMIState change from %s to %s\n", _aszHdmiState[u1HdmiStateOld],_aszHdmiState[u1HdmiState]);
            u1HdmiStateOld = u1HdmiState;
        }
    }
    else // Undefined state
    {
        if (u1HdmiStateOld != u1HdmiState)
        {
            printk( "[HDMI RX]_bHDMIState change from %s to UNDEFINED_STATE\n", _aszHdmiState[u1HdmiStateOld]);
            u1HdmiStateOld = HDMI_STATE_NOTREADY;
        }
    }

}

static UINT32 dwHDMIPixelFreq(void)
{
    UINT32 u4PclkFreq;
    UINT32 u4XclkNum;
    UINT32 u4PclkNum = 0;
    UINT32 u4XclkFreq = (26 *1000 *1000); //26 M crystal

    if(HDMIRX_READ32(REG_VID_CRC_OUT) & XCLK_IN_PCLK_SEL)
    {
        u4PclkNum = 1024;
    }
    else
    {
        u4PclkNum = 128;
    }
	
    u4XclkNum = wHDMIXPCCNT();

    if (u4XclkNum == 0)
    {
        return 1;
    }

    // caculate pixel clock,  u4XclkNum * (1/u4XclkFreq) = u4PclkNum * (1/u4PclkFreq)
    
 	u4PclkFreq = (u4XclkFreq /u4XclkNum)*u4PclkNum; //for range 0xffffffff
    return u4PclkFreq;
}

static UINT8 bHDMIAUDIOSampleRateCal(void)
{
    UINT32 wCTS_HW,wN_HW;
    UINT32 wAudSampleRate;
    UINT8 btmp;
    static UINT8 Ori_audio_FS;

    btmp=20;
    wCTS_HW= u4HalGetRxHwCTSValue();
    wN_HW= u4HalGetRxHwNValue();
    wAudSampleRate= (((dwHDMIPixelFreq()*1000)/wCTS_HW) *((wN_HW*100)/128))/10000;

    if(bHDMIDeepColorStatus()== RX_30BITS_DEEP_COLOR)
    {
        wAudSampleRate=(wAudSampleRate*10)/8;
    }
    else if(bHDMIDeepColorStatus()== RX_36BITS_DEEP_COLOR)
    {
        wAudSampleRate=(wAudSampleRate*12)/8;
    }
    else if(bHDMIDeepColorStatus()== RX_48BITS_DEEP_COLOR)
    {
        wAudSampleRate=(wAudSampleRate*16)/8;
    }

    if(wAudSampleRate> (320-btmp) && wAudSampleRate<(320+btmp))
    {
        if (Ori_audio_FS!=AUD_FS_32K)
        {
            vHalReInitAudioClock();
            Ori_audio_FS=AUD_FS_32K;
        }
        return AUD_FS_32K;
    }
    else if(wAudSampleRate> (441-btmp) && wAudSampleRate<(441+btmp))
    {
        if (Ori_audio_FS!=AUD_FS_44K)
        {
            vHalReInitAudioClock();
            Ori_audio_FS=AUD_FS_44K;
        }
        return AUD_FS_44K;
    }
    else if(wAudSampleRate> (480-btmp) && wAudSampleRate<(480+btmp))
    {
        if (Ori_audio_FS!=AUD_FS_48K)
        {
            vHalReInitAudioClock();
            Ori_audio_FS=AUD_FS_48K;
        }
        return AUD_FS_48K;
    }
    else if(wAudSampleRate> (880-btmp) && wAudSampleRate<(880+btmp))
    {
        if (Ori_audio_FS!=AUD_FS_88K)
        {
            vHalReInitAudioClock();
            Ori_audio_FS=AUD_FS_88K;
        }
        return AUD_FS_88K;
    }
    else if(wAudSampleRate> (960-btmp) && wAudSampleRate<(960+btmp))
    {
        if (Ori_audio_FS!=AUD_FS_96K)
        {
            vHalReInitAudioClock();
            Ori_audio_FS=AUD_FS_96K;
        }
        return AUD_FS_96K;
    }
    else if(wAudSampleRate> (1760-btmp) && wAudSampleRate<(1760+btmp))
    {
        if (Ori_audio_FS!=AUD_FS_176K)
        {
            vHalReInitAudioClock();
            Ori_audio_FS=AUD_FS_176K;
        }
        return AUD_FS_176K;
    }
    else if(wAudSampleRate> (1920-btmp) && wAudSampleRate<(1920+btmp))
    {
        if (Ori_audio_FS!=AUD_FS_192K)
        {
            vHalReInitAudioClock();
            Ori_audio_FS=AUD_FS_192K;

        }
        return AUD_FS_192K;
    }
    else
    {
        return AUD_FS_UNKNOWN;
    }

}

static void vHDMIHandleAudFifoFault(void)
{
    UINT32 HW_CTS_Value;
    UINT8 u1Fs;

    HW_CTS_Value = u4HalGetRxHwCTSValue();


    if (_fgVideoOn)
    {
        //u1Fs = HDMIAUDIOSampleRateCal();
        if (u1Fs == AUD_FS_UNKNOWN)
        {
            vHDMIMuteAudio();
        }
        else
        {
            if (fgHalCheckIsNonDeepColorMode() && (650000 < HW_CTS_Value || 81000 == HW_CTS_Value))
            {
                vHDMIMuteAudio();
            }
            else
            {
                vHDMIUnMuteAudio();
            }
        }
    }
}

static void vHDMIVideoHdmiSetting(void)
{
    UINT8 bReadData;
    // Check NO_AVI

    if (fgHalCheckIsNoAvi())
    {
        vHalClearIntrState1Bit0_Bit7();//Clear Audio releative InT state
        return;
    }

    vHalClearVideoModeByte0();
    vHalClearVideoModeByte1();

    // check AVI infoframe packet type code
    if (fgHalCheckAviInforFrameExist()== FALSE)//no AVI
    {

        // default: RGB
        vHalRxDisable656SyncMode();
        vHalRxDisable422to444UpSample();
	// always YCbCr
		vHalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);//Cb:0x80, Y:0x10, Cr:0x80
        return;
    }
    /*
       AVI DBYTE1
       [6:5] Y1 Y0
       00 - RGB
       01 - YCbCr 422
       10 - YCbCr 444
       */
    bReadData = u1HalReadAviByte1();
    if ((bReadData & 0x60)== 0x00) // RGB
    {
        bReadData = u1HalReadAviByte2();
        vHalRxDisable656SyncMode();
        vHalRxDisable422to444UpSample();
	// always YCbCr
		vHalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);//Cb:0x80, Y:0x10, Cr:0x80
    }
	else if ((bReadData & 0xE0)== 0x60) // 420
	{
	    vHalSetRxYCbCrBlankValue(0x10, 0x10, 0x80);//Cb:0x80, Y:0x10, Cr:0x80
	}
    else // YCbCR
    {
        vHalRxDisable422to444UpSample();
        vHalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);//Cb:0x80, Y:0x10, Cr:0x80
    }
}

static void vHDMIHDCPRst(void)
{
    vHalRxHdcpReset();
    HDMIRX_LOG("[HDMI RX][HDCP] RX HDCP RESET\n");
}

void vHDMITMDSCTRL(UINT8 bOnOff)
{
	if(!_bHPD_Indep_Ctrl)
	{
		if(bOnOff > 0)
		{
			vHalRxHDMITMDSCTRL(TRUE);
		}
		else
		{
		    vHalRxHDMITMDSCTRL(FALSE);
		}
	}
}

void vHDMIRXColorSpaceConveter(void)
{
    
	UINT32 u4data;
    if(bHDMIInputType()==0x0)
    {//YUV
        if(!bHDMI422Input())
        {//YUV444 input
        	printk("[HDMI RX] input 444\n");
            HDMIRX_WRITE32_MASK(REG_SRST,BYPASS_VPROC,BYPASS_VPROC);
            HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, ENRGB2YC);
            HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, ENDOWNSAMPLE);
            HDMIRX_WRITE32_MASK(REG_AUDIO, 0, YUV422_OUT_REPACK);
			u4data = HDMIRX_READ32(REG_VID_MODE);
			printk("4!!!!!!!!!!!!!!!u4data = 0%x\n",u4data);
			
        }
        else
        {//YUV4222 input
        	printk("[HDMI RX] input 422\n");
            HDMIRX_WRITE32_MASK(REG_SRST,BYPASS_VPROC, BYPASS_VPROC);
            HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, ENRGB2YC);
            HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, ENDOWNSAMPLE);
            HDMIRX_WRITE32_MASK(REG_AUDIO,YUV422_OUT_REPACK, YUV422_OUT_REPACK);
        }
		HDMIRX_WRITE32_MASK(REG_SRST, BYPASS_VPROC_ATUO, BYPASS_VPROC_ATUO);
    }
    else
    { //RGB
		printk("[HDMI RX] input RGB\n");
        HDMIRX_WRITE32_MASK(REG_SRST, 0, BYPASS_VPROC);
        HDMIRX_WRITE32_MASK(REG_VID_MODE, ENRGB2YC, ENRGB2YC);
        HDMIRX_WRITE32_MASK(REG_VID_MODE, 0, ENDOWNSAMPLE);
        HDMIRX_WRITE32_MASK(REG_AUDIO, 0, YUV422_OUT_REPACK);

        if(bHDMIDeepColorStatus()!= RX_NON_DEEP)
            HDMIRX_WRITE32_MASK(REG_SRST, 0, BYPASS_VPROC_ATUO);
        else
            HDMIRX_WRITE32_MASK(REG_SRST, BYPASS_VPROC_ATUO, BYPASS_VPROC_ATUO);
    }

}


//HDMI RX HDCP FUNCTION
/************************************************************************
Function    : void RxHDCPSetReceiver(void)

Description : This function will set RX in HDCP Receive Mode only
Parameter   : None
Return      : None
 ********************************************************************/
void RxHDCPSetReceiver(void)
{
    vHalSetRepeaterMode(FALSE);
    _bHDCPMode = HDCP_RECEIVER;
    HDMIRX_LOG("[HDMI RX][HDCP]Rx HDCP RECEIVER MODE\n");
	
    if(_bRxHDCPMode == 2)//debug mode 2:force to repeater
        RxHDCPSetRepeater();
}
/************************************************************************
Function    : void RxHDCPSetRepeater(void)

Description : This function will set RX in HDCP Repeater Mode
Parameter   : None
Return      : None
 ********************************************************************/
//=====================================================================
void RxHDCPSetRepeater(void)
{
    vHalSetRepeaterMode(TRUE);
    _bHDCPMode = HDCP_REPEATER;
    printk("[HDMI RX][HDCP] Rx HDCP REPEATER MODE\n");
	
    if(_bRxHDCPMode == 1)//debug mode 1:force to receiver
        RxHDCPSetReceiver();
}

void vTxSetRxHdcpAuthDone(BOOL fgTxHdcpAuthDone)
{
    _fgTxHDCPAuthDone = fgTxHdcpAuthDone;
    if(fgTxHdcpAuthDone)
        HDMIRX_LOG("[HDMI RX][HDCP] TX Auth Done\n");
    else
        HDMIRX_LOG("[HDMI RX][HDCP] TX UnAuth \n");
}

/************************************************************************
Function    : void vRxHDCPSetTxKsv(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist)

Description : This function will set TX Bstaus, Bksv and down-stream Ksv-List
Parameter   : None
Return      : None
 ********************************************************************/
void vRxHDCPSetTxKsv(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist, BOOL fgTxVMatch)
{
    BYTE i, j;

    _TxDownStreamCount = bTxDownStream;
    _TxBStatus = u2TxBStatus;
    _fgTxVMatch = fgTxVMatch;
    for(i=0; i < 5 ; i++)
        _TxBKsv[i]= *(prbTxBksv+i);

    HDMIRX_LOG("[HDMI RX][HDCP] vRxHDCPSetTxKsv\n") ;
    HDMIRX_LOG("[HDMI RX][HDCP] _TxDownStreamCount = 0x%x\n",_TxDownStreamCount) ;
    HDMIRX_LOG("[HDMI RX][HDCP] _TxBStatus = 0x%x\n",_TxBStatus) ;
    HDMIRX_LOG("[HDMI RX][HDCP] _fgTxVMatch = 0x%x\n",_fgTxVMatch) ;
    HDMIRX_LOG("[HDMI RX][HDCP] _TxBKsv[1~5] = 0x%x,0x%x,0x%x,0x%x,0x%x\n",_TxBKsv[0],_TxBKsv[1],_TxBKsv[2],_TxBKsv[3],_TxBKsv[4]) ;

    if(_TxDownStreamCount <= 9)
    {
        for(j=0; j < _TxDownStreamCount ; j++)
            for(i=0; i < 5 ; i++)
                _TxKsvList[j*5+i]=  *(prbTxKsvlist+j*5+i);
    }
    else
    {
        HDMIRX_LOG("[HDMI RX][HDCP] !!!Error!! TX dowstream over 9 = %d \n", _TxDownStreamCount) ;

    }
        for(j=0; j < _TxDownStreamCount ; j++)
	    if(j < 9)
	    {
			HDMIRX_LOG("[HDMI RX][HDCP] _TxKsvlist[%x][1~5] = 0x%x,0x%x,0x%x,0x%x,0x%x\n",j,_TxKsvList[j*5],_TxKsvList[j*5+1],_TxKsvList[j*5+2],_TxKsvList[j*5+3],_TxKsvList[j*5+4]) ;
	    }
    vTxSetRxHdcpAuthDone(TRUE);
}

BOOL fgTxAuthDone(void)
{
    return _fgTxHDCPAuthDone;
}

BOOL fgRxIsHdmiMode(void)
{
    return(vHalCheckRxIsHdmiMode());
}

void RxAuthStartInt(void)
{
    vSetRxHdcpStatus(RxHDCP_UnAuthenticated);
    RxUse27M();
    vHalClearKsvReadyBit();
}


//=============================================================================
void	RxMergeKSVList(void)
{
    BYTE	i,j;
    BYTE bTxDownStreamCount;
    bTxDownStreamCount = _TxDownStreamCount;
    if(bTxDownStreamCount > 9)
    {
        bTxDownStreamCount = 9;
    }
    for(i=0;i<(RX_MAX_KSV_COUNT*5);i++)
    {
        _RxKsvList[i]=0;
    }

    for(j=0;j<bTxDownStreamCount;j++)
        for(i=0;i<5;i++)
        {
            _RxKsvList[j*5+i]=_TxKsvList[j*5+i];

        }
        HDMIRX_LOG((" [HDMI RX][HDCP]RX Merge TX KSV List() [0]="));
        for(j=0;j<bTxDownStreamCount;j++)
            for(i=0;i<5;i++)
                HDMIRX_LOG("0x%x ",_RxKsvList[j*5+i]);

        printk(("\n"));
}
//=============================================================================
void RxMergeBKSV(void)
{
    int i;
    BYTE bTxDownStreamCount;
    bTxDownStreamCount = _TxDownStreamCount;
    if(bTxDownStreamCount > 9)
    {
        bTxDownStreamCount = 9;
    }
    for(i=0;i<5;i++)
    {
        _RxKsvList[bTxDownStreamCount*5+i]=_TxBKsv[i];
    }
    HDMIRX_LOG("[HDMI RX][HDCP]RX Merge TX BKSV () [0]=");
    for(i=0;i<5;i++)
        HDMIRX_LOG("0x%x ",_RxKsvList[bTxDownStreamCount*5+i]);
    HDMIRX_LOG(("\n"));
}

void RxMergeBstatus(void)
{
    BYTE bTxDepth;
    _RxBstatus = 0;

    _RxDownStreamCount = _TxDownStreamCount+1;
    _RxBstatus |= (_RxDownStreamCount & DEVICE_COUNT) ;
    HDMIRX_LOG(" [HDMI RX][HDCP]_RxDownStreamCount = 0x%x\n",_RxDownStreamCount);
    HDMIRX_LOG(" [HDMI RX][HDCP]_TxDownStreamCount = 0x%x\n",_TxDownStreamCount);
    HDMIRX_LOG(" [HDMI RX][HDCP]_TxBStatus = 0x%x\n",_TxBStatus);


    if((_TxBStatus&MAX_DEVS_EXCEEDED)||(_RxDownStreamCount > RX_MAX_KSV_COUNT ))
        _RxBstatus |=  MAX_DEVS_EXCEEDED;

    bTxDepth = (_TxBStatus & DEVICE_DEPTH)>>8;


    if(_fgUseModifiedDepth)
        _RxBstatus |= ((_u1ModifiedDepth<<8)&DEVICE_DEPTH);
    else
        _RxBstatus |= (((bTxDepth + 1)<<8)&DEVICE_DEPTH);


    if(_fgUseModifiedDepth)
    {
        if(_u1ModifiedDepth >=7)
            _RxBstatus|=MAX_CASCADE_EXCEEDED;
    }
    else
    {
        if((_TxBStatus&MAX_CASCADE_EXCEEDED)||(bTxDepth>=7))
            _RxBstatus|=MAX_CASCADE_EXCEEDED;
    }
    if(vHalCheckRxIsHdmiMode())
        _RxBstatus |= HDMI_MODE;

    HDMIRX_LOG(" [HDMI RX][HDCP]_RxBstatus = 0x%x \n",_RxBstatus);
    HDMIRX_LOG(" [HDMI RX][HDCP]_fgUseModifiedDepth = 0x%x \n",_fgUseModifiedDepth);
    HDMIRX_LOG(" [HDMI RX][HDCP]_u1ModifiedDepth = 0x%x \n",_u1ModifiedDepth);


}

void vHDMIRxGenV(void)
{
    UINT32 u4Addr;
    UINT8 i;
    RxMergeKSVList();
    RxMergeBKSV();
    RxMergeBstatus();
    vHalSetSHALength(_RxDownStreamCount*5);
    vHalSetBstatus(_RxBstatus);
    HDMIRX_LOG(" [HDMI RX][HDCP]_RxBstatus = 0x%x\n",_RxBstatus);
    for(i=0; i<5; i++)
    {
        vHalRptStartAddrClr();
        vHalClearKsvReadyBit();
        vHalSetKsvStop(TRUE);
        vHalWriteKsvList(_RxKsvList, _RxDownStreamCount);
        vHalSetKsvStop(FALSE);
        u4Addr = vHalGetKsvFifoAddr();
        HDMIRX_LOG(" [HDMI RX][HDCP]KSV u4Addr = %d, i = %d\n",u4Addr,i);
        if(u4Addr == _RxDownStreamCount*5)
            break;
    }
    vHalRptStartAddrClr();
    vHalTriggerSHA();
}

void vHDMIRxLoadHdcpKey(void)
{
    vHalLoadHdcpKey(pdInternalRxHdcpKey);
}

void vSetRxHdcpStatus(RxHDCPStateType bStatus)
{
    _RxHDCPState = bStatus;
    HDMIRX_LOG("[HDMI RX][HDCP]Set RX HDCP STATUS = %s\n",cRxHdcpStatus[_RxHDCPState]);
}



void vHDMIRxHdcpService(void)
{
    BOOL fgAVMUTE ;
    UINT8 _bNewHDMIRxHDCPStatus;

    fgAVMUTE = vHalCheckGcpMuteEnable();

    if((_bRxHDCPMode == 3)||(_bRxHDCPMode == 5))//only for debug mode
        return;

	if(!fgIsHdmiRepeater())
        return;

    _bNewHDMIRxHDCPStatus = bHDMIHDCPStatusGet();
    if(_bHDMIRxHDCPStatus != _bNewHDMIRxHDCPStatus)
    {
        HDMIRX_LOG("[HDMI RX][HDCP]_bHDMIRxHDCPStatus=%d,_bNewHDMIRxHDCPStatus=%d,fgAVMUTE=%d \n",_bHDMIRxHDCPStatus,_bNewHDMIRxHDCPStatus,fgAVMUTE );
        _bHDMIRxHDCPStatus = _bNewHDMIRxHDCPStatus;
        if(!_bNewHDMIRxHDCPStatus)
        {
            if(!fgAVMUTE)
            {
                HDMIRX_LOG("[HDMI RX][HDCP] Upstream disable Enc,notify Tx disable HDCP\n" );
                RxAuthStartInt();
            }
        }
    }

    if(!u1HalChkDataEnableExist())
    {
        if(fgHalHdcpHdmiMode())
        {
            HDMIRX_LOG("[HDMI RX][HDCP] No detect Data Enable signal,Reset Rx Hdcp\n" );
            RxAuthStartInt();
            vHalSwResetHdmiRxModule();
        }
        return;
    }

    if(_bHDCPMode == HDCP_RECEIVER)
    {
        {
            if(fgHalHdcpAuthenticationStart())
            {
                vHalClearHdcpAuthenticationStartStatus();
                vGetRxAKsv(_RxAKSVShadow);
                vGetRxBKsv(_RxBKSVShadow);
                vGetRxAn(_RxAnShadow);
                HDMIRX_LOG("[HDMI RX][HDCP][SINK MODE] Rx Auth Start\n");
                HDMIRX_LOG("[HDMI RX][HDCP][SINK MODE] Rx AKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxAKSVShadow[0],_RxAKSVShadow[1],_RxAKSVShadow[2],_RxAKSVShadow[3],_RxAKSVShadow[4]);
                HDMIRX_LOG("[HDMI RX][HDCP][SINK MODE] Rx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxBKSVShadow[0],_RxBKSVShadow[1],_RxBKSVShadow[2],_RxBKSVShadow[3],_RxBKSVShadow[4]);
                HDMIRX_LOG("[HDMI RX][HDCP][SINK MODE] Rx AN[1~8]=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxAnShadow[0],_RxAnShadow[1],_RxAnShadow[2],_RxAnShadow[3],_RxAnShadow[4],_RxAnShadow[5],_RxAnShadow[6],_RxAnShadow[7]);
            }
            _u2RxRiShadow = vGetRxRi();
            if(_u2RxRiShadow!=_u2RxRiShadowOld)
            {
                _u2RxRiShadowOld = _u2RxRiShadow;
                HDMIRX_LOG("[HDMI RX][HDCP][SINK MODE] Rx Ri =0x%x\n",_u2RxRiShadow);
            }
        }
        return;
    }

    if(fgHalHdcpAuthenticationStart())
    {
        vHalClearHdcpAuthenticationStartStatus();
        vGetRxAKsv(_RxAKSVShadow);
        vGetRxBKsv(_RxBKSVShadow);
        vGetRxAn(_RxAnShadow);
        HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] Rx Auth Start\n");
        HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] Rx AKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxAKSVShadow[0],_RxAKSVShadow[1],_RxAKSVShadow[2],_RxAKSVShadow[3],_RxAKSVShadow[4]);
        HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] Rx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxBKSVShadow[0],_RxBKSVShadow[1],_RxBKSVShadow[2],_RxBKSVShadow[3],_RxBKSVShadow[4]);
        HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] Rx AN[1~8]=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxAnShadow[0],_RxAnShadow[1],_RxAnShadow[2],_RxAnShadow[3],_RxAnShadow[4],_RxAnShadow[5],_RxAnShadow[6],_RxAnShadow[7]);
        if(fgHalHdcpAuthenticationStart())
        {
            HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP START STATUS FAIL\n");
        }
        else
        {
            HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP START STATUS SUCCESS\n");
        }
        vTxSetRxHdcpAuthDone(FALSE);
        vSetRxHdcpStatus(RxHDCP_Computations);
    }

    switch(_RxHDCPState)
    {
    case RxHDCP_UnAuthenticated:
        break;
    case RxHDCP_Computations:
        if(fgHalHdcpAuthenticationDone())
        {
            vHalClearHdcpAuthenticationDoneStatus();
            _u2RxRiShadow = vGetRxRi();
            _u2RxRiShadowOld = _u2RxRiShadow;
            HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] Rx Ri =0x%x\n",_u2RxRiShadow);
            if(fgHalHdcpAuthenticationDone())
            {
                HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP DONE STATUE FAIL\n");
            }
            else
            {
                HDMIRX_LOG("[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP DONE STATUE SUCCESS\n");
            }
            vSetRxHdcpStatus(RxHDCP_WaitforDownstream);
            _wRxWaitTxKsvListCount = 0;
        }
        break;
    case RxHDCP_WaitforDownstream:
        _wRxWaitTxKsvListCount++;
        if(_wRxWaitTxKsvListCount > 250)//timer is about 80ms
        {
            RxAuthStartInt();
            break;
        }
        if(fgTxAuthDone())
        {
            vSetRxHdcpStatus(RxHDCP_AssembleKSVList);
        }
        break;
    case RxHDCP_AssembleKSVList:
        vHDMIRxGenV();
        vSetRxHdcpStatus(RxHDCP_WaitVReady);
        break;
    case RxHDCP_WaitVReady:
        if(fgHalIsVReady())
        {
            if(_fgTxVMatch)
            {
                vHalSetKsvReadyBit();
            }
            vSetRxHdcpStatus(RxHDCP_Authenticated);
        }
        break;
    case RxHDCP_Authenticated:
        _u2RxRiShadow = vGetRxRi();
        if(_u2RxRiShadow!=_u2RxRiShadowOld)
        {
            _u2RxRiShadowOld = _u2RxRiShadow;
            printk("[HDMI RX][HDCP][REPEATER MODE] Rx Ri =0x%x\n",_u2RxRiShadow);
        }
        break;
    default:
        break;
    }
}

void RxHdcpMode(UINT8 u1Mode)
{
    _bRxHDCPMode = u1Mode;
    if(_bRxHDCPMode == 1)
        RxHDCPSetReceiver();
    else if(_bRxHDCPMode == 2)
        RxHDCPSetRepeater();
}

BOOL fgUpStreamNeedAuth(void)
{
    if(_RxHDCPState == RxHDCP_UnAuthenticated)
        return(FALSE);
    else
        return(TRUE);
}
extern unsigned int  vinproccnt;
extern bool bDQ;

void vShowRxHpdRsenStatus(void)
{
    if(u1HalChkPwr5VExist() != 0)
        printk("[HDMI RX]Power 5V On  \n");
    else
        printk("[HDMI RX]Power 5V Off  \n");
	printk("vinproccnt = %d,bDQ = %d\n",vinproccnt,bDQ);
	
}

void vShowRxEDIDStatus(void)
{
    printk("==========================[HDMI RX]EDID1 start==================== \n");
    vDumpHdmiRxEdid(0);
    printk("==========================[HDMI RX]EDID1 end====================== \n \n \n");
    printk("==========================[HDMI RX]EDID2 start==================== \n");
    vDumpHdmiRxEdid(1);
    printk("==========================[HDMI RX]EDID2 end====================== \n");
}

void vShowRxHdcpStatus(void)
{
    if(_fgTxHDCPAuthDone)
    {
        printk("[HDMI RX]HDCP Auth\n");
    }
    else
    {
        printk("[HDMI RX]HDCP Not Auth\n");
    }

}

void vShowRxHDCPBstatus(void)
{
    UINT16 u2Temp = 0;
    if(_bHDCPMode == HDCP_REPEATER)
    {
        printk("[HDMI RX] Bstatus = 0x%x \n",_RxBstatus);
        if(_RxBstatus&(0x1<<12))
            printk("[HDMI RX] HDMI MODE = 1 \n");
        else
            printk("[HDMI RX] HDMI MODE = 0 \n");

        if(_RxBstatus&(0x1<<11))
            printk("[HDMI RX] MAX_CASCADE_EXCEEDED = 1 \n");
        else
            printk("[HDMI RX] MAX_CASCADE_EXCEEDED = 0 \n");

        u2Temp = (_RxBstatus>>8)&(0x7);
        printk("[HDMI RX] DEPTH = %d \n",u2Temp);

        if(_RxBstatus&(0x1<<7))
            printk("[HDMI RX] MAX_DEVS_EXCEEDED = 1 \n");
        else
            printk("[HDMI RX] MAX_DEVS_EXCEEDED = 0 \n");

        u2Temp = _RxBstatus & 0x7F;
        printk("[HDMI RX] DEVICE_COUNT = %d \n",u2Temp);

    }
    else
    {
        printk("[HDMI RX] A connected device is only Sink!!\n");
    }

}

void vShowRxSynDetStatus(void)
{
    if(u1HalChkDataEnableExist() == 1)
    {
        printk("[HDMI RX] Detceted Sync\n");
    }
    else
    {
        printk("[HDMI RX] No Detceted Sync\n");
    }

}

void vShowRxHDMIModeStatus(void)
{
    if(vHalCheckRxIsHdmiMode())
    {
        printk("[HDMI RX]RX RECEIVER HDMI \n");
    }
    else
    {
        printk("[HDMI RX]RX RECEIVER DVI  \n");
    }

}

void vShowRxColorSpaceStatus(void)
{
    if(bHDMIInputType()==0x0) // YCbCr
    {
        if(bHDMI422Input() == 0)	// 4:4:4
        {
            printk("[HDMI RX]Color Space = YCbCr444 \n");
        }
        else if(bHDMI422Input() == 1)
        {
            printk("[HDMI RX]Color Space = YCbCr422 \n");
        }
        else
        {
            printk("[HDMI RX]Color Space = YCbCr420 \n");
        }
    }
    else //RGB
    {
        printk("[HDMI RX]Color Space = RGB \n");
    }
}

void vShowRxDeepColorStatus(void)
{
    if(bHDMIDeepColorStatus()== RX_NON_DEEP)
    {
        printk("[HDMI RX]DEEP COLOR = 24 BIT \n");
    }
    else if(bHDMIDeepColorStatus()== RX_30BITS_DEEP_COLOR)
    {
        printk("[HDMI RX]DEEP COLOR = 30 BIT \n");
    }
    else if(bHDMIDeepColorStatus()== RX_36BITS_DEEP_COLOR)
    {
        printk("[HDMI RX]DEEP COLOR = 36 BIT \n");
    }
    else if(bHDMIDeepColorStatus()== RX_48BITS_DEEP_COLOR)
    {
        printk("[HDMI RX]DEEP COLOR = 48 BIT \n");
    }
}

void vShowRx3DStatus(void)
{
    if ((_RxPacket[VENDOR_INFOFRAME].fgValid) && (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_HB0]== VS_INFOFRAME_HEADER))
    {
        if ((_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB4]>>5)==_3D_FORMAT_PRESENT )
        {
            _3DInfo.HDMI_3D_Enable = 1;
            _3DInfo.HDMI_3D_Video_Format = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB4]>>5);
            _3DInfo.HDMI_3D_Structure = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB5]) >> 4;
            if (_3DInfo.HDMI_3D_Structure >= HDMI_3D_Structure_SideBySideHalf)
            {
                _3DInfo.HDMI_3D_EXTDATA = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB6]) >> 4;
            }
            else
            {
                _3DInfo.HDMI_3D_EXTDATA = 0;
            }
        }
        else
        {
            _3DInfo.HDMI_3D_Enable = 0;
        }
    }
    else
    {
        _3DInfo.HDMI_3D_Enable = 0;
    }
    if (_3DInfo.HDMI_3D_Enable)
    {
        printk("[HDMI RX]3D present\n");
        printk("[HDMI RX]3D Format= %d\n",_3DInfo.HDMI_3D_Video_Format);
        printk("[HDMI RX]3D Structure= %s\n", c3DStructure[_3DInfo.HDMI_3D_Structure]);
        printk("[HDMI RX]3D ExtData= %d\n" ,_3DInfo.HDMI_3D_EXTDATA);
    }
    else
    {
        printk("[HDMI RX]No 3D present\n");
    }

}

void vShowRxInputStatus(void)
{
    if(_bHDMICurrSwitch == HDMI_SWITCH_1)
        printk("[HDMI RX]Input Status: HDMI In 1 MODE \n");
}

void vShowRxNCTSStatus(void)
{
    UINT32 wCTS_HW,wN_HW;

    wCTS_HW= u4HalGetRxHwCTSValue();
    wN_HW= u4HalGetRxHwNValue();
    printk("[HDMI RX]N = 0x%x \n",wN_HW);
    printk("[HDMI RX]CTS = 0x%x \n" ,wCTS_HW);

}

void vShowAVMuteStatus(void)
{
    if(_HdmiAvMuteShalow == TRUE)
    {
        printk("[HDMI RX]SET_AVMUTE \n");
    }
    else
    {
        printk("[HDMI RX]Clear_AVMUTE \n");
    }

}

void vHdmiRxHdcpStatus(void)
{
    BYTE i;
    BYTE _RxAKSV[5];
    BYTE _RxBKSV[5];
    BYTE _RxAn[8];

    printk("[HDMI RX]HDCP STATUS \n");
    if(_bHDCPMode == HDCP_RECEIVER)
        printk("[HDMI RX]HDCP RECEIVER MODE\n");
    else
        printk("[HDMI RX]HDCP REPEATER MODE\n");

    printk("[HDMI RX]RX HDCP STATUS = %s\n",cRxHdcpStatus[_RxHDCPState]);
    vGetRxAKsv(_RxAKSV);
    vGetRxBKsv(_RxBKSV);
    vGetRxAn(_RxAn);
    printk("[HDMI RX][HDCP] Rx AKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxAKSV[0],_RxAKSV[1],_RxAKSV[2],_RxAKSV[3],_RxAKSV[4]);
    printk("[HDMI RX][HDCP] Rx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxBKSV[0],_RxBKSV[1],_RxBKSV[2],_RxBKSV[3],_RxBKSV[4]);
    printk("[HDMI RX][HDCP] Rx AN[1~8]=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxAn[0],_RxAn[1],_RxAn[2],_RxAn[3],_RxAn[4],_RxAn[5],_RxAn[6],_RxAn[7]);

    printk("[HDMI TX][HDCP] _TxDownStreamCount = 0x%x\n",_TxDownStreamCount);
    printk("[HDMI TX][HDCP] _TxBStatus = 0x%x\n",_TxBStatus);
    printk("[HDMI TX][HDCP] Tx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",_TxBKsv[0],_TxBKsv[1],_TxBKsv[2],_TxBKsv[3],_TxBKsv[4]);
    for(i=0;i<_TxDownStreamCount;i++)
    {
	  if(i < 9)
        printk("[HDMI TX] Tx KSVLIST[%d][1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",i,_TxKsvList[5*i],_TxKsvList[5*i+1],_TxKsvList[5*i+2],_TxKsvList[5*i+3],_TxKsvList[5*i+4]);
    }
    if(_fgTxHDCPAuthDone)
    {
        printk("[HDMI TX]HDCP Auth\n");
    }
    else
    {
        printk("[HDMI TX]HDCP Not Auth\n");
    }

    printk("[HDMI RX][HDCP] _RxDownStreamCount = 0x%x\n",_RxDownStreamCount);
    printk("[HDMI RX][HDCP] _RxBStatus = 0x%x\n",_RxBstatus);
    for(i=0;i<_RxDownStreamCount;i++)
    {
	  if(i < 10)
        printk("[HDMI RX] Rx KSVLIST[%d][1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",i,_RxKsvList[5*i],_RxKsvList[5*i+1],_RxKsvList[5*i+2],_RxKsvList[5*i+3],_RxKsvList[5*i+4]);
    }
    printk("[HDMI RX][HDCP] RxHdcpKey addr = 0x%x\n",&RxHdcpKey);

}

void vHdmiRxHdmiStatus(void)
{
    UINT32 wCTS_HW,wN_HW;


    if(fgHdmiRepeaterIsBypassMode())
    {
        printk("[HDMI RX]HDMI INTERNAL REPEATER MODE:VIDEO BYPASS MODE \n");
    }
    else if(fgHdmiRepeaterIsDramMode())
    {
        printk("[HDMI RX]HDMI INTERNAL REPEATER MODE:VIDEO DRAM MODE \n");
    }
    else
    {
        printk("[HDMI RX]BDP PLAYER MODE \n");
    }

    if(vHalCheckRxIsHdmiMode())
    {
        printk("[HDMI RX]RX RECEIVER HDMI \n");
    }
    else
    {
        printk("[HDMI RX]RX RECEIVER DVI  \n");
    }

    if(bHDMIDeepColorStatus()== RX_NON_DEEP)
    {
        printk("[HDMI RX]DEEP COLOR = 24 BIT \n");
    }
    else if(bHDMIDeepColorStatus()== RX_30BITS_DEEP_COLOR)
    {
        printk("[HDMI RX]DEEP COLOR = 30 BIT \n");
    }
    else if(bHDMIDeepColorStatus()== RX_36BITS_DEEP_COLOR)
    {
        printk("[HDMI RX]DEEP COLOR = 36 BIT \n");
    }
    else if(bHDMIDeepColorStatus()== RX_48BITS_DEEP_COLOR)
    {
        printk("[HDMI RX]DEEP COLOR = 48 BIT \n");
    }

    if(bHDMIInputType()==0x0) // YCbCr
    {
        if(bHDMI422Input()== 0)  // 4:4:4
        {
            printk("[HDMI RX]Color Space = YCbCr444 \n");
        }
        else if(bHDMI422Input()== 1)
        {
            printk("[HDMI RX]Color Space = YCbCr422 \n");
        }
        else
        {
            printk("[HDMI RX]Color Space = YCbCr420 \n");
        }
    }
    else //RGB
    {
        printk("[HDMI RX]Color Space = RGB \n");
    }

    if(_HdmiAvMuteShalow == TRUE)
    {
        printk("[HDMI RX]SET_AVMUTE \n");
    }
    else
    {
        printk("[HDMI RX]Clear_AVMUTE \n");
    }

    printk("[HDMI RX]HDMI RX STATUS \n");
    printk("[HDMI RX]_bHDMIState = %s\n",cHDMIState[_bHDMIState]);
    printk("_bHdmiRepeaterMode = %d, _bAppHdmiRepeaterMode = %d\n",_bHdmiRepeaterMode,_bAppHdmiRepeaterMode);
    printk("_bHDMICurrSwitch = %d, _bAppHDMICurrSwitch = %d\n",_bHDMICurrSwitch,_bAppHDMICurrSwitch);
    printk("[HDMI RX]_u1RxSysState = %d \n",_u1RxSysState);


    if(_bHdmiRepeaterMode == HDMI_REPEATER_VIDEO_BYPASS_MODE)
    {
        printk("[HDMI RX] HDMI_REPEATER_VIDEO_BYPASS_MODE\n");
    }
    else if(_bHdmiRepeaterMode == HDMI_REPEATER_VIDEO_DRAM_MODE)
    {
        printk("[HDMI RX] HDMI_REPEATER_VIDEO_DRAM_MODE\n");
    }
    else if(_bHdmiRepeaterMode == HDMI_SOURCE_MODE)
    {
        printk("[HDMI RX] HDMI_SOURCE_MODE\n");
    }
    else
    {
        printk("[HDMI RX] HDMI_MODE error\n");
    }
    printk("[HDMI RX]_u1TxEdidReady = %d, _bHdmiAudioOutputMode = %d\n",_u1TxEdidReady,_bHdmiAudioOutputMode);

    {
        if(_bHdmiAudioOutputMode)
        {
            printk("[HDMI RX]HDMI RX AUDIO OUTPUT MODE:HDMI(as User Select HDMI in GUI)\n");
            printk("[HDMI]HDMI internal MODE:BYPASS MODE \n");
        }
        else
        {
            printk("[HDMI RX]HDMI RX AUDIO OUTPUT MODE:Speaker or Speaker+HDMI(as User Select Speaker or Speaker+HDMI in GUI)\n");
            printk("[HDMI]HDMI internal MODE:DRAM MODE \n");
        }
    }


    wCTS_HW= u4HalGetRxHwCTSValue();
    wN_HW= u4HalGetRxHwNValue();
    printk("[HDMI RX]N = %x \n",wN_HW);
    printk("[HDMI RX]CTS = %x \n" ,wCTS_HW);

    if(IS_AUD_MUTE())
    {
        printk("[HDMI RX]HDMI RX AUDIO MUTE \n");
    }
    else
    {
        printk("[HDMI RX]HDMI RX AUDIO UNMUTE \n");
    }


    vShowAviInforFrame();
    vShowAudioInforFrame();
    vShowSPDInforFrame();
    vShowGamutInforFrame();

}

void vHdmiRxStatus(void)
{
    vHdmiRxHdmiStatus();
    vHdmiRxDviStatus();
    vHdmiRxHdcpStatus();
}

void vShowHDMIRxStatus(void)
{
    
    vShowRxHpdRsenStatus();
    vShowRxEDIDStatus();
    vShowRxHdcpStatus();
    vShowRxSynDetStatus();
    vShowRxResoInfoStatus();
    vShowRxHDMIModeStatus();
    vShowRxColorSpaceStatus();
    vShowRxDeepColorStatus();
    vShowRx3DStatus();
    vShowRxInputStatus();
    vShowRxNCTSStatus();
    vShowAVMuteStatus();
	
    //vShowAudioStatus();

}

void vShowHDMIRxInfo(void)
{
    vShowAviInforFrame();
    vShowAudioInforFrame();
    vShowSPDInforFrame();
    vShowMPEGInforFrame();
    vShowACPInforFrame();
    vShowISRC1InforFrame();
    vShowISRC2InforFrame();
    vShowVENDInforFrame();
    vShowGamutInforFrame();
	vShowGCPInforFrame();
	vShowSACDInforFrame();
}

void vHdmiRxPacketDataInit(void)
{
    BYTE i,j;
    unsigned long LastestTime;
    Linux_HAL_GetTime(&LastestTime);
    for(i=0;i<MAX_PACKET;i++)
    {
        _RxPacket[i].fgValid= 0;
        _RxPacket[i].fgChanged= 0;
        _RxPacket[i].u4LastReceivedTime= LastestTime;
        for(j=0;j<31;j++)
        {
            _RxPacket[i].PacketData[j]=0;
        }
    }

    _RxPacket[AVI_INFOFRAME].PacketHeader= AVI_INFOFRAME_HEADER;
    _RxPacket[AUDIO_INFOFRAME].PacketHeader= AUDIO_INFOFRAME_HEADER;
    _RxPacket[MPEG_INFOFRAME].PacketHeader= MPEG_INFOFRAME_HEADER;
    _RxPacket[SPD_INFOFRAME].PacketHeader= SPD_INFOFRAME_HEADER;
    _RxPacket[VENDOR_INFOFRAME].PacketHeader= VS_INFOFRAME_HEADER;
    _RxPacket[ACP_PACKET].PacketHeader= ACP_PACKET_HEADER;
    _RxPacket[ISRC1_PACKET].PacketHeader= ISRC1_PACKET_HEADER;
    _RxPacket[ISRC2_PACKET].PacketHeader= ISRC2_PACKET_HEADER;
    _RxPacket[GAMUT_PACKET].PacketHeader= GAMUT_PACKET_HEADER;

    _RxPacket[AVI_INFOFRAME].u4timeout= 100;//100ms
    _RxPacket[AUDIO_INFOFRAME].u4timeout= 100;
    _RxPacket[MPEG_INFOFRAME].u4timeout= 100;
    _RxPacket[SPD_INFOFRAME].u4timeout= 2000;
    _RxPacket[VENDOR_INFOFRAME].u4timeout= 100;
    _RxPacket[ACP_PACKET].u4timeout= 600;//600ms
    _RxPacket[ISRC1_PACKET].u4timeout= 300;//300ms
    _RxPacket[ISRC2_PACKET].u4timeout= 300;
    _RxPacket[GAMUT_PACKET].u4timeout= 100;

    _RxPacket[AVI_INFOFRAME].bLength= 14;//include checksum
    _RxPacket[AUDIO_INFOFRAME].bLength= 11;//include checksum
    _RxPacket[MPEG_INFOFRAME].bLength= 11;//include checksum
    _RxPacket[SPD_INFOFRAME].bLength= 26;//include checksum
    _RxPacket[VENDOR_INFOFRAME].bLength= 28;
    _RxPacket[ACP_PACKET].bLength= 28;
    _RxPacket[ISRC1_PACKET].bLength= 28;
    _RxPacket[ISRC2_PACKET].bLength= 28;
    _RxPacket[GAMUT_PACKET].bLength= 28;

	vSetSelectUnRecpacket(TRUE,ISRC2_PACKET_HEADER);
	vHalSetVSNewOnly(FALSE);
	vHalSetISRC1NewOnly(FALSE);
	vHalMpegAddrSetSelectPacket(MPEG_INFOFRAME_HEADER);
	//vNotifyHDMIRxACPTypeChange(ACP_LOST_DISABLE);

	_HdmiAvMuteShalow = FALSE;
}

BOOL fgIsrc1PacketValidIsSet(void)
{
    return(_RxPacket[ISRC1_PACKET].PacketData[1] & 0x40);
}

BOOL fgIsrc1PacketContIsSet(void)
{
    return(_RxPacket[ISRC1_PACKET].PacketData[1] & 0x80);
}

BOOL fgChecksumOk(BYTE bType)
{
    BYTE i;
    BYTE bData = 0;
    for(i=3;i<31;i++)
    {
        bData +=_RxPacket[bType].PacketData[i];
    }
    if(bData)
    {
        HDMIRX_LOG( "[HDMI RX]%s CHECKSUM FALSE\n",cHDMIPacketName[bType]);
        return(FALSE);
    }
    else
    {
        HDMIRX_LOG( "[HDMI RX]%s CHECKSUM OK\n",cHDMIPacketName[bType]);
        return(TRUE);
    }

}

void vHdmiRxGetPacketData(void)
{
    BYTE i,j;
    BYTE bIndex;
    BYTE bcount;
    BYTE bBuffer1[31];
    BYTE bBuffer2[31];
    BYTE *prData;
	unsigned char bData=0xff;
    unsigned long CurrTime;
    Linux_HAL_GetTime(&CurrTime);

    bIndex = AVI_INFOFRAME;
    if(fgHalIsINTR3_NEW_AVI())
    {
        vHalClearNewAviIntStatus();
        vHalGetAviInfoframe(bBuffer1);
        vHalGetAviInfoframe(bBuffer2);
        _RxPacket[bIndex].u4LastReceivedTime = CurrTime;
        bcount = 0;
        for(i=0;i<19;i++)
        {
            if(bBuffer1[i] == bBuffer2[i])
                bcount++;
        }
        if(bcount == 19)
        {
            prData = &_RxPacket[bIndex].PacketData[0];
            bcount = 0;
            for(i=0;i<19;i++)
            {
                if(*(prData+i)==bBuffer2[i])
                {
                    bcount++;
                }
                *(prData+i)=bBuffer2[i];
            }
            _RxPacket[bIndex].fgValid = TRUE;
            if(bcount!=19)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
        }

    }
    else
    {
        if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
        {
            if(_RxPacket[bIndex].fgValid == TRUE)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
            _RxPacket[bIndex].fgValid = FALSE;

        }
    }

    bIndex = AUDIO_INFOFRAME;

    if(fgHalIsINTR3_NEW_AUD())
    {
        vHalClearNewAudIntStatus();
        vHalGetAudioInfoframe(bBuffer1);
        vHalGetAudioInfoframe(bBuffer2);
        _RxPacket[bIndex].u4LastReceivedTime = CurrTime;
        bcount = 0;
        for(i=0;i<14;i++)
        {
            if(bBuffer1[i] == bBuffer2[i])
                bcount++;
        }
        if(bcount == 14)
        {
            prData = &_RxPacket[bIndex].PacketData[0];
            bcount = 0;
            for(i=0;i<14;i++)
            {
                if(*(prData+i)==bBuffer2[i])
                {
                    bcount++;
                }
                *(prData+i)=bBuffer2[i];
            }
            _RxPacket[bIndex].fgValid = TRUE;
            if(bcount!=14)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
        }

    }
    else
    {
        if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
        {
            if(_RxPacket[bIndex].fgValid == TRUE)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
            _RxPacket[bIndex].fgValid = FALSE;

        }
    }

    bIndex=SPD_INFOFRAME;

    if(fgHalIsINTR3_NEW_SPD())
    {
        vHalClearNewSpdIntStatus();
        vHalGetSpdInfoframe(bBuffer1);
        vHalGetSpdInfoframe(bBuffer2);
        _RxPacket[bIndex].u4LastReceivedTime = CurrTime;
        bcount = 0;
        for(i=0;i<31;i++)
        {
            if(bBuffer1[i] == bBuffer2[i])
                bcount++;
        }
        if(bcount == 31)
        {
            prData = &_RxPacket[bIndex].PacketData[0];
            bcount = 0;
            for(i=0;i<31;i++)
            {
                if(*(prData+i)==bBuffer2[i])
                {
                    bcount++;
                }
                *(prData+i)=bBuffer2[i];
            }
            _RxPacket[bIndex].fgValid = TRUE;
            if(bcount!=31)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
        }

    }
    else
    {
        if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
        {
            if(_RxPacket[bIndex].fgValid == TRUE)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
            _RxPacket[bIndex].fgValid = FALSE;

        }
    }

    bIndex = VENDOR_INFOFRAME;
if(fgHalIsINTR_NEW_VS())
{
	vHalClearNewVSIntStatus();
	vHalGetVSInfoframe(bBuffer1);
	vHalGetVSInfoframe(bBuffer2);
        _RxPacket[bIndex].u4LastReceivedTime = CurrTime;
	bcount = 0;
	for(i=0;i<31;i++)
	{
	  if(bBuffer1[i] == bBuffer2[i])
		bcount++;
	}
	if(bcount == 31)
	{
	  prData = &_RxPacket[bIndex].PacketData[0];
	  bcount = 0;
	  for(i=0;i<31;i++)
	  {
		if(*(prData+i)==bBuffer2[i])
		{
		  bcount++;
		}
		*(prData+i)=bBuffer2[i];
	  }
	  _RxPacket[bIndex].fgValid = TRUE;
	  if(bcount!=31)
	  {
		_RxPacket[bIndex].fgChanged = TRUE;
	  }
	}

}
else
{
  if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
  {
	if(_RxPacket[bIndex].fgValid == TRUE)
	{
		_RxPacket[bIndex].fgChanged = TRUE;
	}
	_RxPacket[bIndex].fgValid = FALSE;

  }
}
    bIndex = GAMUT_PACKET;

    if(fgHalIsINTR7_GAMUT())
    {
        fgHalClearGamutIntStatus();
        vHalGetGamutPacket(bBuffer1);
        vHalGetGamutPacket(bBuffer2);
        _RxPacket[bIndex].u4LastReceivedTime = CurrTime;
        bcount = 0;
        for(i=0;i<31;i++)
        {
            if(bBuffer1[i] == bBuffer2[i])
                bcount++;
        }
        if(bcount == 31)
        {
            prData = &_RxPacket[bIndex].PacketData[0];
            bcount = 0;
            for(i=0;i<31;i++)
            {
                if(*(prData+i)==bBuffer2[i])
                {
                    bcount++;
                }
                *(prData+i)=bBuffer2[i];
            }
            _RxPacket[bIndex].fgValid = TRUE;
            if(bcount!=31)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
        }

    }
    else
    {
        if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
        {
            if(_RxPacket[bIndex].fgValid == TRUE)
            {
                _RxPacket[bIndex].fgChanged = TRUE;
            }
            _RxPacket[bIndex].fgValid = FALSE;

        }
    }

bIndex = ACP_PACKET;
if(fgHalCheckIsNewAcp())
	{
		fgHalClearAcpIntStatus();
		vHalGetAcpPacket(bBuffer1);
		vHalGetAcpPacket(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;
		for(i=0;i<31;i++)
		{
		  if(bBuffer1[i] == bBuffer2[i])
			bcount++;
		}
		if(bcount == 31)
		{
		   //2010/08/04,ychung, Check if ACP Type is changed , if yes  notify hdmi rx audio task
			if(bBuffer2[0]==ACP_PACKET_HEADER)
			{
			  if(bBuffer2[1]!=_RxPacket[bIndex].PacketData[1])
				{
					  //vNotifyHDMIRxACPTypeChange(bBuffer2[1]);
					  _RxPacket[AUDIO_INFOFRAME].fgChanged = TRUE;   // jitao@20111031 for acp type is DVD-A SACD will mute Audio Infoframe.
				}
			}
		  prData = &_RxPacket[bIndex].PacketData[0];
		  bcount = 0;
		  for(i=0;i<31;i++)
		  {
			if(*(prData+i)==bBuffer2[i])
			{
			  bcount++;
			}
			*(prData+i)=bBuffer2[i];
		  }
		  if(bcount!=31)
		  {
			_RxPacket[bIndex].fgChanged = TRUE;
		  }

		  _RxPacket[bIndex].fgValid = TRUE;
	  }

	}

	else
	{
	  if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
	  {
		if(_RxPacket[bIndex].fgValid == TRUE)
		{
			_RxPacket[bIndex].fgChanged = TRUE;
			//vNotifyHDMIRxACPTypeChange(ACP_LOST_DISABLE);
		}
		_RxPacket[bIndex].fgValid = FALSE;

	  }
	}


	bIndex = MPEG_INFOFRAME;

	if(fgHalIsINTR3_NEW_MPEG())
	{
		vHalClearNewMpegIntStatus();
		vHalGetMpegInfoframe(bBuffer1);
		vHalGetMpegInfoframe(bBuffer2);

		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;
		for(i=0;i<31;i++)
		{
		  if(bBuffer1[i] == bBuffer2[i])
			bcount++;
		}
		if(bcount == 31)
		{
		  prData = &_RxPacket[bIndex].PacketData[0];
		  bcount = 0;
		  for(i=0;i<31;i++)
		  {
			if(*(prData+i)==bBuffer2[i])
			{
			  bcount++;
			}
			*(prData+i)=bBuffer2[i];
		  }
		  _RxPacket[bIndex].fgValid = TRUE;
		  if(bcount!=31)
		  {
			_RxPacket[bIndex].fgChanged = TRUE;
		  }
		}

	}
	else
	{
	  if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
	  {
		if(_RxPacket[bIndex].fgValid == TRUE)
		{
			_RxPacket[bIndex].fgChanged = TRUE;
		}
		_RxPacket[bIndex].fgValid = FALSE;

	  }
	}


	bIndex = ISRC1_PACKET;
	if(fgHalIsINTR_NEW_ISRC1())
	{
	  vHalClearNewISRC1IntStatus();
	  vHalGetISRC1Infoframe(bBuffer1);
	  vHalGetISRC1Infoframe(bBuffer2);
	  _RxPacket[bIndex].u4LastReceivedTime = CurrTime;
	  bcount = 0;
	  for(i=0;i<31;i++)
	  {
		if(bBuffer1[i] == bBuffer2[i])
		  bcount++;
	  }
	  if(bcount == 31)
	  {
		prData = &_RxPacket[bIndex].PacketData[0];
		bcount = 0;
		for(i=0;i<31;i++)
		{
		  if(*(prData+i)==bBuffer2[i])
		  {
			bcount++;
		  }
		  *(prData+i)=bBuffer2[i];
		}
		if(bcount!=31)
		{
		  _RxPacket[bIndex].fgChanged = TRUE;
		}
		if(_RxPacket[bIndex].fgValid == FALSE)
		{
		  _RxPacket[bIndex].fgChanged = TRUE;
		}
		_RxPacket[bIndex].fgValid = TRUE;

	  }

	}
	else
	{
	 if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
	 {
		if(_RxPacket[bIndex].fgValid == TRUE)
		{
			_RxPacket[bIndex].fgChanged = TRUE;
		}
		_RxPacket[bIndex].fgValid = FALSE;

	  }
	}


	bIndex = ISRC2_PACKET;
	if(fgHalIsINTR3_NEW_UNREC())
	{
		vHalClearNewUnRecIntStatus();
		vHalGetUnRecPacket(bBuffer1);
		vHalGetUnRecPacket(bBuffer2);

	  _RxPacket[bIndex].u4LastReceivedTime = CurrTime;
	  bcount = 0;
	  for(i=0;i<31;i++)
	  {
		if(bBuffer1[i] == bBuffer2[i])
		  bcount++;
	  }
	  if(bcount == 31)
	  {
		prData = &_RxPacket[bIndex].PacketData[0];
		bcount = 0;
		for(i=0;i<31;i++)
		{
		  if(*(prData+i)==bBuffer2[i])
		  {
			bcount++;
		  }
		  *(prData+i)=bBuffer2[i];
		}
		if(bcount!=31)
		{
		  _RxPacket[bIndex].fgChanged = TRUE;
		}
		if(_RxPacket[bIndex].fgValid == FALSE)
		{
		  _RxPacket[bIndex].fgChanged = TRUE;
		}
		_RxPacket[bIndex].fgValid = TRUE;
		//printk( "[HDMI RX]ISRC2 received,change to ISRC1\n");
	  }

	}
	else
	{
	 if(Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout, &_RxPacket[bIndex].u4LastReceivedTime, &CurrTime))
	 {
		if(_RxPacket[bIndex].fgValid == TRUE)
		{
			_RxPacket[bIndex].fgChanged = TRUE;
		}
		_RxPacket[bIndex].fgValid = FALSE;

	  }
	}



    for(i=0; i<MAX_PACKET; i++)
    {
        if(_RxPacket[i].fgChanged)
        {
            _RxPacket[i].fgChanged = 0;
            if(_RxPacket[i].fgValid)
            {
                printk( "[HDMI RX]%s NEW/CHG\n",cHDMIPacketName[i]);
                printk( "HB0~HB3=0x%x,0x%x,0x%x\n",_RxPacket[i].PacketData[0],_RxPacket[i].PacketData[1],_RxPacket[i].PacketData[2]);
                printk( "PB00~HB06=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxPacket[i].PacketData[3],_RxPacket[i].PacketData[4],_RxPacket[i].PacketData[5],
                             _RxPacket[i].PacketData[6],_RxPacket[i].PacketData[7],_RxPacket[i].PacketData[8],_RxPacket[i].PacketData[9]);
                printk( "PB07~HB13=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxPacket[i].PacketData[10],_RxPacket[i].PacketData[11],_RxPacket[i].PacketData[12],_RxPacket[i].PacketData[13],
                             _RxPacket[i].PacketData[14],_RxPacket[i].PacketData[15],_RxPacket[i].PacketData[16]);
                printk( "PB14~HB20=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxPacket[i].PacketData[17],_RxPacket[i].PacketData[18],_RxPacket[i].PacketData[19],_RxPacket[i].PacketData[20],_RxPacket[i].PacketData[21],
                             _RxPacket[i].PacketData[22],_RxPacket[i].PacketData[23]);
                printk( "PB21~HB28=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_RxPacket[i].PacketData[24],_RxPacket[i].PacketData[25],_RxPacket[i].PacketData[26],_RxPacket[i].PacketData[27],_RxPacket[i].PacketData[28],
                             _RxPacket[i].PacketData[29],_RxPacket[i].PacketData[30]);
            }
            else
            {
                printk( "[HDMI RX]%s LOST/DISABLE\n",cHDMIPacketName[i]);
                for(j=0;j<31;j++)//don't clear packet header
                {
                    _RxPacket[i].PacketData[j]=0;
                }
            }
            if(fgIsHdmiRepeater())
            {
                vSendHdmiPacket(i,_RxPacket[i].fgValid, &_RxPacket[i].PacketData[0]);
				bData = HDMI_RX_INFOFRAME_NOTIFY;
				if(bData!=0xff)
					vNotifyAppHdmiRxState(bData);
            }
        }
    }

    i = vHalCheckGcpMuteEnable();
    if(_HdmiAvMuteShalow != i)
    {
        _HdmiAvMuteShalow = i;
        if(fgHdmiRepeaterIsBypassMode())
        {
            //vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_GCP, &_HdmiAvMuteShalow);
        }
    }
}

void vHdmiRxSetTxPacket(void)
{
    BYTE i;
	printk("[HDMI Rx]resend all infoframe \n");
    for(i=0; i<MAX_PACKET; i++)
    {
        vSendHdmiPacket(i,_RxPacket[i].fgValid, &_RxPacket[i].PacketData[0]);
    }
}

void vHdmiGet3DInfo(BOOL fgPrintLog)
{
    if ((_RxPacket[VENDOR_INFOFRAME].fgValid) && (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_HB0]== VS_INFOFRAME_HEADER))
    {
        if ((_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB4]>>5)==_3D_FORMAT_PRESENT )
        {
            _3DInfo.HDMI_3D_Enable = 1;
            _3DInfo.HDMI_3D_Video_Format = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB4]>>5);
            _3DInfo.HDMI_3D_Structure = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB5]) >> 4;
            if (_3DInfo.HDMI_3D_Structure >= HDMI_3D_Structure_SideBySideHalf)
            {
                _3DInfo.HDMI_3D_EXTDATA = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB6]) >> 4;
            }
            else
            {
                _3DInfo.HDMI_3D_EXTDATA = 0;
            }
        }
        else
        {
            _3DInfo.HDMI_3D_Enable = 0;
        }
    }
    else
    {
        _3DInfo.HDMI_3D_Enable = 0;
    }

	if(_u1Force3DType)
	{
      _3DInfo.HDMI_3D_Enable = 1;
	  if(_u1Force3DType ==1)
	  	_3DInfo.HDMI_3D_Structure = HDMI_3D_Structure_FramePacking;
	  else if(_u1Force3DType ==2)
	  	_3DInfo.HDMI_3D_Structure = HDMI_3D_Structure_SideBySideHalf;
	  else if(_u1Force3DType ==3)
	  	_3DInfo.HDMI_3D_Structure = HDMI_3D_Structure_TopBottom;

	}

    if (_3DInfo.HDMI_3D_Enable)
    {
        HDMIRX_LOG("[HDMI Rx]3D present\n");
        HDMIRX_LOG("[HDMI Rx]3D Format= %d\n",_3DInfo.HDMI_3D_Video_Format);
        HDMIRX_LOG("[HDMI Rx]3D Structure= %s\n", c3DStructure[_3DInfo.HDMI_3D_Structure]);
        HDMIRX_LOG("[HDMI Rx]3D ExtData= %d\n" ,_3DInfo.HDMI_3D_EXTDATA);
    }
    else
    {
        HDMIRX_LOG("[HDMI Rx]No 3D present\n");
    }

}

void vHdmiRxForce3D(UINT32 bType)
{
	_u1Force3DType = bType;
}

UINT32 u4GetRxHwCTSValue(void)
{
    UINT32 u4Data=0;
    u4Data = u4HalGetRxHwCTSValue();
    return u4Data;


}

UINT32 u4GetRxHwNValue(void)
{
    UINT32 u4Data=0;
    u4Data = u4HalGetRxHwNValue();
    return u4Data;
}


void vTriggleEdidForceUpdate(void)
{
    _u1TxEdidReadyOld = 0x3;//not 0 or 1

}

void vEdidUpdataChk(void)
{
	unsigned char bNfyData=0xff;
    UINT8 bData = 0;
    UINT32 u4Count;
   // bData = fgHDMIAudRxBypassMode();
    if((_u1TxEdidReady!= _u1TxEdidReadyOld)|| (_bHdmiAudioOutputMode!=bData))
    {
		printk( "[HDMI RX][EDID]_u1TxEdidReadyOld = %d,_bHdmiAudioOutputModeOld =%d\n",_u1TxEdidReadyOld,_bHdmiAudioOutputMode);
		printk( "[HDMI RX][EDID]_u1TxEdidReady = %d,_bHdmiAudioOutputMode =%d\n",_u1TxEdidReady,bData);
        _u1TxEdidReadyOld = _u1TxEdidReady;
        _bHdmiAudioOutputMode = bData;
        if(fgIsHdmiRepeater())
        {
            //vTmdsOnOffAndResetHdcp(FALSE);
        }

        if(_u1TxEdidReady == HDMI_PLUG_OUT)
        {
            Default_Edid_BL0_BL1_Write();
            vTxSetRxReceiverMode();
        }
        else
        {
            if (fgIsTxDetectHotPlugIn())
                vTxSetRxRepeaterMode();
            else
                vTxSetRxReceiverMode();
			
            if(_bHdmiAudioOutputMode)
            {
                vSetEdidUpdateMode(1,1);
            }
            else
            {
                vSetEdidUpdateMode(1,0);
            }
            EdidProcessing();


        }

        for(u4Count = 0; u4Count<512;u4Count++)
        {
			if(u4Count<256)
				u1HDMIINEDID[u4Count] = u1HDMIIN1EDID[u4Count];
			else
			{
				u1HDMIINEDID[u4Count] = u1HDMIIN2EDID[u4Count - 256];

				if(u4Count == 256) //  For PA address
					u1HDMIINEDID[u4Count] = _u1EDID0PAOFF+0x80;
				if(u4Count == 257) //For PA High Byte
					u1HDMIINEDID[u4Count] = (_u2EDID0PA>>8)&0xFF;//High Byte
				if(u4Count == 258) //For PA LOW Byte
					u1HDMIINEDID[u4Count] = _u2EDID0PA&0xFF;//Low Byte

			}
        }

		bNfyData = HDMI_RX_STATE_EDID_STATUS;
		if(bNfyData!=0xff)
			vNotifyAppHdmiRxState(bNfyData);


		msleep(300); // because EDID update so fast , cause hpd keep low only 1ms.
		vHDMIHPDHigh(HDMI_SWITCH_1);
		if(_bAppHdmiRepeaterMode == HDMI_SOURCE_MODE)
		{
			_bHDMIState = HDMI_STATE_PWOFF;
		}
		else
		{
	        _bHDMIState = HDMI_STATE_INIT;
		}
	        vDviInitial();
    }
}

void vBdModeChk(void)
{
    UINT8 u1Cont = 0;

    if(_bHdmiRepeaterModeDelayCount < u1Cont)
    {
        _bHdmiRepeaterModeDelayCount++;
        return;
    }
    else
    {
      _bHdmiRepeaterModeDelayCount = u1Cont;
    }

    if((_bHdmiRepeaterMode != _bAppHdmiRepeaterMode)||(_bAppHDMICurrSwitch!=_bHDMICurrSwitch))
    {
        printk("[HDMI RX] _bHdmiRepeaterMode = %d, _bAppHdmiRepeaterMode = %d\n",_bHdmiRepeaterMode,_bAppHdmiRepeaterMode);
        printk("[HDMI RX]_bHDMISwitch from %d TO %d\n",_bHDMICurrSwitch,_bAppHDMICurrSwitch);
        if(fgIsHdmiRepeater())
        {
            hdmi_tmdsonoff(0);
        }


        _fgBDPModeChgRes = TRUE;
        if(_bHdmiRepeaterMode != _bAppHdmiRepeaterMode)
        {
            if(_bAppHdmiRepeaterMode == HDMI_SOURCE_MODE)
            {
                _bHdmiRepeaterMode = _bAppHdmiRepeaterMode;
                vHalDisableHDCPDDCPort();
				{
	                vHDMITMDSCTRL(FALSE);
				}
                vHalRxSwitchPortSelect(_bAppHDMICurrSwitch);
                vHal_Disable_DGI_In();
                _bHDMICurrSwitch = HDMI_SWITCH_INIT;
                _bHDMIState= HDMI_STATE_PWOFF;
                
            }
            else
            {
				{
	                vHDMITMDSCTRL(FALSE);
				}
                vHalRxSwitchPortSelect(_bAppHDMICurrSwitch);
                vHDMIHPDLow(_bAppHDMICurrSwitch);
                _bHdmiRepeaterMode = _bAppHdmiRepeaterMode;
				_bHDMICurrSwitch = _bAppHDMICurrSwitch;
                _bHDMIState= HDMI_STATE_INIT;
            }
        }
        else
        {
            _bHDMICurrSwitch = _bAppHDMICurrSwitch;
            vHalDisableHDCPDDCPort();
			{
	            vHDMITMDSCTRL(FALSE);
			}
            vHalRxSwitchPortSelect(_bHDMICurrSwitch);
            vHDMIHPDLow(_bHDMICurrSwitch);
            _bHDMIState= HDMI_STATE_INIT;
        }

    }
}

//HDMI state monitor, clk/ckdt/scdt/hvtotal/modechange

/*----------------------5V power monitor-----------------------------------------------*/
UINT32 _u4Pwr5VStatus = 0;

void vCheckPwr5vStatus(void)
{
	unsigned char bData=0xff;
	UINT32 u4Pwr5VStatus = 0;
	u4Pwr5VStatus = u1HalChkPwr5VExist();
	if(u4Pwr5VStatus != _u4Pwr5VStatus)
	{
		_u4Pwr5VStatus = u4Pwr5VStatus;
		printk("[HDMI RX]_u4Pwr5VStatus = 0x%x \n",_u4Pwr5VStatus);
		bData = HDMI_RX_STATE_PWER_5V_STATUS;
	}
	
	if(bData!=0xff)
		vNotifyAppHdmiRxState(bData);

}

void vHdmiRxPwrStatus(UINT32 *prData)
{
      printk("[HDMI RX]power 5v status = 0x%x \n",_u4Pwr5VStatus);
     *prData = _u4Pwr5VStatus;
}

BOOL fgIsHdmiRepeater(void)
{
    if((_bAppHdmiRepeaterMode == HDMI_REPEATER_VIDEO_BYPASS_MODE)||(_bAppHdmiRepeaterMode == HDMI_REPEATER_VIDEO_DRAM_MODE))
        return(TRUE);
    else
        return(FALSE);
}

BOOL fgHdmiRepeaterIsBypassMode(void)
{
    if(_bAppHdmiRepeaterMode == HDMI_REPEATER_VIDEO_BYPASS_MODE)
        return(TRUE);
    else
        return(FALSE);
}

BOOL fgHdmiRepeaterIsDramMode(void)
{
    if(_bAppHdmiRepeaterMode == HDMI_REPEATER_VIDEO_DRAM_MODE)
        return(TRUE);
    else
        return(FALSE);
}


extern HDMI_RESOLUTION_MODE_T _bVDITiming;
extern UINT8   _bDviModeDetState;


BOOL fgHDMIRxDetectTmds(void)
{
#if 1//(DRV_SUPPORT_HDMI_RX)
    BYTE bData;
    bData = fgHalChkSCDTEnable();
    return(bData);
#else
    return(TRUE);
#endif
}

BOOL fgHDMIRxIsHdmiMode(void)
{
#if 0//(DRV_SUPPORT_HDMI_RX)
    if(_bHdmiMode)
        return TRUE;
    else
        return FALSE;
#else
    return FALSE;
#endif
}

void vModifyBstatusDepth(UINT8 u1Depth)
{
#if 0//(DRV_SUPPORT_HDMI_RX)
    _fgUseModifiedDepth = TRUE;
    _u1ModifiedDepth= u1Depth;
#endif
}

void vRecoverBstatusDepth(void)
{
#if 0//(DRV_SUPPORT_HDMI_RX)
    _fgUseModifiedDepth = FALSE;
    _u1ModifiedDepth = 0;
#endif
}

BOOL fgBDPModeChgRes(void)
{
#if (0)//DRV_SUPPORT_HDMI_RX)
    if(_fgBDPModeChgRes)
        return TRUE;
    else
        return FALSE;
#else
    return FALSE;
#endif
}

void vClearBDPModeChgResFlag(void)
{
#if 0//(DRV_SUPPORT_HDMI_RX)
    _fgBDPModeChgRes = FALSE;
#endif
}

BYTE bRxAcpType(void)
{
#if 0//(DRV_SUPPORT_HDMI_RX)
    if(_RxPacket[ACP_PACKET].fgValid)
    {
        return(_RxPacket[ACP_PACKET].PacketData[1]);
    }
    else
    {
        return(ACP_TYPE_GENERAL_AUDIO);
    }
#else
    return(ACP_TYPE_GENERAL_AUDIO);
#endif
}

void vGETHDMIRxEDID(HDMI_RX_EDID_T *prData)
{
  #if 1//(DRV_SUPPORT_HDMI_RX)
  UINT32 u2Cnt;
  printk("[HDMI Rx] vGETHDMIRxEDID  \n");
  for(u2Cnt = 0; u2Cnt <512 ; u2Cnt++)
  prData->u1HdmiRxEDID[u2Cnt]= u1HDMIINEDID[u2Cnt];
  #endif

}

void vUserSetHDMIRxEDIDToDrv(unsigned char *prData)
{
	unsigned char u2Cnt;
	printk("[HDMI Rx] vUserSetHDMIRxEDIDToDrv \n");
	if (prData != NULL) {
		memcpy(_Usersetedidtodrv.u1HdmiRxEDID, prData, 256);
	}
}

UINT8 u1ConvertChLayoutToChNumIndex(UINT32 u4ChLayout)
{
    UINT8 u1ChNumIndex = 0;
    switch(u4ChLayout)
    {
        case AUD_INPUT_1_1:
        case AUD_INPUT_2_0:
            u1ChNumIndex = 1;
            break;

        case AUD_INPUT_2_1:
        case AUD_INPUT_3_0:
        case AUD_INPUT_3_0_LRS:
            u1ChNumIndex = 2;
            break;

        case AUD_INPUT_3_1:
        case AUD_INPUT_4_0:
        case AUD_INPUT_3_1_LRS:
        case AUD_INPUT_4_0_CLRS:
            u1ChNumIndex = 3;
            break;

        case AUD_INPUT_4_1:
        case AUD_INPUT_5_0:
        case AUD_INPUT_4_1_CLRS:
            u1ChNumIndex = 4;
            break;

        case AUD_INPUT_5_1:
        case AUD_INPUT_6_0:
        case AUD_INPUT_6_0_Cs:
        case AUD_INPUT_6_0_Ch:
        case AUD_INPUT_6_0_Oh:
        case AUD_INPUT_6_0_Chr:
            u1ChNumIndex = 5;
            break;

        case AUD_INPUT_6_1:
        case AUD_INPUT_7_0:
        case AUD_INPUT_6_1_Cs:
        case AUD_INPUT_6_1_Ch:
        case AUD_INPUT_6_1_Oh:
        case AUD_INPUT_6_1_Chr:
        case AUD_INPUT_7_0_Lh_Rh:
        case AUD_INPUT_7_0_Lsr_Rsr:
        case AUD_INPUT_7_0_Lc_Rc:
        case AUD_INPUT_7_0_Lw_Rw:
        case AUD_INPUT_7_0_Lsd_Rsd:
        case AUD_INPUT_7_0_Lss_Rss:
        case AUD_INPUT_7_0_Lhs_Rhs:
        case AUD_INPUT_7_0_Cs_Ch:
        case AUD_INPUT_7_0_Cs_Oh:
        case AUD_INPUT_7_0_Cs_Chr:
        case AUD_INPUT_7_0_Ch_Oh:
        case AUD_INPUT_7_0_Ch_Chr:
        case AUD_INPUT_7_0_Oh_Chr:
        case AUD_INPUT_7_0_Lss_Rss_Lsr_Rsr:
            u1ChNumIndex = 6;
            break;

        case AUD_INPUT_7_1:
        case AUD_INPUT_7_1_Lh_Rh:
        case AUD_INPUT_7_1_Lsr_Rsr:
        case AUD_INPUT_7_1_Lc_Rc:
        case AUD_INPUT_7_1_Lw_Rw:
        case AUD_INPUT_7_1_Lsd_Rsd:
        case AUD_INPUT_7_1_Lss_Rss:
        case AUD_INPUT_7_1_Lhs_Rhs:
        case AUD_INPUT_7_1_Cs_Ch:
        case AUD_INPUT_7_1_Cs_Oh:
        case AUD_INPUT_7_1_Cs_Chr:
        case AUD_INPUT_7_1_Ch_Oh:
        case AUD_INPUT_7_1_Ch_Chr:
        case AUD_INPUT_7_1_Oh_Chr:
        case AUD_INPUT_7_1_Lss_Rss_Lsr_Rsr:
        case AUD_INPUT_8_0_Lh_Rh_Cs:
            u1ChNumIndex = 7;
            break;
        default:
            break;
    }
    return u1ChNumIndex;
}


UINT8 u1ConvertFsToFsStrIndex(UINT32 u4Fs)
{
    UINT8 u1FsStrIndex = 0;
    switch(u4Fs)
    {
        case 0x0://44.1KHz
            u1FsStrIndex = 2;
            break;
        case 0x02://48KHz
            u1FsStrIndex = 3;
            break;
        case 0x03://32KHz
            u1FsStrIndex = 1;
            break;
        case 0x08://88.2KHz
            u1FsStrIndex = 4;
            break;
        case 0x0A://96KHz
            u1FsStrIndex = 5;
            break;
        case 0x0C://176KHz
            u1FsStrIndex = 6;
            break;
        case 0x0E://192KHz
            u1FsStrIndex = 7;
            break;
        default:
            break;
    }
    return u1FsStrIndex;
}



#define FREQUENCY_10M  (10*1000*1000)
#define FREQUENCY_30M  (30*1000*1000)
#define FREQUENCY_40M  (40*1000*1000)
#define FREQUENCY_50M  (50*1000*1000)
#define FREQUENCY_160M  (160*1000*1000)
#define FREQUENCY_250M  (250*1000*1000)
HDMI_ANA_BAND eBand_pre = HDMI_ANA_BAND_NULL;
HDMI_ANA_BAND eBand = HDMI_ANA_BAND_NULL;

void internal_hdmirx_read(unsigned int u4Reg, unsigned int *p4Data)
{
	*p4Data = (*(volatile unsigned int*)(u4Reg));
}

void internal_hdmirx_write(unsigned int u4Reg, unsigned int u4data)
{
	*(volatile unsigned int*)(u4Reg) = (u4data); 
}

unsigned int ddcci_drv_read(unsigned short u2Reg)
{
    unsigned int u4Data;
    internal_hdmirx_read(DDCCI_REG_BASE+u2Reg, &u4Data);
	//printk("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void ddcci_drv_write(unsigned short u2Reg, unsigned int u4Data)
{
    //printk("[W]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
    internal_hdmirx_write(DDCCI_REG_BASE+u2Reg, u4Data);
}
/////////////////////////////////////////////////



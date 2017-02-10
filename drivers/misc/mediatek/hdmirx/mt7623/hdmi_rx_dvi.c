#include <generated/autoconf.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/switch.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <asm/tlbflush.h>
#include <asm/page.h>



#include "hdmi_rx_task.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_rx_hal.h"
#include "hdmirx.h"
#include "typedef.h"
#include "hdmi_rx_dvi.h"
#include "vga_timing.h"
#include "hdmi_rx_hw.h"
#include "vga_table.h"
#include "rx_io.h"
#include "dgi_if.h"
#include "hdmiedid.h"


#define SV_DV_NONE 0x00
#define SV_DV_DVI 0x01 // [0]
#define SV_DV_HDMI 0x02 // [1]
#define SV_DV_AD_YPbPr 0x04 // [2]
#define SV_DV_AD_RGB 0x08 // [3]
#define SV_DV_CCIR 0x10 // [4]
#define SV_DV_FROM_CCIR 0x20 // [5]
#define SV_DV_DTV 0x40 //[6]

#define SV_ON              1
#define SV_OFF             0

unsigned long _rDviModeChgTime; 	
unsigned long _rDviModeChgStartTime;	  

HAL_TIME_T rOldTimeTmp;
unsigned long u4VsyncDeltaTime = 0;
BOOL   _IsDviDetectDone = FALSE;
UINT8   _bDviModeDetState;
UINT8   _bDviVclk;
UINT16   _wDviHclk; // timing table: table value; otherwise, the value used by timing search
UINT16   _wDviWidth;
UINT16   _wDviHeight;

UINT16 _wDviTmpHtotal;
UINT16 _wDviTmpVtotal;
UINT16 _wDviVTotal;
UINT16 _wDviHtotal;

UINT8   _bDviModeDetState;
UINT8  _bDviRetry ;
UINT8  _bDviModeChged;
UINT8  _bXpcStable;

UINT8   _bDviTiming = MODE_NOSIGNAL;
UINT8 _u1RxCatureTiming = MODE_NOSIGNAL;
HDMI_RESOLUTION_MODE_T _bVDITiming;
UINT8 _bHResChgIntDetectFlg;  

UINT32 _u4NotSupportCnt;
UINT8 _bDviChkState;
extern UINT8 _bHDMIColorSpace;
extern UINT32 _wDVI_WAIT_NOSIGNAL_COUNT;

UINT8   _bDviExtSrc = SV_DV_HDMI;

UINT32   _bDviMdChgCnt;
UINT8   _bDviDeChgCnt;
UINT8   _bDviPixClkChgCnt;
UINT8   _bDviHVClkChgCnt;
UINT8  _bDviNoSigCnt ;
UINT8  _b3DInfoChgCnt;


UINT16 _wDviTmpHtotal;
UINT16 _wDviTmpVtotal;
UINT16   _wDviVTotal;
UINT16  _wDviHtotal;
UINT32  _wHDMIPixelClk;
UINT32 _u4VsyncPeriod;
UINT32 _wDviVstartDefault;
UINT8 _bPWOFFCnt;
UINT8 _bInfoChgCnt;
UINT8   _bDVIEQFlag;
UINT8 _bNotSupportCnt;
UINT8   _bDviModeDetState;
UINT8  _bDviRetry ;
UINT8  _bDviModeChged;
UINT8  _bXpcStable;

#define Diff(a, b)  (((a)>(b))?((a)-(b)):((b)-(a)))
UINT8  _bEQCH0, _bEQCH1, _bEQCH2;


enum
{
    DVI_SEARCH_STATE = 0,
    DVI_WAIT_AUTO_STATE,
    DVI_MODE_CHG_DONE_STATE
};
enum
{
    DVI_NO_SIGNAL,
    DVI_CHK_MODECHG,
    DVI_WAIT_STABLE
};

const CHAR* cDviChkState[3]=
{
    "DVI_NO_SIGNAL",
    "DVI_CHK_MODECHG",
    "DVI_WAIT_STABLE",
};

const CHAR* cDviModeDetState[3]=
{
    "DVI_SEARCH_STATE",
    "DVI_WAIT_AUTO_STATE",
    "DVI_MODE_CHG_DONE_STATE",
};

#define CCIR16_SEPARATE 0
#define CCIR16_EMBEDDED 1
#define CCIR8_SEPARATE 2
#define CCIR8_EMBEDDED 3

#define CCIR_FORMAT CCIR8_EMBEDDED


UINT8 bEnDVIDE = SV_ON;

extern BYTE _bRse640x480PEnable;
extern HDMI_3D_INFOFRAME _3DInfo;
extern HDMI_3D_INFOFRAME _3DInfoOld;
extern UINT8 _bUnplugFlag;

extern UINT8 _bHdmiMode;
extern UINT8 _bHdmiMD;
extern UINT8 _bHDMIScanInfo;
extern UINT8 _bHDMIAspectRatio;
extern UINT8 _bHDMIAFD;
extern UINT8 _bHDMIHDCPStatus;
extern UINT8 _bHDMI422Input;
extern UINT8 _bHDMIITCFlag;
extern UINT8 _bHDMIITCContent;
extern UINT32 _wDVI_WAIT_STABLE_COUNT;
extern  UINT32 _wHDMIBypassFlag;
extern UINT32 _wDVI_WAIT_NOSIGNAL_COUNT;
extern UINT8 _bHDMIColorSpace;

static UINT16 wDviIHSClock(void);
static void vEnterDEMode(UINT8 bonoff);
static void vDviSetInputCapture(UINT8 bMode);
static UINT16 wDviIHSClock(void);

const CHAR* szRxResStr[] =
{
    "MODE_NOSIGNAL",        // No signal   //  0
    "MODE_525I_OVERSAMPLE",      //SDTV 
    "MODE_625I_OVERSAMPLE",       //
    "MODE_480P_OVERSAMPLE",       //SDTV
    "MODE_576P_OVERSAMPLE",
    "MODE_720p_50",               //HDTV 
    "MODE_720p_60",               //HDTV   
    "MODE_1080i_48",              //HDTV  
    "MODE_1080i_50",              //HDTV  
    "MODE_1080i",                 //HDTV
    "MODE_1080p_24",              //HDTV 
    "MODE_1080p_25",
    "MODE_1080p_30",
    "MODE_1080p_50",              //HDTV 
    "MODE_1080p_60",
    "MODE_525I",
    "MODE_625I",
    "MODE_480P",
    "MODE_576P",    
    "MODE_720p_24",   
    "MODE_720p_25",    // 20 
    "MODE_720p_30",        
    "MODE_240P",
    "MODE_540P",
    "MODE_288P",    
    "MODE_480P_24",    
    "MODE_480P_30",        
    "MODE_576P_25",    
    "MODE_3D_720p_50",
    "MODE_3D_720p_60",        
    "MODE_3D_1080p_24",      //30
    "MODE_3D_1080I_60_FRAMEPACKING",   
    "MODE_3D_1080I_50_FRAMEPACKING",     
    "MODE_3D_1080P60HZ",
    "MODE_3D_1080P50HZ",
    "MODE_3D_1080P30HZ",
    "MODE_3D_1080P25HZ",
    "MODE_3D_720P30HZ", 
    "MODE_3D_720P25HZ", 
    "MODE_3D_720P24HZ",
    "MODE_3D_576P50HZ",      // 40
    "MODE_3D_576I50HZ", 
    "MODE_3D_480P60HZ", 
    "MODE_3D_480I60HZ", 
    "MODE_REVERSE1",
    "MODE_REVERSE2",    
    "MODE_HDMI_640_480P",  // = 46,
};

const CHAR* szResStr[] =
{
    "RES_480I",
    "RES_576I",
    "RES_480P",
    "RES_576P",
    "RES_480P_1440",
    "RES_576P_1440",
    "RES_480P_2880",
    "RES_576P_2880",
    "RES_720P60HZ",
    "RES_720P50HZ",
    "RES_1080I60HZ",
    "RES_1080I50HZ",
    "RES_1080P60HZ",
    "RES_1080P50HZ",
    "RES_1080P30HZ",
    "RES_1080P25HZ",
    "RES_480I_2880",
    "RES_576I_2880",
    "RES_1080P24HZ",
    "RES_1080P23_976HZ",
    "RES_1080P29_97HZ",
    "RES_3D_1080P23HZ",
    "RES_3D_1080P24HZ",
    "RES_3D_720P60HZ",
    "RES_3D_720P50HZ",
    "RES_3D_720P30HZ",
    "RES_3D_720P25HZ",
    "RES_3D_576P50HZ",
    "RES_3D_480P60HZ",
    "RES_3D_1080I60HZ",
    "RES_3D_1080I50HZ",
    "RES_3D_1080I30HZ",
    "RES_3D_1080I25HZ",
    "RES_3D_576I25HZ",
    "RES_3D_480I30HZ",
    "RES_3D_576I50HZ",
    "RES_3D_480I60HZ",
    "RES_2D_480I60HZ",
    "RES_2D_576I50HZ",
    "RES_2D_640x480HZ",
    "RES_PANEL_AUO_B089AW01",
    "RES_3D_720P60HZ_TB",
    "RES_3D_720P50HZ_TB",
    "RES_3D_1080I60HZ_SBS_HALF",
    "RES_3D_1080I50HZ_SBS_HALF",
    "RES_3D_1080P23HZ_TB",
    "RES_3D_1080P24HZ_TB",

    "RES_2160P_23_976HZ",
    "RES_2160P_24HZ",
    "RES_2160P_25HZ",
    "RES_2160P_29_97HZ",
    "RES_2160P_30HZ",
    "RES_2161P_24HZ",

    "RES_720P30HZ",
    "RES_720P25HZ",
    "RES_720P24HZ",
    "RES_720P23HZ",

    "RES_3D_1080P60HZ",
    "RES_3D_1080P50HZ",
    "RES_3D_1080P30HZ",
    "RES_3D_1080P29HZ",
    "RES_3D_1080P25HZ",
    "RES_3D_720P24HZ",
    "RES_3D_720P23HZ",

    "RES_3D_1080P60HZ_TB",
    "RES_3D_1080P50HZ_TB",
    "RES_3D_1080P30HZ_TB",
    "RES_3D_1080P29HZ_TB",
    "RES_3D_1080P25HZ_TB",
    "RES_3D_1080I60HZ_TB",
    "RES_3D_1080I50HZ_TB",
    "RES_3D_1080I30HZ_TB",
    "RES_3D_1080I25HZ_TB",
    "RES_3D_720P30HZ_TB",
    "RES_3D_720P25HZ_TB",
    "RES_3D_720P24HZ_TB",
    "RES_3D_720P23HZ_TB",
    "RES_3D_576P50HZ_TB",
    "RES_3D_576I25HZ_TB",
    "RES_3D_576I50HZ_TB",
    "RES_3D_480P60HZ_TB",
    "RES_3D_480I30HZ_TB",
    "RES_3D_480I60HZ_TB",

    "RES_3D_1080P60HZ_SBS_HALF",
    "RES_3D_1080P50HZ_SBS_HALF",
    "RES_3D_1080P30HZ_SBS_HALF",
    "RES_3D_1080P29HZ_SBS_HALF",
    "RES_3D_1080P25HZ_SBS_HALF",
    "RES_3D_1080P24HZ_SBS_HALF",
    "RES_3D_1080P23HZ_SBS_HALF",
    "RES_3D_1080I30HZ_SBS_HALF",
    "RES_3D_1080I25HZ_SBS_HALF",
    "RES_3D_720P60HZ_SBS_HALF",
    "RES_3D_720P50HZ_SBS_HALF",
    "RES_3D_720P30HZ_SBS_HALF",
    "RES_3D_720P25HZ_SBS_HALF",
    "RES_3D_720P24HZ_SBS_HALF",
    "RES_3D_720P23HZ_SBS_HALF",
    "RES_3D_576P50HZ_SBS_HALF",
    "RES_3D_576I25HZ_SBS_HALF",
    "RES_3D_576I50HZ_SBS_HALF",
    "RES_3D_480P60HZ_SBS_HALF",
    "RES_3D_480I30HZ_SBS_HALF",
    "RES_3D_480I60HZ_SBS_HALF",
    "RES_4K2K23976HZ", //104
    "RES_4K2K24HZ", //105
    "RES_2160P_50HZ",
    "RES_2160P_60HZ",
    "RES_2161P_50HZ",
    "RES_2161P_60HZ",
    "RES ERROR,OVERFLOW",

};

const CHAR* szVinResStr[RES_MODE_NUM+2] ={
    "RES_480I=0",
    "RES_576I",
    "RES_480P",
    "RES_576P",
    "RES_480P_1440",
    "RES_576P_1440",
    "RES_480P_2880",
    "RES_576P_2880",
    "RES_720P60HZ",
    "RES_720P50HZ",
    "RES_1080I60HZ",
    "RES_1080I50HZ",
    "RES_1080P60HZ",
    "RES_1080P50HZ",
    "RES_1080P30HZ",
    "RES_1080P25HZ",
    "RES_480I_2880",
    "RES_576I_2880",
    "RES_1080P24HZ",
    "RES_1080P23_976HZ",
    "RES_1080P29_97HZ",
    "RES_3D_1080P23HZ",
    "RES_3D_1080P24HZ",    
    "RES_3D_720P60HZ",
    "RES_3D_720P50HZ",
    "RES_3D_720P30HZ",
    "RES_3D_720P25HZ",
    "RES_3D_576P50HZ",
    "RES_3D_480P60HZ",    
    "RES_3D_1080I60HZ",
    "RES_3D_1080I50HZ",    
    "RES_3D_1080I30HZ",    
    "RES_3D_1080I25HZ",    
    "RES_3D_576I25HZ",
    "RES_3D_480I30HZ",
    "RES_3D_576I50HZ",
    "RES_3D_480I60HZ",        
    "RES_2D_480I60HZ",  
    "RES_2D_576I50HZ",
    "RES_2D_640x480P60HZ",                                                         
    "RES_PANEL_AUO_B089AW01",
    "RES_3D_720P60HZ_TB", 
    "RES_3D_720P50HZ_TB",  
    "RES_3D_1080I60HZ_SBS_HALF", 
    "RES_3D_1080I50HZ_SBS_HALF",   
    "RES_3D_1080P23HZ_TB",
    "RES_3D_1080P24HZ_TB",  
   
    "RES_720P30HZ",                                                              
    "RES_720P25HZ",                                                              
    "RES_720P24HZ",                                                              
    "RES_720P23HZ",                                                              
                                                  
    "RES_3D_1080P60HZ",                                                          
    "RES_3D_1080P50HZ",                                                          
    "RES_3D_1080P30HZ",                                                          
    "RES_3D_1080P29HZ",
    "RES_3D_1080P25HZ",                                                          
    "RES_3D_720P24HZ",                                                           
    "RES_3D_720P23HZ",                                                           
                                        
    "RES_3D_1080P60HZ_TB",                                                       
    "RES_3D_1080P50HZ_TB",                                                       
    "RES_3D_1080P30HZ_TB",                                                       
    "RES_3D_1080P29HZ_TB",
    "RES_3D_1080P25HZ_TB",                                                       
    "RES_3D_1080I60HZ_TB",                                                       
    "RES_3D_1080I50HZ_TB",                                                       
    "RES_3D_1080I30HZ_TB",                                                       
    "RES_3D_1080I25HZ_TB",                                                       
    "RES_3D_720P30HZ_TB",                                                        
    "RES_3D_720P25HZ_TB",                                                        
    "RES_3D_720P24HZ_TB",                                                        
    "RES_3D_720P23HZ_TB",                                                        
    "RES_3D_576P50HZ_TB",                                                        
    "RES_3D_576I25HZ_TB",                                                        
    "RES_3D_576I50HZ_TB",                                                        
    "RES_3D_480P60HZ_TB",                                                        
    "RES_3D_480I30HZ_TB",                                                        
    "RES_3D_480I60HZ_TB",                                                        
                                            
    "RES_3D_1080P60HZ_SBS_HALF",                                                 
    "RES_3D_1080P50HZ_SBS_HALF",                                                 
    "RES_3D_1080P30HZ_SBS_HALF",                                                 
    "RES_3D_1080P29HZ_SBS_HALF",
    "RES_3D_1080P25HZ_SBS_HALF",                                                 
    "RES_3D_1080P24HZ_SBS_HALF",                                                 
    "RES_3D_1080P23HZ_SBS_HALF",                                                 
    "RES_3D_1080I30HZ_SBS_HALF",                                                 
    "RES_3D_1080I25HZ_SBS_HALF",                                                 
    "RES_3D_720P60HZ_SBS_HALF",                                                  
    "RES_3D_720P50HZ_SBS_HALF",                                                  
    "RES_3D_720P30HZ_SBS_HALF",                                                  
    "RES_3D_720P25HZ_SBS_HALF",                                                  
    "RES_3D_720P24HZ_SBS_HALF",                                                  
    "RES_3D_720P23HZ_SBS_HALF",                                                  
    "RES_3D_576P50HZ_SBS_HALF",                                                  
    "RES_3D_576I25HZ_SBS_HALF",                                             
    "RES_3D_576I50HZ_SBS_HALF",                                             
    "RES_3D_480P60HZ_SBS_HALF",                                             
    "RES_3D_480I30HZ_SBS_HALF",                                             
    "RES_3D_480I60HZ_SBS_HALF",                                             

    "RES_MODE_NUM",                                                         
    "RES_AUTO"            
};

#define wDVIGetVtotal()	wHDMIVTotal()
#define wDVIGetHtotal()	wHDMIHTotal()
#define bDVIGetHAct()	fgHDMIHsyncAct()
#define bDVIGetWidth()	wHDMIResoWidth()
#define bDVIGetHeight()	wHDMIResoHeight()

HDMI_RESOLUTION_MODE_T vConvertHdmiRXResToVDORes(UINT8 u2Timing)
{
	HDMI_RESOLUTION_MODE_T e_Res;
	BYTE bVFrontPorch = vHalRxGetVFrontPorch();

	_bRse640x480PEnable=FALSE;

	printk("[HDMI RX]u2Timing = %d,L%d\n", u2Timing,__LINE__);
	switch(u2Timing)
	{
	case MODE_HDMI_640_480P:
	e_Res =RES_2D_640x480HZ ;
	if(bVFrontPorch == 0x12)
	{
		//vSet640x480PEnable(2);//VESA VGA
	}
	else
	{
		//vSet640x480PEnable(1);//CEA VGA
	}

	break;

	case MODE_525I:
		e_Res =RES_480I;
	break;
	
	case MODE_625I:
		e_Res =RES_576I;
	break;
	
	case MODE_480P:
	e_Res = RES_480P;
	break;
	
	case MODE_576P:
	e_Res =RES_576P ;
	break;
	
	case MODE_720p_50:
	if(_3DInfo.HDMI_3D_Enable)
	{
		switch(_3DInfo.HDMI_3D_Structure)
		{
			case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_720P50HZ_TB;
			break;
			
			case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_720P50HZ_SBS_HALF;
			break;
			
			default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
			break;
		}
	}
	else
	{	  	
		e_Res = RES_720P50HZ;
	}
	break;


	case MODE_720p_60:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_720P60HZ_TB;
				break;
				
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_720P60HZ_SBS_HALF;
				break;
				
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res = RES_720P60HZ;
		}
	break;

	case MODE_720p_30:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
					e_Res = RES_3D_720P30HZ_TB;
					break;
					
				case HDMI_3D_Structure_SideBySideHalf:
					e_Res = RES_3D_720P30HZ_SBS_HALF;
					break;
					
				default:
					printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
					printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
					e_Res = RES_MODE_NUM;
					break;
			}
		}
		else
		{		  	
			e_Res = RES_720P30HZ;
		}
	break;

	case MODE_720p_25:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
					e_Res = RES_3D_720P25HZ_TB;
				break;
				
				case HDMI_3D_Structure_SideBySideHalf:
					e_Res = RES_3D_720P25HZ_SBS_HALF;
				break;
				
				default:
					printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
					printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
					e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
		e_Res = RES_720P25HZ;
		}
		break;

	case MODE_720p_24:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_720P24HZ_TB;
				break;
				
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_720P24HZ_SBS_HALF;
				break;
				
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res = RES_720P24HZ;
		}
		break;

	case MODE_3D_720p_50:
		e_Res = RES_3D_720P50HZ;		 
		break;

	case MODE_3D_720p_60:
		e_Res = RES_3D_720P60HZ;		 
		break;
		
	case MODE_3D_720P30HZ:
		e_Res = RES_3D_720P30HZ;		 
		break;

	case MODE_3D_720P25HZ:
		e_Res = RES_3D_720P25HZ;		 
		break;

	case MODE_3D_720P24HZ:
		e_Res = RES_3D_720P24HZ;		 
		break;

	case MODE_1080i_50:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_1080I50HZ_TB;
				break;
				
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_1080I50HZ_SBS_HALF;
				break;
				
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res = RES_1080I50HZ;
		}
		break;

	case MODE_3D_1080I_50_FRAMEPACKING:
		e_Res = RES_3D_1080I50HZ; 
		break;	  

	case MODE_1080i:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_1080I60HZ_TB;
				break;
				
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_1080I60HZ_SBS_HALF;
				break;
				
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		
			e_Res = RES_1080I60HZ;
		}

	break;

	case MODE_3D_1080I_60_FRAMEPACKING:
		e_Res = RES_3D_1080I60HZ;		
		break;	  

	case MODE_1080p_24:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_1080P24HZ_TB;
				break;
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_1080P24HZ_SBS_HALF;
				break;
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res = RES_1080P24HZ;
		}

	break;

	case MODE_3D_1080p_24:
		e_Res = RES_3D_1080P24HZ;				
	break;	  

	case MODE_1080p_50:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_1080P50HZ_TB;
				break;
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_1080P50HZ_SBS_HALF;
				break;
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res =RES_1080P50HZ ;
		}
	break;
	
	case MODE_1080p_60:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_1080P60HZ_TB;
				break;
				
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_1080P60HZ_SBS_HALF;
				break;
				
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res =RES_1080P60HZ ;
		}
	break;
	case MODE_1080p_30:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_1080P30HZ_TB;
				break;
				
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_1080P30HZ_SBS_HALF;
				break;
				
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res =RES_1080P30HZ ;
		}
	break;

	case MODE_3D_1080P30HZ:
		e_Res = RES_3D_1080P30HZ;				
	break;	  

	case MODE_1080p_25:
		if(_3DInfo.HDMI_3D_Enable)
		{
			switch(_3DInfo.HDMI_3D_Structure)
			{
				case HDMI_3D_Structure_TopBottom:
				e_Res = RES_3D_1080P25HZ_TB;
				break;
				case HDMI_3D_Structure_SideBySideHalf:
				e_Res = RES_3D_1080P25HZ_SBS_HALF;
				break;
				default:
				printk("[HDMI Rx] Resolution %s = %d \n",szRxResStr[u2Timing]);
				printk("[HDMI Rx] Unsupport HDMI 3D Structure: Structure = %d \n",_3DInfo.HDMI_3D_Structure);
				e_Res = RES_MODE_NUM;
				break;
			}
		}
		else
		{		  	
			e_Res =RES_1080P25HZ ;
		}
	break;

	case MODE_3D_1080P25HZ:
		e_Res = RES_3D_1080P25HZ;				
	break;	  

	case MODE_2160P_30HZ:
		e_Res = RES_2160P_30HZ;
	break;

	default:
		e_Res =RES_MODE_NUM ;
	break;

	}

	return  e_Res;
}

static UINT16 wDviIHSClock(void)
{
    return dwHDMILineFreq();
}

UINT8 bDviIVSClock(void)
{
    return bHDMIRefreshRate();
}


void vDVISetModeCHG(void)
{
	HDMI_RX_SYNC_LOG("[HDMI RX] _bDviModeChged = %d\n",_bDviModeChged);
	if (_bDviModeChged)
	{
		return ;
	}
    Linux_HAL_GetTime(&_rDviModeChgTime);
	_bDviModeChged = 1 ;
	_bPWOFFCnt=0;
	_bXpcStable = 0;

}

void vDVISetModeDone(void)
{
    if (!_bDviModeChged)
    {
        return ;
    }
	
	
    vHalClearModeChgIntState();
	bHDMIPHYReset(HDMI_RST_DEEPCOLOR);
    vHalRxResetTDFifoAutoRead();  // jiewen, 20090114

    _bHResChgIntDetectFlg = 1;  // jiewen, 20090114
    _bHDMIColorSpace=bHDMIInputType();//color space

    _bDviModeChged = 0 ;

    return ;
}

UINT32 dwDviPIXClock(void)
{
	return (wDviIHSClock()*(wHDMIHTotal()));
}

UINT8 bDviStdTimingSearch(UINT8 bMode, UINT16   wDviHclk, UINT8 bDviVclk, UINT16 wDviHtotal,UINT16 wDviWidth)
{
    UINT8 bSearch;
    UINT8 bSearchEnd;

    // Video Mode
    bSearch = HDTV_SEARCH_START;
    bSearchEnd = HDTV_SEARCH_END;

	printk("[HDMI RX] bMode = %d,wDviHclk = %d,bDviVclk = %d,wDviHtotal = %d,wDviWidth =%d\n",bMode,wDviHclk,bDviVclk,wDviHtotal,wDviWidth);
    //setch hdmi timing
    do
    {   
        if ((bDviVclk >= (Get_HDMIMODE_IVF(bSearch) - 2)) && (bDviVclk <= (Get_HDMIMODE_IVF(bSearch) + 2)))
        {
            if ((wDviHclk >= (Get_HDMIMODE_IHF(bSearch) - 5)) && (wDviHclk <= (Get_HDMIMODE_IHF(bSearch) + 5)))
            {
                if ((wDviHtotal > (Get_HDMIMODE_IHTOTAL(bSearch)-40)) && (wDviHtotal < (Get_HDMIMODE_IHTOTAL(bSearch)+40)))
                {
                    return bSearch;
                }

            }
        }
    }
    while (++bSearch <= bSearchEnd);

    return MODE_NOSUPPORT;
}

UINT16 wDviGetTableHactive(void)
{
	UINT16 wHDMIHactive;
	wHDMIHactive =wHDMIResoWidth();
	if( fgIsVideoTiming(_bDviTiming))
	{
		if((wHDMIHactive> (Get_VGAMODE_IPH_WID(_bDviTiming)-40))
			&&(wHDMIHactive<(Get_VGAMODE_IPH_WID(_bDviTiming)+40)))
		{
			return wHDMIHactive;
		}
		else
		{
			return Get_VGAMODE_IPH_WID(_bDviTiming);
		}
	}
	else
	{
		return  wHDMIHactive ;
	}
}

static void DviSetInputCapture(UINT8 bMode)
{
    _u1RxCatureTiming =  bMode; //kenny add
    return ;
}

void vDviInitial(void)
{
	HDMI_RX_SYNC_LOG("[enter %s]\n",__FUNCTION__);
	_bDviTiming = MODE_WAIT;
	_bDviModeDetState = DVI_SEARCH_STATE;
	_bDviMdChgCnt = 0;
	_bDviDeChgCnt = 0 ;
    _b3DInfoChgCnt = 0;	
	_bDviPixClkChgCnt=0;
	_bDviHVClkChgCnt=0;
	_bDviNoSigCnt = 0 ;
	_wDviVstartDefault=0;
	_bNotSupportCnt=0;
	vDVISetModeCHG();
}

UINT8 bDviRefreshRate(void)
{
	if (bHDMI3DPacketVaild())//3d
	{
		return wHDMI3DGetHVParameter(5);
	}
	else
	{
		if ((bEnDVIDE == 1) && (_bDviTiming == MODE_DE_MODE))
		{
			return _bDviVclk ;
		}
		else
		{
			if ((_IsDviDetectDone) &&(_bDviTiming != MODE_NOSIGNAL) &&(_bDviTiming < MAX_TIMING_FORMAT))
			{
				return Get_VGAMODE_IVF(_bDviTiming);
			}
			else
			{
				return 0;
			}
		}
	}

}

UINT8 bDviInterlace(void)
{
	if ((_IsDviDetectDone) &&(_bDviTiming != MODE_NOSIGNAL) &&(_bDviTiming < MAX_TIMING_FORMAT))
	{
		return (Get_VGAMODE_INTERLACE(_bDviTiming));
	}
	else
	{
		return (0);
	}
}

UINT32 u4GetVsyncPeriod(void)
{
  return u4VsyncDeltaTime;
}

void hdmirx_irq_handle(void)
{
	HAL_TIME_T rDelta;
	HAL_TIME_T rTimeTmp;
    
	if(fgHalIsINTR_VSYNC())
	{
		Linux_HAL_GetTime(&rTimeTmp);
		Linux_HAL_GetDeltaTime(&rDelta, &rOldTimeTmp, &rTimeTmp);
		u4VsyncDeltaTime = rDelta.u4Micros;
		rOldTimeTmp.u4Seconds=rTimeTmp.u4Seconds; 
		rOldTimeTmp.u4Micros=rTimeTmp.u4Micros;
		vHalClearVSYNCIntStatus();
	}
	if((u1HalChkCKDTExist() == FALSE))
	{
		vHalHDMIRxEnableVsyncInt(FALSE);
	}

	if(fgHalIsINTR2_CKDT())  // lost CLOCK
	{
		if((u1HalChkCKDTExist() == FALSE))
		{
			if(fgIsHdmiRepeater())	
			{
				printk("[HDMI RX] CKDT = FALSE\n");
				vHalEnableINTR2_CKDT(FALSE);
			}
		}
		vHalClearINTR2_CKDT();
	}	

}


UINT8 bGetSupportHdmiModeVideoTiming(void)
{
    UINT8 _u8Timing = 0;
    UINT16 _u16Width = wHDMIResoWidth();
    UINT16 _u16Height = wHDMIResoHeight();
    UINT8 _u8Rate = bHDMIRefreshRate();

    if ((_u16Width == 1280) && (_u16Height == 720)) 
    {
        if (_u8Rate == 60) 
        {
            _u8Timing = MODE_720p_60;
        }
        else if (_u8Rate == 50) 
        {
            _u8Timing = MODE_720p_50;
        }
    }
    else if ((_u16Width == 1920) && (_u16Height == 1080)) 
    {
        if (_u8Rate == 60) 
        {
            _u8Timing = MODE_1080p_60;
        }
        else if (_u8Rate == 50) 
        {
            _u8Timing = MODE_1080p_50;
        }
    }
    else if (_u16Width == 720) 
    {
        if (_u8Rate == 60) 
        {
            _u8Timing = MODE_480P;
        }
        else if (_u8Rate == 50) 
        {
            _u8Timing = MODE_576P;
        }
    }
    else
    {
        if (!bDviInterlace())
        {
            _u8Timing=_bDviTiming;
        }
    }

    return _u8Timing ;
}

void vDviModeDetect(void)
{
	VSW_GET_INFO_COND_T _HdmiRxTimingStatus = VSW_COMP_NFY_ERROR;
	DGI_VIDEO_IN_INFO_T rInput;
	if (!((_bHDMIState == HDMI_STATE_AUTH) || (_bHDMIState == HDMI_STATE_PWOFF)))
	{
		return;
	}
    
	HDMI_RX_SYNC_LOG("[HDMI RX]_bDviModeDetState = %d\n",_bDviModeDetState);
	switch (_bDviModeDetState)
	{
		// Timing Search State
		case DVI_SEARCH_STATE:
			//HDMIRX_LOG(HDMIRX_LOG_DEBUG, "SEARCH_STATE\r\n");
			vHalDisableRxAvMute();
			/* check SCDT */
			if (!fgHalChkSCDTEnable())
			{
				_bDviTiming = MODE_NOSIGNAL;
				printk("[HDMI RX]md: no signal\n");
			}
			else
			{
				/* search timing */
				vHdmiGet3DInfo(FALSE);
				_wDviHclk = wDviIHSClock();
				_bDviVclk = bDviIVSClock();

				_bDviTiming =  bDviStdTimingSearch(1, _wDviHclk, _bDviVclk, _wDviHtotal,_wDviWidth);
				printk("[HDMI RX]_bDviTiming = %d \n",_bDviTiming);
			}
			
			if(_bDviTiming == MODE_NOSUPPORT)
			{
				if(_u4NotSupportCnt++ < 90)
				{
					break;
				}
			}
            
			if(_bDviTiming >= MODE_MAX && _bDviTiming < MODE_DE_MODE )  // BP_1000, Power off, green line issue
			{
				_bDviTiming = MODE_NOSIGNAL;
			}
			
			_bDviModeDetState = DVI_MODE_CHG_DONE_STATE;

			// Mode found among timing table
			if ((_bDviTiming > MODE_NOSIGNAL) && (_bDviTiming < MAX_TIMING_FORMAT))
			{

				printk("[HDMI RX]md: timing %d found\n", _bDviTiming);
				if (_bDviTiming != MODE_DE_MODE)
				{
					_wDviHclk = Get_HDMIMODE_IHF(_bDviTiming);
					_bDviVclk = Get_HDMIMODE_IVF(_bDviTiming);
				}
				_bDviModeDetState = DVI_WAIT_AUTO_STATE;

			}
			else if (_bDviTiming != MODE_NOSIGNAL)
			{
				_bDviTiming = MODE_NOSUPPORT;
				printk("[HDMI RX]md: timing not support\n");
			}
			_bDviRetry = 0;
			_wDviWidth =  bDVIGetWidth() ;
			_wDviHeight =  bDVIGetHeight();
			
			break;

			// Wait DVI Auto Done state
		case DVI_WAIT_AUTO_STATE:
			
			if ((_wDviWidth != bDVIGetWidth()) || (_wDviHeight != bDVIGetHeight()))
			{
				if (_bDviRetry++ < 10)
				{
					_wDviWidth =  bDVIGetWidth();
					_wDviHeight =  bDVIGetHeight();
					break;
				}
			}

			_bDviModeDetState = DVI_MODE_CHG_DONE_STATE;
			break;

			// Mode Chg Done State
		case DVI_MODE_CHG_DONE_STATE:
			vHalRxEnableAvMuteRecv();
			//vHDMISetColorRalated();


			_bDviChkState = DVI_CHK_MODECHG;
			//HalHdmiAcrRst();
			//SwitchAudioState(ASTATE_RequestAudio);
			
			// set input capture
			DviSetInputCapture(_bDviTiming);
			printk("[HDMI RX]_bDviTiming = %d\n", _bDviTiming);  
			
			 vHdmiGet3DInfo(TRUE);	  
			 _u4VsyncPeriod = u4GetVsyncPeriod();
			_3DInfoOld.HDMI_3D_Enable = _3DInfo.HDMI_3D_Enable;
			_3DInfoOld.HDMI_3D_Video_Format = _3DInfo.HDMI_3D_Video_Format;
			_3DInfoOld.HDMI_3D_Structure = _3DInfo.HDMI_3D_Structure;
			_3DInfoOld.HDMI_3D_EXTDATA = _3DInfo.HDMI_3D_EXTDATA; 
			
			//change to video mode resolution 
			_bVDITiming = vConvertHdmiRXResToVDORes(_bDviTiming);
			if(_bVDITiming < RES_MODE_NUM)
				printk("[HDMI RX]_bVDITiming = %d, %s\n", _bVDITiming,szResStr[_bVDITiming]);
			else
				printk("[HDMI RX]_bVDITiming = %d,  No Signal \n", _bVDITiming);
            
			_HdmiRxTimingStatus = VSW_COMP_NFY_RESOLUTION_CHG_DONE;
			vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
			vHalEnableINTR2_CKDT(TRUE);
			printk("[HDMI RX][NFY]RESOLUTION_CHG_DONE\n"); 
			
			vDVISetModeDone(); 
			vHDMIRXColorSpaceConveter();
			
			_IsDviDetectDone = TRUE;
			_bDviModeDetState = DVI_SEARCH_STATE;
			HDMI_RX_SYNC_LOG("[enter %s]_IsDviDetectDone = %d\n ",__FUNCTION__,_IsDviDetectDone);
			if(fgHdmiRepeaterIsBypassMode())
			{
				rInput.ePinType = 9;
				rInput.eInputMode = 1;
				vset_dgi_in_mode(rInput,_bVDITiming);
			}
			break;
			
		default:
			break;
	}

}

void vDviChkModeChange(void)
{
	UINT16 wvtemp;
	UINT16 whtemp;
	UINT32 u4DviHclkTemp;
	UINT32 u4DviVclkTemp;	
	unsigned long rDelta; 
	unsigned long rCurTime;
    UINT8 bEQCH0_tmp, bEQCH1_tmp, bEQCH2_tmp;
	VSW_GET_INFO_COND_T _HdmiRxTimingStatus = VSW_COMP_NFY_ERROR;
	// check unstable than time out for mode detect done
	if (_bDviModeChged)
	{
		Linux_HAL_GetTime(&rCurTime);
		rDelta = _wDVI_WAIT_NOSIGNAL_COUNT;
		if((Linux_HAL_GetDeltaTime(&rDelta, &_rDviModeChgTime, &rCurTime)== TRUE))
		{
		   
			printk("[HDMI RX]mc:  Time out to force mode detect done \n");
			if (bDVIGetHAct())//True: DE enbale
			{
				_bDviTiming = MODE_NOSUPPORT;
			}
			else
			{
				_bDviTiming = MODE_NOSIGNAL;
			}
			_wDviHtotal = 0 ;
			_wHDMIPixelClk=0;
			_bDviMdChgCnt = 0 ;
			_bDviChkState = DVI_NO_SIGNAL ;

			_HdmiRxTimingStatus = VSW_COMP_NFY_UNLOCK;
			vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
			printk("[HDMI RX][NFY]HDMI RX UNLOCK L%d\n",__LINE__);  
			vDVISetModeDone();
		}
	}
	
	if (!((_bHDMIState == HDMI_STATE_AUTH) || (_bHDMIState == HDMI_STATE_PWOFF)))
	{
		return;
	}

	if((_bDviChkState != DVI_NO_SIGNAL) || _bUnplugFlag)
	{
		if(_bUnplugFlag)
		{
			_bUnplugFlag=0;
			
			vDviInitial();
			//vDVISetModeCHG();
			_wDviHtotal = 0 ;
			_wHDMIPixelClk=0;
			_bDviTiming = MODE_NOSIGNAL;
			_bDviMdChgCnt = 0 ;
			_bDviChkState = DVI_NO_SIGNAL ;
			//SwitchAudioState(ASTATE_AudioOff);	
			_HdmiRxTimingStatus = VSW_COMP_NFY_UNLOCK;
			vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
			vDVISetModeDone();
		}
		else
		{
			if (!bDVIGetHAct())
			{
				if (++_bDviNoSigCnt >= 200)
				{
					vDviInitial();
					_HdmiRxTimingStatus = VSW_COMP_NFY_UNLOCK;
					vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
					printk("[HDMI RX][NFY]HDMI RX UNLOCK L%d\n",__LINE__);  
					vDVISetModeDone();
					_wDviHtotal = 0 ;
					_wHDMIPixelClk=0;
					_bDviTiming = MODE_NOSIGNAL;
					_bDviMdChgCnt = 0 ;
					_bDviChkState = DVI_NO_SIGNAL ;
				}
			}
			else
			{
				_bDviNoSigCnt =  0 ;
			}
		}
	}

	whtemp = wDVIGetHtotal();
	wvtemp = wDVIGetVtotal();
	u4DviHclkTemp = wDviIHSClock();
	u4DviVclkTemp = bDviIVSClock(); 
	
	HDMI_RX_SYNC_LOG("[HDMI RX]_bDviChkState =%d\n",_bDviChkState);
	switch (_bDviChkState)
	{
		case DVI_NO_SIGNAL :
		if(fgIsHdmiRepeater())	
		{ 
			if(vIsTmdsOn())
			{
				hdmi_tmdsonoff(0);
			}
		}
		if (bDVIGetHAct())
		{
			printk("[HDMI RX]mc: from no signal to wait stable\n");
			_bDVIEQFlag=1;
			_bDviChkState = DVI_WAIT_STABLE ;
			_bDviMdChgCnt = 0;
			Linux_HAL_GetTime(&_rDviModeChgStartTime);
			return ;
		}
		else
		{
			_bPWOFFCnt++;
			if(_bDviModeChged&&(_bPWOFFCnt >3))
			{
				printk("[HDMI RX]mc: DVI_NO_SIGNAL to force mode done \n");
				_bPWOFFCnt=0;
				_bDviTiming = MODE_NOSIGNAL;
				_HdmiRxTimingStatus = VSW_COMP_NFY_UNLOCK;
				vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
				printk("[HDMI RX][NFY]HDMI RX UNLOCK L%d\n",__LINE__);	
				vDVISetModeDone();
			}
		}

		break;
		
	case DVI_CHK_MODECHG:
		if(fgHalCheckRxHResChg() && _bHResChgIntDetectFlg)
		{
			vHalRxResetTDFifoAutoRead();
			vHalClearRxHresChgIntState();	
			_bHResChgIntDetectFlg = 0;
		}
		if(bHDMIInputType() != _bHDMIColorSpace)
		{
			//SwitchAudioState(ASTATE_AudioOff);
			vHDMIRXColorSpaceConveter();
			printk("[HDMI RX]mc: color space change and trigger mode change bHDMIInputType() =%d \n", bHDMIInputType() );
			_bDVIEQFlag=0;									
			_bDviChkState = DVI_WAIT_STABLE  ;
			vDviInitial();		
		}

		
		HDMI_RX_SYNC_LOG("[HDMI RX]_bHDMIScanInfo =%d,_bHDMIAspectRatio = %d, _bHDMIAFD = %d,_bHDMI422Input = %d,_bHDMIITCFlag = %d,_bHDMIITCContent = %d,_bHDMIHDCPStatus = %d\n",
			             _bHDMIScanInfo,_bHDMIAspectRatio,_bHDMIAFD,_bHDMI422Input,_bHDMIITCFlag,_bHDMIITCContent ,_bHDMIHDCPStatus);
		if((_bHDMIScanInfo != bHDMIScanInfo()) ||(_bHDMIAspectRatio!= bHDMIAspectRatio())||(_bHDMIAFD!= bHDMIAFD())
		||(_bHDMI422Input!= bHDMI422Input())||(_bHDMIITCFlag!= bHDMIITCFlag())||(_bHDMIITCContent!= bHDMIITCContent())||(_bHDMIHDCPStatus!= bHDMIHDCPStatusGet()))
		{
		    
			if (_bInfoChgCnt++ > 3) 
			{		
				if((_bHDMIAspectRatio!= bHDMIAspectRatio())||(_bHDMIAFD!= bHDMIAFD()))
				{
					_bHDMIScanInfo = bHDMIScanInfo();
					_bHDMIAspectRatio= bHDMIAspectRatio();
					_bHDMIAFD = bHDMIAFD();
					printk("[HDMI RX][NFY]HDMI RX ASPECT CHG\n");  
					_HdmiRxTimingStatus = VSW_COMP_NFY_ASPECT_CHG;
					vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
				}
				_bInfoChgCnt = 0;

			if(_bHDMI422Input!= bHDMI422Input())
			{
				_bHDMI422Input = bHDMI422Input();										
				printk("[HDMI RX][NFY]HDMI RX COLOR SPACE CHG\n");  
				_HdmiRxTimingStatus = VSW_COMP_NFY_COLOR_SPACE_CHG;
				vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
				vHDMIRXColorSpaceConveter();
			}
			_bHDMIHDCPStatus = bHDMIHDCPStatusGet();
			_bHDMIITCFlag = bHDMIITCFlag();
			_bHDMIITCContent=bHDMIITCContent();

			}
		}
		else 
		{
			_bInfoChgCnt = 0;	
		}

		if (((whtemp >= (_wDviHtotal-5)) && (whtemp <= (_wDviHtotal+5))) &&
		((wvtemp >= (_wDviVTotal-2)) && (wvtemp <= (_wDviVTotal+2))))
		{
		    HDMI_RX_SYNC_LOG("[HDMI RX][NFY]_bDviMdChgCnt %d, L%d\n",_bDviMdChgCnt,__LINE__);  
			_bDviMdChgCnt= 0;
		}
		else
		{
			_bDviMdChgCnt++ ;
		}

		if (((u4DviHclkTemp >= (_wDviHclk-10)) && (u4DviHclkTemp <= (_wDviHclk+10))) &&
		((u4DviVclkTemp >= (_bDviVclk-2)) && (u4DviVclkTemp <= (_bDviVclk+2))))
		{
			_bDviHVClkChgCnt= 0;
		}
		else
		{
			_bDviHVClkChgCnt++ ;
		}


		if (((bDVIGetWidth() >= (_wDviWidth-3)) && (bDVIGetWidth() <= (_wDviWidth+3))) &&
		((bDVIGetHeight() >= (_wDviHeight-3)) && (bDVIGetHeight() <= (_wDviHeight+3))))
		{
			_bDviDeChgCnt= 0;
		}
		else
		{
			_bDviDeChgCnt++ ;
		}
		if ((dwDviPIXClock() >= (_wHDMIPixelClk-(_wHDMIPixelClk/10))) && (dwDviPIXClock()<= (_wHDMIPixelClk+(_wHDMIPixelClk/10))))
		{
			HDMI_RX_SYNC_LOG("[HDMI RX][NFY]_bDviPixClkChgCnt %d, L%d\n",_bDviPixClkChgCnt,__LINE__);	
			_bDviPixClkChgCnt= 0;
		}
		else
		{
			_bDviPixClkChgCnt++ ;
		
		}

		vHdmiGet3DInfo(FALSE);
		if((_3DInfo.HDMI_3D_Enable != _3DInfoOld.HDMI_3D_Enable)||(_3DInfo.HDMI_3D_Video_Format != _3DInfoOld.HDMI_3D_Video_Format)
		||(_3DInfo.HDMI_3D_Structure != _3DInfoOld.HDMI_3D_Structure)||(_3DInfo.HDMI_3D_EXTDATA != _3DInfoOld.HDMI_3D_EXTDATA))
		{
			HDMI_RX_SYNC_LOG("[HDMI RX]3D info change\n");  
			_b3DInfoChgCnt ++;
		}
		else
		{
			_b3DInfoChgCnt = 0;
		}			
        
		HDMI_RX_SYNC_LOG("[HDMI RX]_bDviDeChgCnt = %d,_bDviPixClkChgCnt = %d,_bHdmiMD = %d,_bHdmiMode = %d,_bDviHVClkChgCnt = %d\n",_bDviDeChgCnt,_bDviPixClkChgCnt,_bHdmiMD,_bHdmiMode,_bDviHVClkChgCnt);  
		if (((_bDviMdChgCnt > 1)  || (_bDviDeChgCnt > 1) || (_bDviPixClkChgCnt > 8)) || (_bHdmiMD !=_bHdmiMode)||(_b3DInfoChgCnt > 5)||(_bDviHVClkChgCnt > 5))
		{
			//SwitchAudioState(ASTATE_AudioOff);	
			vHdmiRxPacketDataInit();				
            bHDMIPHYReset(HDMI_RST_ALL);
			vHalResetHdmiRxTotalModule();	

			/*Because Master 1025D's resolution  change is HDMI->DVI->HDMI*/
			if(_bHdmiMD !=_bHdmiMode)
			{
				_bHdmiMD =_bHdmiMode;
			}
			_bDviChkState = DVI_WAIT_STABLE;															
			Linux_HAL_GetTime(&_rDviModeChgStartTime);


			_bDVIEQFlag=0;
			_bDviMdChgCnt = 0 ;
			if (fgIsVgaTiming(_bDviTiming ) || fgIsVideoTiming(_bDviTiming))
			{
				printk("[HDMI RX][NFY]HDMI RX RESOLUTION CHGING\n");  
				_HdmiRxTimingStatus = VSW_COMP_NFY_RESOLUTION_CHGING;
				vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
				vDviInitial();
			}

			}

			//vHDMIRxAudMainTask(); //20ms

		break;
			
		case DVI_WAIT_STABLE:
			
			HDMI_RX_SYNC_LOG("[HDMI RX][NFY]_bHDMIState = %d,L%d\n",_bHDMIState,__LINE__); 
			if (((whtemp >= (_wDviTmpHtotal - 2)) && (whtemp <= (_wDviTmpHtotal + 2))) &&
			((wvtemp >= (_wDviTmpVtotal - 2)) && (wvtemp <= (_wDviTmpVtotal + 2))))
			{
				if ((RegReadFldAlign(HDMIRX_REG_BASE+REG_ANA_STAT0, RG_HDMI_CH0_STATUS)&0x100) == 0x100)
				{
					bEQCH0_tmp = RegReadFldAlign(HDMIRX_REG_BASE+REG_ANA_STAT0, RG_HDMI_CH0_EQERR);
					bEQCH1_tmp = RegReadFldAlign(HDMIRX_REG_BASE+REG_ANA_STAT1, RG_HDMI_CH1_EQERR);
					bEQCH2_tmp = RegReadFldAlign(HDMIRX_REG_BASE+REG_ANA_STAT2, RG_HDMI_CH2_EQERR);
                    printk("==== EQ_temp: 0x%x, 0x%x, 0x%x ====\n", bEQCH0_tmp, bEQCH1_tmp, bEQCH2_tmp);
					if ((Diff(bEQCH0_tmp, _bEQCH0) >= 3) || (Diff(bEQCH1_tmp, _bEQCH1) >= 3) || (Diff(bEQCH2_tmp, _bEQCH2) >= 3))
					{
						_bEQCH0 = bEQCH0_tmp;_bEQCH1 = bEQCH1_tmp;_bEQCH2 = bEQCH2_tmp;
						bHDMIPHYReset(HDMI_RST_EQ);
                        printk("mc: Reset HDMI PHY EQ again #1.....................\n");
					}
					else if ((Diff(bEQCH0_tmp, bEQCH1_tmp) >= 3) || (Diff(bEQCH0_tmp, bEQCH2_tmp) >= 3) || (Diff(bEQCH1_tmp, bEQCH2_tmp) >= 3))
					{
						_bEQCH0 = bEQCH0_tmp;_bEQCH1 = bEQCH1_tmp;_bEQCH2 = bEQCH2_tmp;
						bHDMIPHYReset(HDMI_RST_EQ);
                        printk("mc: Reset HDMI PHY EQ again #2.....................\n");
					}
				}

				if(_bHdmiMD !=_bHdmiMode)
				{
					_bHdmiMD =_bHdmiMode;
				}

				_wDVI_WAIT_STABLE_COUNT = 3 ;//old is 100,zhiqiang modify

				if(_bDviMdChgCnt++ > _wDVI_WAIT_STABLE_COUNT)
				{
					vHalHDMIRxEnableVsyncInt(TRUE);

					if(fgHalCheckRxIsVResStable())
					{
						if(fgHalCheckRxIsVResMute())
						{
							vHalSetRxVMute();
							vHalClearRxVMute();
						}
						printk("[HDMI RX]mc:wait stable to timing search \n");
						_HdmiRxTimingStatus = VSW_COMP_NFY_RESOLUTION_CHGING;
						vVSWGetHDMIRXStatus(_HdmiRxTimingStatus);
						vDviInitial();
						_IsDviDetectDone = FALSE;
						printk("[enter %s]_IsDviDetectDone = %d\n ",__FUNCTION__,_IsDviDetectDone);
						_wDviHtotal = wDVIGetHtotal();
						_wDviVTotal = wDVIGetVtotal();
						_wHDMIPixelClk=dwDviPIXClock();		
						printk("[HDMI RX]_bDviMdChgCnt =%d  H/V Clk Chg, V:%d H:%d \n",_bDviMdChgCnt, wvtemp, whtemp);
						printk("[HDMI RX]pre H/V Clk Chg, V:%d H:%d\n", _wDviVTotal, _wDviHtotal);
						
					}
				}
			}
		else
		{
			printk("[HDMI RX]reset _bDviMdChgCnt =0\n");
			bHDMIPHYReset(HDMI_RST_RTCK);
			bHDMIPHYReset(HDMI_RST_DEEPCOLOR);			  
			_bDviMdChgCnt = 0;
		}
		_wDviTmpHtotal = whtemp;
		_wDviTmpVtotal = wvtemp;

		break;
	
	default:
	break;
	}
}

UINT8 u1GetRxCapturedTiming(void)
{
	return _u1RxCatureTiming ;	
}

BOOL fgCheckRxDetectDone(void)
{
	return _IsDviDetectDone;	
}

BOOL fgRxInputNtsc60(void)
{

    BOOL fgNTSC60 = FALSE;
	
	if(fgIsHdmiRepeater())
	{
		if(_u4VsyncPeriod>((41666+41708)/2))
			  fgNTSC60 = FALSE;	//23
		else if(_u4VsyncPeriod>((40000+41666)/2))
			  fgNTSC60= TRUE;      //24
		else if(_u4VsyncPeriod>((33366+40000)/2))
			  fgNTSC60 = FALSE;	//25
		else if(_u4VsyncPeriod>((33333+33366)/2))
			  fgNTSC60 = FALSE;	//29
		else if(_u4VsyncPeriod>((20000+33333)/2))
			  fgNTSC60= TRUE;      //30
		else if(_u4VsyncPeriod>((16683+20000)/2))
			  fgNTSC60 = FALSE;	//50
		else if(_u4VsyncPeriod>((16668+16683)/2))
			  fgNTSC60 = FALSE;    //59.94
		else
			  fgNTSC60= TRUE;      //60
	}

    return(fgNTSC60);
}

void vVSWGetRXInfo(INPUT_DEVICE_INFO_T *pv_get_info)
{ 
    if((_bDviTiming < MODE_MAX) && (_bDviTiming != MODE_NOSIGNAL))
    {
        pv_get_info->fgIsTimingOk = TRUE;
    }
    else
    {
        pv_get_info->fgIsTimingOk = FALSE;
    }
 
    pv_get_info->eDeviceId = VIN_HDMI_1;
    pv_get_info->eInputRes = _bVDITiming;
   
    if(bHDMI422Input()==1)
    {
		printk("[HDMI RX]YC422_24BIT\n");
		pv_get_info->ePinType = YC422_24BIT;
    }
    else
    {
        printk("[HDMI RX]YC444_36BIT\n");
        pv_get_info->ePinType = YC444_36BIT;
    }


    pv_get_info->eInputMode = FMT_601_MODE;

	if(((_bHDMIAFD==0x08)||(_bHDMIAFD==0x09))&&(_bHDMIAspectRatio==0x01))
    {
        pv_get_info->eAspectRatio =SRC_ASP_4_3_FULL;
    }
    else if(((_bHDMIAFD==0x08)||(_bHDMIAFD==0x0A)||(_bHDMIAFD==0x0B))&&(_bHDMIAspectRatio==0x02))
    {
        pv_get_info->eAspectRatio =SRC_ASP_16_9_FULL;
    }  
    else if((_bHDMIAFD==0x09)&&(_bHDMIAspectRatio==0x02))
    {
        pv_get_info->eAspectRatio =SRC_ASP_16_9_PS;
    }
    else if((_bHDMIAFD==0x0A)&&(_bHDMIAspectRatio==0x01))
    {
        pv_get_info->eAspectRatio =SRC_ASP_16_9_LB;
    }
    else if((_bHDMIAFD==0x0B)&&(_bHDMIAspectRatio==0x01))
    {
        pv_get_info->eAspectRatio =SRC_ASP_14_9_LB;
    }
    else
    	pv_get_info->eAspectRatio =SRC_ASP_UNDEFINED;


    pv_get_info->fgIsJpeg =0; 
    pv_get_info->fgIsCinema =0;
	if(_bRse640x480PEnable == 2)
	{
	  pv_get_info->fgVgaIsCeaType = 0;
	}
	else
	{
	  pv_get_info->fgVgaIsCeaType = 1;
	}		
	if(fgRxInputNtsc60())
	{
	  pv_get_info->fgNTSC60= 1;
	}
	else
	{
	  pv_get_info->fgNTSC60 = 0;
	}		
	

 }

void vShowRxResoInfoStatus(void)
{

	if(_bDviTiming < MODE_MAX)
	  printk("[HDMI RX] Resolution = %d, %s\n", _bDviTiming,szRxResStr[_bDviTiming]);	
	else if(_bDviTiming == MODE_MAX)
	  printk("[HDMI RX]Resolution = MODE_MAX\n"); 
	else if(_bDviTiming == MODE_DE_MODE)
	  printk("[HDMI RX]Resolution = MODE_DE_MODE\n"); 	
	else if(_bDviTiming == MODE_NODISPLAY)
	  printk("[HDMI RX]Resolution = MODE_NODISPLAY\n");	
	else if(_bDviTiming == MODE_NOSUPPORT)
	  printk("[HDMI RX]Resolution = MODE_NOSUPPORT\n");
	else
	  printk("[HDMI RX]Resolution = UNKNOW\n");	

	printk("[HDMI RX]Htotal = %d \n",_wDviHtotal);	
	printk("[HDMI RX]VTotal = %d \n",_wDviVTotal);

	printk("[HDMI RX]_wDviHclk = %d \n",_wDviHclk);	
	printk("[HDMI RX]_bDviVclk = %d \n",_bDviVclk);

	printk("[HDMI RX]_wDviWidth = %d x\n",_wDviWidth);
	printk("[HDMI RX]_wDviHeight = %d \n",_wDviHeight);

}

void vHdmiRxDviStatus(void)
{
	printk("[HDMI RX]Digital Video STATUS \n");
	printk("[HDMI RX]_bDviModeDetState = %s\n",cDviModeDetState[_bDviModeDetState]);
	printk("[HDMI RX]_bDviChkState = %s\n",cDviChkState[_bDviChkState]);
	printk("[HDMI RX]_wDviHclk = %d \n",_wDviHclk);	
	printk("[HDMI RX]_bDviVclk = %d \n",_bDviVclk);
	printk("[HDMI RX]_wDviHtotal = %d \n",_wDviHtotal);	
	printk("[HDMI RX]_wDviVTotal = %d \n",_wDviVTotal);
	printk("[HDMI RX]_wDviWidth = %d x\n",_wDviWidth);
	printk("[HDMI RX]_wDviHeight = %d \n",_wDviHeight);
	
	if(_u4VsyncPeriod>((41666+41708)/2))
		  printk("[HDMI RX]VSYNC Frame rate = 23.976\n");	
	else if(_u4VsyncPeriod>((40000+41666)/2))
		  printk("[HDMI RX]VSYNC Frame rate = 24\n");	
	else if(_u4VsyncPeriod>((33366+40000)/2))
		  printk("[HDMI RX]VSYNC Frame rate = 25\n");		
	else if(_u4VsyncPeriod>((33333+33366)/2))
		  printk("[HDMI RX]VSYNC Frame rate = 29.997\n");		
	else if(_u4VsyncPeriod>((20000+33333)/2))
		  printk("[HDMI RX]VSYNC Frame rate = 30\n");	
	else if(_u4VsyncPeriod>((16683+20000)/2))
		  printk("[HDMI RX]VSYNC Frame rate = 50\n");		
	else if(_u4VsyncPeriod>((16668+16683)/2))
		  printk("[HDMI RX]VSYNC Frame rate = 59.94\n");	
	else
		  printk("[HDMI RX]VSYNC Frame rate = 60\n");	
	
	if(_bDviTiming < MODE_MAX)
	  printk("[HDMI RX]_bDviTiming = %d, %s\n", _bDviTiming,szRxResStr[_bDviTiming]);	
	else if(_bDviTiming == MODE_MAX)
	  printk("[HDMI RX]_bDviTiming = MODE_MAX\n");	
	else if(_bDviTiming == MODE_DE_MODE)
	  printk("[HDMI RX]_bDviTiming = MODE_DE_MODE\n");		
	else if(_bDviTiming == MODE_NODISPLAY)
	  printk("[HDMI RX]_bDviTiming = MODE_NODISPLAY\n");	
	else if(_bDviTiming == MODE_NOSUPPORT)
	  printk("[HDMI RX]_bDviTiming = MODE_NOSUPPORT\n");
	else if(_bDviTiming == MODE_WAIT)
	  printk("[HDMI RX]_bDviTiming = MODE_WAIT\n");	
	else
	  printk("[HDMI RX]_bDviTiming = UNKNOW\n");	
	
	printk("[HDMI RX]_bDviTiming = %d \n",_bDviTiming);
	printk("[HDMI RX]_bDviExtSrc = %d \n",_bDviExtSrc); 
    vHdmiGet3DInfo(TRUE);			
	 if(_bVDITiming < RES_MODE_NUM)
	  printk("[HDMI RX]_bVDITiming = %d, %s\n", _bVDITiming,szResStr[_bVDITiming]);
	else
	 printk("[HDMI RX]_bVDITiming = %d \n", _bVDITiming);
}

UINT8 _bForce2DFlag = 0;//repeater should be 0

UINT8 bHDMI3DPacketVaild(void)
{
    if (_3DInfo.HDMI_3D_Enable)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

UINT16 wDviInputHTotal(void)
{
    UINT16 wHTotal = 0;
    if (bHDMI3DPacketVaild())
    {
        wHTotal = wHDMI3DGetHVParameter(3);
    }
    else
    {
        if (_bDviTiming == MODE_DE_MODE)
        {
            wHTotal =  _wDviHtotal ;
        }
        else if (_bDviTiming < MAX_TIMING_FORMAT)
        {
            wHTotal =  Get_VGAMODE_IHTOTAL(_bDviTiming);
        }
    }
    return wHTotal;
}

UINT16 wDviInputVTotal(void)
{
    UINT16 wVTotal = 0;

    if (bHDMI3DPacketVaild())
    {
        wVTotal = wHDMI3DGetHVParameter(1);
    }
    else
    {
        if (_bDviTiming == MODE_DE_MODE)
        {
            wVTotal = _wDviVTotal ;
        }
        else if (_bDviTiming < MAX_TIMING_FORMAT)
        {
            wVTotal =  Get_VGAMODE_IVTOTAL(_bDviTiming);
        }
    }
    return wVTotal;
}



UINT16 wHDMI3DGetHVParameter(UINT8 bType)
{
    HDMI_3D_INFOFRAME * pInfo;
    pInfo = &_3DInfo;

    if (bType == 1)
    {
        switch (pInfo->HDMI_3D_Structure)
        {
            case HDMI_3D_Structure_FramePacking:
                if ((_bDviTiming == MODE_3D_720p_50) || (_bDviTiming == MODE_3D_720p_60) || (_bDviTiming == MODE_3D_1080p_24) || (_bDviTiming == MODE_3D_1080I_60_FRAMEPACKING))
                {
                    return Get_VGAMODE_IVTOTAL(_bDviTiming) >> 1;
                }
                else
                {
                    if (fgHDMIinterlaced())
                    {
                        return wDVIGetVtotal();
                    }
                    else
                    {
                        return wDVIGetVtotal() >> 1;
                    }
                }

            case HDMI_3D_Structure_LineAlternative:
                if (1 == _bForce2DFlag)
                {
                    printk("decoder can not force LineAlternative from 3D to 2D mode\n");
                }
                return wDVIGetVtotal() >> 1 ;
            case HDMI_3D_Structure_SideBySideFull:
            case HDMI_3D_Structure_SideBySideHalf:
                if (fgHDMIinterlaced())
                {
                    return wDVIGetVtotal() << 1;
                }
                else
                {
                    return wDVIGetVtotal();
                }
            case HDMI_3D_Structure_TopBottom:
                if (1 == _bForce2DFlag)
                {
                    return wDVIGetVtotal() / 2;
                }
                else
                {
                    return wDVIGetVtotal();
                }
            default:
                return wDVIGetVtotal();
        }

    }
    if (bType == 2)
    {
        switch (pInfo->HDMI_3D_Structure)
        {

            case HDMI_3D_Structure_FramePacking:
                if ((_bDviTiming == MODE_3D_720p_50) || (_bDviTiming == MODE_3D_720p_60) || (_bDviTiming == MODE_3D_1080p_24))
                {
                    return (Get_VGAMODE_IPV_LEN(_bDviTiming) - Get_VGAMODE_IPV_STA(_bDviTiming)) >> 1;
                }
                else if (_bDviTiming == MODE_3D_1080I_60_FRAMEPACKING)
                {
                    return (Get_VGAMODE_IPV_LEN(_bDviTiming) - Get_VGAMODE_IPV_STA(_bDviTiming) * 3 - 2) >> 1;//22(real), 23(by decoder), 22(by decoder, this value is in timing table), 23(by decoder)
                }
                else
                {
                    if (fgHDMIinterlaced())
                    {
                        return bDVIGetHeight();
                    }
                    else
                    {
                        return bDVIGetHeight() >> 1;
                    }
                }

            case HDMI_3D_Structure_LineAlternative:
                return bDVIGetHeight() >> 1 ;
            case HDMI_3D_Structure_SideBySideFull:
            case HDMI_3D_Structure_SideBySideHalf:
                if (fgHDMIinterlaced())
                {
                    return bDVIGetHeight() << 1 ;
                }
                else
                {
                    return bDVIGetHeight();

                }
            case HDMI_3D_Structure_TopBottom:
                if (1 == _bForce2DFlag)
                {
                    return bDVIGetHeight() / 2;
                }
                else
                {
                    return bDVIGetHeight();
                }
            default:
                return bDVIGetHeight();
        }
    }
    if (bType == 3)//Htotal
    {
        switch (pInfo->HDMI_3D_Structure)
        {
            case HDMI_3D_Structure_FramePacking:
                if ((_bDviTiming == MODE_3D_720p_50) || (_bDviTiming == MODE_3D_720p_60) || (_bDviTiming == MODE_3D_1080p_24))
                {
                    return Get_VGAMODE_IHTOTAL(_bDviTiming);
                }
                else
                {
                    return wDVIGetHtotal();
                }

            case HDMI_3D_Structure_LineAlternative:
                return wDVIGetHtotal();
            case HDMI_3D_Structure_SideBySideFull:
            case HDMI_3D_Structure_SideBySideHalf:
                if (1 == _bForce2DFlag)
                {
                    return wDVIGetHtotal() / 2;
                }
                else
                {
                    return wDVIGetHtotal();
                }
            case HDMI_3D_Structure_TopBottom:
                return wDVIGetHtotal();
            default:
                return wDVIGetHtotal();
        }

    }
    if (bType == 4)
    {
        switch (pInfo->HDMI_3D_Structure)
        {
            case HDMI_3D_Structure_FramePacking:
            case HDMI_3D_Structure_LineAlternative:
                return bDVIGetWidth();
            case HDMI_3D_Structure_SideBySideFull:
            case HDMI_3D_Structure_SideBySideHalf:
                if (1 == _bForce2DFlag)
                {
                    return bDVIGetWidth() / 2;
                }
                else
                {
                    return bDVIGetWidth();
                }
            case HDMI_3D_Structure_TopBottom:
                return bDVIGetWidth();
            default:
                return bDVIGetWidth();
        }
    }
    if (bType == 5)
    {
        switch (pInfo->HDMI_3D_Structure)
        {
            case HDMI_3D_Structure_FramePacking:
                if (1 == _bForce2DFlag)
                {
                    if (_bDviTiming == MODE_DE_MODE)
                    {
                        return _bDviVclk;
                    }
                    else if (_bDviTiming == MODE_3D_1080I_60_FRAMEPACKING)
                    {
                        return Get_VGAMODE_IVF(_bDviTiming)*2;
                    }
                    else if(_bDviTiming<MODE_MAX)
                    {
                        return Get_VGAMODE_IVF(_bDviTiming);
                    }
                }
                else
                {
                    if (_bDviTiming == MODE_DE_MODE)
                    {
                        return _bDviVclk * 2;
                    }
                    else if (_bDviTiming == MODE_3D_1080I_60_FRAMEPACKING)
                    {
                        return Get_VGAMODE_IVF(_bDviTiming)*4;
                    }
                    else if(_bDviTiming<MODE_MAX)
                    {
                        return Get_VGAMODE_IVF(_bDviTiming)*2;
                    }
                }
            case HDMI_3D_Structure_LineAlternative:
            case HDMI_3D_Structure_SideBySideFull:
            case HDMI_3D_Structure_SideBySideHalf:
            case HDMI_3D_Structure_TopBottom:
                return _bDviVclk;
            default:
                return _bDviVclk;
        }
    }
    return 0;
}

HDMI_RESOLUTION_MODE_T u4GetHdmiRxRes(void)
{
  #if 1//(DRV_SUPPORT_HDMI_RX)
	  return _bVDITiming;
  #else
	  return 0;
  #endif
}


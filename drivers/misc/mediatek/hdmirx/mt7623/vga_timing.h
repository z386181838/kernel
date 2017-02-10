#ifndef _VGA_TIMING_H_
#define _VGA_TIMING_H_

#include "video_in_if.h"
#include "typedef.h"

#define HDTV_SEARCH_START	1
#define HDTV_SEARCH_END		(bHdtvTimings-1)

#define VGA_SEARCH_START		(bHdtvTimings)  //1
#define VGA_SEARCH_END		(bAllTimings-1)

#define DVI_SEARCH_START		(bHdtvTimings)
#define DVI_SEARCH_END		(bHdtvTimings+bVgaTimings-1)

#define MAX_TIMING_FORMAT	(bAllTimings)

#define fgIsUserModeTiming(bMode) (((bMode) >= bUserVgaTimingBegin) && ((bMode) < bAllTimings))
#define fgIsVgaTiming(bMode)   ((((bMode) >= DVI_SEARCH_START) && ((bMode) <= DVI_SEARCH_END)) || ((bMode) == MODE_DE_MODE) || fgIsUserModeTiming(bMode))
#define fgIsVideoTiming(bMode) (((bMode) >= HDTV_SEARCH_START) && ((bMode) <= HDTV_SEARCH_END))
#define fgIsValidTiming(bMode) (fgIsVgaTiming(bMode) || fgIsVideoTiming(bMode) ||fgIsUserModeTiming(bMode))

typedef struct VGAMODE		// 14 bytes
{
	UINT16 IHF; // Horizontal Frequency for timing search
	UINT8 IVF; // Vertical Frequency for timing search
	UINT16 ICLK; // Pixel Frequency
	UINT16 IHTOTAL; // H Total
	UINT16 IVTOTAL; // V Total 
	UINT16 IPH_SYNCW; // H Sync Width
	UINT16 IPH_WID; // H Resolution
	UINT16 IPH_BP; // H Back Porch
	UINT16 IPV_STA; // V Back Porch + Sync Width
	UINT16 IPV_LEN; // V Resolution
	UINT16 COMBINE; // ??
}  VGAMODE ; // using __attribute__((packed)) make armcc --gnu  internal fault ?

enum
{
    MODE_NOSIGNAL = 0,        // No signal
    MODE_525I_OVERSAMPLE = 1,      //SDTV 
    MODE_625I_OVERSAMPLE,       //
    MODE_480P_OVERSAMPLE,       //SDTV
    MODE_576P_OVERSAMPLE,
    MODE_720p_50,               //HDTV 
    MODE_720p_60,               //HDTV   
    MODE_1080i_48,              //HDTV  
    MODE_1080i_50,              //HDTV  
    MODE_1080i,                 //HDTV
    MODE_1080p_24,              //HDTV   10
    MODE_1080p_25,
    MODE_1080p_30,
    MODE_1080p_50,              //HDTV 
    MODE_1080p_60,
    MODE_525I,
    MODE_625I,
    MODE_480P,
    MODE_576P,    
    MODE_720p_24,
    MODE_720p_25,                                 //20
    MODE_720p_30,        
    MODE_240P,
    MODE_540P,
    MODE_288P,    
    MODE_480P_24,    
    MODE_480P_30,        
    MODE_576P_25,    
    MODE_3D_720p_50,
    MODE_3D_720p_60,
    MODE_3D_1080p_24,                                   //30
    MODE_3D_1080I_60_FRAMEPACKING,  
    MODE_3D_1080I_50_FRAMEPACKING,    
    MODE_3D_1080P60HZ,
    MODE_3D_1080P50HZ,
    MODE_3D_1080P30HZ,
    MODE_3D_1080P25HZ,
    MODE_3D_720P30HZ, 
    MODE_3D_720P25HZ, 
    MODE_3D_720P24HZ,
    MODE_3D_576P50HZ,                                   // 40
    MODE_3D_576I50HZ, 
    MODE_3D_480P60HZ, 
    MODE_3D_480I60HZ, 
    MODE_REVERSE1,
    MODE_REVERSE2,    
    MODE_HDMI_640_480P = 46,
    MODE_2160P_30HZ,
    MODE_2160P_25HZ,
    MODE_2160P_24HZ,
    MODE_2161P_24HZ,                                     // 50
    MODE_2160P_50HZ,
    MODE_2160P_60HZ,
    MODE_2161P_50HZ,
    MODE_2161P_60HZ,                                     // 54
	
    MODE_MAX,
    MODE_DE_MODE = 251,
    MODE_NODISPLAY = 252,
    MODE_NOSUPPORT = 253,      // Signal out of range
    MODE_WAIT = 254
};

UINT16 Get_HDMIMODE_IHF(UINT8 mode) ;
UINT8 Get_HDMIMODE_IVF(UINT8 mode) ;
UINT16 Get_HDMIMODE_ICLK(UINT8 mode) ;
UINT16 Get_HDMIMODE_IHTOTAL(UINT8 mode) ;
UINT16 Get_HDMIMODE_IVTOTAL(UINT8 mode) ;
UINT16 Get_HDMIMODE_IPH_STA(UINT8 mode) ;
UINT16 Get_HDMIMODE_IPH_SYNCW(UINT8 mode) ;
UINT16 Get_HDMIMODE_IPH_WID(UINT8 mode) ;
UINT16 Get_HDMIMODE_IPH_BP(UINT8 mode) ;
UINT8 Get_HDMIMODE_IPV_STA(UINT8 mode) ;
UINT16 Get_HDMIMODE_IPV_LEN(UINT8 mode) ;
UINT16 Get_HDMIMODE_COMBINE(UINT8 mode) ;
UINT8 Get_HDMIMODE_OverSample(UINT8 mode) ;

#endif



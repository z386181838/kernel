#ifndef _VGA_TABLE_H_
#define _VGA_TABLE_H_
#include "typedef.h"


#define  USERMODE_TIMING 8
extern const UINT8 bHdtvTimings;
extern const UINT8 bVgaTimings;
extern const UINT8 bUserVgaTimings;
extern const UINT8 bAllTimings;

extern const UINT8 bUserVgaTimingBegin;

typedef struct VGA_USRMODE	
{
	UINT16 vlen:11;	/* reference STA12 */
	UINT16 hsync_wvar:4;
	UINT16 vpol:1;	/* reference STA12 */
	UINT16 hlen:11;	/* reference STA12 */
	UINT16 hlen_var:4;
	UINT16 hpol:1;	/* reference STA12 */
	UINT8 hsync_w;	
	UINT8 isCVTRB:1;
	UINT8 unused:3;
	UINT8 forceXY:2;	/* force XY resolution*/
	UINT8 override:1; /* override preset timing */
	UINT8 id:1;		/*inverse to change return mode number */
} VGA_USRMODE;

typedef struct VGA_USRMODE_EXT
{
	UINT16	h_res;	//orig input h resolotion
} VGA_USRMODE_EXT;

extern VGA_USRMODE rVgaUsrEEP[USERMODE_TIMING];		//both  EEP & RAM
extern VGA_USRMODE_EXT rVgaUsrExt[USERMODE_TIMING]; //only on RAM

// VGA Mode
extern UINT16 Get_VGAMODE_IHF(UINT8 mode) ;
extern UINT8 Get_VGAMODE_IVF(UINT8 mode) ;
extern UINT16 Get_VGAMODE_ICLK(UINT8 mode) ;
extern UINT16 Get_VGAMODE_IHTOTAL(UINT8 mode) ;
extern UINT16 Get_VGAMODE_IVTOTAL(UINT8 mode) ;
extern UINT16 Get_VGAMODE_IPH_STA(UINT8 mode) ;
extern UINT16 Get_VGAMODE_IPH_SYNCW(UINT8 mode) ;
extern UINT16 Get_VGAMODE_IPH_WID(UINT8 mode) ;
extern UINT16 Get_VGAMODE_IPH_BP(UINT8 mode) ;
extern UINT8 Get_VGAMODE_IPV_STA(UINT8 mode) ;
extern UINT16 Get_VGAMODE_IPV_LEN(UINT8 mode) ;
extern UINT16 Get_VGAMODE_COMBINE(UINT8 mode) ;
extern UINT8 Get_VGAMODE_OverSample(UINT8 mode) ;


//#define Get_VGAMODE_OverSample(bMode) (Get_VGAMODE_COMBINE(bMode)&0x01) //oversample
#define Get_VGAMODE_HSyncWidthChk(bMode) ((Get_VGAMODE_COMBINE(bMode)>>1)&0x01)
#define Get_VGAMODE_AmbiguousH(bMode) ((Get_VGAMODE_COMBINE(bMode)>>2)&0x01)
#define Get_VGAMODE_VPol(bMode) ((Get_VGAMODE_COMBINE(bMode)>>3)&0x01)
#define Get_VGAMODE_HPol(bMode) ((Get_VGAMODE_COMBINE(bMode)>>4)&0x01)
#define Get_VGAMODE_VSyncWidthChk(bMode) ((Get_VGAMODE_COMBINE(bMode)>>5)&0x01)
#define Get_VGAMODE_INTERLACE(bMode) ((Get_VGAMODE_COMBINE(bMode)>>6)&0x01)
#define Get_VGAMODE_PolChk(bMode) ((Get_VGAMODE_COMBINE(bMode)>>7)&0x01)
#define Get_HDMIMODE_SupportVideo(bMode) ((Get_VGAMODE_COMBINE(bMode)>>8)&0x01)
#define Get_VGAMODE_VgaDisabled(bMode) ((Get_VGAMODE_COMBINE(bMode)>>9)&0x01)
#define Get_VGAMODE_YpbprDisabled(bMode) ((Get_VGAMODE_COMBINE(bMode)>>10)&0x01)

#endif

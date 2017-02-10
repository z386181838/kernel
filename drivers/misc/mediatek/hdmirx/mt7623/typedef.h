/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/

/******************************************************************************
*[File]             typedef.h
*[Version]          v0.1
*[Revision Date]    2010-01-04
*[Author]           Kenny Hsieh
*[Description]
*    source file for global varabile and function in av_d directory
*
*
******************************************************************************/


#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


#define _XDATA

#define AUDFS_24KHz  6
#define AUDFS_48KHz  2
#define AUDFS_96KHz  10
#define AUDFS_192KHz 14

typedef unsigned char BYTE;	
typedef unsigned char u8;		
typedef unsigned short u16;	
typedef unsigned int u32;	
typedef unsigned char BOOL;

typedef unsigned long long UINT64;

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
typedef unsigned short  USHORT;
typedef signed char     INT8;
typedef signed short    INT16;
typedef signed int      INT32;
typedef unsigned int    DWORD;
typedef void            VOID;
typedef unsigned char   BYTE;
typedef float           FLOAT;
typedef char            CHAR;
typedef	unsigned char	cBYTE;
typedef unsigned char UCHAR;

typedef short SHORT, *PSHORT ;
typedef unsigned long ULONG, *PULONG ;
typedef unsigned long dword, *pdword ;


typedef struct _RX_REG_AUDIO_CHSTS {
 BYTE rev :1;
 BYTE ISLPCM :1;
 BYTE CopyRight : 1;
 BYTE AdditionFormatInfo :3;
 BYTE ChannelStatusMode :2;
 BYTE CategoryCode;
 BYTE SourceNumber :4;
 BYTE ChannelNumber :4;
 BYTE SamplingFreq :4;
 BYTE ClockAccuary :2;
 BYTE rev2 : 2;
 BYTE WorldLen : 4;
 BYTE OriginalSamplingFreq :4;
} RX_REG_AUDIO_CHSTS;



/////////////////////////////////////////////////////////////////////
// Packet and Info Frame definition and datastructure.
/////////////////////////////////////////////////////////////////////

#define VENDORSPEC_INFOFRAME_TYPE 0x01
#define AVI_INFOFRAME_TYPE  0x02
#define SPD_INFOFRAME_TYPE 0x03
#define AUDIO_INFOFRAME_TYPE 0x04
#define MPEG_INFOFRAME_TYPE 0x05

#define VENDORSPEC_INFOFRAME_VER 0x01
#define AVI_INFOFRAME_VER  0x02
#define SPD_INFOFRAME_VER 0x01
#define AUDIO_INFOFRAME_VER 0x01
#define MPEG_INFOFRAME_VER 0x01

#define VENDORSPEC_INFOFRAME_LEN 8
#define AVI_INFOFRAME_LEN 13
#define SPD_INFOFRAME_LEN 25
#define AUDIO_INFOFRAME_LEN 10
#define MPEG_INFOFRAME_LEN 10

#define ACP_PKT_LEN 9
#define ISRC1_PKT_LEN 16
#define ISRC2_PKT_LEN 16

typedef enum _Audio_State_Type {
    ASTATE_AudioOff = 0,
    ASTATE_RequestAudio ,
    ASTATE_ResetAudio,
    ASTATE_WaitForReady,
    ASTATE_AudioOn ,
    ASTATE_Reserved
} Audio_State_Type ;


#define SUCCESS 0
#define FAIL -1

#define ON 1
#define OFF 0

typedef union _AVI_InfoFrame
{
    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        BYTE Scan:2 ;
        BYTE BarInfo:2 ;
        BYTE ActiveFmtInfoPresent:1 ;
        BYTE ColorMode:2 ;
        BYTE FU1:1 ;

        BYTE ActiveFormatAspectRatio:4 ;
        BYTE PictureAspectRatio:2 ;
        BYTE Colorimetry:2 ;

        BYTE Scaling:2 ;
        BYTE FU2:6 ;

        BYTE VIC:7 ;
        BYTE FU3:1 ;

        BYTE PixelRepetition:4 ;
        BYTE FU4:4 ;

        SHORT Ln_End_Top ;
        SHORT Ln_Start_Bottom ;
        SHORT Pix_End_Left ;
        SHORT Pix_Start_Right ;
    } info ;
    struct {
        BYTE AVI_HB[3] ;
        BYTE AVI_DB[AVI_INFOFRAME_LEN] ;
    } pktbyte ;
} AVI_InfoFrame ;

typedef union _Audio_InfoFrame {

    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        BYTE AudioChannelCount:3 ;
        BYTE RSVD1:1 ;
        BYTE AudioCodingType:4 ;

        BYTE SampleSize:2 ;
        BYTE SampleFreq:3 ;
        BYTE Rsvd2:3 ;

        BYTE FmtCoding ;

        BYTE SpeakerPlacement ;

        BYTE Rsvd3:3 ;
        BYTE LevelShiftValue:4 ;
        BYTE DM_INH:1 ;
    } info ;

    struct {
        BYTE AUD_HB[3] ;
        BYTE AUD_DB[AUDIO_INFOFRAME_LEN] ;
    } pktbyte ;

} Audio_InfoFrame ;

typedef union _MPEG_InfoFrame {
    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        ULONG MpegBitRate ;

        BYTE MpegFrame:2 ;
        BYTE Rvsd1:2 ;
        BYTE FieldRepeat:1 ;
        BYTE Rvsd2:3 ;
    } info ;
    struct {
        BYTE MPG_HB[3] ;
        BYTE MPG_DB[MPEG_INFOFRAME_LEN] ;
    } pktbyte ;
} MPEG_InfoFrame ;

// Source Product Description
typedef union _SPD_InfoFrame {
    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        char VN[8] ; // vendor name character in 7bit ascii characters
        char PD[16] ; // product description character in 7bit ascii characters
        BYTE SourceDeviceInfomation ;
    } info ;
    struct {
        BYTE SPD_HB[3] ;
        BYTE SPD_DB[SPD_INFOFRAME_LEN] ;
    } pktbyte ;
} SPD_InfoFrame ;

typedef enum _SYS_STATUS {
    ER_SUCCESS = 0,
    ER_FAIL,
    ER_RESERVED
} SYS_STATUS ;

void HDMIRX_DBG_Init(void);

#endif // _TYPEDEF_H_

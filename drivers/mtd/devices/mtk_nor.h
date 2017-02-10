/*----------------------------------------------------------------------------*
 * No Warranty                                                                *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MTK with respect to any MTK *
 * Deliverables or any use thereof, and MTK Deliverables are provided on an   *
 * "AS IS" basis.  MTK hereby expressly disclaims all such warranties,        *
 * including any implied warranties of merchantability, non-infringement and  *
 * fitness for a particular purpose and any warranties arising out of course  *
 * of performance, course of dealing or usage of trade.  Parties further      *
 * acknowledge that Company may, either presently and/or in the future,       *
 * instruct MTK to assist it in the development and the implementation, in    *
 * accordance with Company's designs, of certain softwares relating to        *
 * Company's product(s) (the "Services").  Except as may be otherwise agreed  *
 * to in writing, no warranties of any kind, whether express or implied, are  *
 * given by MTK with respect to the Services provided, and the Services are   *
 * provided on an "AS IS" basis.  Company further acknowledges that the       *
 * Services may contain errors, that testing is important and Company is      *
 * solely responsible for fully testing the Services and/or derivatives       *
 * thereof before they are used, sublicensed or distributed.  Should there be *
 * any third party action brought against MTK, arising out of or relating to  *
 * the Services, Company agree to fully indemnify and hold MTK harmless.      *
 * If the parties mutually agree to enter into or continue a business         *
 * relationship or other arrangement, the terms and conditions set forth      *
 * hereunder shall remain effective and, unless explicitly stated otherwise,  *
 * shall prevail in the event of a conflict in the terms in any agreements    *
 * entered into between the parties.                                          *
 *---------------------------------------------------------------------------*/
/******************************************************************************
* [File]			mtk_nor.h
* [Version]			v1.0
* [Revision Date]	2011-05-04
* [Author]			Shunli Wang, shunli.wang@mediatek.inc, 82896, 2012-04-27
* [Description]
*	SPI-NOR Driver Header File
* [Copyright]
*	Copyright (C) 2011 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

//#include "x_typedef.h"

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

#define SFLASH_POLLINGREG_COUNT     500000
#define SFLASH_WRITECMD_TIMEOUT     100000
#define SFLASH_WRITEBUSY_TIMEOUT    500000
#define SFLASH_ERASESECTOR_TIMEOUT  200000
#define SFLASH_CHIPERASE_TIMEOUT    500000

#define SFLASH_WRITE_PROTECTION_VAL 0x00
#define SFLASH_MAX_DMA_SIZE      (1024*8)

#define REGION_RANK_FIRST_FLASH           (0x1<<0)
#define REGION_RANK_FIRST_WHOLE_FLASH     (0x1<<1)
#define REGION_RANK_SECOND_FLASH          (0x1<<2)
#define REGION_RANK_SECOND_WHOLE_FLASH    (0x1<<3)
#define REGION_RANK_TWO_WHOLE_FLASH       (REGION_RANK_FIRST_FLASH | \
	                                       REGION_RANK_FIRST_WHOLE_FLASH | \
	                                       REGION_RANK_SECOND_FLASH | \
	                                       REGION_RANK_SECOND_WHOLE_FLASH)




//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
typedef unsigned int		UINT32;
typedef unsigned short		UINT16;
typedef unsigned char		UINT8;
typedef signed int		INT32;
typedef signed short		INT16;
typedef signed char		INT8;
typedef void			VOID;
typedef char			CHAR;

#define VECTOR_FLASH		80


#define SFLASH_BASE         	(0x0000000)


#ifdef CONFIG_ARCH_MT7623
 #define SFLASH_REG_BASE		0x11014000
 #define SYS_BASE		0x10000000
#else
 #error "please define project name"
#endif

#define SFLASH_CMD_REG          ((UINT32)0x00)
#define SFLASH_CNT_REG          ((UINT32)0x04)
#define SFLASH_RDSR_REG         ((UINT32)0x08)
#define SFLASH_RDATA_REG        ((UINT32)0x0C)

#define SFLASH_RADR0_REG        ((UINT32)0x10)
#define SFLASH_RADR1_REG        ((UINT32)0x14)
#define SFLASH_RADR2_REG        ((UINT32)0x18)
#define SFLASH_WDATA_REG        ((UINT32)0x1C)
#define SFLASH_PRGDATA0_REG     ((UINT32)0x20)
#define SFLASH_PRGDATA1_REG     ((UINT32)0x24)
#define SFLASH_PRGDATA2_REG     ((UINT32)0x28)
#define SFLASH_PRGDATA3_REG     ((UINT32)0x2C)
#define SFLASH_PRGDATA4_REG     ((UINT32)0x30)
#define SFLASH_PRGDATA5_REG     ((UINT32)0x34)
#define SFLASH_SHREG0_REG       ((UINT32)0x38)
#define SFLASH_SHREG1_REG       ((UINT32)0x3C)
#define SFLASH_SHREG2_REG       ((UINT32)0x40)
#define SFLASH_SHREG3_REG       ((UINT32)0x44)
#define SFLASH_SHREG4_REG       ((UINT32)0x48)
#define SFLASH_SHREG5_REG       ((UINT32)0x4C)
#define SFLASH_SHREG6_REG       ((UINT32)0x50)
#define SFLASH_SHREG7_REG       ((UINT32)0x54)
#define SFLASH_SHREG8_REG       ((UINT32)0x58)
#define SFLASH_SHREG9_REG       ((UINT32)0x5C)
#define SFLASH_FLHCFG_REG      	((UINT32)0x84)
#define SFLASH_PP_DATA_REG      ((UINT32)0x98)
#define SFLASH_PREBUF_STUS_REG  ((UINT32)0x9C)
#define SFLASH_SF_INTRSTUS_REG  ((UINT32)0xA8)
#define SFLASH_SF_INTREN_REG    ((UINT32)0xAC)
#define SFLASH_SF_TIME_REG      ((UINT32)0x94)
#define SFLASH_CHKSUM_CTL_REG   ((UINT32)0xB8)
#define SFLASH_CHECKSUM_REG     ((UINT32)0xBC)
#define SFLASH_CMD2_REG     	((UINT32)0xC0)
#define SFLASH_WRPROT_REG       ((UINT32)0xC4)
#define SFLASH_RADR3_REG        ((UINT32)0xC8)
#define SFLASH_DUAL_REG         ((UINT32)0xCC)
#define SFLASH_DELSEL0_REG    	((UINT32)0xA0)
#define SFLASH_DELSEL1_REG    	((UINT32)0xA4)
#define SFLASH_DELSEL2_REG    	((UINT32)0xD0)
#define SFLASH_DELSEL3_REG    	((UINT32)0xD4)
#define SFLASH_DELSEL4_REG    	((UINT32)0xD8)

#define SFLASH_CFG1_REG         ((UINT32)0x60)
#define SFLASH_CFG2_REG         ((UINT32)0x64)
#define SFLASH_CFG3_REG         ((UINT32)0x68)
#define SFLASH_STATUS0_REG      ((UINT32)0x70)
#define SFLASH_STATUS1_REG      ((UINT32)0x74)
#define SFLASH_STATUS2_REG      ((UINT32)0x78)
#define SFLASH_STATUS3_REG      ((UINT32)0x7C)

#define REG_CFG1_BRI		((UINT32)0x710)
#define REG_CFG2_BRI		((UINT32)0x714)
#define REG_FDMA_CTL		((UINT32)0x718)
#define REG_FDMA_FADR		((UINT32)0x71C)
#define REG_FDMA_DADR		((UINT32)0x720)
#define REG_FDMA_END_DADR	((UINT32)0x724)
//-----------------------------------------------------------------------------
#define SFLASH_WRBUF_SIZE       128


//-----------------------------------------------------------------------------
#define MAX_FLASHCOUNT  1
#define SFLASHHWNAME_LEN    48

typedef struct
{
    UINT8   u1MenuID;
    UINT8   u1DevID1;
    UINT8   u1DevID2;
    UINT8   u1PPType;
    UINT32  u4ChipSize;
    UINT32  u4SecSize;
    UINT32  u4CEraseTimeoutMSec;

    UINT8   u1WRENCmd;
    UINT8   u1WRDICmd;
    UINT8   u1RDSRCmd;
    UINT8   u1WRSRCmd;
    UINT8   u1READCmd;
    UINT8   u1FASTREADCmd;
    UINT8   u1READIDCmd;
    UINT8   u1SECERASECmd;
    UINT8   u1CHIPERASECmd;
    UINT8   u1PAGEPROGRAMCmd;
    UINT8   u1AAIPROGRAMCmd;
    UINT8   u1DualREADCmd;
    UINT8   u1Protection;
    CHAR    pcFlashStr[SFLASHHWNAME_LEN];
} SFLASHHW_VENDOR_T;

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------
// clock
#define SYS_CLK_CFG_7			((UINT32)0xB0)


// ISR related Macro
#define IRQEN_REG                0xF0008034
#define SFLASH_IRQEN_SHIFT       (22)

#define SFLASH_DMARDE            (0x1<<7)
#define SFLASH_AAIWRE            (0x1<<6)
#define SFLASH_WSRE              (0x1<<5)
#define SFLASH_WRE               (0x1<<4)
#define SFLASH_PRGE              (0x1<<2)
#define SFLASH_RSRE              (0x1<<1)
#define SFLASH_RDE               (0x1<<0)

#define SFLASH_EN_INT            SFLASH_DMARDE

// HOST attribute
#define SFLASH_USE_DMA           (0x1<<0)
#define SFLASH_USE_ISR           (0x1<<1)
#define SFLASH_USE_DUAL_READ     (0x1<<2)

#define LoWord(d)     ((UINT16)(d & 0x0000ffffL))
#define HiWord(d)     ((UINT16)((d >> 16) & 0x0000ffffL))
#define LoByte(w)     ((UINT8)(w & 0x00ff))
#define HiByte(w)     ((UINT8)((w >> 8) & 0x00ff))
#define MidWord(d)    ((UINT16)((d >>8) & 0x00ffff))

#define BYTE0(arg)    (*(UINT8 *)&arg)
#define BYTE1(arg)    (*((UINT8 *)&arg + 1))
#define BYTE2(arg)    (*((UINT8 *)&arg + 2))
#define BYTE3(arg)    (*((UINT8 *)&arg + 3))


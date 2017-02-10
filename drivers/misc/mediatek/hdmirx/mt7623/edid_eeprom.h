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
*[File]             eeprom.h
*[Version]          v0.1
*[Revision Date]    2009-06-16
*[Author]           Kenny Hsieh
*[Description]
*    source file for eeprom
*
*
******************************************************************************/

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "edid_data.h"
#include "typedef.h"
#include "hdmi_rx_hw.h"

#define DDC_CLK_DEFAULT 100 //71KHZ //200//110 khz  if EEPROM VCC=5V, Max =400Khz, But Vcc=3.3 Max VCC=100Khz


#define EDID_EEPROM1 0
#define EDID_EEPROM2 1

// EDID device ID
#define EDIDID     0x50  // 0xA0


#define vWriteHdmirxIoPad(dAddr, dVal)  IO_WRITE32(HDMIRX_PAD_BASE, dAddr, dVal)//*(volatile UINT32 *)(IO_BASE_ADDRESS + HDMI_GRL_REG_OFFSET + bAddr) = bVal
#define dReadHdmirxIoPad(bAddr)         IO_READ32(HDMIRX_PAD_BASE, bAddr)//(UINT8)((*(volatile UINT32 *)(IO_BASE_ADDRESS + HDMI_GRL_REG_OFFSET + bAddr))&0xff)
#define vWriteHdmirxIoPadMsk(dAddr, dVal, dMsk) vWriteHdmirxIoPad((dAddr), (dReadHdmirxIoPad(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))


#define SET_Rx_DDCD_OUT (vWriteHdmirxIoPadMsk(IO_HDMIRX_PAD_MODE, PAD_MODE_SCL_RX, PAD_MODE_SCL_RX_MASK))
#define SET_Rx_DDCD_IN  (vWriteHdmirxIoPadMsk(IO_HDMIRX_PAD_MODE, 0, PAD_MODE_SCL_RX_MASK))

#define SET_Rx_DDCC_OUT (vWriteHdmirxIoPadMsk(IO_HDMIRX_SDA_PAD_MODE, PAD_MODE_SDA_RX, PAD_MODE_SDA_RX_MASK))
#define SET_Rx_DDCC_IN  (vWriteHdmirxIoPadMsk(IO_HDMIRX_SDA_PAD_MODE, 0, PAD_MODE_SDA_RX_MASK))



#define SET_Rx_DDCD  (vWriteHdmirxIoPadMsk(IO_HDMIRX_PAD_OUTPUT_VALUE, 0, IO_PAD_SDA_RX_EN))//set input
#define SET_Rx_DDCC  (vWriteHdmirxIoPadMsk(IO_HDMIRX_PAD_OUTPUT_VALUE, 0, IO_PAD_SCL_RX_EN))//set input 


// *********************************************************************
// Exported API 
// *********************************************************************
extern BOOL fgEeprom1DataRead(BYTE bDevice, BYTE bData_Addr, BYTE bDataCount,
                   BYTE *prBuffer);	   
extern BOOL fgEeprom2DataRead(BYTE bDevice, BYTE bData_Addr, BYTE bDataCount,
                   BYTE *prBuffer);	   
                                      
extern BOOL fgEeprom1DataWrite(BYTE bDevice, BYTE bData_Addr,
                    BYTE bDataCount, BYTE *prData);

extern BOOL fgEeprom2DataWrite(BYTE bDevice, BYTE bData_Addr,
                    BYTE bDataCount, BYTE *prData);                    

#define fgEeprom1ByteWrite(bDevice, bData_Addr, bData) \
          fgEeprom1DataWrite(bDevice, bData_Addr, 1, &bData)

#define fgEeprom2ByteWrite(bDevice, bData_Addr, bData) \
          fgEeprom2DataWrite(bDevice, bData_Addr, 1, &bData)
          
#define fgEeprom1ByteRead(bDevice, bData_Addr, pbData) \
          fgEeprom1DataRead(bDevice, bData_Addr, 1, pbData)
          
#define fgEeprom2ByteRead(bDevice, bData_Addr, pbData) \
          fgEeprom2DataRead(bDevice, bData_Addr, 1, pbData)          


extern void vSWResetEEPROM(BYTE bEEPROEMNo);

void vInitEepromI2cLine(void);
void vSetEepromClock(BYTE bClck);
void vEnableEDIDDLMODE(BOOL fgEnable);
void vWriteEDIDCHKSUM(UINT8 u1DevNum,UINT8 u1Val);
void vWriteEDIDPA(UINT8 u1DevNum,UINT16 u2Val, UINT8 u1Offset);
void vSetEDIDDLADD(UINT8 u1ADD);
void vWriteEDIDRam(UINT32 u4Val);
UINT32 u4ReadEDIDRam(void);
void vHDMIRxDDCChgToMaster(BOOL fgMaster);


// *********************************************************************
// Define I2C Read/Write Flag
// *********************************************************************
#define   FG_SEQREAD    1
#define   FG_RANDREAD   0
#endif//_DDC_I2C_H_

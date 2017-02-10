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
*[File]             edid_eeprom.c
*[Version]          v0.1
*[Revision Date]    2009-06-16
*[Author]           Kenny Hsieh
*[Description]
*    source file for eeprom control
*
*
******************************************************************************/

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

#include "hdmi_rx_ctrl.h"
#include "rx_io.h"
#include "edid_data.h"
#include "hdmi_rx_hal.h"

/****************************************************************************
** Local definitions
****************************************************************************/
#include "edid_eeprom.h"


#define DDC_RETRY_COUNT 1
#define   ACK_DELAY  200     // time out for acknowledge check
#define   BUS_DELAY  255     // time out bus arbitration
//#define   CLK_DELAY  1 //for about 78khz DDC
#define   CLK_DELAY  3 //for about 78khz DDC

extern UINT8 u1HDMIIN1EDID[256];
extern UINT8 u1HDMIIN2EDID[256];

UINT32 _u4EepromI2C_Clk = 10;//3;//for about 54khz DDC
UINT8 _u1EepromLine = EDID_EEPROM1;
/****************************************************************************
** Function prototypes
****************************************************************************/

/****************************************************************************
** Local variable
****************************************************************************/

/****************************************************************************
** Local functions
****************************************************************************/

/****************************************************************************
** EXP API functions
****************************************************************************/

/************************************************************************
     Function : void vInitDDCLine(void)
  Description : Init DDC line GPIO
    Parameter : None
    Return    : None
************************************************************************/
void vInitEepromI2cLine(void)
{
	vSetEepromClock(DDC_CLK_DEFAULT);
}

/************************************************************************
     Function : void vHDMIRxDDCChgToMaster()
  Description : Set DDC Clock
    Parameter : None
    Return    : None
************************************************************************/
void vHDMIRxDDCChgToMaster(BOOL fgMaster)
{

}


/************************************************************************
     Function : void vSetEepromClock(BYTE bClck)
  Description : Set DDC Clock
    Parameter : None
    Return    : None
************************************************************************/
void vSetEepromClock(BYTE bClck)
{
	UINT32 u4Temp;

	u4Temp = (UINT32)((1000/(UINT32)bClck)/5);

	if(u4Temp < 1)
		_u4EepromI2C_Clk = 1;
	else
		_u4EepromI2C_Clk = u4Temp;
	
	printk(" vSetEepromClock =%d\n", _u4EepromI2C_Clk);	
}	



/************************************************************************
     Function : void vSetEepromClock(BYTE bClck)
  Description : Set DDC Clock
    Parameter : None
    Return    : None
************************************************************************/
void vSetEepromLine(BYTE bLineType)
{
	_u1EepromLine = bLineType;	
}	


/************************************************************************
     Function : void vEepormDelay2us(BYTE bValue)
  Description : Delay Routine (bValue = 1 --> 2us, 2->4us)
    Parameter : bDelayValue
    Return    : None
************************************************************************/
void vEepormDelay2us(UINT32 ui4Value)// I2C speed control
{                          
	msleep(ui4Value);
}



/************************************************************************
     Function : void vSetEepromClock(BYTE bClck)
  Description : Set I2C Clock
    Parameter : None
    Return    : None
************************************************************************/
void SET_I2C_DATA(void)
{
	if(_u1EepromLine == EDID_EEPROM1)	
	{
		SET_Rx_DDCD;
	}
	else
	{
		//SET_Rx2_DDCD;
	}		
	
}	

/************************************************************************
     Function : void SET_I2C_CLOCK(void)
  Description : Set I2C Clock
    Parameter : None
    Return    : None
************************************************************************/
void SET_I2C_CLOCK(void)
{
	if(_u1EepromLine == EDID_EEPROM1)	
	{
		SET_Rx_DDCC;
	}
	else
	{
		//SET_Rx2_DDCC;
	}		
}	

/************************************************************************
     Function : void CLR_I2C_DATA(void)
  Description : Low I2C Data
    Parameter : None
    Return    : None
************************************************************************/
void CLR_I2C_DATA(void)
{
	if(_u1EepromLine == EDID_EEPROM1)	
	{
		//CLR_Rx_DDCD;
	}
	else
	{
		//CLR_Rx2_DDCD;
	}		
}	

/************************************************************************
     Function : void CLR_I2C_CLOCK(void)
  Description : Low I2C Clock
    Parameter : None
    Return    : None
************************************************************************/
void CLR_I2C_CLOCK(void)
{
	if(_u1EepromLine == EDID_EEPROM1)	
	{
		//CLR_Rx_DDCC;
	}
	else
	{
		//CLR_Rx2_DDCC;
	}		

}	

/************************************************************************
     Function : void vSetEepromClock(BYTE bClck)
  Description : Read I2C Data
    Parameter : None
    Return    : None
************************************************************************/
BYTE R_I2C_DATA(void)
{
	if(_u1EepromLine == EDID_EEPROM1)	
	{
		//if(Rx_DDCD)
			//return 1;
		//else
			//return 0;
	}
}	

/************************************************************************
     Function : void R_I2C_CLOCK(void)
  Description : Read I2C Clock
    Parameter : None
    Return    : None
************************************************************************/
BYTE  R_I2C_CLOCK(void)
{
	if(_u1EepromLine == EDID_EEPROM1)	
	{
		//if(Rx_DDCC)
			//return 1;
		//else
			//return 0;
	}
}




/************************************************************************
     Function : BOOL fgEepromSend(BYTE bValue)
  Description : Send Routine
                timing : SCL ___|^|___|^|__~__|^|___|^|__
                         SDA __/D7 \_/D6 \_~_/D0 \_/ACK\_
    Parameter : bValue(8-bit output data)
    Return    : TRUE  : successful with ACK from slave
                FALSE  : bus (DDCC = 0) or ACK failure
************************************************************************/
BOOL fgEepromSend(BYTE bValue) 
{
	BYTE bBitMask = 0x80;
	UINT32 dwTime;

// step 1 : 8-bit data transmission
	while(bBitMask)
	{
		if(bBitMask & bValue)
		{
			SET_I2C_DATA();
		}
		else
		{
			CLR_I2C_DATA();
		}
		vEepormDelay2us(_u4EepromI2C_Clk);
		SET_I2C_CLOCK();                    // data clock in, input
		vEepormDelay2us(_u4EepromI2C_Clk);
		CLR_I2C_CLOCK();                    // ready for next clock in, output
		vEepormDelay2us(_u4EepromI2C_Clk);
		bBitMask = bBitMask >> 1;   // MSB first & timing delay
	}

  // step 2 : slave acknowledge check
	SET_I2C_DATA();                      // release SDA for ACK polling, input
	vEepormDelay2us(_u4EepromI2C_Clk);
	SET_I2C_CLOCK();                      // start ACK polling, input
	dwTime = ACK_DELAY;         // time out protection
	vEepormDelay2us(_u4EepromI2C_Clk);

	while(R_I2C_DATA() && --dwTime)
	{
		;  // wait for ACK, SDA=0 or bitMask=0->jump to this loop
	}

	CLR_I2C_CLOCK();                     // end ACK polling, out
	vEepormDelay2us(_u4EepromI2C_Clk);

	if(dwTime)
	{
		return(TRUE);              // return TRUE if ACK detected
	}
	else
	{  	
		return(FALSE);             // return FALSE if time out
	}
}

/************************************************************************
     Function : void vEepromRead(BYTE *prValue, BOOL fgSeq_Read)
  Description : Read Routine
                timing : SCL ___|^|___|^|__~__|^|___|^|__
                         SDA __/D7 \_/D6 \_~_/D0 \_/ACK\_
    Parameter : *prValue(8-bit input pointer value)
    Return    : NONE
************************************************************************/
void vEepromRead(BYTE *prValue, BOOL fgSeq_Read) 
{
	BYTE bBitMask = 0x80;

	*prValue = 0;                 // reset data buffer
	SET_I2C_DATA();                      // make sure SDA released

	vEepormDelay2us(_u4EepromI2C_Clk);

	// step 1 : 8-bit data reception
	while(bBitMask)
	{
		SET_I2C_CLOCK();                    // data clock out
		vEepormDelay2us(_u4EepromI2C_Clk);
		if(R_I2C_DATA())
		{
			*prValue = *prValue | bBitMask;   // Get all data
		}                                   // non-zero bits to buffer
		CLR_I2C_CLOCK();                            // ready for next clock out
		vEepormDelay2us(_u4EepromI2C_Clk);
		bBitMask = bBitMask >> 1;           // shift bit mask & clock delay
	}

	// step 2 : acknowledgement to slave
	if(fgSeq_Read)
	{
		CLR_I2C_DATA();                            // ACK here for Sequential Read
	}
	else
	{
		SET_I2C_DATA();                            // NACK here (for single byte read)
	}

	vEepormDelay2us(_u4EepromI2C_Clk);
	SET_I2C_CLOCK();                    // NACK clock out
	vEepormDelay2us(_u4EepromI2C_Clk);
	CLR_I2C_CLOCK();                    // ready for next clock out
	vEepormDelay2us(_u4EepromI2C_Clk);
	SET_I2C_DATA();                    // release SDA
	vEepormDelay2us(_u4EepromI2C_Clk);

}

/************************************************************************
     Function : BOOL fgEepromStart(BYTE bValue, BOOL fgRead)
  Description : Start Routine
                timing : SCL ^^^^|___|^|___|^|__~__|^|___|^|___|^|__~
                         SDA ^^|____/A6 \_/A5 \_~_/A0 \_/R/W\_/ACK\_~
                              (S)
                               |<--- start condition
    Parameter : bDevice(7-bit slave address) + fgRead(R/W bit)
    Return    : TRUE  : successful with ACK from slave
                FALSE  : bus (DDCC = 0) or ACK failure
************************************************************************/
BOOL fgEepromStart(BYTE bDevice) 
{
	UINT32 dwBusDelayTemp = BUS_DELAY;


	SET_I2C_DATA();            //input,  make sure SDA released
	vEepormDelay2us(_u4EepromI2C_Clk);
	SET_I2C_CLOCK();            // make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	while((!R_I2C_CLOCK()) && (--dwBusDelayTemp))
	{
		;  // simple bus abritration
	}

	if(!dwBusDelayTemp)
	{  	
		return(FALSE);           // time out protection & timing delay
	}

	CLR_I2C_DATA();          // start condition here, output
	vEepormDelay2us(_u4EepromI2C_Clk);
	CLR_I2C_CLOCK();          // ready for clocking, output
	vEepormDelay2us(_u4EepromI2C_Clk);

	return(fgEepromSend(bDevice));// slave address & R/W transmission
}

/************************************************************************
     Function : void vEepromStop(void)
  Description : Stop Routine
                timing : SCL ___|^^^^^
                         SDA xx___|^^^
                                 (P)
                                  |<--- stop condition
    Parameter : NONE
    Return    : NONE
************************************************************************/
void vEepromStop(void) 
{
	CLR_I2C_DATA();          // ready for stop condition
	vEepormDelay2us(_u4EepromI2C_Clk);
	SET_I2C_CLOCK();          // ready for stop condition

	vEepormDelay2us(_u4EepromI2C_Clk);
	SET_I2C_DATA();          // stop condition here
	vEepormDelay2us(_u4EepromI2C_Clk);
}




void vSWResetEEPROM(BYTE bEEPROMNo)
{

	vSetEepromLine(bEEPROMNo);	
	SET_I2C_DATA(); 		   //input,  make sure SDA released
	vEepormDelay2us(_u4EepromI2C_Clk);
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);
	// start condition
	CLR_I2C_DATA(); 

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);

	SET_I2C_DATA(); 		

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 1
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 2
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 3
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 4
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 5
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 6
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 7
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 8
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);
	// 9
	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);

	SET_I2C_CLOCK();			// make sure SCL released

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_DATA();

	vEepormDelay2us(_u4EepromI2C_Clk);

	CLR_I2C_CLOCK(); 

	vEepormDelay2us(_u4EepromI2C_Clk);

	SET_I2C_CLOCK();			

	vEepormDelay2us(_u4EepromI2C_Clk);

	SET_I2C_DATA();

}



/************************************************************************
     Function : BOOL fgDDCDataWrite(BYTE bDevice, BYTE bData_Addr,
                                    BYTE bDataCount, BYTE *prData)
  Description : ByteWrite Routine
    Parameter : bDevice -> Device Address
                bData_Addr -> Data Address
                bDataCount -> Data Content Cont
                *prData -> Data Content Pointer
    Return    : TRUE  : successful with ACK from slave
                FALSE  : bus (DDCC = 0) or ACK failure
************************************************************************/
static BOOL fgRxI2cDataWrite(BYTE bDevice, BYTE bData_Addr,
                    BYTE bDataCount, BYTE *prData)
{
	BYTE bRetry, bOri_Device, bError=0, bOri_count;

	if(bDevice >= 128)
	{
		vEepromStop();
		return(FALSE);             // Device Address exceeds the range
	}

	bOri_Device=bDevice;
	bOri_count=bDataCount;
	for(bRetry=0;bRetry<DDC_RETRY_COUNT;bRetry++)
	{
		bDevice=bOri_Device;	

		bDevice = bDevice << 1;      // Shift the 7-bit address to 7+1 format

		if(!fgEepromStart(bDevice))     // Write Command
		{
			vEepromStop();
			continue; // Device Address exceeds the range
		}

		if(!fgEepromSend(bData_Addr))   // Word Address
		{
			vEepromStop();
			continue;// Device Address exceeds the range
		}

		bDataCount=bOri_count;
		bError=0;
		while(bDataCount)
		{
			if(!fgEepromSend(*(prData++))) // Data Content Write
			{
				vEepromStop();
				bError=1;
				break;// Device Address exceeds the range
			}
			bDataCount--;
		}

		if(bError)
			continue;
		else
		{
			vEepromStop();
			return(TRUE);
		}
	}//for(bRetry=0;bRetry<DDC_RETRY_COUNT;bRetry++)

	vEepromStop();
	return(FALSE);

}

/************************************************************************
     Function : BOOL fgRxI2CDataRead(BYTE bDevice, BYTE bData_Addr,
                                    BYTE bDataCount, BYTE *prData)
  Description : DataRead Routine
    Parameter : bDevice -> Device Address
                bData_Addr -> Data Address
                bDataCount -> Data Content Cont
                *prData -> Data Content Pointer
    Return    : TRUE  : successful with ACK from slave
                FALSE  : bus (SCL = 0) or ACK failure
************************************************************************/
static BOOL fgRxI2CDataRead(BYTE bDevice, BYTE bData_Addr, BYTE bDataCount,
                   BYTE *prData)	                    
{
	BYTE bDeviceOri;
	//WORD wPos;
	BYTE bRetry;
	//BYTE bEDIDReadIndex;
	BYTE bTemp;  

	bDeviceOri = bDevice;


	// Step 1 : Dummy Write
	if(bDevice >= 128)
	{
		vEepromStop();
		return(FALSE);
	}

	for(bRetry=0;bRetry<DDC_RETRY_COUNT;bRetry++)
	{
		bDevice=bDeviceOri ;//kenny
		if (bDevice > EDIDID) //Max'0619'04, 4-block EEDID reading
		{
			if(!fgEepromStart(0x60))  // Write Command
			{     
				//sFlagI2C.fgBusBusy = 0;
				vEepromStop();
				continue;// Start fail
			}
			if(!fgEepromSend(bDevice-EDIDID))// Word Address
			{      		
				//sFlagI2C.fgBusBusy = 0;
				vEepromStop();
				continue;// Data Address Fail
			}
			bDevice = EDIDID;
		}

		bDevice = bDevice << 1;   // Shift the 7-bit address to 7+1 format

		if(!fgEepromStart(bDevice))  // Write Command
		{    	
			// sFlagI2C.fgBusBusy = 0;
			vEepromStop();
			continue;
		}
		if(!fgEepromSend(bData_Addr))// Word Address
		{  	 	
			//sFlagI2C.fgBusBusy = 0;
			vEepromStop();
			continue;
		}

		// Step 2 : Real Read
		bDevice = bDevice + 1;    // Shift the 7-bit address to 7+1 format
		if(!fgEepromStart(bDevice))  // Read Command
		{  	
			//sFlagI2C.fgBusBusy = 0;
			vEepromStop();
			continue;
		}

		while (bDataCount)
		{
			if (bDataCount == 1)
			{
				vEepromRead(&bTemp, FG_RANDREAD);  // Data Content Read
			}
			else
			{
				vEepromRead(&bTemp, FG_SEQREAD);  // Data Content Read
			}

			*prData= bTemp;
			prData++;
			bDataCount--;


		}
		// Step 3 : Stop
		vEepromStop();
		//sFlagI2C.fgBusBusy = 0;  
		return (TRUE);
	}//for

	//retry fail
	// sFlagI2C.fgBusBusy = 0;
	vEepromStop();

	return (FALSE);

}



BOOL fgEeprom1DataRead(BYTE bDevice, BYTE bData_Addr, BYTE bDataCount,
                   BYTE *prData)	
{    
	BOOL fgResult;
	               
	vSetEepromLine(EDID_EEPROM1);	
	fgResult = fgRxI2CDataRead(bDevice, bData_Addr, bDataCount, prData);	
	return fgResult;      	
  
}	


BOOL fgEeprom2DataRead(BYTE bDevice, BYTE bData_Addr, BYTE bDataCount,
                   BYTE *prData)	
{    
	BOOL fgResult;	               
	vSetEepromLine(EDID_EEPROM2);	
	fgResult = fgRxI2CDataRead(bDevice, bData_Addr, bDataCount, prData);	      	
	return fgResult;  
}	


BOOL fgEeprom1DataWrite(BYTE bDevice, BYTE bData_Addr,
                    BYTE bDataCount, BYTE *prData)
{
	BOOL fgResult;
	vSetEepromLine(EDID_EEPROM1);	
	fgResult = fgRxI2cDataWrite(EDIDID, bData_Addr, bDataCount, prData);
	return fgResult;  	
}	                    
                    
                    
BOOL fgEeprom2DataWrite(BYTE bDevice, BYTE bData_Addr,
                    BYTE bDataCount, BYTE *prData)
{
	BOOL fgResult;
	vSetEepromLine(EDID_EEPROM2);	
	fgResult = fgRxI2cDataWrite(EDIDID, bData_Addr, bDataCount, prData);
	return fgResult;  	
}

BOOL fgEEPROM_Enable_Test(BYTE bEDIDNo,BYTE bPageID,BYTE bData_Addr,BYTE *prData)
{
	return FALSE;
}


void vEnableEDIDDLMODE(BOOL fgEnable)
{
	if(fgEnable)
		DDCCI_WRITE32_MASK(REG_DDC_MISC_CNTL0, (1<<26)|(1<<23),(1<<26)|(1<<23));//enable download mode
	else
		DDCCI_WRITE32_MASK(REG_DDC_MISC_CNTL0, (1<<26),(1<<26)|(1<<23));	//disable download mode
}

void vSetEDIDDLADD(UINT8 u1ADD)
{
	DDCCI_WRITE32_MASK(REG_DDC_MISC_CNTL0,(u1ADD&0x7F)<<16,0x7F<<16);
}

void vWriteEDIDCHKSUM(UINT8 u1DevNum,UINT8 u1Val)
{
	if(u1DevNum == 0) // port 0 's checksum
		DDCCI_WRITE32_MASK(REG_DDC_HDMI0_CNTL1, u1Val <<16 ,(0xff<<16));
	else if(u1DevNum == 1)// port 1 's checksum
		DDCCI_WRITE32_MASK(REG_DDC_HDMI1_CNTL1, u1Val <<16 ,(0xff<<16));
	else
		printk("[HDMI Rx] EDIDCHKSUM : wrong dev number \n");
}

void vWriteEDIDPA(UINT8 u1DevNum,UINT16 u2Val, UINT8 u1Offset)
{
	if(u1DevNum == 0) // port 0 's checksum
	{
		DDCCI_WRITE32_MASK(REG_DDC_HDMI0_CNTL2, ((u2Val>>8)&0xFF) <<24 ,(0xff<<24));
		DDCCI_WRITE32_MASK(REG_DDC_HDMI0_CNTL2, u1Offset <<16 ,(0xff<<16));
		DDCCI_WRITE32_MASK(REG_DDC_HDMI0_CNTL2, (u2Val&0xFF) <<8 ,(0xff<<8));
		DDCCI_WRITE32_MASK(REG_DDC_HDMI0_CNTL2, (u1Offset+1) <<0 ,(0xff<<0));
	}
	else if(u1DevNum == 1)// port 1 's checksum
	{
		DDCCI_WRITE32_MASK(REG_DDC_HDMI1_CNTL2, ((u2Val>>8)&0xFF) <<24 ,(0xff<<24));
		DDCCI_WRITE32_MASK(REG_DDC_HDMI1_CNTL2, u1Offset <<16 ,(0xff<<16));
		DDCCI_WRITE32_MASK(REG_DDC_HDMI1_CNTL2, (u2Val&0xFF) <<8 ,(0xff<<8));
		DDCCI_WRITE32_MASK(REG_DDC_HDMI1_CNTL2, (u1Offset+1) <<0 ,(0xff<<0));
	}
	else
		printk("[HDMI Rx] EDIDCHKSUM : wrong dev number \n");
}

void vWriteEDIDRam(UINT32 u4Val)
{
	DDCCI_WRITE32(REG_DDC_EDID_DOWNLOAD_PORT, u4Val);//address 255 is invalid
}
UINT32 u4ReadEDIDRam(void)
{
	UINT32 u4Ret = 0;
	DDCCI_READ32(REG_DDC_EDID_DOWNLOAD_PORT);
	u4Ret = DDCCI_READ32(0x620);
	return u4Ret;
}


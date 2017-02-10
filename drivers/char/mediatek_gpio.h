/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************
 */
#ifndef __MEDIATEK_GPIO_H__
#define __MEDIATEK_GPIO_H__

#if defined (CONFIG_MEDIATEK_MT7623_GPIO_SETTING)
#include "cust_gpio_boot_mt7623.h"
#endif

#if defined (CONFIG_MEDIATEK_MT7623N_GPIO_SETTING)
#include "cust_gpio_boot_mt7623n.h"
#endif

#define GPIO_BASE_REG                       0xF0005000
#define EINT_BASE_REG                       0xF000B000
#define GPIO_DIR_1                   (GPIO_BASE_REG)
#define GPIO_015_DIR               (GPIO_BASE_REG)
#define GPIO_1631_DIR              (GPIO_BASE_REG + 0x10)
#define GPIO_3247_DIR              (GPIO_BASE_REG + 0x20)
#define GPIO_4863_DIR              (GPIO_BASE_REG + 0x30)
#define GPIO_6479_DIR              (GPIO_BASE_REG + 0x40)
#define GPIO_8095_DIR              (GPIO_BASE_REG + 0x50)
#define GPIO_96111_DIR             (GPIO_BASE_REG + 0x60)
#define GPIO_112127_DIR            (GPIO_BASE_REG + 0x70)
#define GPIO_128143_DIR            (GPIO_BASE_REG + 0x80)
#define GPIO_144159_DIR            (GPIO_BASE_REG + 0x90)
#define GPIO_160175_DIR            (GPIO_BASE_REG + 0xa0)

#define MSDC1_CTRL6            (GPIO_BASE_REG + 0xb0)

#define GPIO_DIR_2                  (GPIO_BASE_REG + 0xc0)
#define GPIO_176191_DIR            (GPIO_BASE_REG + 0xc0)
#define GPIO_192207_DIR            (GPIO_BASE_REG + 0xd0)
#define GPIO_208223_DIR            (GPIO_BASE_REG + 0xe0)
#define GPIO_224239_DIR            (GPIO_BASE_REG + 0xf0)
#define GPIO_240255_DIR            (GPIO_BASE_REG + 0x100)
#define GPIO_256271_DIR            (GPIO_BASE_REG + 0x110)
#define GPIO_272279_DIR            (GPIO_BASE_REG + 0x120)

#define GPIO_PENA                   (GPIO_BASE_REG + 0x150)
#define GPIO_015_PENA               (GPIO_BASE_REG + 0x150)
#define GPIO_1631_PENA              (GPIO_BASE_REG + 0x160)
#define GPIO_3247_PENA              (GPIO_BASE_REG + 0x170)
#define GPIO_4863_PENA              (GPIO_BASE_REG + 0x180)
#define GPIO_6479_PENA              (GPIO_BASE_REG + 0x190)
#define GPIO_8095_PENA              (GPIO_BASE_REG + 0x1a0)
#define GPIO_96111_PENA             (GPIO_BASE_REG + 0x1b0)
#define GPIO_112127_PENA            (GPIO_BASE_REG + 0x1c0)
#define GPIO_128143_PENA            (GPIO_BASE_REG + 0x1d0)
#define GPIO_144159_PENA            (GPIO_BASE_REG + 0x1e0)
#define GPIO_160175_PENA            (GPIO_BASE_REG + 0x1f0)
#define GPIO_176191_PENA            (GPIO_BASE_REG + 0x200)
#define GPIO_192207_PENA            (GPIO_BASE_REG + 0x210)
#define GPIO_208223_PENA            (GPIO_BASE_REG + 0x220)
#define GPIO_224239_PENA            (GPIO_BASE_REG + 0x230)
#define GPIO_240255_PENA            (GPIO_BASE_REG + 0x240)
#define GPIO_256271_PENA            (GPIO_BASE_REG + 0x250)
#define GPIO_272279_PENA            (GPIO_BASE_REG + 0x260)


#define GPIO_PSEL                   (GPIO_BASE_REG + 0x280)
#define GPIO_015_PSEL               (GPIO_BASE_REG + 0x280)
#define GPIO_1631_PSEL              (GPIO_BASE_REG + 0x290)
#define GPIO_3247_PSEL              (GPIO_BASE_REG + 0x2a0)
#define GPIO_4863_PSEL              (GPIO_BASE_REG + 0x2b0)
#define GPIO_6479_PSEL              (GPIO_BASE_REG + 0x2c0)
#define GPIO_8095_PSEL              (GPIO_BASE_REG + 0x2d0)
#define GPIO_96111_PSEL             (GPIO_BASE_REG + 0x2e0)
#define GPIO_112127_PSEL            (GPIO_BASE_REG + 0x2f0)
#define GPIO_128143_PSEL            (GPIO_BASE_REG + 0x300)
#define GPIO_144159_PSEL            (GPIO_BASE_REG + 0x310)
#define GPIO_160175_PSEL            (GPIO_BASE_REG + 0x320)
#define GPIO_176191_PSEL            (GPIO_BASE_REG + 0x330)
#define GPIO_192207_PSEL            (GPIO_BASE_REG + 0x340)
#define GPIO_208223_PSEL            (GPIO_BASE_REG + 0x350)
#define GPIO_224239_PSEL            (GPIO_BASE_REG + 0x360)
#define GPIO_240255_PSEL            (GPIO_BASE_REG + 0x370)
#define GPIO_256271_PSEL            (GPIO_BASE_REG + 0x380)
#define GPIO_272279_PSEL            (GPIO_BASE_REG + 0x390)



#define GPIO_DOUT               (GPIO_BASE_REG + 0x500)
#define GPIO_015_DOUT               (GPIO_BASE_REG + 0x500)
#define GPIO_1631_DOUT              (GPIO_BASE_REG + 0x510)
#define GPIO_3247_DOUT              (GPIO_BASE_REG + 0x520)
#define GPIO_4863_DOUT              (GPIO_BASE_REG + 0x530)
#define GPIO_6479_DOUT              (GPIO_BASE_REG + 0x540)
#define GPIO_8095_DOUT              (GPIO_BASE_REG + 0x550)
#define GPIO_96111_DOUT             (GPIO_BASE_REG + 0x560)
#define GPIO_112127_DOUT            (GPIO_BASE_REG + 0x570)
#define GPIO_128143_DOUT            (GPIO_BASE_REG + 0x580)
#define GPIO_144159_DOUT            (GPIO_BASE_REG + 0x590)
#define GPIO_160175_DOUT            (GPIO_BASE_REG + 0x5a0)
#define GPIO_176191_DOUT            (GPIO_BASE_REG + 0x5b0)
#define GPIO_192207_DOUT            (GPIO_BASE_REG + 0x5c0)
#define GPIO_208223_DOUT            (GPIO_BASE_REG + 0x5d0)
#define GPIO_224239_DOUT            (GPIO_BASE_REG + 0x5e0)
#define GPIO_240255_DOUT            (GPIO_BASE_REG + 0x5f0)
#define GPIO_256271_DOUT            (GPIO_BASE_REG + 0x600)
#define GPIO_272279_DOUT            (GPIO_BASE_REG + 0x610)


#define GPIO_015_DIN               (GPIO_BASE_REG + 0x630)
#define GPIO_1631_DIN              (GPIO_BASE_REG + 0x640)
#define GPIO_3247_DIN              (GPIO_BASE_REG + 0x650)
#define GPIO_4863_DIN              (GPIO_BASE_REG + 0x660)
#define GPIO_6479_DIN              (GPIO_BASE_REG + 0x670)
#define GPIO_8095_DIN              (GPIO_BASE_REG + 0x680)
#define GPIO_96111_DIN             (GPIO_BASE_REG + 0x690)
#define GPIO_112127_DIN            (GPIO_BASE_REG + 0x6a0)
#define GPIO_128143_DIN            (GPIO_BASE_REG + 0x6b0)
#define GPIO_144159_DIN            (GPIO_BASE_REG + 0x6c0)
#define GPIO_160175_DIN            (GPIO_BASE_REG + 0x6d0)
#define GPIO_176191_DIN            (GPIO_BASE_REG + 0x6e0)
#define GPIO_192207_DIN            (GPIO_BASE_REG + 0x6f0)
#define GPIO_208223_DIN            (GPIO_BASE_REG + 0x700)
#define GPIO_224239_DIN            (GPIO_BASE_REG + 0x710)
#define GPIO_240255_DIN            (GPIO_BASE_REG + 0x720)
#define GPIO_256271_DIN            (GPIO_BASE_REG + 0x730)
#define GPIO_272279_DIN            (GPIO_BASE_REG + 0x740)





#define GPIO_MODE_REG                (GPIO_BASE_REG + 0x760)


#define GPIO_BANK                (GPIO_BASE_REG + 0xB10)
#define GPIO_IES_EN0             (GPIO_BASE_REG + 0xB20)
#define GPIO_IES_EN1             (GPIO_BASE_REG + 0xB30)
#define GPIO_IES_EN2             (GPIO_BASE_REG + 0xB40)

#define GPIO_SMT_EN0             (GPIO_BASE_REG + 0xB50)
#define GPIO_SMT_EN1             (GPIO_BASE_REG + 0xB60)
#define GPIO_SMT_EN2             (GPIO_BASE_REG + 0xB70)

#define GPIO_TDSEL0             (GPIO_BASE_REG + 0xB80)
#define GPIO_TDSEL1             (GPIO_BASE_REG + 0xB90)
#define GPIO_TDSEL2             (GPIO_BASE_REG + 0xBA0)
#define GPIO_TDSEL3             (GPIO_BASE_REG + 0xBB0)
#define GPIO_TDSEL4             (GPIO_BASE_REG + 0xBC0)
#define GPIO_TDSEL5             (GPIO_BASE_REG + 0xBD0)


#define GPIO_RDSEL0             (GPIO_BASE_REG + 0xC20)
#define GPIO_RDSEL1             (GPIO_BASE_REG + 0xC30)
#define GPIO_RDSEL2             (GPIO_BASE_REG + 0xC40)
#define GPIO_RDSEL3             (GPIO_BASE_REG + 0xC50)
#define GPIO_RDSEL4             (GPIO_BASE_REG + 0xC60)
#define GPIO_RDSEL5             (GPIO_BASE_REG + 0xC70)


#define EINT_031_STA            (EINT_BASE_REG)
#define EINT_3263_STA           (EINT_BASE_REG + 0x04)
#define EINT_6495_STA           (EINT_BASE_REG + 0x08)
#define EINT_96127_STA          (EINT_BASE_REG + 0x0C)
#define EINT_128159_STA         (EINT_BASE_REG + 0x10)
#define EINT_160169_STA         (EINT_BASE_REG + 0x14)



#define EINT_031_ACK            (EINT_BASE_REG+  0x40)
#define EINT_3263_ACK           (EINT_BASE_REG + 0x44)
#define EINT_6495_ACK           (EINT_BASE_REG + 0x48)
#define EINT_96127_ACK          (EINT_BASE_REG + 0x4C)
#define EINT_128159_ACK         (EINT_BASE_REG + 0x50)
#define EINT_160169_ACK         (EINT_BASE_REG + 0x54)


#define EINT_031_MASK_GET            (EINT_BASE_REG + 0x80)
#define EINT_3263_MASK_GET           (EINT_BASE_REG + 0x84)
#define EINT_6495_MASK_GET           (EINT_BASE_REG + 0x88)
#define EINT_96127_MASK_GET          (EINT_BASE_REG + 0x8C)
#define EINT_128159_MASK_GET         (EINT_BASE_REG + 0x90)
#define EINT_160169_MASK_GET         (EINT_BASE_REG + 0x94)


#define EINT_031_MASK_SET            (EINT_BASE_REG + 0xC0)
#define EINT_3263_MASK_SET           (EINT_BASE_REG + 0xC4)
#define EINT_6495_MASK_SET           (EINT_BASE_REG + 0xC8)
#define EINT_96127_MASK_SET          (EINT_BASE_REG + 0xCC)
#define EINT_128159_MASK_SET         (EINT_BASE_REG + 0xD0)
#define EINT_160169_MASK_SET         (EINT_BASE_REG + 0xD4)

#define EINT_031_MASK_CLR            (EINT_BASE_REG + 0x100)
#define EINT_3263_MASK_CLR           (EINT_BASE_REG + 0x104)
#define EINT_6495_MASK_CLR           (EINT_BASE_REG + 0x108)
#define EINT_96127_MASK_CLR          (EINT_BASE_REG + 0x10C)
#define EINT_128159_MASK_CLR         (EINT_BASE_REG + 0x110)
#define EINT_160169_MASK_CLR         (EINT_BASE_REG + 0x114)

#define EINT_031_SENS_GET            (EINT_BASE_REG + 0x140)
#define EINT_3263_SENS_GET           (EINT_BASE_REG + 0x144)
#define EINT_6495_SENS_GET           (EINT_BASE_REG + 0x148)
#define EINT_96127_SENS_GET          (EINT_BASE_REG + 0x14C)
#define EINT_128159_SENS_GET         (EINT_BASE_REG + 0x150)
#define EINT_160169_SENS_GET         (EINT_BASE_REG + 0x154)


#define EINT_031_SENS_SET            (EINT_BASE_REG + 0x180)
#define EINT_3263_SENS_SET           (EINT_BASE_REG + 0x184)
#define EINT_6495_SENS_SET           (EINT_BASE_REG + 0x188)
#define EINT_96127_SENS_SET          (EINT_BASE_REG + 0x18C)
#define EINT_128159_SENS_SET         (EINT_BASE_REG + 0x190)
#define EINT_160169_SENS_SET         (EINT_BASE_REG + 0x194)

#define EINT_031_SENS_CLR            (EINT_BASE_REG + 0x1C0)
#define EINT_3263_SENS_CLR           (EINT_BASE_REG + 0x1C4)
#define EINT_6495_SENS_CLR           (EINT_BASE_REG + 0x1C8)
#define EINT_96127_SENS_CLR          (EINT_BASE_REG + 0x1CC)
#define EINT_128159_SENS_CLR         (EINT_BASE_REG + 0x1D0)
#define EINT_160169_SENS_CLR         (EINT_BASE_REG + 0x1D4)


#define EINT_031_POL_GET            (EINT_BASE_REG + 0x300)
#define EINT_3263_POL_GET           (EINT_BASE_REG + 0x304)
#define EINT_6495_POL_GET           (EINT_BASE_REG + 0x308)
#define EINT_96127_POL_GET          (EINT_BASE_REG + 0x30C)
#define EINT_128159_POL_GET         (EINT_BASE_REG + 0x310)
#define EINT_160169_POL_GET         (EINT_BASE_REG + 0x314)

#define EINT_031_POL_SET            (EINT_BASE_REG + 0x340)
#define EINT_3263_POL_SET           (EINT_BASE_REG + 0x344)
#define EINT_6495_POL_SET           (EINT_BASE_REG + 0x348)
#define EINT_96127_POL_SET          (EINT_BASE_REG + 0x34C)
#define EINT_128159_POL_SET         (EINT_BASE_REG + 0x350)
#define EINT_160169_POL_SET         (EINT_BASE_REG + 0x354)

#define EINT_031_POL_CLR            (EINT_BASE_REG + 0x380)
#define EINT_3263_POL_CLR           (EINT_BASE_REG + 0x384)
#define EINT_6495_POL_CLR           (EINT_BASE_REG + 0x388)
#define EINT_96127_POL_CLR          (EINT_BASE_REG + 0x38C)
#define EINT_128159_POL_CLR         (EINT_BASE_REG + 0x390)
#define EINT_160169_POL_CLR         (EINT_BASE_REG + 0x394)

#define EINT_031_DBNC_GET            (EINT_BASE_REG + 0x500)
#define EINT_3263_DBNC_GET           (EINT_BASE_REG + 0x504)
#define EINT_6495_DBNC_GET          (EINT_BASE_REG + 0x508)
#define EINT_96127_DBNC_GET          (EINT_BASE_REG + 0x50C)

#define EINT_031_DBNC_SET            (EINT_BASE_REG + 0x600)
#define EINT_3263_DBNC_SET           (EINT_BASE_REG + 0x604)
#define EINT_6495_DBNC_SET          (EINT_BASE_REG + 0x608)
#define EINT_96127_DBNC_SET          (EINT_BASE_REG + 0x60C)

#define EINT_031_DBNC_CLR           (EINT_BASE_REG + 0x700)
#define EINT_3263_DBNC_CLR           (EINT_BASE_REG + 0x704)
#define EINT_6495_DBNC_CLR          (EINT_BASE_REG + 0x708)
#define EINT_96127_DBNC_CLR          (EINT_BASE_REG + 0x70C)
/*ioctl cmmd*/
/*
#define	MEDIATEK_GPIO_015_SET_DIR_IN		0x01
#define	MEDIATEK_GPIO_1631_SET_DIR_IN		0x02
#define MEDIATEK_GPIO_3247_SET_DIR_IN   0x03
#define MEDIATEK_GPIO_4863_SET_DIR_IN   0x04
#define MEDIATEK_GPIO_6479_SET_DIR_IN   0x05
#define	MEDIATEK_GPIO_8095_SET_DIR_IN			0x06
#define	MEDIATEK_GPIO_96111_SET_DIR_IN		0x07
#define MEDIATEK_GPIO_112127_SET_DIR_IN		0x08
#define MEDIATEK_GPIO_128143_SET_DIR_IN   0x09
#define MEDIATEK_GPIO_144159_SET_DIR_IN   0x0a
#define MEDIATEK_GPIO_160175_SET_DIR_IN   0x0b
#define MEDIATEK_GPIO_176191_SET_DIR_IN   0x0c
#define MEDIATEK_GPIO_192207_SET_DIR_IN   0x0d
#define MEDIATEK_GPIO_208223_SET_DIR_IN   0x0e
#define MEDIATEK_GPIO_224239_SET_DIR_IN   0x0f
#define MEDIATEK_GPIO_240255_SET_DIR_IN   0x10
#define MEDIATEK_GPIO_256271_SET_DIR_IN   0x11
#define MEDIATEK_GPIO_272279_SET_DIR_IN   0x12


#define	MEDIATEK_GPIO_015_SET_DIR_OUT		0x13
#define	MEDIATEK_GPIO_1631_SET_DIR_OUT		0x14
#define MEDIATEK_GPIO_3247_SET_DIR_OUT   0x15
#define MEDIATEK_GPIO_4863_SET_DIR_OUT   0x16
#define MEDIATEK_GPIO_6479_SET_DIR_OUT   0x17
#define	MEDIATEK_GPIO_8095_SET_DIR_OUT			0x18
#define	MEDIATEK_GPIO_96111_SET_DIR_OUT		0x19
#define MEDIATEK_GPIO_112127_SET_DIR_OUT		0x1a
#define MEDIATEK_GPIO_128143_SET_DIR_OUT   0x1b
#define MEDIATEK_GPIO_144159_SET_DIR_OUT   0x1c
#define MEDIATEK_GPIO_160175_SET_DIR_OUT   0x1d
#define MEDIATEK_GPIO_176191_SET_DIR_OUT   0x1e
#define MEDIATEK_GPIO_192207_SET_DIR_OUT   0x1f
#define MEDIATEK_GPIO_208223_SET_DIR_OUT   0x20
#define MEDIATEK_GPIO_224239_SET_DIR_OUT   0x21
#define MEDIATEK_GPIO_240255_SET_DIR_OUT   0x22
#define MEDIATEK_GPIO_256271_SET_DIR_OUT   0x23
#define MEDIATEK_GPIO_272279_SET_DIR_OUT   0x24






#define MEDIATEK_GPIO_015_WRITE    					0x25
#define MEDIATEK_GPIO_1631_WRITE    				0x26
#define	MEDIATEK_GPIO_3247_WRITE		0x27
#define	MEDIATEK_GPIO_4863_WRITE		0x28
#define MEDIATEK_GPIO_6479_WRITE		0x29
#define MEDIATEK_GPIO_8095_WRITE    0x2a
#define MEDIATEK_GPIO_96111_WRITE   0x2b
#define	MEDIATEK_GPIO_112127_WRITE		0x2c
#define	MEDIATEK_GPIO_128143_WRITE		0x2d

#define	MEDIATEK_GPIO_144159_WRITE		0x2e
#define	MEDIATEK_GPIO_160175_WRITE		0x2f
#define	MEDIATEK_GPIO_176191_WRITE		0x30
#define	MEDIATEK_GPIO_192207_WRITE		0x31
#define	MEDIATEK_GPIO_208223_WRITE		0x32
#define	MEDIATEK_GPIO_224239_WRITE		0x33
#define	MEDIATEK_GPIO_240255_WRITE		0x34
#define	MEDIATEK_GPIO_256271_WRITE		0x35
#define	MEDIATEK_GPIO_272279_WRITE		0x36




#define MEDIATEK_GPIO_015_READ        0x37
#define MEDIATEK_GPIO_1631_READ    		0x38
#define MEDIATEK_GPIO_3247_READ    		0x39
#define	MEDIATEK_GPIO_4863_READ				0x3a
#define	MEDIATEK_GPIO_6479_READ				0x3b
#define MEDIATEK_GPIO_8095_READ				0x3c
#define MEDIATEK_GPIO_96111_READ    	0x3d
#define MEDIATEK_GPIO_112127_READ    	0x3e
#define	MEDIATEK_GPIO_128143_READ			0x3f
#define	MEDIATEK_GPIO_144159_READ			0x40
#define	MEDIATEK_GPIO_160175_READ			0x41
#define	MEDIATEK_GPIO_176191_READ			0x42
#define	MEDIATEK_GPIO_192207_READ			0x43
#define	MEDIATEK_GPIO_208223_READ			0x44
#define	MEDIATEK_GPIO_224239_READ			0x45
#define	MEDIATEK_GPIO_240255_READ			0x46
#define	MEDIATEK_GPIO_256271_READ			0x47
#define	MEDIATEK_GPIO_272279_READ			0x48


#define	MEDIATEK_GPIO_REG_IRQ						0x49
#define MEDIATEK_GPIO_LED_SET						0x4a
#endif
*/
#define MEDIATEK_GPIO_DIR_IN          0x00
//********************************Direction Out******************************//	
#define MEDIATEK_GPIO_DIR_OUT         0x01
/********************************WRITE************************/
#define MEDIATEK_GPIO_WRITE       0x06
/************************************READ************************/
#define MEDIATEK_GPIO_READ        0x03
/********************************IRQ******************************************/							
#define MEDIATEK_GPIO_REG_IRQ     0x04

#define MEDIATEK_GPIO_LED_SET       0x05
			
			
#define MEDIATEK_GPIO_DATA_MASK		0xFFFFFFFF
#define MEDIATEK_GPIO_DIR_ALLIN		0
#define MEDIATEK_GPIO_DIR_ALLOUT		0xFFFFFFFF

#define MEDIATEK_GPIO_NUMBER		255













/*
 * structure used at regsitration
 */
typedef struct {
	unsigned int irq;		//request irq pin number
	unsigned int pinnum; // gpio pin number	
	unsigned int value;  // gpio output value
	unsigned int value_in;  // gpio input value
	pid_t pid;			//process id to notify
} mediatek_gpio_reg_info;

#define MEDIATEK_GPIO_LED_INFINITY	4000
typedef struct {
	int gpio;			//gpio number (0 ~ 23)
	unsigned int on;		//interval of led on
	unsigned int off;		//interval of led off
	unsigned int blinks;		//number of blinking cycles
	unsigned int rests;		//number of break cycles
	unsigned int times;		//blinking times
} mediatek_gpio_led_info;


#define MEDIATEK_GPIO(x)			(1 << x)

#endif
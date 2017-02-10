/*
 ***************************************************************************
 * MEDIATEK Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, mediatek Technology, Inc.
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
 *
 */
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#if defined (CONFIG_MIPS)
#include <linux/config.h>
#endif
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/sched.h>
#ifdef CONFIG_MEDIATEK_GPIO_LED
#include <linux/timer.h>
#endif
#include <asm/uaccess.h>
#include <linux/delay.h>
#include "mediatek_gpio.h"
#include <mach/eint.h>
#include <mach/mt_gpio.h>
//#include "gpio_define.h"

//#include <asm/rt2880/surfboardint.h>

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static  devfs_handle_t devfs_handle;
#endif

#define NAME			"mediatek_gpio"
#define MEDIATEK_GPIO_DEVNAME	"gpio"
#define MEDIATEK_LED_DEBUG 0
#define MEDIATEK_GPIO_LED_LOW_ACT 1
int mediatek_gpio_major = 241;
#if defined (CONFIG_ARCH_MT7623)
int mediatek_gpio_irqnum = 0;
u32 mediatek_eint031_sta = 0;
u32 mediatek_eint3263_sta = 0;
u32 mediatek_eint6495_sta = 0;
u32 mediatek_eint96127_sta = 0;
u32 mediatek_eint128159_sta = 0;
u32 mediatek_eint031_edge_get = 0;
u32 mediatek_eint3263_edge_get = 0;
u32 mediatek_eint6495_edge_get = 0;
u32 mediatek_eint96127_edge_get = 0;
u32 mediatek_eint128159_edge_get = 0;
#endif

static void mediatek_gpio_irq_handler(int irq);

#define MEDIATEK_GPIO_LED_LOW_ACT 1


mediatek_gpio_reg_info mediatek_gpio_info[MEDIATEK_GPIO_NUMBER];
extern unsigned long volatile jiffies;




#define __BUILD_GPIO_DISPATCH(irq_n) \
static void __tc3262_gpio_dispatch##irq_n(void) \
{								\
	mediatek_gpio_irq_handler(irq_n);				\
}	

#define __BUILD_GPIO_DISPATCH_FUNC(irq_n)  __tc3262_gpio_dispatch##irq_n 

/* pre-built 64 irq dispatch function */
__BUILD_GPIO_DISPATCH(0)
__BUILD_GPIO_DISPATCH(1)
__BUILD_GPIO_DISPATCH(2)
__BUILD_GPIO_DISPATCH(3)
__BUILD_GPIO_DISPATCH(4)
__BUILD_GPIO_DISPATCH(5)
__BUILD_GPIO_DISPATCH(6)
__BUILD_GPIO_DISPATCH(7)
__BUILD_GPIO_DISPATCH(8)
__BUILD_GPIO_DISPATCH(9)
__BUILD_GPIO_DISPATCH(10)
__BUILD_GPIO_DISPATCH(11)
__BUILD_GPIO_DISPATCH(12)
__BUILD_GPIO_DISPATCH(13)
__BUILD_GPIO_DISPATCH(14)
__BUILD_GPIO_DISPATCH(15)
__BUILD_GPIO_DISPATCH(16)
__BUILD_GPIO_DISPATCH(17)
__BUILD_GPIO_DISPATCH(18)
__BUILD_GPIO_DISPATCH(19)
__BUILD_GPIO_DISPATCH(20)
__BUILD_GPIO_DISPATCH(21)
__BUILD_GPIO_DISPATCH(22)
__BUILD_GPIO_DISPATCH(23)
__BUILD_GPIO_DISPATCH(24)
__BUILD_GPIO_DISPATCH(25)
__BUILD_GPIO_DISPATCH(26)
__BUILD_GPIO_DISPATCH(27)
__BUILD_GPIO_DISPATCH(28)
__BUILD_GPIO_DISPATCH(29)
__BUILD_GPIO_DISPATCH(30)
__BUILD_GPIO_DISPATCH(31)
__BUILD_GPIO_DISPATCH(32)
__BUILD_GPIO_DISPATCH(33)
__BUILD_GPIO_DISPATCH(34)
__BUILD_GPIO_DISPATCH(35)
__BUILD_GPIO_DISPATCH(36)
__BUILD_GPIO_DISPATCH(37)
__BUILD_GPIO_DISPATCH(38)
__BUILD_GPIO_DISPATCH(39)
__BUILD_GPIO_DISPATCH(40)
__BUILD_GPIO_DISPATCH(41)
__BUILD_GPIO_DISPATCH(42)
__BUILD_GPIO_DISPATCH(43)
__BUILD_GPIO_DISPATCH(44)
__BUILD_GPIO_DISPATCH(45)
__BUILD_GPIO_DISPATCH(46)
__BUILD_GPIO_DISPATCH(47)
__BUILD_GPIO_DISPATCH(48)
__BUILD_GPIO_DISPATCH(49)
__BUILD_GPIO_DISPATCH(50)
__BUILD_GPIO_DISPATCH(51)
__BUILD_GPIO_DISPATCH(52)
__BUILD_GPIO_DISPATCH(53)
__BUILD_GPIO_DISPATCH(54)
__BUILD_GPIO_DISPATCH(55)
__BUILD_GPIO_DISPATCH(56)
__BUILD_GPIO_DISPATCH(57)
__BUILD_GPIO_DISPATCH(58)
__BUILD_GPIO_DISPATCH(59)
__BUILD_GPIO_DISPATCH(60)
__BUILD_GPIO_DISPATCH(61)
__BUILD_GPIO_DISPATCH(62)
__BUILD_GPIO_DISPATCH(63)
__BUILD_GPIO_DISPATCH(64)
__BUILD_GPIO_DISPATCH(65)
__BUILD_GPIO_DISPATCH(66)
__BUILD_GPIO_DISPATCH(67)
__BUILD_GPIO_DISPATCH(68)
__BUILD_GPIO_DISPATCH(69)
__BUILD_GPIO_DISPATCH(70)
__BUILD_GPIO_DISPATCH(71)
__BUILD_GPIO_DISPATCH(72)
__BUILD_GPIO_DISPATCH(73)
__BUILD_GPIO_DISPATCH(74)
__BUILD_GPIO_DISPATCH(75)
__BUILD_GPIO_DISPATCH(76)
__BUILD_GPIO_DISPATCH(77)
__BUILD_GPIO_DISPATCH(78)
__BUILD_GPIO_DISPATCH(79)
__BUILD_GPIO_DISPATCH(80)
__BUILD_GPIO_DISPATCH(81)
__BUILD_GPIO_DISPATCH(82)
__BUILD_GPIO_DISPATCH(83)
__BUILD_GPIO_DISPATCH(84)
__BUILD_GPIO_DISPATCH(85)
__BUILD_GPIO_DISPATCH(86)
__BUILD_GPIO_DISPATCH(87)
__BUILD_GPIO_DISPATCH(88)
__BUILD_GPIO_DISPATCH(89)
__BUILD_GPIO_DISPATCH(90)
__BUILD_GPIO_DISPATCH(91)
__BUILD_GPIO_DISPATCH(92)
__BUILD_GPIO_DISPATCH(93)
__BUILD_GPIO_DISPATCH(94)
__BUILD_GPIO_DISPATCH(95)
__BUILD_GPIO_DISPATCH(96)
__BUILD_GPIO_DISPATCH(97)
__BUILD_GPIO_DISPATCH(98)
__BUILD_GPIO_DISPATCH(99)
__BUILD_GPIO_DISPATCH(100)
__BUILD_GPIO_DISPATCH(101)
__BUILD_GPIO_DISPATCH(102)
__BUILD_GPIO_DISPATCH(103)
__BUILD_GPIO_DISPATCH(104)
__BUILD_GPIO_DISPATCH(105)
__BUILD_GPIO_DISPATCH(106)
__BUILD_GPIO_DISPATCH(107)
__BUILD_GPIO_DISPATCH(108)
__BUILD_GPIO_DISPATCH(109)
__BUILD_GPIO_DISPATCH(110)
__BUILD_GPIO_DISPATCH(111)
__BUILD_GPIO_DISPATCH(112)
__BUILD_GPIO_DISPATCH(113)
__BUILD_GPIO_DISPATCH(114)
__BUILD_GPIO_DISPATCH(115)
__BUILD_GPIO_DISPATCH(116)
__BUILD_GPIO_DISPATCH(117)
__BUILD_GPIO_DISPATCH(118)
__BUILD_GPIO_DISPATCH(119)
__BUILD_GPIO_DISPATCH(120)
__BUILD_GPIO_DISPATCH(121)
__BUILD_GPIO_DISPATCH(122)
__BUILD_GPIO_DISPATCH(123)
__BUILD_GPIO_DISPATCH(124)
__BUILD_GPIO_DISPATCH(125)
__BUILD_GPIO_DISPATCH(126)
__BUILD_GPIO_DISPATCH(127)
__BUILD_GPIO_DISPATCH(128)
__BUILD_GPIO_DISPATCH(129)
__BUILD_GPIO_DISPATCH(130)
__BUILD_GPIO_DISPATCH(131)
__BUILD_GPIO_DISPATCH(132)
__BUILD_GPIO_DISPATCH(133)
__BUILD_GPIO_DISPATCH(134)
__BUILD_GPIO_DISPATCH(135)
__BUILD_GPIO_DISPATCH(136)
__BUILD_GPIO_DISPATCH(137)
__BUILD_GPIO_DISPATCH(138)
__BUILD_GPIO_DISPATCH(139)
__BUILD_GPIO_DISPATCH(140)
__BUILD_GPIO_DISPATCH(141)
__BUILD_GPIO_DISPATCH(142)
__BUILD_GPIO_DISPATCH(143)
__BUILD_GPIO_DISPATCH(144)
__BUILD_GPIO_DISPATCH(145)
__BUILD_GPIO_DISPATCH(146)
__BUILD_GPIO_DISPATCH(147)
__BUILD_GPIO_DISPATCH(148)
__BUILD_GPIO_DISPATCH(149)
__BUILD_GPIO_DISPATCH(150)
__BUILD_GPIO_DISPATCH(151)
__BUILD_GPIO_DISPATCH(152)
__BUILD_GPIO_DISPATCH(153)
__BUILD_GPIO_DISPATCH(154)
__BUILD_GPIO_DISPATCH(155)
__BUILD_GPIO_DISPATCH(156)
__BUILD_GPIO_DISPATCH(157)
__BUILD_GPIO_DISPATCH(158)
__BUILD_GPIO_DISPATCH(159)
__BUILD_GPIO_DISPATCH(160)

static void (*gpio_dispatch_tab[])(void) =
{
__BUILD_GPIO_DISPATCH_FUNC(0),
__BUILD_GPIO_DISPATCH_FUNC(1),
__BUILD_GPIO_DISPATCH_FUNC(2),
__BUILD_GPIO_DISPATCH_FUNC(3),
__BUILD_GPIO_DISPATCH_FUNC(4),
__BUILD_GPIO_DISPATCH_FUNC(5),
__BUILD_GPIO_DISPATCH_FUNC(6),
__BUILD_GPIO_DISPATCH_FUNC(7),
__BUILD_GPIO_DISPATCH_FUNC(8),
__BUILD_GPIO_DISPATCH_FUNC(9),
__BUILD_GPIO_DISPATCH_FUNC(10),
__BUILD_GPIO_DISPATCH_FUNC(11),
__BUILD_GPIO_DISPATCH_FUNC(12),
__BUILD_GPIO_DISPATCH_FUNC(13),
__BUILD_GPIO_DISPATCH_FUNC(14),
__BUILD_GPIO_DISPATCH_FUNC(15),
__BUILD_GPIO_DISPATCH_FUNC(16),
__BUILD_GPIO_DISPATCH_FUNC(17),
__BUILD_GPIO_DISPATCH_FUNC(18),
__BUILD_GPIO_DISPATCH_FUNC(19),
__BUILD_GPIO_DISPATCH_FUNC(20),
__BUILD_GPIO_DISPATCH_FUNC(21),
__BUILD_GPIO_DISPATCH_FUNC(22),
__BUILD_GPIO_DISPATCH_FUNC(23),
__BUILD_GPIO_DISPATCH_FUNC(24),
__BUILD_GPIO_DISPATCH_FUNC(25),
__BUILD_GPIO_DISPATCH_FUNC(26),
__BUILD_GPIO_DISPATCH_FUNC(27),
__BUILD_GPIO_DISPATCH_FUNC(28),
__BUILD_GPIO_DISPATCH_FUNC(29),
__BUILD_GPIO_DISPATCH_FUNC(30),
__BUILD_GPIO_DISPATCH_FUNC(31),
__BUILD_GPIO_DISPATCH_FUNC(32),
__BUILD_GPIO_DISPATCH_FUNC(33),
__BUILD_GPIO_DISPATCH_FUNC(34),
__BUILD_GPIO_DISPATCH_FUNC(35),
__BUILD_GPIO_DISPATCH_FUNC(36),
__BUILD_GPIO_DISPATCH_FUNC(37),
__BUILD_GPIO_DISPATCH_FUNC(38),
__BUILD_GPIO_DISPATCH_FUNC(39),
__BUILD_GPIO_DISPATCH_FUNC(40),
__BUILD_GPIO_DISPATCH_FUNC(41),
__BUILD_GPIO_DISPATCH_FUNC(42),
__BUILD_GPIO_DISPATCH_FUNC(43),
__BUILD_GPIO_DISPATCH_FUNC(44),
__BUILD_GPIO_DISPATCH_FUNC(45),
__BUILD_GPIO_DISPATCH_FUNC(46),
__BUILD_GPIO_DISPATCH_FUNC(47),
__BUILD_GPIO_DISPATCH_FUNC(48),
__BUILD_GPIO_DISPATCH_FUNC(49),
__BUILD_GPIO_DISPATCH_FUNC(50),
__BUILD_GPIO_DISPATCH_FUNC(51),
__BUILD_GPIO_DISPATCH_FUNC(52),
__BUILD_GPIO_DISPATCH_FUNC(53),
__BUILD_GPIO_DISPATCH_FUNC(54),
__BUILD_GPIO_DISPATCH_FUNC(55),
__BUILD_GPIO_DISPATCH_FUNC(56),
__BUILD_GPIO_DISPATCH_FUNC(57),
__BUILD_GPIO_DISPATCH_FUNC(58),
__BUILD_GPIO_DISPATCH_FUNC(59),
__BUILD_GPIO_DISPATCH_FUNC(60),
__BUILD_GPIO_DISPATCH_FUNC(61),
__BUILD_GPIO_DISPATCH_FUNC(62),
__BUILD_GPIO_DISPATCH_FUNC(63),
__BUILD_GPIO_DISPATCH_FUNC(64),
__BUILD_GPIO_DISPATCH_FUNC(65),
__BUILD_GPIO_DISPATCH_FUNC(66),
__BUILD_GPIO_DISPATCH_FUNC(67),
__BUILD_GPIO_DISPATCH_FUNC(68),
__BUILD_GPIO_DISPATCH_FUNC(69),
__BUILD_GPIO_DISPATCH_FUNC(70),
__BUILD_GPIO_DISPATCH_FUNC(71),
__BUILD_GPIO_DISPATCH_FUNC(72),
__BUILD_GPIO_DISPATCH_FUNC(73),
__BUILD_GPIO_DISPATCH_FUNC(74),
__BUILD_GPIO_DISPATCH_FUNC(75),
__BUILD_GPIO_DISPATCH_FUNC(76),
__BUILD_GPIO_DISPATCH_FUNC(77),
__BUILD_GPIO_DISPATCH_FUNC(78),
__BUILD_GPIO_DISPATCH_FUNC(79),
__BUILD_GPIO_DISPATCH_FUNC(80),
__BUILD_GPIO_DISPATCH_FUNC(81),
__BUILD_GPIO_DISPATCH_FUNC(82),
__BUILD_GPIO_DISPATCH_FUNC(83),
__BUILD_GPIO_DISPATCH_FUNC(84),
__BUILD_GPIO_DISPATCH_FUNC(85),
__BUILD_GPIO_DISPATCH_FUNC(86),
__BUILD_GPIO_DISPATCH_FUNC(87),
__BUILD_GPIO_DISPATCH_FUNC(88),
__BUILD_GPIO_DISPATCH_FUNC(89),
__BUILD_GPIO_DISPATCH_FUNC(90),
__BUILD_GPIO_DISPATCH_FUNC(91),
__BUILD_GPIO_DISPATCH_FUNC(92),
__BUILD_GPIO_DISPATCH_FUNC(93),
__BUILD_GPIO_DISPATCH_FUNC(94),
__BUILD_GPIO_DISPATCH_FUNC(95),
__BUILD_GPIO_DISPATCH_FUNC(96),
__BUILD_GPIO_DISPATCH_FUNC(97),
__BUILD_GPIO_DISPATCH_FUNC(98),
__BUILD_GPIO_DISPATCH_FUNC(99),
__BUILD_GPIO_DISPATCH_FUNC(100),
__BUILD_GPIO_DISPATCH_FUNC(101),
__BUILD_GPIO_DISPATCH_FUNC(102),
__BUILD_GPIO_DISPATCH_FUNC(103),
__BUILD_GPIO_DISPATCH_FUNC(104),
__BUILD_GPIO_DISPATCH_FUNC(105),
__BUILD_GPIO_DISPATCH_FUNC(106),
__BUILD_GPIO_DISPATCH_FUNC(107),
__BUILD_GPIO_DISPATCH_FUNC(108),
__BUILD_GPIO_DISPATCH_FUNC(109),
__BUILD_GPIO_DISPATCH_FUNC(110),
__BUILD_GPIO_DISPATCH_FUNC(111),
__BUILD_GPIO_DISPATCH_FUNC(112),
__BUILD_GPIO_DISPATCH_FUNC(113),
__BUILD_GPIO_DISPATCH_FUNC(114),
__BUILD_GPIO_DISPATCH_FUNC(115),
__BUILD_GPIO_DISPATCH_FUNC(116),
__BUILD_GPIO_DISPATCH_FUNC(117),
__BUILD_GPIO_DISPATCH_FUNC(118),
__BUILD_GPIO_DISPATCH_FUNC(119),
__BUILD_GPIO_DISPATCH_FUNC(120),
__BUILD_GPIO_DISPATCH_FUNC(121),
__BUILD_GPIO_DISPATCH_FUNC(122),
__BUILD_GPIO_DISPATCH_FUNC(123),
__BUILD_GPIO_DISPATCH_FUNC(124),
__BUILD_GPIO_DISPATCH_FUNC(125),
__BUILD_GPIO_DISPATCH_FUNC(126),
__BUILD_GPIO_DISPATCH_FUNC(127),
__BUILD_GPIO_DISPATCH_FUNC(128),
__BUILD_GPIO_DISPATCH_FUNC(129),
__BUILD_GPIO_DISPATCH_FUNC(130),
__BUILD_GPIO_DISPATCH_FUNC(131),
__BUILD_GPIO_DISPATCH_FUNC(132),
__BUILD_GPIO_DISPATCH_FUNC(133),
__BUILD_GPIO_DISPATCH_FUNC(134),
__BUILD_GPIO_DISPATCH_FUNC(135),
__BUILD_GPIO_DISPATCH_FUNC(136),
__BUILD_GPIO_DISPATCH_FUNC(137),
__BUILD_GPIO_DISPATCH_FUNC(138),
__BUILD_GPIO_DISPATCH_FUNC(139),
__BUILD_GPIO_DISPATCH_FUNC(140),
__BUILD_GPIO_DISPATCH_FUNC(141),
__BUILD_GPIO_DISPATCH_FUNC(142),
__BUILD_GPIO_DISPATCH_FUNC(143),
__BUILD_GPIO_DISPATCH_FUNC(144),
__BUILD_GPIO_DISPATCH_FUNC(145),
__BUILD_GPIO_DISPATCH_FUNC(146),
__BUILD_GPIO_DISPATCH_FUNC(147),
__BUILD_GPIO_DISPATCH_FUNC(148),
__BUILD_GPIO_DISPATCH_FUNC(149),
__BUILD_GPIO_DISPATCH_FUNC(150),
__BUILD_GPIO_DISPATCH_FUNC(151),
__BUILD_GPIO_DISPATCH_FUNC(152),
__BUILD_GPIO_DISPATCH_FUNC(153),
__BUILD_GPIO_DISPATCH_FUNC(154),
__BUILD_GPIO_DISPATCH_FUNC(155),
__BUILD_GPIO_DISPATCH_FUNC(156),
__BUILD_GPIO_DISPATCH_FUNC(157),
__BUILD_GPIO_DISPATCH_FUNC(158),
__BUILD_GPIO_DISPATCH_FUNC(159),
__BUILD_GPIO_DISPATCH_FUNC(160)
};
#ifdef CONFIG_MEDIATEK_GPIO_LED
#define MEDIATEK_LED_DEBUG 0
#define MEDIATEK_GPIO_LED_FREQ (HZ/10)
struct timer_list mediatek_gpio_led_timer;
mediatek_gpio_led_info mediatek_gpio_led_data[MEDIATEK_GPIO_NUMBER];
#if defined (CONFIG_ARCH_MT7623)
u16 mediatek_gpio015_led_set = 0;
u16 mediatek_gpio015_led_clr = 0;
u16 mediatek_gpio015 = 0;
u16 mediatek_gpio1631_led_set = 0;
u16 mediatek_gpio1631_led_clr = 0;
u16 mediatek_gpio1631 = 0;
u16 mediatek_gpio3247_led_set = 0;
u16 mediatek_gpio3247_led_clr = 0;
u16 mediatek_gpio3247 = 0;
u16 mediatek_gpio4863_led_set = 0;
u16 mediatek_gpio4863_led_clr = 0;
u16 mediatek_gpio4863 = 0;
u16 mediatek_gpio6479_led_set = 0;
u16 mediatek_gpio6479_led_clr = 0;
u16 mediatek_gpio6479 = 0;
u16 mediatek_gpio8095_led_set = 0;
u16 mediatek_gpio8095_led_clr = 0;
u16 mediatek_gpio8095 = 0;
u16 mediatek_gpio96111_led_set = 0;
u16 mediatek_gpio96111_led_clr = 0;
u16 mediatek_gpio96111 = 0;
u16 mediatek_gpio112127_led_set = 0;
u16 mediatek_gpio112127_led_clr = 0;
u16 mediatek_gpio112127 = 0;
u16 mediatek_gpio128143_led_set = 0;
u16 mediatek_gpio128143_led_clr = 0;
u16 mediatek_gpio128143 = 0;
u16 mediatek_gpio144159_led_set = 0;
u16 mediatek_gpio144159_led_clr = 0;
u16 mediatek_gpio144159= 0;
u16 mediatek_gpio160175_led_set = 0;
u16 mediatek_gpio160175_led_clr = 0;
u16 mediatek_gpio160175 = 0;
u16 mediatek_gpio176191_led_set = 0;
u16 mediatek_gpio176191_led_clr = 0;
u16 mediatek_gpio176191 = 0;
u16 mediatek_gpio192207_led_set = 0;
u16 mediatek_gpio192207_led_clr = 0;
u16 mediatek_gpio192207 = 0;
u16 mediatek_gpio208223_led_set = 0;
u16 mediatek_gpio208223_led_clr = 0;
u16 mediatek_gpio208223 = 0;
u16 mediatek_gpio224239_led_set = 0;
u16 mediatek_gpio224239_led_clr = 0;
u16 mediatek_gpio224239 = 0;
u16 mediatek_gpio240255_led_set = 0;
u16 mediatek_gpio240255_led_clr = 0;
u16 mediatek_gpio240255 = 0;
#endif



struct mediatek_gpio_led_status_t {
	int ticks;
	unsigned int ons;
	unsigned int offs;
	unsigned int resting;
	unsigned int times;
} mediatek_gpio_led_stat[MEDIATEK_GPIO_NUMBER];
#endif

#if defined (CONFIG_ARCH_MT7623)
#define MT_EINT_IRQ_ID                      (32 + 113)//10.2 update
#endif


MODULE_DESCRIPTION("MEDIATEK SoC GPIO Driver");
MODULE_AUTHOR("Winfred Lu <winfred_lu@mediatektech.com.tw>");
MODULE_LICENSE("GPL");
//static mediatek_gpio_reg_info info;
int mediatek_gpio_led_set(mediatek_gpio_led_info led)
{
	
#ifdef CONFIG_MEDIATEK_GPIO_LED
	if (0 <= led.gpio && led.gpio < MEDIATEK_GPIO_NUMBER) {
		if (led.on > MEDIATEK_GPIO_LED_INFINITY)
			led.on = MEDIATEK_GPIO_LED_INFINITY;
		if (led.off > MEDIATEK_GPIO_LED_INFINITY)
			led.off = MEDIATEK_GPIO_LED_INFINITY;
		if (led.blinks > MEDIATEK_GPIO_LED_INFINITY)
			led.blinks = MEDIATEK_GPIO_LED_INFINITY;
		if (led.rests > MEDIATEK_GPIO_LED_INFINITY)
			led.rests = MEDIATEK_GPIO_LED_INFINITY;
		if (led.times > MEDIATEK_GPIO_LED_INFINITY)
			led.times = MEDIATEK_GPIO_LED_INFINITY;
		if (led.on == 0 && led.off == 0 && led.blinks == 0 &&
				led.rests == 0) {
			mediatek_gpio_led_data[led.gpio].gpio = -1; //stop it
			return 0;
		}
		//register led data
		mediatek_gpio_led_data[led.gpio].gpio = led.gpio;
		mediatek_gpio_led_data[led.gpio].on = (led.on == 0)? 1 : led.on;
		mediatek_gpio_led_data[led.gpio].off = (led.off == 0)? 1 : led.off;
		mediatek_gpio_led_data[led.gpio].blinks = (led.blinks == 0)? 1 : led.blinks;
		mediatek_gpio_led_data[led.gpio].rests = (led.rests == 0)? 1 : led.rests;
		mediatek_gpio_led_data[led.gpio].times = (led.times == 0)? 1 : led.times;

		//clear previous led status
		mediatek_gpio_led_stat[led.gpio].ticks = -1;
		mediatek_gpio_led_stat[led.gpio].ons = 0;
		mediatek_gpio_led_stat[led.gpio].offs = 0;
		mediatek_gpio_led_stat[led.gpio].resting = 0;
		mediatek_gpio_led_stat[led.gpio].times = 0;

		printk("led=%d, on=%d, off=%d, blinks=%d, reset=%d, time=%d\n",
				mediatek_gpio_led_data[led.gpio].gpio,
				mediatek_gpio_led_data[led.gpio].on,
				mediatek_gpio_led_data[led.gpio].off,
				mediatek_gpio_led_data[led.gpio].blinks,
				mediatek_gpio_led_data[led.gpio].rests,
				mediatek_gpio_led_data[led.gpio].times);
		//set gpio direction to 'out'
    //mt_set_gpio_out(led.gpio , GPIO_OUT_ZERO);

#if MEDIATEK_LED_DEBUG
		printk("gpio_%d - led.on=%d led.off=%d led.blinks=%d led.rests=%d led.times=%d\n",
				led.gpio, led.on, led.off, led.blinks,
				led.rests, led.times);
#endif
	}
	else {
		printk(KERN_ERR NAME ": gpio(%d) out of range\n", led.gpio);
		return -1;
	}
	return 0;
#else
	printk(KERN_ERR NAME ": gpio led support not built\n");
	return -1;
#endif
}
EXPORT_SYMBOL(mediatek_gpio_led_set);

long mediatek_gpio_ioctl(struct file *file, unsigned int req,
		unsigned long arg)
{
	unsigned long tmp;
	//unsigned long tmp_d;
	mediatek_gpio_reg_info info;
#ifdef CONFIG_MEDIATEK_GPIO_LED
	mediatek_gpio_led_info led;
#endif

	req &= MEDIATEK_GPIO_DATA_MASK;

	switch(req) {

		
#if defined (CONFIG_ARCH_MT7623)
#if(1)
  case MEDIATEK_GPIO_DIR_IN:
  	printk("MEDIATEK_GPIO_IN pin number = %lu\n", arg);
  	mt_set_gpio_mode(arg, GPIO_MODE_00);
  	mt_set_gpio_dir(arg, GPIO_DIR_IN);
  break;
//********************************Direction Out******************************//	
  case MEDIATEK_GPIO_DIR_OUT:
  	
  	printk("MEDIATEK_GPIO_OUT pin number = %lu\n", arg);
  	mt_set_gpio_mode(arg, GPIO_MODE_00);
  	mt_set_gpio_dir(arg, GPIO_DIR_OUT);
  break;		
#endif
/********************************WRITE************************/
  case MEDIATEK_GPIO_WRITE:
  	copy_from_user(&info, (mediatek_gpio_reg_info *)arg, sizeof(info));
  	printk("MEDIATEK_GPIO_WRITE pin number = %d\n", info.pinnum);
  	mt_set_gpio_mode(info.pinnum, GPIO_MODE_00);
  	mt_set_gpio_dir(info.pinnum, GPIO_DIR_OUT);
  	mt_set_gpio_out(info.pinnum, info.value);
  break;
/************************************READ************************/
	case MEDIATEK_GPIO_READ:
		copy_from_user(&info, (mediatek_gpio_reg_info *)arg, sizeof(info));
		printk("MEDIATEK_GPIO_READ pin number = %d\n", info.pinnum);
  	mt_set_gpio_mode(info.pinnum, GPIO_MODE_00);
  	mt_set_gpio_dir(info.pinnum, GPIO_DIR_IN);
		tmp = mt_get_gpio_in(info.pinnum);
		info.value_in = tmp;
		copy_to_user( (mediatek_gpio_reg_info __user *)arg, &info, sizeof(info));
		//put_user(tmp, (int __user *)arg);				
	break;					

/********************************IRQ******************************************/							
	case MEDIATEK_GPIO_REG_IRQ:
		copy_from_user(&info, (mediatek_gpio_reg_info *)arg, sizeof(info));
		mediatek_gpio_info[info.irq].pid = info.pid;
		mt_eint_registration(info.irq, EINTF_TRIGGER_FALLING, gpio_dispatch_tab[info.irq], 1); 
		//mt_eint_unmask(info.irq);
		break;
		case MEDIATEK_GPIO_LED_SET:
#ifdef CONFIG_MEDIATEK_GPIO_LED
		copy_from_user(&led, (mediatek_gpio_led_info *)arg, sizeof(led));
		mt_set_gpio_mode(led.gpio, GPIO_MODE_00);
  	mt_set_gpio_dir(led.gpio, GPIO_DIR_OUT);
		mediatek_gpio_led_set(led);
#else
		printk(KERN_ERR NAME ": gpio led support not built\n");
#endif
		break;					
#endif		//end 8590


	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

int mediatek_gpio_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_INC_USE_COUNT;
#else
	try_module_get(THIS_MODULE);
#endif
	return 0;
}

int mediatek_gpio_release(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#else
	module_put(THIS_MODULE);
#endif
	return 0;
}

struct file_operations mediatek_gpio_fops =
{
	owner:		THIS_MODULE,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	unlocked_ioctl:	mediatek_gpio_ioctl,
#else
	ioctl:		mediatek_gpio_ioctl,
#endif
	open:		mediatek_gpio_open,
	release:	mediatek_gpio_release,
};



#ifdef CONFIG_MEDIATEK_GPIO_LED

#if MEDIATEK_GPIO_LED_LOW_ACT


#if defined (CONFIG_ARCH_MT7623)
#define __LED_ON(gpio)      mediatek_gpio015_led_clr |= MEDIATEK_GPIO(gpio);
#define __LED_OFF(gpio)     mediatek_gpio015_led_set |= MEDIATEK_GPIO(gpio);
#define __LED1631_ON(gpio)      mediatek_gpio1631_led_clr |= MEDIATEK_GPIO((gpio-16));
#define __LED1631_OFF(gpio)     mediatek_gpio1631_led_set |= MEDIATEK_GPIO((gpio-16));
#define __LED3247_ON(gpio)      mediatek_gpio3247_led_clr |= MEDIATEK_GPIO((gpio-32));
#define __LED3247_OFF(gpio)     mediatek_gpio3247_led_set |= MEDIATEK_GPIO((gpio-32));
#define __LED4863_ON(gpio)      mediatek_gpio4863_led_clr |= MEDIATEK_GPIO((gpio-48));
#define __LED4863_OFF(gpio)     mediatek_gpio4863_led_set |= MEDIATEK_GPIO((gpio-48));
#define __LED6479_ON(gpio)      mediatek_gpio6479_led_clr |= MEDIATEK_GPIO((gpio-64));
#define __LED6479_OFF(gpio)     mediatek_gpio6479_led_set |= MEDIATEK_GPIO((gpio-64));
#define __LED8095_ON(gpio)      mediatek_gpio8095_led_clr |= MEDIATEK_GPIO((gpio-80));
#define __LED8095_OFF(gpio)     mediatek_gpio8095_led_set |= MEDIATEK_GPIO((gpio-80));
#define __LED96111_ON(gpio)      mediatek_gpio96111_led_clr |= MEDIATEK_GPIO((gpio-96));
#define __LED96111_OFF(gpio)     mediatek_gpio96111_led_set |= MEDIATEK_GPIO((gpio-96));
#define __LED112127_ON(gpio)      mediatek_gpio112127_led_clr |= MEDIATEK_GPIO((gpio-112));
#define __LED112127_OFF(gpio)     mediatek_gpio112127_led_set |= MEDIATEK_GPIO((gpio-112));
#define __LED128143_ON(gpio)      mediatek_gpio128143_led_clr |= MEDIATEK_GPIO((gpio-128));
#define __LED128143_OFF(gpio)     mediatek_gpio128143_led_set |= MEDIATEK_GPIO((gpio-128));
#define __LED144159_ON(gpio)      mediatek_gpio144159_led_clr |= MEDIATEK_GPIO((gpio-144));
#define __LED144159_OFF(gpio)     mediatek_gpio144159_led_set |= MEDIATEK_GPIO((gpio-144));
#define __LED160175_ON(gpio)      mediatek_gpio160175_led_clr |= MEDIATEK_GPIO((gpio-160));
#define __LED160175_OFF(gpio)     mediatek_gpio160175_led_set |= MEDIATEK_GPIO((gpio-160));
#define __LED176191_ON(gpio)      mediatek_gpio176191_led_clr |= MEDIATEK_GPIO((gpio-176));
#define __LED176191_OFF(gpio)     mediatek_gpio176191_led_set |= MEDIATEK_GPIO((gpio-176));
#define __LED192207_ON(gpio)      mediatek_gpio192207_led_clr |= MEDIATEK_GPIO((gpio-192));
#define __LED192207_OFF(gpio)     mediatek_gpio192207_led_set |= MEDIATEK_GPIO((gpio-192));
#define __LED208223_ON(gpio)      mediatek_gpio208223_led_clr |= MEDIATEK_GPIO((gpio-208));
#define __LED208223_OFF(gpio)     mediatek_gpio208223_led_set |= MEDIATEK_GPIO((gpio-208));
#define __LED224239_ON(gpio)      mediatek_gpio224239_led_clr |= MEDIATEK_GPIO((gpio-224));
#define __LED224239_OFF(gpio)     mediatek_gpio224239_led_set |= MEDIATEK_GPIO((gpio-224));
#define __LED240255_ON(gpio)      mediatek_gpio240255_led_clr |= MEDIATEK_GPIO((gpio-240));
#define __LED240255_OFF(gpio)     mediatek_gpio240255_led_set |= MEDIATEK_GPIO((gpio-240));

#endif

#else//High_ACT

#if defined (CONFIG_ARCH_MT7623)
#define __LED_ON(gpio)      mediatek_gpio015_led_set |= MEDIATEK_GPIO(gpio);
#define __LED_OFF(gpio)     mediatek_gpio015_led_clr |= MEDIATEK_GPIO(gpio);
#define __LED1631_ON(gpio)      mediatek_gpio1631_led_set |= MEDIATEK_GPIO((gpio-16));
#define __LED1631_OFF(gpio)     mediatek_gpio1631_led_clr |= MEDIATEK_GPIO((gpio-16));
#define __LED3247_ON(gpio)      mediatek_gpio3247_led_set |= MEDIATEK_GPIO((gpio-32));
#define __LED3247_OFF(gpio)     mediatek_gpio3247_led_clr |= MEDIATEK_GPIO((gpio-32));
#define __LED4863_ON(gpio)      mediatek_gpio4863_led_set |= MEDIATEK_GPIO((gpio-48));
#define __LED4863_OFF(gpio)     mediatek_gpio4863_led_clr |= MEDIATEK_GPIO((gpio-48));
#define __LED6479_ON(gpio)      mediatek_gpio6479_led_set |= MEDIATEK_GPIO((gpio-64));
#define __LED6479_OFF(gpio)     mediatek_gpio6479_led_clr |= MEDIATEK_GPIO((gpio-64));
#define __LED8095_ON(gpio)      mediatek_gpio8095_led_set |= MEDIATEK_GPIO((gpio-80));
#define __LED8095_OFF(gpio)     mediatek_gpio8095_led_clr |= MEDIATEK_GPIO((gpio-80));
#define __LED96111_ON(gpio)      mediatek_gpio96111_led_set |= MEDIATEK_GPIO((gpio-96));
#define __LED96111_OFF(gpio)     mediatek_gpio96111_led_clr |= MEDIATEK_GPIO((gpio-96));
#define __LED112127_ON(gpio)      mediatek_gpio112127_led_set |= MEDIATEK_GPIO((gpio-112));
#define __LED112127_OFF(gpio)     mediatek_gpio112127_led_clr |= MEDIATEK_GPIO((gpio-112));
#define __LED128143_ON(gpio)      mediatek_gpio128143_led_set |= MEDIATEK_GPIO((gpio-128));
#define __LED128143_OFF(gpio)     mediatek_gpio128143_led_clr |= MEDIATEK_GPIO((gpio-128));

#define __LED144159_ON(gpio)      mediatek_gpio144159_led_set |= MEDIATEK_GPIO((gpio-144));
#define __LED144159_OFF(gpio)     mediatek_gpio144159_led_clr |= MEDIATEK_GPIO((gpio-144));
#define __LED160175_ON(gpio)      mediatek_gpio160175_led_set |= MEDIATEK_GPIO((gpio-160));
#define __LED160175_OFF(gpio)     mediatek_gpio160175_led_clr |= MEDIATEK_GPIO((gpio-160));
#define __LED176191_ON(gpio)      mediatek_gpio176191_led_set |= MEDIATEK_GPIO((gpio-176));
#define __LED176191_OFF(gpio)     mediatek_gpio176191_led_clr |= MEDIATEK_GPIO((gpio-176));
#define __LED192207_ON(gpio)      mediatek_gpio192207_led_set |= MEDIATEK_GPIO((gpio-192));
#define __LED192207_OFF(gpio)     mediatek_gpio192207_led_clr |= MEDIATEK_GPIO((gpio-192));
#define __LED208223_ON(gpio)      mediatek_gpio208223_led_set |= MEDIATEK_GPIO((gpio-208));
#define __LED208223_OFF(gpio)     mediatek_gpio208223_led_clr |= MEDIATEK_GPIO((gpio-208));
#define __LED224239_ON(gpio)      mediatek_gpio224239_led_set |= MEDIATEK_GPIO((gpio-224));
#define __LED224239_OFF(gpio)     mediatek_gpio224239_led_clr |= MEDIATEK_GPIO((gpio-224));
#define __LED240255_ON(gpio)      mediatek_gpio240255_led_set |= MEDIATEK_GPIO((gpio-240));
#define __LED240255_OFF(gpio)     mediatek_gpio240255_led_clr |= MEDIATEK_GPIO((gpio-240));
#endif
#endif


void mediatek_gpio_led_do_timer(unsigned long unused)
{
	int i;
	unsigned int x;
	//printk("*************mediatek_gpio_led_do_timer\n");

#if defined (CONFIG_ARCH_MT7623)
	for (i = 0; i < 16; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1){ //-1 means unused	
			continue;
		}
		mediatek_gpio015 = (1 << i);
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED_ON(i);	
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED_OFF(i);	
			continue;
		}	
		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		
		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}	
/***********************************************************/	
	for (i = 16; i < 32; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio1631 = 1 << (i - 16);
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED1631_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED1631_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED1631_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED1631_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED1631_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED1631_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}
/*************************************************/
	for (i = 32; i < 48; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio3247 = 1 << (i - 32);
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED3247_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED3247_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED3247_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED3247_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED3247_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED3247_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}	
/****************************************************/	
	for (i = 48; i < 64; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio4863 = 1 << (i - 48);
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED4863_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED4863_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED4863_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED4863_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED4863_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED4863_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}		
/******************************************************/
	for (i = 64; i < 80; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio6479 = 1 << (i - 64);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED6479_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED6479_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED6479_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED6479_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED6479_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED6479_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}	
/********************************************************/
	for (i = 80; i < 96; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio8095 = 1 << (i - 80);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED8095_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED8095_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED8095_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED8095_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED8095_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED8095_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}	
/**************************************************/	
for (i = 96; i < 112; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio96111 = 1 << (i - 96);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED96111_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED96111_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED96111_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED96111_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED96111_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED96111_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}		
/*********************************************************/	
for (i = 112; i < 128; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio112127 = 1 << (i - 112);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED112127_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED112127_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED112127_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED112127_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED112127_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED112127_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}			
	/*******************************************************/
for (i = 128; i < 143; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
			mediatek_gpio128143 =  1 << (i - 128);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED128143_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED128143_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED128143_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED128143_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED128143_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED128143_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}				
	/*******************************************************/
for (i = 144; i < 159; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
			mediatek_gpio144159 =  1 << (i - 144);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED144159_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED144159_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED144159_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED144159_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED144159_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED144159_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}					
	/*******************************************************/
for (i = 160; i < 175; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
			mediatek_gpio160175 =  1 << (i - 160);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED160175_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED160175_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED160175_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED160175_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED160175_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED160175_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}		
	/*******************************************************/
for (i = 176; i < 191; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
			mediatek_gpio176191 =  1 << (i - 176);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED176191_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED176191_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED160175_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED176191_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED176191_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED176191_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}			
	/*******************************************************/
for (i = 192; i < 207; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
			mediatek_gpio192207 =  1 << (i - 192);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED192207_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED192207_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED192207_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED192207_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED192207_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED192207_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}
	
	
		/*******************************************************/
for (i = 208; i < 223; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
			mediatek_gpio208223 =  1 << (i - 208);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED208223_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED208223_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED208223_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED208223_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED208223_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED208223_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}			
		/*******************************************************/
for (i = 224; i < 239; i++) {
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
			mediatek_gpio224239 =  1 << (i - 224);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED224239_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED224239_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED224239_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED224239_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED224239_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED224239_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}																						
	/*******************************************************/
for (i = 240; i < 255; i++) {
	
		mediatek_gpio_led_stat[i].ticks++;
		if (mediatek_gpio_led_data[i].gpio == -1) //-1 means unused
			continue;
		mediatek_gpio240255 = 1 << (i - 240);	
		if (mediatek_gpio_led_data[i].on == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].off == 0) { //always on
			__LED240255_ON(i);
			continue;
		}
		if (mediatek_gpio_led_data[i].off == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].on == 0 ||
				mediatek_gpio_led_data[i].blinks == 0 ||
				mediatek_gpio_led_data[i].times == 0) { //always off
			__LED240255_OFF(i);
			continue;
		}

		//led turn on or off
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			x = mediatek_gpio_led_stat[i].ticks % (mediatek_gpio_led_data[i].on
					+ mediatek_gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = mediatek_gpio_led_data[i].blinks / 2;
			b = mediatek_gpio_led_data[i].rests / 2;
			c = mediatek_gpio_led_data[i].blinks % 2;
			d = mediatek_gpio_led_data[i].rests % 2;
			o = mediatek_gpio_led_data[i].on + mediatek_gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + mediatek_gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = mediatek_gpio_led_stat[i].ticks %
				(t + b * o + mediatek_gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < mediatek_gpio_led_data[i].on) {
			__LED240255_ON(i);
			if (mediatek_gpio_led_stat[i].ticks && x == 0)
				mediatek_gpio_led_stat[i].offs++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d on,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}
		else {
			__LED240255_OFF(i);
			if (x == mediatek_gpio_led_data[i].on)
				mediatek_gpio_led_stat[i].ons++;
#if MEDIATEK_LED_DEBUG
			printk("t%d gpio%d off,", mediatek_gpio_led_stat[i].ticks, i);
#endif
		}

		//blinking or resting
		if (mediatek_gpio_led_data[i].blinks == MEDIATEK_GPIO_LED_INFINITY ||
				mediatek_gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = mediatek_gpio_led_stat[i].ons + mediatek_gpio_led_stat[i].offs;
			if (!mediatek_gpio_led_stat[i].resting) {
				if (x == mediatek_gpio_led_data[i].blinks) {
					mediatek_gpio_led_stat[i].resting = 1;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
					mediatek_gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == mediatek_gpio_led_data[i].rests) {
					mediatek_gpio_led_stat[i].resting = 0;
					mediatek_gpio_led_stat[i].ons = 0;
					mediatek_gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (mediatek_gpio_led_stat[i].resting) {
			__LED240255_OFF(i);
#if MEDIATEK_LED_DEBUG
			printk("resting,");
		} else {
			printk("blinking,");
#endif
		}

		//number of times
		if (mediatek_gpio_led_data[i].times != MEDIATEK_GPIO_LED_INFINITY)
		{
			if (mediatek_gpio_led_stat[i].times ==
					mediatek_gpio_led_data[i].times) {
				__LED240255_OFF(i);
				mediatek_gpio_led_data[i].gpio = -1; //stop
			}
#if MEDIATEK_LED_DEBUG
			printk("T%d\n", mediatek_gpio_led_stat[i].times);
		} else {
			printk("T@\n");
#endif
		}
	}					
	
	
	
#endif



#if defined (CONFIG_ARCH_MT7623)
		*(volatile u16 *)(GPIO_015_DOUT) = (*(volatile u16 *)(GPIO_015_DOUT) & (~(mediatek_gpio015))) | mediatek_gpio015_led_set;		
		*(volatile u16 *)(GPIO_1631_DOUT) = (*(volatile u16 *)(GPIO_1631_DOUT) & (~(mediatek_gpio1631))) | mediatek_gpio1631_led_set;	
		*(volatile u16 *)(GPIO_3247_DOUT) = (*(volatile u16 *)(GPIO_3247_DOUT) & (~(mediatek_gpio3247))) | mediatek_gpio3247_led_set;
		*(volatile u16 *)(GPIO_4863_DOUT) = (*(volatile u16 *)(GPIO_4863_DOUT) & (~(mediatek_gpio4863))) | mediatek_gpio4863_led_set;	
		*(volatile u16 *)(GPIO_6479_DOUT) = (*(volatile u16 *)(GPIO_6479_DOUT) & (~(mediatek_gpio6479))) | mediatek_gpio6479_led_set;	
		*(volatile u16 *)(GPIO_8095_DOUT) = (*(volatile u16 *)(GPIO_8095_DOUT) & (~(mediatek_gpio8095))) | mediatek_gpio8095_led_set;
		*(volatile u16 *)(GPIO_96111_DOUT) = (*(volatile u16 *)(GPIO_96111_DOUT) & (~(mediatek_gpio96111))) | mediatek_gpio96111_led_set;
		*(volatile u16 *)(GPIO_112127_DOUT) = (*(volatile u16 *)(GPIO_112127_DOUT) & (~(mediatek_gpio112127))) | mediatek_gpio112127_led_set;
    *(volatile u16 *)(GPIO_128143_DOUT) = (*(volatile u16 *)(GPIO_128143_DOUT) & (~(mediatek_gpio128143))) | mediatek_gpio128143_led_set;
    *(volatile u16 *)(GPIO_144159_DOUT) = (*(volatile u16 *)(GPIO_144159_DOUT) & (~(mediatek_gpio144159))) | mediatek_gpio144159_led_set;
    *(volatile u16 *)(GPIO_160175_DOUT) = (*(volatile u16 *)(GPIO_160175_DOUT) & (~(mediatek_gpio160175))) | mediatek_gpio160175_led_set;
    *(volatile u16 *)(GPIO_176191_DOUT) = (*(volatile u16 *)(GPIO_176191_DOUT) & (~(mediatek_gpio176191))) | mediatek_gpio176191_led_set;
    *(volatile u16 *)(GPIO_192207_DOUT) = (*(volatile u16 *)(GPIO_192207_DOUT) & (~(mediatek_gpio192207))) | mediatek_gpio192207_led_set;
    *(volatile u16 *)(GPIO_208223_DOUT) = (*(volatile u16 *)(GPIO_208223_DOUT) & (~(mediatek_gpio208223))) | mediatek_gpio208223_led_set;
    *(volatile u16 *)(GPIO_224239_DOUT) = (*(volatile u16 *)(GPIO_224239_DOUT) & (~(mediatek_gpio224239))) | mediatek_gpio224239_led_set;                    
    *(volatile u16 *)(GPIO_240255_DOUT) = (*(volatile u16 *)(GPIO_240255_DOUT) & (~(mediatek_gpio240255))) | mediatek_gpio240255_led_set;
#endif

#if defined (CONFIG_ARCH_MT7623)
#if MEDIATEK_LED_DEBUG
	printk("mediatek_gpio015_led_set= %x, mediatek_gpio015_led_clr= %x\n", mediatek_gpio015_led_set, mediatek_gpio015_led_clr);
  printk("mediatek_gpio1631_led_set= %x, mediatek_gpio1631_led_clr= %x\n", mediatek_gpio1631_led_set, mediatek_gpio1631_led_clr);
	printk("mediatek_gpio3247_led_set= %x, mediatek_gpio3247_led_clr= %x\n", mediatek_gpio3247_led_set, mediatek_gpio3247_led_clr);
  printk("mediatek_gpio4863_led_set= %x, mediatek_gpio4863_led_clr= %x\n", mediatek_gpio4863_led_set, mediatek_gpio4863_led_clr);  
	printk("mediatek_gpio6479_led_set= %x, mediatek_gpio6479_led_clr= %x\n", mediatek_gpio6479_led_set, mediatek_gpio6479_led_clr);
  printk("mediatek_gpio8095_led_set= %x, mediatek_gpio8095_led_clr= %x\n", mediatek_gpio8095_led_set, mediatek_gpio8095_led_clr);    
  printk("mediatek_gpio96111_led_set= %x, mediatek_gpio96111_led_clr= %x\n", mediatek_gpio96111_led_set, mediatek_gpio96111_led_clr);   
  printk("mediatek_gpio112127_led_set= %x, mediatek_gpio112127_led_clr= %x\n", mediatek_gpio112127_led_set, mediatek_gpio112127_led_clr);    
  printk("mediatek_gpio128143_led_set= %x, mediatek_gpio128143_led_clr= %x\n", mediatek_gpio128143_led_set, mediatek_gpio128143_led_clr); 
  printk("mediatek_gpio144159_led_set= %x, mediatek_gpio144159_led_clr= %x\n", mediatek_gpio144159_led_set, mediatek_gpio144159_led_clr); 
  printk("mediatek_gpio160175_led_set= %x, mediatek_gpio160175_led_clr= %x\n", mediatek_gpio160175_led_set, mediatek_gpio160175_led_clr);
  printk("mediatek_gpio176191_led_set= %x, mediatek_gpio176191_led_clr= %x\n", mediatek_gpio176191_led_set, mediatek_gpio176191_led_clr);
  printk("mediatek_gpio192207_led_set= %x, mediatek_gpio192207_led_clr= %x\n", mediatek_gpio192207_led_set, mediatek_gpio192207_led_clr); 
  printk("mediatek_gpio208223_led_set= %x, mediatek_gpio208223_led_clr= %x\n", mediatek_gpio208223_led_set, mediatek_gpio208223_led_clr);
  printk("mediatek_gpio224239_led_set= %x, mediatek_gpio224239_led_clr= %x\n", mediatek_gpio224239_led_set, mediatek_gpio224239_led_clr);
  printk("mediatek_gpio240255_led_set= %x, mediatek_gpio240255_led_clr= %x\n", mediatek_gpio240255_led_set, mediatek_gpio240255_led_clr);           
 #endif   
 mediatek_gpio015_led_set = 0;
 mediatek_gpio015_led_clr = 0;
 mediatek_gpio1631_led_set = 0;
 mediatek_gpio1631_led_clr = 0;
 mediatek_gpio3247_led_set = 0;
 mediatek_gpio3247_led_clr = 0;
 mediatek_gpio4863_led_set = 0;
 mediatek_gpio4863_led_clr = 0;
 mediatek_gpio6479_led_set = 0;
 mediatek_gpio6479_led_clr = 0;
 mediatek_gpio8095_led_set = 0;
 mediatek_gpio8095_led_clr = 0;
 mediatek_gpio96111_led_set = 0;
 mediatek_gpio96111_led_clr = 0;
 mediatek_gpio112127_led_set = 0;
 mediatek_gpio112127_led_clr = 0;
 mediatek_gpio128143_led_set = 0;
 mediatek_gpio128143_led_clr = 0;
 mediatek_gpio144159_led_set = 0;
 mediatek_gpio144159_led_clr = 0;
 mediatek_gpio160175_led_set = 0;
 mediatek_gpio160175_led_clr = 0; 
 mediatek_gpio176191_led_set = 0;
 mediatek_gpio176191_led_clr = 0;  
 mediatek_gpio192207_led_set = 0;
 mediatek_gpio192207_led_clr = 0;  
 mediatek_gpio208223_led_set = 0;
 mediatek_gpio208223_led_clr = 0;
 mediatek_gpio224239_led_set = 0;
 mediatek_gpio224239_led_clr = 0; 
 mediatek_gpio240255_led_set = 0;
 mediatek_gpio240255_led_clr = 0;
#endif


	init_timer(&mediatek_gpio_led_timer);
	mediatek_gpio_led_timer.expires = jiffies + MEDIATEK_GPIO_LED_FREQ;
	add_timer(&mediatek_gpio_led_timer);
}

void mediatek_gpio_led_init_timer(void)
{
	int i;

	for (i = 0; i < MEDIATEK_GPIO_NUMBER; i++)
		mediatek_gpio_led_data[i].gpio = -1; //-1 means unused
#if MEDIATEK_GPIO_LED_LOW_ACT


#if defined (CONFIG_ARCH_MT7623)
	mediatek_gpio015_led_set = 0xffff;																		
	mediatek_gpio1631_led_set = 0xffff;																		
	mediatek_gpio3247_led_set = 0xffff;																		
	mediatek_gpio4863_led_set = 0xffff;																		
	mediatek_gpio6479_led_set = 0xffff;																		
	mediatek_gpio8095_led_set = 0xffff;																		
	mediatek_gpio96111_led_set = 0xffff;																		
	mediatek_gpio112127_led_set = 0xffff;																		
	mediatek_gpio128143_led_set = 0xffff;		
	mediatek_gpio240255_led_set = 0xffff;																		
	mediatek_gpio240255_led_set = 0xffff;																		
#endif



#else // mediatek_GPIO_LED_LOW_ACT //



#if defined (CONFIG_ARCH_MT7623)
	mediatek_gpio015_led_clr = 0xffff;																		
	mediatek_gpio1631_led_clr = 0xffff;																		
	mediatek_gpio3247_led_clr = 0xffff;																		
	mediatek_gpio4863_led_clr = 0xffff;																		
	mediatek_gpio6479_led_clr = 0xffff;																		
	mediatek_gpio8095_led_clr = 0xffff;																		
	mediatek_gpio96111_led_clr = 0xffff;																		
	mediatek_gpio112127_led_clr = 0xffff;																		
	mediatek_gpio128143_led_clr = 0xffff;
 mediatek_gpio240255_led_clr = 0xffff;	
#endif

#endif // mediatek_GPIO_LED_LOW_ACT //

	init_timer(&mediatek_gpio_led_timer);
	mediatek_gpio_led_timer.function = mediatek_gpio_led_do_timer;
	mediatek_gpio_led_timer.expires = jiffies + MEDIATEK_GPIO_LED_FREQ;
	add_timer(&mediatek_gpio_led_timer);
}
#endif

/*

static void mt_gpio_default_setting(void)
{
	unsigned int idx;
	u16 val;
	printk("**************mt_gpio_default_setting\n");
	//GPIO Direction
	for (idx = 0; idx < 11; idx++) {
		val = gpio_init_dir1_data[idx];
		*(volatile u32 *)(GPIO_DIR_1 + (idx*0x10))  = val;
  } 
	for (idx = 0; idx < 7; idx++) {
		val = gpio_init_dir2_data[idx];
		*(volatile u32 *)(GPIO_DIR_2 + (idx*0x10))  = val;
  } 	
  //GPIO Pull Enable	
	for (idx = 0; idx < 18; idx++){
		val = gpio_init_pullen_data[idx];
		*(volatile u32 *)(GPIO_PENA + (idx*0x10))  = val;
  }
  //GPIO Pull Select
	for (idx = 0; idx < 18; idx++){ 
		val = gpio_init_pullsel_data[idx];
		*(volatile u32 *)(GPIO_PSEL + (idx*0x10))  = val;
   }
  //GPIO Data out  
  for (idx = 0; idx < 18; idx++) {
		val = gpio_init_dout_data[idx];
    	*(volatile u32 *)(GPIO_DOUT + (idx*0x10))  = val;
  }
	//GPIO mode
	for (idx = 0; idx < 56; idx++) {
		val = gpio_init_mode_data[idx];
		*(volatile u32 *)(GPIO_MODE_REG + (idx*0x10))  = val;
  } 
	
	
	
	
}
*/
int __init mediatek_gpio_init(void)
{
int result = 0;

printk("#################mediatek_gpio_init\n");

#ifdef  CONFIG_DEVFS_FS
	if (devfs_register_chrdev(mediatek_gpio_major, MEDIATEK_GPIO_DEVNAME,
				&mediatek_gpio_fops)) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return -EIO;
	}
	devfs_handle = devfs_register(NULL, mediatek_GPIO_DEVNAME,
			DEVFS_FL_DEFAULT, mediatek_gpio_major, 0,
			S_IFCHR | S_IRUGO | S_IWUGO, &mediatek_gpio_fops, NULL);
#else
	
	result = register_chrdev(mediatek_gpio_major, MEDIATEK_GPIO_DEVNAME,
			&mediatek_gpio_fops);
	if (result < 0) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return result;
	}
	if (mediatek_gpio_major == 0) {
		mediatek_gpio_major = result;
		printk(KERN_DEBUG NAME ": got dynamic major %d\n", result);
	}
#endif


// mt_gpio_default_setting(); //GPIO default setting. customer can modify it;
 
 
#ifdef CONFIG_MEDIATEK_GPIO_LED
	mediatek_gpio_led_init_timer();
#endif
	printk("mediatek gpio driver initialized\n");

	return 0;
}

void __exit mediatek_gpio_exit(void)
{
#ifdef  CONFIG_DEVFS_FS
	devfs_unregister_chrdev(mediatek_gpio_major, MEDIATEK_GPIO_DEVNAME);
	devfs_unregister(devfs_handle);
#else
	unregister_chrdev(mediatek_gpio_major, MEDIATEK_GPIO_DEVNAME);
#endif

#ifdef CONFIG_MEDIATEK_GPIO_LED
	del_timer(&mediatek_gpio_led_timer);
#endif
	printk("mediatek gpio driver exited\n");
}

/*
 * send a signal(SIGUSR1) to the registered user process whenever any gpio
 * interrupt comes
 * (called by interrupt handler)
 */
void mediatek_gpio_notify_user(int usr)
{
	struct task_struct *p = NULL;

	if (mediatek_gpio_irqnum < 0 || MEDIATEK_GPIO_NUMBER <= mediatek_gpio_irqnum) {
		printk(KERN_ERR NAME ": gpio irq number out of range\n");
		return;
	}

	//don't send any signal if pid is 0 or 1
	if ((int)mediatek_gpio_info[mediatek_gpio_irqnum].pid < 2){
		printk("don't send any signal if pid is 0 or 1 \n");
		return;
	}
		
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	p = find_task_by_vpid(mediatek_gpio_info[mediatek_gpio_irqnum].pid);
#else
	p = find_task_by_pid(mediatek_gpio_info[mediatek_gpio_irqnum].pid);
#endif

	if (NULL == p) {
		printk(KERN_ERR NAME ": no registered process to notify\n");
		return;
	}

	if (usr == 1) {
		printk(KERN_NOTICE NAME ": sending a SIGUSR1 to process %d\n",
				mediatek_gpio_info[mediatek_gpio_irqnum].pid);
		send_sig(SIGUSR1, p, 0);
	}
	else if (usr == 2) {
		printk(KERN_NOTICE NAME ": sending a SIGUSR2 to process %d\n",
				mediatek_gpio_info[mediatek_gpio_irqnum].pid);
		send_sig(SIGUSR2, p, 0);
	}
}





static void mediatek_gpio_irq_handler(int irq)
{
#if defined (CONFIG_ARCH_MT7623)
	struct gpio_time_record {
		unsigned long falling;
		unsigned long rising;
	};
	static struct gpio_time_record record[MEDIATEK_GPIO_NUMBER];
	unsigned long now;
	int i;
	now = jiffies;
  mt_eint_unmask(irq);
	mediatek_eint031_sta = le32_to_cpu(*(volatile u32 *)(EINT_031_STA));
	mediatek_eint3263_sta = le32_to_cpu(*(volatile u32 *)(EINT_3263_STA));
	mediatek_eint6495_sta = le32_to_cpu(*(volatile u32 *)(EINT_6495_STA));
	mediatek_eint96127_sta = le32_to_cpu(*(volatile u32 *)(EINT_96127_STA));	
	mediatek_eint128159_sta = le32_to_cpu(*(volatile u32 *)(EINT_128159_STA));
	
	
  mediatek_eint031_edge_get = le32_to_cpu(*(volatile u32 *)(EINT_031_POL_GET));
	mediatek_eint3263_edge_get = le32_to_cpu(*(volatile u32 *)(EINT_3263_POL_GET));
	mediatek_eint6495_edge_get = le32_to_cpu(*(volatile u32 *)(EINT_6495_POL_GET));
	mediatek_eint96127_edge_get = le32_to_cpu(*(volatile u32 *)(EINT_96127_POL_GET));
	mediatek_eint128159_edge_get = le32_to_cpu(*(volatile u32 *)(EINT_128159_POL_GET));
  mdelay(1);

	for (i = 0; i < 32; i++) {
		if (! (mediatek_eint031_sta & (1 << i)))
			continue;
			mediatek_gpio_irqnum = i;
		if (mediatek_eint031_edge_get & (1 << i)) { //falling edge
			//*(volatile u32 *)(EINT_031_POL_CLR) = (1 << i); //enable falling edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_NEG);
			if (record[i].rising != 0 && time_before_eq(now,
						record[i].rising + 40L)) {
				/*
				 * If the interrupt comes in a short period,
				 * it might be floating. We ignore it.
				 */
			}
			else {
				record[i].rising = now;
				if (time_before(now, record[i].falling + 200L)) {
					//one click
					printk("one click\n");
					mediatek_gpio_notify_user(1);
				}
				else {
					//press for several seconds
					printk("press for several seconds\n");
					mediatek_gpio_notify_user(2);
				}
			}
		}
		else { //falling edge
			 record[i].falling = now;
			// *(volatile u32 *)(EINT_031_POL_SET) = (1 << i); //enable rising edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_POS);
		}
		break;
	}
			
			
	for (i = 32; i < 64; i++) {
		if (! (mediatek_eint3263_sta & (1 << (i - 32))))
			continue;
		mediatek_gpio_irqnum = i;
		if (mediatek_eint3263_edge_get & (1 << (i - 32))) {
			//*(volatile u32 *)(EINT_031_POL_CLR) = (1 << (i-32)); //enable falling edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_NEG);
			if (record[i].rising != 0 && time_before_eq(now,
						record[i].rising + 40L)) {
			}
			else {
				record[i].rising = now;
				if (time_before(now, record[i].falling + 200L)) {
					mediatek_gpio_notify_user(1);
				}
				else {
					mediatek_gpio_notify_user(2);
				}
			}
		}
		else {
			record[i].falling = now;
			//*(volatile u32 *)(EINT_3263_POL_SET) = (1 << (i-32)); //enable rising edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_POS);
		}
		break;
	}
	
	for (i = 64; i < 96; i++) {
		if (! (mediatek_eint6495_sta & (1 << (i - 64))))
			continue;
		mediatek_gpio_irqnum = i;
		if (mediatek_eint6495_edge_get & (1 << (i - 64))) {
			//*(volatile u32 *)(EINT_6495_POL_CLR) = (1 << (i-64)); //enable falling edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_NEG);
			if (record[i].rising != 0 && time_before_eq(now,
						record[i].rising + 40L)) {
			}
			else {
				record[i].rising = now;
				if (time_before(now, record[i].falling + 200L)) {
					mediatek_gpio_notify_user(1);
				}
				else {
					mediatek_gpio_notify_user(2);
				}
			}
		}
		else {
			record[i].falling = now;
			//*(volatile u32 *)(EINT_6495_POL_SET) = (1 << (i-64)); //enable rising edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_POS);
		}
		break;
	}					
			
	for (i = 96; i < 128; i++) {
		if (! (mediatek_eint96127_sta & (1 << (i - 96))))
			continue;
		mediatek_gpio_irqnum = i;
		if (mediatek_eint96127_edge_get & (1 << (i - 96))) {
			//*(volatile u32 *)(EINT_96127_POL_CLR) = (1 << (i-96)); //enable falling edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_NEG);
			if (record[i].rising != 0 && time_before_eq(now,
						record[i].rising + 40L)) {
			}
			else {
				record[i].rising = now;
				if (time_before(now, record[i].falling + 200L)) {
					mediatek_gpio_notify_user(1);
				}
				else {
					mediatek_gpio_notify_user(2);
				}
			}
		}
		else {
			record[i].falling = now;
			//*(volatile u32 *)(EINT_96127_POL_SET) = (1 << (i-96)); //enable rising edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_POS);
		}
		break;
	}						
	for (i = 128; i < 160; i++) {
		if (! (mediatek_eint128159_sta & (1 << (i - 128))))
			continue;
		mediatek_gpio_irqnum = i;
		if (mediatek_eint128159_edge_get & (1 << (i - 128))) {  //0: falling 1: rising
			//*(volatile u32 *)(EINT_128159_POL_CLR) = (1 << (i-128)); //enable falling edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_NEG);
			if (record[i].rising != 0 && time_before_eq(now,
						record[i].rising + 40L)) {
			}
			else {
				record[i].rising = now;
				if (time_before(now, record[i].falling + 200L)) {
					mediatek_gpio_notify_user(1);
				}
				else {
					mediatek_gpio_notify_user(2);
				}
			}
		}
		else {
			record[i].falling = now;
			//*(volatile u32 *)(EINT_128159_POL_SET) = (1 << (i-128)); //enable rising edge
			mt_eint_set_polarity(mediatek_gpio_irqnum, MT_EINT_POL_POS);
		}
		break;
	}						
#endif			


}

module_init(mediatek_gpio_init);
module_exit(mediatek_gpio_exit);
//MODULE_DESCRIPTION("MEDIATEK SoC GPIO Controller API Module");
//MODULE_AUTHOR("Harry");
//MODULE_LICENSE("GPL");
//MODULE_VERSION(MOD_VERSION_GPIO);

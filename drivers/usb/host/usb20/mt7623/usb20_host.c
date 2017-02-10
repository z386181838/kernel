/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/xlog.h>
#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <linux/musb/musb_core.h>
#include <linux/platform_device.h>
#include <linux/musb/musbhsdma.h>
#include <cust_gpio_usage.h>
#include <linux/switch.h>
#include "usb20.h"



#ifdef CONFIG_USB_MTK_OTG
extern struct musb *mtk_musb;

static struct musb_fifo_cfg fifo_cfg_host[] = {
{ .hw_ep_num =  1, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  1, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  5, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	5, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  6, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	6, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_RX,   .maxpacket = 64,  .mode = MUSB_BUF_SINGLE},
};

u32 delay_time = 15;
module_param(delay_time,int,0644);
u32 delay_time1 = 55;
module_param(delay_time1,int,0644);

void mt_usb_set_vbus(struct musb *musb, int is_on)
{
	DBG(0,"mt65xx_usb20_vbus++,is_on=%d\r\n",is_on);
	if(is_on){		
	#if (defined(CONFIG_MTK_BQ24296_SUPPORT) && defined(P1_PROJECT)) 	
	mt_set_gpio_mode(GPIO8,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO8, GPIO_DIR_IN);
	tbl_charger_otg_vbus((work_busy(&musb->id_pin_work.work)<< 8)| 1);	
	DBG(0,"mt65xx_usb20_vbus++,P1_PROJECT  is_on=%d\r\n",is_on);
	#elif (defined(CONFIG_MTK_BQ24296_SUPPORT) && defined(P2_PROJECT))
	mt_set_gpio_mode(GPIO20,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO20, GPIO_DIR_IN);
	tbl_charger_otg_vbus((work_busy(&musb->id_pin_work.work)<< 8)| 1);	
	DBG(0,"mt65xx_usb20_vbus++,P2_PROJECT  is_on=%d\r\n",is_on);
	#else
	mt_set_gpio_mode(GPIO237,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO237, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO237,GPIO_OUT_ONE);	
	DBG(0,"mt65xx_usb20_vbus++,M1_PROJECT  is_on=%d\r\n",is_on);
	#endif		
	} else {	
	#if (defined(CONFIG_MTK_BQ24296_SUPPORT) && defined(P1_PROJECT))	
	mt_set_gpio_mode(GPIO8,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO8, GPIO_DIR_IN);
	tbl_charger_otg_vbus((work_busy(&musb->id_pin_work.work)<< 8)| 1);
	DBG(0,"mt65xx_usb20_vbus++,P1_PROJECT  is_on=%d\r\n",is_on);
	#elif (defined(CONFIG_MTK_BQ24296_SUPPORT) && defined(P2_PROJECT))
	mt_set_gpio_mode(GPIO20,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO20, GPIO_DIR_IN);	
	tbl_charger_otg_vbus((work_busy(&musb->id_pin_work.work)<< 8)| 0);	
	DBG(0,"mt65xx_usb20_vbus++,P2_PROJECT  is_on=%d\r\n",is_on);
	#else
	mt_set_gpio_mode(GPIO237,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO237, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO237,GPIO_OUT_ZERO);	
	DBG(0,"mt65xx_usb20_vbus++,M1_PROJECT  is_on=%d\r\n",is_on);
	#endif
	}
}

int mt_usb_get_vbus_status(struct musb *musb)
{
#if 1
	return true;
#else
	int	ret = 0;

	if ((musb_readb(musb->mregs, MUSB_DEVCTL)& MUSB_DEVCTL_VBUS) != MUSB_DEVCTL_VBUS) {
		ret = 1;
	} else {
		DBG(0, "VBUS error, devctl=%x, power=%d\n", musb_readb(musb->mregs,MUSB_DEVCTL), musb->power);
	}
    printk("vbus ready = %d \n", ret);
	return ret;
#endif
}

void mt_usb_init_drvvbus(void)
{
#if 0
#ifndef MTK_USB_NO_BATTERY    //because 8127 box have no charger IC
//#ifdef  CONFIG_MTK_SMART_BATTERY    //because 8127 box have no charger IC
#if !(defined(SWITCH_CHARGER) || defined(FPGA_PLATFORM))
	mt_set_gpio_mode(GPIO_OTG_DRVVBUS_PIN,GPIO_OTG_DRVVBUS_PIN_M_GPIO);//should set GPIO2 as gpio mode.
	mt_set_gpio_dir(GPIO_OTG_DRVVBUS_PIN,GPIO_DIR_OUT);
	mt_get_gpio_pull_enable(GPIO_OTG_DRVVBUS_PIN);
	mt_set_gpio_pull_select(GPIO_OTG_DRVVBUS_PIN,GPIO_PULL_UP);
#endif
//#endif
#endif
#endif
}

u32 sw_deboun_time = 400;
module_param(sw_deboun_time,int,0644);
struct switch_dev otg_state;
extern int ep_config_from_table_for_host(struct musb *musb);

static bool musb_is_host(void)
{
	u8 devctl = 0;
    int iddig_state = 1;
    bool usb_is_host = 0;

    DBG(0,"will mask PMIC charger detection\n");
#ifndef FPGA_PLATFORM
    pmic_chrdet_int_en(0);
#endif

    musb_platform_enable(mtk_musb);

#if 1//def ID_PIN_USE_EX_EINT
    iddig_state = mt_get_gpio_in(GPIO44);
	DBG(0,"iddig_state = %d\n", iddig_state);
#else
    iddig_state = 0 ;
    devctl = musb_readb(mtk_musb->mregs,MUSB_DEVCTL);
    DBG(0, "devctl = %x before end session\n", devctl);
    devctl &= ~MUSB_DEVCTL_SESSION;	// this will cause A-device change back to B-device after A-cable plug out
    musb_writeb(mtk_musb->mregs, MUSB_DEVCTL, devctl);
    msleep(delay_time);

    devctl = musb_readb(mtk_musb->mregs,MUSB_DEVCTL);
    DBG(0,"devctl = %x before set session\n",devctl);

    devctl |= MUSB_DEVCTL_SESSION;
    musb_writeb(mtk_musb->mregs,MUSB_DEVCTL,devctl);
    msleep(delay_time1);
    devctl = musb_readb(mtk_musb->mregs,MUSB_DEVCTL);
    DBG(0,"devclt = %x\n",devctl);
#endif
    if ( devctl & MUSB_DEVCTL_BDEVICE || iddig_state) {
        DBG(0,"will unmask PMIC charger detection\n");
#ifndef FPGA_PLATFORM
        pmic_chrdet_int_en(1);
#endif
        usb_is_host = false;
    } else {
        usb_is_host = true;
    }
	DBG(0,"usb_is_host = %d\n", usb_is_host);
	return usb_is_host;
}

void musb_session_restart(struct musb *musb)
{
	void __iomem	*mbase = musb->mregs;
	musb_writeb(mbase, MUSB_DEVCTL, (musb_readb(mbase, MUSB_DEVCTL) & (~MUSB_DEVCTL_SESSION)));
	DBG(0,"[MUSB] stopped session for VBUSERROR interrupt\n");
	USBPHY_SET8(0x6d, 0x3c);
	USBPHY_SET8(0x6c, 0x10);
	USBPHY_CLR8(0x6c, 0x2c);
	DBG(0,"[MUSB] force PHY to idle, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
	mdelay(5);
	USBPHY_CLR8(0x6d, 0x3c);
	USBPHY_CLR8(0x6c, 0x3c);
	DBG(0,"[MUSB] let PHY resample VBUS, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
	musb_writeb(mbase, MUSB_DEVCTL, (musb_readb(mbase, MUSB_DEVCTL) | MUSB_DEVCTL_SESSION));
	DBG(0,"[MUSB] restart session\n");
}

void switch_int_to_device(struct musb *musb)
{
#if 1//def ID_PIN_USE_EX_EINT
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_POS);
	mt_eint_unmask(IDDIG_EINT_PIN);
#else
	 musb_writel(musb->mregs,USB_L1INTP,0);
	 musb_writel(musb->mregs,USB_L1INTM,IDDIG_INT_STATUS|musb_readl(musb->mregs,USB_L1INTM));
#endif
	 DBG(0,"switch_int_to_device is done\n");
}

void switch_int_to_host(struct musb *musb)
{
#if 1//def ID_PIN_USE_EX_EINT
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_NEG);
	mt_eint_unmask(IDDIG_EINT_PIN);
#else
	musb_writel(musb->mregs,USB_L1INTP,IDDIG_INT_STATUS);
	musb_writel(musb->mregs,USB_L1INTM,IDDIG_INT_STATUS|musb_readl(musb->mregs,USB_L1INTM));
#endif
	DBG(0,"switch_int_to_host is done\n");
}

void switch_int_to_host_and_mask(struct musb *musb)
{
#if 1//def ID_PIN_USE_EX_EINT
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_NEG);
	mt_eint_mask(IDDIG_EINT_PIN);
#else
	musb_writel(musb->mregs,USB_L1INTM,(~IDDIG_INT_STATUS)&musb_readl(musb->mregs,USB_L1INTM)); //mask before change polarity
	mb();
	musb_writel(musb->mregs,USB_L1INTP,IDDIG_INT_STATUS);
#endif
	DBG(0,"swtich_int_to_host_and_mask is done\n");
}

static void musb_id_pin_work(struct work_struct *data)
{	
		#if 1// def ID_PIN_USE_EX_EINT
		u8 devctl = 0;
		#endif
		
		unsigned long flags;
		
		#ifdef P2_PROJECT
		musb_platform_set_vbus(mtk_musb, 1);
		msleep(100);
		DBG(0, "P2_PROJECT\n");
		#endif
		
		spin_lock_irqsave(&mtk_musb->lock, flags);
		musb_generic_disable(mtk_musb);
		spin_unlock_irqrestore(&mtk_musb->lock, flags);

		down(&mtk_musb->musb_lock);
		DBG(0, "work start, is_host=%d\n", mtk_musb->is_host);
		if(mtk_musb->in_ipo_off) {
			DBG(0, "do nothing due to in_ipo_off\n");
			goto out;
		}
		mtk_musb ->is_host = musb_is_host();
		DBG(0,"musb is as %s\n",mtk_musb->is_host?"host":"device");
		switch_set_state((struct switch_dev *)&otg_state, mtk_musb->is_host);
	
		if (mtk_musb->is_host) {
		#ifdef MTK_USB_NO_BATTERY  
				 musb_stop(mtk_musb);
		#endif
			
			//setup fifo for host mode
			ep_config_from_table_for_host(mtk_musb);
			wake_lock(&mtk_musb->usb_lock);
			
			#ifndef P2_PROJECT
			musb_platform_set_vbus(mtk_musb, 1);
			DBG(0, "not P2_PROJECT\n");
			#endif
			
			/* for no VBUS sensing IP*/        
			/* wait VBUS ready */
			msleep(100);		
			musb_start(mtk_musb);
			MUSB_HST_MODE(mtk_musb);			
		#if 1 //def ID_PIN_USE_EX_EINT
			/* clear session*/
			devctl = musb_readb(mtk_musb->mregs,MUSB_DEVCTL);
			musb_writeb(mtk_musb->mregs, MUSB_DEVCTL, (devctl&(~MUSB_DEVCTL_SESSION)));
			/* USB MAC OFF*/
			/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X */
			USBPHY_SET8(0x6c, 0x10);
			USBPHY_CLR8(0x6c, 0x2e);
			USBPHY_SET8(0x6d, 0x3e);
			DBG(0,"force PHY to idle, 0x6d=%x, 0x6c=%x\n",USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
			/* wait */
			msleep(5);
			/* restart session */
			devctl = musb_readb(mtk_musb->mregs,MUSB_DEVCTL);
			musb_writeb(mtk_musb->mregs, MUSB_DEVCTL, (devctl| MUSB_DEVCTL_SESSION));
			/* USB MAC ONand Host Mode*/
			/* VBUSVALID=1, AVALID=1, BVALID=1, SESSEND=0, IDDIG=0 */
			USBPHY_CLR8(0x6c, 0x10);
			USBPHY_SET8(0x6c, 0x2c);
			USBPHY_SET8(0x6d, 0x3e);
			DBG(0,"force PHY to host mode, 0x6d=%x, 0x6c=%x\n",USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
        #endif
			//musb_start(mtk_musb);
			//MUSB_HST_MODE(mtk_musb);
			switch_int_to_device(mtk_musb);
		} else {
			DBG(0,"devctl is %x\n",musb_readb(mtk_musb->mregs,MUSB_DEVCTL));
			musb_writeb(mtk_musb->mregs,MUSB_DEVCTL,0);
			if (wake_lock_active(&mtk_musb->usb_lock))
				wake_unlock(&mtk_musb->usb_lock);
			musb_platform_set_vbus(mtk_musb, 0);
			/* for no VBUS sensing IP */
        #if 1//def ID_PIN_USE_EX_EINT
			/* USB MAC OFF*/
			/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X */
			USBPHY_SET8(0x6c, 0x10);
			USBPHY_CLR8(0x6c, 0x2e);
			USBPHY_SET8(0x6d, 0x3e);
			DBG(0,"force PHY to idle, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
        #endif
			musb_stop(mtk_musb);
			//ALPS00849138
			mtk_musb->xceiv->state =  OTG_STATE_B_IDLE;
			MUSB_DEV_MODE(mtk_musb);
			switch_int_to_host(mtk_musb);
			#ifdef MTK_USB_NO_BATTERY
			musb_start(mtk_musb);
			#endif
		}
	out:
		DBG(0, "work end, is_host=%d\n", mtk_musb->is_host);
		up(&mtk_musb->musb_lock);
	

}


#if 1//def ID_PIN_USE_EX_EINT

static void mt_usb_ext_iddig_int(void)
{
    if (!mtk_musb->is_ready) {
        /* dealy 5 sec if usb function is not ready */
        schedule_delayed_work(&mtk_musb->id_pin_work,5000*HZ/1000);
    } else {
        schedule_delayed_work(&mtk_musb->id_pin_work,sw_deboun_time*HZ/1000);
    }
	DBG(0,"id pin interrupt assert\n");
}
#endif

void mt_usb_iddig_int(struct musb *musb)
{
    u32 usb_l1_ploy = musb_readl(musb->mregs,USB_L1INTP);  
    DBG(0,"id pin interrupt assert,polarity=0x%x\n",usb_l1_ploy);
    if (usb_l1_ploy & IDDIG_INT_STATUS) {
        usb_l1_ploy &= (~IDDIG_INT_STATUS);
    } else {
        usb_l1_ploy |= IDDIG_INT_STATUS;
    }

    musb_writel(musb->mregs,USB_L1INTP,usb_l1_ploy);
    musb_writel(musb->mregs,USB_L1INTM,(~IDDIG_INT_STATUS)&musb_readl(musb->mregs,USB_L1INTM));

    if (!mtk_musb->is_ready) {
        /* dealy 5 sec if usb function is not ready */		
        schedule_delayed_work(&mtk_musb->id_pin_work,5000*HZ/1000);
		
    } else {    	
        schedule_delayed_work(&mtk_musb->id_pin_work,sw_deboun_time*HZ/1000);
	}    
}

void static otg_int_init(void)
{
#if 1//def ID_PIN_USE_EX_EINT
	mt_set_gpio_mode(GPIO44, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO44, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO44, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO44, GPIO_PULL_UP);
    mt_eint_set_sens(26, MT_LEVEL_SENSITIVE);
    mt_eint_set_hw_debounce(26,64);
    mt_eint_registration(26, EINTF_TRIGGER_LOW, mt_usb_ext_iddig_int, FALSE);
#else
	u32 phy_id_pull = 0;
	mt_set_gpio_mode(GPIO236, GPIO_MODE_02);
   	mt_set_gpio_dir(GPIO236, GPIO_DIR_IN);
   	mt_set_gpio_pull_enable(GPIO236, GPIO_PULL_ENABLE);
   	mt_set_gpio_pull_select(GPIO236, GPIO_PULL_UP);

	phy_id_pull = __raw_readl((void __iomem *)U2PHYDTM1);
	phy_id_pull |= ID_PULL_UP;
	__raw_writel(phy_id_pull,(void __iomem *)U2PHYDTM1);

	musb_writel(mtk_musb->mregs,USB_L1INTM,IDDIG_INT_STATUS|musb_readl(mtk_musb->mregs,USB_L1INTM));	
#endif	
}


void mt_usb_otg_init(struct musb *musb)
{
    /*init drrvbus*/
	mt_usb_init_drvvbus();
	DBG(0, "mt_usb_otg_init\n");

	/* init idpin interrupt */
    otg_int_init();

	/* EP table */
    musb->fifo_cfg_host = fifo_cfg_host;
    musb->fifo_cfg_host_size = ARRAY_SIZE(fifo_cfg_host);

    otg_state.name = "otg_state";
	otg_state.index = 0;
	otg_state.state = 0;

	if(switch_dev_register(&otg_state))
		printk("switch_dev_register fail\n");
	else
        printk("switch_dev register success\n");			
	INIT_DELAYED_WORK(&musb->id_pin_work, musb_id_pin_work);	
}
#else

/* for not define CONFIG_USB_MTK_OTG */
void mt_usb_otg_init(struct musb *musb){}
void mt_usb_init_drvvbus(void){}
void mt_usb_set_vbus(struct musb *musb, int is_on){}
int mt_usb_get_vbus_status(struct musb *musb){return 1;}
void mt_usb_iddig_int(struct musb *musb){}
void switch_int_to_device(struct musb *musb){}
void switch_int_to_host(struct musb *musb){}
void switch_int_to_host_and_mask(struct musb *musb){}
void musb_session_restart(struct musb *musb){}

#endif

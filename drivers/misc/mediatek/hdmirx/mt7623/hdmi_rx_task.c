
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include "hdmi_rx_task.h"
#include "hdmirx.h"
#include "typedef.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_rx_dvi.h"
#include "edid_data.h"
#include "mach/irqs.h"
#include "hdmi_rx_hw.h"
#include "vin_drv_if.h"
#include "hdmihdcp.h"

BOOL _fgDisableHdmiRx = TRUE;
BYTE _bRse640x480PEnable = FALSE;//0;disable  1:CEA VGA  2:VESA VGA

static int hdmirx_main_kthread(void *data);
static int hdmirx_irq_kthread(void *data);
void hdmirx_task_handle(void);
int hdmi_rx_tmr_isr(void);
static void vHdmiRxDetectService(void);
void hdmirx_tmr_isr(unsigned long n);
void vSendHdmiRxCmd(unsigned char u1icmd);
static void vSetHdmiRxSrvTimeOut(unsigned long i4_count);
void hdmirx_reg_dump(void);

BOOL _fgHDMI_Rx_wait_finish = FALSE;


#define MAX_HDMI_RX_TMR_NUMBER  4

#define HDMI_UNUSED_RX_TIMER    (-1)
#define HDMI_RX_CTRL_TMR_INX  0
#define HDCP_RX_CTRL_TMR_INX  1
#define HDMI_RX_DELAY_TMR_INX  2
#define HDMI_RX_SRV_TMR_INX  3


static uint32_t HDMI_RX_TMR_ISR_TICKS = 20;
static struct timer_list _hHdmiRxTimer;

static volatile unsigned long _HdmiRxTmrValue[MAX_HDMI_RX_TMR_NUMBER];

unsigned long _rHdmiRxTmrStart[MAX_HDMI_RX_TMR_NUMBER];
unsigned long _rHdmiRxTmrStop[MAX_HDMI_RX_TMR_NUMBER];
unsigned long _rHdmiRxTmrDelta[MAX_HDMI_RX_TMR_NUMBER];

static struct task_struct *hdmirx_main_task = NULL;
wait_queue_head_t hdmirx_main_wq;
atomic_t hdmirx_main_event = ATOMIC_INIT(0);

static struct task_struct *hdmirx_irq_task = NULL;
wait_queue_head_t hdmirx_irq_wq;  
atomic_t hdmirx_irq_event = ATOMIC_INIT(0);

size_t hdmirx_Cmd = 0xff;

unsigned char _u1RxSysState = RX_IDLE_STATE;//RX_CHANGE_EDID_STATE;
extern unsigned char _u1TxEdidReady ;
extern unsigned char _u1TxEdidReadyOld;

UINT8 _u1TxEdidReady = HDMI_PLUG_OUT;

size_t hdmirx_drvinit = FALSE;
size_t hdmirxdrv_log_on = 1;
size_t hdmirx_powerenable = 0;

void vSendHdmiRxCmd(unsigned char u1icmd)
{
	hdmirx_Cmd = u1icmd;
}

void vClearHdmiRxCmd(void)
{
	hdmirx_Cmd = 0xff;
}

static void vSetHdmiRxSrvTimeOut(unsigned long i4_count)
{
	_HdmiRxTmrValue[HDMI_RX_SRV_TMR_INX] = i4_count;
	_rHdmiRxTmrStart[HDMI_RX_SRV_TMR_INX]= jiffies;
}

void vChageRxSysState(unsigned char u1State)
{
    _u1RxSysState = u1State;
}

void hdmi_rx_suspend(void)
{
}

void hdmi_rx_resume(void)
{
}

void hdmi_rx_repeatermode_enter(void)
{
	vHDMIInterRxInit();
}

void hdmi_rx_repeatermode_exit(void)
{
	//hdmi_rx_uninit();
}

void vSet640x480PEnable(BYTE bType)
{
	_bRse640x480PEnable = bType;
}	

void vEnableHdmiRxTask(unsigned char u1Enable)
{
    if(u1Enable)
    {
        if(_fgDisableHdmiRx == TRUE)//temply add
        {
            _fgDisableHdmiRx = FALSE;
        }
    }
    else
    {
        if(_fgDisableHdmiRx == FALSE)//temply add
        {
            _fgDisableHdmiRx = TRUE;
            vSetHdmiRxSrvTimeOut(HDMI_RX_TIMER_5MS);
            vChageRxSysState(RX_DETECT_STATE);
        }

    }

}

void hdmirx_tmr_isr(unsigned long n)
{
	unsigned long i;
	unsigned long DeltaMs;

	for (i=0; i<MAX_HDMI_RX_TMR_NUMBER; i++)
	{
		if (_HdmiRxTmrValue[i] > 0 )
		{
			_rHdmiRxTmrStop[i]= jiffies;
			_rHdmiRxTmrDelta[i] = _rHdmiRxTmrStart[i]+_HdmiRxTmrValue[i]*HZ/1000;
			if(time_after(_rHdmiRxTmrStop[i], _rHdmiRxTmrDelta[i]))
			{
				_HdmiRxTmrValue[i] = 0; 
			} 
			
			if((i==HDMI_RX_SRV_TMR_INX)&&( _HdmiRxTmrValue[HDMI_RX_SRV_TMR_INX] ==0 ))
			{
				vSendHdmiRxCmd(HDMI_RX_SERVICE_CMD);
			}
		}
	}  
	
		atomic_set(&hdmirx_main_event, 1);
        wake_up_interruptible(&hdmirx_main_wq);
		mod_timer(&_hHdmiRxTimer, jiffies + HDMI_RX_TMR_ISR_TICKS/(1000/HZ));
}

static void HDMI_RX_TmrReset(void)
{
    INT32 i;
    for (i=0; i<MAX_HDMI_RX_TMR_NUMBER; i++) 
	{
        _HdmiRxTmrValue[i] = HDMI_UNUSED_RX_TIMER;
    }
}

int hdmi_rx_tmr_isr(void)
{
    memset((void*)&_hHdmiRxTimer, 0, sizeof(_hHdmiRxTimer));
	HDMI_RX_TmrReset();
    _hHdmiRxTimer.expires  = jiffies + 1000/(1000/HZ);   // wait 1s to stable
    _hHdmiRxTimer.function = hdmirx_tmr_isr;     
    _hHdmiRxTimer.data     = 0;
    init_timer(&_hHdmiRxTimer);
    add_timer(&_hHdmiRxTimer);
	
    return 0;
}

extern UINT8 _bRxHDCPMode ;

static void vHdmiRxDetectService(void)
{
    if(_fgDisableHdmiRx)
        return;
    if(_bRxHDCPMode==5)
		return;
    vHDMIRxHpdLoop();
	
	if(fgIsHdmiRepeater())    
	{
		if (!fgCheckRxDetectDone())
		{
			vDviModeDetect();
		}
		else
		{
			vDviChkModeChange();
		}
	}

}

void hdmirx_task_handle(void)
{
    UINT8 i = 0;
	_u1RxSysState = RX_DETECT_STATE;
	hdmirx_drvinit = TRUE;
	hdmirx_Cmd = HDMI_RX_SERVICE_CMD;
	//printk("[HDMIRX-1]%s,%d\n", __func__,__LINE__);
	
	vEnableHdmiRxTask(TRUE);
    while(hdmirx_drvinit == TRUE)
    {
		wait_event_interruptible(hdmirx_main_wq, atomic_read(&hdmirx_main_event));
		atomic_set(&hdmirx_main_event,0);
		
		if(hdmirx_Cmd == HDMI_RX_SERVICE_CMD)
		{
	        switch(_u1RxSysState)
	        {
	        case RX_CHANGE_EDID_STATE:
	            break;

	        case RX_DETECT_STATE:
	            vHdmiRxDetectService();
				vHDMIRxHdcpService();					
	            vSetHdmiRxSrvTimeOut(HDMI_RX_TIMER_20MS);
	            break;

	        case RX_IDLE_STATE:
	            //HDMIHPDHigh(0);
	            break;


	        default:
	            break;
	        }
		
		}
		else if(hdmirx_Cmd == HDMI_DISABLE_HDMI_RX_TASK_CMD)
		{
			hdmirx_drvinit = 0;
		}
    }
	_fgHDMI_Rx_wait_finish = TRUE;
}

static int hdmirx_main_kthread(void *data)
{
	struct sched_param param = { .sched_priority = RTPM_PRIO_CAMERA_PREVIEW };
	sched_setscheduler(current, SCHED_RR, &param);
	
    printk("[HDMIRX-1]%s\n", __func__);
	
    if (request_irq(MT_HDMIRX_IRQ_ID,hdmirx_irq_handle, IRQF_TRIGGER_LOW, "mtkhdmirx", NULL) < 0)
    {
		printk("[HDMIRX-1]%s,%d\n", __func__,__LINE__);
        RETNULL(1);
    }
	for( ;; ) 
	{
		hdmirx_task_handle();
		if (kthread_should_stop())
			break;
	}

	return 0;
}
extern bool bDQ;
unsigned int  vinproccnt = 0;

static struct task_struct *VinTask = NULL;
static int VinProc(void *data)
{
	VIDEO_IN_BUFFER_INFO videoInBuffer;
	struct sched_param param = { .sched_priority = RTPM_PRIO_CAMERA_PREVIEW };
	sched_setscheduler(current, SCHED_RR, &param);
	for( ;; ) 
	{
	    vinproccnt ++;
		if (bDQ) {
			memset(&videoInBuffer, 0, sizeof(VIDEO_IN_BUFFER_INFO));
			
			if (vDQBuf(&videoInBuffer) == 0) {
				if (videoInBuffer.u4BufIndex != -1) {
					vQBuf(videoInBuffer.u4BufIndex);
				}
			}
			if (kthread_should_stop())
				break;
		}
		msleep(20);
	}

	return 0;
}

static int hdmirx_irq_kthread(void *data)
{
	struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
	sched_setscheduler(current, SCHED_RR, &param);
	
	for( ;; ) 
	{
		wait_event_interruptible(hdmirx_irq_wq, atomic_read(&hdmirx_irq_event));
		atomic_set(&hdmirx_irq_event,0);
		hdmirx_irq_handle();
		if (kthread_should_stop())
			break;
	}

	return 0;
}

int hdmi_rx_internal_init(void)
{		 
	
    printk("[HDMIRX-1]%s\n", __func__);
	vHDMIInterRxInit();
	
    init_waitqueue_head(&hdmirx_main_wq);
    hdmirx_main_task = kthread_create(hdmirx_main_kthread, NULL, "hdmirx_timer_kthread"); 
    wake_up_process(hdmirx_main_task);

    if (VinTask == NULL) {
			VinTask = kthread_create(VinProc, NULL, "VinTask");
			if (VinTask == NULL) {
				printk("VinBufQueue, failed to create VinTask\n");
				return;
			} 
    		wake_up_process(VinTask);
		} 
    //init_waitqueue_head(&hdmirx_irq_wq);
    //hdmirx_irq_task = kthread_create(hdmirx_irq_kthread, NULL, "hdmirx_irq_kthread"); 
    //wake_up_process(hdmirx_irq_task);
	
    return 0;
	
}

signed int hdmi_rx_uninit(void)
{
	if(hdmirx_drvinit)
	{
		vSendHdmiRxCmd(HDMI_DISABLE_HDMI_RX_TASK_CMD);

		while (!_fgHDMI_Rx_wait_finish)
			msleep(5);
		printk("[HDMI RX] HDMI_RX_DRVUnInit  Done \n");		
	}

	return 1;
}

int hdmirx_internal_power_on(void)
{
	if(hdmirx_powerenable==1) return 0;
	hdmirx_powerenable = 1;
	
    HDMIRX_DIG_PowerOn();
	vHDMIInterRxInit();
	
    memset((void*)&_hHdmiRxTimer, 0, sizeof(_hHdmiRxTimer));
	HDMI_RX_TmrReset();
    _hHdmiRxTimer.expires  = jiffies + 1000/(1000/HZ);   // wait 1s to stable
    _hHdmiRxTimer.function = hdmirx_tmr_isr;     
    _hHdmiRxTimer.data     = 0;
    init_timer(&_hHdmiRxTimer);
    add_timer(&_hHdmiRxTimer);
	
    return 0;

}

void hdmirx_internal_power_off(void)
{
	if(hdmirx_powerenable==0) return 0;
	hdmirx_powerenable = 0;
	
	HDMIRX_DIG_PowerOff();
	hdmirxanapoweroff();
    
}

void hdmirx_reg_dump(void)
{
    //hdmirx_drv->dump();
    
    return;
}

void hdmirx_read(unsigned int u2Reg, unsigned int *p4Data)
{
    printk("Reg read= 0x%08x, data = 0x%08x\n", u2Reg, *p4Data);
}

void hdmirx_write(unsigned int u2Reg, unsigned int u4Data)
{
    printk("Reg write= 0x%08x, data = 0x%08x\n", u2Reg, u4Data);
}

void vUpdateHdmiRxEdid(UINT8 u1UpdateType)
{
#if 1//(DRV_SUPPORT_HDMI_RX) 
	vHDMIHPDLow(1);
	if(u1UpdateType == 0)
	{
		Default_Edid_BL0_BL1_Write();  	
	}
	else if(u1UpdateType == 1)
	{
		EdidProcessing();	
	}		
	vHDMIHPDHigh(1);
#endif  
}


void vDumpHdmiRxEdid(UINT8 u1Edid)
{
	vReadRxEDID(u1Edid);
}

void vTxSetRxReceiverMode(void)
{
	printk("[HDMI RX]vTxSetRxReceiverMode  \n");
	RxHDCPSetReceiver();	
}	



void vTxSetRxRepeaterMode(void)
{
	printk("[HDMI RX]vTxSetRxRepeaterMode \n");
	RxHDCPSetRepeater();	
}	

void vIssueHdmiRxUpdateEdidCmd(UINT8 u1EdidReady)
{
	printk("[HDMI RX]!!_u1TxEdidReady = %d, u1EdidReady = %d\n",_u1TxEdidReady, u1EdidReady);
	_u1TxEdidReady = u1EdidReady;
}



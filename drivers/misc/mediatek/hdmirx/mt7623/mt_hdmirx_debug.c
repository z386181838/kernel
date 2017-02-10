#if 1//defined(MTK_HDMI_SUPPORT)
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>

#include <mach/mt_typedefs.h>
#include <linux/vmalloc.h>
#include <mach/m4u.h>
#include <linux/dma-mapping.h>
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
#include "hdmi_rx_ctrl.h"
#include "video_in_if.h"
#include "dgi_if.h"
#include "vin_main.h"
#include "vin_hal.h"
#include "vin_drv_if.h"
#include "hdmi_rx_dvi.h"
#include "edid_data.h"


void DBG_Init(void);
void DBG_Deinit(void);
extern void hdmirx_log_enable(unsigned short enable);

// ---------------------------------------------------------------------------
//  External variable declarations
// ---------------------------------------------------------------------------

//extern LCM_DRIVER *lcm_drv;
// ---------------------------------------------------------------------------
//  Debug Options
// ---------------------------------------------------------------------------


static char STR_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > hdmirx\n"
    "\n"
    "ACTION\n"
    "        hdmirx:[on|off]\n"
    "             enable hdmirx video output\n"
    "\n";

// TODO: this is a temp debug solution

#define HDMIRX_PRINTF(fmt, arg...)  \
			do { \
				 temp_len = sprintf(buf,fmt,##arg);  \
				 buf += temp_len; \
				 len += temp_len; \
			}while (0)

#define REGISTER_WRITE32(u4Addr, u4Val)     (*((volatile unsigned int*)(u4Addr)) = (u4Val))
#define REGISTER_READ32(u4Addr)             (*((volatile unsigned int*)(u4Addr)))

static UINT32 reg_start, lenth, reg_index;

static ssize_t hdmirx_core_show_info(struct device *dev, 
	                                         struct device_attribute *attr, char *buf)
{
	struct attribute *pattr = &(attr->attr);

	UINT32 len = 0; 
	UINT32 temp_len = 0; 	

	HDMIRX_PRINTF("[hdmirx]%s,%d \n", __func__, __LINE__);
	HDMIRX_PRINTF("[hdmirx] %s \n",pattr->name);
	printk("read start address:0x%08x lenth:0x%x  %x\n",reg_start,lenth,reg_start+lenth);
	for (reg_index = reg_start; reg_index <(reg_start+lenth);)
	{
		printk("read:0x%08x = 0x%08x\n",reg_index,REGISTER_READ32(reg_index));
		HDMIRX_PRINTF("read:0x%08x = 0x%08x\n",reg_index,REGISTER_READ32(reg_index));
		reg_index += 0x4;
		if(reg_index >(reg_start+lenth))
			break;
	}
    return len;
}
			
static ssize_t hdmirx_core_store_info(struct device *dev, 
                                            struct device_attribute *attr,
		                                    const char *buf, size_t count)
{
    int val;
	UINT32 reg;
	UINT32 temp_len = 0; 
	UINT32 len = 0;
	
	struct attribute *pattr = &(attr->attr);	
	printk("input cmd %s \n",pattr->name);
    if (strcmp(pattr->name, "read") == 0)
	{  
		sscanf(buf, "%x %x", &reg_start , &lenth);
		return count;
	}
	if (strcmp(pattr->name, "write") == 0)
	{  
		sscanf(buf, "%x %x", &reg , &val);
		printk("write reg(0x%08x) =  val(0x%08x)\n", reg, val);
		REGISTER_WRITE32(reg, val);
		printk("read:0x%08x = 0x%08x\n",reg,REGISTER_READ32(reg));
		return count;
	}
  return count;
}


static DEVICE_ATTR(read, 0664, hdmirx_core_show_info, hdmirx_core_store_info);

static DEVICE_ATTR(write, 0664, hdmirx_core_show_info, hdmirx_core_store_info);

static struct device_attribute *hdmirx_attr_list[] = {
	&dev_attr_read,
	&dev_attr_write,
};

int mt_hdmirx_create_attr(struct device *dev) 
{
    int idx, err = 0;
    int num = (int)(sizeof(hdmirx_attr_list)/sizeof(hdmirx_attr_list[0]));
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) {
        if ((err = device_create_file(dev, hdmirx_attr_list[idx])))
            break;
    }
    
    return err;
}
/*---------------------------------------------------------------------------*/
int mt_hdmirx_delete_attr(struct device *dev)
{
    int idx ,err = 0;
    int num = (int)(sizeof(hdmirx_attr_list)/sizeof(hdmirx_attr_list[0]));
    
    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) 
        device_remove_file(dev, hdmirx_attr_list[idx]);

    return err;
}

bool bDQ = false;
static void process_dbg_opt(const char *opt)
{
    DGI_VIDEO_IN_INFO_T rInput;
    INPUT_DEVICE_INFO_T rVdoInInfo;
	HDMI_RX_STATUS_MODE rHDMIInmode;
    u32 reg;
    unsigned int val;
    unsigned int vadr_regstart;
    unsigned int vadr_regend;
    unsigned int eDgiFMTMode;
    unsigned int eDgiInputMode;
    unsigned int res;
	unsigned int debugmode;
	unsigned char eSrcType;
	unsigned char eVideoinDramFmt;
	unsigned char eVideoinPutFmt;
	UINT32 temp_len = 0; 
	UINT32 len = 0;
	char *buf;
	buf = opt;
	VIDEO_IN_REQBUF requestBuffer;
	VIDEO_IN_BUFFER_INFO videoInBuffer;
	int i, temp_va = 0;
	void* obj[4];
    static int internal_buffer_init = 0;
    M4U_PORT_STRUCT portStruct;
    int tmpBufferSize;
	int videoinbuffersize;
	
    printk("[HDMIRX]%s\n", __func__);
    if (0 == strncmp(buf, "phyinit", 7))
    {
        _HdmiRxPHYInit();
    }
    else if (0 == strncmp(buf, "gtmingnoreset", 13))
    {
        _GetTimingInfomationNoreset();
    }
	else if(0 == strncmp(buf, "hdmirxtaskinit", 14))
	{
		hdmi_rx_internal_init();  
	}
	else if (0 == strncmp(buf, "hdmirxtmrisr", 12))
	{
		hdmi_rx_tmr_isr();
	}
	else if (0 == strncmp(buf, "setrepeatermode:", 16))
	{
		sscanf(buf+16, "%d", &rHDMIInmode);
		printk("rHDMIInmode = %d\n",rHDMIInmode);
		//HDMIRX_PRINTF("rHDMIInmode = %d\n",rHDMIInmode);
		//MMSYS_BDP_POWER_ON();
		vHdmiRepeaterMode(rHDMIInmode);
	}
    else if (0 == strncmp(buf, "log:", 4))
    {
        sscanf(buf+4, "0x%x", &debugmode);
        hdmirx_log_enable(debugmode);
    }
    else if (0 == strncmp(buf, "hdmirxcrc", 9))
    {
	    _HdmiRxCrcCheck(50);
    }
    else if (0 == strncmp(buf, "bypassmode", 10))
    {
		//sscanf(buf+11, "%d/%d", &eDgiFMTMode,&eDgiInputMode);
		rInput.ePinType = YC444_36BIT;
		rInput.eInputMode = FMT_601_MODE;
		res = u4GetHdmiRxRes();
		printk("BYPASS MODE FMT = %d,eInputMode = %d,res = %d\n",eDgiFMTMode,eDgiInputMode,res);
		vset_dgi_in_mode(rInput,res);
    }
    else if (0 == strncmp(buf, "drammode:", 9))
    {
		//sscanf(buf+9, "%d/%d/%d/%d",&eSrcType,&res,&eVideoinDramFmt,&eVideoinPutFmt);
		
		sscanf(buf+9, "%d",&res);
		rVdoInInfo.eInputMode = FMT_601_MODE;
		rVdoInInfo.ePinType = YC444_36BIT;
		rVdoInInfo.eInputRes = (HDMI_RESOLUTION_MODE_T)res;
		rVdoInInfo.rVdoInWDramType.eVdoInAddrMode = VIN_LINEAR;
		rVdoInInfo.rVdoInWDramType.eVdoInSwapMode = VIN_SWAP_MODE_0;
		rVdoInInfo.rVdoInWDramType.eVdoInDramFmt = VIN_422;
		rVdoInInfo.eVdoInAR = SRC_ASP_16_9_FULL;
		rVdoInInfo.fgVgaIsCeaType = FALSE;
		rVdoInInfo.eDeviceId = VIN_HDMI_1;
		
		vin_hal_set_src_type(rVdoInInfo.eDeviceId); //internal Rx
			
		memset(&videoInBuffer, 0, sizeof(VIDEO_IN_BUFFER_INFO));
        
        if(fgVinSetCfgInfo(rVdoInInfo))
        {
        #if 0
			tmpBufferSize = 1920 * 1080 * 2;
			
			if (0 == internal_buffer_init)
			{
				internal_buffer_init = 1;
			
				printk("indirect link alloc internal buffer\n");
				temp_va = (unsigned int)vmalloc(tmpBufferSize);
				memset(temp_va, 0xfe, tmpBufferSize);
				
				printk("VinBufQueue, vmalloc memory, temp_va=0x%x\n",temp_va);
				if (((void*)temp_va) == NULL)
				{
					printk("vmalloc 0x%x bytes fail\n", tmpBufferSize);
					return;
				}
				if (m4u_alloc_mva(BDP_WR_CHANNEL_VDI,
							temp_va,
							tmpBufferSize,
							0,
							0,
							&videoInBuffer.wch1mva))
				{
					printk("m4u_alloc_mva for videoInBuffer.wch1mva fail\n");
					return;
				}
				m4u_dma_cache_maint(BDP_WR_CHANNEL_VDI,
						(void const *)temp_va,
						tmpBufferSize,
						DMA_BIDIRECTIONAL);
				
				videoInBuffer.vAddrInfo.u4YAddr = temp_va;
				videoInBuffer.vAddrInfo.u4CAddr = temp_va + 1920 * 1080;
				videoInBuffer.vAddrInfo.u4CBufferSize = 1920*1080/2;
				videoInBuffer.vAddrInfo.u4YBufferSize = 1920*1080;
				
				videoInBuffer.vAddrInfo_va.u4YAddr = videoInBuffer.wch1mva;
				videoInBuffer.vAddrInfo_va.u4CAddr = videoInBuffer.wch1mva + 1920 * 1080;
				videoInBuffer.vAddrInfo_va.u4CBufferSize = 1920*1080;
				videoInBuffer.vAddrInfo_va.u4YBufferSize = 1920*1080;
				
				printk("videoInBuffer.vAddrInfo_va.u4YAddr = 0x%x,videoInBuffer.vAddrInfo_va.u4CAddr = 0x%x\n",\
					   videoInBuffer.vAddrInfo_va.u4YAddr,videoInBuffer.vAddrInfo_va.u4CAddr);
				printk("videoInBuffer.vAddrInfo.u4YAddr = 0x%x,videoInBuffer.vAddrInfo.u4CAddr = 0x%x\n",\
					   videoInBuffer.vAddrInfo.u4YAddr,videoInBuffer.vAddrInfo.u4CAddr);
				
			}	 
			printk("config m4u start\n\n");
			vHal_VinSetBufPtr(&videoInBuffer.vAddrInfo_va);
			#endif
			vVinSetPBType(VIN_PB_TYPE_START_CMD);
		}
    }
    else if (strncmp(buf, "w:",2) == 0)
	{
		sscanf(buf+2, "%x=%x", &reg , &val);
		printk("w:0x%08x=0x%08x\n", reg, val);
		HDMIRX_PRINTF("w:0x%08x=0x%08x\n", reg, val);
        *(volatile unsigned int*)(reg) = val;	
	}
    else if (strncmp(buf, "r:",2) == 0)
	{
		sscanf(buf+2, "%x/%x", &vadr_regstart , &vadr_regend);
		vadr_regend  &= 0x3ff;
		printk("r:0x%08x/0x%08x\n", vadr_regstart, vadr_regend);
		vadr_regend = vadr_regstart + vadr_regend;
		while (vadr_regstart <= vadr_regend)
		{
			 printk("0x%08x = 0x%08x\n",vadr_regstart,*(volatile unsigned int*)(vadr_regstart));
			 HDMIRX_PRINTF("0x%08x = 0x%08x\n",vadr_regstart,*(volatile unsigned int*)(vadr_regstart));
			 vadr_regstart += 4;
		}
	}
	else if(strncmp(buf, "rgcstt",6) == 0)
	{
		vShowHDMIRxStatus();
	}
	else if(strncmp(buf, "loadedid",8) == 0)
	{
		vLoadEdidFromCODE();
	}
	else if(strncmp(buf, "rgifrm",6) == 0)
	{
		vShowHDMIRxInfo();
	}
	else if(strncmp(buf, "rgbstt",6) == 0)
	{
		vShowRxHDCPBstatus();
	}
	else if(strncmp(buf, "rgpa",4) == 0)
	{
		vShowEdidPhyAddress();
	}
	else if(strncmp(buf, "showvinstatus",13) == 0)
	{
		vShowvideoinstatus();
	}
	else if (strncmp(buf, "requestbuf:", 11) == 0) {
		memset(&requestBuffer, 0, sizeof(VIDEO_IN_REQBUF));
		sscanf(buf+11, "%d", &requestBuffer.u4BufCount);
		printk("vadr_regstart: %d\n", requestBuffer.u4BufCount);
		vRequestBuffer(&requestBuffer);
	} 
	else if (strncmp(buf, "initbuf", 7) == 0) 
	{
		printk("VinBufQueue, running initbuf command\n");
		videoinbuffersize = 1920 *1080 ;
		memset(&videoInBuffer, 0, sizeof(VIDEO_IN_BUFFER_INFO));
		for (i = 0; i < 2; i++) {
			
			obj[i*2] = (unsigned int)vmalloc(videoinbuffersize);
			obj[i*2+1] = (unsigned int)vmalloc(videoinbuffersize);
			printk("VinBufQueue, kzalloc memory (%d), obj[%d]=0x%x, obj[%d]=0x%x\n", \
				i, i*2, obj[i*2], i*2+1, obj[i*2+1]);
			memset(obj[i*2], 0xfe, videoinbuffersize);
			memset(obj[i*2+1], 0xfe, videoinbuffersize);
			
			if (obj[i*2] == NULL || obj[i*2+1] == NULL) {
				printk("VinBufQueue, kzalloc memory failed, i(%d), obj[%d]=0x%x, obj[%d]=0x%x\n", \
					i, i*2, obj[i*2], i*2+1, obj[i*2+1]);
				return;
			}
			if (m4u_alloc_mva(BDP_WR_CHANNEL_VDI,
						obj[i*2],
						videoinbuffersize,
						0,
						0,
						&videoInBuffer.wch1m4uva[i*2]))
			{
				printk("m4u_alloc_mva for videoInBuffer.wch1mva fail\n");
				return;
			}
			
			if (m4u_alloc_mva(BDP_WR_CHANNEL_VDI,
						obj[i*2+1],
						videoinbuffersize,
						0,
						0,
						&videoInBuffer.wch1m4uva[i*2+1]))
			{
				printk("m4u_alloc_mva for videoInBuffer.wch1mva fail\n");
				return;
			}
			m4u_dma_cache_maint(BDP_WR_CHANNEL_VDI,
					(void const *)obj[i*2],
					videoinbuffersize,
					DMA_BIDIRECTIONAL);
			
			m4u_dma_cache_maint(BDP_WR_CHANNEL_VDI,
					(void const *)obj[i*2+1],
					videoinbuffersize,
					DMA_BIDIRECTIONAL);
			
			printk("i = %d @L%d\n",i,__LINE__);
			videoInBuffer.u4BufIndex = i;
			videoInBuffer.vAddrInfo.u4CAddr = obj[i*2];
			videoInBuffer.vAddrInfo.u4YAddr = obj[i*2+1];
			videoInBuffer.vAddrInfo.u4CBufferSize = 1920*1080;
			videoInBuffer.vAddrInfo.u4YBufferSize = 1920*1080;

			videoInBuffer.vAddrInfo_va.u4YAddr = videoInBuffer.wch1m4uva[i*2];
			videoInBuffer.vAddrInfo_va.u4CAddr = videoInBuffer.wch1m4uva[i*2+1];
			videoInBuffer.vAddrInfo_va.u4CBufferSize = 1920*1080;
			videoInBuffer.vAddrInfo_va.u4YBufferSize = 1920*1080;
			printk("videoInBuffer.u4BufIndex = %d @L%d\n",videoInBuffer.u4BufIndex,__LINE__);
			vInitBuffer(&videoInBuffer);
		}
		printk("VinBufQueue, end initbuf command\n");
	}
	else if (strncmp(buf, "startDQ:", 8) == 0) {
		sscanf(buf+8, "%d", &bDQ);
		printk("bDQ = %d\n",bDQ);
		
		if (bDQ == 0) {
			bDQ = false;
		} else {
			bDQ = true;
		}
		printk("VinBufQueue, bDQ: %d\n", bDQ);
	}
	else if(strncmp(buf, "hdmirxhdcpmode:", 15) == 0)
	{
		int hdcpmode;
		sscanf(buf+15, "%d", &hdcpmode);
		printk(" hdcpmode = %d\n", hdcpmode);
		RxHdcpMode(hdcpmode);
	}
    else
    {
        goto Error;
    }

    return;

Error:
    printk("[hdmirx] parse command error!\n\n%s", STR_HELP);
}

static void process_dbg_cmd(char *cmd)
{
    char *tok;

    printk("[hdmirx] %s\n", cmd);

    while ((tok = strsep(&cmd, " ")) != NULL)
    {
        process_dbg_opt(tok);
    }
}

// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

struct dentry *videoin_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}


static char debug_buffer[2048];

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    int n = 0;
    n = strlen(debug_buffer);
    debug_buffer[n++] = 0;

    return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, n);
}


static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
    size_t ret;

    ret = count;

    if (count > debug_bufmax)
    {
        count = debug_bufmax;
    }

    if (copy_from_user(&debug_buffer, ubuf, count))
    {
        return -EFAULT;
    }

    debug_buffer[count] = 0;

    process_dbg_cmd(debug_buffer);

    return ret;
}


static struct file_operations debug_fops =
{
    .read  = debug_read,
    .write = debug_write,
    .open  = debug_open,
};


void HDMIRX_DBG_Init(void)
{
    printk("[HDMIRX]%s\n", __func__);
    videoin_dbgfs = debugfs_create_file("hdmirx",
                                       S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);
}


void HDMIRX_DBG_Deinit(void)
{
    debugfs_remove(videoin_dbgfs);
}

#endif

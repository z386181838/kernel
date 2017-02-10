#if defined(CONFIG_MTK_HDMI_SUPPORT)
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>

#include <mach/mt_typedefs.h>


#if defined(CONFIG_MTK_INTERNAL_HDMI_SUPPORT)
#include "internal_hdmi_drv.h"
#elif defined(MTK_INTERNAL_MHL_SUPPORT)
#include "inter_mhl_drv.h"
#else
#include "hdmi_drv.h"
#endif
//#include "hdmitx.h"
//#include "hdmitx_drv.h"

#if defined(MTK_INTERNAL_MHL_SUPPORT)
#include "mhl_dbg.h"
#endif

#include "hdmihdcp.h"
#include "hdmicec.h"

void DBG_Init(void);
void DBG_Deinit(void);

extern void hdmi_log_enable(int enable);
extern void hdmi_cable_fake_plug_in(void);
extern void hdmi_cable_fake_plug_out(void);
extern void hdmi_mmp_enable(int enable);
extern void hdmi_pattern(int enable);

extern void hdmi_drvlog_enable(unsigned short enable);
extern void hdmi_video_config_ext(HDMI_VIDEO_RESOLUTION res, HDMI_VIDEO_OUT_MODE mode);
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
    "        echo [ACTION]... > hdmi\n"
    "\n"
    "ACTION\n"
    "        hdmitx:[on|off]\n"
    "             enable hdmi video output\n"
    "\n";

static char debug_buffer[2048];
extern size_t hdmi_cec_on;


extern void hdmi_log_enable(int enable);
extern void hdmi_enablehdcp(unsigned char u1hdcponoff);
extern unsigned int u4ReadNValue(void);
extern unsigned int u4ReadCtsValue(void);
extern unsigned int i4SharedInfo (unsigned int u4Index);
extern void UnMuteHDMIAudio(void);
extern void MuteHDMIAudio(void);
extern void vUnBlackHDMIOnly(void);
extern void vBlackHDMIOnly(void);
extern int hdmi_av_enable(int arg);
extern HDMI_AV_INFO_T _stAvdAVInfo;
extern unsigned char _bflagvideomute;
extern unsigned char _bflagaudiomute;
extern unsigned char _bsvpvideomute;
extern unsigned char _bsvpaudiomute;
extern unsigned char _bHdcpOff;


// TODO: this is a temp debug solution
//extern void hdmi_cable_fake_plug_in(void);
//extern int hdmi_drv_init(void);
static void process_dbg_cmd(char *cmd)
{
    char *opcode;
	char * oprand;

	sprintf(debug_buffer + strlen(debug_buffer), "cmd:%s\n", cmd);

	opcode = strsep(&cmd, "=");
	if(opcode == NULL)
		goto error;

	if (0 == strncmp(opcode, "hdcpkeytype", 11))
	{
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(0 == strncmp(oprand, "1", 1))
		{	
			sprintf(debug_buffer + strlen(debug_buffer), "hdcp key type: EXTERNAL_KEY\n");
			vMoveHDCPInternalKey(EXTERNAL_KEY);
		}else if (0 == strncmp(oprand, "2", 1))
		{
			sprintf(debug_buffer + strlen(debug_buffer), "hdcp key type: INTERNAL_NOENCRYPT_KEY\n");
			vMoveHDCPInternalKey(INTERNAL_NOENCRYPT_KEY);
		}else if(0 == strncmp(oprand, "3", 1))
		{
			sprintf(debug_buffer + strlen(debug_buffer), "hdcp key type: INTERNAL_ENCRYPT_KEY\n");
			vMoveHDCPInternalKey(INTERNAL_ENCRYPT_KEY);
		}
	}
	else if (0 == strncmp(opcode, "enablehdcp", 10))
	{
		int enableType = 0;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 10, &enableType))
			goto error;
		hdmi_enablehdcp(enableType);
		sprintf(debug_buffer + strlen(debug_buffer), "enable hdcp %d\n", enableType);
	}
	else if (0 == strncmp(opcode, "res", 3))
	{
		int res = 0;
		int mode = 0;
		char * arg = NULL;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&res))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&mode))
			goto error;

		hdmi_video_config_ext(res, mode);

		sprintf(debug_buffer + strlen(debug_buffer), "set resolution, res = %d, mode = %d\n", res, mode);
	}
	else if (0 == strncmp(opcode, "cecsend", 7))
	{
		CEC_SEND_MSG_T msg;
		char * arg;
		int i = 0;
		
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.ui1_init_addr)))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.ui1_dest_addr)))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.ui2_opcode)))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.z_operand_size)))
			goto error;

		if(msg.t_frame_info.z_operand_size > CEC_MAX_OPERAND_SIZE)
			goto error;

		for(i = 0; i< msg.t_frame_info.z_operand_size; i++){
			arg = strsep(&oprand, ",");
			if(arg == NULL)
				break;

			if(kstrtoint(arg, 10, (int *)&(msg.t_frame_info.aui1_operand[i])))
				goto error;
		}
		msg.pv_tag = NULL;
		msg.b_enqueue_ok = false;

		sprintf(debug_buffer + strlen(debug_buffer), "cec msg info, init addr:%d, dest addr:%d, opcode:%d, oprand size:%d\n", 
			msg.t_frame_info.ui1_init_addr,
			msg.t_frame_info.ui1_dest_addr,
			msg.t_frame_info.ui2_opcode,
			msg.t_frame_info.z_operand_size);

		hdmi_CECMWSend(&msg);
	}
	else if (0 == strncmp(opcode, "cecgetaddr", 10))
	{
		CEC_ADDRESS cec_addr;
		hdmi_NotifyApiCECAddress(&cec_addr);
		sprintf(debug_buffer + strlen(debug_buffer), "pa:%d, la:%d\n", cec_addr.ui2_pa, cec_addr.ui1_la);
	}
	else if (0 == strncmp(opcode, "cecenable", 9))
	{
		int enableType;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 10, &enableType))
			goto error;

		hdmi_CECMWSetEnableCEC(enableType);
		sprintf(debug_buffer + strlen(debug_buffer),"enable cec , enable type:%d\n", enableType);
	}
	else if (0 == strncmp(opcode, "cecstatus", 9))
	{
		if(hdmi_cec_on == 1)
			sprintf(debug_buffer + strlen(debug_buffer),"cec on\n");
		else
			sprintf(debug_buffer + strlen(debug_buffer),"cec off\n");
	}
	else if (0 == strncmp(opcode, "enablelog", 9))
	{
		int enableType;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		if(kstrtoint(oprand, 16, &enableType))
			goto error;
		
		hdmi_drvlog_enable(enableType);
		sprintf(debug_buffer + strlen(debug_buffer),"enable log , enable type:%d\n", enableType);
	}
	else if (0 == strncmp(opcode, "status", 6))
	{
		sprintf(debug_buffer + strlen(debug_buffer),"e_resolution : %d\n", _stAvdAVInfo.e_resolution);
		sprintf(debug_buffer + strlen(debug_buffer),"fgHdmiOutEnable : %d\n", _stAvdAVInfo.fgHdmiOutEnable);
		sprintf(debug_buffer + strlen(debug_buffer),"u2VerFreq : %d\n", _stAvdAVInfo.u2VerFreq);
		sprintf(debug_buffer + strlen(debug_buffer),"b_hotplug_state : %d\n", _stAvdAVInfo.b_hotplug_state);
		sprintf(debug_buffer + strlen(debug_buffer),"e_video_color_space : %d\n", _stAvdAVInfo.e_video_color_space);
		sprintf(debug_buffer + strlen(debug_buffer),"e_deep_color_bit : %d\n", _stAvdAVInfo.e_deep_color_bit);
		sprintf(debug_buffer + strlen(debug_buffer),"ui1_aud_out_ch_number : %d\n", _stAvdAVInfo.ui1_aud_out_ch_number);
		sprintf(debug_buffer + strlen(debug_buffer),"e_hdmi_fs : %d\n", _stAvdAVInfo.e_hdmi_fs);
		sprintf(debug_buffer + strlen(debug_buffer),"bMuteHdmiAudio : %d\n", _stAvdAVInfo.bMuteHdmiAudio);
		sprintf(debug_buffer + strlen(debug_buffer),"u1HdmiI2sMclk : %d\n", _stAvdAVInfo.u1HdmiI2sMclk);
		sprintf(debug_buffer + strlen(debug_buffer),"u1hdcponoff : %d\n", _stAvdAVInfo.u1hdcponoff);
		sprintf(debug_buffer + strlen(debug_buffer),"u1audiosoft : %d\n", _stAvdAVInfo.u1audiosoft);
		sprintf(debug_buffer + strlen(debug_buffer),"fgHdmiTmdsEnable : %d\n", _stAvdAVInfo.fgHdmiTmdsEnable);
		sprintf(debug_buffer + strlen(debug_buffer),"out_mode : %d\n", _stAvdAVInfo.out_mode);
		sprintf(debug_buffer + strlen(debug_buffer),"e_hdmi_aud_in : %d\n", _stAvdAVInfo.e_hdmi_aud_in);
		sprintf(debug_buffer + strlen(debug_buffer),"e_iec_frame : %d\n", _stAvdAVInfo.e_iec_frame);
		sprintf(debug_buffer + strlen(debug_buffer),"e_aud_code : %d\n", _stAvdAVInfo.e_aud_code);
		sprintf(debug_buffer + strlen(debug_buffer),"u1Aud_Input_Chan_Cnt : %d\n", _stAvdAVInfo.u1Aud_Input_Chan_Cnt);
		sprintf(debug_buffer + strlen(debug_buffer),"e_I2sFmt : %d\n", _stAvdAVInfo.e_I2sFmt);
		sprintf(debug_buffer + strlen(debug_buffer),"ACR N= %d, CTS = %d \n", u4ReadNValue(),u4ReadCtsValue());
		sprintf(debug_buffer + strlen(debug_buffer),"_bflagvideomute =%d \n", _bflagvideomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bflagaudiomute =%d \n", _bflagaudiomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bsvpvideomute =%d \n", _bsvpvideomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bsvpaudiomute =%d \n", _bsvpaudiomute);
		sprintf(debug_buffer + strlen(debug_buffer),"_bHdcpOff =%d \n", _bHdcpOff);;
		sprintf(debug_buffer + strlen(debug_buffer),"i4SharedInfo(SI_EDID_VSDB_EXIST) = %d \n", i4SharedInfo(SI_EDID_VSDB_EXIST));
	}
	else if(0 == strncmp(opcode, "mute", 9))
	{
		char * arg;
		int mute_type; //0:video;1 audio
		int mute_en;// 1: mute; 0: unmute
		
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, &mute_type))
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, &mute_en))
			goto error;

		if(mute_type == 0 && mute_en == 1)
		{
			vBlackHDMIOnly();
			sprintf(debug_buffer + strlen(debug_buffer),"mute video\n");
		}
		else if(mute_type == 0 && mute_en == 0)
		{
			vUnBlackHDMIOnly();
			sprintf(debug_buffer + strlen(debug_buffer),"unmute video\n");
		}
		else if(mute_type == 1 && mute_en == 1)
		{
			MuteHDMIAudio();
			sprintf(debug_buffer + strlen(debug_buffer),"mute audio\n");
		}
		else if(mute_type == 1 && mute_en == 0)
		{
			UnMuteHDMIAudio();
			sprintf(debug_buffer + strlen(debug_buffer),"unmute audio\n");
		}
	}
	else if(0 == strncmp(opcode, "avenable", 9))
	{
		char * arg;
		int enable;
		oprand = strsep(&cmd, "=");
		if(oprand == NULL)
			goto error;

		arg = strsep(&oprand, ",");
		if(arg == NULL)
			goto error;

		if(kstrtoint(arg, 10, &enable))
			goto error;

		sprintf(debug_buffer + strlen(debug_buffer),"av enable , arg = %d\n", enable);
		hdmi_av_enable(enable);
	}

	return ;

	error:
		sprintf(debug_buffer + strlen(debug_buffer), "parse command error!\n");
}


// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

struct dentry *hdmitx_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
	if(strlen(debug_buffer))
		return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, strlen(debug_buffer));

    return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));
}



static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
	char cmd_buffer[128];
    const int debug_bufmax = sizeof(cmd_buffer) - 1;
    size_t ret;

	memset(cmd_buffer, 0, sizeof(cmd_buffer));
    ret = count;

    if (count > debug_bufmax)
    {
        count = debug_bufmax;
    }

    if (copy_from_user(&cmd_buffer, ubuf, count))
    {
        return -EFAULT;
    }

    cmd_buffer[count] = 0;
	memset(debug_buffer, 0, sizeof(debug_buffer));
    process_dbg_cmd(cmd_buffer);

    return ret;
}


static struct file_operations debug_fops =
{
    .read  = debug_read,
    .write = debug_write,
    .open  = debug_open,
};


void HDMI_DBG_Init(void)
{
    hdmitx_dbgfs = debugfs_create_file("hdmi",
                                       S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);
}


void HDMI_DBG_Deinit(void)
{
    debugfs_remove(hdmitx_dbgfs);
}

#endif

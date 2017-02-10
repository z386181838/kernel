#ifndef _HDMI_RX_HW_H_
#define _HDMI_RX_HW_H_
#include "hal_io.h"

extern unsigned int ddcci_drv_read(unsigned short u2Reg);
extern void ddcci_drv_write(unsigned short u2Reg, unsigned int u4Data);

#define TOP_CKGEN_BASE    0xF0000000
#define HDMIRX_REG_BASE  (0xFC006000)
#define HDMIRX_PAD_BASE     (0xF0005000)
//HDMI RX analog setting
#define HDMIRX_ANA_REG_BASE     0xF0209000      
//DDCCI
#define DDCCI_REG_BASE     (0xF0015000)      

#define HAL_READ32(_reg_)           			(*((volatile UINT32*)(_reg_)))
#define HAL_WRITE32(_reg_, _val_)   			(*((volatile UINT32*)(_reg_)) = (_val_))
	
#define IO_READ32(base, offset)						HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)		HAL_WRITE32((base) + (offset), (value))
// RX Digtal part 
#define HDMIRX_READ32(offset)           IO_READ32(HDMIRX_REG_BASE, (offset))
#define HDMIRX_WRITE32(offset, value)     IO_WRITE32(HDMIRX_REG_BASE, (offset), (value))
#define HDMIRX_WRITE32_MASK(offset, value, mask)  HDMIRX_WRITE32(offset, ((value & mask) | (HDMIRX_READ32(offset) & (~mask))))

#define HDMIRX_SET_BIT(offset, Bit)        HDMIRX_WRITE32(offset, HDMIRX_READ32(offset) | (Bit))
#define HDMIRX_CLR_BIT(offset, Bit)        HDMIRX_WRITE32(offset, HDMIRX_READ32(offset) & (~(Bit)))

// base is 0x00000000
#define vRxWriteReg(dAddr, dVal) *(volatile UINT32 *)(TOP_CKGEN_BASE + dAddr) = dVal// IO_WRITE32(IO_BASE, dAddr, dVal)//*(volatile DWRD *)(IO_BASE_ADDRESS + HDMI_SYS_REG_OFFSET + dAddr) = dVal
#define vRxReadReg(dAddr) (UINT32)((*(volatile UINT32 *)(TOP_CKGEN_BASE + dAddr))&0xffffffff)
#define vRxWriteRegMsk(dAddr, dVal, dMsk) vRxWriteReg((dAddr), (vRxReadReg(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

// RX analog part 
#define HDMIRX_A_READ32(dAddr)             IO_READ32(HDMIRX_ANA_REG_BASE, (dAddr))
#define HDMIRX_A_WRITE32(dAddr, dVal)     IO_WRITE32(HDMIRX_ANA_REG_BASE, (dAddr), (dVal))
#define HDMIRX_A_WRITE32_MASK(dAddr, dVal, dMsk)  HDMIRX_A_WRITE32(dAddr, ((dVal & dMsk) | (HDMIRX_A_READ32(dAddr) & (~dMsk))))

// DDCCI
#define DDCCI_READ32(dAddr)             ddcci_drv_read((dAddr))
#define DDCCI_WRITE32(dAddr, dVal)     ddcci_drv_write((dAddr), (dVal))
#define DDCCI_WRITE32_MASK(dAddr, dVal, dMsk)  DDCCI_WRITE32((dAddr), (DDCCI_READ32(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define REG_HDMI_ID                                                     (0x000)
    #define DEV_ID              (0xFFFF<<16)
    #define VHDL_ID             (0xFFFF<<0)

#define REG_SRST                                                        (0x004)
    #define DEEP_RST            (0x1<<31) 
    #define BYPASS_DP           (0x1<<30)
    #define DEEP_STA            (0x3<<28)
    #define BYPASS_VPROC_ATUO   (0x1<<27)
    #define BYPASS_VPROC        (0x1<<26)
    #define PHASE_OFFSET        (0x3<<24)
    #define PWR5V_RX2           (0x1<<23)
    #define PWR5V_RX1           (0x1<<22)
    #define CKDTIN_HDMI         (0x1<<21)
    #define BAND_SEL_HW         (0x1<<20)
    #define PWR5V_RX0           (0x1<<19)
    #define VSYNC_CAP           (0x1<<18)
    #define CKDT                (0x1<<17)
    #define SCDT                (0x1<<16)
    #define HDCP_RST_AUTO       (0x1<<15)
    #define ACR_RST_AUTO        (0x1<<14)
    #define AAC_RST             (0x1<<13)
    #define SW_RST_AUTO         (0x1<<12)
    #define HDCP_RST            (0x1<<11)
    #define ACR_RST             (0x1<<10)
    #define FIFO_RST            (0x1<<9)
    #define SW_RST              (0x1<<8)
    #define REV_ID              (0xff<<0)

#define REG_SYS_CTRL                                                    (0x008)
	#define SYS_CTRL_0 (HDMIRX_REG_BASE+0x008)
	#define SYS_CTRL_1 (HDMIRX_REG_BASE+0x009)
	#define SYS_CTRL_2 (HDMIRX_REG_BASE+0x00a)
	#define SYS_CTRL_3 (HDMIRX_REG_BASE+0x00b)
	
    #define DDC_DEL_EN          (0x1<<15)
    #define LONG_STA_HOLD       (0x1<<13)
    #define DDC_EN              (0x1<<12)
    #define ONET_PULSE_EN       (0x1<<11)
    #define PLL_COREID          (0x1<<10)
    #define SYS_RST             (0x1<<9)
    #define RX_EN               (0x1<<8)
    #define OCLKDIV             (0x3<<6)
    #define IDCK                (0x3<<4)
    #define PIXS                (0x1<<3)
    #define BSEL                (0x1<<2)
    #define EDGE                (0x1<<1)
    #define PD_ALL              (0x1<<0)
	
#define REG_VID_SET                                                     (0x010)
    #define CP_PKT_EN         (0x1<<31)
    #define AV_MUTE_CLR       (0x1<<30)
    #define BYPASS_T4_PKT_ERR (0x1<<29)
    #define HDCP_DEC_CAP      (0x1<<28)
    #define CEA_NEW_UNREC_SEL (0x1<<27)
    #define CLR_SUM_TH        (0x1<<26)
    #define S3C3_SET_MUTE     (0x1<<25)
    #define S3C3_CLR_MUTE     (0x1<<24)
    #define H_VDO_GEN         (0x1<<23)
    #define V_VDO_GEN         (0x1<<22)
    #define E_VDO_GEN         (0x1<<19)
    #define R_VDO_GEN         (0x1<<18)
    #define G_VDO_GEN         (0x1<<17)
    #define B_VDO_GEN         (0x1<<16)
    #define EEPROM_ID         (0x3f<<10)
    #define EEPROM_ID_SW      (0x3<<8)
    #define KSCL_H            (0x1<<7)
    #define MSIF_WIGLE_EN     (0x1<<6)
    #define B7_INV            (0x1<<5)
    #define B6_INV            (0x1<<4)
    #define H_V_DE_SWAP       (0x1<<3)
    #define DE_INV            (0x1<<2)
    #define AUTO_OCLKDIV_EN   (0x1<<1)
    #define MUTE_CKDT         (0x1<<0)

#define REG_MUTE_SET                                                    (0x014)
    #define DDC_DEGLITCH_EN   (0x1<<31)
    #define BYPASS_CTL3_FSM   (0x1<<30)
    #define BYPASS_CNTL3      (0x1<<29)
    #define BYPASS_CNTL2      (0x1<<28)
    #define BYPASS_CNTL0      (0x1<<27)
    #define BYPASS_DI_GB      (0x1<<26)
    #define BYPASS_DEC_HV     (0x1<<25)
    #define AUTO_DEC_HV       (0x1<<24)
    #define OP_WINDOW_WIDE    (0x1<<23)
    #define BYPSS_CTL3_FSM_DVI (0x1<<22)
	
    #define ZERO_SEL          (0x1<<21)
    #define TDFIFO_SYNC_EN    (0x1<<20)
    #define NO_VSYNC_EN       (0x1<<19)
    #define RI_AUTO_CLR       (0x1<<18)
    #define AUTO_MUTE_VIDEO   (0x1<<7)
    #define AUTO_MUTE_AUDIO   (0x1<<6)
    #define AUTO_MUTE_CTL3    (0x1<<5)
    #define POL_DET_SEL       (0x1<<4)
    #define POL_DET           (0xf<<0)
	
#define REG_SHD_BKSV0                                                   (0x018)
    #define HDCP_BKSV2        (0xff<<24)
    #define HDCP_BKSV1        (0xff<<16)
    #define HDCP_AINFO        (0xff<<8)

#define REG_SHD_BKSV1                                                   (0x01c)
    #define HDCP_RI0_15_8     (0xff<<24)
    #define HDCP_BKSV5        (0xff<<16)
    #define HDCP_BKSV4        (0xff<<8)
    #define HDCP_BKSV3        (0xff<<0)
	
#define REG_SHD_AKSV                                                    (0x020)
    #define HDCP_AKSV3        (0xff<<24)
    #define HDCP_AKSV2        (0xff<<16)
    #define HDCP_AKSV1        (0xff<<8)
    #define HDCP_RI0_7_0      (0xff<<0)
	
#define REG_SHD_AN0                                                     (0x024)
    #define HDCP_AN2          (0xff<<24)
    #define HDCP_AN1          (0xff<<16)
    #define HDCP_AKSV5        (0xff<<8)
    #define HDCP_AKSV4        (0xff<<0)
		
#define REG_SHD_AN1                                                     (0x028)
    #define HDCP_AN6          (0xff<<24)
    #define HDCP_AN5          (0xff<<16)
    #define HDCP_AN4          (0xff<<8)
    #define HDCP_AN3          (0xff<<0)
	
#define REG_SHD_BSTATUS                                                 (0x02c)
    #define DEV_EXCEED        (0x1<<31)
    #define DEV_CNT           (0x7f<<24)
    #define HDMI_CAPABLE      (0x1<<23)
    #define REPEATER          (0x1<<22)
    #define REG_FIFO_RDY      (0x1<<21)
    #define FAST              (0x1<<20)
    #define FIFO_RDY_WP       (0x1<<19)  
    #define BCAPS1            (0x1<<17)
    #define BCAPS0            (0x1<<16)
    #define HDCP_AN8          (0xff<<8)
    #define HDCP_AN7          (0xff<<0)
	
#define REG_HDCP_STAT                                                   (0x030)
    #define HDCP_DECRYPT      (0x1<<21)
    #define HDCP_AUTH         (0x1<<20)
    #define HDCP_SPHST        (0xf<<16)
    #define HDCP_DEBUG_WP     (0x1<<15)
    #define HDCP_DEBUG        (0x7f<<8)
    #define BSTATUS           (0x7<<5)
    #define RHDMI_MODE_EN     (0x1<<4)
    #define RPT_CASE_EXCEED   (0x1<<3)
    #define DEPTH             (0x7<<0)
		
#define REG_SHA_LENGTH                                                  (0x034)
    #define REG_HBR_LRCK_DLY  (0x1<<31)
    #define REG_B_SHIFT       (0x1<<30)
    #define REG_BP_DLY        (0x1<<29)
    #define REG_HBR_BLOCK     (0x1<<28)
    #define REG_MCLK_RST      (0x1<<27)
    #define SHA_MODE          (0x1<<26)
    #define RPT_V_RDY         (0x1<<24)
    #define V_H$_WP           (0x1<<23)
    #define RPT_START_ADDR_CLR (0x1<<22)
    #define SHA_GO            (0x1<<20)
    #define RPT_START_ADDR_9_8 (0x3<<18)
    #define SHA_LENGTH        (0x3ff<<8)
    #define RPT_START_ADDR_7_0 (0xff<<0)
	
#define REG_VID_HRES                                                    (0x038)
    #define VID_HRES_STB       (0x1<<31)
    #define VRES_CHG_DET_SEL   (0x1<<30)
    #define INTERLACE_DET_SEL  (0x1<<29)
    #define VID_HRES_12_0      (0x1fff<<16)
    #define HCHG_CNT_THR       (0xf<<12)
    #define HSTB_CNT_THR       (0xf<<8)
    #define FLD_DET_SEL        (0x1<<7)
    #define FLD_DET_INI        (0x1<<6)
    #define HRES_THR           (0x3f<<0)
	
#define REG_VID_VRES                                                    (0x03c)
    #define VCHG_CNT_THR       (0xf<<28)
    #define VSTB_CNT_THR       (0xf<<24)
    #define VRES_MUTE_AUTO_CLR (0x1<<23)
    #define VRES_MUTE_CLR      (0x1<<22)
    #define VRES_THR           (0x3f<<16)
    #define VID_VRES_STB       (0x1<<15)
    #define VID_VRES_MUTE      (0x1<<14)
    #define VID_VRES_11_0      (0xfff<<0)
	
#define REG_INTR                                                        (0x040)
    #define INTR_DE_EMPH_CHANGE (0x1<<7)
    #define INTR_AUD_FMT_CHANGE (0x1<<6)
    #define AUD_BLOCK_INTR      (0x1<<5)
    #define AUD_MUTE_INTR       (0x1<<4)
    #define VIDEO_MUTE_INTR     (0x1<<3)
    #define GCP__MUTE_INTR      (0x1<<2)
    #define V_RDY_INTR          (0x1<<1)
    #define KSV_RDY_INTR        (0x1<<0)
	
#define REG_INTR_MASK                                                   (0x044)
    #define INTR_DE_EMPH_CHG_MASK (0x1<<7)
    #define INTR_AUD_FMT_CHG_MASK (0x1<<6)
    #define INTR8_MASK5       (0x1<<5)
    #define INTR8_MASK4       (0x1<<4)
    #define INTR8_MASK3       (0x1<<3)
    #define INTR8_MASK2       (0x1<<2)
    #define INTR8_MASK1       (0x1<<1)
    #define INTR8_MASK0       (0x1<<0)
	
#define REG_VID_MODE                                                    (0x048)
    #define BLANKDATA1        (0xff<<24)
    #define ENSYNCCODES       (0x1<<23)
    #define ENMUXYC           (0x1<<22)
    #define ENDITHER          (0x1<<21)
    #define ENRGB2YCRANGE     (0x1<<20)
    #define ENRGB2YC          (0x1<<19)
    #define ENUPSAMPLE        (0x1<<18)
    #define ENDOWNSAMPLE      (0x1<<17)
    #define ENYC2RGBRANGE     (0x1<<11)
    #define ENYC2RGB          (0x1<<10)
    #define INVERTVSYNC       (0x1<<7)
    #define INVERTHSYNC       (0x1<<6)
    #define REG_MATRIX_NEW    (0x1<<3)
    #define YC2RGBMODE        (0x1<<2)
    #define EXTBITMODE        (0x1<<1)
    #define RGB2YCMODE        (0x1<<0)
	
#define REG_VID_BLANK                                                   (0x04c)
    #define INTERLACE_LINE_DIFF_TH (0xf<<28)
    #define VID_PIXELS        (0xfff<<16)
    #define BLANKDATA3        (0xff<<8)
    #define BLANKDATA2        (0xff<<0)
	
#define REG_VID_STAT                                                    (0x050)
    #define VFRONTPORCH       (0x3f<<24)
    #define V2ACTIVELINES     (0x7f<<16)
    #define VID_DELINES       (0x1fff<<0)
	
#define REG_VID_CH_MAP                                                  (0x054)
    #define SW_CSTAT_8_7       (0x3<<19)
    #define CHANNEL_MAP        (0x7<<16)
    #define INTERLACEDOUT      (0x1<<10)
    #define VSYNCPOL           (0x1<<9)
    #define HSYNCPOL           (0x1<<8)
    #define SW_CSTAT_6_0       (0x7f<<1)
    #define F2BACKPORCH        (0x1<<0)
	
#define REG_VID_HFP                                                     (0x058)
    #define VID_HSACTIVEWIDTH_7_0 (0xff<<24)
    #define VID_HFRONTPORCH    (0x7ff<<8)
	
#define REG_VID_AOF                                                     (0x05c)
    #define VIDOUTMODE         (0xff<<24)
    #define VID_HSACTIVEWIDTH_9_8 (0x3<<0)
	
#define REG_INTR_VS_ISRC1                                               (0x060)
    #define INTR_VSYNC         (0x1<<19)
    #define INTR_NO_VS_PKT     (0x1<<18)
    #define INTR_NEW_VS_PKT    (0x1<<17)
    #define INTR_NEW_ISRC1_PKT (0x1<<16)
    #define INTR_VSYNC_MASK    (0x1<<3)
    #define INTR_NO_VS_PKT_MASK (0x1<<2)
    #define INTR_NEW_VS_PKT_MASK (0x1<<1)
    #define INTR_NEW_ISRC1_PKT_MASK (0x1<<0)
	
#define REG_MCM_REPD                                                    (0x064)
	
#define REG_VID_CRC_CHK                                                 (0x068)
	
#define REG_VID_CRC_OUT                                                 (0x06c)
    #define AAC_XCLK_IN_PCLK_7_0 (0xff<<24)
    #define XCLK_IN_PCLK_SEL    (0x1<<23)
    #define XCLK_IN_DPCLK       (0x1<<22)
    #define CCIR_MTK            (0x1<<21)
    #define RD_BU_KS_EN         (0x1<<20)
    #define AAC_XCLK_IN_PCLK_10_8 (0x7<<16)
    #define INTR_BP             (0x1<<15)
    #define INTR_BP_MASK        (0x1<<14)
    #define INTR_HDMI_CLR_MUTE  (0x1<<13)
    #define INTR_HDMI_CLR_MUTE_MASK (0x1<<12)
    #define CRC_ALG_SEL         (0x1<<8)
    #define ABC_XCLK_DIFF_TH    (0xff<<0)
	
#define REG_INTR_STATE0                                                 (0x070)
    #define INTR3_CEA_NEW_CP    (0x1<<31)
    #define INTR3_CP_SET_MUTE   (0x1<<30)
    #define INTR3_P_ERR         (0x1<<29)
    #define INTR3_NEW_UNREC     (0x1<<28)
    #define INTR3_NEW_MPEG      (0x1<<27)
    #define INTR3_NEW_AUD       (0x1<<26)
    #define INTR3_NEW_SPD       (0x1<<25)
    #define INTR3_NEW_AVI       (0x1<<24)
    #define INTR2_HDMI_MODE     (0x1<<23)
    #define INTR2_VSYNC         (0x1<<22)
    #define INTR2_SOFT_INTR_EN  (0x1<<21)
    #define INTR2_CKDT          (0x1<<20)
    #define INTR2_SCDT          (0x1<<19)
    #define INTR2_GOT_CTS       (0x1<<18)
    #define INTR2_NEW_AUD_PKT   (0x1<<17)
    #define INTR2_CLK_CHG       (0x1<<16)
    #define INTR1_HW_CTS_CHG    (0x1<<15)
    #define INTR1_HW_N_CHG      (0x1<<14)
    #define INTR1_PKT_ERR       (0x1<<13)
    #define INTR1_PLL_UNLOCKED  (0x1<<12)
    #define INTR1_FIFO_ERR      (0x1<<11)
    #define INTR1_BCH_PKT_ERR_ALERT (0x1<<10)
    #define INTR1_AUTH_START    (0x1<<9)
    #define INTR1_AUTH_DONE     (0x1<<8)
    #define SOFT_INTR_EN        (0x1<<3)
    #define INTR_OD             (0x1<<2)
    #define INTR_POLARITY       (0x1<<1)
    #define INTR_STATE          (0x1<<0)
		
#define REG_INTR_MASK0                                                  (0x074)
    #define NEW_ACP_ONLY        (0x1<<5)
    #define NEW_UNREC_ONLY      (0x1<<4)
    #define NEW_MPEG_ONLY       (0x1<<3)
    #define NEW_AUD_ONLY        (0x1<<2)
    #define NEW_SPD_ONLY        (0x1<<1)
    #define NEW_AVI_ONLY        (0x1<<0)
	
#define REG_INTR_STATE1                                                 (0x078)
    #define INTR7_DST           (0x1<<30)
    #define INTR7_RATIO_ERROR   (0x1<<29)
    #define INTR7_AUD_CH_STAT   (0x1<<28)
    #define INTR7_GCP_CD_CHG    (0x1<<27)
    #define INTR7_GAMUT         (0x1<<26)
    #define INTR7_HBR           (0x1<<25)
    #define INTR7_SACD          (0x1<<24)
    #define INTR6_PRE_UNDERUN   (0x1<<23)
    #define INTR6_PRE_OVERUN    (0x1<<22)
	#define INTR6_PWR5V_RX2 	(0x1<<20)
	#define INTR6_PWR5V_RX1 	(0x1<<19)
    #define INTR6_NEW_ACP       (0x1<<18)
    #define INTR6_P_ERR2        (0x1<<17)
    #define INTR6_PWR5V_RX0     (0x1<<16)
    #define INTR5_FN_CHG        (0x1<<15)
    #define INTR5_AUDIO_MUTE    (0x1<<14)
    #define INTR5_BCH_AUDIO_ALERT (0x1<<13)
    #define INTR5_VRESCHG       (0x1<<12)
    #define INTR5_HRESCHG       (0x1<<11)
    #define INTR5_POLCHG        (0x1<<10)
    #define INTR5_INTERLACEOUT  (0x1<<9)
    #define INTR5_AUD_SAMPLE_F  (0x1<<8)
    #define INTR4_PKT_RECEIVED_ALERT (0x1<<7)
    #define INTR4_HDCP_PKT_ERR_ALERT (0x1<<6)
    #define INTR4_T4_PKT_ERR_ALERT   (0x1<<5)
    #define INTR4_NO_AVI         (0x1<<4)
    #define INTR4_CTS_DROPPED_ERR (0x1<<3)
    #define INTR4_CTS_REUSED_ERR  (0x1<<2)
    #define INTR4_OVERRUN         (0x1<<1)
    #define INTR4_UNDERRUN        (0x1<<0)
	
    #define HDMIRX_INT_STATUS_CHK  (INTR4_UNDERRUN|INTR4_OVERRUN|INTR5_AUD_SAMPLE_F)
		
#define REG_INTR_MASK1                                                  (0x07c)
	
#define REG_TMDS_ECTRL                                                  (0x080)
	
#define REG_N_HDMI_CTRL                                                 (0x084)
    #define TT2_EXT_PKT_EN         (0x1<<31)
    #define TT2_GCP_ERR_CLR_EN     (0x1<<30)
    #define TT2_CTSMUL4_EN         (0x1<<29)
    #define TT2_NMUL4_EN           (0x1<<28)
    #define TT2_CTS_DIV4_CARRY_EN  (0x1<<27)
    #define TT2_CTSDIV4_EN         (0x1<<26)
    #define TT2_NDIV4_CARRY_EN     (0x1<<25)
    #define TT2_NDIV4_EN           (0x1<<24)
    #define TT1_SPDIF_IGN_CH       (0x1<<23)
    #define TT1_SPDIF_PICK_8CH     (0x1<<22)
    #define TT1_SPDIF_BYP_OVERUN   (0x1<<21)
    #define TT1_SPDIF_B_8CH        (0x1<<20)
    #define TT1_SPDIF_NO_FLAT      (0x1<<19)
    #define TT1_SPDIF_VUCP_8CH     (0x1<<18)
    #define TT1_SPDIF_REQ_8CH      (0x1<<17)
    #define TT1_8CH                (0x1<<16)
    #define TT0_CD_NO_CLR          (0x1<<15)
    #define TT0_PHASE_SW_SEL       (0x1<<14)
    #define TT0_PP_SW_SEL          (0x1<<13)
    #define TT0_CD_SW_SEL          (0x1<<12)
    #define TT0_HBR_8CH            (0x1<<11)
    #define TT0_GAMUT_EN           (0x1<<10)
    #define TT0_HBR_EN             (0x1<<9)
    #define TT0_SACD_EN            (0x1<<8)
		
#define REG_APLL0                                                       (0x088)
	
#define REG_APLL1                                                       (0x08c)
    #define ACR_DPLL_LOCK          (0x1<<8)
    #define PRESDIVSELOVR          (0x1<<7)
    #define MODOVR                 (0x1<<6)
    #define SDINOVR                (0x1<<5)
    #define FS32OVR                (0x1<<4)
    #define USOVR                  (0x1<<3)
    #define PREDIVSELOVRSEL        (0x1<<2)
    #define FS32OVRVAL             (0x1<<1)
    #define NCLKSEL                (0x1<<0)
	
#define REG_TMDS_CTRL1                                                  (0x094)
    #define EQSEL                  (0xF<<28)
    #define EQ_GAIN                (0x3<<20)
    #define MCK90SEL               (0x1<<26)
	
#define REG_CKDT                                                        (0x09c)
	
#define REG_AEC_CTRL                                                    (0x0b4)
    #define EXP_EN_15_8          (0xff<<24)
    #define EXP_EN_7_0           (0xff<<16)
    #define AAC_OUT_OFF_EN       (0x1<<13)
    #define AUD_DIV_MIN          (0x1<<12)
    #define AAC_MUTE_STAT        (0x1<<11)
    #define AVC_EN               (0x1<<10)
    #define AAC_ALL              (0x1<<9)
    #define AAC_EN               (0x1<<8)
	
#define REG_AEC_EN                                                      (0x0b8)
	
#define REG_ECC_THRES0                                                  (0x0bc)
	
#define REG_ECC_THRES1                                                  (0x0c0)
	
#define REG_ECC_THRES2                                                  (0x0c4)
	
#define REG_PKT_CNT0                                                    (0x0c8)
	
#define REG_PKT_CNT1                                                    (0x0cc)
	
#define REG_HDCP_DS0                                                    (0x0d4)
	
#define REG_HDCP_DS1                                                    (0x0d8)
	
#define REG_HDCP_DS2                                                    (0x0dc)
	
#define REG_HDCP_DS3                                                    (0x0e0)
	
#define REG_HDCP_DS4                                                    (0x0e4)
	
#define REG_HDCP_DS5                                                    (0x0e8)
	
#define REG_HDCP_DS6                                                    (0x0ec)
	
#define REG_HDCP_DS7                                                    (0x0f0)
	
#define REG_DDC_BOX                                                     (0x0f4)
	
#define REG_EPST                                                        (0x0f8)
	#define EPST_0 (HDMIRX_REG_BASE+0x0f8)
	#define EPST_1 (HDMIRX_REG_BASE+0x0f9)
	#define EPST_2 (HDMIRX_REG_BASE+0x0fa)
	#define EPST_3 (HDMIRX_REG_BASE+0x0fb)

	#define OPT_DN_KSV	  (0x1<<12)
	#define LD_KSV 	  (0x1<<21)
	#define KS_MASK_7_0 (0xff<<24)

#define REG_KS_MASK                                                     (0x0fc)
	#define KS_MASK_0 (HDMIRX_REG_BASE+0x0fc)
	#define KS_MASK_1 (HDMIRX_REG_BASE+0x0fd)
	#define KS_MASK_2 (HDMIRX_REG_BASE+0x0fe)
	#define KS_MASK_3 (HDMIRX_REG_BASE+0x0ff)

	
#define REG_ACR_CTRL1                                                   (0x100)
    #define N_VAL_SW_7_0      (0xff<<24)
    #define FM_IN_VAL_SW      (0x3<<22)
    #define FM_VAL_SW         (0x3<<20)
    #define FS_VAL_SW         (0xf<<16)
    #define CTS_DROPPED_AUTO_EN (0x1<<7)
    #define POST_HW_SW_SEL    (0x1<<6)
    #define UPLL_HW_SW_SEL    (0x1<<5)
    #define CTS_HW_SW_SEL     (0x1<<4)
    #define N_HW_SW_SEL       (0x1<<3)
    #define CTS_REUSED_AUTO_EN (0x1<<2)
    #define FS_HW_SW_SEL      (0x1<<1)
    #define ACR_INIT_WP       (0x1<<0)
	
#define REG_N_SVAL                                                      (0x104)
    #define N_VAL_HW_15_0     (0xffff<<16)
    #define N_VAL_SW_19_8     (0xfff<<0)
	
#define REG_N_HVAL                                                      (0x108)
    #define CTS_VAL_SW_19_0   (0xfffff<<8)
    #define N_VAL_HW_19_16    (0xf<<0)
	
#define REG_CTS_HVAL                                                    (0x10c)
    #define UPLL_VAL_SW       (0xff<<24)
    #define CTS_VAL_HW_19_0   (0xfffff<<0)
	
#define REG_UPLL_HVAL                                                   (0x110)
	
#define REG_LK_THRS_SVAL                                                (0x114)
	#define LK_THRS_SVAL_0 (HDMIRX_REG_BASE+0x114)
	#define LK_THRS_SVAL_1 (HDMIRX_REG_BASE+0x115)
	#define LK_THRS_SVAL_2 (HDMIRX_REG_BASE+0x116)
	#define LK_THRS_SVAL_3 (HDMIRX_REG_BASE+0x117)

    #define FS_FILTER_EN       (0x1<<28)
    #define RHDMI_AUD_SAMPLE_F (0xf<<24)
        #define AUD_FS_44K			 (0x0<<24)
        #define AUD_FS_88K			 (0x8<<24)
        #define AUD_FS_176K 		 (0xc<<24)
        #define AUD_FS_48K			 (0x2<<24)
        #define AUD_FS_96K			 (0xa<<24)
        #define AUD_FS_192K 		 (0xe<<24)
        #define AUD_FS_32K			 (0x3<<24)
        #define AUD_FS_UNKNOWN		 (0x1<<24)
    #define LS_THRS_SVAL_19_16 (0xf<<16)
    #define LK_THRS_SVAL_15_8  (0xff<<8)
    #define LK_THRS_SVAL_7_0   (0xff<<0)
	
#define REG_ACR_CTRL3                                                   (0x118)
	#define ACR_CTRL3_0 (HDMIRX_REG_BASE+0x118)
	#define ACR_CTRL3_1 (HDMIRX_REG_BASE+0x119)
	#define ACR_CTRL3_2 (HDMIRX_REG_BASE+0x11a)
	#define ACR_CTRL3_3 (HDMIRX_REG_BASE+0x11b)
	
#define REPEATER_KSV                                                    (0x11c)
	#define KSV_STOP_PLS_SEL (0x1<<11) 
	#define KSV_STOP         (0x1<<10) 
	#define KSV_RISC_WR_SEL  (0x1<<9) 
	
#define KSV_DATA                                                        (0x120)
	
#define REG_I2S_CTRL                                                    (0x124)
	#define I2S_CTRL_0 (HDMIRX_REG_BASE+0x124)
	#define I2S_CTRL_1 (HDMIRX_REG_BASE+0x125)
	#define I2S_CTRL_2 (HDMIRX_REG_BASE+0x126)
	#define I2S_CTRL_3 (HDMIRX_REG_BASE+0x127)

    #define SD3_EN             (0x1<<31)
    #define SD2_EN             (0x1<<30)
    #define SD1_EN             (0x1<<29)
    #define SD0_EN             (0x1<<28)
    #define MCLK_EN            (0x1<<27)
    #define VUCP               (0x1<<25)
    #define PCM                (0x1<<24)
    #define INVALID_EN         (0x1<<23)
    #define CLK_EDGE           (0x1<<22)
    #define SIZE               (0x1<<21)
    #define MSB                (0x1<<20)
    #define WS                 (0x1<<19)
    #define JUSTIFY            (0x1<<18)
    #define DATA_DIR           (0x1<<17)
    #define FIRST_BIT          (0x1<<16)
    #define AAC_ST             (0x7<<0)
	
#define REG_AUDRX_CTRL                                                  (0x128)
	#define AUDRX_CTRL_0 (HDMIRX_REG_BASE+0x128)
	#define AUDRX_CTRL_1 (HDMIRX_REG_BASE+0x129)
	#define AUDRX_CTRL_2 (HDMIRX_REG_BASE+0x12a)
	#define AUDRX_CTRL_3 (HDMIRX_REG_BASE+0x12b)

    #define AUD_CH_STATE2       (0xff<<24)
    #define AUD_CH_STAT1_7_6    (0x3<<22)
    #define AUD_CH_STAT1_5_3    (0x7<<19)
    #define AUD_CH_STAT1_2      (0x1<<18)
    #define AUD_CH_STAT1_1      (0x1<<17)
    #define AUD_CH_STAT1_0      (0x1<<16)
    #define I2S_LENGTH_EN       (0x1<<15)
    #define SWR_EN              (0x1<<14)
    #define HW_MUTE_EN          (0x1<<13)
    #define PASS_SPDIF_ERR      (0x1<<12)
    #define PASS_AUD_ERR        (0x1<<11)
    #define I2S_MODE            (0x1<<10)
    #define SPDIF_MODE          (0x1<<9)
    #define SPDIF_EN            (0x1<<8)
    #define SD3_MAP             (0x3<<6)
    #define SD2_MAP             (0x3<<4)
    #define SD1_MAP             (0x3<<2)
    #define SD0_MAP             (0x1<<0)
	
#define REG_CHST0                                                       (0x12c)
    #define CS_BIT15_8          (0xff<<24)
    #define SWAP                (0xf<<20)
    #define CS_BIT2             (0x1<<18)
    #define OW_EN               (0x1<<16)
    #define DIV_INCR            (0xff<<8)
    #define CH_NUM1             (0xf<<4)
    #define SOURCE1             (0xf<<0)
	
#define REG_CHST1                                                       (0x130)
    #define HDMI_MODE_SW_VALUE  (0x1<<25)
    #define HDMI_MODE_OVERWRITE (0x1<<24)
    #define I2S_LENGTH          (0xf<<20)
    #define CH3_MUTE            (0x1<<19)
    #define CH2_MUTE            (0x1<<18)
    #define CH1_MUTE            (0x1<<17)
    #define CH0_MUTE            (0x1<<16)
    #define AUD_ORG_FS          (0xf<<12)
    #define AUD_LENGTH          (0x7<<9)
    #define AUD_LENGTH_MAX      (0x1<<8)
    #define AUD_ACCURACY        (0xf<<4)
    #define AUD_SAMPLE_F        (0xf<<0)
	
#define REG_AUDP_STAT                                                   (0x134)
    #define MUTE_OUT_POL        (0x1<<26)
    #define AUDIO_MUTE          (0x1<<25)
    #define VIDEO_MUTE          (0x1<<24)
    #define STA_HDCP_CTL3_STATE (0xf<<20)
    #define BYP_OE_FLT          (0x1<<19)
    #define BYP_DVIFILT         (0x1<<18)
    #define BYP_SYNC            (0x1<<17)
    #define BYP_DALIGN          (0x1<<16)
    #define PREAMBLE_CRI        (0x1f<<8)
    #define STA_HDCP_CTL3_DET   (0x1<<7)
    #define STA_HDCP_EN_DET     (0x1<<6)
    #define STA_DI_PRMBL_DET    (0x1<<5)
    #define STA_VID_PRMBL_DET   (0x1<<4)
    #define HDMI_LAYOUT         (0x1<<3)
    #define HDMI_MUTE           (0x1<<2)
    #define HDMI_MODE_EN        (0x1<<1)
    #define HDMI_MODE_DET       (0x1<<0)
	
#define REG_TEST_CTRL                                                   (0x138)
	#define TEST_CTRL_0 (HDMIRX_REG_BASE+0x138)
	#define TEST_CTRL_1 (HDMIRX_REG_BASE+0x139)
	#define TEST_CTRL_2 (HDMIRX_REG_BASE+0x13a)
	#define TEST_CTRL_3 (HDMIRX_REG_BASE+0x13b)

    #define TST_XCLK            (0x1<<31)
    #define TST_CKDT            (0x1<<30)
    #define TST_PLLREF          (0x1<<23)
    #define TST_PLLCK           (0x1<<22)
    #define TST_OSCK            (0x1<<21)
    #define TST_APLLCK          (0x1<<20)
    #define TST_SIF             (0x1<<19)
    #define RBIST_CLK           (0x1<<18)
    #define CORE_ISO_EN         (0x1<<16)
    #define HDMI_FIFO_DIFF      (0x7F<<8)
    #define HDCP_CRI            (0x1F<<0)
	
#define REG_PD_SYS                                                      (0x13c)
	#define PD_SYS_0 (HDMIRX_REG_BASE+0x13c)
	#define PD_SYS_1 (HDMIRX_REG_BASE+0x13d)
	#define PD_SYS_2 (HDMIRX_REG_BASE+0x13e)
	#define PD_SYS_3 (HDMIRX_REG_BASE+0x13f)

    #define PD_AO               (0x1<<31)
    #define PD_VO               (0x1<<30)
    #define PD_APLL             (0x1<<29)
    #define PD_12CHAN           (0x1<<27)
    #define PD_FULL             (0x1<<26)
    #define PD_OSC              (0x1<<25)
    #define PD_XTAL_APLL        (0x1<<24)
    #define PD_PCLK             (0x1<<23)
    #define PD_MCLK             (0x1<<22)
    #define PD_TERM             (0x1<<20)
    #define PD_QO               (0x1<<19)
    #define PD_QE               (0x1<<18)
    #define PD_VHDE             (0x1<<17)
    #define PD_ODCK             (0x1<<16)
    #define PD_TOTAL            (0x1<<0)
	
#define REG_AVIRX0                                                      (0x140)
    #define CEA_AVI_CHECKSUM    (0xff<<24)
    #define CEA_AVI_LENGTH      (0xff<<16)
    #define CEA_AVI_HEADER      (0xffff<<0)
	
#define REG_AVIRX1                                                      (0x144)
	
#define REG_AVIRX2                                                      (0x148)
	
#define REG_AVIRX3                                                      (0x14c)
	
#define REG_AVIRX4                                                      (0x150)
	
#define REG_PCW_CHG_SW                                                  (0x154)
	
#define REG_HDMI_SPH                                                    (0x158)
	
#define REG_HDMI_NEW                                                    (0x15c)
	
#define REG_SPDRX0                                                      (0x160)
	
#define REG_SPDRX1                                                      (0x164)
	
#define REG_SPDRX2                                                      (0x168)
	
#define REG_SPDRX3                                                      (0x16c)
	
#define REG_SPDRX4                                                      (0x170)
	
#define REG_SPDRX5                                                      (0x174)
	
#define REG_SPDRX6                                                      (0x178)
	
#define REG_SPDRX7                                                      (0x17c)
	
#define REG_AUDRX0                                                      (0x180)
    #define CEA_AUD_CHECKSUM   (0xff<<24)
    #define CEA_AUD_LENGTH     (0xff<<16)
    #define CEA_AUD_HEADER_15_8 (0xff<<8)
    #define CEA_AUD_HEADER_7_0 (0xff<<0)
	
#define REG_AUDRX1                                                      (0x184)
    #define CEA_AUD_DBYTE4     (0xff<<24)
    #define CEA_AUD_DBYTE3     (0xff<<16)
    #define CEA_AUD_DBYTE2     (0xff<<8)
    #define CEA_AUD_DBYTE1     (0xff<<0)
		
#define REG_AUDRX2                                                      (0x188)
    #define CEA_AUD_DBYTE8     (0xff<<24)
    #define CEA_AUD_DBYTE7     (0xff<<16)
    #define CEA_AUD_DBYTE6     (0xff<<8)
    #define CEA_AUD_DBYTE5     (0xff<<0)
	
#define REG_AUDRX3                                                      (0x18c)
    #define CEA_AUD_DBYTE10    (0xff<<8)
    #define CEA_AUD_DBYTE9     (0xff<<0)
	
#define REG_DDS_STATUS0                                                 (0x190)
	
#define REG_DDS_STATUS1                                                 (0x194)
	
#define REG_DDS_STATUS2                                                 (0x198)
	
#define REG_MPEGRX0                                                     (0x1a0)
	
#define REG_MPEGRX1                                                     (0x1a4)
	
#define REG_MPEGRX2                                                     (0x1a8)
	
#define REG_MPEGRX3                                                     (0x1ac)
	
#define REG_MPEGRX4                                                     (0x1b0)
	
#define REG_MPEGRX5                                                     (0x1b4)
	
#define REG_MPEGRX6                                                     (0x1b8)
	
#define REG_MPEGRX7                                                     (0x1bc)
    #define MPEG_DEC            (0xff<<24)
	
#define REG_UNRECRX0                                                    (0x1c0)
	
#define REG_UNRECRX1                                                    (0x1c4)
	
#define REG_UNRECRX2                                                    (0x1c8)
	
#define REG_UNRECRX3                                                    (0x1cc)
	
#define REG_UNRECRX4                                                    (0x1d0)
	
#define REG_UNRECRX5                                                    (0x1d4)
	
#define REG_UNRECRX6                                                    (0x1d8)
	
#define REG_UNRECRX7                                                    (0x1dc)
	
#define REG_ACPRX0                                                      (0x1e0)
	
#define REG_ACPRX1                                                      (0x1e4)
	
#define REG_ACPRX2                                                      (0x1e8)
	
#define REG_ACPRX3                                                      (0x1ec)
	
#define REG_ACPRX4                                                      (0x1f0)
	
#define REG_ACPRX5                                                      (0x1f4)
	
#define REG_ACPRX6                                                      (0x1f8)
	
#define REG_ACPRX7                                                      (0x1fc)
    #define ACP_DEC           (0xff<<24)
	
#define REG_GAMUTRX0                                                    (0x200)
	
#define REG_GAMUTRX1                                                    (0x204)
	
#define REG_GAMUTRX2                                                    (0x208)
	
#define REG_GAMUTRX3                                                    (0x20c)
	
#define REG_GAMUTRX4                                                    (0x210)
	
#define REG_GAMUTRX5                                                    (0x214)
	
#define REG_GAMUTRX6                                                    (0x218)
	
#define REG_GAMUTRX7                                                    (0x21c)
	
#define REG_N_HDMI_CTRL1                                                (0x220)
    #define TT88_3_SPDIF_CS_SEL  (0x3<<26)
    #define TT88_3_SPDIF_CH_SEL  (0x3<<24)
    #define TT88_2_I2S_BOUT      (0x1<<21)
    #define TT88_2_I2S_EXT_OUT   (0x1<<20)
    #define TT88_2_CD_CLR_VSYNC_CNT (0xf<<16)
    #define TT88_1_PP_SW        (0xf<<12)
    #define TT88_1_CD_SW        (0xf<<8)
    #define TT88_0_NO_VALID_CD_DLY  (0x1<<7)
    #define TT88_0_BYPS_NO_VALID_CD (0x1<<6)
    #define TT88_0_NEW_GAMUT_ONLY   (0x1<<4)
    #define TT88_0_PHASE_SW         (0x1<<3)
    #define TT88_0_CEA_SPDIF_STAT_SEL (0x1<<0)
	
#define REG_N_HDMI_CTRL2                                                (0x224)
	
#define REG_N_HDMI_CTRL3                                                (0x228)
    #define WS_PERIOD           (0xff<<24)
    #define SCK_PERIOD          (0xff<<16)
    #define ACP_VS_SET          (0x3<<10)
    #define SOFT_ACP_CLR        (0x1<<9)
    #define ACP_CLR_EN          (0x1<<8)
    #define EXT_PKT_DEC         (0xff<<0)
	
#define REG_N_HDMI_CTRL4                                                (0x22c)
    #define TMDS_CK_PERIOD      (0xff<<12)
    #define RADDR               (0x3f<<6)
    #define WADDR               (0x3f<<0)
	
#define REG_N_HDMI_CTRL5                                                (0x230)
	
#define REG_CTS_HW_ADP0                                                 (0x238)
	
#define REG_CTS_HW_ADP1                                                 (0x23c)
	
#define REG_CTS_STATUS                                                  (0x240)
	
#define REG_SDINOVRVAL                                                  (0x244)
	
#define REG_CTS_HW_ADP2                                                 (0x248)
	
#define REG_CTS_HW_ADP3                                                 (0x24c)
	
#define REG_CTS_HW_ADP4                                                 (0x250)
	
#define REG_CTS_HW_ADP5                                                 (0x25c)
	
#define REG_HDMI_DIG_MACRO                                              (0x260)
    #define C_DATA_SYNC_AUTO   (0x1<<15)
		
#define REG_CKDT_RST                                                    (0x264)
    #define RG_RST_LEV         (0x1<<14)
    #define PIXCLK_RST_MODULE_EN (0x1<<13)
    #define PCLK_RST_MODULE_EN (0x1<<12)
    #define DEBUG_SELECT       (0x3<<10)
    #define HDMI_MACRO_SW_RST  (0x1<<9)
    #define PCLK_DET_RST_EN    (0x1<<8)
    #define LOCK_CNT           (0xff<<0)
	
#define REG_HDMI_DIG_MACRO1                                             (0x268)
	
#define REG_AUDIO                                                       (0x26c)
    #define YUV422_OUT_REPACK   (0x1<<15)
	
#define REG_AUDIO1                                                      (0x270)
    #define DDC_SEL             (0x1<<25)
	#define PWR5V_SEL			(0x1<<24)
    #define DSD_SEL             (0x1<<19)
    #define HBR_SEl             (0x1<<18)
    #define HBR_I2S_CLK_SEL     (0x1<<17)
    #define HBR_MCLK_PDWNC      (0x1<<16)
    #define RST_WR_ADDR         (0x1<<11)
    #define RST_RD_ADDR         (0x1<<10)
    #define SET_FIFO_EMPTY      (0x1<<9)
    #define SRAM_READ_SEL       (0x1<<8)
    #define REG_DSD_SIZE        (0x1<<7)
    #define REG_DSD_SCK_ON      (0x1<<6)
    #define DSD_REQ_CNT         (0x3f<<0)
	
#define REG_ANA_STAT0                                                   (0x274)
	#define RGS_HDMI_CH0_STATUS  (0xffff<<0)
	#define RG_HDMI_CH0_STATUS	Fld(16, 0, AC_MSKB1) //15~0
	#define RG_HDMI_CH0_EQERR  Fld(4, 16, AC_MSKB1) //19:16
	
#define REG_ANA_STAT1                                                   (0x278)
	#define RG_HDMI_CH1_STATUS	Fld(16, 0, AC_MSKB1) //15~0
	#define RG_HDMI_CH1_EQERR  Fld(4, 16, AC_MSKB1) //19:16
	
#define REG_ANA_STAT2                                                   (0x27c)
	#define RG_HDMI_CH2_STATUS	Fld(16, 0, AC_MSKB1) //15~0
	#define RG_HDMI_CH2_EQERR  Fld(4, 16, AC_MSKB1) //19:16
	
#define HDCP_DEV                                                        (0x280)

#define HDCP_ADDR                                                       (0x284)

#define HDCP_DATA                                                       (0x288)

#define REG_AUDIO_INFO                                                  (0x294)
	
#define REG_ANA_STAT3                                                   (0x298)
	
#define REG_HDMI_MACRO_STAT0                                            (0x29c)
	
#define REG_HDMI_MACRO_STAT1                                            (0x2a0)
	
#define REG_HDM_MACRO_CRC                                               (0x2a4)
	
#define REG_HDMI_MACRO_STAT2                                            (0x2a8)
	
#define REG_ISRC1RX0                                                    (0x2ac)
    #define REG_VS_VS_SET        (0xf<<4)
    #define RG_SOFT_VS_CLR       (0x1<<3)
    #define RG_VS_CLR_EN         (0x1<<2)
    #define REG_NEW_VS_ONLY      (0x1<<1)
    #define REG_NEW_ISRC1_ONLY   (0x1<<0)
	
#define REG_ISRC1RX1                                                    (0x2b0)
	
#define REG_ISRC1RX2                                                    (0x2b4)
	
#define REG_ISRC1RX3                                                    (0x2b8)
	
#define REG_ISRC1RX4                                                    (0x2bc)
	
#define REG_VSRX0                                                       (0x2c0)
	
#define REG_VSRX1                                                       (0x2c4)
	
#define REG_VSRX2                                                       (0x2c8)
	
#define REG_VSRX3                                                       (0x2cc)
	
#define REG_VSRX4                                                       (0x2d0)
	
#define REG_VSRX5                                                       (0x2d4)
	
#define REG_VSRX6                                                       (0x2d8)
	
#define REG_VSRX7                                                       (0x2dc)
	
#define REG_AUDIO2                                                      (0x2e0)

//analog
#define REG_HDMI_RX_CON00                                                     (0x150)
    #define RG_HDMI_CDR_RST           (0x1<<31)
    #define RG_HDMI_CDR_STOP          (0x1<<30)
    #define RG_HDMI_ATEST_EN          (0x1<<28)
    #define RG_HDMI_ANAMON            (0x7<<24)
    #define RG_HDMI_CH0_CDRLPI        (0x7F<<16)
    #define RG_HDMI_CH1_CDRLPI        (0x7F<<8)
    #define RG_HDMI_CH2_CDRLPI        (0x7F<<0)
	
#define REG_HDMI_RX_CON01                                                     (0x154)
    #define RG_HDMI_BAND_SEL_SW       (0x1<<31)
    #define RG_HDMI_BAND_SEL_SW_EN    (0x1<<30)
    #define RG_HDMI_CKDT_AEN          (0x1<<28)
    #define RG_HDMI_CKDT_SET          (0x1<<27)
    #define RG_HDMI_HYBIAS            (0x3<<25)
    #define RG_HDMI_CLKBUF_EN         (0x1<<24)
    #define RG_HDMI_DATA_RST          (0x1<<23)
    #define RG_HDMI_EQ_PD             (0x1<<22)
    #define RG_HDMI_D2SBIAS           (0x3<<20)
    #define RG_HDMI_EQ_FILT           (0xF<<16)
    #define RG_HDMI_EQ_FIL_SEL        (0x7<<13)
    #define RG_HDMI_EQ_PRBSCK_SEL     (0x1<<12)
    #define RG_HDMI_EQ_SWRSTSEL       (0x1<<11)
    #define RG_HDMI_EQ_RST            (0x1<<10)
    #define RG_HDMI_HY                (0x3<<8)
    #define RG_HDMI_EQ_PATTERN_SEL    (0x1<<7)
    #define RG_HDMI_EQBIAS            (0x3<<5)
    #define RG_HDMI_FULL_EN           (0x1<<3)
    #define RG_HDMI_VREF              (0x7<<0)
	
#define REG_HDMI_RX_CON02                                                     (0x158)
    #define RG_HDMI_0_MHLEN             (0x1<<31)
    #define RG_HDMI_0_MHL_MODE          (0x1<<30)
    #define RG_HDMI_0_FTMODE            (0x1<<29)
    #define RG_HDMI_0_PI_EN             (0x1<<28)
    #define RG_HDMI_0_ZSEN_HDMI        (0x1<<27)
    #define RG_HDMI_0_KPGAIN            (0x7<<24)
    #define RG_HDMI_0_PLLCK_EN          (0x1<<21)
    #define RG_HDMI_0_RXAFE_EN_SW       (0x1<<20)
    #define RG_HDMI_0_RXAFECH0_EN       (0x1<<19)
    #define RG_HDMI_0_RXAFECH1_EN       (0x1<<18)
    #define RG_HDMI_0_RXAFECH2_EN       (0x1<<17)
    #define RG_HDMI_0_TERM_EN           (0x1<<16)
    #define RG_HDMI_0_ZSEN_EN           (0x1<<15)
    #define RG_HDMI_0_TERM_MODE         (0x1<<14)
    #define RG_HDMI_0_RXMODE            (0x3<<12)
	#define RG_HDMI_0_NWELL_ISO         (0x1<<11)
    #define RG_HDMI_0_EQBAND            (0x3<<9)
    #define RG_HDMI_0_TEST_SEL          (0x1<<8)
    #define RG_HDMI_0_EQ_GAIN           (0x3<<4)
    #define RG_HDMI_0_DEEPCLRCLK_PDB    (0x1<<1)
    #define RG_HDMI_0_DEEPCLRCLK_RSTN   (0x1<<0) 
#define REG_HDMI_RX_CON03                                                     (0x15C)
    #define RG_HDMI_PLL_BAND            (0x3F<<24)
	#define RG_HDMI_0_PLL_AUTOK_EN		(0x1<<23)
	#define RG_HDMI_0_PLL_AUTOK_LOAD 	(0x1<<22)
    #define RG_HDMI_PLL_BC              (0x3<<20)
    #define RG_HDMI_PLL_BIC             (0x7<<16)
    #define RG_HDMI_PLL_BIR             (0xF<<12)
    #define RG_HDMI_PLL_BP              (0xF<<8)
    #define RG_HDMI_PLL_BR              (0x7<<4)
	#define RG_HDMI_0_PLL_AUTOK_KS		(0x3<<2)
	#define RG_HDMI_0_PLL_AUTOK_KF		(0x3<<0)

#define REG_HDMI_RX_CON04                                                     (0x160)
    #define RG_HDMI_PLL_DIVCTRL         (0x3<<30)
    #define RG_HDMI_PLL_DIVEN           (0x3F<<24)
    #define RG_HDMI_PLL_FBSEL           (0x3<<22)
    #define RG_HDMI_PLL_BYPASS          (0x1<<21)
    #define RG_HDMI_PLL_MONEN           (0x1<<20)
	#define RG_HDMI_0_PLL_MONREF_EN		(0x1<<19)
    #define RG_HDMI_0_PLL_MONVC_EN      (0x1<<18)
	#define RG_HDMI_0_PLL_POSDIV		(0x3<<16)
    #define RG_HDMI_PLL_PREDIV          (0x3<<12)
	#define RG_HDMI_0_PLL_EN			(0x1<<11)
	#define RG_HDMI_0_PLL_LVORD_EN		(0x1<<10)
	#define RG_HDMI_0_PLL_DETEN		    (0x1<<8)
	#define RG_HDMI_0_TESTE			    (0x1<<7)
	#define RG_HDMI_0_PLL_ICADJ		    (0x3<<5)
	#define RG_HDMI_0_PLL_KVSEL 		(0x3<<3)
	#define RG_HDMI_0_PLL_BCA			(0x1<<2)
	#define RG_HDMI_0_PLL_480PEN		(0x1<<1)
    #define RG_HDMI_0_PLL_VCOBH         (0x1<<0)
#define REG_HDMI_RX_CON05                                                     (0x164)
    #define RG_HDMI_0_PLL_OSC_RST         (0x1<<31)
	#define RG_HDMI_0_CKMON_JITSEL		  (0x1<<30)
    #define RG_HDMI_0_CKMON_DIV4EN        (0x1<<29)
	#define RG_HDMI_0_CKMON_JITEN         (0x1<<28)
    #define RG_HDMI_0_PLL_V11EN           (0x1<<27)
    #define RG_HDMI_0_FLAG_SEL        (0x1<<26)
    #define RG_HDMI_0_ABIST_EN      (0x1<<25)
    #define RG_HDMI_0_ABISTCK_SEL      (0x1<<24)
    #define RG_HDMI_0_CKMON_CKSET          (0x3<<21)
    #define RG_HDMI_0_CH0_MONEN          (0x1<<20)
    #define RG_HDMI_0_CH1_MONEN       (0x1<<19)
    #define RG_HDMI_0_CH2_MONEN       (0x1<<18)
    #define RG_HDMI_0_CKMON_PLLSEL         (0x1<<17)
    #define RG_HDMI_0_EQ_CALEN         (0x1<<16)
    #define RG_HDMI_0_CH2_EQMC          (0xF<<12)
    #define RG_HDMI_0_CH1_EQMC          (0xf<<8)
    #define RG_HDMI_0_CH0_EQMC          (0xf<<4)
	#define RG_HDMI_0_BG_EN			(0x1<<3)
	#define RG_HDMI_0_BG_CHOP_EN			(0x1<<2)
    #define RG_HDMI_0_BG_DIV          (0x3<<0)
#define REG_HDMI_RX_CON06                                                     (0x168)
	#define RG_HDMI_0_PLL_FBKDIV       (0x7F<<0)

#define REG_HDMI_RX_CON07                                            (0x16c)
	
#define REG_HDMI_RX_CON08                                            (0x170)
    #define RGS_HDMI_0_CH0_CDRLPO       (0x7F<<24)
    #define RGS_HDMI_0_CH0_ABIST_FLAG1  (0x1<<21)
    #define RGS_HDMI_0_CH0_ABIST_FLAG2  (0x1<<20)
    #define RGS_HDMI_0_CH0_EQERR        (0xF<<16)
    #define RGS_HDMI_0_CH0_STATUS       (0xFFFF<<0)
	
#define REG_HDMI_RX_CON09                                            (0x174)
    #define RGS_HDMI_0_CH1_CDRLPO       (0x7F<<24)
    #define RGS_HDMI_0_CH1_ABIST_FLAG1  (0x1<<21)
    #define RGS_HDMI_0_CH1_ABIST_FLAG2  (0x1<<20)
    #define RGS_HDMI_0_CH1_EQERR        (0xF<<16)
    #define RGS_HDMI_0_CH1_STATUS       (0xFFFF<<0)
	
#define REG_HDMI_RX_CON10                                            (0x178)
	#define RGS_HDMI_0_CH2_CDRLPO		(0x7F<<24)
	#define RGS_HDMI_0_CH2_ABIST_FLAG1	(0x1<<21)
	#define RGS_HDMI_0_CH2_ABIST_FLAG2	(0x1<<20)
	#define RGS_HDMI_0_CH2_EQERR		(0xF<<16)
	#define RGS_HDMI_0_CH2_STATUS		(0xFFFF<<0)
	
#define REG_HDMI_RX_CON11                                            (0x17C)
	#define RGS_HDMI_0_CH0_EQMM		    (0x1<<31)
	#define RGS_HDMI_0_CH1_EQMM      	(0x1<<30)
	#define RGS_HDMI_0_CH2_EQMM     	(0x1<<29)
	#define RGS_HDMI_0_PLL_AUTOK_PASS	(0x1<<24)
	#define RGS_HDMI_0_PLL_AUTOK_FAIL	(0x1<<23)
	#define RGS_HDMI_0_PLL_AUTOK_BAND   (0x7F<<16)

/*-----------------------------------------------------------------------------------
//moudle: DDCCI
//function: IO base 0x1C005000
------------------------------------------------------------------------------------*/

#define REG_DDC_HDMI0_CNTL0                                                     (0x000)
#define REG_DDC_HDMI0_CNTL1                                                     (0x004)
    #define EDID_CHK_SUM           (0xFF<<16)
    #define HDMI0_EDID_DEV_ADDR    (0xFF<<8)
    #define HDMI0_EDID_PADR_OFST   (0xFF<<0)
#define REG_DDC_HDMI1_CNTL0                                                     (0x008)
#define REG_DDC_HDMI1_CNTL1                                                     (0x00C)
#define REG_DDC_MISC_CNTL0                                                      (0x01C)
    #define DEBUG_SEL              (0x7<<28)
    #define DLADR_AUTO_INC         (0x1<<26)
    #define CHANNEL_SEL            (0x3<<24)
    #define EDID_DL_MODE           (0x1<<23)
    #define EDID_DL_ADDR           (0x7F<<16)
#define REG_DDC_EDID_READBACK_DATA                                               (0x020)
#define REG_DDC_INTERRUPT                                                        (0x024)
#define REG_DDC_INTERRUPT_EN                                                     (0x028)
#define REG_DDC_INTERRUPT_CLR                                                    (0x02C)
#define REG_DDC_DDCCI_RDATA0                                                     (0x030)
#define REG_DDC_DDCCI_RDATA1                                                     (0x034)
#define REG_DDC_DDCCI_RDATA2                                                     (0x038)
#define REG_DDC_DDCCI_RDATA3                                                     (0x03C)
#define REG_DDC_DDCCI_WDATA0                                                     (0x040)
#define REG_DDC_DDCCI_WDATA1                                                     (0x044)
#define REG_DDC_DDCCI_WDATA2                                                     (0x048)
#define REG_DDC_DDCCI_WDATA3                                                     (0x04C)
#define REG_DDC_EDID_DOWNLOAD_PORT                                               (0x050)
    #define EDID_DL_PORT           (0xFFFFFFFF<<0)
#define REG_DDC_SIF_SLAVE0_PKT_L                                                (0x054)
#define REG_DDC_HDMI0_CNTL2                                                     (0x060)
#define REG_DDC_HDMI0_CNTL3                                                     (0x064)
#define REG_DDC_HDMI1_CNTL2                                                     (0x068)
#define REG_DDC_HDMI1_CNTL3                                                     (0x06C)


#define IO_HDMIRX_PAD_PD 0x100
    
	#define IO_PAD_SDA_RX_PD (1<<4)
	#define IO_PAD_SCL_RX_PD (1<<5)
	#define IO_PAD_HPD_RX_PD (1<<7)
	#define IO_PAD_PWR_RX_PD (1<<8)

#define IO_PULL_EN 0x240
	#define IO_HDMIRX_SDA_PULL_EN (1<<4)
	#define IO_HDMIRX_SCL_PULL_EN (1<<5)

#define IO_HDMIRX_PAD_EN 0x247
	#define IO_PAD_HPD_RX_EN (1<<7)
	#define IO_PAD_PWR_RX_EN (1<<8)
	
#define IO_HDMIRX_PAD_OUTPUT_VALUE 0x5F0
	#define IO_PAD_SDA_RX_EN (1<<4)
	#define IO_PAD_SCL_RX_EN (1<<5)

#define IO_HDMIRX_PAD_DIN2 0x640
#define IO_PAD_HDMI_RX_POWER_5V_STATUS (1<<10)

#define IO_HDMIRX_PAD_PD 0x720
#define IO_PAD_HOT_PLUG_PD (1<<11)

#define IO_HDMIRX_SDA_PAD_MODE 0xA60

#define PAD_MODE_SDA_RX      0x1 << 12
#define PAD_MODE_SDA_RX_MASK 0x7 << 12

#define IO_HDMIRX_PAD_MODE 0xA70
#define PAD_MODE_SCL_RX      0x1 << 0
#define PAD_MODE_SCL_RX_MASK 0x7 << 0
#define PAD_MODE_HPD_RX      0x1 << 6
#define PAD_MODE_HPD_RX_MASK 0x7 << 6
#define PAD_MODE_TEST_OUTP_RX      0x1 << 9
#define PAD_MODE_TEST_OUTP_RX_MASK 0x7 << 9

#define vWriteHdmirxIoPad(dAddr, dVal)  (IO_WRITE32(HDMIRX_PAD_BASE, (dAddr), (dVal)))
#define dReadHdmirxIoPad(dAddr)         (IO_READ32(HDMIRX_PAD_BASE, (dAddr)))
#define vWriteHdmirxIoPadMsk(dAddr, dVal, dMsk) vWriteHdmirxIoPad((dAddr), (dReadHdmirxIoPad(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))


#endif
	

#ifndef MT_SD_H
#define  MT_SD_H

#include <linux/bitops.h>
#include <linux/mmc/host.h>
#include <mach/sync_write.h>
#include <mach/mt_reg_base.h>
#include <linux/semaphore.h>

#define MTK_MSDC_USE_CMD23
#ifdef MTK_MSDC_USE_CMD23
#define MSDC_USE_AUTO_CMD23   1 
#endif

#define OBSOLETE_IN_MT7623    0

/* the macro need removed after SQC */
#define REMOVEED_FOR_MT6592 (1)
#define MODIFY_FOR_MT6592   (1)
//#define MSDC_DMA_BOUNDARY_LIMITAION


#define MAX_GPD_NUM         (1 + 1)  /* one null gpd */
#define MAX_BD_NUM          (1024)
#define MAX_BD_PER_GPD      (MAX_BD_NUM)
#define HOST_MAX_NUM        (4)
#define CLK_SRC_MAX_NUM		(1)

#if (0 == REMOVEED_FOR_MT6592)
#define PERI_MSDC0_PDN      (13)
#define PERI_MSDC1_PDN      (14)
#define PERI_MSDC2_PDN      (15)
#define PERI_MSDC3_PDN      (16)
#define PERI_MSDC4_PDN      (17)
#endif

#define CUST_EINT_POLARITY_LOW              0
#define CUST_EINT_POLARITY_HIGH             1
#define CUST_EINT_DEBOUNCE_DISABLE          0
#define CUST_EINT_DEBOUNCE_ENABLE           1
#define CUST_EINT_EDGE_SENSITIVE            0
#define CUST_EINT_LEVEL_SENSITIVE           1
//////////////////////////////////////////////////////////////////////////////

/* msdc2 EINT just for evb only */
//#define EINT_MSDC2_INS_NUM              11
//#define EINT_MSDC2_INS_DEBOUNCE_CN      1
#define EINT_MSDC2_INS_POLARITY         CUST_EINT_POLARITY_LOW
//#define EINT_MSDC2_INS_SENSITIVE        CUST_EINT_LEVEL_SENSITIVE
//#define EINT_MSDC2_INS_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE

//#define EINT_MSDC1_INS_NUM              16
#define EINT_MSDC1_INS_DEBOUNCE_CN     500 
#define EINT_MSDC1_INS_POLARITY         CUST_EINT_POLARITY_LOW
#define EINT_MSDC1_INS_SENSITIVE        CUST_EINT_LEVEL_SENSITIVE
#define EINT_MSDC1_INS_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE

//#define MSDC_CLKSRC_REG     (0xf100000C)
#define MSDC_DESENSE_REG	(0xf0007070)

#ifdef MTK_SDIOAUTOK_SUPPORT
#define MTK_SDIO30_ONLINE_TUNING_SUPPORT
//#define ONLINE_TUNING_DVTTEST
//#define OT_LATENCY_TEST
//#define OT_ON
#endif  // MTK_SDIOAUTOK_SUPPORT

#ifdef CONFIG_MT7623_FPGA
#define FPGA_PLATFORM
#endif
#define MSDC_AUTOCMD12          (0x0001)
#define MSDC_AUTOCMD23          (0x0002)
#define MSDC_AUTOCMD19          (0x0008)
#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
#define MSDC_AUTOCMD53          (0x0004)
#endif	// MTK_SDIO30_ONLINE_TUNING_SUPPORT
/*--------------------------------------------------------------------------*/
/* Common Macro                                                             */
/*--------------------------------------------------------------------------*/
#define REG_ADDR(x)                 ((volatile u32*)(base + OFFSET_##x))

/*--------------------------------------------------------------------------*/
/* Common Definition                                                        */
/*--------------------------------------------------------------------------*/
#define MSDC_FIFO_SZ            (128)
#define MSDC_FIFO_THD           (64)  // (128)
#define MSDC_NUM                (4)

#define MSDC_MS                 (0) //No memory stick mode, 0 use to gate clock
#define MSDC_SDMMC              (1)

#define MSDC_MODE_UNKNOWN       (0)
#define MSDC_MODE_PIO           (1)
#define MSDC_MODE_DMA_BASIC     (2)
#define MSDC_MODE_DMA_DESC      (3)
#define MSDC_MODE_DMA_ENHANCED  (4)
#define MSDC_MODE_MMC_STREAM    (5)

#define MSDC_BUS_1BITS          (0)
#define MSDC_BUS_4BITS          (1)
#define MSDC_BUS_8BITS          (2)

#define MSDC_BRUST_8B           (3)
#define MSDC_BRUST_16B          (4)
#define MSDC_BRUST_32B          (5)
#define MSDC_BRUST_64B          (6)

#define MSDC_PIN_PULL_NONE      (0)
#define MSDC_PIN_PULL_DOWN      (1)
#define MSDC_PIN_PULL_UP        (2)
#define MSDC_PIN_PULL_UP_10K    (2)
#define MSDC_PIN_PULL_UP_50K    (3)
#define MSDC_PIN_KEEP           (4)

#define MSDC_EMMC_BOOTMODE0     (0)     /* Pull low CMD mode */
#define MSDC_EMMC_BOOTMODE1     (1)     /* Reset CMD mode */

enum {
    RESP_NONE = 0,
    RESP_R1,
    RESP_R2,
    RESP_R3,
    RESP_R4,
    RESP_R5,
    RESP_R6,
    RESP_R7,
    RESP_R1B
};

/*--------------------------------------------------------------------------*/
/* Register Offset                                                          */
/*--------------------------------------------------------------------------*/
#define OFFSET_MSDC_CFG                  (0x00)
#define OFFSET_MSDC_IOCON                (0x04)
#define OFFSET_MSDC_PS                   (0x08)
#define OFFSET_MSDC_INT                  (0x0c)
#define OFFSET_MSDC_INTEN                (0x10)
#define OFFSET_MSDC_FIFOCS               (0x14)
#define OFFSET_MSDC_TXDATA               (0x18)
#define OFFSET_MSDC_RXDATA               (0x1c)
#define OFFSET_SDC_CFG                   (0x30)
#define OFFSET_SDC_CMD                   (0x34)
#define OFFSET_SDC_ARG                   (0x38)
#define OFFSET_SDC_STS                   (0x3c)
#define OFFSET_SDC_RESP0                 (0x40)
#define OFFSET_SDC_RESP1                 (0x44)
#define OFFSET_SDC_RESP2                 (0x48)
#define OFFSET_SDC_RESP3                 (0x4c)
#define OFFSET_SDC_BLK_NUM               (0x50)
#define OFFSET_SDC_VOL_CHG               (0x54)
#define OFFSET_SDC_CSTS                  (0x58)
#define OFFSET_SDC_CSTS_EN               (0x5c)
#define OFFSET_SDC_DCRC_STS              (0x60)

/* Only for EMMC Controller 4 registers below */
#define OFFSET_EMMC_CFG0                 (0x70)
#define OFFSET_EMMC_CFG1                 (0x74)
#define OFFSET_EMMC_STS                  (0x78)
#define OFFSET_EMMC_IOCON                (0x7c)

#define OFFSET_SDC_ACMD_RESP             (0x80)
#define OFFSET_SDC_ACMD19_TRG            (0x84)
#define OFFSET_SDC_ACMD19_STS            (0x88)
#define OFFSET_MSDC_DMA_HIGH4BIT         (0x8C)
#define OFFSET_MSDC_DMA_SA               (0x90)
#define OFFSET_MSDC_DMA_CA               (0x94)
#define OFFSET_MSDC_DMA_CTRL             (0x98)
#define OFFSET_MSDC_DMA_CFG              (0x9c)
#define OFFSET_MSDC_DBG_SEL              (0xa0)
#define OFFSET_MSDC_DBG_OUT              (0xa4)
#define OFFSET_MSDC_DMA_LEN              (0xa8)
#define OFFSET_MSDC_PATCH_BIT0           (0xb0)
#define OFFSET_MSDC_PATCH_BIT1           (0xb4)
#define OFFSET_MSDC_PATCH_BIT2           (0xb8)

/* Only for SD/SDIO Controller 6 registers below */
#define OFFSET_DAT0_TUNE_CRC             (0xc0)
#define OFFSET_DAT1_TUNE_CRC             (0xc4)
#define OFFSET_DAT2_TUNE_CRC             (0xc8)
#define OFFSET_DAT3_TUNE_CRC             (0xcc)
#define OFFSET_CMD_TUNE_CRC              (0xd0)
#define OFFSET_SDIO_TUNE_WIND            (0xd4)

#define OFFSET_MSDC_PAD_TUNE             (0xF0)
#define OFFSET_MSDC_PAD_TUNE0            (0xF0)
#define OFFSET_MSDC_PAD_TUNE1            (0xF4)
#define OFFSET_MSDC_DAT_RDDLY0           (0xF8)
#define OFFSET_MSDC_DAT_RDDLY1           (0xFC)

#define OFFSET_MSDC_DAT_RDDLY2           (0x100)
#define OFFSET_MSDC_DAT_RDDLY3           (0x104)

#define OFFSET_MSDC_HW_DBG               (0x110)
#define OFFSET_MSDC_VERSION              (0x114)
#define OFFSET_MSDC_ECO_VER              (0x118)

/* Only for EMMC 5.0 Controller 4 registers below */
#define OFFSET_EMMC50_PAD_CTL0           (0x180)
#define OFFSET_EMMC50_PAD_DS_CTL0        (0x184)
#define OFFSET_EMMC50_PAD_DS_TUNE        (0x188)
#define OFFSET_EMMC50_PAD_CMD_TUNE       (0x18c)
#define OFFSET_EMMC50_PAD_DAT01_TUNE     (0x190)
#define OFFSET_EMMC50_PAD_DAT23_TUNE     (0x194)
#define OFFSET_EMMC50_PAD_DAT45_TUNE     (0x198)
#define OFFSET_EMMC50_PAD_DAT67_TUNE     (0x19c)
#define OFFSET_EMMC51_CFG0               (0x204)
#define OFFSET_EMMC50_CFG0               (0x208)
#define OFFSET_EMMC50_CFG1               (0x20c)
#define OFFSET_EMMC50_CFG2               (0x21c)
#define OFFSET_EMMC50_CFG3               (0x220)
#define OFFSET_EMMC50_CFG4               (0x224)
#define OFFSET_EMMC50_BLOCK_LENGTH       (0x228)
/*--------------------------------------------------------------------------*/
/* Register Address                                                         */
/*--------------------------------------------------------------------------*/
/* common register */
#define MSDC_CFG                         REG_ADDR(MSDC_CFG)
#define MSDC_IOCON                       REG_ADDR(MSDC_IOCON)
#define MSDC_PS                          REG_ADDR(MSDC_PS)
#define MSDC_INT                         REG_ADDR(MSDC_INT)
#define MSDC_INTEN                       REG_ADDR(MSDC_INTEN)
#define MSDC_FIFOCS                      REG_ADDR(MSDC_FIFOCS)
#define MSDC_TXDATA                      REG_ADDR(MSDC_TXDATA)
#define MSDC_RXDATA                      REG_ADDR(MSDC_RXDATA)

/* sdmmc register */
#define SDC_CFG                          REG_ADDR(SDC_CFG)
#define SDC_CMD                          REG_ADDR(SDC_CMD)
#define SDC_ARG                          REG_ADDR(SDC_ARG)
#define SDC_STS                          REG_ADDR(SDC_STS)
#define SDC_RESP0                        REG_ADDR(SDC_RESP0)
#define SDC_RESP1                        REG_ADDR(SDC_RESP1)
#define SDC_RESP2                        REG_ADDR(SDC_RESP2)
#define SDC_RESP3                        REG_ADDR(SDC_RESP3)
#define SDC_BLK_NUM                      REG_ADDR(SDC_BLK_NUM)
#define SDC_VOL_CHG                      REG_ADDR(SDC_VOL_CHG)
#define SDC_CSTS                         REG_ADDR(SDC_CSTS)
#define SDC_CSTS_EN                      REG_ADDR(SDC_CSTS_EN)
#define SDC_DCRC_STS                     REG_ADDR(SDC_DCRC_STS)

/* emmc register*/
#define EMMC_CFG0                        REG_ADDR(EMMC_CFG0)
#define EMMC_CFG1                        REG_ADDR(EMMC_CFG1)
#define EMMC_STS                         REG_ADDR(EMMC_STS)
#define EMMC_IOCON                       REG_ADDR(EMMC_IOCON)

/* auto command register */
#define SDC_ACMD_RESP                    REG_ADDR(SDC_ACMD_RESP)
#define SDC_ACMD19_TRG                   REG_ADDR(SDC_ACMD19_TRG)
#define SDC_ACMD19_STS                   REG_ADDR(SDC_ACMD19_STS)

/* dma register */
#define MSDC_DMA_HIGH4BIT                REG_ADDR(MSDC_DMA_HIGH4BIT)
#define MSDC_DMA_SA                      REG_ADDR(MSDC_DMA_SA)
#define MSDC_DMA_CA                      REG_ADDR(MSDC_DMA_CA)
#define MSDC_DMA_CTRL                    REG_ADDR(MSDC_DMA_CTRL)
#define MSDC_DMA_CFG                     REG_ADDR(MSDC_DMA_CFG)

/* debug register */
#define MSDC_DBG_SEL                     REG_ADDR(MSDC_DBG_SEL)
#define MSDC_DBG_OUT                     REG_ADDR(MSDC_DBG_OUT)
#define MSDC_DMA_LEN                     REG_ADDR(MSDC_DMA_LEN)

/* misc register */
#define MSDC_PATCH_BIT                   REG_ADDR(MSDC_PATCH_BIT0)   /* Alias for MT8127 */
#define MSDC_PATCH_BIT0                  REG_ADDR(MSDC_PATCH_BIT0)
#define MSDC_PATCH_BIT1                  REG_ADDR(MSDC_PATCH_BIT1)
#define MSDC_PATCH_BIT2                  REG_ADDR(MSDC_PATCH_BIT2)
#define DAT0_TUNE_CRC                    REG_ADDR(DAT0_TUNE_CRC)
#define DAT1_TUNE_CRC                    REG_ADDR(DAT1_TUNE_CRC)
#define DAT2_TUNE_CRC                    REG_ADDR(DAT2_TUNE_CRC)
#define DAT3_TUNE_CRC                    REG_ADDR(DAT3_TUNE_CRC)
#define CMD_TUNE_CRC                     REG_ADDR(CMD_TUNE_CRC)
#define SDIO_TUNE_WIND                   REG_ADDR(SDIO_TUNE_WIND)
#define MSDC_PAD_TUNE                    REG_ADDR(MSDC_PAD_TUNE)
#define MSDC_PAD_TUNE0                   REG_ADDR(MSDC_PAD_TUNE0)
#define MSDC_PAD_TUNE1                   REG_ADDR(MSDC_PAD_TUNE1)

/* data read delay */
#define MSDC_DAT_RDDLY0                  REG_ADDR(MSDC_DAT_RDDLY0)
#define MSDC_DAT_RDDLY1                  REG_ADDR(MSDC_DAT_RDDLY1)
#define MSDC_DAT_RDDLY2                  REG_ADDR(MSDC_DAT_RDDLY2)
#define MSDC_DAT_RDDLY3                  REG_ADDR(MSDC_DAT_RDDLY3)

#define MSDC_HW_DBG                      REG_ADDR(MSDC_HW_DBG)
#define MSDC_VERSION                     REG_ADDR(MSDC_VERSION)
#define MSDC_ECO_VER                     REG_ADDR(MSDC_ECO_VER)
/* eMMC 5.0 register */
#define EMMC50_PAD_CTL0                  REG_ADDR(EMMC50_PAD_CTL0)
#define EMMC50_PAD_DS_CTL0               REG_ADDR(EMMC50_PAD_DS_CTL0)
#define EMMC50_PAD_DS_TUNE               REG_ADDR(EMMC50_PAD_DS_TUNE)
#define EMMC50_PAD_CMD_TUNE              REG_ADDR(EMMC50_PAD_CMD_TUNE)
#define EMMC50_PAD_DAT01_TUNE            REG_ADDR(EMMC50_PAD_DAT01_TUNE)
#define EMMC50_PAD_DAT23_TUNE            REG_ADDR(EMMC50_PAD_DAT23_TUNE)
#define EMMC50_PAD_DAT45_TUNE            REG_ADDR(EMMC50_PAD_DAT45_TUNE)
#define EMMC50_PAD_DAT67_TUNE            REG_ADDR(EMMC50_PAD_DAT67_TUNE)
#define EMMC51_CFG0                      REG_ADDR(EMMC51_CFG0)
#define EMMC50_CFG0                      REG_ADDR(EMMC50_CFG0)
#define EMMC50_CFG1                      REG_ADDR(EMMC50_CFG1)
#define EMMC50_CFG2                      REG_ADDR(EMMC50_CFG2)
#define EMMC50_CFG3                      REG_ADDR(EMMC50_CFG3)
#define EMMC50_CFG4                      REG_ADDR(EMMC50_CFG4)
#define EMMC50_BLOCK_LENGTH              REG_ADDR(EMMC50_BLOCK_LENGTH)

/*--------------------------------------------------------------------------*/
/* Register Mask                                                            */
/*--------------------------------------------------------------------------*/

/* MSDC_CFG mask */
#define MSDC_CFG_MODE           (0x1   <<  0)     /* RW */
#define MSDC_CFG_CKPDN          (0x1   <<  1)     /* RW */
#define MSDC_CFG_RST            (0x1   <<  2)     /* A0 */
#define MSDC_CFG_PIO            (0x1   <<  3)     /* RW */
#define MSDC_CFG_CKDRVEN        (0x1   <<  4)     /* RW */
#define MSDC_CFG_BV18SDT        (0x1   <<  5)     /* RW */
#define MSDC_CFG_BV18PSS        (0x1   <<  6)     /* R  */
#define MSDC_CFG_CKSTB          (0x1   <<  7)     /* R  */
#define MSDC_CFG_CKDIV          (0xFFF <<  8)     /* RW   !!! MT7623 change 0xFF ->0xFFF*/
#define MSDC_CFG_CKMOD          (0x3   << 20)     /* W1C  !!! MT7623 change 16 ->21 only for eMCC 5.0*/
#define MSDC_CFG_CKMOD_HS400    (0x1   << 22)     /* RW   !!! MT7623 change 18 ->22 only for eMCC 5.0*/
#define MSDC_CFG_START_BIT      (0x3   << 23)     /* RW   !!! MT7623 change 19 ->23 only for eMCC 5.0*/
#define MSDC_CFG_SCLK_STOP_DDR  (0x1   << 25)     /* RW   !!! MT7623 change 21 ->25 */

/* MSDC_IOCON mask */
#define MSDC_IOCON_SDR104CKS    (0x1   <<  0)     /* RW */
#define MSDC_IOCON_RSPL         (0x1   <<  1)     /* RW */
#define MSDC_IOCON_R_D_SMPL     (0x1   <<  2)     /* RW */
#define MSDC_IOCON_DSPL          MSDC_IOCON_R_D_SMPL /* alias */

#define MSDC_IOCON_DDLSEL       (0x1   <<  3)     /* RW */
#define MSDC_IOCON_DDR50CKD     (0x1   <<  4)     /* RW */
#define MSDC_IOCON_R_D_SMPL_SEL (0x1   <<  5)     /* RW */
#define MSDC_IOCON_RDSPLSEL     MSDC_IOCON_R_D_SMPL_SEL     /* RW */	/* Alias New definition of MSDC_CODA_SD30_v2.13 */
#define MSDC_IOCON_DSPLSEL      MSDC_IOCON_R_D_SMPL_SEL     /* Alias */

#define MSDC_IOCON_W_D_SMPL     (0x1   <<  8)     /* RW */
#define MSDC_IOCON_W_DSPL        MSDC_IOCON_W_D_SMPL     /* Alias */


#define MSDC_IOCON_W_D_SMPL_SEL (0x1   <<  9)     /* RW */
#define MSDC_IOCON_WDSPLSEL     MSDC_IOCON_W_D_SMPL_SEL     /* RW */	/* Alias New definition of MSDC_CODA_SD30_v2.13 */
#define MSDC_IOCON_W_D0SPL      (0x1   << 10)     /* RW */
#define MSDC_IOCON_W_D1SPL      (0x1   << 11)     /* RW */
#define MSDC_IOCON_W_D2SPL      (0x1   << 12)     /* RW */
#define MSDC_IOCON_W_D3SPL      (0x1   << 13)     /* RW */

#define MSDC_IOCON_R_D0SPL      (0x1   << 16)     /* RW */
#define MSDC_IOCON_R_D1SPL      (0x1   << 17)     /* RW */
#define MSDC_IOCON_R_D2SPL      (0x1   << 18)     /* RW */
#define MSDC_IOCON_R_D3SPL      (0x1   << 19)     /* RW */
#define MSDC_IOCON_R_D4SPL      (0x1   << 20)     /* RW */
#define MSDC_IOCON_R_D5SPL      (0x1   << 21)     /* RW */
#define MSDC_IOCON_R_D6SPL      (0x1   << 22)     /* RW */
#define MSDC_IOCON_R_D7SPL      (0x1   << 23)     /* RW */
//#define MSDC_IOCON_RISCSZ       (0x3   << 24)     /* RW  !!! MT7623  remove*/

/* MSDC_PS mask */
#define MSDC_PS_CDEN            (0x1   <<  0)     /* RW */
#define MSDC_PS_CDSTS           (0x1   <<  1)     /* RU  */

#define MSDC_PS_CDDEBOUNCE      (0xF   << 12)     /* RW */
#define MSDC_PS_DAT             (0xFF  << 16)     /* RU */
#define MSDC_PS_DAT8PIN         (0xFF  << 16)     /* RU */
#define MSDC_PS_DAT4PIN         (0xF   << 16)     /* RU */
#define MSDC_PS_DAT0            (0x1   << 16)     /* RU */

#define MSDC_PS_CMD             (0x1   << 24)     /* RU */

#define MSDC_PS_WP              (0x1   << 31)     /* RU  */

/* MSDC_INT mask */
#define MSDC_INT_MMCIRQ         (0x1   <<  0)     /* W1C */
#define MSDC_INT_CDSC           (0x1   <<  1)     /* W1C */

#define MSDC_INT_ACMDRDY        (0x1   <<  3)     /* W1C */
#define MSDC_INT_ACMDTMO        (0x1   <<  4)     /* W1C */
#define MSDC_INT_ACMDCRCERR     (0x1   <<  5)     /* W1C */
#define MSDC_INT_DMAQ_EMPTY     (0x1   <<  6)     /* W1C */
#define MSDC_INT_SDIOIRQ        (0x1   <<  7)     /* W1C Only for SD/SDIO */
#define MSDC_INT_CMDRDY         (0x1   <<  8)     /* W1C */
#define MSDC_INT_CMDTMO         (0x1   <<  9)     /* W1C */
#define MSDC_INT_RSPCRCERR      (0x1   << 10)     /* W1C */
#define MSDC_INT_CSTA           (0x1   << 11)     /* R */
#define MSDC_INT_XFER_COMPL     (0x1   << 12)     /* W1C */
#define MSDC_INT_DXFER_DONE     (0x1   << 13)     /* W1C */
#define MSDC_INT_DATTMO         (0x1   << 14)     /* W1C */
#define MSDC_INT_DATCRCERR      (0x1   << 15)     /* W1C */
#define MSDC_INT_ACMD19_DONE    (0x1   << 16)     /* W1C */
#define MSDC_INT_BDCSERR        (0x1   << 17)     /* W1C */
#define MSDC_INT_GPDCSERR       (0x1   << 18)     /* W1C */
#define MSDC_INT_DMAPRO         (0x1   << 19)     /* W1C */
#define MSDC_INT_GOBOUND        (0x1   << 20)     /* W1C Only for SD/SDIO ACMD 53*/
#define MSDC_INT_GEAR_OUT_BOUND MSDC_INT_GOBOUND  /* Alias */

#define MSDC_INT_ACMD53_DONE    (0x1   << 21)     /* W1C Only for SD/SDIO ACMD 53*/
#define MSDC_INT_ACMD53_FAIL    (0x1   << 22)     /* W1C Only for SD/SDIO ACMD 53*/
#define MSDC_INT_AXI_RESP_ERR   (0x1   << 23)     /* W1C Only for eMMC 5.0*/

/* MSDC_INTEN mask */
#define MSDC_INTEN_MMCIRQ       (0x1   <<  0)     /* RW */
#define MSDC_INTEN_CDSC         (0x1   <<  1)     /* RW */

#define MSDC_INTEN_ACMDRDY      (0x1   <<  3)     /* RW */
#define MSDC_INTEN_ACMDTMO      (0x1   <<  4)     /* RW */
#define MSDC_INTEN_ACMDCRCERR   (0x1   <<  5)     /* RW */
#define MSDC_INTEN_DMAQ_EMPTY   (0x1   <<  6)     /* RW */
#define MSDC_INTEN_SDIOIRQ      (0x1   <<  7)     /* RW Only for SDIO*/
#define MSDC_INTEN_CMDRDY       (0x1   <<  8)     /* RW */
#define MSDC_INTEN_CMDTMO       (0x1   <<  9)     /* RW */
#define MSDC_INTEN_RSPCRCERR    (0x1   << 10)     /* RW */
#define MSDC_INTEN_CSTA         (0x1   << 11)     /* RW */
#define MSDC_INTEN_XFER_COMPL   (0x1   << 12)     /* RW */
#define MSDC_INTEN_DXFER_DONE   (0x1   << 13)     /* RW */
#define MSDC_INTEN_DATTMO       (0x1   << 14)     /* RW */
#define MSDC_INTEN_DATCRCERR    (0x1   << 15)     /* RW */
#define MSDC_INTEN_ACMD19_DONE  (0x1   << 16)     /* RW */
#define MSDC_INTEN_BDCSERR      (0x1   << 17)     /* RW */
#define MSDC_INTEN_GPDCSERR     (0x1   << 18)     /* RW */
#define MSDC_INTEN_DMAPRO       (0x1   << 19)     /* RW */
#define MSDC_INTEN_GOBOUND      (0x1   << 20)     /* RW  Only for SD/SDIO ACMD 53*/
#define MSDC_INTEN_ACMD53_DONE  (0x1   << 21)     /* RW  Only for SD/SDIO ACMD 53*/
#define MSDC_INTEN_ACMD53_FAIL  (0x1   << 22)     /* RW  Only for SD/SDIO ACMD 53*/
#define MSDC_INTEN_AXI_RESP_ERR (0x1   << 23)     /* RW  Only for eMMC 5.0*/

#define MSDC_INTEN_DFT       (  MSDC_INTEN_MMCIRQ        |MSDC_INTEN_CDSC        | MSDC_INTEN_ACMDRDY\
                                |MSDC_INTEN_ACMDTMO      |MSDC_INTEN_ACMDCRCERR  | MSDC_INTEN_DMAQ_EMPTY /*|MSDC_INTEN_SDIOIRQ*/\
                                |MSDC_INTEN_CMDRDY       |MSDC_INTEN_CMDTMO      | MSDC_INTEN_RSPCRCERR   |MSDC_INTEN_CSTA\
                                |MSDC_INTEN_XFER_COMPL   |MSDC_INTEN_DXFER_DONE  | MSDC_INTEN_DATTMO      |MSDC_INTEN_DATCRCERR\
                                |MSDC_INTEN_BDCSERR      |MSDC_INTEN_ACMD19_DONE | MSDC_INTEN_GPDCSERR     /*|MSDC_INTEN_DMAPRO*/\
                                /*|MSDC_INTEN_GOBOUND   |MSDC_INTEN_ACMD53_DONE |MSDC_INTEN_ACMD53_FAIL |MSDC_INTEN_AXI_RESP_ERR*/)


/* MSDC_FIFOCS mask */
#define MSDC_FIFOCS_RXCNT       (0xFF  <<  0)     /* R */
#define MSDC_FIFOCS_TXCNT       (0xFF  << 16)     /* R */
#define MSDC_FIFOCS_CLR         (0x1   << 31)     /* RW */

/* SDC_CFG mask */
#define SDC_CFG_SDIOINTWKUP     (0x1   <<  0)     /* RW */
#define SDC_CFG_INSWKUP         (0x1   <<  1)     /* RW */
#define SDC_CFG_BUSWIDTH        (0x3   << 16)     /* RW */
#define SDC_CFG_SDIO            (0x1   << 19)     /* RW */
#define SDC_CFG_SDIOIDE         (0x1   << 20)     /* RW */
#define SDC_CFG_INTATGAP        (0x1   << 21)     /* RW */
#define SDC_CFG_DTOC            (0xFF  << 24)     /* RW */

/* SDC_CMD mask */
#define SDC_CMD_OPC             (0x3F  <<  0)     /* RW */
#define SDC_CMD_BRK             (0x1   <<  6)     /* RW */
#define SDC_CMD_RSPTYP          (0x7   <<  7)     /* RW */
#define SDC_CMD_DTYP            (0x3   << 11)     /* RW */
#define SDC_CMD_RW              (0x1   << 13)     /* RW */
#define SDC_CMD_STOP            (0x1   << 14)     /* RW */
#define SDC_CMD_GOIRQ           (0x1   << 15)     /* RW */
#define SDC_CMD_BLKLEN          (0xFFF << 16)     /* RW */
#define SDC_CMD_AUTOCMD         (0x3   << 28)     /* RW */
#define SDC_CMD_VOLSWTH         (0x1   << 30)     /* RW */
#define SDC_CMD_ACMD53          (0x1   << 31)     /* RW Only for SD/SDIO ACMD 53*/

/* SDC_STS mask */
#define SDC_STS_SDCBUSY         (0x1   <<  0)     /* RW */
#define SDC_STS_CMDBUSY         (0x1   <<  1)     /* RW */
#define SDC_STS_CMD_WR_BUSY     (0x1   << 16)     /* RW !!! MT7623  Add*/
#define SDC_STS_SWR_COMPL       (0x1   << 31)     /* RW */

/* SDC_VOL_CHG mask */
#define SDC_VOL_CHG_VCHGCNT     (0xFFFF<<  0)     /* RW  !!! MT7623  Add*/
/* SDC_DCRC_STS mask */
#define SDC_DCRC_STS_POS        (0xFF  <<  0)     /* RO */
#define SDC_DCRC_STS_NEG        (0xFF  <<  8)     /* RO */

/* EMMC_CFG0 mask */
#define EMMC_CFG0_BOOTSTART     (0x1   <<  0)     /* WO Only for eMMC */
#define EMMC_CFG0_BOOTSTOP      (0x1   <<  1)     /* WO Only for eMMC */
#define EMMC_CFG0_BOOTMODE      (0x1   <<  2)     /* RW Only for eMMC */
#define EMMC_CFG0_BOOTACKDIS    (0x1   <<  3)     /* RW Only for eMMC */

#define EMMC_CFG0_BOOTWDLY      (0x7   << 12)     /* RW Only for eMMC */
#define EMMC_CFG0_BOOTSUPP      (0x1   << 15)     /* RW Only for eMMC */

/* EMMC_CFG1 mask */
#define EMMC_CFG1_BOOTDATTMC   (0xFFFFF<<  0)     /* RW Only for eMMC */
#define EMMC_CFG1_BOOTACKTMC    (0xFFF << 20)     /* RW Only for eMMC */

/* EMMC_STS mask */
#define EMMC_STS_BOOTCRCERR     (0x1   <<  0)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTACKERR     (0x1   <<  1)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTDATTMO     (0x1   <<  2)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTACKTMO     (0x1   <<  3)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTUPSTATE    (0x1   <<  4)     /* RU Only for eMMC */
#define EMMC_STS_BOOTACKRCV     (0x1   <<  5)     /* W1C Only for eMMC */
#define EMMC_STS_BOOTDATRCV     (0x1   <<  6)     /* RU Only for eMMC */

/* EMMC_IOCON mask */
#define EMMC_IOCON_BOOTRST      (0x1   <<  0)     /* RW Only for eMMC */

/* SDC_ACMD19_TRG mask */
#define SDC_ACMD19_TRG_TUNESEL  (0xF   <<  0)     /* RW */

/* DMA_SA_HIGH4BIT mask */
#define DMA_SA_HIGH4BIT_L4BITS  (0xF   <<  0)     /* RW  !!! MT7623  Add*/
/* MSDC_DMA_CTRL mask */
#define MSDC_DMA_CTRL_START     (0x1   <<  0)     /* WO */
#define MSDC_DMA_CTRL_STOP      (0x1   <<  1)     /* AO */
#define MSDC_DMA_CTRL_RESUME    (0x1   <<  2)     /* WO */
#define MSDC_DMA_CTRL_READYM    (0x1   <<  3)     /* RO  !!! MT7623  Add*/

#define MSDC_DMA_CTRL_MODE      (0x1   <<  8)     /* RW */
#define MSDC_DMA_CTRL_ALIGN     (0x1   <<  9)     /* RW !!! MT7623  Add*/
#define MSDC_DMA_CTRL_LASTBUF   (0x1   << 10)     /* RW */
#define MSDC_DMA_CTRL_SPLIT1K   (0x1   << 11)     /* RW !!! MT7623  Add*/
#define MSDC_DMA_CTRL_BRUSTSZ   (0x7   << 12)     /* RW */
// #define MSDC_DMA_CTRL_XFERSZ    (0xffffUL << 16)/* RW */

/* MSDC_DMA_CFG mask */
#define MSDC_DMA_CFG_STS              (0x1  <<  0)     /* R */
#define MSDC_DMA_CFG_DECSEN           (0x1  <<  1)     /* RW */
#define MSDC_DMA_CFG_LOCKDISABLE      (0x1  <<  2)     /* RW !!! MT7623  Add*/
//#define MSDC_DMA_CFG_BDCSERR        (0x1  <<  4)     /* R */
//#define MSDC_DMA_CFG_GPDCSERR       (0x1  <<  5)     /* R */
#define MSDC_DMA_CFG_AHBEN            (0x3  <<  8)     /* RW */
#define MSDC_DMA_CFG_ACTEN            (0x3  << 12)     /* RW */

#define MSDC_DMA_CFG_CS12B            (0x1  << 16)     /* RW */
#define MSDC_DMA_CFG_OUTB_STOP        (0x1  << 17)     /* RW */

/* MSDC_PATCH_BIT0 mask */
//#define MSDC_PB0_RESV1              (0x1  <<  0)
#define MSDC_PB0_EN_8BITSUP           (0x1  <<  1)    /* RW */
#define MSDC_PB0_DIS_RECMDWR          (0x1  <<  2)    /* RW */
//#define MSDC_PB0_RESV2                        (0x1  <<  3)
#define MSDC_PB0_RDDATSEL             (0x1  <<  3)    /* RW !!! MT7623 Add for SD/SDIO/eMMC 4.5*/
#define MSDC_PB0_ACMD53_CRCINTR       (0x1  <<  4)    /* RW !!! MT7623 Add only for SD/SDIO */
#define MSDC_MASK_ACMD53_CRC_ERR_INTR MSDC_PB0_ACMD53_CRCINTR /* Alias */

#define MSDC_PB0_ACMD53_ONESHOT       (0x1  <<  5)    /* RW !!! MT7623 Add ony for SD/SDIO */
#define MSDC_ACMD53_FAIL_ONE_SHOT     MSDC_PB0_ACMD53_ONESHOT /* Alias */

//#define MSDC_PB0_RESV3                        (0x1  <<  6)
#define MSDC_PB0_DESC_UP_SEL          (0x1  <<  6)    /* RW !!! MT7623  Add*/
#define MSDC_PB0_INT_DAT_LATCH_CK_SEL (0x7  <<  7)    /* RW */
#define MSDC_INT_DAT_LATCH_CK_SEL      MSDC_PB0_INT_DAT_LATCH_CK_SEL  /* alias */

#define MSDC_PB0_CKGEN_MSDC_DLY_SEL   (0x1F << 10)    /* RW */
#define MSDC_CKGEN_MSDC_DLY_SEL        MSDC_PB0_CKGEN_MSDC_DLY_SEL  /* alias */


#define MSDC_PB0_FIFORD_DIS           (0x1  << 15)    /* RW */
//#define MSDC_PB0_SDIO_DBSSEL                  (0x1  << 16)    /* RW !!! MT7623  change*/
#define MSDC_PB0_MSDC_BLKNUMSEL       (0x1  << 16)    /* RW !!! MT7623  change ACMD23*/
#define MSDC_PB0_BLKNUM_SEL           MSDC_PB0_MSDC_BLKNUMSEL /* alias */

#define MSDC_PB0_SDIO_INTCSEL         (0x1  << 17)    /* RW */
#define MSDC_PB0_SDIO_BSYDLY          (0xF  << 18)    /* RW */
#define MSDC_PB0_SDC_WDOD             (0xF  << 22)    /* RW */
#define MSDC_PB0_CMDIDRTSEL           (0x1  << 26)    /* RW */
#define MSDC_PB0_CMDFAILSEL           (0x1  << 27)    /* RW */
#define MSDC_PB0_SDIO_INTDLYSEL       (0x1  << 28)    /* RW */
#define MSDC_PB0_SPCPUSH              (0x1  << 29)    /* RW */
#define MSDC_PB0_DETWR_CRCTMO         (0x1  << 30)    /* RW */
#define MSDC_PB0_EN_DRVRSP            (0x1  << 31)    /* RW */

/* MSDC_PATCH_BIT1 mask */
#define MSDC_PB1_WRDAT_CRCS_TA_CNTR   (0x7  <<  0)    /* RW */
#define MSDC_PATCH_BIT1_WRDAT_CRCS    MSDC_PB1_WRDAT_CRCS_TA_CNTR /* alias */


#define MSDC_PB1_CMD_RSP_TA_CNTR      (0x7  <<  3)    /* RW */
#define MSDC_PATCH_BIT1_CMD_RSP       MSDC_PB1_CMD_RSP_TA_CNTR  /* alias */

//#define MSDC_PB1_RESV3                        (0x3  <<  6)
#define MSDC_PB1_GET_BUSY_MARGIN      (0x1  <<  6)    /* RW !!! MT7623  Add */
//#define MSDC_PB1_GET_BUSY_MA          MSDC_PB1_GET_BUSY_MARGIN /*Alias */

#define MSDC_PB1_GET_CRC_MARGIN       (0x1  <<  7)    /* RW !!! MT7623  Add */
//#define MSDC_PB1_GET_CRC_MA           MSDC_PB1_GET_CRC_MARGIN /*Alias */

#define MSDC_PB1_BIAS_TUNE_28NM       (0xF  <<  8)    /* RW */
#define MSDC_PB1_BIAS_EN18IO_28NM     (0x1  << 12)    /* RW */
#define MSDC_PB1_BIAS_EXT_28NM        (0x1  << 13)    /* RW */

//#define MSDC_PB1_RESV2                        (0x3  << 14)
#define MSDC_PB1_RESET_GDMA           (0x1  << 15)    /* RW !!! MT7623  Add */
//#define MSDC_PB1_RESV1                        (0x7F << 16)
#define MSDC_PB1_EN_SINGLE_BURST      (0x1  << 16)    /* RW !!! MT7623  Add */
#define MSDC_PB1_EN_FORCE_STOP_GDMA   (0x1  << 17)    /* RW !!! MT7623  Add  for eMMC 5.0 only*/
#define MSDC_PB1_DCM_DIV_SEL2         (0x3  << 18)    /* RW !!! MT7623  Add  for eMMC 5.0 only*/
#define MSDC_PB1_DCM_DIV_SEL1         (0x1  << 20)    /* RW !!! MT7623  Add */
#define MSDC_PB1_DCM_EN               (0x1  << 21)    /* RW !!! MT7623  Add */
#define MSDC_PB1_AXI_WRAP_CKEN        (0x1  << 22)    /* RW !!! MT7623  Add for eMMC 5.0 only*/
#define MSDC_PB1_AHBCKEN              (0x1  << 23)    /* RW */
#define MSDC_PB1_CKSPCEN              (0x1  << 24)    /* RW */
#define MSDC_PB1_CKPSCEN              (0x1  << 25)    /* RW */
#define MSDC_PB1_CKVOLDETEN           (0x1  << 26)    /* RW */
#define MSDC_PB1_CKACMDEN             (0x1  << 27)    /* RW */
#define MSDC_PB1_CKSDEN               (0x1  << 28)    /* RW */
#define MSDC_PB1_CKWCTLEN             (0x1  << 29)    /* RW */
#define MSDC_PB1_CKRCTLEN             (0x1  << 30)    /* RW */
#define MSDC_PB1_CKSHBFFEN            (0x1  << 31)    /* RW */

/* MSDC_PATCH_BIT2 mask */
#define MSDC_PB2_ENHANCEGPD           (0x1  <<  0)    /* RW !!! MT7623  Add */
#define MSDC_PB2_SUPPORT64G           (0x1  <<  1)    /* RW !!! MT7623  Add */
#define MSDC_PB2_RESPWAIT             (0x3  <<  2)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGRDATCNT           (0x1F <<  4)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGRDAT              (0x1  <<  9)    /* RW !!! MT7623  Add */

#define MSDC_PB2_INTCRESPSEL          (0x1  << 11)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGRESPCNT           (0x7  << 12)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGRESP              (0x1  << 15)    /* RW !!! MT7623  Add */
#define MSDC_PB2_RESPSTSENSEL         (0x7  << 16)    /* RW !!! MT7623  Add */

#define MSDC_PB2_POPENCNT             (0xF  << 20)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGCRCSTSSEL         (0x1  << 24)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGCRCSTSEDGE        (0x1  << 25)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGCRCSTSCNT         (0x3  << 26)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CFGCRCSTS            (0x1  << 28)    /* RW !!! MT7623  Add */
#define MSDC_PB2_CRCSTSENSEL          (0x7  << 29)    /* RW !!! MT7623  Add */

/* SDIO_TUNE_WIND mask */
#define SDIO_TUNE_WIND_TUNEWINDOW     (0x1F  <<  0)     /* RW !!! MT7623  Add for SD/SDIO only*/

/* MSDC_PAD_TUNE/MSDC_PAD_TUNE0 mask */
#define MSDC_PAD_TUNE_DATWRDLY        (0x1F  <<  0)     /* RW */

#define MSDC_PAD_TUNE_DELAYEN         (0x1   <<  7)     /* RW !!! MT7623  Add*/
#define MSDC_PAD_TUNE_DATRRDLY        (0x1F  <<  8)     /* RW */
#define MSDC_PAD_TUNE_DATRRDLYSEL     (0x1   << 13)     /* RW !!! MT7623  Add*/

#define MSDC_PAD_TUNE_RXDLYSEL        (0x1   << 15)     /* RW !!! MT7623  Add*/
#define MSDC_PAD_TUNE_CMDRDLY         (0x1F  << 16)     /* RW */
#define MSDC_PAD_TUNE_CMDRDLYSEL      (0x1   << 21)     /* RW !!! MT7623  Add*/
#define MSDC_PAD_TUNE_CMDRRDLY        (0x1F  << 22)     /* RW */
#define MSDC_PAD_TUNE_CLKTXDLY        (0x1F  << 27)     /* RW */

/* MSDC_PAD_TUNE1 mask */

#define MSDC_PAD_TUNE1_DATRRDLY2      (0x1F  <<  8)     /* RW  !!! MT7623  Add*/
#define MSDC_PAD_TUNE1_DATRDLY2SEL    (0x1   << 13)     /* RW  !!! MT7623  Add*/

#define MSDC_PAD_TUNE1_CMDRDLY2       (0x1F  << 16)     /* RW  !!! MT7623  Add*/
#define MSDC_PAD_TUNE1_CMDRDLY2SEL    (0x1   << 21)     /* RW  !!! MT7623  Add*/


/* MSDC_DAT_RDDLY0 mask */
#define MSDC_DAT_RDDLY0_D3            (0x1F  <<  0)     /* RW */
#define MSDC_DAT_RDDLY0_D2            (0x1F  <<  8)     /* RW */
#define MSDC_DAT_RDDLY0_D1            (0x1F  << 16)     /* RW */
#define MSDC_DAT_RDDLY0_D0            (0x1F  << 24)     /* RW */

/* MSDC_DAT_RDDLY1 mask */
#define MSDC_DAT_RDDLY1_D7            (0x1F  <<  0)     /* RW */

#define MSDC_DAT_RDDLY1_D6            (0x1F  <<  8)     /* RW */

#define MSDC_DAT_RDDLY1_D5            (0x1F  << 16)     /* RW */

#define MSDC_DAT_RDDLY1_D4            (0x1F  << 24)     /* RW */

/* MSDC_DAT_RDDLY2 mask */
#define MSDC_DAT_RDDLY2_D3            (0x1F  <<  0)     /* RW !!! MT7623  Add*/

#define MSDC_DAT_RDDLY2_D2            (0x1F  <<  8)     /* RW !!! MT7623  Add*/

#define MSDC_DAT_RDDLY2_D1            (0x1F  << 16)     /* RW !!! MT7623  Add*/

#define MSDC_DAT_RDDLY2_D0            (0x1F  << 24)     /* RW !!! MT7623  Add*/

/* MSDC_DAT_RDDLY3 mask */
#define MSDC_DAT_RDDLY3_D7            (0x1F  <<  0)     /* RW !!! MT7623  Add*/

#define MSDC_DAT_RDDLY3_D6            (0x1F  <<  8)     /* RW !!! MT7623  Add*/

#define MSDC_DAT_RDDLY3_D5            (0x1F  << 16)     /* RW !!! MT7623  Add*/

#define MSDC_DAT_RDDLY3_D4            (0x1F  << 24)     /* RW !!! MT7623  Add*/

/* MSDC_HW_DBG_SEL mask */
#define MSDC_HW_DBG0_SEL              (0xFF  <<  0)     /* RW DBG3->DBG0 !!! MT7623  Change*/
#define MSDC_HW_DBG1_SEL              (0x3F  <<  8)     /* RW DBG2->DBG1 !!! MT7623  Add*/

#define MSDC_HW_DBG2_SEL              (0xFF  << 16)     /* RW DBG1->DBG2 !!! MT7623  Add*/
//#define MSDC_HW_DBG_WRAPTYPE_SEL    (0x3   << 22)           /* RW !!! MT7623  Removed*/
#define MSDC_HW_DBG3_SEL              (0x3F  << 24)     /* RW DBG0->DBG3 !!! MT7623  Add*/
#define MSDC_HW_DBG_WRAP_SEL          (0x1   << 30)     /* RW */


/* MSDC_EMMC50_PAD_CTL0 mask*/
#define MSDC_EMMC50_PAD_CTL0_DCCSEL   (0x1  <<  0)     /* RW */
#define MSDC_EMMC50_PAD_CTL0_HLSEL    (0x1  <<  1)     /* RW */
#define MSDC_EMMC50_PAD_CTL0_DLP0     (0x3  <<  2)     /* RW */
#define MSDC_EMMC50_PAD_CTL0_DLN0     (0x3  <<  4)     /* RW */
#define MSDC_EMMC50_PAD_CTL0_DLP1     (0x3  <<  6)     /* RW */
#define MSDC_EMMC50_PAD_CTL0_DLN1     (0x3  <<  8)     /* RW */

/* MSDC_EMMC50_PAD_DS_CTL0 mask */
#define MSDC_EMMC50_PAD_DS_CTL0_SR    (0x1  <<  0)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_R0    (0x1  <<  1)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_R1    (0x1  <<  2)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_PUPD  (0x1  <<  3)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_IES   (0x1  <<  4)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_SMT   (0x1  <<  5)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_RDSEL (0x3F <<  6)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_TDSEL (0xF  << 12)     /* RW */
#define MSDC_EMMC50_PAD_DS_CTL0_DRV   (0x7  << 16)     /* RW */


/* EMMC50_PAD_DS_TUNE mask */
#define MSDC_EMMC50_PAD_DS_TUNE_DLYSEL  (0x1  <<  0)  /* RW */
#define MSDC_EMMC50_PAD_DS_TUNE_DLY2SEL (0x1  <<  1)  /* RW */
#define MSDC_EMMC50_PAD_DS_TUNE_DLY1    (0x1F <<  2)  /* RW */
#define MSDC_EMMC50_PAD_DS_TUNE_DLY2    (0x1F <<  7)  /* RW */
#define MSDC_EMMC50_PAD_DS_TUNE_DLY3    (0x1F << 12)  /* RW */

/* EMMC50_PAD_CMD_TUNE mask */
#define MSDC_EMMC50_PAD_CMD_TUNE_DLY3SEL (0x1  <<  0)  /* RW */
#define MSDC_EMMC50_PAD_CMD_TUNE_RXDLY3  (0x1F <<  1)  /* RW */
#define MSDC_EMMC50_PAD_CMD_TUNE_TXDLY   (0x1F <<  6)  /* RW */

/* EMMC50_PAD_DAT01_TUNE mask */
#define MSDC_EMMC50_PAD_DAT0_RXDLY3SEL   (0x1  <<  0)  /* RW */
#define MSDC_EMMC50_PAD_DAT0_RXDLY3      (0x1F <<  1)  /* RW */
#define MSDC_EMMC50_PAD_DAT0_TXDLY       (0x1F <<  6)  /* RW */
#define MSDC_EMMC50_PAD_DAT1_RXDLY3SEL   (0x1  << 16)  /* RW */
#define MSDC_EMMC50_PAD_DAT1_RXDLY3      (0x1F << 17)  /* RW */
#define MSDC_EMMC50_PAD_DAT1_TXDLY       (0x1F << 22)  /* RW */

/* EMMC50_PAD_DAT23_TUNE mask */
#define MSDC_EMMC50_PAD_DAT2_RXDLY3SEL   (0x1  <<  0)  /* RW */
#define MSDC_EMMC50_PAD_DAT2_RXDLY3      (0x1F <<  1)  /* RW */
#define MSDC_EMMC50_PAD_DAT2_TXDLY       (0x1F <<  6)  /* RW */
#define MSDC_EMMC50_PAD_DAT3_RXDLY3SEL   (0x1  << 16)  /* RW */
#define MSDC_EMMC50_PAD_DAT3_RXDLY3      (0x1F << 17)  /* RW */
#define MSDC_EMMC50_PAD_DAT3_TXDLY       (0x1F << 22)  /* RW */

/* EMMC50_PAD_DAT45_TUNE mask */
#define MSDC_EMMC50_PAD_DAT4_RXDLY3SEL   (0x1  <<  0)  /* RW */
#define MSDC_EMMC50_PAD_DAT4_RXDLY3      (0x1F <<  1)  /* RW */
#define MSDC_EMMC50_PAD_DAT4_TXDLY       (0x1F <<  6)  /* RW */
#define MSDC_EMMC50_PAD_DAT5_RXDLY3SEL   (0x1  << 16)  /* RW */
#define MSDC_EMMC50_PAD_DAT5_RXDLY3      (0x1F << 17)  /* RW */
#define MSDC_EMMC50_PAD_DAT5_TXDLY       (0x1F << 22)  /* RW */

/* EMMC50_PAD_DAT67_TUNE mask */
#define MSDC_EMMC50_PAD_DAT6_RXDLY3SEL   (0x1  <<  0)  /* RW */
#define MSDC_EMMC50_PAD_DAT6_RXDLY3      (0x1F <<  1)  /* RW */
#define MSDC_EMMC50_PAD_DAT6_TXDLY       (0x1F <<  6)  /* RW */
#define MSDC_EMMC50_PAD_DAT7_RXDLY3SEL   (0x1  << 16)  /* RW */
#define MSDC_EMMC50_PAD_DAT7_RXDLY3      (0x1F << 17)  /* RW */
#define MSDC_EMMC50_PAD_DAT7_TXDLY       (0x1F << 22)  /* RW */

/* EMMC51_CFG0 mask */
#define MSDC_EMMC51_CFG_CMDQ_EN          (0x1   <<  0) /* RW !!! MT7623  Add*/
#define MSDC_EMMC51_CFG_WDAT_CNT         (0x3FF <<  1) /* RW !!! MT7623  Add*/
#define MSDC_EMMC51_CFG_RDAT_CNT         (0x3FF << 11) /* RW !!! MT7623  Add*/
#define MSDC_EMMC51_CFG_CMDQ_CMD_EN      (0x1   << 21) /* RW !!! MT7623  Add*/


/* EMMC50_CFG0 mask */
#define MSDC_EMMC50_CFG_PADCMD_LATCHCK         (0x1  <<  0)  /* RW*/
#define MSDC_EMMC50_CFG_CRCSTS_CNT             (0x3  <<  1)  /* RW*/
#define MSDC_EMMC50_CFG_CRCSTS_EDGE            (0x1  <<  3)  /* RW*/
#define MSDC_EMMC50_CFG_CRC_STS_EDGE           MSDC_EMMC50_CFG_CRCSTS_EDGE /*alias */

#define MSDC_EMMC50_CFG_CRCSTS_SEL             (0x1  <<  4)  /* RW*/
#define MSDC_EMMC50_CFG_CRC_STS_SEL            MSDC_EMMC50_CFG_CRCSTS_SEL /*alias */

#define MSDC_EMMC50_CFG_ENDBIT_CHKCNT          (0xF  <<  5)  /* RW*/
#define MSDC_EMMC50_CFG_CMDRSP_SEL             (0x1  <<  9)  /* RW*/
#define MSDC_EMMC50_CFG_CMD_RESP_SEL           MSDC_EMMC50_CFG_CMDRSP_SEL  /*alias */

#define MSDC_EMMC50_CFG_CMDEDGE_SEL            (0x1  << 10)  /* RW*/
#define MSDC_EMMC50_CFG_ENDBIT_CNT             (0x3FF<< 11)  /* RW*/
#define MSDC_EMMC50_CFG_READDAT_CNT            (0x7  << 21)  /* RW*/
#define MSDC_EMMC50_CFG_EMMC50_MONSEL          (0x1  << 24)  /* RW*/
#define MSDC_EMMC50_CFG_MSDC_WRVALID           (0x1  << 25)  /* RW*/
#define MSDC_EMMC50_CFG_MSDC_RDVALID           (0x1  << 26)  /* RW*/
#define MSDC_EMMC50_CFG_MSDC_WRVALID_SEL       (0x1  << 27)  /* RW*/
#define MSDC_EMMC50_CFG_MSDC_RDVALID_SEL       (0x1  << 28)  /* RW*/
#define MSDC_EMMC50_CFG_MSDC_TXSKEW_SEL        (0x1  << 29)  /* RW*/
//#define MSDC_EMMC50_CFG_MSDC_GDMA_RESET      (0x1  << 31)  /* RW !!! MT7623  Removed*/

/* EMMC50_CFG1 mask */
#define MSDC_EMMC50_CFG1_WRPTR_MARGIN          (0xFF <<  0)  /* RW*/
#define MSDC_EMMC50_CFG1_CKSWITCH_CNT          (0x7  <<  8)  /* RW*/
#define MSDC_EMMC50_CFG1_RDDAT_STOP            (0x1  << 11)  /* RW*/
#define MSDC_EMMC50_CFG1_WAIT8CLK_CNT          (0xF  << 12)  /* RW*/
#define MSDC_EMMC50_CFG1_EMMC50_DBG_SEL        (0xFF << 16)  /* RW*/
#define MSDC_EMMC50_CFG1_PSH_CNT               (0x7  << 24)  /* RW !!! MT7623  Add*/
#define MSDC_EMMC50_CFG1_PSH_PS_SEL            (0x1  << 27)  /* RW !!! MT7623  Add*/
#define MSDC_EMMC50_CFG1_DS_CFG                (0x1  << 28)  /* RW !!! MT7623  Add*/

/* EMMC50_CFG2 mask */
//#define MSDC_EMMC50_CFG2_AXI_GPD_UP          (0x1  << 0)  /* RW !!! MT7623  Removed*/
#define MSDC_EMMC50_CFG2_AXI_IOMMU_WR_EMI      (0x1  <<  1) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_SHARE_EN_WR_EMI   (0x1  <<  2) /* RW*/

#define MSDC_EMMC50_CFG2_AXI_IOMMU_RD_EMI      (0x1  <<  7) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_SHARE_EN_RD_EMI   (0x1  <<  8) /* RW*/

#define MSDC_EMMC50_CFG2_AXI_BOUND_128B        (0x1  << 13) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_BOUND_256B        (0x1  << 14) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_BOUND_512B        (0x1  << 15) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_BOUND_1K          (0x1  << 16) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_BOUND_2K          (0x1  << 17) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_BOUND_4K          (0x1  << 18) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_RD_OUTSTANDING_NUM (0x1F << 19) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_RD_OUTS_NUM       MSDC_EMMC50_CFG2_AXI_RD_OUTSTANDING_NUM /*alias */

#define MSDC_EMMC50_CFG2_AXI_SET_LET           (0xF  << 24) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_SET_LEN           MSDC_EMMC50_CFG2_AXI_SET_LET /*alias */

#define MSDC_EMMC50_CFG2_AXI_RESP_ERR_TYPE     (0x3  << 28) /* RW*/
#define MSDC_EMMC50_CFG2_AXI_BUSY              (0x1  << 30) /* RW*/


/* EMMC50_CFG3 mask */
#define MSDC_EMMC50_CFG3_OUTSTANDING_WR        (0x1F <<  0) /* RW*/
#define MSDC_EMMC50_CFG3_OUTS_WR               MSDC_EMMC50_CFG3_OUTSTANDING_WR /*alias */

#define MSDC_EMMC50_CFG3_ULTRA_SET_WR          (0x3F <<  5) /* RW*/
#define MSDC_EMMC50_CFG3_PREULTRA_SET_WR       (0x3F << 11) /* RW*/
#define MSDC_EMMC50_CFG3_ULTRA_SET_RD          (0x3F << 17) /* RW*/
#define MSDC_EMMC50_CFG3_PREULTRA_SET_RD       (0x3F << 23) /* RW*/

/* EMMC50_CFG4 mask */
#define MSDC_EMMC50_CFG4_IMPR_ULTRA_SET_WR     (0xFF <<  0) /* RW*/
#define MSDC_EMMC50_CFG4_IMPR_ULTRA_SET_RD     (0xFF <<  8) /* RW*/
#define MSDC_EMMC50_CFG4_ULTRA_EN              (0x3  << 16) /* RW*/
#define MSDC_EMMC50_CFG4_WRAP_SEL              (0x1F << 18) /* RW !!! MT7623  Add*/

/* EMMC50_BLOCK_LENGTH mask */
#define EMMC50_BLOCK_LENGTH_BLKLENGTH          (0xFF <<  0) /* RW !!! MT7623  Add*/

/* MSDC_CFG[START_BIT] value */
#define START_AT_RISING             (0x0)
#define START_AT_FALLING            (0x1)
#define START_AT_RISING_AND_FALLING (0x2)
#define START_AT_RISING_OR_FALLING  (0x3)

#define MSDC_SMPL_RISING        (0)
#define MSDC_SMPL_FALLING       (1)
#define MSDC_SMPL_SEPERATE      (2)


#define TYPE_CMD_RESP_EDGE      (0)
#define TYPE_WRITE_CRC_EDGE     (1)
#define TYPE_READ_DATA_EDGE     (2)
#define TYPE_WRITE_DATA_EDGE    (3)

#define CARD_READY_FOR_DATA             (1<<8)
#define CARD_CURRENT_STATE(x)           ((x&0x00001E00)>>9)

/* 
 *  This definition is used for MT7623_GPIO_RegMap.doc by candy.li on 2014-9-03 
 *  Zhanyong.Wang
 *  2014/11/28
 */
#define GPIO_PIN_MAX                          280
#if 0
#ifndef GPIO_BASE
#define GPIO_BASE                             0x10005000
#endif
#if GPIO_BASE != 0x10005000
#error  Please check your GPIO_BASE definition!!!!
#endif
#endif

#define REG_GPIO_BASE(x)                     ((volatile unsigned int *)(gpiobase + OFFSET_##x))

#define GPIO_DIR_ASS                         175
#define GPIO_DIR_PIN2REG(n)                  ((volatile unsigned int *)(gpiobase + (((n) <= GPIO_DIR_ASS) ? 0 : 0x10) +  (((n) % GPIO_PIN_MAX) & 0x1F0))) /* 0: input, 1: output */

#define GPIO_DATOUT_PIN2REG(n)               ((volatile unsigned int *)(gpiobase +  0x500 +  (((n) % GPIO_PIN_MAX) & 0x1F0)))
#define GPIO_DATIN_PIN2REG(n)                ((volatile unsigned int *)(gpiobase +  0x630 +  (((n) % GPIO_PIN_MAX) & 0x1F0)))

#define GPIO_PULLEN_PIN2REG(n)               ((volatile unsigned int *)(gpiobase +  0x150 +  (((n) % GPIO_PIN_MAX) & 0x1F0))) /* 0: disable, 1: enable */
#define GPIO_PULLSEL_PIN2REG(n)              ((volatile unsigned int *)(gpiobase +  0x280 +  (((n) % GPIO_PIN_MAX) & 0x1F0))) /* 0: pull down, 1: pull up */

#define BIT_GPIOMASK(n)                      (1 << (((n) % GPIO_PIN_MAX) & 0x0F))                   


#define GPIO_MODE_PIN2REG(n)                 ((volatile unsigned int *)(GPIO_BASE +  0x760 +  ((((n) % GPIO_PIN_MAX)/5) << 4)))

#define BIT_MODE_GPIOMASK(n)                 (0x07 << (3*(((n) % GPIO_PIN_MAX)%5))) 
#define BV_GPIO_MSDC_MODE                    1

#define BIT_MSDC_CLK_R0R1PUPD                (0x07 << 8)
#define BIT_MSDC_CMD_R0R1PUPD                (0x07 << 8)
#define BIT_MSDC_R0R1PUPD(line)              BIT_MSDC_##line##_R0R1PUPD

#define BIT_MSDC_DAT0_R0R1PUPD               (0x07 << 0)
#define BIT_MSDC_DAT1_R0R1PUPD               (0x07 << 4)
#define BIT_MSDC_DAT2_R0R1PUPD               (0x07 << 8)
#define BIT_MSDC_DAT3_R0R1PUPD               (0x07 <<12)
#define BIT_MSDC_DAT4_R0R1PUPD               (0x07 << 0)
#define BIT_MSDC_DAT5_R0R1PUPD               (0x07 << 4)
#define BIT_MSDC_DAT6_R0R1PUPD               (0x07 << 8)
#define BIT_MSDC_DAT7_R0R1PUPD               (0x07 <<12)

#define BIT_MSDC_RSTB_R0R1PUPD               (0x07 << 0)

#define BIT_MSDC_DSL_R0R1PUPD                (0x07 << 4)
#define BIT_MSDC_RCLK_R0R1PUPD               (0x07 << 8)

#define BV_MSDCRX_PUR0R1_HIGHZ                0
#define BV_MSDCRX_PUR0R1_50KOHM               2
#define BV_MSDCRX_PUR0R1_10KOHM               4
#define BV_MSDCRX_PUR0R1_08KOHM               6
#define BV_MSDCRX_PDR0R1_HIGHZ                1
#define BV_MSDCRX_PDR0R1_50KOHM               3
#define BV_MSDCRX_PDR0R1_10KOHM               5
#define BV_MSDCRX_PDR0R1_08KOHM               7

#define BIT_MSDC_CLK_E8E4E2                  (0x07 << 0)
#define BIT_MSDC_CMD_E8E4E2                  (0x07 << 0)
#define BIT_MSDC_DAT_E8E4E2                  (0x07 << 0)
#define BIT_MSDC_RCLK_E8E4E2                 (0x07 << 0)
#define BIT_GPIO_MSDC_18VE8E4E2              (0x07 << 0)
#define BV_MSDCTX_18VE8E4E2_01RD78U85MA       0
#define BV_MSDCTX_18VE8E4E2_03RD78U85MA       1
#define BV_MSDCTX_18VE8E4E2_04RD78U85MA       2
#define BV_MSDCTX_18VE8E4E2_06RD78U85MA       3
#define BV_MSDCTX_18VE8E4E2_07RD78U85MA       4
#define BV_MSDCTX_18VE8E4E2_10RD78U85MA       5
#define BV_MSDCTX_18VE8E4E2_11RD78U85MA       6
#define BV_MSDCTX_18VE8E4E2_13RD78U85MA       7

#define BIT_GPIO_MSDC_33VE8E4E2              (0x07 << 0)
#define BV_MSDCTX_33VE8E4E2_01RD14U17MA       0
#define BV_MSDCTX_33VE8E4E2_03RD14U17MA       1
#define BV_MSDCTX_33VE8E4E2_04RD14U17MA       2
#define BV_MSDCTX_33VE8E4E2_06RD14U17MA       3

/*
   MSDC Driving Strength Definition: specify as gear instead of driving
*/
#define MSDC_DRVN_GEAR0                       BV_MSDCTX_18VE8E4E2_01RD78U85MA
#define MSDC_DRVN_GEAR1                       BV_MSDCTX_18VE8E4E2_03RD78U85MA
#define MSDC_DRVN_GEAR2                       BV_MSDCTX_18VE8E4E2_04RD78U85MA
#define MSDC_DRVN_GEAR3                       BV_MSDCTX_18VE8E4E2_06RD78U85MA
#define MSDC_DRVN_GEAR4                       BV_MSDCTX_18VE8E4E2_07RD78U85MA
#define MSDC_DRVN_GEAR5                       BV_MSDCTX_18VE8E4E2_10RD78U85MA
#define MSDC_DRVN_GEAR6                       BV_MSDCTX_18VE8E4E2_11RD78U85MA
#define MSDC_DRVN_GEAR7                       BV_MSDCTX_18VE8E4E2_13RD78U85MA
#define MSDC_DRVN_DONT_CARE                   MSDC_DRVN_GEAR0

/* id valid range "MSDC[0, 3]" index"[0, 5/6]" */
#define MSDC_CTRL(id, index)                  GPIO_##id##_CTRL##index

#define PIN4_CLK(id)                          PIN4##id##_CLK
#define PIN4_CMD(id)                          PIN4##id##_CMD
#define PIN4_RSTB(id)                         PIN4##id##_RSTB
#define PIN4_DAT(id,line)                     PIN4##id##_##line
#define PIN4_DSL(id)                          PIN4##id##_DSL
#define PIN4_INS(id)                          PIN4##id##_INS
      
/* id valid range "MSDC[0, 4]"*/      
#define MSDC_CLK_SMT(id)                      B_##id##CLK_SMT
#define MSDC_CLK_R0(id)                       B_##id##CLK_R0
#define MSDC_CLK_R1(id)                       B_##id##CLK_R1
#define MSDC_CLK_PUPD(id)                     B_##id##CLK_PUPD
#define MSDC_CLK_IES(id)                      B_##id##CLK_IES
#define MSDC_CLK_SR(id)                       B_##id##CLK_SR
#define MSDC_CLK_E8(id)                       B_##id##CLK_E8
#define MSDC_CLK_E4(id)                       B_##id##CLK_E4
#define MSDC_CLK_E2(id)                       B_##id##CLK_E2
		
#define MSDC_CMD_SMT(id)                      B_##id##CMD_SMT
#define MSDC_CMD_R0(id)                       B_##id##CMD_R0
#define MSDC_CMD_R1(id)                       B_##id##CMD_R1
#define MSDC_CMD_PUPD(id)                     B_##id##CMD_PUPD
#define MSDC_CMD_IES(id)                      B_##id##CMD_IES
#define MSDC_CMD_SR(id)                       B_##id##CMD_SR
#define MSDC_CMD_E8(id)                       B_##id##CMD_E8
#define MSDC_CMD_E4(id)                       B_##id##CMD_E4
#define MSDC_CMD_E2(id)                       B_##id##CMD_E2
		
#define MSDC_DAT_IES(id)                      B_##id##DAT_IES
#define MSDC_DAT_SR(id)                       B_##id##DAT_SR
#define MSDC_DAT_E8(id)                       B_##id##DAT_E8
#define MSDC_DAT_E4(id)                       B_##id##DAT_E4
#define MSDC_DAT_E2(id)                       B_##id##DAT_E2

/* use line valid range "DAT[0, 7]" */
#define MSDC_DAT_SMT(id,line)                 B_##id##line##_SMT
#define MSDC_DAT_R0(id,line)                  B_##id##line##_R0
#define MSDC_DAT_R1(id,line)                  B_##id##line##_R1
#define MSDC_DAT_PUPD(id,line)                B_##id##line##_PUPD
		
#define MSDC_DSL_SMT(id)                      B_##id##DSL_SMT
#define MSDC_DSL_R0(id)                       B_##id##DSL_R0
#define MSDC_DSL_R1(id)                       B_##id##DSL_R1
#define MSDC_DSL_PUPD(id)                     B_##id##DSL_PUPD
		
#define MSDC_RSTB_SMT(id)                     B_##id##RSTB_SMT
#define MSDC_RSTB_R0(id)                      B_##id##RSTB_R0
#define MSDC_RSTB_R1(id)                      B_##id##RSTB_R1
#define MSDC_RSTB_PUPD(id)                    B_##id##RSTB_PUPD
		
#define MSDC_PAD_RDSEL(id)                    B_##id##PAD_RDSEL
#define MSDC_PAD_TDSEL(id)                    B_##id##PAD_TDSEL



#define MSDC_INS_SMT(id)                      B_##id##INS_SMT
/* *RCLK* Obsolete control pin by hui.zeng confirmed, fengguo.gao and zhanyong.wang present 2014/12/18*/
#define MSDC_RCLK_SMT(id)                     B_##id##RCLK_SMT           
#define MSDC_RCLK_R0(id)                      B_##id##RCLK_R0
#define MSDC_RCLK_R1(id)                      B_##id##RCLK_R1
#define MSDC_RCLK_PUPD(id)                    B_##id##RCLK_PUPD
											
#define MSDC_RCLK_IES(id)                     B_##id##RCLK_IES
#define MSDC_RCLK_SR(id)                      B_##id##RCLK_SR
#define MSDC_RCLK_E8(id)                      B_##id##RCLK_E8 
#define MSDC_RCLK_E4(id)                      B_##id##RCLK_E4 
#define MSDC_RCLK_E2(id)                      B_##id##RCLK_E2 
											
#define MSDC_RCLK_RDSEL(id)                   B_##id##RCLK_RDSEL  
#define MSDC_RCLK_TDSEL(id)                   B_##id##RCLK_TDSEL   

/* id valid range "EINT[0, 1]" for Eint number 14,15,16,17,21 */
#define EINT_CTRL(index)                      GPIO_EINT##index##_CTRL

#define BIT_EINT_PUPDR1R0(eintid)             BIT_EINT##eintid##_PUPDR1R0

#define BIT_EINT14_PUPDR1R0                   (0x0F  <<  0) 
#define BIT_EINT15_PUPDR1R0                   (0x0F  <<  4) 
#define BIT_EINT16_PUPDR1R0                   (0x0F  <<  8) 
#define BIT_EINT17_PUPDR1R0                   (0x0F  << 12) 

#define BIT_EINT21_PUPDR1R0                   (0x0F  <<  0) 

#define BV_EINT_PUR0R1_HIGHZ                  0
#define BV_EINT_PUR0R1_50KOHM                 1
#define BV_EINT_PUR0R1_10KOHM                 2
#define BV_EINT_PUR0R1_08KOHM                 3
#define BV_EINT_PDR0R1_HIGHZ                  4
#define BV_EINT_PDR0R1_50KOHM                 5
#define BV_EINT_PDR0R1_10KOHM                 6
#define BV_EINT_PDR0R1_08KOHM                 7

#define BIT_EINT_PUPD(eintid)                     B_##eintid##_PUPD
#define BIT_EINT_R1R0(eintid)                     B_##eintid##_R1R0

/* ******************************Don't modify these source code below ******************************/
#define OFFSET_EINT0_CTRL                        0xE70
#define OFFSET_EINT1_CTRL                        0xE80

#ifdef EINT0_CTRL
#undef EINT0_CTRL
#endif
#ifdef EINT1_CTRL
#undef EINT1_CTRL
#endif

#define GPIO_EINT0_CTRL                         REG_GPIO_BASE(EINT0_CTRL)
#define GPIO_EINT1_CTRL                         REG_GPIO_BASE(EINT1_CTRL)

/* 0x10005E70 OFFSET_EINT0_CTRL   EINT Pad R0/R1/PUPD Control 0 Register 0x1115 */
#define B_EINT17_BACKUP3                        (  1 << 14)
#define B_EINT17_PUPD                           (  1 << 13)
#define B_EINT17_R1R0                           (  3 << 12)
#define B_EINT16_BACKUP2                        (  1 << 11)
#define B_EINT16_PUPD                           (  1 << 10)
#define B_EINT16_R1R0                           (  3 <<  8)
#define B_EINT15_BACKUP1                        (  1 <<  7)
#define B_EINT15_PUPD                           (  1 <<  6)
#define B_EINT15_R1R0                           (  3 <<  4)
#define B_EINT14_BACKUP0                        (  1 <<  3)
#define B_EINT14_PUPD                           (  1 <<  2)
#define B_EINT14_R1R0                           (  3 <<  0)

/* 0x10005E80 OFFSET_EINT1_CTRL   EINT Pad R0/R1/PUPD Control 1 Register 0x0001 */
#define B_EINTCTRL_BACKUP0                      (0xFFF<< 4)
#define B_EINT21_BACKUP0                        (  1 <<  3)
#define B_EINT21_PUPD                           (  1 <<  2)
#define B_EINT21_R1R0                           (  3 <<  0)

/* 
 * MSDC0  GPIO Control definition 
 *  DAT7   DAT6 DAT5 DAT4 RSTB CMD CLK DAT3 DAT2 DAT1 DAT0 
 * GPIO111 112  113  114  115  116 117 118  119  120  121
 *   1      1    1    1    0    1   0   1    1    1    1  
 ***************************************************************************/
#define PIN4MSDC0_CLK                           117
#define PIN4MSDC0_CMD                           116
#define PIN4MSDC0_RSTB                          115
#define PIN4MSDC0_DAT0                          121
#define PIN4MSDC0_DAT1                          120
#define PIN4MSDC0_DAT2                          119
#define PIN4MSDC0_DAT3                          118
#define PIN4MSDC0_DAT4                          114
#define PIN4MSDC0_DAT5                          113
#define PIN4MSDC0_DAT6                          112
#define PIN4MSDC0_DAT7                          111

#define OFFSET_MSDC0_CTRL0                      0xCC0
#define OFFSET_MSDC0_CTRL1                      0xCD0
#define OFFSET_MSDC0_CTRL2                      0xCE0
#define OFFSET_MSDC0_CTRL3                      0xCF0
#define OFFSET_MSDC0_CTRL4                      0xD00
#define OFFSET_MSDC0_CTRL5                      0xD10
#define OFFSET_MSDC0_CTRL6                      0xD20

#ifdef MSDC0_CTRL0
#undef MSDC0_CTRL0
#endif
#ifdef MSDC0_CTRL1
#undef MSDC0_CTRL1
#endif
#ifdef MSDC0_CTRL2
#undef MSDC0_CTRL2
#endif
#ifdef MSDC0_CTRL3
#undef MSDC0_CTRL3
#endif
#ifdef MSDC0_CTRL4
#undef MSDC0_CTRL4
#endif
#ifdef MSDC0_CTRL5
#undef MSDC0_CTRL5
#endif
#ifdef MSDC0_CTRL6
#undef MSDC0_CTRL6
#endif

#define GPIO_MSDC0_CTRL0                        REG_GPIO_BASE(MSDC0_CTRL0)
#define GPIO_MSDC0_CTRL1                        REG_GPIO_BASE(MSDC0_CTRL1)
#define GPIO_MSDC0_CTRL2                        REG_GPIO_BASE(MSDC0_CTRL2)
#define GPIO_MSDC0_CTRL3                        REG_GPIO_BASE(MSDC0_CTRL3)
#define GPIO_MSDC0_CTRL4                        REG_GPIO_BASE(MSDC0_CTRL4)
#define GPIO_MSDC0_CTRL5                        REG_GPIO_BASE(MSDC0_CTRL5)
#define GPIO_MSDC0_CTRL6                        REG_GPIO_BASE(MSDC0_CTRL6)

/* 0x10005CC0 MSDC0_CTRL0   MSDC 0 CLK pad Control Register 0x0311 */
#define B_MSDC0CLK_BACKUP1                     (0xF << 12)
#define B_MSDC0CLK_SMT                         (  1 << 11)
#define B_MSDC0CLK_R0                          (  1 << 10)
#define B_MSDC0CLK_R1                          (  1 <<  9)
#define B_MSDC0CLK_PUPD                        (  1 <<  8)
#define B_MSDC0CLK_BACKUP0                     (0x7 <<  5)
#define B_MSDC0CLK_IES                         (  1 <<  4)
#define B_MSDC0CLK_SR                          (  1 <<  3)
#define B_MSDC0CLK_E8                          (  1 <<  2)
#define B_MSDC0CLK_E4                          (  1 <<  1)
#define B_MSDC0CLK_E2                          (  1 <<  0)

/* 0x10005CD0 MSDC0_CTRL1   MSDC 0 CMD pad Control Register 0x0411 */
#define B_MSDC0CMD_BACKUP1                     (0xF << 12)
#define B_MSDC0CMD_SMT                         (  1 << 11)
#define B_MSDC0CMD_R0                          (  1 << 10)
#define B_MSDC0CMD_R1                          (  1 <<  9)
#define B_MSDC0CMD_PUPD                        (  1 <<  8)
#define B_MSDC0CMD_BACKUP0                     (0x7 <<  5)
#define B_MSDC0CMD_IES                         (  1 <<  4)
#define B_MSDC0CMD_SR                          (  1 <<  3)
#define B_MSDC0CMD_E8                          (  1 <<  2)
#define B_MSDC0CMD_E4                          (  1 <<  1)
#define B_MSDC0CMD_E2                          (  1 <<  0)

/* 0x10005CE0 MSDC0_CTRL2   MSDC 0 DAT pad Control Register 0x0011 */
#define B_MSDC0DAT_BACKUP1                     (0x7FF<< 5)
#define B_MSDC0DAT_IES                         (  1 <<  4)
#define B_MSDC0DAT_SR                          (  1 <<  3)
#define B_MSDC0DAT_E8                          (  1 <<  2)
#define B_MSDC0DAT_E4                          (  1 <<  1)
#define B_MSDC0DAT_E2                          (  1 <<  0)


/* 0x10005CF0 MSDC0_CTRL3   MSDC 0 DAT pad Control Register 0x4444 */
#define B_MSDC0DAT3_SMT                         (  1 << 15)
#define B_MSDC0DAT3_R0                          (  1 << 14)
#define B_MSDC0DAT3_R1                          (  1 << 13)
#define B_MSDC0DAT3_PUPD                        (  1 << 12)

#define B_MSDC0DAT2_SMT                         (  1 << 11)
#define B_MSDC0DAT2_R0                          (  1 << 10)
#define B_MSDC0DAT2_R1                          (  1 <<  9)
#define B_MSDC0DAT2_PUPD                        (  1 <<  8)

#define B_MSDC0DAT1_SMT                         (  1 <<  7)
#define B_MSDC0DAT1_R0                          (  1 <<  6)
#define B_MSDC0DAT1_R1                          (  1 <<  5)
#define B_MSDC0DAT1_PUPD                        (  1 <<  4)

#define B_MSDC0DAT0_SMT                         (  1 <<  3)
#define B_MSDC0DAT0_R0                          (  1 <<  2)
#define B_MSDC0DAT0_R1                          (  1 <<  1)
#define B_MSDC0DAT0_PUPD                        (  1 <<  0)


/* 0x10005D00 MSDC0_CTRL4   MSDC 0 DAT pad Control Register 0x4444 */
#define B_MSDC0DAT7_SMT                         (  1 << 15)
#define B_MSDC0DAT7_R0                          (  1 << 14)
#define B_MSDC0DAT7_R1                          (  1 << 13)
#define B_MSDC0DAT7_PUPD                        (  1 << 12)

#define B_MSDC0DAT6_SMT                         (  1 << 11)
#define B_MSDC0DAT6_R0                          (  1 << 10)
#define B_MSDC0DAT6_R1                          (  1 <<  9)
#define B_MSDC0DAT6_PUPD                        (  1 <<  8)

#define B_MSDC0DAT5_SMT                         (  1 <<  7)
#define B_MSDC0DAT5_R0                          (  1 <<  6)
#define B_MSDC0DAT5_R1                          (  1 <<  5)
#define B_MSDC0DAT5_PUPD                        (  1 <<  4)

#define B_MSDC0DAT4_SMT                         (  1 <<  3)
#define B_MSDC0DAT4_R0                          (  1 <<  2)
#define B_MSDC0DAT4_R1                          (  1 <<  1)
#define B_MSDC0DAT4_PUPD                        (  1 <<  0)

/* 0x10005D10 MSDC0_CTRL5   MSDC 0 DAT pad Control Register  0x0004 */
#define B_MSDC0DAT_BACKUP                       (0xFFF<< 4)
#define B_MSDC0RSTB_SMT                         (  1 <<  3)
#define B_MSDC0RSTB_R0                          (  1 <<  2)
#define B_MSDC0RSTB_R1                          (  1 <<  1)
#define B_MSDC0RSTB_PUPD                        (  1 <<  0)

/* 0x10005D20 MSDC0_CTRL6   MSDC 0 pad Control Register      0x000A */
#define B_MSDC0PAD_BACKUP                       (0x3F << 10)
#define B_MSDC0PAD_RDSEL                        (0x3F <<  4)
#define B_MSDC0PAD_TDSEL                        (0x0F <<  0)

/* 
 * MSDC1  GPIO Control definition 
 *  CMD     CLK     DAT0     DAT1   DAT2    DAT3   INS
 * GPIO105  106     107      108    109     110    261  with func 1(mode 1)
 *    1      0       1        1      1       1         should be these/reset value 
 ***************************************************************************/
#define PIN4MSDC1_CLK                            106
#define PIN4MSDC1_CMD                            105
#define PIN4MSDC1_DAT0                           107
#define PIN4MSDC1_DAT1                           108
#define PIN4MSDC1_DAT2                           109
#define PIN4MSDC1_DAT3                           110
#define PIN4MSDC1_INS                            261

#define OFFSET_MSDC1_CTRL0                       0xD30
#define OFFSET_MSDC1_CTRL1                       0xD40
#define OFFSET_MSDC1_CTRL2                       0xD50
#define OFFSET_MSDC1_CTRL3                       0xD60
#define OFFSET_MSDC1_CTRL4                       0xD70
#define OFFSET_MSDC1_CTRL5                       0xD80

#define OFFSET_MSDC1_CTRL6                       0x0B0

#ifdef MSDC1_CTRL0
#undef MSDC1_CTRL0
#endif
#ifdef MSDC1_CTRL1
#undef MSDC1_CTRL1
#endif
#ifdef MSDC1_CTRL2
#undef MSDC1_CTRL2
#endif
#ifdef MSDC1_CTRL3
#undef MSDC1_CTRL3
#endif
#ifdef MSDC1_CTRL4
#undef MSDC1_CTRL4
#endif
#ifdef MSDC1_CTRL5
#undef MSDC1_CTRL5
#endif
#ifdef MSDC1_CTRL6
#undef MSDC1_CTRL6
#endif

#define GPIO_MSDC1_CTRL0                         REG_GPIO_BASE(MSDC1_CTRL0)
#define GPIO_MSDC1_CTRL1                         REG_GPIO_BASE(MSDC1_CTRL1)
#define GPIO_MSDC1_CTRL2                         REG_GPIO_BASE(MSDC1_CTRL2)
#define GPIO_MSDC1_CTRL3                         REG_GPIO_BASE(MSDC1_CTRL3)
#define GPIO_MSDC1_CTRL4                         REG_GPIO_BASE(MSDC1_CTRL4)
#define GPIO_MSDC1_CTRL5                         REG_GPIO_BASE(MSDC1_CTRL5)

#define GPIO_MSDC1_CTRL6                         REG_GPIO_BASE(MSDC1_CTRL6)

/* 0x10005D30 MSDC1_CTRL0   MSDC 1 CLK pad Control Register 0x0311 */
#define B_MSDC1CLK_BACKUP1                      (0xF << 12)
#define B_MSDC1CLK_SMT                          (  1 << 11)
#define B_MSDC1CLK_R0                           (  1 << 10)
#define B_MSDC1CLK_R1                           (  1 <<  9)
#define B_MSDC1CLK_PUPD                         (  1 <<  8)
#define B_MSDC1CLK_BACKUP0                      (0x7 <<  5)
#define B_MSDC1CLK_IES                          (  1 <<  4)
#define B_MSDC1CLK_SR                           (  1 <<  3)
#define B_MSDC1CLK_E8                           (  1 <<  2)
#define B_MSDC1CLK_E4                           (  1 <<  1)
#define B_MSDC1CLK_E2                           (  1 <<  0)

/* 0x10005D40 MSDC1_CTRL1   MSDC 1 CMD pad Control Register 0x0211 */
#define B_MSDC1CMD_BACKUP1                      (0xF << 12)
#define B_MSDC1CMD_SMT                          (  1 << 11)
#define B_MSDC1CMD_R0                           (  1 << 10)
#define B_MSDC1CMD_R1                           (  1 <<  9)
#define B_MSDC1CMD_PUPD                         (  1 <<  8)
#define B_MSDC1CMD_BACKUP0                      (0x7 <<  5)
#define B_MSDC1CMD_IES                          (  1 <<  4)
#define B_MSDC1CMD_SR                           (  1 <<  3)
#define B_MSDC1CMD_E8                           (  1 <<  2)
#define B_MSDC1CMD_E4                           (  1 <<  1)
#define B_MSDC1CMD_E2                           (  1 <<  0)

/* 0x10005D50 MSDC1_CTRL2   MSDC 1 DAT pad Control Register 0x0011 */
#define B_MSDC1DAT_BACKUP1                      (0x7FF<< 5)
#define B_MSDC1DAT_IES                          (  1 <<  4)
#define B_MSDC1DAT_SR                           (  1 <<  3)
#define B_MSDC1DAT_E8                           (  1 <<  2)
#define B_MSDC1DAT_E4                           (  1 <<  1)
#define B_MSDC1DAT_E2                           (  1 <<  0)

/* 0x10005D60 MSDC1_CTRL3   MSDC 1 DAT pad Control Register  0x2222 */
#define B_MSDC1DAT3_SMT                          (  1 << 15)
#define B_MSDC1DAT3_R0                           (  1 << 14)
#define B_MSDC1DAT3_R1                           (  1 << 13)
#define B_MSDC1DAT3_PUPD                         (  1 << 12)

#define B_MSDC1DAT2_SMT                          (  1 << 11)
#define B_MSDC1DAT2_R0                           (  1 << 10)
#define B_MSDC1DAT2_R1                           (  1 <<  9)
#define B_MSDC1DAT2_PUPD                         (  1 <<  8)

#define B_MSDC1DAT1_SMT                          (  1 <<  7)
#define B_MSDC1DAT1_R0                           (  1 <<  6)
#define B_MSDC1DAT1_R1                           (  1 <<  5)
#define B_MSDC1DAT1_PUPD                         (  1 <<  4)

#define B_MSDC1DAT0_SMT                          (  1 <<  3)
#define B_MSDC1DAT0_R0                           (  1 <<  2)
#define B_MSDC1DAT0_R1                           (  1 <<  1)
#define B_MSDC1DAT0_PUPD                         (  1 <<  0)

/* 0x10005D70 MSDC1_CTRL4   MSDC 1 DAT pad Control Register  0x0000 */
#define B_MSDC1DAT_BACKUP                        (0xFFFF<<0)

/* 0x10005D80 MSDC1_CTRL5   MSDC 1 pad Control Register      0x00CA */
#define B_MSDC1PAD_BACKUP                        (0x3F << 10)
#define B_MSDC1PAD_RDSEL                         (0x3F <<  4)
#define B_MSDC1PAD_TDSEL                         (0x0F <<  0)

/* 0x100050B0 MSDC1_CTRL6   MSDC 1 INS Pad Control Register  0x0000 */
#define B_MSDC1INS_BACKUP1                       (0xFFF<<  4)
#define B_MSDC1INS_SMT                           (  1  <<  3)
#define B_MSDC1INS_BACKUP0                       (0x07 <<  0)

/* 
 * MSDC2  GPIO Control definition 
 *  CMD     CLK   DAT0    DAT1  DAT2   DAT3
 * GPIO85   86    87      88    89     90          with func 1 (mode 1)
 *    1     0     1       1     1      1           should be these 
 ***************************************************************************/
#define PIN4MSDC2_CLK                             86
#define PIN4MSDC2_CMD                             85
#define PIN4MSDC2_DAT0                            87
#define PIN4MSDC2_DAT1                            88
#define PIN4MSDC2_DAT2                            89
#define PIN4MSDC2_DAT3                            90

#define OFFSET_MSDC2_CTRL0                        0xD90
#define OFFSET_MSDC2_CTRL1                        0xDA0
#define OFFSET_MSDC2_CTRL2                        0xDB0
#define OFFSET_MSDC2_CTRL3                        0xDC0
#define OFFSET_MSDC2_CTRL4                        0xDD0
#define OFFSET_MSDC2_CTRL5                        0xDE0

#ifdef MSDC2_CTRL0
#undef MSDC2_CTRL0
#endif
#ifdef MSDC2_CTRL1
#undef MSDC2_CTRL1
#endif
#ifdef MSDC2_CTRL2
#undef MSDC2_CTRL2
#endif
#ifdef MSDC2_CTRL3
#undef MSDC2_CTRL3
#endif
#ifdef MSDC2_CTRL4
#undef MSDC2_CTRL4
#endif
#ifdef MSDC2_CTRL5
#undef MSDC2_CTRL5
#endif
#ifdef MSDC2_CTRL6
#undef MSDC2_CTRL6
#endif


#define GPIO_MSDC2_CTRL0                          REG_GPIO_BASE(MSDC2_CTRL0)
#define GPIO_MSDC2_CTRL1                          REG_GPIO_BASE(MSDC2_CTRL1)
#define GPIO_MSDC2_CTRL2                          REG_GPIO_BASE(MSDC2_CTRL2)
#define GPIO_MSDC2_CTRL3                          REG_GPIO_BASE(MSDC2_CTRL3)
#define GPIO_MSDC2_CTRL4                          REG_GPIO_BASE(MSDC2_CTRL4)
#define GPIO_MSDC2_CTRL5                          REG_GPIO_BASE(MSDC2_CTRL5)

/* 0x10005D90 MSDC2_CTRL0   MSDC 2 CLK pad Control Register 0x0411 */
#define B_MSDC2CLK_BACKUP1                       (0xF << 12)
#define B_MSDC2CLK_SMT                           (  1 << 11)
#define B_MSDC2CLK_R0                            (  1 << 10)
#define B_MSDC2CLK_R1                            (  1 <<  9)
#define B_MSDC2CLK_PUPD                          (  1 <<  8)
#define B_MSDC2CLK_BACKUP0                       (0x7 <<  5)
#define B_MSDC2CLK_IES                           (  1 <<  4)
#define B_MSDC2CLK_SR                            (  1 <<  3)
#define B_MSDC2CLK_E8                            (  1 <<  2)
#define B_MSDC2CLK_E4                            (  1 <<  1)
#define B_MSDC2CLK_E2                            (  1 <<  0)

/* 0x10005DA0 MSDC2_CTRL1   MSDC 2 CMD pad Control Register 0x0511 */
#define B_MSDC2CMD_BACKUP1                       (0xF << 12)
#define B_MSDC2CMD_SMT                           (  1 << 11)
#define B_MSDC2CMD_R0                            (  1 << 10)
#define B_MSDC2CMD_R1                            (  1 <<  9)
#define B_MSDC2CMD_PUPD                          (  1 <<  8)
#define B_MSDC2CMD_BACKUP0                       (0x7 <<  5)
#define B_MSDC2CMD_IES                           (  1 <<  4)
#define B_MSDC2CMD_SR                            (  1 <<  3)
#define B_MSDC2CMD_E8                            (  1 <<  2)
#define B_MSDC2CMD_E4                            (  1 <<  1)
#define B_MSDC2CMD_E2                            (  1 <<  0)

/* 0x10005DB0 MSDC2_CTRL2   MSDC 2 DAT pad Control Register 0x0011 */
#define B_MSDC2DAT_BACKUP1                       (0x7FF<< 5)
#define B_MSDC2DAT_IES                           (  1 <<  4)
#define B_MSDC2DAT_SR                            (  1 <<  3)
#define B_MSDC2DAT_E8                            (  1 <<  2)
#define B_MSDC2DAT_E4                            (  1 <<  1)
#define B_MSDC2DAT_E2                            (  1 <<  0)


/* 0x10005DC0 MSDC2_CTRL3   MSDC 2 DAT pad Control Register 0x5555 */
#define B_MSDC2DAT3_SMT                          (  1 << 15)
#define B_MSDC2DAT3_R0                           (  1 << 14)
#define B_MSDC2DAT3_R1                           (  1 << 13)
#define B_MSDC2DAT3_PUPD                         (  1 << 12)

#define B_MSDC2DAT2_SMT                          (  1 << 11)
#define B_MSDC2DAT2_R0                           (  1 << 10)
#define B_MSDC2DAT2_R1                           (  1 <<  9)
#define B_MSDC2DAT2_PUPD                         (  1 <<  8)

#define B_MSDC2DAT1_SMT                          (  1 <<  7)
#define B_MSDC2DAT1_R0                           (  1 <<  6)
#define B_MSDC2DAT1_R1                           (  1 <<  5)
#define B_MSDC2DAT1_PUPD                         (  1 <<  4)

#define B_MSDC2DAT0_SMT                          (  1 <<  3)
#define B_MSDC2DAT0_R0                           (  1 <<  2)
#define B_MSDC2DAT0_R1                           (  1 <<  1)
#define B_MSDC2DAT0_PUPD                         (  1 <<  0)

/* 0x10005DD0 MSDC2_CTRL4   MSDC 2 DAT pad Control Register  0x0000 */
#define B_MSDC2DAT_BACKUP                        (0xFFFF<<0)

/* 0x10005DE0 MSDC2_CTRL5   MSDC 2 pad Control Register      0x0000 */
#define B_MSDC2PAD_BACKUP                        (0x3F << 10)
#define B_MSDC2PAD_RDSEL                         (0x3F <<  4)
#define B_MSDC2PAD_TDSEL                         (0x0F <<  0)

/*
 * MSDC3  GPIO Control definition 
 *  DAT7   DAT6 DAT5 DAT4 DAT3 DAT2 DAT1 DAT0 RSTB CMD CLK DSL
 * GPIO250 251  252  253  254  255  256  257  249  258 259 260
 *   1      1    1    1    1    1    1    1    0    1   0   1     
 ***************************************************************************/
#define PIN4MSDC3_CLK                             259
#define PIN4MSDC3_CMD                             258
#define PIN4MSDC3_RSTB                            249
#define PIN4MSDC3_DAT0                            257
#define PIN4MSDC3_DAT1                            256
#define PIN4MSDC3_DAT2                            255
#define PIN4MSDC3_DAT3                            254
#define PIN4MSDC3_DAT4                            253
#define PIN4MSDC3_DAT5                            252
#define PIN4MSDC3_DAT6                            251
#define PIN4MSDC3_DAT7                            250
#define PIN4MSDC3_DSL                             260

#define OFFSET_MSDC3_CTRL0                        0xC90
#define OFFSET_MSDC3_CTRL1                        0xCB0
#define OFFSET_MSDC3_CTRL2                        0xFC0
#define OFFSET_MSDC3_CTRL3                        0xF40
#define OFFSET_MSDC3_CTRL4                        0x130
#define OFFSET_MSDC3_CTRL5                        0x140
#define OFFSET_MSDC3_CTRL6                        0x620

#define OFFSET_MSDC3_CTRL7                        0x3A0
#define OFFSET_MSDC3_CTRL8                        0x430

#ifdef MSDC3_CTRL0
#undef MSDC3_CTRL0
#endif
#ifdef MSDC3_CTRL1
#undef MSDC3_CTRL1
#endif
#ifdef MSDC3_CTRL2
#undef MSDC3_CTRL2
#endif
#ifdef MSDC3_CTRL3
#undef MSDC3_CTRL3
#endif
#ifdef MSDC3_CTRL4
#undef MSDC3_CTRL4
#endif
#ifdef MSDC3_CTRL5
#undef MSDC3_CTRL5
#endif
#ifdef MSDC3_CTRL6
#undef MSDC3_CTRL6
#endif
#ifdef MSDC3_CTRL7
#undef MSDC3_CTRL7
#endif
#ifdef MSDC3_CTRL8
#undef MSDC3_CTRL8
#endif

#define GPIO_MSDC3_CTRL0                          REG_GPIO_BASE(MSDC3_CTRL0)
#define GPIO_MSDC3_CTRL1                          REG_GPIO_BASE(MSDC3_CTRL1)
#define GPIO_MSDC3_CTRL2                          REG_GPIO_BASE(MSDC3_CTRL2)
#define GPIO_MSDC3_CTRL3                          REG_GPIO_BASE(MSDC3_CTRL3)
#define GPIO_MSDC3_CTRL4                          REG_GPIO_BASE(MSDC3_CTRL4)
#define GPIO_MSDC3_CTRL5                          REG_GPIO_BASE(MSDC3_CTRL5)
#define GPIO_MSDC3_CTRL6                          REG_GPIO_BASE(MSDC3_CTRL6)         

#define GPIO_MSDC3_CTRL7                          REG_GPIO_BASE(MSDC3_CTRL7)
#define GPIO_MSDC3_CTRL8                          REG_GPIO_BASE(MSDC3_CTRL8)   

/* 0x10005C90 MSDC3_CTRL0   MSDC 3 CLK pad Control Register 0x0314 */
#define B_MSDC3CLK_BACKUP1                       (0xF << 12)
#define B_MSDC3CLK_SMT                           (  1 << 11)
#define B_MSDC3CLK_R0                            (  1 << 10)
#define B_MSDC3CLK_R1                            (  1 <<  9)
#define B_MSDC3CLK_PUPD                          (  1 <<  8)
#define B_MSDC3CLK_BACKUP0                       (0x7 <<  5)
#define B_MSDC3CLK_IES                           (  1 <<  4)
#define B_MSDC3CLK_SR                            (  1 <<  3)
#define B_MSDC3CLK_E8                            (  1 <<  2)
#define B_MSDC3CLK_E4                            (  1 <<  1)
#define B_MSDC3CLK_E2                            (  1 <<  0)

/* 0x10005CB0 MSDC3_CTRL1   MSDC 3 CMD pad Control Register 0x0314 */
#define B_MSDC3CMD_BACKUP1                       (0xF << 12)
#define B_MSDC3CMD_SMT                           (  1 << 11)
#define B_MSDC3CMD_R0                            (  1 << 10)
#define B_MSDC3CMD_R1                            (  1 <<  9)
#define B_MSDC3CMD_PUPD                          (  1 <<  8)
#define B_MSDC3CMD_BACKUP0                       (0x7 <<  5)
#define B_MSDC3CMD_IES                           (  1 <<  4)
#define B_MSDC3CMD_SR                            (  1 <<  3)
#define B_MSDC3CMD_E8                            (  1 <<  2)
#define B_MSDC3CMD_E4                            (  1 <<  1)
#define B_MSDC3CMD_E2                            (  1 <<  0)

/* 0x10005FC0 MSDC3_CTRL2   MSDC 3 DAT pad Control Register 0x0014 */
#define B_MSDC3DAT_BACKUP1                       (0x7FF<< 5)
#define B_MSDC3DAT_IES                           (  1 <<  4)
#define B_MSDC3DAT_SR                            (  1 <<  3)
#define B_MSDC3DAT_E8                            (  1 <<  2)
#define B_MSDC3DAT_E4                            (  1 <<  1)
#define B_MSDC3DAT_E2                            (  1 <<  0)


/* 0x10005F40 MSDC3_CTRL3   MSDC 3 DAT pad Control Register 0x4444 */
#define B_MSDC3DAT3_SMT                          (  1 << 15)
#define B_MSDC3DAT3_R0                           (  1 << 14)
#define B_MSDC3DAT3_R1                           (  1 << 13)
#define B_MSDC3DAT3_PUPD                         (  1 << 12)

#define B_MSDC3DAT2_SMT                          (  1 << 11)
#define B_MSDC3DAT2_R0                           (  1 << 10)
#define B_MSDC3DAT2_R1                           (  1 <<  9)
#define B_MSDC3DAT2_PUPD                         (  1 <<  8)

#define B_MSDC3DAT1_SMT                          (  1 <<  7)
#define B_MSDC3DAT1_R0                           (  1 <<  6)
#define B_MSDC3DAT1_R1                           (  1 <<  5)
#define B_MSDC3DAT1_PUPD                         (  1 <<  4)

#define B_MSDC3DAT0_SMT                          (  1 <<  3)
#define B_MSDC3DAT0_R0                           (  1 <<  2)
#define B_MSDC3DAT0_R1                           (  1 <<  1)
#define B_MSDC3DAT0_PUPD                         (  1 <<  0)


/* 0x10005130 MSDC3_CTRL4   MSDC 3 DAT pad Control Register 0x4444 */
#define B_MSDC3DAT7_SMT                          (  1 << 15)
#define B_MSDC3DAT7_R0                           (  1 << 14)
#define B_MSDC3DAT7_R1                           (  1 << 13)
#define B_MSDC3DAT7_PUPD                         (  1 << 12)

#define B_MSDC3DAT6_SMT                          (  1 << 11)
#define B_MSDC3DAT6_R0                           (  1 << 10)
#define B_MSDC3DAT6_R1                           (  1 <<  9)
#define B_MSDC3DAT6_PUPD                         (  1 <<  8)

#define B_MSDC3DAT5_SMT                          (  1 <<  7)
#define B_MSDC3DAT5_R0                           (  1 <<  6)
#define B_MSDC3DAT5_R1                           (  1 <<  5)
#define B_MSDC3DAT5_PUPD                         (  1 <<  4)

#define B_MSDC3DAT4_SMT                          (  1 <<  3)
#define B_MSDC3DAT4_R0                           (  1 <<  2)
#define B_MSDC3DAT4_R1                           (  1 <<  1)
#define B_MSDC3DAT4_PUPD                         (  1 <<  0)

/* 0x10005140 MSDC3_CTRL5   MSDC 3 DAT pad Control Register 0x0404 */
#define B_MSDC3DAT_BACKUP                        (  1 << 15)

#define B_MSDC3DSL_SMT                           (  1 <<  7)
#define B_MSDC3DSL_R0                            (  1 <<  6)
#define B_MSDC3DSL_R1                            (  1 <<  5)
#define B_MSDC3DSL_PUPD                          (  1 <<  4)

#define B_MSDC3RSTB_SMT                          (  1 <<  3)
#define B_MSDC3RSTB_R0                           (  1 <<  2)
#define B_MSDC3RSTB_R1                           (  1 <<  1)
#define B_MSDC3RSTB_PUPD                         (  1 <<  0)

/* 0x10005620 MSDC3_CTRL6   MSDC 3 pad Control Register      0x00AC */
#define B_MSDC3PAD_BACKUP                        (0x3F << 10)
#define B_MSDC3PAD_RDSEL                         (0x3F <<  4)
#define B_MSDC3PAD_TDSEL                         (0x0F <<  0)

/* 0x100053A0 MSDC3_CTRL7   MSDC 3 RCLK pad Control Register 0x0414 */
#define B_MSDC3RCLK_BACKUP1                      (0xF << 12)
#define B_MSDC3RCLK_SMT                          (  1 << 11)
#define B_MSDC3RCLK_R0                           (  1 << 10)
#define B_MSDC3RCLK_R1                           (  1 <<  9)
#define B_MSDC3RCLK_PUPD                         (  1 <<  8)
#define B_MSDC3RCLK_BACKUP0                      (0x7 <<  5)
#define B_MSDC3RCLK_IES                          (  1 <<  4)
#define B_MSDC3RCLK_SR                           (  1 <<  3)
#define B_MSDC3RCLK_E8                           (  1 <<  2)
#define B_MSDC3RCLK_E4                           (  1 <<  1)
#define B_MSDC3RCLK_E2                           (  1 <<  0)

/* 0x10005430 MSDC3_CTRL8   MSDC 3 RCLK SEL Control Register   0x00AC */
#define B_MSDC3RCLK_BACKUP                       (0x3F << 10)
#define B_MSDC3RCLK_RDSEL                        (0x3F <<  4)
#define B_MSDC3RCLK_TDSEL                        (0x0F <<  0)

typedef enum __MSDC_PIN_STATE {
     MSDC_PST_PU_HIGHZ = 0,
     MSDC_PST_PD_HIGHZ,				 
     MSDC_PST_PU_50KOHM,               
     MSDC_PST_PD_50KOHM,				 
     MSDC_PST_PU_10KOHM,               
     MSDC_PST_PD_10KOHM,				 
     MSDC_PST_PU_08KOHM,               
     MSDC_PST_PD_08KOHM,
     MSDC_PST_MAX
}MSDC_PIN_STATE;

/*
  *  this definition is used for MT7623_TOPCKGEN_Registers.docx by chengbin.Gao 
  *  Zhanyong.Wang
  *  2014/12/14
  */
#if 0  
#ifndef TOPCKGEN_BASE
#define TOPCKGEN_BASE                             0x10000000
#endif
#if TOPCKGEN_BASE != 0x10000000
#error  Please check your TOPCKGEN_BASE definition!!!!
#endif
#endif

#define REG_TOPCKGEN_BASE(x)                     ((volatile unsigned int *)(topckgenbase + OFFSET_##x))

/*valid reg name list: 
    CLKMODE, CLKCFGUPDATE, 
    CLKCFG2 /CLKCFG2_SET/CLKCFG2_CLR, 
    CLKCFG3 /CLKCFG3_SET/CLKCFG3_CLR
    CLKCFG14/CLKCFG14_SET/CLKCFG14_CLR
    */
#define TOPCKGEN_CTRL(topckgenreg)               TOPCKGEN_##topckgenreg

/*
  * id valid range "MSDC[0, 3]"
  * reg valid range "CLKCFG2/3/14" 
  */

#define MSDC_CKGEN_SEL(id,reg)                    B_CKGEN_##reg##_##id##_SEL
#define MSDC_CKGEN_INV(id,reg)                    B_CKGEN_##reg##_##id##_INV
#define MSDC_CKGEN_PDN(id,reg)                    B_CKGEN_##reg##_##id##_PDN
#define MSDC_CKGEN_UPDATE(id)                     B_CKGEN_CLKCFGUPDATE_##id

#define MSDC_CKGENHCLK_SEL(id,reg)                B_CKGEN_##reg##_##id##HCLK_SEL
#define MSDC_CKGENHCLK_INV(id,reg)                B_CKGEN_##reg##_##id##HCLK_INV
//#define MSDC_CKGENHCLK_PDN(id,reg)                B_CKGEN_##reg##_##id##HCLK_PDN
#define MSDC_CKGENHCLK_UPDATE(id)                 B_CKGEN_CLKCFGUPDATE_##id##HCLK

#define OFFSET_TOPCKGEN_CLK_MODE                  0x0000
#define OFFSET_TOPCKGEN_CLK_CFG_UPDATE            0x0004
#define OFFSET_TOPCKGEN_CLK_CFG2                  0x0060
#define OFFSET_TOPCKGEN_CLK_CFG2_SET              0x0064
#define OFFSET_TOPCKGEN_CLK_CFG2_CLR              0x0068
#define OFFSET_TOPCKGEN_CLK_CFG3                  0x0070
#define OFFSET_TOPCKGEN_CLK_CFG3_SET              0x0074
#define OFFSET_TOPCKGEN_CLK_CFG3_CLR              0x0078
#define OFFSET_TOPCKGEN_CLK_CFG6                  0x00A0
#define OFFSET_TOPCKGEN_CLK_CFG6_SET              0x00A4
#define OFFSET_TOPCKGEN_CLK_CFG6_CLR              0x00A8
#define OFFSET_TOPCKGEN_CLK_CFG14                 0x00E0
#define OFFSET_TOPCKGEN_CLK_CFG14_SET             0x00E4
#define OFFSET_TOPCKGEN_CLK_CFG14_CLR             0x00E8


/* 
  * 0x10000000 CLK_MODE  Clock 26MHz, 32KHz PDN Control Register  0x00000000 
  *************************************************************************/
  
#define TOPCKGEN_CLKMODE                          REG_TOPCKGEN_BASE(TOPCKGEN_CLK_MODE)
#define B_CKGEN_CLKMODE_CGDIS                     (1 <<  0)
#define B_CKGEN_CLKMODE_PDNMD32KHZ                (1 <<  8)
#define B_CKGEN_CLKMODE_PDNCONN32KHZ              (1 << 10)

/*
 *
 * 0x10000060 CLK_CFG_2  Function Clock selection Register 2  0x00000000 
 * 0x10000064 SET CLK_CFG_2  Function Clock selection Register 2  0x00000000 
 * 0x10000068 CLR CLK_CFG_2  Function Clock selection Register 2  0x00000000 
 *************************************************************************/
#define TOPCKGEN_CLKCFG2                          REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG2)
#define TOPCKGEN_CLKCFG2_SET                      REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG2_SET)
#define TOPCKGEN_CLKCFG2_CLR                      REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG2_CLR)
#define B_CKGEN_CLKCFG2_MSDC0_SEL                 (0x7 << 24)
#define B_CKGEN_CLKCFG2_MSDC0_INV                 (  1 << 28)
#define B_CKGEN_CLKCFG2_MSDC0_PDN                 (  1 << 31)


/*
 *
 * 0x10000070 CLK_CFG_3  Function Clock selection Register 3  0x00000000 
 * 0x10000074 SET CLK_CFG_3  Function Clock selection Register 3  0x00000000 
 * 0x10000078 CLR CLK_CFG_3  Function Clock selection Register 3  0x00000000 
 *************************************************************************/
#define B_CKGEN_CLKCFG3_MSDC1_SEL                 (0x7 <<  0)
#define B_CKGEN_CLKCFG3_MSDC1_INV                 (  1 <<  4)
#define B_CKGEN_CLKCFG3_MSDC1_PDN                 (  1 <<  7)

#define B_CKGEN_CLKCFG3_MSDC2_SEL                 (0x7 <<  8)
#define B_CKGEN_CLKCFG3_MSDC2_INV                 (  1 << 12)
#define B_CKGEN_CLKCFG3_MSDC2_PDN                 (  1 << 15)

/*
 *
 * 0x100000A0 CLK_CFG_6  Function Clock selection Register 6  0x00000000 
 * 0x100000A4 SET CLK_CFG_6  Function Clock selection Register 6  0x00000000 
 * 0x100000A8 CLR CLK_CFG_6  Function Clock selection Register 6  0x00000000 
 *************************************************************************/
#define TOPCKGEN_CLKCFG6                          REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG6)
#define TOPCKGEN_CLKCFG6_SET                      REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG6_SET)
#define TOPCKGEN_CLKCFG6_CLR                      REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG6_CLR)
#define B_CKGEN_CLKCFG6_MSDC3HCLK_SEL             (0x3 << 24)
//#define B_CKGEN_CLKCFG6_MSDC3HCLK_INV                  (  1 << 28)
#define B_CKGEN_CLKCFG6_MSDC3HCLK_PDN             (  1 << 31)

/*
 *
 * 0x100000E0 CLK_CFG_14  Function Clock selection Register 14  0x00000000 
 * 0x100000E4 SET CLK_CFG_14  Function Clock selection Register 14  0x00000000 
 * 0x100000E8 CLR CLK_CFG_14  Function Clock selection Register 14  0x00000000 
 *************************************************************************/
#define TOPCKGEN_CLKCFG14                         REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG14)
#define TOPCKGEN_CLKCFG14_SET                     REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG14_SET)
#define TOPCKGEN_CLKCFG14_CLR                     REG_TOPCKGEN_BASE(TOPCKGEN_CLK_CFG14_CLR)
#define B_CKGEN_CLKCFG14_MSDC3_SEL                (0x7 <<  8)
#define B_CKGEN_CLKCFG14_MSDC3_INV                (  1 << 12)
#define B_CKGEN_CLKCFG14_MSDC3_PDN                (  1 << 15)


/* each PLL have different gears for select
 * software can used mux interface from clock management module to select */
enum {
    MSDC50_CLKSRC4HCLK_26MHZ  = 0,
    MSDC50_CLKSRC4HCLK_273MHZ,  
    MSDC50_CLKSRC4HCLK_182MHZ,  
    MSDC50_CLKSRC4HCLK_78MHZ,
    MSDC_DONOTCARE_HCLK,
    MSDC50_CLKSRC4HCLK_MAX
};

enum {
    MSDC50_CLKSRC_26MHZ  = 0,
    MSDC50_CLKSRC_400MHZ,  /* MSDCPLL_CK */
    MSDC50_CLKSRC_182MHZ,  /*MSDCPLL_D2 */
    MSDC50_CLKSRC_136MHZ,
    MSDC50_CLKSRC_156MHZ,
    MSDC50_CLKSRC_200MHZ,  /*MSDCPLL_D4 */
    MSDC50_CLKSRC_100MHZ,
    MSDC50_CLKSRC_50MHZ,
    MSDC50_CLKSRC_MAX
};

/* MSDC0/1/2 
     PLL MUX SEL List */
enum {
    MSDC30_CLKSRC_26MHZ   = 0,
    MSDC30_CLKSRC_200MHZ,
    MSDC30_CLKSRC_182MHZ,
    MSDC30_CLKSRC_91MHZ,
    MSDC30_CLKSRC_156MHZ,
    MSDC30_CLKSRC_104MHZ,
    MSDC30_CLKSRC_MAX
};

#define MSDC50_CLKSRC_DEFAULT     MSDC50_CLKSRC_200MHZ
#define MSDC30_CLKSRC_DEFAULT     MSDC30_CLKSRC_200MHZ

#if 0 
///mach/mt_pm_ldo.h
typedef enum MT65XX_POWER_VOL_TAG
{
    VOL_DEFAULT,
    VOL_0900 = 900,
    VOL_1000 = 1000,
    VOL_1100 = 1100,
    VOL_1200 = 1200,
    VOL_1300 = 1300,
    VOL_1350 = 1350,
    VOL_1500 = 1500,
    VOL_1800 = 1800,
    VOL_2000 = 2000,
    VOL_2100 = 2100,
    VOL_2500 = 2500,
    VOL_2800 = 2800,
    VOL_3000 = 3000,
    VOL_3300 = 3300,
    VOL_3400 = 3400,
    VOL_3500 = 3500,
    VOL_3600 = 3600
} MT65XX_POWER_VOLTAGE;
#endif
#if 0
/* SD_PAD_CTL0-2 were removed from MSDC IP. For IO port attr, like IES\SMT\SR\E16\E8\E4\E2\PU\PD\PUPD\R1\R0 
 * up level to msdc top. msdc top impliment by GPIO, so need sync with the GPIO designer(MingTe MT6582) 
 * for detail */
#define MSDC0_GPIO_CLK_BASE                   (GPIO_BASE + 0xC00)
#define MSDC0_GPIO_CMD_BASE                   (GPIO_BASE + 0xC10)
#define MSDC0_GPIO_DAT_BASE                   (GPIO_BASE + 0xC20)
//#define MSDC0_GPIO_PAD_BASE                   (GPIO_BASE + 0xC30)
#define MDSC0_DAT_PAD_1						  (GPIO_BASE + 0xC30)
#define MDSC0_DAT_PAD_2						  (GPIO_BASE + 0xC40)
#define MDSC0_DAT_PAD_3						  (GPIO_BASE + 0xC50)
#define MDSC0_DAT_PAD_TD_RDSEL				  (GPIO_BASE + 0xC60) //0x000A default

#define MSDC1_GPIO_CLK_BASE                   (GPIO_BASE + 0xC70)
#define MSDC1_GPIO_CMD_BASE                   (GPIO_BASE + 0xC80)
#define MSDC1_GPIO_DAT_BASE                   (GPIO_BASE + 0xC90)
#define MDSC1_DAT_PAD_1						  (GPIO_BASE + 0xCA0)
#define MDSC1_DAT_PAD_2						  (GPIO_BASE + 0xCB0)
#define MDSC1_DAT_PAD_TD_RDSEL				  (GPIO_BASE + 0xCC0) //0x00CA default
//#define MSDC1_GPIO_PAD_BASE                   (GPIO_BASE + 0xC70) //should remove it later

#define MSDC2_GPIO_CLK_BASE                   (GPIO_BASE + 0xCD0)
#define MSDC2_GPIO_CMD_BASE                   (GPIO_BASE + 0xCE0)
#define MSDC2_GPIO_DAT_BASE                   (GPIO_BASE + 0xCF0)
#define MDSC2_DAT_PAD_1						  (GPIO_BASE + 0xD00)
#define MDSC2_DAT_PAD_2						  (GPIO_BASE + 0xD10)
#define MDSC2_DAT_PAD_TD_RDSEL				  (GPIO_BASE + 0xD20) //0x00 default
//#define MSDC2_GPIO_PAD_BASE                   (GPIO_BASE + 0xC70) //should remove it later
//#define MSDC2_GPIO_PAD_BASE                   (GPIO_BASE + 0xCB0)

//#define GPIO_MSDC0_TDSEL_MASK				(0x0FUL << 0)
//#define GPIO_MSDC0_RDSEL_MASK				(0x3FUL << 4)
//These TDSEL and RDSEL mask are the same for MSDC0, 1, 2 on MT8127
#define GPIO_PAD_TDSEL_MASK                   (0xFUL  <<  0) 
#define GPIO_PAD_RDSEL_MASK                   (0x3FUL <<  4)
#define GPIO_PAD_TUNE_MASK                    (0xFUL  << 10)

#if 1
//jct debug, just for build pass, will remove it later
#define GPIO_R0_MASK                          (0x1UL  <<  0)
#define GPIO_R1_MASK                          (0x1UL  <<  1)
#endif
#if 0
#define GPIO_R0_MASK                          (0x1UL  <<  0)
#define GPIO_R1_MASK                          (0x1UL  <<  1)
#else
//these are same on MSDC0, MSDC1, MSDC2 on MT8127
#define GPIO_MSDC_CLK_R0_MASK			(0x01UL << 10)
#define GPIO_MSDC_CLK_R1_MASK			(0x01UL <<  9)
#define GPIO_MSDC_CMD_R0_MASK			(0x01UL << 10)
#define GPIO_MSDC_CMD_R1_MASK			(0x01UL <<  9)
#define GPIO_MSDC_DAT0_R0				(0x01UL << 2)
#define GPIO_MSDC_DAT1_R0				(0x01UL << 6)
#define GPIO_MSDC_DAT2_R0				(0x01UL << 10)
#define GPIO_MSDC_DAT3_R0				(0x01UL << 14)
#define GPIO_MSDC_DAT_R0_MASK	(GPIO_MSDC_DAT0_R0| GPIO_MSDC_DAT1_R0| GPIO_MSDC_DAT2_R0 |GPIO_MSDC_DAT3_R0)
#define GPIO_MSDC_DAT0_R1				(0x01UL << 1)
#define GPIO_MSDC_DAT1_R1				(0x01UL << 5)
#define GPIO_MSDC_DAT2_R1				(0x01UL << 9)
#define GPIO_MSDC_DAT3_R1				(0x01UL << 13)
#define GPIO_MSDC_DAT_R1_MASK	(GPIO_MSDC_DAT0_R1| GPIO_MSDC_DAT1_R1| GPIO_MSDC_DAT2_R1 |GPIO_MSDC_DAT3_R1)

#endif
//#define GPIO_PUPD_MASK                        (0x1UL  <<  2)
//MSDC0 
#define GPIO_MSDC0_CLK_PUPD_MASK			  (0x01UL << 8)
#define GPIO_MSDC0_CMD_PUPD_MASK			  (0x01UL << 8)

#define MSDC0_DAT_PAD_DAT0_PUPD					(0x01UL << 0)
#define MSDC0_DAT_PAD_DAT1_PUPD					(0x01UL << 4)
#define MSDC0_DAT_PAD_DAT2_PUPD					(0x01UL << 8)
#define MSDC0_DAT_PAD_DAT3_PUPD					(0x01UL << 12)
#define MSDC0_DAT_PAD_PUPD_MASK					(MSDC0_DAT_PAD_DAT0_PUPD | MSDC0_DAT_PAD_DAT1_PUPD | MSDC0_DAT_PAD_DAT2_PUPD| MSDC0_DAT_PAD_DAT3_PUPD)
#define MSDC0_DAT_PAD_DAT4_PUPD					(0x01UL << 0)
#define MSDC0_DAT_PAD_DAT5_PUPD					(0x01UL << 4)
#define MSDC0_DAT_PAD_DAT6_PUPD					(0x01UL << 8)
#define MSDC0_DAT_PAD_DAT7_PUPD					(0x01UL << 12)

//MSDC1 
#define GPIO_MSDC1_CLK_PUPD_MASK			  (0x01UL << 8)
#define GPIO_MSDC1_CMD_PUPD_MASK			  (0x01UL << 8)

#define MSDC1_DAT_PAD_DAT0_PUPD					(0x01UL << 0)
#define MSDC1_DAT_PAD_DAT1_PUPD					(0x01UL << 4)
#define MSDC1_DAT_PAD_DAT2_PUPD					(0x01UL << 8)
#define MSDC1_DAT_PAD_DAT3_PUPD					(0x01UL << 12)
#define MSDC1_DAT_PAD_PUPD_MASK					(MSDC1_DAT_PAD_DAT0_PUPD | MSDC1_DAT_PAD_DAT1_PUPD | MSDC1_DAT_PAD_DAT2_PUPD| MSDC1_DAT_PAD_DAT3_PUPD)

//MSDC2 
#define GPIO_MSDC2_CLK_PUPD_MASK			  (0x01UL << 8)
#define GPIO_MSDC2_CMD_PUPD_MASK			  (0x01UL << 8)

#define MSDC2_DAT_PAD_DAT0_PUPD					(0x01UL << 0)
#define MSDC2_DAT_PAD_DAT1_PUPD					(0x01UL << 4)
#define MSDC2_DAT_PAD_DAT2_PUPD					(0x01UL << 8)
#define MSDC2_DAT_PAD_DAT3_PUPD					(0x01UL << 12)
#define MSDC2_DAT_PAD_PUPD_MASK					(MSDC2_DAT_PAD_DAT0_PUPD | MSDC2_DAT_PAD_DAT1_PUPD | MSDC2_DAT_PAD_DAT2_PUPD| MSDC2_DAT_PAD_DAT3_PUPD)


#define GPIO_PD_MASK                          (0x1UL  <<  4)
#define GPIO_PU_MASK                          (0x1UL  <<  5)

#define GPIO_DAT0_PD_MASK                     (0x1UL  <<  0)
#define GPIO_DAT0_PU_MASK                     (0x1UL  <<  1)
#define GPIO_DAT1_PD_MASK                     (0x1UL  <<  2)
#define GPIO_DAT1_PU_MASK                     (0x1UL  <<  3)
#define GPIO_DAT2_PD_MASK                     (0x1UL  <<  4)
#define GPIO_DAT2_PU_MASK                     (0x1UL  <<  5)
#define GPIO_DAT3_PD_MASK                     (0x1UL  <<  6)
#define GPIO_DAT3_PU_MASK                     (0x1UL  <<  7)

#define GPIO_MSDC0_E2_MASK                    (0x1UL  <<  0)
#define GPIO_MSDC0_E4_MASK                    (0x1UL  <<  1)
#define GPIO_MSDC0_E8_MASK                    (0x1UL  <<  2)

#define GPIO_MSDC1_E2_MASK                    (0x1UL  <<  0)
#define GPIO_MSDC1_E4_MASK                    (0x1UL  <<  1)
#define GPIO_MSDC1_E8_MASK                    (0x1UL  <<  2)

#define GPIO_MSDC1_MSDC2_E4_MASK              (0x1UL  <<  8)
#define GPIO_MSDC1_MSDC2_E8_MASK              (0x1UL  <<  9)
#define GPIO_MSDC1_MSDC2_E16_MASK             (0x1UL  << 10)

/* T28ns PAD TX Pull-Down Driving Strength Control */
#define GPIO_MSDC0_DRVN                       (GPIO_MSDC0_E2_MASK | GPIO_MSDC0_E4_MASK | GPIO_MSDC0_E8_MASK)
#define GPIO_MSDC1_DRVN                       (GPIO_MSDC1_E2_MASK | GPIO_MSDC1_E4_MASK | GPIO_MSDC1_E8_MASK)
#define GPIO_MSDC1_MSDC2_DRVN                 (GPIO_MSDC1_MSDC2_E4_MASK | GPIO_MSDC1_MSDC2_E8_MASK | GPIO_MSDC1_MSDC2_E16_MASK)

#if 1
//jct debug, below only for build pass, will remove it later
#define GPIO_PUPD_MASK                        (0x1UL  <<  8)
#define GPIO_SR_MASK                          (0x1UL  << 12)
#define GPIO_SMT_MASK                         (0x1UL  << 13)
#define GPIO_IES_MASK                         (0x1UL  << 14)
#endif
#if 0
#define GPIO_SR_MASK                          (0x1UL  << 12)
#define GPIO_SMT_MASK                         (0x1UL  << 13)
#define GPIO_IES_MASK                         (0x1UL  << 14)
#else
//MSDC0
#define GPIO_MSDC0_CLK_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC0_CLK_IES_MASK			(0x01UL << 4)  //default 1
#define GPIO_MSDC0_CLK_SMT_MASK			(0x01UL << 11)

#define GPIO_MSDC0_CMD_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC0_CMD_IES_MASK			(0x01UL << 4)
#define GPIO_MSDC0_CMD_SMT_MASK			(0x01UL << 11)  //default 1

#define GPIO_MSDC0_DAT_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC0_DAT_IES_MASK			(0x01UL << 4)
#define GPIO_MSDC0_DAT0_SMT				(0x01UL << 3)
#define GPIO_MSDC0_DAT1_SMT				(0x01UL << 7)
#define GPIO_MSDC0_DAT2_SMT				(0x01UL << 11)
#define GPIO_MSDC0_DAT3_SMT				(0x01UL << 15)
#define GPIO_MSDC0_DAT_SMT_MASK			(GPIO_MSDC0_DAT0_SMT| GPIO_MSDC0_DAT1_SMT| GPIO_MSDC0_DAT2_SMT| GPIO_MSDC0_DAT3_SMT)
#define GPIO_MSDC0_DAT4_SMT				(0x01UL << 3)
#define GPIO_MSDC0_DAT5_SMT				(0x01UL << 7)
#define GPIO_MSDC0_DAT6_SMT				(0x01UL << 11)
#define GPIO_MSDC0_DAT7_SMT				(0x01UL << 15)
//MSDC1
#define GPIO_MSDC1_CLK_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC1_CLK_IES_MASK			(0x01UL << 4)  //default 1
#define GPIO_MSDC1_CLK_SMT_MASK			(0x01UL << 11)

#define GPIO_MSDC1_CMD_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC1_CMD_IES_MASK			(0x01UL << 4)
#define GPIO_MSDC1_CMD_SMT_MASK			(0x01UL << 11)  //default 1

#define GPIO_MSDC1_DAT_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC1_DAT_IES_MASK			(0x01UL << 4)
#define GPIO_MSDC1_DAT0_SMT				(0x01UL << 3)
#define GPIO_MSDC1_DAT1_SMT				(0x01UL << 7)
#define GPIO_MSDC1_DAT2_SMT				(0x01UL << 11)
#define GPIO_MSDC1_DAT3_SMT				(0x01UL << 15)
#define GPIO_MSDC1_DAT_SMT_MASK			(GPIO_MSDC1_DAT0_SMT| GPIO_MSDC1_DAT1_SMT| GPIO_MSDC1_DAT2_SMT| GPIO_MSDC1_DAT3_SMT)
//MSDC2
#define GPIO_MSDC2_CLK_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC2_CLK_IES_MASK			(0x01UL << 4)  //default 1
#define GPIO_MSDC2_CLK_SMT_MASK			(0x01UL << 11)

#define GPIO_MSDC2_CMD_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC2_CMD_IES_MASK			(0x01UL << 4)
#define GPIO_MSDC2_CMD_SMT_MASK			(0x01UL << 11)  //default 1

#define GPIO_MSDC2_DAT_SR_MASK			(0x01UL << 3)
#define GPIO_MSDC2_DAT_IES_MASK			(0x01UL << 4)
#define GPIO_MSDC2_DAT0_SMT				(0x01UL << 3)
#define GPIO_MSDC2_DAT1_SMT				(0x01UL << 7)
#define GPIO_MSDC2_DAT2_SMT				(0x01UL << 11)
#define GPIO_MSDC2_DAT3_SMT				(0x01UL << 15)
#define GPIO_MSDC2_DAT_SMT_MASK			(GPIO_MSDC2_DAT0_SMT| GPIO_MSDC2_DAT1_SMT| GPIO_MSDC2_DAT2_SMT| GPIO_MSDC2_DAT3_SMT)

#endif

/* add pull down/up mode define */
#define MSDC0_PULL_NONE     (2)
#define MSDC0_PU_10K        (1)
#define MSDC0_PU_50K        (3)
#define MSDC0_PU_8K         (4)
#define MSDC0_PD_10K        (0)
#define MSDC0_PD_50K        (5)
#define MSDC0_PD_8K         (6)        

#define MSDC1_PULL_NONE     (2)
#define MSDC1_PU_50K        (1)
#define MSDC1_PD_50K        (0) 

#define MSDC2_PULL_NONE     (2)
#define MSDC2_PU_50K        (1)
#define MSDC2_PD_50K        (0)
#endif

typedef enum MSDC_POWER {
    MSDC_VIO18_MC1 = 0,
    MSDC_VIO18_MC2,
    MSDC_VIO28_MC1,
    MSDC_VIO28_MC2,
    MSDC_VMC,
    MSDC_VGP6,
} MSDC_POWER_DOMAIN;


/*--------------------------------------------------------------------------*/
/* Descriptor Structure                                                     */
/*--------------------------------------------------------------------------*/
typedef struct {
    u32  hwo       :1; /* could be changed by hw */
    u32  bdp       :1;
    u32  rsv0      :6;	
    u32  chksum    :8;
    u32  intr      :1;
    u32  rsv1      :7;
    u32  nxtgpdh4b :4; /* !!!MT7623 Add*/
    u32  nxtbdh4b  :4; /* !!!MT7623 Add*/
	
    u32  next;
	
    u32  ptr;
	
    u32  buflen:24;  /* bitwidth 16 -> 24 !!!MT7623 Change*/
    u32  extlen:8;   /* bit start   8 -> 24 !!!MT7623 Change*/
    //u32  rsv2:8;     /* bit start  24 -> 31 !!!MT7623 removed*/

    u32  arg;
	
    u32  blknum;
	
    u32  cmd;
} gpd_t;

typedef struct {
    u32  eol       :1;
    u32  rsv0      :7;	
    u32  chksum    :8;	
    u32  rsv1      :1;
    u32  blkpad    :1;
    u32  dwpad     :1;	
    u32  rsv2      :5;	
    u32  nxtbdh4b  :4; /* !!!MT7623 Add*/
    u32  nxtDath4b :4; /* !!!MT7623 Add*/
	
    u32  next;
	
    u32  ptr;
	
    u32  buflen    :24; /* bitwidth 16 -> 24 !!!MT7623 Change*/
    u32  rsv3      : 8; /* bitwidth 16 -> 8 !!!MT7623 Change*/
} bd_t;

struct scatterlist_ex {
    u32 cmd;
    u32 arg;
    u32 sglen;
    struct scatterlist *sg;
};

#define DMA_FLAG_NONE       (0x00000000)
#define DMA_FLAG_EN_CHKSUM  (0x00000001)
#define DMA_FLAG_PAD_BLOCK  (0x00000002)
#define DMA_FLAG_PAD_DWORD  (0x00000004)

struct msdc_dma {
    u32 flags;                   /* flags */
    u32 xfersz;                  /* xfer size in bytes */
    u32 sglen;                   /* size of scatter list */
    u32 blklen;                  /* block size */
    struct scatterlist *sg;      /* I/O scatter list */
    struct scatterlist_ex *esg;  /* extended I/O scatter list */
    u8  mode;                    /* dma mode        */
    u8  burstsz;                 /* burst size      */
    u8  intr;                    /* dma done interrupt */
    u8  padding;                 /* padding */
    u32 cmd;                     /* enhanced mode command */
    u32 arg;                     /* enhanced mode arg */
    u32 rsp;                     /* enhanced mode command response */
    u32 autorsp;                 /* auto command response */

    gpd_t *gpd;                  /* pointer to gpd array */
    bd_t  *bd;                   /* pointer to bd array */
    dma_addr_t gpd_addr;         /* the physical address of gpd array */
    dma_addr_t bd_addr;          /* the physical address of bd array */
    u32 used_gpd;                /* the number of used gpd elements */
    u32 used_bd;                 /* the number of used bd elements */
};

struct tune_counter
{
	u32 time_cmd;
	u32 time_read;
	u32 time_write;
};
struct msdc_saved_para
{
	u32							pad_tune;
	u32							ddly0;
	u32							ddly1;
	u8							cmd_resp_ta_cntr;
	u8							wrdat_crc_ta_cntr;
	u8							suspend_flag;
	u32 						msdc_cfg;
	u32 						mode;
	u32 						div;
	u32 						sdc_cfg;
	u32 						iocon;
	int             ddr;
	u32             hz;
	u8							int_dat_latch_ck_sel;
	u8							ckgen_msdc_dly_sel;
	u8							inten_sdio_irq;
	u8							write_timeout_eco1; /* for write: 3T need wait before host check busy after crc status */
	u8							write_timeout_eco2; /* for write: host check timeout change to 16T */
};

#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
struct ot_data
{
    u32 eco_ver;
    u32 orig_blknum;
    u32 orig_patch_bit0;
    u32 orig_iocon;

#define DMA_ON 0
#define DMA_OFF 1
    u32 orig_dma;
    u32 orig_cmdrdly;
    u32 orig_ddlsel;
    u32 orig_paddatrddly;
    u32 orig_paddatwrdly;
    u32 orig_dat0rddly;
    u32 orig_dat1rddly;
    u32 orig_dat2rddly;
    u32 orig_dat3rddly;
    u32 orig_dtoc;

    u32 cmdrdly;
    u32 datrddly;
    u32 dat0rddly;
    u32 dat1rddly;
    u32 dat2rddly;
    u32 dat3rddly;

    u32 cmddlypass;
    u32 datrddlypass;
    u32 dat0rddlypass;
    u32 dat1rddlypass;
    u32 dat2rddlypass;
    u32 dat3rddlypass;

    u32 fCmdTestedGear;
    u32 fDatTestedGear;
    u32 fDat0TestedGear;
    u32 fDat1TestedGear;
    u32 fDat2TestedGear;
    u32 fDat3TestedGear;

    u32 rawcmd;
    u32 rawarg;
    u32 tune_wind_size;
    u32 fn;
    u32 addr;
	u32 retry;
};

struct ot_work_t
{
	struct delayed_work ot_delayed_work;
	struct msdc_host *host;
	int         chg_volt;
	atomic_t    ot_disable;
	atomic_t    autok_done;
	atomic_t    need_ot;        // For MD sleep/OT handshake
	atomic_t    dev_sleep_sts;   // For MD sleep/OT handshake
	struct      completion ot_complete;
};
#endif // MTK_SDIO30_ONLINE_TUNING_SUPPORT

struct msdc_host
{
    struct msdc_hw              *hw;

    struct mmc_host             *mmc;           /* mmc structure */
    struct mmc_command          *cmd;
    struct mmc_data             *data;
    struct mmc_request          *mrq; 
    int                         cmd_rsp;
    int                         cmd_rsp_done;
    int                         cmd_r1b_done;

    int                         error; 
    spinlock_t                  lock;           /* mutex */
    spinlock_t                  clk_gate_lock;
	spinlock_t                  remove_bad_card;	/*to solve removing bad card race condition with hot-plug enable*/
    int                         clk_gate_count;
    struct semaphore            sem; 

    u32                         blksz;          /* host block size */
    u32                         base;           /* host base address */    
    u32                         fpgagpiobase;           /* host base address */    
    u32                         gpiobase;           /* host base address */    
    u32                         ckgenbase;           /* host base address */    
    u32                         id;             /* host id */
    int                         pwr_ref;        /* core power reference count */

    u32                         xfer_size;      /* total transferred size */

    struct msdc_dma             dma;            /* dma channel */
    u32                         dma_addr;       /* dma transfer address */
    u32                         dma_left_size;  /* dma transfer left size */
    u32                         dma_xfer_size;  /* dma transfer size in bytes */
    int                         dma_xfer;       /* dma transfer mode */

    u32                         timeout_ns;     /* data timeout ns */
    u32                         timeout_clks;   /* data timeout clks */

    atomic_t                    abort;          /* abort transfer */

    int                         irq;            /* host interrupt */

    struct tasklet_struct       card_tasklet;
	//struct delayed_work       	remove_card;
#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
  #ifdef MTK_SDIO30_DETECT_THERMAL
	int	                        pre_temper;	    /* previous set temperature */
	struct timer_list           ot_timer;
	bool                        ot_period_check_start;
  #endif  // MTK_SDIO30_DETECT_THERMAL
	struct ot_work_t            ot_work;
	atomic_t                    ot_done;
#endif // MTK_SDIO30_ONLINE_TUNING_SUPPORT
    atomic_t                    sdio_stopping;

    struct completion           cmd_done;
    struct completion           xfer_done;
    struct pm_message           pm_state;

    u32                         mclk;           /* mmc subsystem clock */
    u32                         hclk;           /* host clock speed */		
    u32                         sclk;           /* SD/MS clock speed */
    u8                          core_clkon;     /* Host core clock on ? */
    u8                          card_clkon;     /* Card clock on ? */
    u8                          core_power;     /* core power */    
    u8                          power_mode;     /* host power mode */
    u8                          card_inserted;  /* card inserted ? */
    u8                          suspend;        /* host suspended ? */    
    u8                          sdio_suspend;   /* sdio suspend ? */
    u8                          app_cmd;        /* for app command */     
    u32                         app_cmd_arg;    
    u64                         starttime;
    struct timer_list           timer;     
    struct tune_counter         t_counter;
	u32							rwcmd_time_tune;
	int							read_time_tune;
    int                         write_time_tune;
	u32							write_timeout_uhs104;
	u32							read_timeout_uhs104;
	u32							write_timeout_emmc;
	u32							read_timeout_emmc;
	u8						    autocmd;
	u32							sw_timeout;
	u32							power_cycle; /* power cycle done in tuning flow*/
	bool						power_cycle_enable;/*Enable power cycle*/
	u32							sd_30_busy;
	bool						tune;
	MSDC_POWER_DOMAIN			power_domain;
	bool						ddr;
	struct msdc_saved_para		saved_para;	
	int 						sd_cd_polarity;
	int							sd_cd_insert_work; //to make sure insert mmc_rescan this work in start_host when boot up
												   //driver will get a EINT(Level sensitive) when boot up phone with card insert
	bool						block_bad_card;											   
	void	(*power_control)(struct msdc_host *host,u32 on);
	void	(*power_switch)(struct msdc_host *host,u32 on);
};
typedef enum {
   TRAN_MOD_PIO,
   TRAN_MOD_DMA,
   TRAN_MOD_NUM
}transfer_mode;

typedef enum {
   OPER_TYPE_READ,
   OPER_TYPE_WRITE,
   OPER_TYPE_NUM
}operation_type;

struct dma_addr{
   u32 start_address;
   u32 size;
   u8 end; 
   struct dma_addr *next;
};

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

#ifdef FPGA_PLATFORM
#define DBGCHK(field, val)                 (((~(field)) & ((val) << (uffs(field) - 1))) ? printk("!!!WARNING Override(F.%x vs V.%x):%s@%u\n",field, ((val) << (uffs(field) - 1)),__func__, __LINE__), 1 : 0)
#define DMAGPDPTR_DBGCHK(gpdaddr, align)   ((((unsigned long)(gpdaddr)) & ((align)-1L)) ? printk("!!!ERROR GPD Address has required HW %uB alignment %p):%s@%u\n",align,(gpdaddr), __func__, __LINE__), 1 : 0)
#define DMABDPTR_DBGCHK(bdaddr, align)     ((((unsigned long)(bdaddr))  & ((align)-1L)) ? printk("!!!ERROR BD  Address has required HW %uB alignment %p):%s@%u\n",align,(bdaddr) , __func__, __LINE__), 1 : 0)
#define DMAARGPTR_DBGCHK(arg, align)       ((((unsigned long)(arg))     & ((align)-1)) ? printk("!!!ERROR ARG Address has required HW %uB alignment %x):%s@%u\n",align,(arg)    , __func__, __LINE__), 1 : 0)
#else
#define DBGCHK(field, val)
#define DMAGPDPTR_DBGCHK(gpdaddr, align)
#define DMABDPTR_DBGCHK(bdaddr, align)
#define DMAARGPTR_DBGCHK(arg, align)
#endif

#define sdr_read8(reg)           __raw_readb((const volatile void *)reg)
#define sdr_read16(reg)          __raw_readw((const volatile void *)reg)
#define sdr_read32(reg)          __raw_readl((const volatile void *)reg)
#if 0
#define sdr_write8(reg,val)      __raw_writeb(val,reg)
#define sdr_write16(reg,val)     __raw_writew(val,reg)
#define sdr_write32(reg,val)     __raw_writel(val,reg)
#define sdr_set_bits(reg,bs)     ((*(volatile u32*)(reg)) |= (u32)(bs))
#define sdr_clr_bits(reg,bs)     ((*(volatile u32*)(reg)) &= ~((u32)(bs)))
#else
#define sdr_write8(reg,val)      mt65xx_reg_sync_writeb(val,reg)
#define sdr_write16(reg,val)     mt65xx_reg_sync_writew(val,reg)
#define sdr_write32(reg,val)     mt65xx_reg_sync_writel(val,reg)
#define sdr_set_bits(reg,bs) \
	do{\
		volatile unsigned int tv = sdr_read32(reg);\
		tv |= (u32)(bs); \
		sdr_write32(reg,tv); \
	}while(0)
#define sdr_clr_bits(reg,bs) \
do{\
		volatile unsigned int tv = sdr_read32(reg);\
		tv &= ~((u32)(bs)); \
		sdr_write32(reg,tv); \
	}while(0)

#endif

#define sdr_set_field(reg,field,val) \
    do {	\
        volatile unsigned int tv = sdr_read32(reg);	\
        tv &= ~(field); \
        tv |= ((val) << (uffs((unsigned int)field) - 1)); \
        sdr_write32(reg,tv); \
    } while(0)
#define sdr_get_field(reg,field,val) \
    do {	\
        volatile unsigned int tv = sdr_read32(reg);	\
        val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
    } while(0)
#define sdr_set_field_discrete(reg,field,val) \
			do {	\
				volatile unsigned int tv = sdr_read32(reg); \
				tv = (val == 1) ? (tv|(field)):(tv & ~(field));\
				sdr_write32(reg,tv); \
			} while(0)
#define sdr_get_field_discrete(reg,field,val) \
			do {	\
				volatile unsigned int tv = sdr_read32(reg); \
				val = tv & (field) ; \
				val = (val == field) ? 1 :0;\
			} while(0)

#endif /* end of MT6592_SD_H */


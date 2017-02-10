#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>

#if defined (CONFIG_MT7623_FPGA)
#define I2C_PCIEPHY_ACCESS_BYTE		(1)
#define I2C_PCIEPHY_ADDR		(0x60)
#endif
#if defined (CONFIG_ARCH_MT7623)
#define TOP_BASE	(0xF0000000)
#define SYSCTL_BASE	HIFSYS_BASE
#define PCIE_BASE	HIFSYS_PCI_BASE
#define USB2_BASE	HIFSYS_USB_HOST2_BASE
#define PCIEP0_BASE	(PCIE_BASE + 0x2000)
#define PCIEP1_BASE	(PCIE_BASE + 0x3000)
#define PCIEP2_BASE	(PCIE_BASE + 0x4000)
#define MEM_WIN		(0x1A150000)
#define IO_WIN		(0x1A160000)
#define MEM_DIRECT1	(0x60000000)
#define MEMORY_BASE 0x80000000
#endif

#define REGADDR(x, y)	(x##_BASE + y)
#define REGDATA(x)			*((volatile unsigned int *)(x))	

#define GPIOMODE5	REGADDR(TOP, 0x57A0)
#define GPIOMODE42	REGADDR(TOP, 0x59F0)
#define GPIOMODE51	REGADDR(TOP, 0x5A80)
#define GPIOMODE52	REGADDR(TOP, 0x5A90)
#define SWID		REGADDR(TOP, 0x700000C)
#define SYSCFG1		REGADDR(SYSCTL, 0x14)
#define CLKCFG0		REGADDR(SYSCTL, 0x2C)
#define CLKCFG1		REGADDR(SYSCTL, 0x30)
#define RSTCTL		REGADDR(SYSCTL, 0x34)
#define GPIOMODE	REGADDR(SYSCTL, 0x60)
#define PCICFG		REGADDR(PCIE, 0x0)
#define PCIINT		REGADDR(PCIE, 0x8)
#define PCIENA		REGADDR(PCIE, 0xC)
#define CFGADDR		REGADDR(PCIE, 0x20)
#define CFGDATA		REGADDR(PCIE, 0x24)
#define MEMBASE		REGADDR(PCIE, 0x28)
#define IOBASE		REGADDR(PCIE, 0x2C)
#define SRAM_EN		REGADDR(PCIE, 0xB0)
#define BAR0SETUP(x)	REGADDR(PCIE##x, 0x10)
#define BAR1SETUP(x)	REGADDR(PCIE##x, 0x14)
#define IMBASEBAR0(x)	REGADDR(PCIE##x, 0x18)
#define PCIE_ID(x)	REGADDR(PCIE##x, 0x30)
#define PCIE_CLASS(x)	REGADDR(PCIE##x, 0x34)
#define PCIE_SUBID(x)	REGADDR(PCIE##x, 0x38)
#define PCIE_SISTAT(x)	REGADDR(PCIE##x, 0x50)
#define DLLECR(x)	REGADDR(PCIE##x, 0x60)
#define ECRCCR(x)	REGADDR(PCIE##x, 0x64)
#define LTSSM_DELAY(x)	REGADDR(PCIE##x, 0x70)
#define PHY_RST		REGADDR(PCIE, 0x8000)
#define PHY_EN		REGADDR(PCIE, 0x8004)
#define PHY_P0_CTL	REGADDR(PCIE, 0x9000)
#define PHY_P1_CTL	REGADDR(PCIE, 0xA000)
#define PHY_P2_CTL	REGADDR(USB2, 0x4000)

#define PERST_MODE(x)	(x<<10)
#define PCIINT0_EN(x)	(x<<20)
#define PCIINT1_EN(x)	(x<<21)
#define PCIINT2_EN(x)	(x<<22)
#define PCIRST0(x)	(x<<1)
#define PCIRST1(x)	(x<<2)
#define PCIRST2(x)	(x<<3)
#define EXTREGNUM(x)	(x<<24)
#define BUSNUM(x)	(x<<16)
#define DEVICENUM(x)	(x<<11)
#define FUNNUM(x)	(x<<8)
#define REGNUM(x)	(x<<0)
	
#define P0_PHY_RST		1<<0
#define P1_PHY_RST		1<<1
#define CSR_TRGT_HI_PERF_EN	1<<11
#define CSR_TRGT_HI_PERF_EN1	1<<12
#define CSR_TRGT_HI_PERF_EN2	1<<13

#define PCIE_DBG_MSG	0x1
#define PCIE_PHY_MSG	0x2
#define CR_CEHCK_MSG	0x4
#define DRV_INFO_MSG	0x8
#define PCIE_PRINT(level, fmt, args...) do { \
						if ((pcie_debug & level) > 0) \
							printk(fmt, ##args); \
					} while (0)


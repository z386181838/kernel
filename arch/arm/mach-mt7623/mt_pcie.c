/*
 *  linux/arch/arm/mach-versatile/pci.c
 *
 * (C) Copyright Koninklijke Philips Electronics NV 2004. All rights reserved.
 * You can redistribute and/or modify this software under the terms of version 2
 * of the GNU General Public License as published by the Free Software Foundation.
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * Koninklijke Philips Electronics nor its subsidiaries is obligated to provide any support for this software.
 *
 * ARM Versatile PCI driver.
 *
 * 14/04/2005 Initial version, colin.king@philips.com
 *
 */
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/pci.h>
#include "mt_pcie.h"

extern unsigned long switch_address_bytes(unsigned long addr_bytes);
extern unsigned int switch_device_address(unsigned int dev_addr);
extern void pcie_phy_calibration(unsigned int port, int debug);
int pcie_link_status = 0;
int pcie_debug = DRV_INFO_MSG;
#define PCIE_LINK_RETRY 1

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, 
			    unsigned long reg, unsigned long *val)
{
	unsigned int address;

	/* set addr */
	address = EXTREGNUM(((reg & 0xf00)>>8)) | BUSNUM(bus) | 
		  DEVICENUM(dev) | FUNNUM(func) | 
		  REGNUM(reg & 0xfc) ;
	PCIE_PRINT(PCIE_DBG_MSG, "%s: bus=%x, dev=%lu, func=%x, reg=0x%x, address=0x%x\n", __func__,  bus, dev, func, reg, address);
	/* start the configuration cycle */
	REGDATA(CFGADDR) = (REGDATA(CFGADDR) & 0xf0000000) | address;
	/* read the data */
	*val = REGDATA(CFGDATA);

	return;
}

void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, 
			     unsigned long reg, unsigned long val)
{
	unsigned int address;

	/* set addr */
	address = EXTREGNUM(((reg & 0xf00)>>8)) | BUSNUM(bus) | 
		  DEVICENUM(dev) | FUNNUM(func) | 
		  REGNUM(reg & 0xfc) ;

	PCIE_PRINT(PCIE_DBG_MSG, "%s: bus=%x, dev=%lu, func=%x, reg=0x%x, address=0x%x\n", __func__,  bus, dev, func, reg, address);
	/* start the configuration cycle */
	REGDATA(CFGADDR) = (REGDATA(CFGADDR) & 0xf0000000) | address;
	/* read the data */
	REGDATA(CFGDATA) = val;

	return;
}

static int config_read_config(struct pci_bus *bus, unsigned int devfn, int where,
			      int size, u32 *val)
{
	unsigned int slot = PCI_SLOT(devfn);
	u8 func = PCI_FUNC(devfn);
	u32 data;
	u8 offset = (u8)where & 0x3;
	
	read_config(bus->number, slot, (u32)func, where, (unsigned long *)&data);

	switch (size) {
	case 1:
		*val = (data>>(offset*8)) & 0xff;
		break;
	case 2:
		*val = (data>>(offset*8)) & 0xffff;
		break;
	default:
		*val = data;
	}
	PCIE_PRINT(PCIE_DBG_MSG, "%s: bus=%d, dev=%d, func=%d, offset=0x%x, size=%d, data=0x%x\n", 
				  __func__,  bus->number, slot, 
				  (u32)func, where, size, data);

	return PCIBIOS_SUCCESSFUL;
}

static int config_write_config(struct pci_bus *bus, unsigned int devfn, int where,
			       int size, u32 val)
{
	unsigned int slot = PCI_SLOT(devfn);
	u8 func = PCI_FUNC(devfn);
	u32 data;
	u8 offset = (u8)where & 0x3;

	read_config(bus->number, slot, (u32)func, where, (unsigned long *)&data);

	PCIE_PRINT(PCIE_DBG_MSG, "%s: bus=%d, dev=%d, func=%d, offset=0x%x, rdata=0x%x, wdata=0x%x\n", __func__,  bus->number, slot, (u32)func, where, data, val);
	switch (size) {
	case 1:
		data &= ~(0xff<<(offset*8));
		data |= (val&0xff)<<(offset*8);
		break;
	case 2:
		data &= ~(0xffff<<(offset*8));
		data |= (val&0xffff)<<(offset*8);;
		break;
	default:
		data = val;
	}
	//PCIE_PRINT("%s: size=%d, wdata=0x%x\n", __func__, size, data);
	write_config(bus->number, slot, (u32)func, where, data);

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops mtk_pcie_ops = {
	.read   = config_read_config,
	.write  = config_write_config,
};

static struct resource pci_mem = {
	.name   = "PCIe Memory space",
	.start  = MEM_DIRECT1,
	.end    = (u32)((MEM_DIRECT1 + (unsigned char *)0x0fffffff)),
	.flags  = IORESOURCE_MEM,
};

static struct resource pci_io = {
	.name   = "PCIe IO space",
	.start  = IO_WIN,
	.end    = (u32)((IO_WIN + (unsigned char *)0x0ffff)),
	.flags  = IORESOURCE_IO,
};

int __init mtk_pcie_setup(int nr, struct pci_sys_data *sys)
{
	if (pcie_link_status == 0)
		return 0;
	request_resource(&ioport_resource, &pci_io);
	request_resource(&iomem_resource, &pci_mem);

	pci_add_resource_offset(&sys->resources, &pci_io, sys->io_offset);
	pci_add_resource_offset(&sys->resources, &pci_mem, sys->mem_offset);

	return 1;
}

struct pci_bus * __init mtk_pcie_scan_bus(int nr, struct pci_sys_data *sys)
{
	if (pcie_link_status == 0)
		return NULL;
	return pci_scan_root_bus(NULL, sys->busnr, &mtk_pcie_ops, sys,
				 &sys->resources);
}

static int __init mtk_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	int i, irq = -1;
	struct resource *res;
	u16 cmd;
	u32 val;

	if (pcie_link_status == 0)
		return irq;
	if ((dev->bus->number ==0) && (slot == 0)) {
		write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 0, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
		PCIE_PRINT(DRV_INFO_MSG, "BAR0 at bus %d, slot %d\n", dev->bus->number, slot);
	} else if ((dev->bus->number ==0) && (slot == 1)) {
		write_config(0, 1, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 1, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
		PCIE_PRINT(DRV_INFO_MSG, "BAR0 at bus %d, slot %d\n", dev->bus->number, slot);
	} else if ((dev->bus->number ==0) && (slot == 2)) {
		write_config(0, 2, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 2, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
		PCIE_PRINT(DRV_INFO_MSG, "BAR0 at bus %d, slot %d\n", dev->bus->number, slot);
	} else if (dev->bus->number == 1) {
		switch (pcie_link_status) {
		case 2:
		case 6:
			irq = RALINK_INT_PCIE1;
			break;
		case 4:
			irq = RALINK_INT_PCIE2;
			break;
		default:
			irq = RALINK_INT_PCIE0;
		}
		PCIE_PRINT(DRV_INFO_MSG, "bus=0x%x, slot = 0x%x, pin=0x%x, irq=0x%x\n", dev->bus->number, slot, pin, irq);
	} else if (dev->bus->number == 2) {
		switch (pcie_link_status) {
		case 5:
		case 6:
			irq = RALINK_INT_PCIE2;
			break;
		default:
			irq = RALINK_INT_PCIE1;
		}
		PCIE_PRINT(DRV_INFO_MSG, "bus=0x%x, slot = 0x%x, pin=0x%x, irq=0x%x\n", dev->bus->number, slot, pin, irq);
	} else if (dev->bus->number == 3) {
		irq = RALINK_INT_PCIE2;
		PCIE_PRINT(DRV_INFO_MSG, "bus=0x%x, slot = 0x%x, pin=0x%x, irq=0x%x\n", dev->bus->number, slot, pin, irq);
	} 
	for(i=0;i<6;i++){
		res = (struct resource *) &dev->resource[i];
		PCIE_PRINT(PCIE_DBG_MSG, "res[%d]->name = %s\n", i, res->name);
		PCIE_PRINT(PCIE_DBG_MSG, "res[%d]->start = %x\n", i, res->start);
		PCIE_PRINT(PCIE_DBG_MSG, "res[%d]->end = %x\n", i, res->end);
		PCIE_PRINT(PCIE_DBG_MSG, "res[%d]->flags = %x\n", i, (unsigned int)res->flags);
	}

	pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 0x14);  //configure cache line size 0x14
	pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0xFF);  //configure latency timer 0x10
	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
	pci_write_config_word(dev, PCI_COMMAND, cmd);
	pci_write_config_byte(dev, PCI_INTERRUPT_LINE, irq);

	return irq;
}

void __init gpio_share_switch(void)
{
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE5 SHAREPIN (%x) =%x\n", __func__, GPIOMODE5, REGDATA(GPIOMODE5));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE42 SHAREPIN (%x) =%x\n", __func__, GPIOMODE42, REGDATA(GPIOMODE42));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE51 SHAREPIN (%x) =%x\n", __func__, GPIOMODE51, REGDATA(GPIOMODE51));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE52 SHAREPIN (%x) =%x\n", __func__, GPIOMODE52, REGDATA(GPIOMODE52));
#if defined (CONFIG_PCIE_PORT0)
	REGDATA(GPIOMODE42) &= ~(0xE00);
	REGDATA(GPIOMODE42) |= 0x600;	/* GPIOMODE42 [11:9] = 3'h3 for PCIE0_PERST_N */
	REGDATA(GPIOMODE51) &= ~(0x3F);
	REGDATA(GPIOMODE51) |= 0x6;	/* GPIOMODE51 [2:0] = 3'h6 for PCIE0_CLKREQ_N */
	REGDATA(GPIOMODE51) |= 0x30;	/* GPIOMODE51 [5:3] = 3'h6 for PCIE0_WAKE_N */
#endif
#if defined (CONFIG_PCIE_PORT1)
	REGDATA(GPIOMODE42) &= ~(0x7000);
	REGDATA(GPIOMODE42) |= 0x3000;	/* GPIOMODE42 [14:12] = 3'h3 for PCIE1_PERST_N*/
	REGDATA(GPIOMODE51) &= ~(0xFC0);
	REGDATA(GPIOMODE51) |= 0x180;	/* GPIOMODE51 [8:6] = 3'h6 for PCIE1_CLKREQ_N */
	REGDATA(GPIOMODE51) |= 0xC00;	/* GPIOMODE51 [11:9] = 3'h6 for PCIE1_WAKE_N */
#endif
#if defined (CONFIG_PCIE_PORT2)
	REGDATA(GPIOMODE5) &= ~(0x7000);
	REGDATA(GPIOMODE5) |= 0x2000;	/* GPIOMODE5 [14:12] = 3'h2 PCIE2_PERST_N */
	REGDATA(GPIOMODE51) &= ~(0x7000);
	REGDATA(GPIOMODE51) |= 0x6000;	/* GPIOMODE51 [14:12] = 3'h6 for PCIE2_CLKREQ_N */
	REGDATA(GPIOMODE52) &= ~(0x7);
	REGDATA(GPIOMODE52) |= 0x6;	/* GPIOMODE52 [2:0] = 3'h6 for PCIE2_WAKE_N */
#endif
	mdelay(10);
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE5 SHAREPIN (%x) =%x\n", __func__, GPIOMODE5, REGDATA(GPIOMODE5));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE42 SHAREPIN (%x) =%x\n", __func__, GPIOMODE42, REGDATA(GPIOMODE42));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE51 SHAREPIN (%x) =%x\n", __func__, GPIOMODE51, REGDATA(GPIOMODE51));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: GPIO_MODE52 SHAREPIN (%x) =%x\n", __func__, GPIOMODE52, REGDATA(GPIOMODE52));
}

void __init pcie_phy_config(void)
{
#if defined (CONFIG_PCIE_PORT0)
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC00, REGDATA(PHY_P0_CTL+0xC00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0x9E0, REGDATA(PHY_P0_CTL+0x9E0));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0x9E4, REGDATA(PHY_P0_CTL+0x9E4));
	REGDATA(PHY_P0_CTL+0xC00) &= ~(0x33000); 
	REGDATA(PHY_P0_CTL+0xC00) |=   0x22000;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xB04, REGDATA(PHY_P0_CTL+0xB04));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xB00, REGDATA(PHY_P0_CTL+0xB00));
	REGDATA(PHY_P0_CTL+0xB04) &= ~(0xe0000000); 
	REGDATA(PHY_P0_CTL+0xB04) |=   0x40000000;
	REGDATA(PHY_P0_CTL+0xB00) &= ~(0xe); 
	REGDATA(PHY_P0_CTL+0xB00) |=   0x4;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC3C, REGDATA(PHY_P0_CTL+0xC3C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC48, REGDATA(PHY_P0_CTL+0xC48));
	REGDATA(PHY_P0_CTL+0xC3C) &= ~(0xffff0000); 
	REGDATA(PHY_P0_CTL+0xC3C) |=   0x3c0000;
	REGDATA(PHY_P0_CTL+0xC48) &= ~(0xffff); 
	REGDATA(PHY_P0_CTL+0xC48) |=   0x36;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC0C, REGDATA(PHY_P0_CTL+0xC0C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC08, REGDATA(PHY_P0_CTL+0xC08));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC10, REGDATA(PHY_P0_CTL+0xC10));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC14, REGDATA(PHY_P0_CTL+0xC14));
	REGDATA(PHY_P0_CTL+0xC0C) &= ~(0x30000000); 
	REGDATA(PHY_P0_CTL+0xC0C) |=   0x10000000;
	REGDATA(PHY_P0_CTL+0xC08) &= ~(0x3800c0);
	REGDATA(PHY_P0_CTL+0xC08) |=   0xc0;
	REGDATA(PHY_P0_CTL+0xC10) &= ~(0xf0000); 
	REGDATA(PHY_P0_CTL+0xC10) |=   0x20000;
	REGDATA(PHY_P0_CTL+0xC0C) &= ~(0xf000); 
	REGDATA(PHY_P0_CTL+0xC0C) |=   0x1000;
	REGDATA(PHY_P0_CTL+0xC14) &= ~(0xf0000); 
	REGDATA(PHY_P0_CTL+0xC14) |=   0xa0000;
#endif
#if 0 /* RG_PCIE_SIGDET_VTH */
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xB28, REGDATA(PHY_P0_CTL+0xB28));
	REGDATA(PHY_P0_CTL+0xB28) &= ~(0x180000); 
	REGDATA(PHY_P0_CTL+0xB28) |=  (0x180000); 
#endif
#if 0 /* SSC Disable */
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0x938, REGDATA(PHY_P0_CTL+0x938));
	REGDATA(PHY_P0_CTL+0x938) &= ~(0xC000);
	REGDATA(PHY_P0_CTL+0x938) |=  (0x8000);
#endif
	mdelay(10);
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC00, REGDATA(PHY_P0_CTL+0xC00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0x9E0, REGDATA(PHY_P0_CTL+0x9E0));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0x9E4, REGDATA(PHY_P0_CTL+0x9E4));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0x97C, REGDATA(PHY_P0_CTL+0x97C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xB04, REGDATA(PHY_P0_CTL+0xB04));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xB00, REGDATA(PHY_P0_CTL+0xB00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC3C, REGDATA(PHY_P0_CTL+0xC3C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC48, REGDATA(PHY_P0_CTL+0xC48));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC0C, REGDATA(PHY_P0_CTL+0xC0C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC08, REGDATA(PHY_P0_CTL+0xC08));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC10, REGDATA(PHY_P0_CTL+0xC10));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xC14, REGDATA(PHY_P0_CTL+0xC14));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0x938, REGDATA(PHY_P0_CTL+0x938));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P0_CTL+0xB28, REGDATA(PHY_P0_CTL+0xB28));
#if defined (CONFIG_PCIE_PORT1)
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC00, REGDATA(PHY_P1_CTL+0xC00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0x9E0, REGDATA(PHY_P1_CTL+0x9E0));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0x9E4, REGDATA(PHY_P1_CTL+0x9E4));
	REGDATA(PHY_P1_CTL+0xC00) &= ~(0x33000); 
	REGDATA(PHY_P1_CTL+0xC00) |=   0x22000;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xB04, REGDATA(PHY_P1_CTL+0xB04));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xB00, REGDATA(PHY_P1_CTL+0xB00));
	REGDATA(PHY_P1_CTL+0xB04) &= ~(0xe0000000); 
	REGDATA(PHY_P1_CTL+0xB04) |=   0x40000000;
	REGDATA(PHY_P1_CTL+0xB00) &= ~(0xe); 
	REGDATA(PHY_P1_CTL+0xB00) |=   0x4;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC3C, REGDATA(PHY_P1_CTL+0xC3C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC48, REGDATA(PHY_P1_CTL+0xC48));
	REGDATA(PHY_P1_CTL+0xC3C) &= ~(0xffff0000); 
	REGDATA(PHY_P1_CTL+0xC3C) |=   0x3c0000;
	REGDATA(PHY_P1_CTL+0xC48) &= ~(0xffff); 
	REGDATA(PHY_P1_CTL+0xC48) |=   0x36;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC0C, REGDATA(PHY_P1_CTL+0xC0C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC08, REGDATA(PHY_P1_CTL+0xC08));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC10, REGDATA(PHY_P1_CTL+0xC10));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC14, REGDATA(PHY_P1_CTL+0xC14));
	REGDATA(PHY_P1_CTL+0xC0C) &= ~(0x30000000); 
	REGDATA(PHY_P1_CTL+0xC0C) |=   0x10000000;
	REGDATA(PHY_P1_CTL+0xC08) &= ~(0x3800c0);
	REGDATA(PHY_P1_CTL+0xC08) |=   0xc0;
	REGDATA(PHY_P1_CTL+0xC10) &= ~(0xf0000); 
	REGDATA(PHY_P1_CTL+0xC10) |=   0x20000;
	REGDATA(PHY_P1_CTL+0xC0C) &= ~(0xf000); 
	REGDATA(PHY_P1_CTL+0xC0C) |=   0x1000;
	REGDATA(PHY_P1_CTL+0xC14) &= ~(0xf0000); 
	REGDATA(PHY_P1_CTL+0xC14) |=   0xa0000;
#if 0 /* RG_PCIE_SIGDET_VTH */
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xB28, REGDATA(PHY_P1_CTL+0xB28));
	REGDATA(PHY_P1_CTL+0xB28) &= ~(0x180000); 
	REGDATA(PHY_P1_CTL+0xB28) |=  (0x180000); 
#endif
#if 0 /* SSC Disable */
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0x938, REGDATA(PHY_P1_CTL+0x938));
	REGDATA(PHY_P1_CTL+0x938) &= ~(0xC000);
	REGDATA(PHY_P1_CTL+0x938) |=  (0x8000);
#endif
	mdelay(10);
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC00, REGDATA(PHY_P1_CTL+0xC00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0x9E0, REGDATA(PHY_P1_CTL+0x9E0));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0x9E4, REGDATA(PHY_P1_CTL+0x9E4));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0x97C, REGDATA(PHY_P1_CTL+0x97C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xB04, REGDATA(PHY_P1_CTL+0xB04));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xB00, REGDATA(PHY_P1_CTL+0xB00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC3C, REGDATA(PHY_P1_CTL+0xC3C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC48, REGDATA(PHY_P1_CTL+0xC48));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC0C, REGDATA(PHY_P1_CTL+0xC0C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC08, REGDATA(PHY_P1_CTL+0xC08));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC10, REGDATA(PHY_P1_CTL+0xC10));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xC14, REGDATA(PHY_P1_CTL+0xC14));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0x938, REGDATA(PHY_P1_CTL+0x938));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P1_CTL+0xB28, REGDATA(PHY_P1_CTL+0xB28));
#endif
#if defined (CONFIG_PCIE_PORT2)
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC00, REGDATA(PHY_P2_CTL+0xC00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0x9E0, REGDATA(PHY_P2_CTL+0x9E0));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0x9E4, REGDATA(PHY_P2_CTL+0x9E4));
	REGDATA(PHY_P2_CTL+0xC00) &= ~(0x33000); 
	REGDATA(PHY_P2_CTL+0xC00) |=   0x22000;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xB04, REGDATA(PHY_P2_CTL+0xB04));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xB00, REGDATA(PHY_P2_CTL+0xB00));
	REGDATA(PHY_P2_CTL+0xB04) &= ~(0xe0000000); 
	REGDATA(PHY_P2_CTL+0xB04) |=   0x40000000;
	REGDATA(PHY_P2_CTL+0xB00) &= ~(0xe); 
	REGDATA(PHY_P2_CTL+0xB00) |=   0x4;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC3C, REGDATA(PHY_P2_CTL+0xC3C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC48, REGDATA(PHY_P2_CTL+0xC48));
	REGDATA(PHY_P2_CTL+0xC3C) &= ~(0xffff0000); 
	REGDATA(PHY_P2_CTL+0xC3C) |=   0x3c0000;
	REGDATA(PHY_P2_CTL+0xC48) &= ~(0xffff); 
	REGDATA(PHY_P2_CTL+0xC48) |=   0x36;
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC0C, REGDATA(PHY_P2_CTL+0xC0C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC08, REGDATA(PHY_P2_CTL+0xC08));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC10, REGDATA(PHY_P2_CTL+0xC10));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC14, REGDATA(PHY_P2_CTL+0xC14));
	REGDATA(PHY_P2_CTL+0xC0C) &= ~(0x30000000); 
	REGDATA(PHY_P2_CTL+0xC0C) |=   0x10000000;
	REGDATA(PHY_P2_CTL+0xC08) &= ~(0x3800c0);
	REGDATA(PHY_P2_CTL+0xC08) |=   0xc0;
	REGDATA(PHY_P2_CTL+0xC10) &= ~(0xf0000); 
	REGDATA(PHY_P2_CTL+0xC10) |=   0x20000;
	REGDATA(PHY_P2_CTL+0xC0C) &= ~(0xf000); 
	REGDATA(PHY_P2_CTL+0xC0C) |=   0x1000;
	REGDATA(PHY_P2_CTL+0xC14) &= ~(0xf0000); 
	REGDATA(PHY_P2_CTL+0xC14) |=   0xa0000;
#if 0 /* RG_PCIE_SIGDET_VTH */
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xB28, REGDATA(PHY_P2_CTL+0xB28));
	REGDATA(PHY_P2_CTL+0xB28) &= ~(0x180000); 
	REGDATA(PHY_P2_CTL+0xB28) |=  (0x180000); 
#endif
#if 0 /* SSC Disable */
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0x938, REGDATA(PHY_P2_CTL+0x938));
	REGDATA(PHY_P2_CTL+0x938) &= ~(0xC000);
	REGDATA(PHY_P2_CTL+0x938) |=  (0x8000);
#endif
	mdelay(10);
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC00, REGDATA(PHY_P2_CTL+0xC00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0x9E0, REGDATA(PHY_P2_CTL+0x9E0));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0x9E4, REGDATA(PHY_P2_CTL+0x9E4));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0x97C, REGDATA(PHY_P2_CTL+0x97C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xB04, REGDATA(PHY_P2_CTL+0xB04));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xB00, REGDATA(PHY_P2_CTL+0xB00));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC3C, REGDATA(PHY_P2_CTL+0xC3C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC48, REGDATA(PHY_P2_CTL+0xC48));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC0C, REGDATA(PHY_P2_CTL+0xC0C));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC08, REGDATA(PHY_P2_CTL+0xC08));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC10, REGDATA(PHY_P2_CTL+0xC10));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xC14, REGDATA(PHY_P2_CTL+0xC14));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0x938, REGDATA(PHY_P2_CTL+0x938));
	PCIE_PRINT(PCIE_PHY_MSG, "%s: PCIe PHY (%x) =%x\n", __func__, PHY_P2_CTL+0xB28, REGDATA(PHY_P2_CTL+0xB28));
#endif
}

void __init mtk_pcie_preinit(void)
{
	unsigned long val = 0;
	int retry;

	PCIE_PRINT(DRV_INFO_MSG, "########## PCIe Host Initialization ##########\n");
	PCIE_PRINT(CR_CEHCK_MSG, "%s:  (%x) =%x\n", __func__, 0xF0209000, REGDATA(0xF0209000));
	PCIE_PRINT(CR_CEHCK_MSG, "%s:  (%x) =%x\n", __func__, 0xF020922C, REGDATA(0xF020922C));
	PCIE_PRINT(CR_CEHCK_MSG, "%s:  (%x) =%x\n", __func__, 0xF0209220, REGDATA(0xF0209220));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: System CTRL (%x) =%x\n", __func__, RSTCTL, REGDATA(RSTCTL));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: System CLKCFG0 (%x) =%x\n", __func__, CLKCFG0, REGDATA(CLKCFG0));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: System CLKCFG3 (%x) =%x\n", __func__, CLKCFG1, REGDATA(CLKCFG1));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: System SYSCFG1 (%x) =%x\n", __func__, SYSCFG1, REGDATA(SYSCFG1));
	pcibios_min_io = 0;
	pcibios_min_mem = 0;	
	
	/* TODO: switch GPIO sharing pin */
	gpio_share_switch();

	/* PCIe High-Performance Mode */
#if defined (CONFIG_PCIE_PORT0)
	val |= CSR_TRGT_HI_PERF_EN;
	if (REGDATA(SWID) == 0x101)
		REGDATA(CLKCFG0) &= ~(CSR_TRGT_HI_PERF_EN);
	else
		REGDATA(CLKCFG0) |= CSR_TRGT_HI_PERF_EN;
	PCIE_PRINT(CR_CEHCK_MSG, "%s: System CLKCFG0 (%x) =%x\n", __func__, CLKCFG0, REGDATA(CLKCFG0));
#endif
#if defined (CONFIG_PCIE_PORT1) || defined (CONFIG_PCIE_PORT2)
	val = 0;
#if defined (CONFIG_PCIE_PORT1)
	val |= CSR_TRGT_HI_PERF_EN1;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= CSR_TRGT_HI_PERF_EN2;
#endif
	if (REGDATA(SWID) == 0x101)
		REGDATA(SYSCFG1) &= ~val;
	else
		REGDATA(SYSCFG1) |= val;
	PCIE_PRINT(CR_CEHCK_MSG, "%s: System SYSCFG1 (%x) =%x\n", __func__, SYSCFG1, REGDATA(SYSCFG1));
#endif

#if defined (CONFIG_PCIE_PORT2)
	/* USB-PCIe co-phy switch */
	REGDATA(SYSCFG1) &= ~(0x300000);
	PCIE_PRINT(CR_CEHCK_MSG, "%s: PCIe/USB3 combo PHY mode (%x) =%x\n", __func__, SYSCFG1, REGDATA(SYSCFG1));
#endif

#if 0 /* just only for FPGA: switching sclk (or not) must be confirm by designer */
	/* configure CLK */
	REGDATA(CLKCFG0) &= ~(0x7<<8);	// set 3 Ports to internal 125MHZ
#if defined (CONFIG_PCIE_PORT0)
	val = 0x1<<8;	//port0 link PCIe PHY
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= 0x1<<9;	//port1 link PCIe PHY
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= 0x1<<10;		//port2 link PCIe PHY
#endif
	//REGDATA(CLKCFG0) |= val;
	mdelay(10);
	PCIE_PRINT("%s: System CLKCFG0 (%x) =%x\n", __func__, CLKCFG0, REGDATA(CLKCFG0));
#endif

	/* PCIe RC Reset */
	val = 0;
#if defined (CONFIG_PCIE_PORT0)
	val |= RSTCTL_PCIE0_RST;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= RSTCTL_PCIE1_RST;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= RSTCTL_PCIE2_RST;
#endif
	/* assert PCIe RC */
	REGDATA(RSTCTL) |= val;
	mdelay(10);
	//PCIE_PRINT("%s: System CTRL (%x) =%x\n", __func__, RSTCTL, REGDATA(RSTCTL));
	/* de-assert PCIe RC */
	REGDATA(RSTCTL) &= ~val;
	mdelay(10);
	PCIE_PRINT(CR_CEHCK_MSG, "%s: System CTRL (%x) =%x\n", __func__, RSTCTL, REGDATA(RSTCTL));

#if ! defined (CONFIG_MT7623_FPGA)
	/* Configure PCIe PHY */
	pcie_phy_config();
#endif
#if 1
#if defined (CONFIG_PCIE_PORT0)
	read_config(0, 0, 0, 0x73c, &val);
	PCIE_PRINT(DRV_INFO_MSG, "PCIe Port0 FC credit = %x", (unsigned int)val);
	val &= ~(0x9fff)<<16;
	val |= 0x806c<<16;
	write_config(0, 0, 0, 0x73c, val);
	read_config(0, 0, 0, 0x73c, &val);
	PCIE_PRINT(DRV_INFO_MSG, " ---> %x\n", (unsigned int)val);
#endif
#if defined (CONFIG_PCIE_PORT1)
	read_config(0, 1, 0, 0x73c, &val);
	PCIE_PRINT(DRV_INFO_MSG, "PCIe Port1 FC credit = %x", (unsigned int)val);
	val &= ~(0x9fff)<<16;
	val |= 0x806c<<16;
	write_config(0, 1, 0, 0x73c, val);
	read_config(0, 1, 0, 0x73c, &val);
	PCIE_PRINT(DRV_INFO_MSG, " ---> %x\n", (unsigned int)val);
#endif
#if defined (CONFIG_PCIE_PORT2)
	read_config(0, 2, 0, 0x73c, &val);
	PCIE_PRINT(DRV_INFO_MSG, "PCIe Port2 FC credit = %x", (unsigned int)val);
	val &= ~(0x9fff)<<16;
	val |= 0x806c<<16;
	write_config(0, 2, 0, 0x73c, val);
	read_config(0, 2, 0, 0x73c, &val);
	PCIE_PRINT(DRV_INFO_MSG, " ---> %x\n", (unsigned int)val);
#endif
#endif

	/* PCIe EP reset */
	val = 0;
#if defined (CONFIG_PCIE_PORT0)
	val |= PCIRST0(0x1);
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= PCIRST1(0x1);
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= PCIRST2(0x1);
#endif
	/* assert PCIe EP & enable Hi-Perf mode */
	REGDATA(PCICFG) |= val;
	mdelay(10);
	//PCIE_PRINT("%s: PERST_N (%x) =%x\n", __func__, PCICFG, REGDATA(PCICFG));
	/* de-assert PCIe EP */
	REGDATA(PCICFG) &= ~(val);
	mdelay(10);
	PCIE_PRINT(CR_CEHCK_MSG, "%s: PERST_N (%x) =%x\n", __func__, PCICFG, REGDATA(PCICFG));

	mdelay(500);	/* at least 100ms delay because PCIe v2.0 need more time from Gen1 to Gen2 */

	PCIE_PRINT(DRV_INFO_MSG, "########## PCIe Link Status ##########\n");
	PCIE_PRINT(CR_CEHCK_MSG, "%s: Port0 (%x) =%x\n", __func__, PCIE_SISTAT(P0), REGDATA(PCIE_SISTAT(P0)));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: Port1 (%x) =%x\n", __func__, PCIE_SISTAT(P1), REGDATA(PCIE_SISTAT(P1)));
	PCIE_PRINT(CR_CEHCK_MSG, "%s: Port2 (%x) =%x\n", __func__, PCIE_SISTAT(P2), REGDATA(PCIE_SISTAT(P2)));
#if defined (CONFIG_PCIE_PORT0)
	retry = 0;
	do {
		if((REGDATA(PCIE_SISTAT(P0)) & 0x1) == 0)
		{
			PCIE_PRINT(DRV_INFO_MSG, "PCIE0 Link down\n");
			REGDATA(RSTCTL) |= RSTCTL_PCIE0_RST;
			pcie_link_status &= ~(1<<0);
		} else {
			PCIE_PRINT(DRV_INFO_MSG, "PCIE0 Link up\n");
			pcie_link_status |= 1<<0;
		}
	} while (!(pcie_link_status & 0x1) && ++retry < PCIE_LINK_RETRY);
#endif
#if defined (CONFIG_PCIE_PORT1)
	retry = 0;
	do {
		if((REGDATA(PCIE_SISTAT(P1)) & 0x1) == 0)
		{
			PCIE_PRINT(DRV_INFO_MSG, "PCIE1 Link down\n");
			REGDATA(RSTCTL) |= RSTCTL_PCIE1_RST;
			pcie_link_status &= ~(1<<1);
		} else {
			PCIE_PRINT(DRV_INFO_MSG, "PCIE1 Link up\n");
			pcie_link_status |= 1<<1;
		}
	} while (!(pcie_link_status & 0x1<<1) && ++retry < PCIE_LINK_RETRY);
#endif
#if defined (CONFIG_PCIE_PORT2)
	retry = 0;
	do {
		if((REGDATA(PCIE_SISTAT(P2)) & 0x1) == 0)
		{
			PCIE_PRINT(DRV_INFO_MSG, "PCIE2 Link down\n");
			REGDATA(RSTCTL) |= RSTCTL_PCIE2_RST;
			pcie_link_status &= ~(1<<2);
		} else {
			PCIE_PRINT(DRV_INFO_MSG, "PCIE2 Link up\n");
			pcie_link_status |= 1<<2;
		}
	} while (!(pcie_link_status & 0x1<<2) && ++retry < PCIE_LINK_RETRY);
#endif

	PCIE_PRINT(DRV_INFO_MSG,"PCIe Link Status = 0x%x\n", pcie_link_status);
	if (pcie_link_status == 0)
		return;
#if 0
	PCIE_PRINT("##### Configure Device number setting of Virtual PCI-PCI bridge #####\n");
	/*
	   pcie(2/1/0) link status pcie2_num       pcie1_num       pcie0_num
	   3'b000                  x               x               x
	   3'b001                  x               x               0
	   3'b010                  x               0               x
	   3'b011                  x               1               0
	   3'b100                  0               x               x
	   3'b101                  1               x               0
	   3'b110                  1               0               x
	   3'b111                  2               1               0
	 */
	val = REGDATA(PCICFG);
	switch(pcie_link_status) {
	case 2:
		val &= ~0x00ff0000;
		val |= 0x1 << 16;    //port0
		val |= 0x0 << 20;    //port1
		break;
	case 4:
		val &= ~0x0fff0000;
		val |= 0x1 << 16;    //port0
		val |= 0x2 << 20;    //port1
		val |= 0x0 << 24;    //port2
		break;
	case 5:
		val &= ~0x0fff0000;
		val |= 0x0 << 16;    //port0
		val |= 0x2 << 20;    //port1
		val |= 0x1 << 24;    //port2
		break;
	case 6:
		val &= ~0x0fff0000;
		val |= 0x2 << 16;    //port0
		val |= 0x0 << 20;    //port1
		val |= 0x1 << 24;    //port2
		break;
	}
	REGDATA(PCICFG) = val;
	PCIE_PRINT("%s: PCICFG[27:16] (%x) =%x\n", __func__, PCICFG,
	       REGDATA(PCICFG));
	PCIE_PRINT("PCIE Device number setting of Virtual PCI-PCI Setup OK\n");
#endif
	REGDATA(MEMBASE) = MEM_WIN; // 0xffffffff
	REGDATA(IOBASE) = IO_WIN;
	if((pcie_link_status & 0x1) != 0) {
		REGDATA(PCIENA) |= PCIINT0_EN(0x1);
		REGDATA(BAR0SETUP(P0)) = 0x7FFF0001;        //open 7FFF:2G; ENABLE
		REGDATA(IMBASEBAR0(P0)) = MEMORY_BASE;
		REGDATA(PCIE_CLASS(P0)) = 0x06040001;
		PCIE_PRINT(DRV_INFO_MSG, "PCIE0 Setup OK\n");
	}
	if ((pcie_link_status & 0x2) != 0) {
		REGDATA(PCIENA) |= PCIINT1_EN(0x1);
		REGDATA(BAR0SETUP(P1)) = 0x7FFF0001;        //open 7FFF:2G; ENABLE
		REGDATA(IMBASEBAR0(P1)) = MEMORY_BASE;
		REGDATA(PCIE_CLASS(P1)) = 0x06040001;
		PCIE_PRINT(DRV_INFO_MSG, "PCIE1 Setup OK\n");
	}
	if ((pcie_link_status & 0x4) != 0) {
		REGDATA(PCIENA) |= PCIINT2_EN(0x1);
		REGDATA(BAR0SETUP(P2)) = 0x7FFF0001;        //open 7FFF:2G; ENABLE
		REGDATA(IMBASEBAR0(P2)) = MEMORY_BASE;
		REGDATA(PCIE_CLASS(P2)) = 0x06040001;
		PCIE_PRINT(DRV_INFO_MSG, "PCIE2 Setup OK\n");
	}
	val = 0;
	PCIE_PRINT(CR_CEHCK_MSG, "Interrupt enable status: %x\n", REGDATA(PCIENA));
	switch(pcie_link_status) {
	case 7:
		read_config(0, 2, 0, 0x4, &val);
		write_config(0, 2, 0, 0x4, val|0x4);
		read_config(0, 2, 0, 0x70c, &val);
		val &= ~(0xff)<<8;
		val |= 0x50<<8;
		write_config(0, 2, 0, 0x70c, val);
		read_config(0, 2, 0, 0x70c, &val);
		PCIE_PRINT(DRV_INFO_MSG, "Bus0-Slot2 N_FTS = %x\n", (unsigned int)val);
	case 3:
	case 5:
	case 6:
		read_config(0, 1, 0, 0x4, &val);
		write_config(0, 1, 0, 0x4, val|0x4);
		read_config(0, 1, 0, 0x70c, &val);
		val &= ~(0xff)<<8;
		val |= 0x50<<8;
		write_config(0, 1, 0, 0x70c, val);
		read_config(0, 1, 0, 0x70c, &val);
		PCIE_PRINT(DRV_INFO_MSG, "Bus0-Slot1 N_FTS = %x\n", (unsigned int)val);
	default:
		read_config(0, 0, 0, 0x4, &val);
		write_config(0, 0, 0, 0x4, val|0x4); //bus master enable
		read_config(0, 0, 0, 0x70c, &val);
		val &= ~(0xff)<<8;
		val |= 0x50<<8;
		write_config(0, 0, 0, 0x70c, val);
		read_config(0, 0, 0, 0x70c, &val);
		PCIE_PRINT(DRV_INFO_MSG, "Bus0-Slot0 N_FTS = %x\n", (unsigned int)val);
	}
	PCIE_PRINT(DRV_INFO_MSG, "########## PCIe Initialization Done ##########\n");
}

static void mtk_pcie_postinit(void)
{
#if defined (CONFIG_PCIE_PORT0)
	if((pcie_link_status & 0x1) == 0) {
		REGDATA(RSTCTL) &= ~(RSTCTL_PCIE0_RST);
		REGDATA(PHY_RST) &= ~(P0_PHY_RST);
		REGDATA(CLKCFG0) &= ~(1<<8);
	}
#endif
#if defined (CONFIG_PCIE_PORT1)
	if ((pcie_link_status & 0x2) == 0) {
		REGDATA(RSTCTL) &= ~(RSTCTL_PCIE1_RST);
		REGDATA(PHY_RST) &= ~(P1_PHY_RST);
		REGDATA(CLKCFG0) &= ~(1<<9);
	}
#endif
#if defined (CONFIG_PCIE_PORT2)
	if ((pcie_link_status & 0x4) == 0) {
		REGDATA(RSTCTL) &= ~(RSTCTL_PCIE2_RST);
		REGDATA(RSTCTL) |= RSTCTL_UPHY_RST;
		REGDATA(CLKCFG0) &= ~(1<<10);
	}
#endif
}

static struct hw_pci mtk_pci __initdata = {
//	.swizzle		= pci_std_swizzle,
	.map_irq		= mtk_pcie_map_irq,
	.nr_controllers		= 1,
	.setup			= mtk_pcie_setup,
	.scan			= mtk_pcie_scan_bus,
	.postinit		= mtk_pcie_postinit,
	.preinit		= mtk_pcie_preinit,
};

extern void mt7623_ethifsys_init(void);
static int __init mtk_pcie_init(void)
{
	PCIE_PRINT(DRV_INFO_MSG, "########## PCIe Host Contoller  ##########\n");
	mt7623_ethifsys_init();
#if defined (CONFIG_MT7623_FPGA)
	int port = 0;
#if defined (CONFIG_PCIE_PORT0)
	port |= 0x1<<0;
#endif
#if defined (CONFIG_PCIE_PORT1)
	port |= 0x1<<1;
#endif
#if defined (CONFIG_PCIE_PORT2)
	port |= 0x1<<2;
#endif
	PCIE_PRINT(DRV_INFO_MSG, "PCIE PHY initialize %x\n", port);
	switch_address_bytes(I2C_PCIEPHY_ACCESS_BYTE);
	switch_device_address(I2C_PCIEPHY_ADDR);
	pcie_phy_calibration(port, 0);	// 3 Ports
#endif /* CONFIG_MT7623_FPGA */
	pci_common_init(&mtk_pci);
	return 0;
}

#if defined (CONFIG_MT7623_FPGA)
late_initcall(mtk_pcie_init);
#else
subsys_initcall(mtk_pcie_init);
//late_initcall(mtk_pcie_init);
#endif /* CONFIG_MT7623_FPGA */

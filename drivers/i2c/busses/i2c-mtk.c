/*
 *
 * (C) Copyright 2011
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
 *
 * mt8320 I2C Bus Controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/io.h>
#include <asm/system.h>
#include <i2c-mtk.h>
/*#include <mach/i2c.h>*/
#include <linux/i2c.h>

/*#define I2C_PRINT_ERROR_LOG*/
/******************************internal API********************************************************/
static inline void i2c_writew(struct mt_i2c *i2c, u8 offset, u16 value)
{
	writew(value, (void *)((i2c->base) + (offset)));
}

static inline u16 i2c_readw(struct mt_i2c *i2c, u8 offset)
{
	return readw((void const *)((i2c->base) + (offset)));
}


/***********************************i2c debug********************************************************/
/* #define I2C_DEBUG_FS */
#ifdef I2C_DEBUG_FS
#define PORT_COUNT 7
#define MESSAGE_COUNT 16
#define I2C_T_DMA 1
#define I2C_T_TRANSFERFLOW 2
#define I2C_T_SPEED 3
  /*7 ports,16 types of message */
U8 i2c_port[PORT_COUNT][MESSAGE_COUNT];


#ifdef I2C_DRIVER_IN_KERNEL
static ssize_t show_config(struct device *dev, struct device_attribute *attr, char *buff)
{
	S32 i = 0;
	S32 j = 0;
	ssize_t rsl = 0;
	char *buf = buff;
	for (i = 0; i < PORT_COUNT; i++) {
		for (j = 0; j < MESSAGE_COUNT; j++)
			i2c_port[i][j] += '0';
		strncpy(buf, (char *)i2c_port[i], MESSAGE_COUNT);
		buf += MESSAGE_COUNT;
		*buf = '\n';
		buf++;
		for (j = 0; j < MESSAGE_COUNT; j++)
			i2c_port[i][j] -= '0';
	}
	rsl = buf - buff;
	return rsl;
}

static ssize_t set_config(struct device *dev, struct device_attribute *attr, const char *buf,
			  size_t count)
{
	S32 port, type, status;
	int status = -1;
	status = sscanf(buf, "%d %d %d", &port, &type, &status);
	if (status > 0) {
		if (port >= PORT_COUNT || port < 0 || type >= MESSAGE_COUNT || type < 0) {
			/*Invalid param */
			dev_err(dev, "i2c debug system: Parameter overflowed!\n");
		} else {
			if (status != 0)
				i2c_port[port][type] = 1;
			else
				i2c_port[port][type] = 0;

			dev_alert(dev,
				  "port:%d type:%d status:%s\n"
				  "i2c debug system: Parameter accepted!\n",
				  port, type, status ? "on" : "off");
		}
	} else {
		/*parameter invalid */
		dev_err(dev, "i2c debug system: Parameter invalid!\n");
	}
	return count;
}

static DEVICE_ATTR(debug, S_IRUGO | S_IWUSR | S_IWGRP, show_config, set_config);
#endif
#else
#define I2C_INFO(mt_i2c, type, format, arg...)
#endif
/***********************************common API********************************************************/
static void mt_i2c_clock_enable(struct mt_i2c *i2c)
{
	struct mt_i2c_data *pdata = dev_get_platdata(i2c->adap.dev.parent);
#if (!defined(CONFIG_MT_I2C_FPGA_ENABLE))
	if (i2c->dma_en)
		pdata->enable_dma_clk(pdata, true);
	pdata->enable_clk(pdata, true);
#endif
	return;
}

static void mt_i2c_clock_disable(struct mt_i2c *i2c)
{
	struct mt_i2c_data *pdata = dev_get_platdata(i2c->adap.dev.parent);
#if (!defined(CONFIG_MT_I2C_FPGA_ENABLE))
	if (i2c->dma_en)
		pdata->enable_dma_clk(pdata, false);
	pdata->enable_clk(pdata, false);
#endif
	return;
}

static void free_i2c_dma_bufs(struct mt_i2c *i2c)
{
	if (i2c->dma_buf.vaddr)
		dma_free_coherent(NULL, PAGE_SIZE, i2c->dma_buf.vaddr, i2c->dma_buf.paddr);
}

/*Set i2c port speed*/
static S32 i2c_set_speed(struct mt_i2c *i2c)
{
	S32 ret = 0;
	static S32 mode;
	static U32 khz;
	/* U32 base = i2c->base; */
	U16 step_cnt_div = 0;
	U16 sample_cnt_div = 0;
	U32 tmp, sclk = 0, hclk = i2c->clk;
	U16 max_step_cnt_div = 0;
	U32 diff, min_diff = i2c->clk;
	U16 sample_div = MAX_SAMPLE_CNT_DIV;
	U16 step_div = 0;
	/* compare the current speed with the latest mode */
	if ((i2c->speed == i2c->last_speed) && (i2c->mode == i2c->last_mode)) {
		ret = 0;
		goto end;
	}
	mode = i2c->mode;
	khz = i2c->speed;

	max_step_cnt_div = (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
	step_div = max_step_cnt_div;

	if ((mode == FS_MODE && khz > MAX_FS_MODE_SPEED)
	    || (mode == HS_MODE && khz > MAX_HS_MODE_SPEED)) {
		dev_err(i2c->dev, "the speed is too fast for this mode.\n");
		I2C_BUG_ON((mode == FS_MODE && khz > MAX_FS_MODE_SPEED)
			   || (mode == HS_MODE && khz > MAX_HS_MODE_SPEED));
		ret = -EINVAL;
		goto end;
	}
	dev_info(i2c->dev, "first:khz=%d,mode=%d sclk=%d,min_diff=%d,max_step_cnt_div=%d\n",
		khz, mode, sclk, min_diff, max_step_cnt_div);
	/*Find the best combination */
	for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
		for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
			sclk = (hclk >> 1) / (sample_cnt_div * step_cnt_div);
			if (sclk > khz)
				continue;
			diff = khz - sclk;
			if (diff < min_diff) {
				min_diff = diff;
				sample_div = sample_cnt_div;
				step_div = step_cnt_div;
			}
		}
	}
	sample_cnt_div = sample_div;
	step_cnt_div = step_div;
	sclk = hclk / (2 * sample_cnt_div * step_cnt_div);
	dev_info(i2c->dev,
		 "second:sclk=%d khz=%d,i2c->speed=%d hclk=%d sample_cnt_div=%d,step_cnt_div=%d.\n",
		 sclk, khz, i2c->speed, hclk, sample_cnt_div, step_cnt_div);
	if (sclk > khz) {
		dev_err(i2c->dev, "%s mode: unsupported speed (%ldkhz)\n",
			(mode == HS_MODE) ? "HS" : "ST/FT", (long int)khz);
		I2C_BUG_ON(sclk > khz);
		ret = -ENOTSUPP;
		goto end;
	}

	step_cnt_div--;
	sample_cnt_div--;

	/* spin_lock(&i2c->lock); */

	if (mode == HS_MODE) {

		/*Set the hignspeed timing control register */
		tmp = i2c_readw(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp = (0 & 0x7) << 8 | (16 & 0x3f) << 0 | tmp;
		i2c->timing_reg = tmp;

		/*Set the hign speed mode register */
		tmp = i2c_readw(i2c, OFFSET_HS) & ~((0x7 << 12) | (0x7 << 8));
		tmp = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
		/*Enable the hign speed transaction */
		tmp |= 0x0001;
		i2c->high_speed_reg = tmp;
		/* i2c_writew(i2c, OFFSET_HS, tmp); */
	} else {
		/*Set non-highspeed timing */
		tmp = i2c_readw(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0 | tmp;
		i2c->timing_reg = tmp;
		/* i2c_writelw(i2c, OFFSET_TIMING, tmp); */
		/*Disable the high speed transaction */
		/* I2CERR("NOT HS_MODE============================1\n"); */
		tmp = i2c_readw(i2c, OFFSET_HS) & ~(0x0001);
		/* I2CERR("NOT HS_MODE============================2\n"); */
		i2c->high_speed_reg = tmp;
		/* i2c_writelw(i2c, OFFSET_HS, tmp); */
		/* I2CERR("NOT HS_MODE============================3\n"); */
	}
	/* spin_unlock(&i2c->lock); */
end:
	i2c->last_speed = i2c->speed;
	i2c->last_mode = i2c->mode;
	return ret;
}

void _i2c_dump_info(struct mt_i2c *i2c)
{
#ifdef I2C_PRINT_ERROR_LOG
	dev_err(i2c->dev, "I2C structure:\nMode %x\nSt_rs %x\n", i2c->mode, i2c->st_rs);
	dev_err(i2c->dev, "I2C structure:\nDma_en %x\nOp %x\n", i2c->dma_en, i2c->op);
	dev_err(i2c->dev,
		"I2C structure:\nTrans_len %x\nTrans_num %x\nTrans_auxlen %x\n",
		i2c->trans_data.trans_len, i2c->trans_data.trans_num, i2c->trans_data.trans_auxlen);
	dev_err(i2c->dev,
		"I2C structure:\nData_size %x\nIrq_stat %x\nTrans_stop %u\n",
		i2c->trans_data.data_size, i2c->irq_stat, atomic_read(&i2c->trans_stop));
	dev_err(i2c->dev,
		"I2C structure:\nTrans_comp %u\nTrans_error %u\n",
		atomic_read(&i2c->trans_comp), atomic_read(&i2c->trans_err));

	dev_err(i2c->dev, "base address %x\n", i2c->base);
	dev_err(i2c->dev,
		"I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\n",
		(i2c_readw(i2c, OFFSET_SLAVE_ADDR)), (i2c_readw(i2c, OFFSET_INTR_MASK)));
	dev_err(i2c->dev,
		"I2C register:\nINTR_STAT %x\nCONTROL %x\n",
		(i2c_readw(i2c, OFFSET_INTR_STAT)), (i2c_readw(i2c, OFFSET_CONTROL)));
	dev_err(i2c->dev,
		"I2C register:\nTRANSFER_LEN %x\nTRANSAC_LEN %x\n",
		(i2c_readw(i2c, OFFSET_TRANSFER_LEN)), (i2c_readw(i2c, OFFSET_TRANSAC_LEN)));
	dev_err(i2c->dev,
		"I2C register:\nDELAY_LEN %x\nTIMING %x\n",
		(i2c_readw(i2c, OFFSET_DELAY_LEN)), (i2c_readw(i2c, OFFSET_TIMING)));
	dev_err(i2c->dev,
		"I2C register:\nSTART %x\nFIFO_STAT %x\n",
		(i2c_readw(i2c, OFFSET_START)), (i2c_readw(i2c, OFFSET_FIFO_STAT)));
	dev_err(i2c->dev,
		"I2C register:\nIO_CONFIG %x\nHS %x\n",
		(i2c_readw(i2c, OFFSET_IO_CONFIG)), (i2c_readw(i2c, OFFSET_HS)));
	if (i2c->platform_flag & MT_I2C_6595) {
			dev_err(i2c->dev,
				"I2C register:\nDEBUGSTAT %x\nEXT_CONF %x\nOFFSET_TRANSFER_LEN_AUX %x\n",
				(i2c_readw(i2c, OFFSET_DEBUGSTAT)), (i2c_readw(i2c, OFFSET_EXT_CONF)),
				(i2c_readw(i2c, OFFSET_TRANSFER_LEN_AUX)));
	}	else {
		dev_err(i2c->dev,
			"I2C register:\nDEBUGSTAT %x\nEXT_CONF %x\nPATH_DIR %x\n",
			(i2c_readw(i2c, OFFSET_DEBUGSTAT)), (i2c_readw(i2c, OFFSET_EXT_CONF)),
			(i2c_readw(i2c, OFFSET_PATH_DIR)));
	}
#endif
	return;
}

static int _i2c_deal_result(struct mt_i2c *i2c)
{
#ifdef I2C_DRIVER_IN_KERNEL
	long tmo = i2c->adap.timeout;
#else
	long tmo = 1;
#endif
	U16 data_size = 0;
	U8 *ptr = i2c->msg_buf;
	S32 ret = i2c->msg_len;
/*Interrupt mode,wait for interrupt wake up */
	tmo = wait_event_timeout(i2c->wait, atomic_read(&i2c->trans_stop), tmo);

	/*Save status register status to i2c struct */
#ifdef I2C_DRIVER_IN_KERNEL
	if (i2c->irq_stat & I2C_TRANSAC_COMP) {
		atomic_set(&i2c->trans_err, 0);
		atomic_set(&i2c->trans_comp, 1);
	}
	atomic_set(&i2c->trans_err, i2c->irq_stat & (I2C_HS_NACKERR | I2C_ACKERR));
#endif

	/*Check the transfer status */
	if (!(tmo == 0 || atomic_read(&i2c->trans_err))) {
		/*Transfer success ,we need to get data from fifo */
		/*only read mode or write_read mode and */
		/*fifo mode need to get data */
		if ((!i2c->dma_en) && (i2c->op == I2C_MASTER_RD || i2c->op == I2C_MASTER_WRRD)) {
			data_size = (i2c_readw(i2c, OFFSET_FIFO_STAT) >> 4) & 0x000F;
			BUG_ON(data_size > i2c->msg_len);
			/* I2CLOG("data_size=%d\n",data_size); */
			while (data_size--) {
				*ptr = i2c_readw(i2c, OFFSET_DATA_PORT);
				ptr++;
			}
		}
#ifdef I2C_DEBUG_FS
		_i2c_dump_info(i2c);
#endif
	} else {
		/*Timeout or ACKERR */
		if (tmo == 0) {
#ifdef I2C_PRINT_ERROR_LOG
			dev_err(i2c->dev, "id=%d,addr: %x, transfer timeout\n", i2c->id, i2c->addr);
#endif
			ret = -ETIMEDOUT;
		} else {
#ifdef I2C_PRINT_ERROR_LOG
			dev_err(i2c->dev, "id=%d,addr: %x, transfer error\n", i2c->id, i2c->addr);
#endif
			ret = -EREMOTEIO;
		}
		if (i2c->irq_stat & I2C_HS_NACKERR) {
#ifdef I2C_PRINT_ERROR_LOG
			dev_err(i2c->dev, "I2C_HS_NACKERR\n");
#endif
		}
		if (i2c->irq_stat & I2C_ACKERR) {
#ifdef I2C_PRINT_ERROR_LOG
			dev_err(i2c->dev, "I2C_ACKERR\n");
#endif
		}
		if (i2c->filter_msg == FALSE) {	/* TEST */
			_i2c_dump_info(i2c);
		}

		spin_lock(&i2c->lock);
		/*Reset i2c port */
		i2c_writew(i2c, OFFSET_SOFTRESET, 0x0001);
		/*Set slave address */
		i2c_writew(i2c, OFFSET_SLAVE_ADDR, 0x0000);
		/*Clear interrupt status */
		i2c_writew(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
		/*Clear fifo address */
		i2c_writew(i2c, OFFSET_FIFO_ADDR_CLR, 0x0001);
		i2c->last_mode = -1;
		i2c->last_speed = 0;
		spin_unlock(&i2c->lock);
	}
	return ret;
}

static void _i2c_write_reg(struct mt_i2c *i2c)
{
	U8 *ptr = i2c->msg_buf;
	U32 data_size = i2c->trans_data.data_size;
	U32 addr_reg = 0;
	struct mt_i2c_data *pdata = dev_get_platdata(i2c->adap.dev.parent);
	/*We must set wrapper bit before setting other register */
	if (pdata->flags & MT_WRAPPER_BUS)
		i2c_writew(i2c, OFFSET_PATH_DIR, I2C_CONTROL_WRAPPER);

	i2c_writew(i2c, OFFSET_CONTROL, i2c->control_reg);

	/*set start condition */
	if (i2c->speed <= 100)
		i2c_writew(i2c, OFFSET_EXT_CONF, 0x8001);
	else
		i2c_writew(i2c, OFFSET_EXT_CONF, 0x1800);

	/* set timing reg */
	i2c_writew(i2c, OFFSET_TIMING, i2c->timing_reg);
	i2c_writew(i2c, OFFSET_HS, i2c->high_speed_reg);

	if (0 == i2c->delay_len)
		i2c->delay_len = 2;
	if (~i2c->control_reg & I2C_CONTROL_RS) {	/* bit is set to 1, i.e.,use repeated stop */
		i2c_writew(i2c, OFFSET_DELAY_LEN, i2c->delay_len);
	}

	/*Set ioconfig */
	if (i2c->pushpull)
		i2c_writew(i2c, OFFSET_IO_CONFIG, 0x0000);
	else
		i2c_writew(i2c, OFFSET_IO_CONFIG, 0x0003);

	/*Set slave address */
	addr_reg = i2c->read_flag ? ((i2c->addr << 1) | 0x1) : ((i2c->addr << 1) & ~0x1);
	i2c_writew(i2c, OFFSET_SLAVE_ADDR, addr_reg);
	/*Clear interrupt status */
	i2c_writew(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	/*Clear fifo address */
	i2c_writew(i2c, OFFSET_FIFO_ADDR_CLR, 0x0001);
	/*Setup the interrupt mask flag */
	/*Enable interrupt */
	i2c_writew(i2c, OFFSET_INTR_MASK,
		   i2c_readw(i2c,
			     OFFSET_INTR_MASK) | (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	/*Set transfer len */
	if (i2c->platform_flag & MT_I2C_6595) {
		i2c_writew(i2c, OFFSET_TRANSFER_LEN, i2c->trans_data.trans_len & 0xFF);
		i2c_writew(i2c, OFFSET_TRANSFER_LEN_AUX, i2c->trans_data.trans_auxlen & 0xFF);
	} else {
		i2c_writew(i2c, OFFSET_TRANSFER_LEN,
		((i2c->trans_data.trans_auxlen & 0x1F) << 8) | (i2c->trans_data.trans_len & 0xFF));
	}

	/*Set transaction len */
	i2c_writew(i2c, OFFSET_TRANSAC_LEN, i2c->trans_data.trans_num & 0xFF);

	/*Prepare buffer data to start transfer */
	if (i2c->dma_en) {
		if (I2C_MASTER_RD == i2c->op) {
			writel(0x0000, (void *)(i2c->pdmabase + OFFSET_INT_FLAG));
			writel(0x0001, (void *)(i2c->pdmabase + OFFSET_CON));
			writel((u32) i2c->msg_buf, (void *)(i2c->pdmabase + OFFSET_RX_MEM_ADDR));
			writel(i2c->trans_data.data_size, (void *)(i2c->pdmabase + OFFSET_RX_LEN));
		} else if (I2C_MASTER_WR == i2c->op) {
			writel(0x0000, (void *)(i2c->pdmabase + OFFSET_INT_FLAG));
			writel(0x0000, (void *)(i2c->pdmabase + OFFSET_CON));
			writel((u32) i2c->msg_buf, (void *)(i2c->pdmabase + OFFSET_TX_MEM_ADDR));
			writel(i2c->trans_data.data_size, (void *)(i2c->pdmabase + OFFSET_TX_LEN));
		} else {
			writel(0x0000, (void *)(i2c->pdmabase + OFFSET_INT_FLAG));
			writel(0x0000, (void *)(i2c->pdmabase + OFFSET_CON));
			writel((u32) i2c->msg_buf, (void *)(i2c->pdmabase + OFFSET_TX_MEM_ADDR));
			writel((u32) i2c->msg_buf, (void *)(i2c->pdmabase + OFFSET_RX_MEM_ADDR));
			writel(i2c->trans_data.trans_len, (void *)(i2c->pdmabase + OFFSET_TX_LEN));
			writel(i2c->trans_data.trans_auxlen,
			       (void *)(i2c->pdmabase + OFFSET_RX_LEN));
		}
		/* flash config before sending start */
		mb();
		writel(0x0001, (void *)(i2c->pdmabase + OFFSET_EN));


	} else {
		/*Set fifo mode data */
		if (I2C_MASTER_RD == i2c->op) {
			/*do not need set fifo data */
		} else {	/*both write && write_read mode */
			while (data_size--) {
				i2c_writew(i2c, OFFSET_DATA_PORT, *ptr);
				ptr++;
			}
		}
	}
	/*Set trans_data */
	i2c->trans_data.data_size = data_size;
	if (0x0 == (i2c_readw(i2c, OFFSET_TIMING)))
		i2c_writew(i2c, OFFSET_TIMING, 0x1410);

	/* disable dcm, since default/reset is 1 */
	if (i2c->platform_flag & MT_I2C_6595)
		i2c_writew(i2c, OFFSET_DCM_EN, 0x0);
}

static S32 _i2c_get_transfer_len(struct mt_i2c *i2c)
{
	S32 ret = 0;
	u16 trans_num = 0;
	u16 data_size = 0;
	u16 trans_len = 0;
	u16 trans_auxlen = 0;
	/* I2CFUC(); */
	/*Get Transfer len and transaux len */
	if (FALSE == i2c->dma_en) {	/*non-DMA mode */
		if (I2C_MASTER_WRRD != i2c->op) {
			trans_len = (i2c->msg_len) & 0xFF;
			trans_num = (i2c->msg_len >> 8) & 0xFF;
			if (0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len * trans_num;

			if (!trans_len || !trans_num || trans_len * trans_num > 8) {
				dev_err(i2c->dev,
					" non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n",
					trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_num || trans_len * trans_num > 8);
				ret = -EINVAL;
			}
		} else {
			trans_len = (i2c->msg_len) & 0xFF;
			trans_auxlen = (i2c->msg_len >> 8) & 0xFF;
			trans_num = 2;
			data_size = trans_len;
			if (!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8) {
				dev_err(i2c->dev,
					" WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n",
					trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_auxlen || trans_len > 8
					   || trans_auxlen > 8);
				ret = -EINVAL;
			}
		}
	} else {		/*DMA mode */
		if (I2C_MASTER_WRRD != i2c->op) {
			trans_len = (i2c->msg_len) & 0xFF;
			trans_num = (i2c->msg_len >> 8) & 0xFF;
			if (0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len * trans_num;

			if (!trans_len || !trans_num || trans_len > MAX_DMA_TRANS_SIZE
			    || trans_num > MAX_DMA_TRANS_NUM) {
				dev_err(i2c->dev,
					" DMA non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n",
					trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_num
					   || trans_len > MAX_DMA_TRANS_SIZE
					   || trans_num > MAX_DMA_TRANS_NUM);
				ret = -EINVAL;
			}
		} else {
			trans_len = (i2c->msg_len) & 0xFF;
			trans_auxlen = (i2c->msg_len >> 8) & 0xFF;
			trans_num = 2;
			data_size = trans_len;
			if (!trans_len || !trans_auxlen || trans_len > MAX_DMA_TRANS_SIZE
			    || trans_auxlen > MAX_DMA_TRANS_NUM) {
				dev_err(i2c->dev,
					" DMA WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n",
					trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_auxlen
					   || trans_len > MAX_DMA_TRANS_SIZE
					   || trans_auxlen > MAX_DMA_TRANS_NUM);
				ret = -EINVAL;
			}
		}
	}

	i2c->trans_data.trans_num = trans_num;
	i2c->trans_data.trans_len = trans_len;
	i2c->trans_data.data_size = data_size;
	i2c->trans_data.trans_auxlen = trans_auxlen;

	return ret;
}

static S32 _i2c_transfer_interface(struct mt_i2c *i2c)
{
	S32 return_value = 0;
	S32 ret = 0;
	U8 *ptr = i2c->msg_buf;
	/* I2CFUC(); */

	if (i2c->dma_en) {
		dev_info(i2c->dev, "DMA Transfer mode!\n");
		if (i2c->pdmabase == 0) {
			dev_err(i2c->dev, " I2C%d doesnot support DMA mode!\n", i2c->id);
			I2C_BUG_ON(i2c->pdmabase == NULL);
			ret = -EINVAL;
			goto err;
		}
		if ((U32) ptr > DMA_ADDRESS_HIGH) {
			dev_err(i2c->dev, " DMA mode should use physical buffer address!\n");
			I2C_BUG_ON((U32) ptr > DMA_ADDRESS_HIGH);
			ret = -EINVAL;
			goto err;
		}
	}
#ifdef I2C_DRIVER_IN_KERNEL
	atomic_set(&i2c->trans_stop, 0);
	atomic_set(&i2c->trans_comp, 0);
	atomic_set(&i2c->trans_err, 0);
#endif
	i2c->irq_stat = 0;

	return_value = _i2c_get_transfer_len(i2c);
	if (return_value < 0) {
		dev_err(i2c->dev, "_i2c_get_transfer_len fail,return_value=%d\n", return_value);
		ret = -EINVAL;
		goto err;
	}

	return_value = i2c_set_speed(i2c);
	if (return_value < 0) {
		dev_err(i2c->dev, "i2c_set_speed fail,return_value=%d\n", return_value);
		ret = -EINVAL;
		goto err;
	}
	/*Set Control Register */
	i2c->control_reg = I2C_CONTROL_ACKERR_DET_EN | I2C_CONTROL_CLK_EXT_EN;
	if (i2c->dma_en)
		i2c->control_reg |= I2C_CONTROL_DMA_EN;

	if (I2C_MASTER_WRRD == i2c->op)
		i2c->control_reg |= I2C_CONTROL_DIR_CHANGE;

	if (HS_MODE == i2c->mode
	    || (i2c->trans_data.trans_num > 1 && I2C_TRANS_REPEATED_START == i2c->st_rs)) {
		i2c->control_reg |= I2C_CONTROL_RS;
	}


	spin_lock(&i2c->lock);
	_i2c_write_reg(i2c);

	/*All register must be prepared before setting the start bit [SMP] */
	mb();
#ifdef I2C_DRIVER_IN_KERNEL
	/*This is only for 3D CAMERA */
	if (i2c->i2c_3dcamera_flag) {
		spin_unlock(&i2c->lock);
		if (g_i2c[0] == NULL)
			g_i2c[0] = i2c;
		else
			g_i2c[1] = i2c;

		goto end;
	}
#endif
	/*Start the transfer */
	i2c_writew(i2c, OFFSET_START, 0x0001);
	spin_unlock(&i2c->lock);
	ret = _i2c_deal_result(i2c);
	dev_info(i2c->dev, "After i2c transfer .....\n");
err:
end:
	return ret;
}

/*=========API in kernel=====================================================================*/
static void _i2c_translate_msg(struct mt_i2c *i2c, struct mt_i2c_msg *msg)
{
	struct mt_i2c_data *pdata = dev_get_platdata(i2c->adap.dev.parent);

  /*-------------compatible with 77/75 driver------*/
	if (msg->addr & 0xFF00)
		msg->ext_flag |= msg->addr & 0xFF00;

	dev_info(i2c->dev, "Before i2c transfer .....\n");

	i2c->msg_buf = msg->buf;
	i2c->msg_len = msg->len;
	if (msg->ext_flag & I2C_RS_FLAG)
		i2c->st_rs = I2C_TRANS_REPEATED_START;
	else
		i2c->st_rs = I2C_TRANS_STOP;

	if (msg->ext_flag & I2C_DMA_FLAG)
		i2c->dma_en = TRUE;
	else
		i2c->dma_en = FALSE;

	if (msg->ext_flag & I2C_WR_FLAG)
		i2c->op = I2C_MASTER_WRRD;
	else {
		if (msg->flags & I2C_M_RD)
			i2c->op = I2C_MASTER_RD;
		else
			i2c->op = I2C_MASTER_WR;
	}
	if (i2c->speed == 0)
		i2c->speed = MAX_ST_MODE_SPEED;
	if (i2c->speed <= MAX_ST_MODE_SPEED) {
		i2c->mode = ST_MODE;
	} else {
		if ((pdata->flags & MT_I2C_HS))
			i2c->mode = HS_MODE;
		else
			i2c->mode = FS_MODE;
	}
	if (msg->ext_flag & I2C_A_FILTER_MSG)
		i2c->filter_msg = TRUE;
	else
		i2c->filter_msg = FALSE;
	i2c->delay_len = pdata->delay_len;

	/*Set ioconfig */
	if (msg->ext_flag & I2C_PUSHPULL_FLAG)
		i2c->pushpull = TRUE;
	else
		i2c->pushpull = FALSE;

	if (msg->ext_flag & I2C_3DCAMERA_FLAG)
		i2c->i2c_3dcamera_flag = TRUE;
	else
		i2c->i2c_3dcamera_flag = FALSE;

}

static S32 mt_i2c_start_xfer(struct mt_i2c *i2c, struct mt_i2c_msg *msg)
{
	S32 return_value = 0;
	S32 ret = msg->len;
	/* start=========================Check param valid=====================================// */
	/* get the read/write flag */
	i2c->read_flag = (msg->flags & I2C_M_RD);
	i2c->addr = msg->addr;
	if (i2c->addr == 0) {
		dev_err(i2c->dev, " addr is invalid.\n");
		I2C_BUG_ON(i2c->addr == NULL);
		ret = -EINVAL;
		goto err;
	}

	if (msg->buf == NULL) {
		dev_err(i2c->dev, " data buffer is NULL.\n");
		I2C_BUG_ON(msg->buf == NULL);
		ret = -EINVAL;
		goto err;
	}
	if (g_i2c[0] == i2c || g_i2c[1] == i2c) {
		dev_err(i2c->dev, "mt-i2c%d: Current I2C Adapter is busy.\n", i2c->id);
		ret = -EINVAL;
		goto err;
	}
	/* start=========================translate msg to mt_i2c===============================// */
	_i2c_translate_msg(i2c, msg);
	/*This is only for 3D CAMERA *//*Save address infomation for 3d camera */
#ifdef I2C_DRIVER_IN_KERNEL
	if (i2c->i2c_3dcamera_flag) {
		if (g_msg[0].buf == NULL)
			memcpy((void *)&g_msg[0], msg, sizeof(struct i2c_msg));
		else
			memcpy((void *)&g_msg[1], msg, sizeof(struct i2c_msg));
	}
#endif
	/* end=========================translate msg to mt_i2c===============================// */
	mt_i2c_clock_enable(i2c);
	return_value = _i2c_transfer_interface(i2c);
	if (!(msg->ext_flag & I2C_3DCAMERA_FLAG))
		mt_i2c_clock_disable(i2c);
	if (return_value < 0) {
		ret = -EINVAL;
		goto err;
	}
err:

	return ret;
}

static int mt_i2c_copy_to_dma(struct mt_i2c *i2c, struct mt_i2c_msg *msgs)
{
	if (((msgs->len) >> 8) & 0x00FF)
		memcpy(i2c->dma_buf.vaddr, (msgs->buf),
		       (msgs->len & 0x00FF) * ((msgs->len >> 8) & 0x00FF));
	else
		memcpy(i2c->dma_buf.vaddr, (msgs->buf), (msgs->len & 0x00FF));
	msgs->buf = (u8 *) i2c->dma_buf.paddr;
	return 0;
}

static int mt_i2c_copy_from_dma(struct mt_i2c *i2c, struct mt_i2c_msg *msg, unsigned char *temp_buf)
{
	if (msg->ext_flag & I2C_WR_FLAG)
		memcpy(temp_buf, i2c->dma_buf.vaddr, (msg->len >> 8) & 0xFF);
	else if (msg->flags & I2C_M_RD) {
		if (((msg->len) >> 8) & 0x00FF)
			memcpy(temp_buf, i2c->dma_buf.vaddr,
			       (msg->len & 0xFF) * ((msg->len >> 8) & 0xFF));
		else
			memcpy(temp_buf, i2c->dma_buf.vaddr, (msg->len & 0xFF));
	}
	msg->buf = temp_buf;
	return 0;
}

static bool mt_i2c_should_combine(struct mt_i2c *i2c, struct mt_i2c_msg *msg)
{
	u16 *p_addr = i2c->pdata->need_wrrd;
	struct mt_i2c_msg *next_msg = msg + 1;
	if (p_addr
	    && (next_msg->len & 0xFF) < 32
	    && msg->addr == next_msg->addr && !(msg->flags & I2C_M_RD)
	    && (next_msg->flags & I2C_M_RD) == I2C_M_RD) {
		u16 addr;
		while ((addr = *p_addr++)) {
			if (addr == msg->addr)
				return true;
		}
	}
	return false;
}

static S32 mt_i2c_do_transfer(struct mt_i2c *i2c, struct mt_i2c_msg *msgs, int num)
{
	int ret = 0;
	int left_num = num;
	struct mt_i2c_msg *msg = msgs;

	while (left_num--) {
		u8 *temp_for_dma = 0;
		u8 *temp_combine = 0;
		bool dma_need_copy_back = false;
		bool combined = false;
		bool restore = false;
		struct mt_i2c_msg *next_msg = msg + 1;

		if (left_num > 0 && mt_i2c_should_combine(i2c, msg)) {
			if (((next_msg->len) & 0xFF) >= ((msg->len) & 0xFF)) {
				memcpy(next_msg->buf, msg->buf, msg->len);
			} else {
				restore = true;
				temp_combine = next_msg->buf;
				next_msg->buf = msg->buf;
			}
			next_msg->flags &= ~I2C_M_RD;
			next_msg->ext_flag |= I2C_WR_FLAG | I2C_RS_FLAG;
			next_msg->len = (next_msg->len << 8) | msg->len;

			msg = next_msg;
			left_num--;
			combined = true;
		}
		if ((!(msg->ext_flag & I2C_DMA_FLAG))
			&& (((msg->len & 0xFF) > 8)
			|| (((msg->len >> 8) & 0xFF) > 8)
			|| combined)) {
			dma_need_copy_back = true;
			msg->ext_flag |= I2C_DMA_FLAG;
			temp_for_dma = msg->buf;
			mt_i2c_copy_to_dma(i2c, msg);
		}

		ret = mt_i2c_start_xfer(i2c, msg);
		if (ret < 0)
			return ret == -EINVAL ? ret : -EAGAIN;

		if (dma_need_copy_back) {
			mt_i2c_copy_from_dma(i2c, msg, temp_for_dma);
			msg->ext_flag &= ~I2C_DMA_FLAG;
		}
		if (restore) {
			memcpy(temp_combine, msgs->buf, (msg->len >> 8) & 0xFF);
			msg->buf = temp_combine;
		}

		msg++;
	}
	/*the return value is number of executed messages */
	return num;
}

S32 mt_i2c_transfer(struct i2c_adapter *adap, struct mt_i2c_msg msgs[], S32 num)
{
	S32 ret = 0;
	S32 retry;
	struct mt_i2c *i2c = i2c_get_adapdata(adap);

	for (retry = 0; retry < adap->retries; retry++) {
		ret = mt_i2c_do_transfer(i2c, msgs, num);
		if (ret != -EAGAIN)
			break;
		if (retry < adap->retries - 1)
			udelay(100);
	}
	return (ret != -EAGAIN) ? ret : -EREMOTEIO;
}

#ifdef I2C_DRIVER_IN_KERNEL
static S32 _i2c_deal_result_3dcamera(struct mt_i2c *i2c, struct mt_i2c_msg *msg)
{
	U16 addr = msg->addr;
	U16 read = (msg->flags & I2C_M_RD);
	i2c->msg_buf = msg->buf;
	i2c->msg_len = msg->len;
	i2c->addr = read ? ((addr << 1) | 0x1) : ((addr << 1) & ~0x1);
	return _i2c_deal_result(i2c);
}
#endif
static int mt_i2c_transfer_standard(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	int i = 0, j = 0;
	int rc = 0;
	struct mt_i2c_msg msg_ext[num*2];

	for (i = 0, j = 0; i < num; i++) {
		/*expand the msg to mt_msg */
		int nseq = msgs[i].len / 0xFC;
		const int last_seq = msgs[i].len % 0xFC;
		if (nseq > 0 && nseq <= 252 && (msgs[i].len <= PAGE_SIZE)) {
			if (unlikely(j >= ARRAY_SIZE(msg_ext)))
				goto mt_i2c_transfer_standard_return;

			msg_ext[j].addr = msgs[i].addr;
			msg_ext[j].flags = msgs[i].flags;
			msg_ext[j].ext_flag = 0;

			msg_ext[j].len = ((nseq << 8) | 0xFC);
			msg_ext[j].buf = msgs[i].buf;
			j++;
		} else if (nseq > 252 || (msgs[i].len > PAGE_SIZE)) {
			goto mt_i2c_transfer_standard_return;
		}

		if (last_seq > 0) {
			if (unlikely(j >= ARRAY_SIZE(msg_ext)))
				goto mt_i2c_transfer_standard_return;

			msg_ext[j].addr = msgs[i].addr;
			msg_ext[j].flags = msgs[i].flags;
			msg_ext[j].ext_flag = 0;

			msg_ext[j].len = last_seq;
			msg_ext[j].buf = msgs[i].buf + (nseq * 0xFC);
			j++;
		}
	}

	rc = mt_i2c_transfer(adap, msg_ext, j);
	if (likely(rc > 0))
		rc = num;

mt_i2c_transfer_standard_return:
	return rc;
}

/**
 * mt_i2c_master_send - issue a single I2C message in master transmit mode
 * @client: Handle to slave device
 * @buf: Data that will be written to the slave
 * @count: How many bytes to write, must be less than 64k since msg.len is u16
 * @ext_flag: Controller special flags.
 *
 * Returns negative errno, or else the number of bytes written.
 */
int mt_i2c_master_send(const struct i2c_client *client,
	const char *buf, int count, u32 ext_flag)
{
	int ret;
	struct i2c_adapter *adap = client->adapter;
	struct mt_i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = (char *)buf;
	msg.ext_flag = ext_flag;
	ret = mt_i2c_transfer(adap, &msg, 1);

	/*
	 * If everything went ok (i.e. 1 msg transmitted), return #bytes
	 * transmitted, else error code.
	 */
	return (ret == 1) ? count : ret;
}
EXPORT_SYMBOL(mt_i2c_master_send);

/**
 * i2c_master_recv - issue a single I2C message in master receive mode
 * @client: Handle to slave device
 * @buf: Where to store data read from slave
 * @count: How many bytes to read, must be less than 64k since msg.len is u16
 * @ext_flag: Controller special flags
 *
 * Returns negative errno, or else the number of bytes read.
 */
int mt_i2c_master_recv(const struct i2c_client *client,
	char *buf, int count, u32 ext_flag)
{
	struct i2c_adapter *adap = client->adapter;
	struct mt_i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;
	msg.ext_flag = ext_flag;
	ret = mt_i2c_transfer(adap, &msg, 1);

	/*
	 * If everything went ok (i.e. 1 msg received), return #bytes received,
	 * else error code.
	 */
	return (ret == 1) ? count : ret;
}
EXPORT_SYMBOL(mt_i2c_master_recv);



/*
static void mt_i2c_post_isr(mt_i2c *i2c)
{
  if (i2c->irq_stat & I2C_TRANSAC_COMP) {
    atomic_set(&i2c->trans_err, 0);
    atomic_set(&i2c->trans_comp, 1);
  }

  if (i2c->irq_stat & I2C_HS_NACKERR) {
    if (i2c->filter_msg==FALSE)
      I2CERR("I2C_HS_NACKERR\n");
  }

  if (i2c->irq_stat & I2C_ACKERR) {
    if (i2c->filter_msg==FALSE)
      I2CERR("I2C_ACKERR\n");
  }
  atomic_set(&i2c->trans_err, i2c->irq_stat & (I2C_HS_NACKERR | I2C_ACKERR));
}*/

/*interrupt handler function*/
static irqreturn_t mt_i2c_irq(S32 irqno, void *dev_id)
{
	struct mt_i2c *i2c = (struct mt_i2c *)dev_id;
	/*Clear interrupt mask */
	i2c_writew(i2c, OFFSET_INTR_MASK,
		   i2c_readw(i2c,
			     OFFSET_INTR_MASK) & ~(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	/*Save interrupt status */
	i2c->irq_stat = i2c_readw(i2c, OFFSET_INTR_STAT);
	/*Clear interrupt status,write 1 clear */
	i2c_writew(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));

	/*Wake up process */
	atomic_set(&i2c->trans_stop, 1);
	wake_up(&i2c->wait);
	return IRQ_HANDLED;
}

/*This function is only for 3d camera*/

S32 mt_wait4_i2c_complete(void)
{
	struct mt_i2c *i2c0 = g_i2c[0];
	struct mt_i2c *i2c1 = g_i2c[1];
	S32 result0 = 0;
	S32 result1 = 0;
	S32 ret = 0;

	if ((i2c0 == NULL) || (i2c1 == NULL)) {
		/*What's wrong? */
		ret = -EINVAL;
		goto end;
	}

	result0 = _i2c_deal_result_3dcamera(i2c0, &g_msg[0]);
	result1 = _i2c_deal_result_3dcamera(i2c1, &g_msg[1]);

	if (result0 < 0 || result1 < 0)
		ret = -EINVAL;

	if (NULL != i2c0)
		mt_i2c_clock_disable(i2c0);
	if (NULL != i2c1)
		mt_i2c_clock_disable(i2c1);

end:
	g_i2c[0] = NULL;
	g_i2c[1] = NULL;

	g_msg[0].buf = NULL;
	g_msg[1].buf = NULL;

	return ret;
}

static U32 mt_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm mt_i2c_algorithm = {
	.master_xfer = mt_i2c_transfer_standard,
	.smbus_xfer = NULL,
	.functionality = mt_i2c_functionality,
};

static inline void mt_i2c_init_hw(struct mt_i2c *i2c)
{
	struct mt_i2c_data *pdata = dev_get_platdata(i2c->adap.dev.parent);
	/* power on */
	if (i2c->platform_flag & MT_I2C_6595) {
		dev_err(i2c->dev,
					"mt-i2c:mt_6595 pwr ctl.\n");
		pdata->i2c_power_ctl(i2c->id, 1);
	}
	i2c_writew(i2c, OFFSET_SOFTRESET, 0x0001);
	/*if(check chip)*/
	if (i2c->platform_flag & MT_I2C_6595)
		i2c_writew(i2c, OFFSET_DCM_EN, 0x0);

}

static void mt_i2c_free(struct mt_i2c *i2c)
{
	if (!i2c)
		return;
	free_i2c_dma_bufs(i2c);
	free_irq(i2c->irqnr, i2c);
	i2c_del_adapter(&i2c->adap);
	kfree(i2c);
}

static S32 mt_i2c_probe(struct platform_device *pdev)
{
	S32 ret, irq;
	struct mt_i2c *i2c = NULL;
	struct resource *res;
	struct resource *res_ap_dma;
	struct mt_i2c_data *pdata = dev_get_platdata(&pdev->dev);

	/* Request platform_device IO resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	res_ap_dma = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (res == NULL || irq < 0 || res_ap_dma == NULL)
		return -ENODEV;

	/* Request IO memory */
	if (!request_mem_region(res->start, resource_size(res), pdev->name))
		return -ENOMEM;

	i2c = kzalloc(sizeof(struct mt_i2c), GFP_KERNEL);
	if (NULL == (i2c))
		return -ENOMEM;

	/* initialize mt_i2c structure */
	i2c->id = pdev->id;
	i2c->base = res->start;
	/* i2c->base     = 0x11011000; */
	i2c->irqnr = irq;
#if (defined(CONFIG_MT_I2C_FPGA_ENABLE))
	i2c->clk = I2C_CLK_RATE;
#else
	i2c->clk = pdata->get_func_clk(pdata);
	/* FIX me if clock manager add this Macro */
	i2c->pdn = pdata->pdn;
#endif
	i2c->dev = &i2c->adap.dev;

	i2c->adap.dev.parent = &pdev->dev;
	i2c->adap.nr = i2c->id;
	i2c->adap.owner = THIS_MODULE;
	i2c->adap.algo = &mt_i2c_algorithm;
	i2c->adap.algo_data = NULL;
	i2c->adap.timeout = 2 * HZ;	/*2s */
	i2c->adap.retries = 1;	/*DO NOT TRY */
	i2c->pdata = pdata;
	i2c->dma_buf.vaddr =
		dma_alloc_coherent(NULL, PAGE_SIZE,
			  &i2c->dma_buf.paddr, GFP_KERNEL);

	snprintf(i2c->adap.name, sizeof(i2c->adap.name), I2C_DRV_NAME);

	i2c->pdmabase = res_ap_dma->start;
	i2c->platform_flag = pdata->flags;

	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

	ret = request_irq(irq, mt_i2c_irq, IRQF_TRIGGER_LOW, I2C_DRV_NAME, i2c);

	if (ret) {
		dev_err(&pdev->dev, "Can Not request I2C IRQ %d\n", irq);
		goto free;
	}

	mt_i2c_init_hw(i2c);
	i2c_set_adapdata(&i2c->adap, i2c);
	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret) {
		dev_err(&pdev->dev, "failed to add i2c bus to i2c core\n");
		goto free;
	}
	platform_set_drvdata(pdev, i2c);
	i2c->speed = pdata->speed;

#ifdef I2C_DEBUG_FS
	ret = device_create_file(i2c->dev, &dev_attr_debug);
#endif
	return ret;

free:
	mt_i2c_free(i2c);
	return ret;
}


static S32 mt_i2c_remove(struct platform_device *pdev)
{
	struct mt_i2c *i2c = platform_get_drvdata(pdev);
	if (i2c) {
		platform_set_drvdata(pdev, NULL);
		mt_i2c_free(i2c);
	}
	return 0;
}

#ifdef CONFIG_PM
static S32 mt_i2c_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* struct mt_i2c *i2c = platform_get_drvdata(pdev); */
	/* dev_dbg(i2c->dev,"[I2C %d] Suspend!\n", i2c->id); */
	return 0;
}

static S32 mt_i2c_resume(struct platform_device *pdev)
{
	/* struct mt_i2c *i2c = platform_get_drvdata(pdev); */
	/* dev_dbg(i2c->dev,"[I2C %d] Resume!\n", i2c->id); */
	return 0;
}
#else
#define mt_i2c_suspend  NULL
#define mt_i2c_resume NULL
#endif

static struct platform_driver mt_i2c_driver = {
	.probe = mt_i2c_probe,
	.remove = mt_i2c_remove,
	.suspend = mt_i2c_suspend,
	.resume = mt_i2c_resume,
	.driver = {
		   .name = I2C_DRV_NAME,
		   .owner = THIS_MODULE,
		   },
};

static S32 __init mt_i2c_init(void)
{
	return platform_driver_register(&mt_i2c_driver);
}

static void __exit mt_i2c_exit(void)
{
	platform_driver_unregister(&mt_i2c_driver);
}

module_init(mt_i2c_init);
module_exit(mt_i2c_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek I2C Bus Driver");
MODULE_AUTHOR("Infinity Chen <infinity.chen@mediatek.com>");

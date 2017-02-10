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
#include <asm/scatterlist.h>
#include <linux/scatterlist.h>
#include <asm/io.h>
//#include <mach/dma.h>
#include <asm/system.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_gpio.h>
#include <mach/mt_clkmgr.h>
#include <mt_i2c.h>
#include <mach/sync_write.h>
#include "mach/memory.h"
#include "cust_gpio_usage.h"
#include "mt_pcie.h"

#if defined (CONFIG_MT7623_FPGA)
#include <linux/delay.h>
#endif  /* #if defined (CONFIG_MT7623_FPGA) */

//define ONLY_KERNEL
/******************************internal API********************************************************/
inline void i2c_writel(mt_i2c * i2c, U8 offset, U16 value)
{
  //__raw_writew(value, (i2c->base) + (offset));
  mt65xx_reg_sync_writel(value, (i2c->base) + (offset));
}

inline U32 i2c_readl(mt_i2c * i2c, U8 offset)
{
  return __raw_readl((void*)((i2c->base) + (offset)));
}
/***********************************declare  API**************************/
static void mt_i2c_clock_enable(mt_i2c *i2c);
static void mt_i2c_clock_disable(mt_i2c *i2c);

/***********************************I2C common Param **************************/
volatile U32 I2C_TIMING_REG_BACKUP[7]={0};
volatile U32 I2C_HIGHSP_REG_BACKUP[7]={0};
/***********************************I2C Param only used in kernel*****************/
/*this field is only for 3d camera*/
#ifdef I2C_DRIVER_IN_KERNEL
static struct i2c_msg g_msg[2];
static mt_i2c  *g_i2c[2];
#define I2C_DRV_NAME        "mt-i2c"
#endif
/***********************************i2c global variable********************************************************/
#if defined (CONFIG_MT7623_FPGA)
#define I2C_BUS_ID      (1)
#define I2C_PHY_ADDR    (0x60)
#define I2C_READ_BLOCK  (16)
#define I2C_WRITE_BLOCK (8)
#define PCIE0_I2C_BUS	(0)
#define PCIE1_I2C_BUS	(1)
#define PCIE2_I2C_BUS	(2)
#define I2C_PCIE_DBG(fmt, arg...)	do { if (i2c_pcie_phy_debug == 1) printk(fmt, ##arg); } while(0)

unsigned long   i2c_address_bytes = 1;
unsigned int    i2c_dev_base_addr = I2C_PHY_ADDR;
//unsigned int    i2c_bus_id = I2C_BUS_ID;
unsigned int    i2c_bus_id;
static int i2c_pcie_phy_debug;
#endif  /* #if defined (CONFIG_MT7623_FPGA) */
/***********************************i2c debug********************************************************/
//#define I2C_DEBUG_FS
#ifdef I2C_DEBUG_FS
  #define PORT_COUNT 7
  #define MESSAGE_COUNT 16
  #define I2C_T_DMA 1
  #define I2C_T_TRANSFERFLOW 2
  #define I2C_T_SPEED 3
  /*7 ports,16 types of message*/
  U8 i2c_port[ PORT_COUNT ][ MESSAGE_COUNT ];
#if 0
  #define I2CINFO( type, format, arg...) do { \
    if ( type < MESSAGE_COUNT && type >= 0 ) { \
      if ( i2c_port[i2c->id][0] != 0 && ( i2c_port[i2c->id][type] != 0 || i2c_port[i2c->id][MESSAGE_COUNT - 1] != 0) ) { \
        I2CLOG( format, ## arg); \
      } \
    } \
  } while (0)
#endif
#define I2CINFO( type, format, arg...) do { \
        I2CLOG( format, ## arg); \
  } while (0)

  #ifdef I2C_DRIVER_IN_KERNEL
    static ssize_t show_config(struct device *dev, struct device_attribute *attr, char *buff)
    {
      S32 i = 0;
      S32 j = 0;
      char *buf = buff;
      for ( i =0; i < PORT_COUNT; i++){
        for ( j=0;j < MESSAGE_COUNT; j++) i2c_port[i][j] += '0';
        strncpy(buf, (char *)i2c_port[i], MESSAGE_COUNT);
        buf += MESSAGE_COUNT;
        *buf = '\n';
        buf++;
        for ( j=0;j < MESSAGE_COUNT; j++) i2c_port[i][j] -= '0';
      }
      return (buf - buff);
    }

    static ssize_t set_config(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
    {
      S32 port,type,status;

      if ( sscanf(buf, "%d %d %d", &port, &type, &status) ) {
        if ( port >= PORT_COUNT || port < 0 || type >= MESSAGE_COUNT || type < 0 ) {
          /*Invalid param*/
          I2CERR("i2c debug system: Parameter overflowed!\n");
        } else {
          if ( status != 0 )
            i2c_port[port][type] = 1;
          else
            i2c_port[port][type] = 0;

          I2CLOG("port:%d type:%d status:%s\ni2c debug system: Parameter accepted!\n", port, type, status?"on":"off");
        }
      } else {
        /*parameter invalid*/
        I2CERR("i2c debug system: Parameter invalid!\n");
      }
      return count;
    }

    static DEVICE_ATTR(debug, S_IRUGO|S_IWUGO, show_config, set_config);
  #endif
#else
  #define I2CINFO(type, format, arg...)
#endif
/***********************************common API********************************************************/
/*Set i2c port speed*/
static S32 i2c_set_speed(mt_i2c *i2c)
{
  S32 ret = 0;
  static S32 mode = 0;
  static U32 khz = 0;
  static U8 last_id = 0;
  //U32 base = i2c->base;
  U16 step_cnt_div = 0;
  U16 sample_cnt_div = 0;
  U32 tmp, sclk, hclk = i2c->clk;
  U16 max_step_cnt_div = 0;
  U32 diff, min_diff = i2c->clk;
  U16 sample_div = MAX_SAMPLE_CNT_DIV;
  U16 step_div = 0;
  //I2CFUC();
  //I2CLOG("i2c_set_speed=================\n");
  //compare the current speed with the latest mode
  if((mode == i2c->mode) && (khz == i2c->speed)){
	  if(i2c->id == last_id){//same controller
		  I2CINFO( I2C_T_SPEED, " same controller still set sclk to %ldkhz\n", i2c->speed);
	  } else {//diff controller
		  I2CINFO( I2C_T_SPEED, " diff controller also set sclk to %ldkhz\n", i2c->speed);
		  i2c->timing_reg = I2C_TIMING_REG_BACKUP[i2c->id] = I2C_TIMING_REG_BACKUP[last_id];
		  i2c->high_speed_reg = I2C_HIGHSP_REG_BACKUP[i2c->id] = I2C_HIGHSP_REG_BACKUP[last_id];
	  }
	  ret = 0;
	  goto end;
  }
  mode=i2c->mode;
  khz = i2c->speed;

  max_step_cnt_div = (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
  step_div = max_step_cnt_div;

  if((mode == FS_MODE && khz > MAX_FS_MODE_SPEED) || (mode == HS_MODE && khz > MAX_HS_MODE_SPEED)){
    I2CERR(" the speed is too fast for this mode.\n");
    I2C_BUG_ON((mode == FS_MODE && khz > MAX_FS_MODE_SPEED) || (mode == HS_MODE && khz > MAX_HS_MODE_SPEED));
    ret = -EINVAL_I2C;
    goto end;
  }
//  I2CERR("first:khz=%d,mode=%d sclk=%d,min_diff=%d,max_step_cnt_div=%d\n",khz,mode,sclk,min_diff,max_step_cnt_div);
  /*Find the best combination*/
  for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
      for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
        sclk = (hclk >> 1) / (sample_cnt_div * step_cnt_div);
        if (sclk > khz)
          continue;
        diff = khz - sclk;
        if (diff < min_diff) {
          min_diff = diff;
          sample_div = sample_cnt_div;
          step_div   = step_cnt_div;
        }
      }
    }
    sample_cnt_div = sample_div;
    step_cnt_div   = step_div;
  sclk = hclk / (2 * sample_cnt_div * step_cnt_div);
  //I2CERR("second:sclk=%d khz=%d,i2c->speed=%d hclk=%d sample_cnt_div=%d,step_cnt_div=%d.\n",sclk,khz,i2c->speed,hclk,sample_cnt_div,step_cnt_div);
  if (sclk > khz) {
    I2CERR("%s mode: unsupported speed (%ldkhz)\n",(mode == HS_MODE) ? "HS" : "ST/FT", (long int)khz);
    I2CLOG("i2c->clk=%d,sclk=%d khz=%d,i2c->speed=%d hclk=%d sample_cnt_div=%d,step_cnt_div=%d.\n",i2c->clk,sclk,khz,i2c->speed,hclk,sample_cnt_div,step_cnt_div);
    I2C_BUG_ON(sclk > khz);
    ret = -ENOTSUPP_I2C;
    goto end;
  }

  step_cnt_div--;
  sample_cnt_div--;

  //spin_lock(&i2c->lock);

  if (mode == HS_MODE) {

    /*Set the hignspeed timing control register*/
    tmp = i2c_readl(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
    tmp = (0 & 0x7) << 8 | (16 & 0x3f) << 0 | tmp;
    i2c->timing_reg=tmp;
    //i2c_writel(i2c, OFFSET_TIMING, tmp);
    I2C_TIMING_REG_BACKUP[i2c->id]=tmp;

    /*Set the hign speed mode register*/
    tmp = i2c_readl(i2c, OFFSET_HS) & ~((0x7 << 12) | (0x7 << 8));
    tmp = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
    /*Enable the hign speed transaction*/
    tmp |= 0x0001;
    i2c->high_speed_reg=tmp;
    I2C_HIGHSP_REG_BACKUP[i2c->id]=tmp;
    //i2c_writel(i2c, OFFSET_HS, tmp);
  }
  else {
    /*Set non-highspeed timing*/
    tmp  = i2c_readl(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
    tmp  = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0 | tmp;
    i2c->timing_reg=tmp;
    I2C_TIMING_REG_BACKUP[i2c->id]=tmp;
    //i2c_writel(i2c, OFFSET_TIMING, tmp);
    /*Disable the high speed transaction*/
    //I2CERR("NOT HS_MODE============================1\n");
    tmp = i2c_readl(i2c, OFFSET_HS) & ~(0x0001);
    //I2CERR("NOT HS_MODE============================2\n");
    i2c->high_speed_reg=tmp;
    I2C_HIGHSP_REG_BACKUP[i2c->id]=tmp;
    //i2c_writel(i2c, OFFSET_HS, tmp);
    //I2CERR("NOT HS_MODE============================3\n");
  }
  //spin_unlock(&i2c->lock);
  I2CINFO( I2C_T_SPEED, " set sclk to %ldkhz(orig:%ldkhz), sample=%d,step=%d\n", sclk, khz, sample_cnt_div, step_cnt_div);
end:
  last_id = i2c->id;
  return ret;
}

void _i2c_dump_info(mt_i2c *i2c)
{
  //I2CFUC();
  I2CLOG("I2C structure:\n"
    I2CTAG"Clk=%d,Id=%d,Speed mode=%x,St_rs=%x,Dma_en=%x,Op=%x,Poll_en=%x,Irq_stat=%x\n"
    I2CTAG"Trans_len=%x,Trans_num=%x,Trans_auxlen=%x,Data_size=%x,speed=%d\n"
    I2CTAG"Trans_stop=%u,Trans_comp=%u,Trans_error=%u\n",
    i2c->clk,i2c->id,i2c->mode,i2c->st_rs,i2c->dma_en,i2c->op,i2c->poll_en,i2c->irq_stat,\
    i2c->trans_data.trans_len,i2c->trans_data.trans_num,i2c->trans_data.trans_auxlen,i2c->trans_data.data_size,i2c->speed,\
    atomic_read(&i2c->trans_stop),atomic_read(&i2c->trans_comp),atomic_read(&i2c->trans_err));

  I2CLOG("base address 0x%x\n",i2c->base);
  I2CLOG("I2C register:\n"
    I2CTAG"SLAVE_ADDR=%x,INTR_MASK=%x,INTR_STAT=%x,CONTROL=%x,TRANSFER_LEN=%x\n"
    I2CTAG"TRANSAC_LEN=%x,DELAY_LEN=%x,TIMING=%x,START=%x,FIFO_STAT=%x\n"
    I2CTAG"IO_CONFIG=%x,HS=%x,DCM_EN=%x,DEBUGSTAT=%x,EXT_CONF=%x\n",
    (i2c_readl(i2c, OFFSET_SLAVE_ADDR)),
    (i2c_readl(i2c, OFFSET_INTR_MASK)),
    (i2c_readl(i2c, OFFSET_INTR_STAT)),
    (i2c_readl(i2c, OFFSET_CONTROL)),
    (i2c_readl(i2c, OFFSET_TRANSFER_LEN)),
    (i2c_readl(i2c, OFFSET_TRANSAC_LEN)),
    (i2c_readl(i2c, OFFSET_DELAY_LEN)),
    (i2c_readl(i2c, OFFSET_TIMING)),
    (i2c_readl(i2c, OFFSET_START)),
    (i2c_readl(i2c, OFFSET_FIFO_STAT)),
    (i2c_readl(i2c, OFFSET_IO_CONFIG)),
    (i2c_readl(i2c, OFFSET_HS)),
    (i2c_readl(i2c, OFFSET_DCM_EN)),
    (i2c_readl(i2c, OFFSET_DEBUGSTAT)),
    (i2c_readl(i2c, OFFSET_EXT_CONF)));
  /*
  I2CERR("DMA register:\nINT_FLAG %x\nCON %x\nTX_MEM_ADDR %x\nRX_MEM_ADDR %x\nTX_LEN %x\nRX_LEN %x\nINT_EN %x\nEN %x\nINT_BUF_SIZE %x\n",
      (__raw_readl(i2c->pdmabase+OFFSET_INT_FLAG)),
      (__raw_readl(i2c->pdmabase+OFFSET_CON)),
      (__raw_readl(i2c->pdmabase+OFFSET_TX_MEM_ADDR)),
      (__raw_readl(i2c->pdmabase+OFFSET_RX_MEM_ADDR)),
      (__raw_readl(i2c->pdmabase+OFFSET_TX_LEN)),
      (__raw_readl(i2c->pdmabase+OFFSET_RX_LEN)),
      (__raw_readl(i2c->pdmabase+OFFSET_INT_EN)),
      (__raw_readl(i2c->pdmabase+OFFSET_EN)),
      (__raw_readl(i2c->pdmabase+OFFSET_INT_BUF_SIZE)));
	  */
  /*clock*/
  I2CLOG("Clock stat(0x10003018) is 0x%08x, %s\n",__raw_readl((void*)0xF0003018), (__raw_readl((void*)0xF0003018)>>21) & (1 << i2c->id)?"disable":"enable");

  I2CLOG("mt_get_gpio_in I2C0_SDA=%d,I2C0_SCA=%d,I2C1_SDA=%d,I2C1_SCA=%d",\
		  mt_get_gpio_in(GPIO_I2C0_SDA_PIN),mt_get_gpio_in(GPIO_I2C0_SCA_PIN),mt_get_gpio_in(GPIO_I2C1_SDA_PIN),mt_get_gpio_in(GPIO_I2C1_SCA_PIN));
  return;
}
static S32 _i2c_deal_result(mt_i2c *i2c)
{
#ifdef I2C_DRIVER_IN_KERNEL
  long tmo = i2c->adap.timeout;
#else
  long tmo = 1;
#endif
  U16 data_size = 0;
  U8 *ptr = i2c->msg_buf;
  S32 ret = i2c->msg_len;
  long tmo_poll = 0xffff;
  //I2CFUC();
  //addr_reg = i2c->read_flag ? ((i2c->addr << 1) | 0x1) : ((i2c->addr << 1) & ~0x1);

  if(i2c->poll_en)
  { /*master read && poll mode*/
    for (;;)
    { /*check the interrupt status register*/
      i2c->irq_stat = i2c_readl(i2c, OFFSET_INTR_STAT);
      //I2CLOG("irq_stat = 0x%x\n", i2c->irq_stat);
      if(i2c->irq_stat & (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP))
      {
        atomic_set(&i2c->trans_stop, 1);
        spin_lock(&i2c->lock);
        /*Clear interrupt status,write 1 clear*/
        i2c_writel(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
        spin_unlock(&i2c->lock);
        break;
      }
        tmo_poll --;
        if(tmo_poll == 0) {
        tmo = 0;
        break;
      }
    }
  } else { /*Interrupt mode,wait for interrupt wake up*/
    tmo = wait_event_timeout(i2c->wait,atomic_read(&i2c->trans_stop), tmo);
  }

  /*Save status register status to i2c struct*/
  #ifdef I2C_DRIVER_IN_KERNEL
  if (i2c->irq_stat & I2C_TRANSAC_COMP) {
    atomic_set(&i2c->trans_err, 0);
    atomic_set(&i2c->trans_comp, 1);
  }
  atomic_set(&i2c->trans_err, i2c->irq_stat & (I2C_HS_NACKERR | I2C_ACKERR));
  #endif

  /*Check the transfer status*/
  if (!(tmo == 0 || atomic_read(&i2c->trans_err)) )
  {
    /*Transfer success ,we need to get data from fifo*/
    if((!i2c->dma_en) && (i2c->op == I2C_MASTER_RD || i2c->op == I2C_MASTER_WRRD) )
    { /*only read mode or write_read mode and fifo mode need to get data*/
      data_size = (i2c_readl(i2c, OFFSET_FIFO_STAT) >> 4) & 0x000F;
	  BUG_ON(data_size > i2c->msg_len);
    //I2CLOG("data_size=%d\n",data_size);
      while (data_size--)
      {
        *ptr = i2c_readl(i2c, OFFSET_DATA_PORT);
        //I2CLOG("addr %x read byte = 0x%x\n", i2c->addr, *ptr);
      ptr++;
      }
    }
    #ifdef I2C_DEBUG_FS
      _i2c_dump_info(i2c);
    #endif
  }else
  {
    /*Timeout or ACKERR*/
    if ( tmo == 0 ){
      I2CERR("id=%d,addr: %x, transfer timeout\n",i2c->id, i2c->addr);
      ret = -ETIMEDOUT_I2C;
    } else
    {
      I2CERR("id=%d,addr: %x, transfer error\n",i2c->id,i2c->addr);
      ret = -EREMOTEIO_I2C;
    }
    if (i2c->irq_stat & I2C_HS_NACKERR)
      I2CERR("I2C_HS_NACKERR\n");
    if (i2c->irq_stat & I2C_ACKERR)
      I2CERR("I2C_ACKERR\n");
    if (i2c->filter_msg==FALSE) //TEST
    {
      _i2c_dump_info(i2c);
   }

    spin_lock(&i2c->lock);
    /*Reset i2c port*/
    i2c_writel(i2c, OFFSET_SOFTRESET, 0x0001);
    /*Set slave address*/
    i2c_writel( i2c, OFFSET_SLAVE_ADDR, 0x0000 );
    /*Clear interrupt status*/
    i2c_writel(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP));
    /*Clear fifo address*/
    i2c_writel(i2c, OFFSET_FIFO_ADDR_CLR, 0x0001);

    spin_unlock(&i2c->lock);
  }
  return ret;
}


static void _i2c_write_reg(mt_i2c *i2c)
{
  U8 *ptr = i2c->msg_buf;
  U32 data_size=i2c->trans_data.data_size;
  U32 addr_reg=0;
  //I2CFUC();

  i2c_writel(i2c, OFFSET_CONTROL, i2c->control_reg);

  /*set start condition */
  if(i2c->speed <= 100){
    i2c_writel(i2c,OFFSET_EXT_CONF, 0x8001);
  } else {
	i2c_writel(i2c,OFFSET_EXT_CONF, 0x1800);
  }
  //set timing reg
  i2c_writel(i2c, OFFSET_TIMING, i2c->timing_reg);
  i2c_writel(i2c, OFFSET_HS, i2c->high_speed_reg);

  if(0 == i2c->delay_len)
    i2c->delay_len = 2;
  if(~i2c->control_reg & I2C_CONTROL_RS){  // bit is set to 1, i.e.,use repeated stop
    i2c_writel(i2c, OFFSET_DELAY_LEN, i2c->delay_len);
  }

  /*Set ioconfig*/
  if (i2c->pushpull) {
      i2c_writel(i2c, OFFSET_IO_CONFIG, 0x0000);
  } else {
      i2c_writel(i2c, OFFSET_IO_CONFIG, 0x0003);
  }

  /*Set slave address*/

  addr_reg = i2c->read_flag ? ((i2c->addr << 1) | 0x1) : ((i2c->addr << 1) & ~0x1);
  i2c_writel(i2c, OFFSET_SLAVE_ADDR, addr_reg);
  /*Clear interrupt status*/
  i2c_writel(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
  /*Clear fifo address*/
  i2c_writel(i2c, OFFSET_FIFO_ADDR_CLR, 0x0001);
  /*Setup the interrupt mask flag*/
  if(i2c->poll_en)
    i2c_writel(i2c, OFFSET_INTR_MASK, i2c_readl(i2c, OFFSET_INTR_MASK) & ~(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP)); /*Disable interrupt*/
  else
    i2c_writel(i2c, OFFSET_INTR_MASK, i2c_readl(i2c, OFFSET_INTR_MASK) | (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP)); /*Enable interrupt*/
  /*Set transfer len */
  i2c_writel(i2c, OFFSET_TRANSFER_LEN, ((i2c->trans_data.trans_auxlen & 0x1F) << 8) | (i2c->trans_data.trans_len & 0xFF));
  /*Set transaction len*/
  i2c_writel(i2c, OFFSET_TRANSAC_LEN, i2c->trans_data.trans_num & 0xFF);

  /*Prepare buffer data to start transfer*/

  if(i2c->dma_en)
  {
    if (I2C_MASTER_RD == i2c->op) {
      mt65xx_reg_sync_writel(0x0000, i2c->pdmabase + OFFSET_INT_FLAG);
      mt65xx_reg_sync_writel(0x0001, i2c->pdmabase + OFFSET_CON);
      mt65xx_reg_sync_writel((U32)i2c->msg_buf, i2c->pdmabase + OFFSET_RX_MEM_ADDR);
      mt65xx_reg_sync_writel(i2c->trans_data.data_size, i2c->pdmabase + OFFSET_RX_LEN);
    } else if (I2C_MASTER_WR == i2c->op) {
      mt65xx_reg_sync_writel(0x0000, i2c->pdmabase + OFFSET_INT_FLAG);
      mt65xx_reg_sync_writel(0x0000, i2c->pdmabase + OFFSET_CON);
      mt65xx_reg_sync_writel((U32)i2c->msg_buf, i2c->pdmabase + OFFSET_TX_MEM_ADDR);
      mt65xx_reg_sync_writel(i2c->trans_data.data_size, i2c->pdmabase + OFFSET_TX_LEN);
    } else {
      mt65xx_reg_sync_writel(0x0000, i2c->pdmabase + OFFSET_INT_FLAG);
      mt65xx_reg_sync_writel(0x0000, i2c->pdmabase + OFFSET_CON);
      mt65xx_reg_sync_writel((U32)i2c->msg_buf, i2c->pdmabase + OFFSET_TX_MEM_ADDR);
      mt65xx_reg_sync_writel((U32)i2c->msg_buf, i2c->pdmabase + OFFSET_RX_MEM_ADDR);
      mt65xx_reg_sync_writel(i2c->trans_data.trans_len, i2c->pdmabase + OFFSET_TX_LEN);
      mt65xx_reg_sync_writel(i2c->trans_data.trans_auxlen, i2c->pdmabase + OFFSET_RX_LEN);
    }
    I2C_MB();
    mt65xx_reg_sync_writel(0x0001, i2c->pdmabase + OFFSET_EN);

    I2CINFO( I2C_T_DMA, "addr %.2x dma %.2X byte\n", i2c->addr, i2c->trans_data.data_size);
    I2CINFO( I2C_T_DMA, "DMA Register:INT_FLAG:0x%x,CON:0x%x,TX_MEM_ADDR:0x%x, \
                 RX_MEM_ADDR:0x%x,TX_LEN:0x%x,RX_LEN:0x%x,EN:0x%x\n",\
                  readl(i2c->pdmabase + OFFSET_INT_FLAG),\
                  readl(i2c->pdmabase + OFFSET_CON),\
                  readl(i2c->pdmabase + OFFSET_TX_MEM_ADDR),\
                  readl(i2c->pdmabase + OFFSET_RX_MEM_ADDR),\
                  readl(i2c->pdmabase + OFFSET_TX_LEN),\
                  readl(i2c->pdmabase + OFFSET_RX_LEN),\
                  readl(i2c->pdmabase + OFFSET_EN));

  }
  else
  {
    /*Set fifo mode data*/
    if (I2C_MASTER_RD == i2c->op)
    {
      /*do not need set fifo data*/
    }else
    { /*both write && write_read mode*/
      while (data_size--)
      {
        i2c_writel(i2c, OFFSET_DATA_PORT, *ptr);
        //dev_info(i2c->dev, "addr %.2x write byte = 0x%.2X\n", addr, *ptr);
        ptr++;
      }
    }
  }
  /*Set trans_data*/
  i2c->trans_data.data_size = data_size;

	i2c_writel(i2c,OFFSET_DCM_EN, 0x0);//disable dcm, since default/reset is 1
}
static S32 _i2c_get_transfer_len(mt_i2c *i2c)
{
  S32 ret = I2C_OK;
  u16 trans_num = 0;
  u16 data_size = 0;
  u16 trans_len = 0;
  u16 trans_auxlen = 0;
  //I2CFUC();
  /*Get Transfer len and transaux len*/
  if(FALSE == i2c->dma_en)
  { /*non-DMA mode*/
    if(I2C_MASTER_WRRD != i2c->op)
    {
      trans_len = (i2c->msg_len) & 0xFF;
      trans_num = (i2c->msg_len >> 8) & 0xFF;
      if(0 == trans_num)
        trans_num = 1;
      trans_auxlen = 0;
      data_size = trans_len*trans_num;

      if(!trans_len || !trans_num || trans_len*trans_num > 8)
      {
        I2CERR(" non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
        I2C_BUG_ON(!trans_len || !trans_num || trans_len*trans_num > 8);
        ret = -EINVAL_I2C;
      }
    } else
    {
      trans_len = (i2c->msg_len) & 0xFF;
      trans_auxlen = (i2c->msg_len >> 8) & 0xFF;
      trans_num = 2;
      data_size = trans_len;
      if(!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8)
      {
        I2CERR(" WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
        I2C_BUG_ON(!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8);
        ret = -EINVAL_I2C;
      }
    }
  }
  else
  { /*DMA mode*/
    if(I2C_MASTER_WRRD != i2c->op)
    {
      trans_len = (i2c->msg_len) & 0xFF;
      trans_num = (i2c->msg_len >> 8) & 0xFF;
      if(0 == trans_num)
        trans_num = 1;
      trans_auxlen = 0;
      data_size = trans_len*trans_num;

      if(!trans_len || !trans_num || trans_len > 255 || trans_num > 255)
      {
        I2CERR(" DMA non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
        I2C_BUG_ON(!trans_len || !trans_num || trans_len > 255 || trans_num > 255);
        ret = -EINVAL_I2C;
      }
      I2CINFO(I2C_T_DMA, "DMA non-WRRD mode!trans_len=%x, tans_num=%x, trans_auxlen=%x\n",trans_len, trans_num, trans_auxlen);
    } else
    {
      trans_len = (i2c->msg_len) & 0xFF;
      trans_auxlen = (i2c->msg_len >> 8) & 0xFF;
      trans_num = 2;
      data_size = trans_len;
      if(!trans_len || !trans_auxlen || trans_len > 255 || trans_auxlen > 31)
      {
        I2CERR(" DMA WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
        I2C_BUG_ON(!trans_len || !trans_auxlen || trans_len > 255 || trans_auxlen > 31);
        ret = -EINVAL_I2C;
      }
      I2CINFO(I2C_T_DMA, "DMA WRRD mode!trans_len=%x, tans_num=%x, trans_auxlen=%x\n",trans_len, trans_num, trans_auxlen);
    }
  }

  i2c->trans_data.trans_num = trans_num;
  i2c->trans_data.trans_len = trans_len;
  i2c->trans_data.data_size = data_size;
  i2c->trans_data.trans_auxlen = trans_auxlen;

  return ret;
}
static S32 _i2c_transfer_interface(mt_i2c *i2c)
{
  S32 return_value=0;
  S32 ret=0;
  U8 *ptr = i2c->msg_buf;
  //I2CFUC();

  if(i2c->dma_en)
  {
    I2CINFO( I2C_T_DMA, "DMA Transfer mode!\n");
    if (i2c->pdmabase == 0) {
      I2CERR(" I2C%d doesnot support DMA mode!\n",i2c->id);
      I2C_BUG_ON(i2c->pdmabase == NULL);
      ret = -EINVAL_I2C;
      goto err;
    }
    if((U32)ptr > DMA_ADDRESS_HIGH){
      I2CERR(" DMA mode should use physical buffer address!\n");
      I2C_BUG_ON((U32)ptr > DMA_ADDRESS_HIGH);
      ret = -EINVAL_I2C;
      goto err;
    }
  }
#ifdef I2C_DRIVER_IN_KERNEL
  atomic_set(&i2c->trans_stop, 0);
  atomic_set(&i2c->trans_comp, 0);
  atomic_set(&i2c->trans_err, 0);
#endif
  i2c->irq_stat = 0;

  return_value=_i2c_get_transfer_len(i2c);
  if ( return_value < 0 ){
    I2CERR("_i2c_get_transfer_len fail,return_value=%d\n",return_value);
    ret =-EINVAL_I2C;
    goto err;
  }
  //get clock
  #ifdef CONFIG_MT_I2C_FPGA_ENABLE
    i2c->clk  = I2C_CLK_RATE;
  #else
    i2c->clk  = mt_get_bus_freq()/16;
  #endif

  return_value=i2c_set_speed(i2c);
  if ( return_value < 0 ){
    I2CERR("i2c_set_speed fail,return_value=%d\n",return_value);
    ret =-EINVAL_I2C;
    goto err;
  }
  /*Set Control Register*/
#if defined (CONFIG_MT7623_FPGA)
  i2c->control_reg = I2C_CONTROL_ACKERR_DET_EN;
#else
  i2c->control_reg = I2C_CONTROL_ACKERR_DET_EN | I2C_CONTROL_CLK_EXT_EN;
#endif
  if(i2c->dma_en) {
    i2c->control_reg |= I2C_CONTROL_DMA_EN;
  }
  if(I2C_MASTER_WRRD == i2c->op)
    i2c->control_reg |= I2C_CONTROL_DIR_CHANGE;

  if(HS_MODE == i2c->mode || (i2c->trans_data.trans_num > 1 && I2C_TRANS_REPEATED_START == i2c->st_rs)) {
    i2c->control_reg |= I2C_CONTROL_RS;
  }

  spin_lock(&i2c->lock);
  _i2c_write_reg(i2c);

  /*All register must be prepared before setting the start bit [SMP]*/
  I2C_MB();
#ifdef I2C_DRIVER_IN_KERNEL
  /*This is only for 3D CAMERA*/
  if (i2c->i2c_3dcamera_flag)
  {
    spin_unlock(&i2c->lock);
    if (g_i2c[0] == NULL)
    g_i2c[0] = i2c;
    else
    g_i2c[1] = i2c;

    goto end;
  }
#endif
  I2CINFO( I2C_T_TRANSFERFLOW, "Before start .....\n");
  #ifdef I2C_DEBUG_FS
    I2CLOG("mt_get_gpio_in I2C0_SDA=%d,I2C0_SCA=%d,I2C1_SDA=%d,I2C1_SCA=%d",\
      mt_get_gpio_in(GPIO_I2C0_SDA_PIN),mt_get_gpio_in(GPIO_I2C0_SCA_PIN),mt_get_gpio_in(GPIO_I2C1_SDA_PIN),mt_get_gpio_in(GPIO_I2C1_SCA_PIN));
  #endif

  /*Start the transfer*/
  i2c_writel(i2c, OFFSET_START, 0x0001);
  spin_unlock(&i2c->lock);
  ret = _i2c_deal_result(i2c);
  I2CINFO(I2C_T_TRANSFERFLOW, "After i2c transfer .....\n");
err:
end:
    return ret;
}
/*=========API in kernel=====================================================================*/
static void _i2c_translate_msg(mt_i2c *i2c,struct i2c_msg *msg)
{
  /*-------------compatible with 77/75 driver------*/
  if(msg->addr & 0xFF00){
    msg->ext_flag |= msg->addr & 0xFF00;
  }
  I2CINFO( I2C_T_TRANSFERFLOW, "Before i2c transfer .....\n");

  i2c->msg_buf = msg->buf;
  i2c->msg_len = msg->len;
  if(msg->ext_flag & I2C_RS_FLAG)
    i2c->st_rs = I2C_TRANS_REPEATED_START;
  else
    i2c->st_rs = I2C_TRANS_STOP;

  if(msg->ext_flag & I2C_DMA_FLAG)
    i2c->dma_en = TRUE;
  else
    i2c->dma_en = FALSE;

  if (msg->ext_flag & I2C_WR_FLAG)
    i2c->op = I2C_MASTER_WRRD;
  else
  {
    if(msg->flags & I2C_M_RD)
      i2c->op = I2C_MASTER_RD;
    else
      i2c->op = I2C_MASTER_WR;
  }
  if(msg->ext_flag & I2C_POLLING_FLAG)
    i2c->poll_en = TRUE;
  else
    i2c->poll_en = FALSE;
  if(msg->ext_flag & I2C_A_FILTER_MSG)
    i2c->filter_msg = TRUE;
  else
    i2c->filter_msg = FALSE;
  i2c->delay_len = (msg->timing & 0xff0000) >> 16;

  ///*Set device speed,set it before set_control register
  if(0 == (msg->timing & 0xFFFF)){
    i2c->mode  = ST_MODE;
    i2c->speed = MAX_ST_MODE_SPEED;
  }
  else
  {
    if (msg->ext_flag & I2C_HS_FLAG)
      i2c->mode  = HS_MODE;
    else
      i2c->mode  = FS_MODE;

    i2c->speed= msg->timing & 0xFFFF;
  }

  /*Set ioconfig*/
  if (msg->ext_flag & I2C_PUSHPULL_FLAG)
    i2c->pushpull=TRUE;
  else
    i2c->pushpull=FALSE;

  if (msg->ext_flag & I2C_3DCAMERA_FLAG)
    i2c->i2c_3dcamera_flag=TRUE;
  else
    i2c->i2c_3dcamera_flag=FALSE;

}

static S32 mt_i2c_start_xfer(mt_i2c *i2c, struct i2c_msg *msg)
{
  S32 return_value = 0;
  S32 ret = msg->len;
  //start=========================Check param valid=====================================//
  //I2CLOG(" mt_i2c_start_xfer.\n");
  //get the read/write flag
  i2c->read_flag=(msg->flags & I2C_M_RD);
  i2c->addr=msg->addr;
  if(i2c->addr == 0){
    I2CERR(" addr is invalid.\n");
    I2C_BUG_ON(i2c->addr == NULL);
    ret = -EINVAL_I2C;
    goto err;
  }

  if(msg->buf == NULL){
    I2CERR(" data buffer is NULL.\n");
    I2C_BUG_ON(msg->buf == NULL);
    ret = -EINVAL_I2C;
    goto err;
  }
  if (g_i2c[0] == i2c || g_i2c[1] == i2c) {
    I2CERR("mt-i2c%d: Current I2C Adapter is busy.\n", i2c->id);
    ret = -EINVAL_I2C;
    goto err;
  }
  //start=========================translate msg to mt_i2c===============================//
  _i2c_translate_msg(i2c,msg);
  /*This is only for 3D CAMERA*//*Save address infomation for 3d camera*/
  #ifdef I2C_DRIVER_IN_KERNEL
    if (i2c->i2c_3dcamera_flag)
    {
      if (g_msg[0].buf == NULL)
        memcpy((void *)&g_msg[0], msg, sizeof(struct i2c_msg));
      else
        memcpy((void *)&g_msg[1], msg, sizeof(struct i2c_msg));
    }
  #endif
  //end=========================translate msg to mt_i2c===============================//
  mt_i2c_clock_enable(i2c);
  return_value =_i2c_transfer_interface(i2c);
  if (!(msg->ext_flag & I2C_3DCAMERA_FLAG))
	mt_i2c_clock_disable(i2c);
  if ( return_value < 0 )
  {
    ret =-EINVAL_I2C;
    goto err;
  }
err:
  return ret;
}

static S32 mt_i2c_do_transfer(mt_i2c *i2c, struct i2c_msg *msgs, S32 num)
{
  S32 ret = 0;
  S32 left_num = num;

  while (left_num--) {
      ret = mt_i2c_start_xfer(i2c, msgs++);
      if ( ret < 0 ){
        if ( ret != -EINVAL_I2C ) /*We never try again when the param is invalid*/
          return -EAGAIN;
        else
          return -EINVAL_I2C;
      }
  }
  /*the return value is number of executed messages*/
  return num;
}

static S32 mt_i2c_transfer(struct i2c_adapter *adap, struct i2c_msg msgs[], S32 num)
{
  S32 ret = 0;
  S32 retry;
  mt_i2c *i2c = i2c_get_adapdata(adap);

  for (retry = 0; retry < adap->retries; retry++)
  {
    ret = mt_i2c_do_transfer(i2c, msgs, num);
    if (ret != -EAGAIN) {
      break;
    }
    if ( retry < adap->retries - 1 )
      udelay(100);
  }

  if (ret != -EAGAIN)
    return ret;
  else
    return -EREMOTEIO;
}
#ifdef I2C_DRIVER_IN_KERNEL
static S32 _i2c_deal_result_3dcamera(mt_i2c *i2c, struct i2c_msg *msg)
{
  U16 addr = msg->addr;
  U16 read = (msg->flags & I2C_M_RD);
  i2c->msg_buf=msg->buf;
  i2c->msg_len=msg->len;
  i2c->addr = read ? ((addr << 1) | 0x1) : ((addr << 1) & ~0x1);
  return _i2c_deal_result(i2c);
}
#endif

static void mt_i2c_clock_enable(mt_i2c *i2c)
{
  #if (!defined(CONFIG_MT_I2C_FPGA_ENABLE))
	if (i2c->dma_en){
		I2CINFO( I2C_T_TRANSFERFLOW, "Before dma clock enable .....\n");
		enable_clock(MT_CG_PERI_AP_DMA, "i2c");
	}
    I2CINFO( I2C_T_TRANSFERFLOW, "Before i2c clock enable .....\n");
    enable_clock(i2c->pdn, "i2c");
    I2CINFO( I2C_T_TRANSFERFLOW, "clock enable done.....\n");
  #endif
  return;
}
static void mt_i2c_clock_disable(mt_i2c *i2c)
{
  #if (!defined(CONFIG_MT_I2C_FPGA_ENABLE))
	if (i2c->dma_en){
		I2CINFO( I2C_T_TRANSFERFLOW, "Before dma clock disable .....\n");
		disable_clock(MT_CG_PERI_AP_DMA, "i2c");
	}
    I2CINFO( I2C_T_TRANSFERFLOW, "Before i2c clock disable .....\n");
    disable_clock(i2c->pdn, "i2c");
    I2CINFO( I2C_T_TRANSFERFLOW, "clock disable done .....\n");
  #endif
  return;
}
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
  mt_i2c *i2c = (mt_i2c *)dev_id;
  //U32 base = i2c->base;

  I2CINFO( I2C_T_TRANSFERFLOW, "i2c interrupt coming.....\n");
  //I2CLOG("mt_i2c_irq\n");
  /*Clear interrupt mask*/
  i2c_writel(i2c,OFFSET_INTR_MASK,i2c_readl(i2c,OFFSET_INTR_MASK) & ~(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
  /*Save interrupt status*/
  i2c->irq_stat = i2c_readl(i2c, OFFSET_INTR_STAT);
  /*Clear interrupt status,write 1 clear*/
  i2c_writel(i2c,OFFSET_INTR_STAT, (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
  //dev_info(i2c->dev, "I2C interrupt status 0x%04X\n", i2c->irq_stat);

  /*Wake up process*/
  atomic_set(&i2c->trans_stop, 1);
  wake_up(&i2c->wait);
  return IRQ_HANDLED;
}

/*This function is only for 3d camera*/

S32 mt_wait4_i2c_complete(void)
{
  mt_i2c *i2c0 = g_i2c[0];
  mt_i2c *i2c1 = g_i2c[1];
  S32 result0, result1;
  S32 ret = 0;

  if ((i2c0 == NULL) ||(i2c1 == NULL)) {
    /*What's wrong?*/
    ret = -EINVAL_I2C;
    goto end;
  }

  result0 = _i2c_deal_result_3dcamera(i2c0, &g_msg[0]);
  result1 = _i2c_deal_result_3dcamera(i2c1, &g_msg[1]);

  if (result0 < 0 || result1 < 0) {
    ret = -EINVAL_I2C;
  }
  if(NULL != i2c0) mt_i2c_clock_disable(i2c0);
  if(NULL != i2c1) mt_i2c_clock_disable(i2c1);

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
  .master_xfer   = mt_i2c_transfer,
  .smbus_xfer    = NULL,
  .functionality = mt_i2c_functionality,
};
static inline void mt_i2c_init_hw(mt_i2c *i2c)
{
  i2c_writel(i2c,OFFSET_SOFTRESET, 0x0001);
  i2c_writel(i2c,OFFSET_DCM_EN, 0x0);
}

static void mt_i2c_free(mt_i2c *i2c)
{
  if (!i2c)
    return;

  free_irq(i2c->irqnr, i2c);
  i2c_del_adapter(&i2c->adap);
  kfree(i2c);
}

static S32 mt_i2c_probe(struct platform_device *pdev)
{
  S32 ret, irq;
  mt_i2c *i2c = NULL;
  struct resource *res;

  /* Request platform_device IO resource*/
  res   = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  irq   = platform_get_irq(pdev, 0);
  if (res == NULL || irq < 0)
       return -ENODEV;

  /* Request IO memory */
  if (!request_mem_region(res->start, resource_size(res), pdev->name)) {
    return -ENOMEM;
  }

  if (NULL == (i2c = kzalloc(sizeof(mt_i2c), GFP_KERNEL)))
    return -ENOMEM;

  /* initialize mt_i2c structure */
  i2c->id   = pdev->id;
  i2c->base = IO_PHYS_TO_VIRT(res->start);
  //i2c->base = 0x11011000;
  i2c->irqnr  = irq;
  #if (defined(CONFIG_MT_I2C_FPGA_ENABLE))
    i2c->clk  = I2C_CLK_RATE;
  #else
    i2c->clk  = mt_get_bus_freq();// is not ready

    switch(i2c->id){
      case 0:
        i2c->pdn = MT_CG_PERI_I2C0;
        break;
      case 1:
        i2c->pdn = MT_CG_PERI_I2C1;
        break;
      case 2:
        i2c->pdn = MT_CG_PERI_I2C2;
        break;
      default:
        dev_err(&pdev->dev, "Error id %d\n", i2c->id);
        break;
    }
  #endif
  i2c->dev  = &i2c->adap.dev;

  i2c->adap.dev.parent  = &pdev->dev;
  i2c->adap.nr      = i2c->id;
  i2c->adap.owner     = THIS_MODULE;
  i2c->adap.algo      = &mt_i2c_algorithm;
  i2c->adap.algo_data   = NULL;
  i2c->adap.timeout   = 2 * HZ; /*2s*/
  i2c->adap.retries   = 1; /*DO NOT TRY*/

  snprintf(i2c->adap.name, sizeof(i2c->adap.name), I2C_DRV_NAME);

  i2c->pdmabase = AP_DMA_BASE + 0x200 + (0x80*(i2c->id));

  spin_lock_init(&i2c->lock);
  init_waitqueue_head(&i2c->wait);

  ret = request_irq(irq, mt_i2c_irq, IRQF_TRIGGER_LOW, I2C_DRV_NAME, i2c);

  if (ret){
    dev_err(&pdev->dev, "Can Not request I2C IRQ %d\n", irq);
    goto free;
  }

  mt_i2c_init_hw(i2c);
  i2c_set_adapdata(&i2c->adap, i2c);
  ret = i2c_add_numbered_adapter(&i2c->adap);
  if (ret){
    dev_err(&pdev->dev, "failed to add i2c bus to i2c core\n");
    goto free;
  }
  platform_set_drvdata(pdev, i2c);

#ifdef I2C_DEBUG_FS
  ret = device_create_file(i2c->dev, &dev_attr_debug);
  if ( ret ){
    /*Do nothing*/
  }
#endif

  return ret;

free:
  mt_i2c_free(i2c);
  return ret;
}


static S32 mt_i2c_remove(struct platform_device *pdev)
{
  mt_i2c *i2c = platform_get_drvdata(pdev);
  if (i2c) {
    platform_set_drvdata(pdev, NULL);
    mt_i2c_free(i2c);
  }
  return 0;
}

#ifdef CONFIG_PM
  static S32 mt_i2c_suspend(struct platform_device *pdev, pm_message_t state)
  {
     // struct mt_i2c *i2c = platform_get_drvdata(pdev);
      //dev_dbg(i2c->dev,"[I2C %d] Suspend!\n", i2c->id);
      return 0;
  }

  static S32 mt_i2c_resume(struct platform_device *pdev)
  {
     // struct mt_i2c *i2c = platform_get_drvdata(pdev);
     // dev_dbg(i2c->dev,"[I2C %d] Resume!\n", i2c->id);
      return 0;
  }
#else
  #define mt_i2c_suspend  NULL
  #define mt_i2c_resume NULL
#endif

static struct platform_driver mt_i2c_driver = {
  .probe   = mt_i2c_probe,
  .remove  = mt_i2c_remove,
  .suspend = mt_i2c_suspend,
  .resume  = mt_i2c_resume,
  .driver  = {
        .name  = I2C_DRV_NAME,
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

#if defined (CONFIG_MT7623_FPGA)
unsigned long switch_bus_id(unsigned long bus_id)
{
	i2c_bus_id = bus_id;
	printk("I2C controller Bus ID is %x\n", (unsigned int)i2c_bus_id);
	return i2c_bus_id;
}
EXPORT_SYMBOL(switch_bus_id);

unsigned long switch_address_bytes(unsigned long addr_bytes)
{
	i2c_address_bytes = addr_bytes;
	printk("I2C controller address bytes is %x\n", (unsigned int)addr_bytes);
	return i2c_address_bytes;
}
EXPORT_SYMBOL(switch_address_bytes);

unsigned int switch_device_address(unsigned int dev_addr)
{
	i2c_dev_base_addr = dev_addr;
	printk("I2C controller base address 0x%x\n", dev_addr);
	return i2c_dev_base_addr;
}
EXPORT_SYMBOL(switch_device_address);

void i2c_write(u32 address, u8 *data, u32 nbytes)
{
    int ret = 0;
    int number = 0;
    int trans_num = 0;
    int timing = 0;
    int length = 0;
    unsigned int ext_flag = 0;
    char * vir_addr = NULL;    
    char temp_buf[8];    

    if (i2c_address_bytes == 2){
        length = (int)nbytes + 2;

        if (nbytes > 8){
            vir_addr = kzalloc(length, GFP_KERNEL);
            if(vir_addr == NULL){
                printk("kzalloc fail");
                goto wr_err;
            }
            *(vir_addr)     = (address >> 8) & 0xFF;
            *(vir_addr+1)   = address & 0xFF;
            memcpy((vir_addr+2), data, nbytes);
        }
        else{
            temp_buf[0]     = (char)(address >> 8) & 0xFF;
            temp_buf[1]     = (char)(address & 0xFF);
            memcpy(&temp_buf[2], data, nbytes);
        }
    }
    else if (i2c_address_bytes == 1){
        length = (int)nbytes + 1;

        if (nbytes > 8){
            vir_addr = kzalloc(length, GFP_KERNEL);
            if(vir_addr == NULL){
                printk("kzalloc fail");
                goto wr_err;
            }
            *(vir_addr)     = address & 0xFF;
            memcpy((vir_addr+1), data, nbytes);
        }
        else{
            temp_buf[0]     = (char)(address & 0xFF);
            memcpy(&temp_buf[1], data, nbytes);
        }
    }
    else{
        goto wr_err;
    }

    ext_flag &= 0x7FFFFFFF;
    ext_flag |= I2C_PUSHPULL_FLAG;
    ext_flag |= I2C_POLLING_FLAG;
    number = (trans_num << 8 ) | length;

    if (nbytes > 8)
        ret = i2c_trans_data(i2c_bus_id, i2c_dev_base_addr, vir_addr, number, ext_flag, timing);
    else
        ret = i2c_trans_data(i2c_bus_id, i2c_dev_base_addr, temp_buf, number, ext_flag, timing);
    if (ret < 0){
        printk("failed to i2c write (ret=%d)\n", ret);
        goto wr_err;
    }

wr_err:
    if (vir_addr)
        kfree(vir_addr);
}
EXPORT_SYMBOL(i2c_write);

void i2c_read(u8 *data, u32 nbytes)
{
    int ret = 0;
    int number = 0;
    int trans_num = 0;
    int timing = 0;
    int length = 0;
    unsigned int ext_flag = 0;

    ext_flag |= 0x80000000;
    ext_flag |= I2C_PUSHPULL_FLAG;
    ext_flag |= I2C_POLLING_FLAG;
    trans_num = 1;
    length = (int)nbytes;
    number = (trans_num << 8 ) | length;

    ret = i2c_trans_data(i2c_bus_id, i2c_dev_base_addr, data, number, ext_flag, timing);
    if (ret < 0){
        printk("failed to i2c read (ret=%d)\n", ret);
        goto rd_err;
    }

rd_err:
    return;
}
EXPORT_SYMBOL(i2c_read);

int  i2c_WriteRead(u32 address, u8 *data, u32 nbytes)
{
    int ret = 0;
    int number = 0;
    int trans_auxlen = 0;
    int timing = 0;
    int length = 0;
    unsigned int ext_flag = 0;

    if (i2c_address_bytes == 2){
        length = 2;
        *(data)     = (address >> 8) & 0xFF;
        *(data+1)   = address & 0xFF;
    }
    else if (i2c_address_bytes == 1){
        length = 1;
        *(data)     = address & 0xFF;
    }
    else{
        goto wrrd_err;
    }

    ext_flag |= I2C_WR_FLAG;
    ext_flag |= I2C_PUSHPULL_FLAG;
    ext_flag |= I2C_POLLING_FLAG;
    trans_auxlen = (int)nbytes;
    number = (trans_auxlen << 8 ) | length;

    ret = i2c_trans_data(i2c_bus_id, i2c_dev_base_addr, data, number, ext_flag, timing);
    if (ret < 0){
        printk("failed to i2c WriteRead (ret=%d)\n", ret);
        goto wrrd_err;
    }

wrrd_err:
    return ret;
}
EXPORT_SYMBOL(i2c_WriteRead);

void random_read_block(u32 address, u8 *data)
{
#if 1
    i2c_WriteRead(address, data, I2C_READ_BLOCK);
#else
    i2c_write(address, data, 0);
    i2c_read(data, I2C_READ_BLOCK);
#endif
}
EXPORT_SYMBOL(random_read_block);

#if 0   /* for API UT */
void random_read_block_test(u32 address, u8 *data)
{
    i2c_write(address, data, 0);
    i2c_read(data, I2C_READ_BLOCK);
}
EXPORT_SYMBOL(random_read_block_test);
#endif  /* for API UT */


u8 random_read_one_byte(u32 address)
{
    u8 data;
#if 1
    i2c_WriteRead(address, &data, 1);
#else
    i2c_write(address, &data, 0);
	i2c_read(&data, 1);
#endif
	return (data);
}
EXPORT_SYMBOL(random_read_one_byte);

#if 0   /* for API UT */
u8 random_read_one_byte_test(u32 address)
{
    u8 data;

    i2c_write(address, &data, 0);
	i2c_read(&data, 1);
	return (data);
}
EXPORT_SYMBOL(random_read_one_byte_test);
#endif  /* for API UT */

void random_write_block(u32 address, u8 *data)
{
    i2c_write(address, data, I2C_WRITE_BLOCK);
	mdelay(5);
}
EXPORT_SYMBOL(random_write_block);

void random_write_one_byte(u32 address, u8 *data)
{
    i2c_write(address, data, 1);
	mdelay(5);
}
EXPORT_SYMBOL(random_write_one_byte);

void i2c_eeprom_read(u32 address, u8 *data, u32 nbytes)
{
	int i;
	int nblock = nbytes / I2C_READ_BLOCK;
	int rem = nbytes % I2C_READ_BLOCK;

	for (i=0; i<nblock; i++) {
		random_read_block(address+i*I2C_READ_BLOCK, &data[i*I2C_READ_BLOCK]);
	}

	if (rem) {
		int offset = nblock*I2C_READ_BLOCK;
		for (i=0; i<rem; i++) {
			data[offset+i] = random_read_one_byte(address+offset+i);
		}
	}
}

void i2c_pcie_phy_write(u32 address, u8 data, u8 bitmask, u32 bit_pos)
{
	u8 k;

	k = random_read_one_byte(address);
	k &= ~bitmask;
	k |= ((data << bit_pos) & bitmask);
	random_write_one_byte(address, &k);
}

void i2c_eeprom_dump(void)
{
	u32 a;
	u8 v;

	for (a = 0; a < 256; a++) {
		if (a % 16 == 0)
			printk("%4x : ", a);
		v = random_read_one_byte(a);
		printk("%02x ", v);
		if (a % 16 == 15)
			printk("\n");
	}
}

void pcie_phy_init(void)
{
	/* Wwitch PHY mode to PCIe Host */
	i2c_pcie_phy_write(0xff, 0x20, 0xff, 0);	//0xFC[31:24]   0x20	//Change bank 2
	i2c_pcie_phy_write(0x00, 0x00, 0x10, 4);	//0x00[04:04]	0x00 	//rg_ssusb_force_pcie_mode_sel=b0, Host mode
	i2c_pcie_phy_write(0x00, 0x01, 0x08, 3);	//0x00[03:03]	0x01	//rg_ssusb_force_pcie_mode_en=b1
	i2c_pcie_phy_write(0x00, 0x00, 0x06, 1);	//0x00[02:01] 	0x00 	//rg_ssusb_force_phy_mode=0b0, PCI-e mode
	i2c_pcie_phy_write(0x00, 0x01, 0x01, 0);	//0x00[00:00] 	0x01 	//rg_ssusb_force_phy_mode_en=0b1

	/* PHY initialization */
	i2c_pcie_phy_write(0xff, 0x40, 0xff, 0);	//0xFC[31:24] 	0x40      //Change bank 4
	i2c_pcie_phy_write(0x3a, 0x24, 0xff, 0);	//0x38[31:16]   0x24      //TX SSC min margin issue
	i2c_pcie_phy_write(0x3b, 0x00, 0xff, 0);	//		0x00      //TX SSC min margin issue
	i2c_pcie_phy_write(0x38, 0x47, 0xff, 0);	//0x38[15:00]   0x47      //TX SSC min margin issue
	i2c_pcie_phy_write(0x39, 0x00, 0xff, 0);	//		0x00      //TX SSC min margin issue
	i2c_pcie_phy_write(0x3e, 0x47, 0xff, 0);	//0x3C[31:16]   0x47      //TX SSC min margin issue
	i2c_pcie_phy_write(0x3f, 0x00, 0xff, 0);	//		0x00      //TX SSC min margin issue
	i2c_pcie_phy_write(0x42, 0x47, 0xff, 0);	//0x40[31:16]   0x47      //TX SSC min margin issue
	i2c_pcie_phy_write(0x43, 0x00, 0xff, 0);	//		0x00      //TX SSC min margin issue
	i2c_pcie_phy_write(0x44, 0x24, 0xff, 0);	//0x44[15:00]   0x24      //TX SSC min margin issue
	i2c_pcie_phy_write(0x45, 0x00, 0xff, 0);	//		0x00      //TX SSC min margin issue
	i2c_pcie_phy_write(0x48, 0x47, 0xff, 0);	//0x48[15:00]   0x47      //TX SSC min margin issue
	i2c_pcie_phy_write(0x49, 0x00, 0xff, 0);	//		0x00      //TX SSC min margin issue

	i2c_pcie_phy_write(0xff, 0x30, 0xff, 0);	//0xFC[31:24] 	0x30      //Change bank 3
	i2c_pcie_phy_write(0x2b, 0x0f, 0x0f, 0);	//0x28[27:24]	0x0f      //PCIe Host Gen1 REFCLK issue

	i2c_pcie_phy_write(0xff, 0x10, 0xff, 0);	//0xFC[31:24] 	0x10      //Change bank 1
	i2c_pcie_phy_write(0x7f, 0x05, 0xf0, 4);	//0x7C[31:28]	0x05      //Better Jitter Tolerance performance
	i2c_pcie_phy_write(0x7f, 0x02, 0x0f, 0);	//0x7C[27:24]	0x02      //Better Jitter Tolerance performance

	i2c_pcie_phy_write(0xff, 0x40, 0xff, 0);	//0xFC[31:24] 	0x40      //Change bank 4
	i2c_pcie_phy_write(0x0d, 0x01, 0x70, 4);	//0x0C[14:12]	0x01      //TX PLL loop peaking over 3dB issue
	i2c_pcie_phy_write(0x13, 0x08, 0x0f, 0);	//0x10[27:24]	0x08      //TX PLL loop peaking over 3dB issue
	i2c_pcie_phy_write(0x09, 0x01, 0x03, 0);	//0x08[09:08]	0x01      //TX PLL loop peaking over 3dB issue

	i2c_pcie_phy_write(0xff, 0x20, 0xff, 0);	//0xFC[31:24] 	0x20      //Change bank 2
	i2c_pcie_phy_write(0x03, 0x0b, 0x0f, 0);	//0x00[27:24]	0x0b      //For E-BUF threshold adjustment

	printk("**********     PhyIP initial done     **********\n");
}

unsigned int pcie_link_state(u32 pmask)
{
	unsigned int value = 0;

	switch (pmask) {
	case 1:
		REGDATA(RSTCTL) |= RSTCTL_PCIE0_RST;		// pcie reset assert
		mdelay(10);
		REGDATA(RSTCTL) &= ~(RSTCTL_PCIE0_RST);	// pcie reset de-assert
		mdelay(10);
		REGDATA(PCICFG) |= PCIRST0(0x1);	//pcie PERST_N assert
		mdelay(10);
		REGDATA(PCICFG) &= ~PCIRST0(0x1);     //pcie PERST_N de-assert
		mdelay(500);	/* at least 100ms delay because PCIe v2.0 need more time from Gen1 to Gen2 */
		I2C_PCIE_DBG("SYSRST=%x\n", REGDATA(RSTCTL));
		I2C_PCIE_DBG("PERST_N=%x\n", REGDATA(PCICFG));
		value = *((unsigned int *)0xFA142050);
		I2C_PCIE_DBG("Port 0 link status=%x\n", value);
		break;
	case 2:
		REGDATA(RSTCTL) |= RSTCTL_PCIE1_RST;		// pcie reset assert
		mdelay(10);
		REGDATA(RSTCTL) &= ~(RSTCTL_PCIE1_RST);	// pcie reset de-assert
		mdelay(10);
		REGDATA(PCICFG) |= PCIRST1(0x1);	//pcie PERST_N assert
		mdelay(10);
		REGDATA(PCICFG) &= ~PCIRST1(0x1);     //pcie PERST_N de-assert
		mdelay(500);	/* at least 100ms delay because PCIe v2.0 need more time from Gen1 to Gen2 */
		I2C_PCIE_DBG("SYSRST=%x\n",REGDATA(RSTCTL) );
		I2C_PCIE_DBG("PERST_N=%x\n", REGDATA(PCICFG));
		value = *((unsigned int *)0xFA143050);
		I2C_PCIE_DBG("Port 1 link status=%x\n", value);
		break;
	case 4:
		REGDATA(RSTCTL) |= RSTCTL_PCIE2_RST;		// pcie reset assert
		mdelay(10);
		REGDATA(RSTCTL) &= ~(RSTCTL_PCIE2_RST);	// pcie reset de-assert
		mdelay(10);
		REGDATA(PCICFG) |= PCIRST2(0x1);	//pcie PERST_N assert
		mdelay(10);
		REGDATA(PCICFG) &= ~PCIRST2(0x1);     //pcie PERST_N de-assert
		mdelay(500);	/* at least 100ms delay because PCIe v2.0 need more time from Gen1 to Gen2 */
		I2C_PCIE_DBG("SYSRST=%x\n", REGDATA(RSTCTL));
		I2C_PCIE_DBG("PERST_N=%x\n", REGDATA(PCICFG));
		value = *((unsigned int *)0xFA144050);
		I2C_PCIE_DBG("Port 2 link status=%x\n", value);
	}
	return value;
}

struct clock_region {
	u8 start;
	u8 end;
	u8 count;
};
static struct clock_region target;
void pcie_phy_scan(u32 pmask)
{
	int i ,link;
	u8 c_start=0xff, c_count=0;

#if 0 /* just only for FPGA: switching sclk (or not) must be confirm by designer */
	/* configure CLKCFG */
	*((unsigned int *)0xFA00002C) &= ~(0x7<<8);	// set 3 Ports to internal 125MHZ
	switch (pmask) {
	case 1:
		*((unsigned int *)0xFA00002C) |= 0x1<<8;
		break;
	case 2:
		*((unsigned int *)0xFA00002C) |= 0x1<<9;
		break;
	case 4:
		*((unsigned int *)0xFA00002C) |= 0x1<<10;
		break;
	default:
		return;
	}
#endif
	I2C_PCIE_DBG("CFGCLK=%x\n", *((unsigned int *)0xFA00002c));
	printk("**********     Clock Phase Scan     **********\n");
	target.count = c_count;
	//switch_bus_id(I2C_BUS_ID1);
	for (i=0;i<32;i++) {
		i2c_pcie_phy_write(0xff, 0x50, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		i2c_pcie_phy_write(0x0b, i, 0xf8, 3);		//0x08[31:27]   0x20      //Change bank 2
		I2C_PCIE_DBG("PCLK=0x%x\n", i);
		mdelay(100);
		link = pcie_link_state(pmask);
		switch(link) {
		case 1:
			if (c_count == 0)
				c_start = i;
			c_count++;
			I2C_PCIE_DBG("c_count=0x%x\n", c_count);
			break;
		case 0:
			if (c_count > 0 && c_count > target.count) {
				target.start = c_start;
				target.end = i - 1;
				target.count = c_count;
				I2C_PCIE_DBG("target.start=0x%x\n", target.start);
				I2C_PCIE_DBG("target.end=0x%x\n", target.end);
				I2C_PCIE_DBG("target.count=0x%x\n", target.count);
			}
			c_start=0xff;
			c_count=0;
		}
	}
	if (c_count > 0 && c_count > target.count) {
		target.start = c_start;
		target.end = i - 1;
		target.count = c_count;
		I2C_PCIE_DBG("target.start=0x%x\n", target.start);
		I2C_PCIE_DBG("target.end=0x%x\n", target.end);
		I2C_PCIE_DBG("target.count=0x%x\n", target.count);
	}
	udelay(1000);
}

void pcie_phy_pclk_set(void)
{
	u8 set_pclk;

	if (target.count <= 0) {
		printk("No clock phase could be link !\n");
		return;
	}
	set_pclk = target.start;
	set_pclk += ((target.end - target.start) + 1) / 2;
	printk("Good Regin: start=0x%x, end=0x%x => pclk=%x\n", 
		target.start, target.end, set_pclk);
	i2c_pcie_phy_write(0xff, 0x50, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
	i2c_pcie_phy_write(0x0b, set_pclk, 0xf8, 3);		//0x08[31:27]   0x20      //Change bank 2
}

void pcie_phy_calibration(unsigned int port_mask, int debug)
{
	i2c_pcie_phy_debug = debug;

	if (port_mask & 0x1) {
		switch_bus_id(PCIE0_I2C_BUS);
		pcie_phy_init();
		pcie_phy_scan(0x1);
		pcie_phy_pclk_set();
	}
	if (port_mask & 0x2) {
		switch_bus_id(PCIE1_I2C_BUS);
		pcie_phy_init();
		pcie_phy_scan(0x2);
		pcie_phy_pclk_set();
	}
	if (port_mask & 0x4) {
		switch_bus_id(PCIE2_I2C_BUS);
		pcie_phy_init();
		pcie_phy_scan(0x4);
		pcie_phy_pclk_set();
	}
}
EXPORT_SYMBOL(pcie_phy_calibration);

void i2c_pcie_show(unsigned int port_mask)
{
	unsigned long value = 0;

	i2c_pcie_phy_debug = 1;
	/* configure CLKCFG */
	*((unsigned int *)0xFA00002C) &= ~(0x7<<8);	// set 3 Ports to internal 125MHZ
	if (port_mask & 0x1)
		value |= 0x1<<8;
	if (port_mask & 0x2)
		value |= 0x1<<9;
	if (port_mask & 0x4)
		value |= 0x1<<10;
	*((unsigned int *)0xFA00002C) |= value;
	printk("CFGCLK=%x\n", *((unsigned int *)0xFA00002c));
	mdelay(100);
	if (port_mask & 0x1) {
		switch_bus_id(PCIE0_I2C_BUS);
		i2c_pcie_phy_write(0xff, 0x10, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 1\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x20, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 2\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x30, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 3\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x40, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 4\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x50, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 5\n");
		i2c_eeprom_dump();
		pcie_link_state(0x1);
	}
	if (port_mask & 0x2) {
		switch_bus_id(PCIE1_I2C_BUS);
		i2c_pcie_phy_write(0xff, 0x10, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 1\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x20, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 2\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x30, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 3\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x40, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 4\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x50, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 5\n");
		i2c_eeprom_dump();
		pcie_link_state(0x2);
	}
	if (port_mask & 0x4) {
		switch_bus_id(PCIE2_I2C_BUS);
		i2c_pcie_phy_write(0xff, 0x10, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 1\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x20, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 2\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x30, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 3\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x40, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 4\n");
		i2c_eeprom_dump();
		i2c_pcie_phy_write(0xff, 0x50, 0xff, 0);	//0xFC[31:24]   0x50      //Change bank 5
		printk("BANK 5\n");
		i2c_eeprom_dump();
		pcie_link_state(0x4);
	}
	i2c_pcie_phy_debug = 0;
}
EXPORT_SYMBOL(i2c_pcie_show);

#if defined (CONFIG_RALINK_I2S) ||  defined (CONFIG_RALINK_I2S_MODULE)
void i2c_wmcodec_write(u32 address, u32 data)
{
        char addr_buf;
        char data_buf;

	switch_bus_id(I2C_BUS_ID);

        switch_address_bytes(1);
#if defined(CONFIG_SND_SOC_WM8960) || ((!defined(CONFIG_SND_RALINK_SOC)) && defined(CONFIG_I2S_WM8960))
        switch_device_address(WM8960_ADDR);
#elif defined(CONFIG_SND_SOC_WM8750) || ((!defined(CONFIG_SND_RALINK_SOC)) && defined(CONFIG_I2S_WM8750)) || ((!defined(CONFIG_SND_RALINK_SOC)) && defined(CONFIG_I2S_WM8751))
        switch_device_address(WM8751_ADDR);
#else
        switch_device_address(WM8960_ADDR);
#endif

        addr_buf = (char)(address<<1)|(0x1&(data>>8));
        data_buf = (char)(data & 0xFF);
        //printk("addr=0x%08x, data=0x%80x\n", addr_buf, data_buf);
        i2c_write(addr_buf, &data_buf, 1);

}
EXPORT_SYMBOL(i2c_wmcodec_write);
#endif
#endif  /* #if defined (CONFIG_MT7623_FPGA) */

module_init(mt_i2c_init);
module_exit(mt_i2c_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek I2C Bus Driver");
MODULE_AUTHOR("Infinity Chen <infinity.chen@mediatek.com>");

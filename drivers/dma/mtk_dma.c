#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/memory.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/version.h>

#include "mtk_hsdma.h"
#include "mtk_dma.h"


/************************ DMA engine API functions ****************************/

#define to_mtk_dma_chan(chan)            \
	container_of(chan, struct mtk_dma_chan, common)
static int hsdma_rx_dma_owner_idx0;
static int hsdma_rx_calc_idx0;
static unsigned long hsdma_tx_cpu_owner_idx0 = 0;
static unsigned long updateCRX = 0;

dma_cookie_t mtk_dma_tx_submit(struct dma_async_tx_descriptor *tx)
{
	dma_cookie_t cookie;
	
	//printk("%s\n",__FUNCTION__);
	if (tx->chan)
		cookie = tx->chan->cookie;

	return cookie;
}

struct HSdmaReqEntry HSDMA_Entry;

#define MIN_MTKDMA_PKT_LEN	128
struct dma_async_tx_descriptor *
mtk_dma_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
		size_t len, unsigned long flags)
{
	struct mtk_dma_chan *mtk_chan = to_mtk_dma_chan(chan);
	unsigned long mid_offset;
	unsigned long i;
 
	//printk("%x->%x len=%d ch=%d\n", src, dest, len, chan->chan_id);
	spin_lock_bh(&mtk_chan->lock);

	//if (len < MIN_MTKDMA_PKT_LEN) {
	//	dma_async_tx_descriptor_init(&mtk_chan->txd, chan);
	//	memcpy(phys_to_virt(dest), phys_to_virt(src), len);	
	//} else {
		if ((dest & 0x03) != 0) {
			dma_async_tx_descriptor_init(&mtk_chan->txd, chan);
		  	memcpy(phys_to_virt(dest), phys_to_virt(src), len);	
		}
		else {
			//mid_offset = len;
			hsdma_rx_dma_owner_idx0 = (hsdma_rx_calc_idx0 + 1) % NUM_HSDMA_RX_DESC;
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info1.SDP0 = (src & 0xFFFFFFFF);
			HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info1.PDP0 = (dest & 0xFFFFFFFF);
	  
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.SDL0 = len;
			HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info2.PLEN0 = len;
		  
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.LS0_bit = 1;
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.DDONE_bit = 0;
			HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info2.DDONE_bit = 0;		
		
			hsdma_tx_cpu_owner_idx0 = (hsdma_tx_cpu_owner_idx0+1) % NUM_HSDMA_TX_DESC;
			hsdma_rx_calc_idx0 = (hsdma_rx_calc_idx0 + 1) % NUM_HSDMA_RX_DESC;
			sysRegWrite(HSDMA_TX_CTX_IDX0, cpu_to_le32((u32)hsdma_tx_cpu_owner_idx0));
			
			dma_async_tx_descriptor_init(&mtk_chan->txd, chan);
		  
		}
	//}

	spin_unlock_bh(&mtk_chan->lock);

	return &mtk_chan->txd;
}

int hsdma_housekeeping(void)
{
	int i;

#if 1
	i = (sysRegRead(HSDMA_RX_CALC_IDX0)+1)%NUM_HSDMA_RX_DESC;	   

	while (1) {
		if (HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit == 1) { 
			//HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit = 0; // RX_Done_bit=1->0
			updateCRX=i;
			i = (i + 1)%NUM_HSDMA_RX_DESC;
		}	
		else {
			break;
		}
	} 
	sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32)updateCRX)); //update RX CPU IDX 
#else
	for (i=0;i<NUM_HSDMA_RX_DESC;i++){
		if (HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit == 1) { 
			updateCRX = i; 
			HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit = 0; // RX_Done_bit=1->0
			sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32)updateCRX)); //update RX CPU IDXSA
		}	
	}
	sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32)updateCRX)); //update RX CPU IDX
#endif

	return 0;
}
//EXPORT_SYMBOL(hsdma_housekeeping);


void set_fe_HSDMA_glo_cfg(void)
{
	int HSDMA_glo_cfg = 0;

	printk("%s\n",__FUNCTION__);
	HSDMA_glo_cfg = (HSDMA_TX_WB_DDONE | HSDMA_RX_DMA_EN | HSDMA_TX_DMA_EN | HSDMA_BT_SIZE_16DWORDS | HSDMA_MUTI_ISSUE );
	sysRegWrite(HSDMA_GLO_CFG, HSDMA_glo_cfg);
}

int HSDMA_init(void)
{
	int		i;
	unsigned int	regVal;

	printk("%s\n",__FUNCTION__);
	while (1) {
		regVal = sysRegRead(HSDMA_GLO_CFG);
		if ((regVal & HSDMA_RX_DMA_BUSY)) {
			printk("\n  RX_DMA_BUSY !!! ");
			continue;
		}
		if ((regVal & HSDMA_TX_DMA_BUSY)) {
			printk("\n  TX_DMA_BUSY !!! ");
			continue;
		}
		break;
	}
	//initial TX ring0
	HSDMA_Entry.HSDMA_tx_ring0 = pci_alloc_consistent(NULL, NUM_HSDMA_TX_DESC * sizeof(struct HSDMA_txdesc), &HSDMA_Entry.phy_hsdma_tx_ring0);
	printk("\n hsdma_phy_tx_ring0 = 0x%08x, hsdma_tx_ring0 = 0x%p\n", HSDMA_Entry.phy_hsdma_tx_ring0, HSDMA_Entry.HSDMA_tx_ring0);
	
		
	for (i=0; i < NUM_HSDMA_TX_DESC; i++) {
		memset(&HSDMA_Entry.HSDMA_tx_ring0[i],0,sizeof(struct HSDMA_txdesc));
		HSDMA_Entry.HSDMA_tx_ring0[i].hsdma_txd_info2.LS0_bit = 1;
		HSDMA_Entry.HSDMA_tx_ring0[i].hsdma_txd_info2.DDONE_bit = 1;
	}

	//initial RX ring0
	HSDMA_Entry.HSDMA_rx_ring0 = pci_alloc_consistent(NULL, NUM_HSDMA_RX_DESC * sizeof(struct HSDMA_rxdesc), &HSDMA_Entry.phy_hsdma_rx_ring0);
	
	
	for (i = 0; i < NUM_HSDMA_RX_DESC; i++) {
		memset(&HSDMA_Entry.HSDMA_rx_ring0[i],0,sizeof(struct HSDMA_rxdesc));
		HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit = 0;
		HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.LS0 = 0;
	}	
		printk("\n hsdma_phy_rx_ring0 = 0x%08x, hsdma_rx_ring0 = 0x%p\n",HSDMA_Entry.phy_hsdma_rx_ring0,HSDMA_Entry.HSDMA_rx_ring0);
	
	// HSDMA_GLO_CFG
	regVal = sysRegRead(HSDMA_GLO_CFG);
	regVal &= 0x000000FF;
	sysRegWrite(HSDMA_GLO_CFG, regVal);
	regVal=sysRegRead(HSDMA_GLO_CFG);
	/* Tell the adapter where the TX/RX rings are located. */
	//TX0
	//sysRegWrite(HSDMA_TX_BASE_PTR0, phys_to_bus((u32) HSDMA_Entry.phy_hsdma_tx_ring0));
	sysRegWrite(HSDMA_TX_BASE_PTR0, HSDMA_Entry.phy_hsdma_tx_ring0);
	sysRegWrite(HSDMA_TX_MAX_CNT0, cpu_to_le32((u32) NUM_HSDMA_TX_DESC));
	sysRegWrite(HSDMA_TX_CTX_IDX0, 0);
	hsdma_tx_cpu_owner_idx0 = 0;
	sysRegWrite(HSDMA_RST_CFG, HSDMA_PST_DTX_IDX0);
	printk("TX_CTX_IDX0 = %x\n", sysRegRead(HSDMA_TX_CTX_IDX0));
	printk("TX_DTX_IDX0 = %x\n", sysRegRead(HSDMA_TX_DTX_IDX0));

	    
	//RX0
	//sysRegWrite(HSDMA_RX_BASE_PTR0, phys_to_bus((u32) HSDMA_Entry.phy_hsdma_rx_ring0));
	sysRegWrite(HSDMA_RX_BASE_PTR0, HSDMA_Entry.phy_hsdma_rx_ring0);
	sysRegWrite(HSDMA_RX_MAX_CNT0,  cpu_to_le32((u32) NUM_HSDMA_RX_DESC));
	sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32) (NUM_HSDMA_RX_DESC - 1)));
	hsdma_rx_calc_idx0 = hsdma_rx_dma_owner_idx0 =  sysRegRead(HSDMA_RX_CALC_IDX0);
	sysRegWrite(HSDMA_RST_CFG, HSDMA_PST_DRX_IDX0);
	printk("RX_CRX_IDX0 = %x\n", sysRegRead(HSDMA_RX_CALC_IDX0));
	printk("RX_DRX_IDX0 = %x\n", sysRegRead(HSDMA_RX_DRX_IDX0));

	set_fe_HSDMA_glo_cfg();
	printk("HSDMA_GLO_CFG = %x\n", sysRegRead(HSDMA_GLO_CFG));
	return 1;
}

/**
 * mtk_dma_status - poll the status of an XOR transaction
 * @chan: XOR channel handle
 * @cookie: XOR transaction identifier
 * @txstate: XOR transactions state holder (or NULL)
 */
enum dma_status mtk_dma_status(struct dma_chan *chan,
					  dma_cookie_t cookie,
					  struct dma_tx_state *txstate)
{
	hsdma_housekeeping();

	return DMA_SUCCESS;
}


irqreturn_t mtk_dma_interrupt_handler(int irq, void *data)
{
	//printk("%s\n",__FUNCTION__);

	return IRQ_HANDLED;
}

void mtk_dma_issue_pending(struct dma_chan *chan)
{
	//printk("%s\n",__FUNCTION__);
}


int mtk_dma_alloc_chan_resources(struct dma_chan *chan)
{
	//("%s\n",__FUNCTION__);

	return 0;
}

void mtk_dma_free_chan_resources(struct dma_chan *chan)
{
	//printk("%s\n",__FUNCTION__);

}

int mtk_dma_probe(struct platform_device *pdev)
{
	struct dma_device *dma_dev;
	struct mtk_dma_chan *mtk_chan;
	int err;
	int ret;
	unsigned long reg_int_mask=0;

	//printk("%s\n",__FUNCTION__);
	
	dma_dev = devm_kzalloc(&pdev->dev, sizeof(*dma_dev), GFP_KERNEL);
	if (!dma_dev)
		return -ENOMEM;


	INIT_LIST_HEAD(&dma_dev->channels);
	dma_cap_zero(dma_dev->cap_mask);
	dma_cap_set(DMA_MEMCPY, dma_dev->cap_mask);
	//dma_cap_set(DMA_SLAVE, dma_dev->cap_mask);
	dma_dev->device_alloc_chan_resources = mtk_dma_alloc_chan_resources;
	dma_dev->device_free_chan_resources = mtk_dma_free_chan_resources;
	dma_dev->device_tx_status = mtk_dma_status;
	dma_dev->device_issue_pending = mtk_dma_issue_pending;
	dma_dev->device_prep_dma_memcpy = mtk_dma_prep_dma_memcpy;
	dma_dev->dev = &pdev->dev;

	mtk_chan = devm_kzalloc(&pdev->dev, sizeof(*mtk_chan), GFP_KERNEL);
        if (!mtk_chan) {
		return -ENOMEM;
	}

	spin_lock_init(&mtk_chan->lock);	
	INIT_LIST_HEAD(&mtk_chan->chain);
	INIT_LIST_HEAD(&mtk_chan->completed_slots);
	INIT_LIST_HEAD(&mtk_chan->all_slots);
	mtk_chan->common.device = dma_dev;
	mtk_chan->txd.tx_submit = mtk_dma_tx_submit;

	list_add_tail(&mtk_chan->common.device_node, &dma_dev->channels);

	err = dma_async_device_register(dma_dev);

	if (0 != err) {
		pr_err("ERR_MDMA:device_register failed: %d\n", err);
		return 1;
	}
	
	sysRegWrite(HSDMA_INT_MASK, reg_int_mask  & ~(HSDMA_FE_INT_TX));  // disable int TX DONE
	sysRegWrite(HSDMA_INT_MASK, reg_int_mask  & ~(HSDMA_FE_INT_RX) );  // disable int RX DONE
	printk("reg_int_mask=%lu, INT_MASK= %x \n", reg_int_mask, sysRegRead(HSDMA_INT_MASK));
  	HSDMA_init();	

	return 0;
}

int  mtk_dma_remove(struct platform_device *dev)
{
	struct dma_device *dma_dev = platform_get_drvdata(dev);

	//printk("%s\n",__FUNCTION__);

	dma_async_device_unregister(dma_dev);

	return 0;
}

struct platform_device mt7623_hsdma_device = {
    .name   = "mt7623-hsdma",
    .id     = -1,
};

struct platform_driver mtk_dma_driver = {
	.probe		= mtk_dma_probe,
	.remove		= mtk_dma_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "mt7623-hsdma",
	},
};

int __init mtk_dma_init(void)
{
	int rc;

	rc = platform_device_register(&mt7623_hsdma_device);
	if (rc) {
	        printk("****[mtk_dma_init] Unable to device register(%d)\n", rc);
        	return rc;
	}    
	rc = platform_driver_register(&mtk_dma_driver);
	return rc;
}
//module_init(mtk_dma_init);
late_initcall(mtk_dma_init);

MODULE_DESCRIPTION("DMA engine driver for Mediatek DMA engine");
MODULE_LICENSE("GPL");

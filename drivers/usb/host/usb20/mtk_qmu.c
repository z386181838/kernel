#ifdef MUSB_QMU_SUPPORT
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/list.h>
#include <linux/musb/musb_qmu.h>
#include <linux/musb/musb_host.h>

static PGPD Rx_gpd_head[15];
static PGPD Tx_gpd_head[15];
static PGPD Rx_gpd_end[15];
static PGPD Tx_gpd_end[15];
static PGPD Rx_gpd_last[15];
static PGPD Tx_gpd_last[15];
static GPD_R Rx_gpd_List[15];
static GPD_R Tx_gpd_List[15];
static u64 Rx_gpd_Offset[15];
static u64 Tx_gpd_Offset[15];

//jingao: add
#define QMU_ONE_BY_ONE  0
//jingao: add

extern void __iomem* qmu_base;

#ifdef MUSB_QMU__HOST_BULK_RX_AUTO_SPLIT 
#define QMU_RX_SPLIT_BLOCK_SIZE (32*1024)
#define QMU_RX_SPLIT_THRE    (64*1024) //(64*1024)
#endif


u8 PDU_calcCksum(u8 *data, int len)
{
	u8 *uDataPtr, ckSum;
	int i;

	*(data + 1) = 0x0;
	uDataPtr = data;
	ckSum = 0;
	for (i = 0; i < len; i++)
		ckSum += *(uDataPtr + i);

	return 0xFF - ckSum;
}

static PGPD get_gpd(u8 isRx, u32 num){
	PGPD ptr;
	if(isRx) {
		ptr = Rx_gpd_List[num].pNext;
		Rx_gpd_List[num].pNext = (PGPD)((u8 *)(Rx_gpd_List[num].pNext) + GPD_LEN_ALIGNED);

		if ( Rx_gpd_List[num].pNext >= Rx_gpd_List[num].pEnd ) {
			Rx_gpd_List[num].pNext = Rx_gpd_List[num].pStart;
		}
	} else {
		ptr = Tx_gpd_List[num].pNext;
		Tx_gpd_List[num].pNext = (PGPD)((u8 *)(Tx_gpd_List[num].pNext) + GPD_LEN_ALIGNED);

		if ( Tx_gpd_List[num].pNext >= Tx_gpd_List[num].pEnd ) {
			Tx_gpd_List[num].pNext = Tx_gpd_List[num].pStart;
		}
	}
	return ptr;
}

static void gpd_ptr_align(u8 isRx, u32 num, PGPD ptr)
{
	if(isRx)
		Rx_gpd_List[num].pNext = (PGPD)((u8 *)(ptr) + GPD_LEN_ALIGNED);
	else
		Tx_gpd_List[num].pNext = (PGPD)((u8 *)(ptr) + GPD_LEN_ALIGNED);
}

static dma_addr_t gpd_virt_to_phys(void *vaddr, u8 isRx, u32 num)
{
	dma_addr_t paddr;

	if (isRx) {
		paddr = (dma_addr_t)((u64)(unsigned long)vaddr - Rx_gpd_Offset[num]);
	} else {
		paddr = (dma_addr_t)((u64)(unsigned long)vaddr - Tx_gpd_Offset[num]);
	}

	QMU_INFO("%s[%d]phys=%p<->virt=%p\n",
			((isRx == RXQ)?"RQ":"TQ"), num, (void *)paddr, vaddr);

	return paddr;
}

static void *gpd_phys_to_virt(dma_addr_t paddr, u8 isRx, u32 num)
{
	void *vaddr;


	if (isRx) {
		vaddr = (void *)(unsigned long)((u64)paddr + Rx_gpd_Offset[num]);
	} else {
		vaddr = (void *)(unsigned long)((u64)paddr + Tx_gpd_Offset[num]);
	}
	QMU_INFO("%s[%d]phys=%p<->virt=%p\n",
			((isRx == RXQ)?"RQ":"TQ"), num , (void *)paddr, vaddr);

	return vaddr;
}

static void init_gpd_list(u8 isRx, int num, PGPD ptr, PGPD io_ptr, u32 size)
{
	if (isRx) {
		Rx_gpd_List[num].pStart = ptr;
		Rx_gpd_List[num].pEnd = (PGPD)( (u8*)(ptr + size) + (GPD_EXT_LEN*size) );
		Rx_gpd_Offset[num]=(u64)(unsigned long)ptr - (u64)(unsigned long)io_ptr;
		ptr++;
		Rx_gpd_List[num].pNext = (PGPD)((u8*)ptr + GPD_EXT_LEN);

		QMU_INFO("Rx_gpd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n", \
				num, Rx_gpd_List[num].pStart, Rx_gpd_List[num].pNext, Rx_gpd_List[num].pEnd);
		QMU_INFO("Rx_gpd_Offset[%d]=%p\n", num, (void *)(unsigned long)Rx_gpd_Offset[num]);
	} else {
		Tx_gpd_List[num].pStart = ptr;
		Tx_gpd_List[num].pEnd = (PGPD)( (u8*)(ptr + size) + (GPD_EXT_LEN*size) );
		Tx_gpd_Offset[num]=(u64)(unsigned long)ptr - (u64)(unsigned long)io_ptr;
		ptr++;
		Tx_gpd_List[num].pNext = (PGPD)((u8*)ptr + GPD_EXT_LEN);

		QMU_INFO("Tx_gpd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n", \
				num, Tx_gpd_List[num].pStart, Tx_gpd_List[num].pNext, Tx_gpd_List[num].pEnd);
		QMU_INFO("Tx_gpd_Offset[%d]=%p\n", num, (void *)(unsigned long)Tx_gpd_Offset[num]);
	}
}

int qmu_init_gpd_pool(struct device *dev){
	u32 i, size;
	TGPD *ptr,*io_ptr;
	dma_addr_t  dma_handle;
	u32 gpd_sz;

	gpd_sz = (u32)(u64)sizeof(TGPD);
	QMU_INFO("sizeof(TGPD):%d\n", gpd_sz);
	if(gpd_sz != GPD_SZ){
		QMU_ERR("ERR!!!, GPD SIZE != %d\n", GPD_SZ);
	}

	for ( i = 1; i<= RXQ_NUM; i++) {
		/* Allocate Rx GPD */

        //printk("qmu_init_gpd_pool RXQ_NUM 11 i = %d\n",i);
		size = GPD_LEN_ALIGNED * MAX_GPD_NUM;
		ptr = (TGPD*)dma_alloc_coherent(dev, size, &dma_handle, GFP_KERNEL);
		if(!ptr){
			return -ENOMEM ;
		}
		memset(ptr, 0 , size);
		io_ptr = (TGPD *)(dma_handle);

		init_gpd_list(RXQ, i, ptr, io_ptr, MAX_GPD_NUM);
		Rx_gpd_head[i]= ptr;
		QMU_INFO("ALLOC RX GPD Head [%d] Virtual Mem=%p, DMA addr=%p\n", i, Rx_gpd_head[i], io_ptr);
		Rx_gpd_end[i] = Rx_gpd_last[i] = Rx_gpd_head[i];
		TGPD_CLR_FLAGS_HWO(Rx_gpd_end[i]);
		gpd_ptr_align(RXQ, i, Rx_gpd_end[i]);
		QMU_INFO("RQSAR[%d]=%p\n", i, (void *)gpd_virt_to_phys(Rx_gpd_end[i],RXQ,i));
	}

	for ( i = 1; i<= TXQ_NUM; i++) {
        //printk("qmu_init_gpd_pool TXQ_NUM 11 i = %d\n",i);
		/* Allocate Tx GPD */
		size = GPD_LEN_ALIGNED * MAX_GPD_NUM;
		ptr = (TGPD*)dma_alloc_coherent(dev, size, &dma_handle, GFP_KERNEL);
		if(!ptr){
			return -ENOMEM ;
		}
		memset(ptr, 0 , size);
		io_ptr = (TGPD *)(dma_handle);

		init_gpd_list(TXQ, i, ptr, io_ptr, MAX_GPD_NUM);
		Tx_gpd_head[i]= ptr;
		QMU_INFO("ALLOC TX GPD Head [%d] Virtual Mem=%p, DMA addr=%p\n", i, Tx_gpd_head[i], io_ptr);
		Tx_gpd_end[i] = Tx_gpd_last[i] = Tx_gpd_head[i];
		TGPD_CLR_FLAGS_HWO(Tx_gpd_end[i]);
		gpd_ptr_align(TXQ, i, Tx_gpd_end[i]);
		QMU_INFO("TQSAR[%d]=%p\n", i, (void *)gpd_virt_to_phys(Tx_gpd_end[i],TXQ,i));
	}

	return 0;
}

void qmu_reset_gpd_pool(u32 ep_num, u8 isRx)
{
	u32 size = GPD_LEN_ALIGNED * MAX_GPD_NUM;

	/* SW reset */
	if(isRx){
		memset(Rx_gpd_head[ep_num], 0 , size);
		Rx_gpd_end[ep_num] = Rx_gpd_last[ep_num] = Rx_gpd_head[ep_num];
		TGPD_CLR_FLAGS_HWO(Rx_gpd_end[ep_num]);
		gpd_ptr_align(isRx, ep_num, Rx_gpd_end[ep_num]);

	}else{
		memset(Tx_gpd_head[ep_num], 0 , size);
		Tx_gpd_end[ep_num] = Tx_gpd_last[ep_num] = Tx_gpd_head[ep_num];
		TGPD_CLR_FLAGS_HWO(Tx_gpd_end[ep_num]);
		gpd_ptr_align(isRx, ep_num, Tx_gpd_end[ep_num]);
	}
}

void qmu_destroy_gpd_pool(struct device *dev){

	int i;
	u32 size = GPD_LEN_ALIGNED * MAX_GPD_NUM;

	for ( i = 1; i<= RXQ_NUM; i++) {
		dma_free_coherent(dev, size, Rx_gpd_head[i], gpd_virt_to_phys(Rx_gpd_head[i], RXQ, i));
	}

	for ( i = 1; i<= TXQ_NUM; i++) {
		dma_free_coherent(dev, size, Tx_gpd_head[i], gpd_virt_to_phys(Tx_gpd_head[i], TXQ, i));
	}
}
//#ifdef MUSB_QMU_SUPPORT_HOST
static void prepare_rx_gpd_ioc(u8 *pBuf, u32 data_len, u8 ep_num,u8 isioc)
{
	TGPD* gpd;

	/* get gpd from tail */
	gpd = Rx_gpd_end[ep_num];

	TGPD_SET_DATA(gpd, pBuf);
	TGPD_CLR_FORMAT_BDP(gpd);

	TGPD_SET_DataBUF_LEN(gpd, data_len);
	TGPD_SET_BUF_LEN(gpd, 0);

//	TGPD_CLR_FORMAT_BPS(gpd);
	if(isioc){
	TGPD_SET_IOC(gpd);
	}

	
	/* update gpd tail */
	Rx_gpd_end[ep_num] = get_gpd(RXQ ,ep_num);
	QMU_INFO("[RX]""Rx_gpd_end[%d]=%p gpd=%p\n", ep_num, Rx_gpd_end[ep_num], gpd);
	memset(Rx_gpd_end[ep_num], 0 , GPD_LEN_ALIGNED);
	TGPD_CLR_FLAGS_HWO(Rx_gpd_end[ep_num]);

	/* make sure struct ready before set to next*/
	mb();
	TGPD_SET_NEXT(gpd, gpd_virt_to_phys(Rx_gpd_end[ep_num], RXQ, ep_num));

	TGPD_SET_CHKSUM_HWO(gpd, 16);

	/* make sure struct ready before HWO */
	mb();
	TGPD_SET_FLAGS_HWO(gpd);
}
//#endif

static void prepare_rx_gpd(u8 *pBuf, u32 data_len, u8 ep_num)
{
	TGPD* gpd;

	/* get gpd from tail */
	gpd = Rx_gpd_end[ep_num];

	TGPD_SET_DATA(gpd, pBuf);
	TGPD_CLR_FORMAT_BDP(gpd);

	TGPD_SET_DataBUF_LEN(gpd, data_len);
	TGPD_SET_BUF_LEN(gpd, 0);

//	TGPD_CLR_FORMAT_BPS(gpd);
	TGPD_SET_IOC(gpd);
	
	/* update gpd tail */
	Rx_gpd_end[ep_num] = get_gpd(RXQ ,ep_num);
	QMU_INFO("[RX]""Rx_gpd_end[%d]=%p gpd=%p\n", ep_num, Rx_gpd_end[ep_num], gpd);
	memset(Rx_gpd_end[ep_num], 0 , GPD_LEN_ALIGNED);
	TGPD_CLR_FLAGS_HWO(Rx_gpd_end[ep_num]);

	/* make sure struct ready before set to next*/
	mb();
	TGPD_SET_NEXT(gpd, gpd_virt_to_phys(Rx_gpd_end[ep_num], RXQ, ep_num));

	TGPD_SET_CHKSUM_HWO(gpd, 16);

	/* make sure struct ready before HWO */
	mb();
	TGPD_SET_FLAGS_HWO(gpd);
}
//#ifdef MUSB_QMU_SUPPORT_HOST
static void prepare_tx_gpd_ioc(u8 *pBuf, u32 data_len, u8 ep_num, u8 zlp,u8 isioc)
{
	TGPD* gpd;

	/* get gpd from tail */
	gpd = Tx_gpd_end[ep_num];

	TGPD_SET_DATA(gpd, pBuf);
	TGPD_CLR_FORMAT_BDP(gpd);

	TGPD_SET_BUF_LEN(gpd, data_len);
	TGPD_SET_EXT_LEN(gpd, 0);

	if (zlp)
		TGPD_SET_FORMAT_ZLP(gpd);
	else
		TGPD_CLR_FORMAT_ZLP(gpd);

	//TGPD_CLR_FORMAT_BPS(gpd);

	if(isioc){
	TGPD_SET_IOC(gpd);
	}
	

	/* update gpd tail */
	Tx_gpd_end[ep_num] = get_gpd(TXQ ,ep_num);
	QMU_INFO("[TX]""Tx_gpd_end[%d]=%p gpd=%p\n", ep_num, Tx_gpd_end[ep_num], gpd);
	memset(Tx_gpd_end[ep_num], 0 , GPD_LEN_ALIGNED);
	TGPD_CLR_FLAGS_HWO(Tx_gpd_end[ep_num]);


	/* make sure struct ready before set to next*/
	mb();
	TGPD_SET_NEXT(gpd, gpd_virt_to_phys(Tx_gpd_end[ep_num], TXQ, ep_num));

	TGPD_SET_CHKSUM_HWO(gpd, 16);

	/* make sure struct ready before HWO */
	mb();
	TGPD_SET_FLAGS_HWO(gpd);

}
//#endif

static void prepare_tx_gpd(u8 *pBuf, u32 data_len, u8 ep_num, u8 zlp)
{
	TGPD* gpd;

	/* get gpd from tail */
	gpd = Tx_gpd_end[ep_num];

	TGPD_SET_DATA(gpd, pBuf);
	TGPD_CLR_FORMAT_BDP(gpd);

	TGPD_SET_BUF_LEN(gpd, data_len);
	TGPD_SET_EXT_LEN(gpd, 0);

	if (zlp)
		TGPD_SET_FORMAT_ZLP(gpd);
	else
		TGPD_CLR_FORMAT_ZLP(gpd);

	//TGPD_CLR_FORMAT_BPS(gpd);

	TGPD_SET_IOC(gpd);

	/* update gpd tail */
	Tx_gpd_end[ep_num] = get_gpd(TXQ ,ep_num);
	QMU_INFO("[TX]""Tx_gpd_end[%d]=%p gpd=%p\n", ep_num, Tx_gpd_end[ep_num], gpd);
	memset(Tx_gpd_end[ep_num], 0 , GPD_LEN_ALIGNED);
	TGPD_CLR_FLAGS_HWO(Tx_gpd_end[ep_num]);


	/* make sure struct ready before set to next*/
	mb();
	TGPD_SET_NEXT(gpd, gpd_virt_to_phys(Tx_gpd_end[ep_num], TXQ, ep_num));

	TGPD_SET_CHKSUM_HWO(gpd, 16);

	/* make sure struct ready before HWO */
	mb();
	TGPD_SET_FLAGS_HWO(gpd);

}

void mtk_qmu_resume(u8 ep_num, u8 isRx)
{
	void __iomem* base = qmu_base;
	
	if (!isRx){
		MGC_WriteQMU32(base,  MGC_O_QMU_TQCSR(ep_num), DQMU_QUE_RESUME);
		if(!MGC_ReadQMU32(base, MGC_O_QMU_TQCSR(ep_num))){
			QMU_ERR("TQCSR[%d]=%x\n", ep_num, MGC_ReadQMU32(base, MGC_O_QMU_TQCSR(ep_num)));
			MGC_WriteQMU32(base,  MGC_O_QMU_TQCSR(ep_num), DQMU_QUE_RESUME);
			QMU_ERR("TQCSR[%d]=%x\n", ep_num, MGC_ReadQMU32(base, MGC_O_QMU_TQCSR(ep_num)));
		}
	}else{
		MGC_WriteQMU32(base,  MGC_O_QMU_RQCSR(ep_num), DQMU_QUE_RESUME);
		if(!MGC_ReadQMU32(base, MGC_O_QMU_RQCSR(ep_num))){
			QMU_ERR("RQCSR[%d]=%x\n", ep_num, MGC_ReadQMU32(base, MGC_O_QMU_RQCSR(ep_num)));
			MGC_WriteQMU32(base,  MGC_O_QMU_RQCSR(ep_num), DQMU_QUE_RESUME);
			QMU_ERR("RQCSR[%d]=%x\n", ep_num, MGC_ReadQMU32(base, MGC_O_QMU_RQCSR(ep_num)));
		}
	}	
	
}

bool mtk_is_qmu_enabled(u8 ep_num, u8 isRx)
{
	void __iomem* base = qmu_base;
	if(isRx){
		if(MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR)&(USB_QMU_Rx_EN(ep_num))){
			return true;
		}
	}
	else{
		if(MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR)&(USB_QMU_Tx_EN(ep_num))){
			return true;
		}
	}
	return false;
}

void mtk_qmu_enable(struct musb *musb, u8 ep_num, u8 isRx)
{
	struct musb_ep *musb_ep;
	u32 QCR;
	void __iomem* base = qmu_base;
	void __iomem        *mbase = musb->mregs;
    void __iomem		*epio;
    u16    csr = 0;
    u16 intr_e = 0;

    epio = musb->endpoints[ep_num].regs;
	musb_ep_select(mbase, ep_num);

	if (isRx){
		QMU_WARN("enable RQ(%d)\n", ep_num);

		/* enable dma */
		csr |= MUSB_RXCSR_DMAENAB;

		/* check ISOC */
		musb_ep = &musb->endpoints[ep_num].ep_out;
		if (musb_ep->type == USB_ENDPOINT_XFER_ISOC)
			csr |= MUSB_RXCSR_P_ISO;
		musb_writew(epio, MUSB_RXCSR, csr);

		/* turn off intrRx */
		intr_e = musb_readw(mbase, MUSB_INTRRXE);
		intr_e = intr_e & (~(1<<(ep_num)));
		musb_writew(mbase, MUSB_INTRRXE, intr_e);

		/* set 1st gpd and enable */
		MGC_WriteQMU32(base, MGC_O_QMU_RQSAR(ep_num), gpd_virt_to_phys(Rx_gpd_end[ep_num], RXQ, ep_num));
		MGC_WriteQUCS32(base, MGC_O_QUCS_USBGCSR,  MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR)|(USB_QMU_Rx_EN(ep_num)));

#ifdef CFG_CS_CHECK
		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR0);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR0, QCR | DQMU_RQCS_EN(ep_num));
#endif

#ifdef CFG_RX_ZLP_EN
		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR3);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR3, QCR | DQMU_RX_ZLP(ep_num));
#endif

#ifdef CFG_RX_COZ_EN
		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR3);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR3, QCR | DQMU_RX_COZ(ep_num));
#endif

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_QIMCR, DQMU_M_RX_DONE(ep_num)|DQMU_M_RQ_EMPTY|DQMU_M_RXQ_ERR|DQMU_M_RXEP_ERR);


#ifdef CFG_EMPTY_CHECK
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_REPEMPMCR, DQMU_M_RX_EMPTY(ep_num));
#else
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_QIMSR, DQMU_M_RQ_EMPTY);
#endif

		QCR = DQMU_M_RX_LEN_ERR(ep_num);
#ifdef CFG_CS_CHECK
		QCR |= DQMU_M_RX_GPDCS_ERR(ep_num);
#endif

#ifdef CFG_RX_ZLP_EN
		QCR |= DQMU_M_RX_ZLP_ERR(ep_num);
#endif
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_RQEIMCR, QCR);


		MGC_WriteQIRQ32(base, MGC_O_QIRQ_REPEIMCR, DQMU_M_RX_EP_ERR(ep_num));

		mb();
		/* qmu start */
		MGC_WriteQMU32(base, MGC_O_QMU_RQCSR(ep_num), DQMU_QUE_START);

	}else{
		QMU_WARN("enable TQ(%d)\n", ep_num);

		/* enable dma */
		csr |= MUSB_TXCSR_DMAENAB;

		/* check ISOC */
		musb_ep = &musb->endpoints[ep_num].ep_in;
		if (musb_ep->type==USB_ENDPOINT_XFER_ISOC)
			csr |= MUSB_TXCSR_P_ISO;
		musb_writew(epio, MUSB_TXCSR, csr);

		/* turn off intrTx */
		intr_e = musb_readw(mbase, MUSB_INTRTXE);
		intr_e = intr_e & (~(1<< ep_num));
		musb_writew(mbase, MUSB_INTRTXE, intr_e);

		/* set 1st gpd and enable */
		MGC_WriteQMU32(base, MGC_O_QMU_TQSAR(ep_num), gpd_virt_to_phys(Tx_gpd_end[ep_num], TXQ, ep_num));
		MGC_WriteQUCS32(base, MGC_O_QUCS_USBGCSR,  MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR)|(USB_QMU_Tx_EN(ep_num)));

#ifdef CFG_CS_CHECK
		QCR= MGC_ReadQMU32(base, MGC_O_QMU_QCR0);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR0, QCR|DQMU_TQCS_EN(ep_num));
#endif

#if (TXZLP==HW_MODE)
		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR2);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR2, QCR|DQMU_TX_ZLP(ep_num));
#elif (TXZLP==GPD_MODE)
		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR2);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR2, QCR|DQMU_TX_MULTIPLE(ep_num));
#endif

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_QIMCR, DQMU_M_TX_DONE(ep_num)|DQMU_M_TQ_EMPTY|DQMU_M_TXQ_ERR|DQMU_M_TXEP_ERR);

#ifdef CFG_EMPTY_CHECK
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TEPEMPMCR, DQMU_M_TX_EMPTY(ep_num));
#else
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_QIMSR, DQMU_M_TQ_EMPTY);
#endif

		QCR = DQMU_M_TX_LEN_ERR(ep_num);
#ifdef CFG_CS_CHECK
		QCR |= DQMU_M_TX_GPDCS_ERR(ep_num) | DQMU_M_TX_BDCS_ERR(ep_num);
#endif
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TQEIMCR, QCR);

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TEPEIMCR, DQMU_M_TX_EP_ERR(ep_num));

		mb();
		/* qmu start */
		MGC_WriteQMU32(base, MGC_O_QMU_TQCSR(ep_num), DQMU_QUE_START);
	}
}

void mtk_qmu_stop(u8 ep_num, u8 isRx)
{
    void __iomem* base = qmu_base;
	if(!isRx){
		if(MGC_ReadQMU16(base, MGC_O_QMU_TQCSR(ep_num)) & DQMU_QUE_ACTIVE){
			MGC_WriteQMU32(base,  MGC_O_QMU_TQCSR(ep_num), DQMU_QUE_STOP);
			QMU_WARN("Stop TQ %d\n", ep_num);
		}else{
			QMU_WARN("TQ %d already inactive\n", ep_num);
		}
	} else {
		if(MGC_ReadQMU16(base, MGC_O_QMU_RQCSR(ep_num)) & DQMU_QUE_ACTIVE){
			MGC_WriteQMU32(base,  MGC_O_QMU_RQCSR(ep_num), DQMU_QUE_STOP);
			QMU_WARN("Stop RQ %d\n", ep_num);
		}else{
			QMU_WARN("RQ %d already inactive\n", ep_num);
		}
	}
}

static void mtk_qmu_disable(u8 ep_num, u8 isRx)
{
	u32 QCR;
    void __iomem* base = qmu_base;

	QMU_WARN("disable %s(%d)\n", isRx?"RQ":"TQ", ep_num);

	mtk_qmu_stop(ep_num, isRx);
	if(isRx){
		/// clear Queue start address
		MGC_WriteQMU32(base, MGC_O_QMU_RQSAR(ep_num), 0);

		// KOBE, in denali, different EP QMU EN is separated in MGC_O_QUCS_USBGCSR ??
		MGC_WriteQUCS32(base, MGC_O_QUCS_USBGCSR,  MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR)&(~(USB_QMU_Rx_EN(ep_num))));

		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR0);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR0, QCR&(~(DQMU_RQCS_EN(ep_num))));
		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR3);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR3, QCR&(~(DQMU_RX_ZLP(ep_num))));

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_QIMSR, DQMU_M_RX_DONE(ep_num));
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_REPEMPMSR, DQMU_M_RX_EMPTY(ep_num));
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_RQEIMSR, DQMU_M_RX_LEN_ERR(ep_num)|DQMU_M_RX_GPDCS_ERR(ep_num)|DQMU_M_RX_ZLP_ERR(ep_num));
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_REPEIMSR, DQMU_M_RX_EP_ERR(ep_num));
	}else{
		/// clear Queue start address
		MGC_WriteQMU32(base, MGC_O_QMU_TQSAR(ep_num), 0);

		// KOBE, in denali, different EP QMU EN is separated in MGC_O_QUCS_USBGCSR ??
		MGC_WriteQUCS32(base, MGC_O_QUCS_USBGCSR,  MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR)&(~(USB_QMU_Tx_EN(ep_num))));

		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR0);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR0, QCR&(~(DQMU_TQCS_EN(ep_num))));
		QCR = MGC_ReadQMU32(base, MGC_O_QMU_QCR2);
		MGC_WriteQMU32(base, MGC_O_QMU_QCR2, QCR&(~(DQMU_TX_ZLP(ep_num))));

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_QIMSR, DQMU_M_TX_DONE(ep_num));
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TEPEMPMSR, DQMU_M_TX_EMPTY(ep_num));
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TQEIMSR, DQMU_M_TX_LEN_ERR(ep_num)|DQMU_M_TX_GPDCS_ERR(ep_num)|DQMU_M_TX_BDCS_ERR(ep_num));
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TEPEIMSR, DQMU_M_TX_EP_ERR(ep_num));
	}
}
//#ifdef MUSB_QMU_SUPPORT_HOST
void mtk_qmu_insert_task_ioc(u8 ep_num, u8 isRx, u8* buf, u32 length, u8 zlp,u8 isioc)
{
	QMU_INFO("mtk_qmu_insert_task_ioc ep_num: %d, isRx: %d, buf: %p, length: %d zlp: %d isioc: %d\n",
			ep_num, isRx, buf, length,zlp,isioc);
	if (isRx){
		/* rx don't care zlp input */		
		prepare_rx_gpd_ioc(buf, length, ep_num,isioc);		
	}
	else{
		prepare_tx_gpd_ioc(buf, length, ep_num,zlp,isioc);
	}
}
//#endif

void mtk_qmu_insert_task(u8 ep_num, u8 isRx, u8* buf, u32 length, u8 zlp)
{
	QMU_INFO("mtk_qmu_insert_task ep_num: %d, isRx: %d, buf: %p, length: %d\n",
			ep_num, isRx, buf, length);
	if (isRx){
		/* rx don't care zlp input */		
		prepare_rx_gpd(buf, length, ep_num);		
	}
	else{
		prepare_tx_gpd(buf, length, ep_num, zlp);
	}
}

void qmu_done_rx(struct musb *musb, u8 ep_num)
{
	void __iomem* base = qmu_base;

	TGPD* gpd = Rx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(unsigned long)MGC_ReadQMU32(base, MGC_O_QMU_RQCPR(ep_num));
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_out;
	struct usb_request	*request = NULL;
	struct musb_request	*req;
	uint32_t done = 1;

	//jingao:add 	
	//void __iomem		*epio = musb->endpoints[ep_num].regs;	
	//u16 		   csr;
	//csr = musb_readw(epio, MUSB_RXCSR);
	//jingao:add
#if 1
	/*Transfer PHY addr got from QMU register to VIR addr*/
	gpd_current = (TGPD*)gpd_phys_to_virt((dma_addr_t)gpd_current, RXQ, ep_num);	
	if (gpd == gpd_current) {		
		//printk("jingao:gpd-------------------------------\n");
		return;
	}
#endif
	//printk("jingao:in -->");
	//trying to give_back the request to gadget driver.
	req = next_request(musb_ep);
	if (!req) {
		QMU_ERR("[RXD]""%s Cannot get next request of %d, "
			"but QMU has done.\n", __func__, ep_num);
		return;
	} else {
		request = &req->request;
	}

	if(unlikely(!gpd || !gpd_current)) {
		QMU_ERR("[RXD][ERROR] EP%d, gpd=%p, gpd_current=%p, ishwo=%d, rx_gpd_last=%p, 	RQCPR=0x%x\n",
							ep_num, gpd, gpd_current,
							((gpd == NULL) ? 999 : TGPD_IS_FLAGS_HWO(gpd)),
							Rx_gpd_last[ep_num],
							MGC_ReadQMU32(base, MGC_O_QMU_RQCPR(ep_num)));
		return;
	}

	if(TGPD_IS_FLAGS_HWO(gpd)) {
		QMU_ERR("[RXD][ERROR]""HWO=1!!\n");
		QMU_ERR("[RXD][ERROR]""HWO=1!!\n");
		QMU_ERR("[RXD][ERROR]""HWO=1!!\n");
		QMU_ERR("[RXD][ERROR]""HWO=1!!\n");
		QMU_ERR("[RXD][ERROR]""HWO=1!!\n");
		//BUG_ON(1);
		return;
	}

	/* NORMAL EXEC FLOW */
	while(gpd != gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {
		u32 rcv_len = (u32)TGPD_GET_BUF_LEN(gpd);
		u32 buf_len  = (u32)TGPD_GET_DataBUF_LEN(gpd);

		if(rcv_len > buf_len){
			QMU_ERR("[RXD][ERROR] rcv(%d) > buf(%d) AUK!?\n", rcv_len, buf_len);
		}

		QMU_INFO("[RXD]""gpd=%p ->HWO=%d, Next_GPD=%p, RcvLen=%d, BufLen=%d, pBuf=%p\n",
				gpd, TGPD_GET_FLAG(gpd), TGPD_GET_NEXT(gpd), rcv_len, buf_len, TGPD_GET_DATA(gpd));

		request->actual += rcv_len;

		if(unlikely (!TGPD_GET_NEXT(gpd) || !TGPD_GET_DATA(gpd))) {
			QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
			//BUG_ON(1);
			break;
		}

		gpd = TGPD_GET_NEXT(gpd);

		gpd = gpd_phys_to_virt((dma_addr_t)gpd, RXQ, ep_num);

		if(!gpd) {
			QMU_ERR("[RXD][ERROR] !gpd, EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] !gpd, EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] !gpd, EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] !gpd, EP%d ,gpd=%p\n", ep_num, gpd);
			QMU_ERR("[RXD][ERROR] !gpd, EP%d ,gpd=%p\n", ep_num, gpd);
			//BUG_ON(1);
			break;
		}

		Rx_gpd_last[ep_num] = gpd;

#if 1 //jingao:add
		//printk("jingao request->number_of_packets %d \n",request->number_of_packets);
		if(request->number_of_packets >0){
			struct usb_iso_packet_descriptor *d;
			d = request->iso_frame_desc + request->iso_index;
			d->actual_length = rcv_len;			
			//req->request.actual += TGPD_GET_BUF_LEN(gpd);
			request->iso_index++;
			done = (request->iso_index == request->number_of_packets) ? true : false ;
		}else{
			done = true;
		}
#endif
		if(done){
#if 0			
			csr = musb_readw(epio, MUSB_RXCSR);
			if (csr & MUSB_RXCSR_P_ISO) {
				if (csr & MUSB_RXCSR_P_OVERRUN) {
					csr &= ~MUSB_RXCSR_P_OVERRUN;
					musb_writew(epio, MUSB_RXCSR, csr);			
					printk("%s iso overrun on %p\n", musb_ep->name, request);
					if (request )
						request->status = -EOVERFLOW;
				}
			}			
#endif			
			musb_g_giveback(musb_ep, request, 0);
		}
#if 1		
		req = next_request(musb_ep);
		if(!req){
			return;
		}else{  	
			request = &req->request;				
		}	
#endif		
	} //end while
	//printk("out while :request %p, request ->index = %d\n",request,request->iso_index);

	/* QMU should keep take HWO gpd , so there is error*/
	if(gpd != gpd_current && TGPD_IS_FLAGS_HWO(gpd)) {
		QMU_ERR("[RXD][ERROR]""gpd=%p\n", gpd);

		QMU_ERR("[RXD][ERROR]""EP%d RQCSR=%x, RQSAR=%x, RQCPR=%x, RQLDPR=%x\n",
				ep_num,
				MGC_ReadQMU32(base, MGC_O_QMU_RQCSR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_RQSAR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_RQCPR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_RQLDPR(ep_num)));

		QMU_ERR("[RXD][ERROR]""QCR0=%x, QCR2=%x, QCR3=%x, QGCSR=%x\n",
				MGC_ReadQMU32(base, MGC_O_QMU_QCR0),
				MGC_ReadQMU32(base, MGC_O_QMU_QCR2),
				MGC_ReadQMU32(base, MGC_O_QMU_QCR3),
				MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR));

		QMU_ERR("[RXD][ERROR]""HWO=%d, Next_GPD=%p ,DataBufLen=%d, "
			"DataBuf=%p, RecvLen=%d, Endpoint=%d\n",
			(u32)TGPD_GET_FLAG(gpd), TGPD_GET_NEXT(gpd),
			(u32)TGPD_GET_DataBUF_LEN(gpd), TGPD_GET_DATA(gpd),
			(u32)TGPD_GET_BUF_LEN(gpd), (u32)TGPD_GET_EPaddr(gpd));
	}

	QMU_INFO("[RXD]""%s EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Rx_gpd_last[ep_num], Rx_gpd_end[ep_num]);
}

void qmu_done_tx(struct musb *musb, u8 ep_num)
{
	void __iomem* base = qmu_base;
	TGPD* gpd = Tx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(unsigned long)MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num));
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_in;
	struct usb_request	*request = NULL;
	struct musb_request	*req = NULL;

	/*Transfer PHY addr got from QMU register to VIR addr*/
	gpd_current = gpd_phys_to_virt((dma_addr_t)gpd_current, TXQ, ep_num);

	/*
                      gpd or Last       gdp_current
                           |                  |
            |->  GPD1 --> GPD2 --> GPD3 --> GPD4 --> GPD5 -|
            |----------------------------------------------|
	*/

	QMU_INFO("[TXD]""%s EP%d, Last=%p, Current=%p, End=%p\n",
		__func__, ep_num, gpd, gpd_current, Tx_gpd_end[ep_num]);

	/*gpd_current should at least point to the next GPD to the previous last one.*/
	if (gpd == gpd_current) {
		QMU_ERR("[TXD] gpd(%p) == gpd_current(%p)\n",
				gpd, gpd_current);
		return;
	}

	if(TGPD_IS_FLAGS_HWO(gpd)) {
		QMU_ERR("[TXD] HWO=1, CPR=%x\n", MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));
		QMU_ERR("[TXD] HWO=1, CPR=%x\n", MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));
		QMU_ERR("[TXD] HWO=1, CPR=%x\n", MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));
		QMU_ERR("[TXD] HWO=1, CPR=%x\n", MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));
		QMU_ERR("[TXD] HWO=1, CPR=%x\n", MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));
		//BUG_ON(1);
		return;
	}

	/* NORMAL EXEC FLOW */
	while (gpd != gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {

		QMU_INFO("[TXD]""gpd=%p ->HWO=%d, BPD=%d, Next_GPD=%p, DataBuffer=%p, "
			"BufferLen=%d request=%p\n",
			gpd, (u32)TGPD_GET_FLAG(gpd), (u32)TGPD_GET_FORMAT(gpd),
			TGPD_GET_NEXT(gpd), TGPD_GET_DATA(gpd), (u32)TGPD_GET_BUF_LEN(gpd), req);

		if(!TGPD_GET_NEXT(gpd)) {
			QMU_ERR("[TXD][ERROR]""Next GPD is null!!\n");
			QMU_ERR("[TXD][ERROR]""Next GPD is null!!\n");
			QMU_ERR("[TXD][ERROR]""Next GPD is null!!\n");
			QMU_ERR("[TXD][ERROR]""Next GPD is null!!\n");
			QMU_ERR("[TXD][ERROR]""Next GPD is null!!\n");
			//BUG_ON(1);
			break;
		}

		gpd = TGPD_GET_NEXT(gpd);

		gpd = gpd_phys_to_virt((dma_addr_t)gpd, TXQ, ep_num);

		/* trying to give_back the request to gadget driver. */
		req = next_request(musb_ep);
		if (!req) {
			QMU_ERR("[TXD]""%s Cannot get next request of %d, "
				"but QMU has done.\n", __func__, ep_num);
			return;
		} else {
			request = &req->request;
		}

		Tx_gpd_last[ep_num] = gpd;
		musb_g_giveback(musb_ep, request, 0);
		req = next_request(musb_ep);
		if (req != NULL) {
			request = &req->request;
		}
	}

	if(gpd!=gpd_current && TGPD_IS_FLAGS_HWO(gpd)) {

		QMU_ERR("[TXD][ERROR]""EP%d TQCSR=%x, TQSAR=%x, TQCPR=%x\n",
				ep_num,
				MGC_ReadQMU32(base, MGC_O_QMU_TQCSR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_TQSAR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));

		QMU_ERR("[RXD][ERROR]""QCR0=%x, QCR2=%x, QCR3=%x, QGCSR=%x\n",
				MGC_ReadQMU32(base, MGC_O_QMU_QCR0),
				MGC_ReadQMU32(base, MGC_O_QMU_QCR2),
				MGC_ReadQMU32(base, MGC_O_QMU_QCR3),
				MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR));

		QMU_ERR("[TXD][ERROR]""HWO=%d, BPD=%d, Next_GPD=%p, DataBuffer=%p, "
							"BufferLen=%d, Endpoint=%d\n",
							(u32)TGPD_GET_FLAG(gpd), (u32)TGPD_GET_FORMAT(gpd),
							TGPD_GET_NEXT(gpd), TGPD_GET_DATA(gpd),
							(u32)TGPD_GET_BUF_LEN(gpd), (u32)TGPD_GET_EPaddr(gpd));
	}

	QMU_INFO("[TXD]""%s EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Tx_gpd_last[ep_num], Tx_gpd_end[ep_num]);

	req = next_request(musb_ep);
	if (!req) {
		return;
	}

	/* special case handle for zero request , only solve 1 zlp case*/
	if (req != NULL) {
		if (request->length == 0) {

			QMU_WARN("[TXD]""==Send ZLP== %p\n", req);
			musb_tx_zlp_qmu(musb, req->epnum);

			QMU_WARN("[TXD]""Giveback ZLP of EP%d, actual:%d, length:%d %p\n",
				req->epnum, request->actual, request->length, request);
			musb_g_giveback(musb_ep, request, 0);
			//jingao:add			
			req = next_request(musb_ep);
			//jingao:add
		}
	}
#if 1 //jingao:add
	//jingao:add
	if(req!=NULL){
		//musb_kick_D_CmdQ(musb, req);
	}else{
		//printk("jingao:no req!\n");
	}
	//jingao:add
#endif	//jingao:add
	
}

void flush_ep_csr(struct musb *musb, u8 ep_num, u8 isRx)
{
    void __iomem        *mbase = musb->mregs;
    struct musb_hw_ep    *hw_ep = musb->endpoints + ep_num;
    void __iomem        *epio = hw_ep->regs;
    u16 csr, wCsr;

    if (epio == NULL)
        QMU_ERR("epio == NULL\n");
    if (hw_ep == NULL)
        QMU_ERR("hw_ep == NULL\n");

    if (isRx)
    {
        csr = musb_readw(epio, MUSB_RXCSR);
        csr |= MUSB_RXCSR_FLUSHFIFO | MUSB_RXCSR_RXPKTRDY;
        if (musb->is_host)
            csr &= ~MUSB_RXCSR_H_REQPKT;

        /* write 2x to allow double buffering */
        //CC: see if some check is necessary
        musb_writew(epio, MUSB_RXCSR, csr);
        musb_writew(epio, MUSB_RXCSR, csr | MUSB_RXCSR_CLRDATATOG);
    }
    else
    {
        csr = musb_readw(epio, MUSB_TXCSR);
        if (csr&MUSB_TXCSR_TXPKTRDY)
        {
            wCsr = csr | MUSB_TXCSR_FLUSHFIFO | MUSB_TXCSR_TXPKTRDY;
            musb_writew(epio, MUSB_TXCSR, wCsr);
        }

        csr |= MUSB_TXCSR_FLUSHFIFO&~MUSB_TXCSR_TXPKTRDY;
        musb_writew(epio, MUSB_TXCSR, csr);
        musb_writew(epio, MUSB_TXCSR, csr | MUSB_TXCSR_CLRDATATOG);
        //CC: why is this special?
        musb_writew(mbase, MUSB_INTRTX, 1<<ep_num);
    }
}

void mtk_disable_q(struct musb *musb, u8 ep_num, u8 isRx){
    void __iomem        *mbase = musb->mregs;
    struct musb_hw_ep    *hw_ep = musb->endpoints + ep_num;
    void __iomem        *epio = hw_ep->regs;
    u16    csr;

    mtk_qmu_disable(ep_num, isRx);
	qmu_reset_gpd_pool(ep_num, isRx);

    musb_ep_select(mbase, ep_num);
    if(isRx){
        csr = musb_readw(epio, MUSB_RXCSR);
        csr &= ~MUSB_RXCSR_DMAENAB;
        musb_writew(epio, MUSB_RXCSR, csr);
        flush_ep_csr(musb, ep_num,  isRx);
    }else{
        csr = musb_readw(epio, MUSB_TXCSR);
        csr &= ~MUSB_TXCSR_DMAENAB;
        musb_writew(epio, MUSB_TXCSR, csr);
        flush_ep_csr(musb, ep_num,  isRx);
    }
}

void mtk_qmu_err_recover(struct musb *musb, u8 ep_num, u8 isRx, bool is_len_err)
{
	struct musb_ep  *musb_ep;
	struct musb_request *request;

	/* same action as musb_flush_qmu */
	mtk_qmu_stop(ep_num, isRx);
	qmu_reset_gpd_pool(ep_num, isRx);

	/* same action as musb_restart_qmu */
	flush_ep_csr(musb, ep_num, isRx);
	mtk_qmu_enable(musb, ep_num, isRx);

	if(isRx){
		musb_ep = &musb->endpoints[ep_num].ep_out;
	}else{
		musb_ep = &musb->endpoints[ep_num].ep_in;
	}

	/* requeue all req , basically the same as musb_kick_D_CmdQ */
	list_for_each_entry(request, &musb_ep->req_list, list) {
		QMU_ERR("request 0x%p length(0x%d) len_err(%d)\n", request, request->request.length, is_len_err);

		if(request->request.dma != DMA_ADDR_INVALID)
		{
			if(request->tx)
			{
				QMU_ERR("[TX] gpd=%p, epnum=%d, len=%d\n", Tx_gpd_end[ep_num], ep_num, request->request.length);
				request->request.actual = request->request.length;
				if(request->request.length > 0) {
					QMU_ERR("[TX]""Send non-ZLP cases\n");
					mtk_qmu_insert_task(request->epnum,
							isRx,
							(u8*)request->request.dma,
							request->request.length, ((request->request.zero==1)?1:0));

				} else if(request->request.length == 0) {
					/* this case may be a problem */
					QMU_ERR("[TX]""Send ZLP cases, may be a problem!!!\n");
					musb_tx_zlp_qmu(musb, request->epnum);
					musb_g_giveback(musb_ep, &(request->request), 0);
				}else{
					QMU_ERR("ERR, TX, request->request.length(%d)\n", request->request.length);
				}
			} else {
				QMU_ERR("[RX] gpd=%p, epnum=%d, len=%d\n",
						Rx_gpd_end[ep_num], ep_num, request->request.length);
				mtk_qmu_insert_task(request->epnum,
						isRx,
						(u8*)request->request.dma,
						request->request.length, ((request->request.zero==1)?1:0));
			}
		}
	}
   	QMU_ERR("RESUME QMU\n");
	/* RESUME QMU */
	mtk_qmu_resume(ep_num, isRx);
}

void mtk_qmu_irq_err(struct musb *musb, u32 qisar)
{
	u8 i;
	u32 wQmuVal;
	u32 wRetVal;
	void __iomem* base = qmu_base;
   	u8 err_ep_num = 0;
	bool is_len_err = false;
	u8 isRx;

	wQmuVal = qisar;

	//RXQ ERROR
	if (wQmuVal & DQMU_M_RXQ_ERR)
	{
		wRetVal = MGC_ReadQIRQ32(base, MGC_O_QIRQ_RQEIR) & (~(MGC_ReadQIRQ32(base, MGC_O_QIRQ_RQEIMR)));
		QMU_ERR("RQ error in QMU mode![0x%x]\n", wRetVal);

		isRx = RXQ;
		for (i = 1; i <= RXQ_NUM; i++)
		{
			if (wRetVal & DQMU_M_RX_GPDCS_ERR(i))
			{
				QMU_ERR("RQ %d GPD checksum error!\n", i);
				err_ep_num = i;
			}
			if (wRetVal & DQMU_M_RX_LEN_ERR(i))
			{
				QMU_ERR("RQ %d recieve length error!\n", i);				
				err_ep_num = i;
				is_len_err = true;
			}
			if (wRetVal & DQMU_M_RX_ZLP_ERR(i))
			{
				QMU_ERR("RQ %d recieve an zlp packet!\n", i);
			}
		}
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_RQEIR, wRetVal);
	}

	//TXQ ERROR
	if (wQmuVal & DQMU_M_TXQ_ERR)
	{
		isRx = TXQ;
		wRetVal = MGC_ReadQIRQ32(base, MGC_O_QIRQ_TQEIR) & (~(MGC_ReadQIRQ32(base, MGC_O_QIRQ_TQEIMR)));
		QMU_ERR("TQ error in QMU mode![0x%x]\n", wRetVal);

		for (i=1; i<=RXQ_NUM; i++)
		{
			if (wRetVal & DQMU_M_TX_BDCS_ERR(i))
			{
				QMU_ERR("TQ %d BD checksum error!\n", i);
				err_ep_num = i;
			}
			if (wRetVal & DQMU_M_TX_GPDCS_ERR(i))
			{
				QMU_ERR("TQ %d GPD checksum error!\n", i);
				err_ep_num = i;
			}
			if (wRetVal & DQMU_M_TX_LEN_ERR(i))
			{
				QMU_ERR("TQ %d buffer length error!\n", i);
				err_ep_num = i;
				is_len_err = true;
			}
		}
		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TQEIR, wRetVal);
	}

	//RX EP ERROR
	if (wQmuVal & DQMU_M_RXEP_ERR)
	{
		isRx = RXQ;
		wRetVal = MGC_ReadQIRQ32(base, MGC_O_QIRQ_REPEIR) & (~(MGC_ReadQIRQ32(base, MGC_O_QIRQ_REPEIMR)));
		QMU_ERR("Rx endpoint error in QMU mode![0x%x]\n", wRetVal);

		for (i=1; i<=RXQ_NUM; i++)
		{
			if (wRetVal & DQMU_M_RX_EP_ERR(i))
			{
				QMU_ERR("RX EP %d ERR\n", i);
				err_ep_num = i;
			}
		}

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_REPEIR, wRetVal);
	}

	//TX EP ERROR
	if(wQmuVal & DQMU_M_TXEP_ERR)
	{
		isRx = TXQ;
		wRetVal = MGC_ReadQIRQ32(base, MGC_O_QIRQ_TEPEIR)& (~(MGC_ReadQIRQ32(base, MGC_O_QIRQ_TEPEIMR)));
		QMU_ERR("Tx endpoint error in QMU mode![0x%x]\n", wRetVal);

		for (i=1; i<=TXQ_NUM; i++){
			if (wRetVal & DQMU_M_TX_EP_ERR(i))
			{
				QMU_ERR("TX EP %d ERR\n", i);
				err_ep_num = i;
			}
		}

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TEPEIR, wRetVal);
	}

	//RXQ EMPTY
	if (wQmuVal & DQMU_M_RQ_EMPTY)
	{
		wRetVal = MGC_ReadQIRQ32(base, MGC_O_QIRQ_REPEMPR)
			& (~(MGC_ReadQIRQ32(base, MGC_O_QIRQ_REPEMPMR)));
		QMU_ERR("RQ Empty in QMU mode![0x%x]\n", wRetVal);

		for (i=1; i<=RXQ_NUM; i++)
		{
			if (wRetVal & DQMU_M_RX_EMPTY(i))
			{
				QMU_ERR("RQ %d Empty!\n", i);
			}
		}

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_REPEMPR, wRetVal);
	}

	//TXQ EMPTY
	if (wQmuVal & DQMU_M_TQ_EMPTY)
	{
		wRetVal = MGC_ReadQIRQ32(base, MGC_O_QIRQ_TEPEMPR)
			& (~(MGC_ReadQIRQ32(base, MGC_O_QIRQ_TEPEMPMR)));
		QMU_ERR("TQ Empty in QMU mode![0x%x]\n", wRetVal);

		for (i=1; i<=TXQ_NUM; i++)
		{
			if (wRetVal & DQMU_M_TX_EMPTY(i))
			{
				QMU_ERR("TQ %d Empty!\n", i);
			}
		}

		MGC_WriteQIRQ32(base, MGC_O_QIRQ_TEPEMPR, wRetVal);
	}

	/* QMU ERR RECOVER , only servie one ep error ?*/
	if(err_ep_num){
		mtk_qmu_err_recover(musb, err_ep_num, isRx, is_len_err);
	}
}

#ifdef MUSB_QMU_SUPPORT_HOST
int mtk_kick_CmdQ(struct musb *musb, int isRx, struct musb_qh *qh)
         {
             void __iomem        *mbase = musb->mregs;
             u16    csr = 0;
             bool isHost = musb->is_host;
             u16 intr_e = 0;
             struct urb		*urb = next_urb(qh);
             struct musb_hw_ep	*hw_ep = qh->hw_ep;
             void __iomem		*epio = hw_ep->regs;
             unsigned int offset;
             u8 bIsIoc;
             u8 *pBuffer;
             u32 dwLength;
             u16 i;

	      if(!urb) {
                  DBG(4, "NO urb!!\n");
		  return -1; // KOBE : should we return a value 
              }

              DBG(4, "\n");
             musb_ep_set_qh(hw_ep, isRx, qh);
			 DBG(4, "\n");

             if(!mtk_is_qmu_enabled(hw_ep->epnum,isRx))
             {
                  DBG(4, "! mtk_is_qmu_enabled\n");
                 // musb_ep_set_qh(hw_ep, isRx, qh);
                 musb_ep_select(mbase, hw_ep->epnum);
				 DBG(4, "\n");
                 flush_ep_csr(musb, hw_ep->epnum,  isRx);
				 DBG(4, "\n");

                 if (isRx)
                 {
                      DBG(4, "isRX = 1\n");

                     if (qh->type == USB_ENDPOINT_XFER_ISOC)
                     {
                          DBG(4, "USB_ENDPOINT_XFER_ISOC\n");
                         if(qh->hb_mult== 3)
                             musb_writew(epio, MUSB_RXMAXP, qh->maxpacket|0x1000);
                         else if(qh->hb_mult == 2)
                             musb_writew(epio, MUSB_RXMAXP, qh->maxpacket|0x800);
                         else
                             musb_writew(epio, MUSB_RXMAXP, qh->maxpacket);
                     }
                     else {
                          DBG(4, "!! USB_ENDPOINT_XFER_ISOC\n");
                         musb_writew(epio, MUSB_RXMAXP, qh->maxpacket);
                         }


                     if (isHost)
                     {
                          DBG(4, "isHOST\n");
                         musb_writew(epio, MUSB_RXCSR, MUSB_RXCSR_DMAENAB);
                         //CC: speed?
                         musb_writeb(epio, MUSB_RXTYPE, qh->type_reg);
                         musb_writeb(epio, MUSB_RXINTERVAL, qh->intv_reg);
                     }
                     else
                     {
                          DBG(4, "!! isHOST\n");
                         csr |= MUSB_RXCSR_DMAENAB;
                         if (qh->type == USB_ENDPOINT_XFER_ISOC)
                         {
                              DBG(4, "USB_ENDPOINT_XFER_ISOC\n");
                             csr |= MUSB_RXCSR_P_ISO;
                         }
                         else
                         {
                              DBG(4, "!! USB_ENDPOINT_XFER_ISOC\n");
                             csr &= ~MUSB_RXCSR_P_ISO;
                             csr &= ~MUSB_RXCSR_DISNYET;
                         }

                         musb_writew(epio, MUSB_RXCSR, csr);
                     }
#ifdef CONFIG_USB_MTK_HDRC
                     if (musb->is_multipoint) {
                              DBG(4, "is_multipoint\n");
                         musb_write_rxfunaddr(musb->mregs, hw_ep->epnum, qh->addr_reg);
                         musb_write_rxhubaddr(musb->mregs, hw_ep->epnum, qh->h_addr_reg);
                         musb_write_rxhubport(musb->mregs, hw_ep->epnum, qh->h_port_reg);

                     } else{
                              DBG(4, "!! is_multipoint\n");
                         musb_writeb(musb->mregs, MUSB_FADDR, qh->addr_reg);
                         }
#endif
					 DBG(4, "\n");

                     //turn off intrRx
                     intr_e = musb_readw(musb->mregs, MUSB_INTRRXE);
                     intr_e = intr_e & (~(1<<(hw_ep->epnum)));
                     musb_writew(musb->mregs, MUSB_INTRRXE, intr_e);
                 }
                 else
                 {
                  DBG(4, "isTX = 1\n");
                     musb_writew(epio, MUSB_TXMAXP, qh->maxpacket);

                     if (isHost)
                     {
                          DBG(4, "isHOST\n");
                         musb_writew(epio, MUSB_TXCSR, MUSB_TXCSR_DMAENAB);
                         //CC: speed?
                         musb_writeb(epio, MUSB_TXTYPE, qh->type_reg);
                         musb_writeb(epio, MUSB_TXINTERVAL, qh->intv_reg);
                     }
                     else
                     {
                          DBG(4, "!! isHOST\n");
                         csr |= MUSB_TXCSR_DMAENAB;
                         if (qh->type==USB_ENDPOINT_XFER_ISOC) {
                              DBG(4, "USB_ENDPOINT_XFER_ISOC\n");
                             csr |= MUSB_TXCSR_P_ISO;
                             }
                         else {
                              DBG(4, "!! USB_ENDPOINT_XFER_ISOC\n");
                             csr &= ~MUSB_TXCSR_P_ISO;
                             }

                         musb_writew(epio, MUSB_TXCSR, csr);
                     }
#ifdef CONFIG_USB_MTK_HDRC
                     if (musb->is_multipoint) {
                              DBG(4, "is_multipoint\n");
                         musb_write_txfunaddr(mbase, hw_ep->epnum, qh->addr_reg);
                         musb_write_txhubaddr(mbase, hw_ep->epnum, qh->h_addr_reg);
                         musb_write_txhubport(mbase, hw_ep->epnum, qh->h_port_reg);
                         /* FIXME if !epnum, do the same for RX ... */
                     } else {
                              DBG(4, "!! is_multipoint\n");
                         musb_writeb(mbase, MUSB_FADDR, qh->addr_reg);//set the address of the device,very important!!
                         }
#endif
                     //turn off intrTx
                     intr_e = musb_readw(musb->mregs, MUSB_INTRTXE);
                     intr_e = intr_e & (~(1<<hw_ep->epnum));
                     musb_writew(musb->mregs, MUSB_INTRTXE, intr_e);
                 }
                 //mtk_enable_q(musb, hw_ep->epnum, isRx, 0, 1, 0);
                              DBG(4, "mtk_qmu_enable\n");
                 mtk_qmu_enable(musb, hw_ep->epnum, isRx); //JEREMY
             }

             if (qh->type == USB_ENDPOINT_XFER_ISOC)
             {
                              DBG(4, "USB_ENDPOINT_XFER_ISOC\n");
                 pBuffer = (uint8_t *)urb->transfer_dma;

                 for(i=0; i<urb->number_of_packets; i++)
                 {
                     offset = urb->iso_frame_desc[i].offset;
                     dwLength = urb->iso_frame_desc[i].length;
                     /* If interrupt on complete ? */
                     bIsIoc = (i == (urb->number_of_packets-1)) ? TRUE : FALSE;
#if 0
                     if (mtk_qmu_insert_task(hw_ep->epnum, isRx, pBuffer+offset, dwLength, bIsIoc) < 0)
                     {
                         INFO("[USB] Insert Task Error !\n");
                         return; // KOBE should we return the error code ? 
                     }
#else
                              DBG(4, "mtk_qmu_insert_task\n");
					if(isRx){
                    	mtk_qmu_insert_task_ioc(hw_ep->epnum, isRx, pBuffer+offset, dwLength, 0,bIsIoc);
						mtk_qmu_resume(hw_ep->epnum, isRx);
					}else{
						mtk_qmu_insert_task_ioc(hw_ep->epnum, isRx, pBuffer+offset, dwLength,1,1);
						mtk_qmu_resume(hw_ep->epnum, isRx);
					}
#endif
                 }
             }
             else
             {
#ifndef MUSB_QMU__HOST_BULK_RX_AUTO_SPLIT             

                 /* Must be the bulk transfer type */
                 pBuffer = (uint8_t *)urb->transfer_dma;

                 /*
                 Note current GPD only support 16 bits transferred data length.
                 Currently no software workaround this problem.
                 */        
                 if (urb->transfer_buffer_length >= 65536) {
	                  DBG(4, "[USB] Insert Task LEN Error !\n");

                 }
                 dwLength = urb->transfer_buffer_length;
                 bIsIoc = 1;

#else
				/* Must be the bulk transfer type */
				pBuffer = (uint8_t *)urb->transfer_dma;
				//reuse isoc urb->number_of_packets
				//if((urb->transfer_buffer_length < QMU_RX_SPLIT_THRE) | usb_pipeout(urb->pipe)){
				if(urb->transfer_buffer_length < QMU_RX_SPLIT_THRE){
				 	 DBG(4,"urb->transfer_buffer_length : %d\n",urb->transfer_buffer_length);
					 dwLength = urb->transfer_buffer_length;
					 bIsIoc = 1; 
					 if(isRx){
                    	mtk_qmu_insert_task_ioc(hw_ep->epnum, isRx, pBuffer+offset, dwLength, 0,bIsIoc);
						mtk_qmu_resume(hw_ep->epnum, isRx);
					}else{
						mtk_qmu_insert_task_ioc(hw_ep->epnum, isRx, pBuffer+offset, dwLength,0,bIsIoc);
						mtk_qmu_resume(hw_ep->epnum, isRx);
					}
					 //mtk_qmu_insert_task(hw_ep->epnum, isRx, pBuffer, dwLength, bIsIoc);
					 //mtk_qmu_resume(hw_ep->epnum, isRx);
				 }else{
				    //reuse isoc urb->unmber_of_packets
				    urb->number_of_packets = ((urb->transfer_buffer_length) +QMU_RX_SPLIT_BLOCK_SIZE-1)/(QMU_RX_SPLIT_BLOCK_SIZE);
					for(i=0; i<urb->number_of_packets; i++)
	                 {
	                     offset = QMU_RX_SPLIT_BLOCK_SIZE*i;
	                     dwLength = QMU_RX_SPLIT_BLOCK_SIZE;
	                     /* If interrupt on complete ? */
	                     bIsIoc = (i == (urb->number_of_packets-1)) ? TRUE : FALSE;
						 dwLength = (i == (urb->number_of_packets-1)) ? ((urb->transfer_buffer_length) %QMU_RX_SPLIT_BLOCK_SIZE) : dwLength;
						 if(dwLength==0) 
						 	  dwLength = QMU_RX_SPLIT_BLOCK_SIZE;
	                     //mtk_qmu_insert_task(hw_ep->epnum, isRx, pBuffer, dwLength, bIsIoc);
	                     if(isRx){
                    	mtk_qmu_insert_task_ioc(hw_ep->epnum, isRx, pBuffer+offset, dwLength, 0,bIsIoc);
						mtk_qmu_resume(hw_ep->epnum, isRx);
						}else{
						mtk_qmu_insert_task_ioc(hw_ep->epnum, isRx, pBuffer+offset, dwLength,0,bIsIoc);
						mtk_qmu_resume(hw_ep->epnum, isRx);
						}
						 //mtk_qmu_resume(hw_ep->epnum, isRx);
	                 }						 
				 }
#endif
             }

             DBG(4, "\n");
             return 0;
}

void mtk_q_advance_schedule(struct musb *musb, struct urb *urb, struct musb_hw_ep *hw_ep, int is_in)
{
#if 0
    struct musb_qh        *qh = musb_ep_get_qh(hw_ep, is_in);
    struct musb_hw_ep    *ep = qh->hw_ep;
    int            ready = qh->is_ready;
    int            status = 0;
#endif
	struct musb_qh        *qh = NULL;
    struct musb_hw_ep    *ep = NULL;
    int            ready = 0;
    int            status = 0;
	DBG(4, "hw_ep = %p\n",hw_ep);
	qh = musb_ep_get_qh(hw_ep, is_in);
	if(NULL == qh){
		DBG(4, "(NULL == qh) \n");
		return;
	}
	DBG(4, "qh = %p qh->ready = %d\n",qh,qh->is_ready);
	ep = qh->hw_ep;
	if(NULL == ep){
		DBG(4, "(NULL == ep) \n");
		return;
	}
	DBG(4, "ep = %p\n",ep);
	ready = qh->is_ready;
	DBG(4, "\n");
//#ifdef CONFIG_MT7118_HOST
#if 1
    struct dma_controller *dma_controller;
    struct dma_channel *dma_channel;
#endif
    struct urb			*nexturb;

    status = (urb->status == -EINPROGRESS) ? 0 : urb->status;
	DBG(4, "\n");

    /* save toggle eagerly, for paranoia */
    switch (qh->type) {
        //CC: do we need to save toggle for Q handled bulk transfer?
    case USB_ENDPOINT_XFER_BULK:
		DBG(0,"\n");
        musb_save_toggle(qh, is_in, urb);
		DBG(0,"\n");
        break;
    case USB_ENDPOINT_XFER_ISOC:
        if (status == 0 && urb->error_count) {
			DBG(0,"\n");
            status = -EXDEV;
        	}
        break;
    }

//#ifdef CONFIG_MT7118_HOST
#if 1
    /* candidate for DMA? */
    dma_controller = musb->dma_controller;
    dma_channel = is_in ? ep->rx_channel : ep->tx_channel;
    if(dma_channel)
    {
        dma_controller->channel_release(dma_channel);
        dma_channel=NULL;
        if(is_in)
            ep->rx_channel=NULL;
        else
            ep->tx_channel=NULL;
    }
	DBG(4, "\n");
#endif
	DBG(0,"\n");

    qh->is_ready = 0;
    musb_giveback(musb, urb, status);
    qh->is_ready = ready;
	DBG(0,"\n");

    /* reclaim resources (and bandwidth) ASAP; deschedule it, and
     * invalidate qh as soon as list_empty(&hep->urb_list)
     */
#if 1 
    if (list_empty(&qh->hep->urb_list)) {
        struct list_head	*head;
        if (is_in)
            ep->rx_reinit = 1;
        else
            ep->tx_reinit = 1;

        /* Clobber old pointers to this qh */
		DBG(0,"\n");
        musb_ep_set_qh(ep, is_in, NULL);
        qh->hep->hcpriv = NULL;
		DBG(0,"\n");

        switch (qh->type) {
        case USB_ENDPOINT_XFER_BULK:
            /* fifo policy for these lists, except that NAKing
             * should rotate a qh to the end (for fairness).
             */
            //   kfree(qh);
            //  qh = NULL;
            //We don't use this list, just free qh
            DBG(0,"\n");
            if (qh->mux == 1) {
				DBG(0,"\n");
                head = qh->ring.prev;
                list_del(&qh->ring);
                kfree(qh);
                qh = first_qh(head);
				DBG(0,"\n");
                break;
            }

            break;
        case USB_ENDPOINT_XFER_ISOC:
			DBG(0,"\n");
            kfree(qh);
            qh = NULL;
			DBG(0,"\n");
            break;
        }
    }
#endif
    //if (qh != NULL && qh->is_ready && next_urb(qh)!= NULL) {
    
    if (qh != NULL && qh->is_ready) {
	DBG(0,"\n");
        mtk_kick_CmdQ(musb, is_in, qh);
	DBG(0,"\n");
    }
}

void h_qmu_done_rx(struct musb *musb, u8 ep_num)
{
	void __iomem* base = qmu_base;

	TGPD* gpd = Rx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(unsigned long)MGC_ReadQMU32(base, MGC_O_QMU_RQCPR(ep_num));
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_out;
	struct usb_request	*request = NULL;
	struct musb_request	*req;
	struct musb_hw_ep	 *hw_ep = musb->endpoints + ep_num;				
	struct musb_qh 	   *qh = hw_ep->in_qh;
	struct urb		  *urb=NULL;

	bool done = true;
	//trying to give_back the request to gadget driver.
	urb = next_urb(qh);
        if (unlikely(!urb)) {
            printk(KERN_ALERT "BOGUS RX%d ready,qh%p\n", ep_num,hw_ep->in_qh);            
	    mtk_qmu_stop(ep_num, USB_DIR_IN);			
            return;
        }
	DBG(4, "\n");

	
	/*Transfer PHY addr got from QMU register to VIR addr*/
	gpd_current = (TGPD*)gpd_phys_to_virt((dma_addr_t)gpd_current, RXQ, ep_num);
	DBG(4, "\n");

	QMU_INFO("[RXD]""%s EP%d, Last=%p, Current=%p, End=%p\n",
		__func__, ep_num, gpd, gpd_current, Rx_gpd_end[ep_num]);

	/* gpd_current should at least point to the next GPD to the previous last one */
	if (gpd == gpd_current) {

		QMU_ERR("[RXD][ERROR] gpd(%p) == gpd_current(%p)\n",
				gpd, gpd_current);

		QMU_ERR("[RXD][ERROR]""EP%d RQCSR=%x, RQSAR=%x, RQCPR=%x, RQLDPR=%x\n",
				ep_num, 
				MGC_ReadQMU32(base, MGC_O_QMU_RQCSR(ep_num)), 
				MGC_ReadQMU32(base, MGC_O_QMU_RQSAR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_RQCPR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_RQLDPR(ep_num))); 

		QMU_ERR("[RXD][ERROR]""QCR0=%x, QCR2=%x, QCR3=%x, QGCSR=%x\n", 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR0), 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR2), 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR3), 
				MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR));

		QMU_ERR("[RXD][ERROR]""HWO=%d, Next_GPD=%p ,DataBufLen=%d, "
				"DataBuf=%p, RecvLen=%d, Endpoint=%d\n",
				(u32)TGPD_GET_FLAG(gpd), TGPD_GET_NEXT(gpd),
				(u32)TGPD_GET_DataBUF_LEN(gpd), TGPD_GET_DATA(gpd),
				(u32)TGPD_GET_BUF_LEN(gpd), (u32)TGPD_GET_EPaddr(gpd));

		return;
	}

	if(!gpd || !gpd_current) {

		QMU_ERR("[RXD][ERROR] EP%d, gpd=%p, gpd_current=%p, ishwo=%d, rx_gpd_last=%p, 	RQCPR=0x%x\n",
							ep_num, gpd, gpd_current, 
							((gpd == NULL) ? 999 : TGPD_IS_FLAGS_HWO(gpd)),
							Rx_gpd_last[ep_num], 
							MGC_ReadQMU32(base, MGC_O_QMU_RQCPR(ep_num)));
		return;
	}

	if(TGPD_IS_FLAGS_HWO(gpd)) {
		QMU_ERR("[RXD][ERROR]""HWO=1!!\n");
		BUG_ON(1);
	}

	/* NORMAL EXEC FLOW */
	while(gpd != gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {
		u32 rcv_len = (u32)TGPD_GET_BUF_LEN(gpd);
		u32 buf_len  = (u32)TGPD_GET_DataBUF_LEN(gpd);

		if(rcv_len > buf_len){
			QMU_ERR("[RXD][ERROR] rcv(%d) > buf(%d) AUK!?\n", rcv_len, buf_len);
		}
#if 0

		if (!TGPD_GET_NEXT(gpd) || !TGPD_GET_DATA(gpd)) {
			QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
			BUG_ON(1);
		}

		gpd = TGPD_GET_NEXT(gpd);
		//DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		gpd = gpd_phys_to_virt((dma_addr_t)gpd, RXQ, ep_num);
		DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		if(!gpd) {
			QMU_ERR("[RXD][ERROR]""%s EP%d ,gpd=%p\n", __func__, ep_num, gpd);
			BUG_ON(1);
		}
		DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		Rx_gpd_last[ep_num] = gpd;
		DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		DBG(4, "hw_ep = %p\n",hw_ep);

		if(usb_pipebulk(urb->pipe) && urb->transfer_buffer_length >= QMU_RX_SPLIT_THRE && usb_pipein(urb->pipe)){
			urb->actual_length += TGPD_GET_BUF_LEN(gpd);
		    qh->offset += TGPD_GET_BUF_LEN(gpd);
			qh->iso_idx++;
			done = (qh->iso_idx == urb->number_of_packets) ? true : false ;
		}else{
			urb->actual_length = TGPD_GET_BUF_LEN(gpd);
			qh->offset = TGPD_GET_BUF_LEN(gpd);
			done = true;
		}
		if(done){
			mtk_q_advance_schedule(musb, urb, hw_ep, USB_DIR_IN);			
			urb = next_urb(qh);
			qh->iso_idx = 0;
		}
#else

		if (!TGPD_GET_NEXT(gpd) || !TGPD_GET_DATA(gpd)) {
			QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
			BUG_ON(1);
		}


		if(usb_pipebulk(urb->pipe) && urb->transfer_buffer_length >= QMU_RX_SPLIT_THRE && usb_pipein(urb->pipe)){
			urb->actual_length += TGPD_GET_BUF_LEN(gpd);
			qh->offset += TGPD_GET_BUF_LEN(gpd);
			qh->iso_idx++;
			done = (qh->iso_idx == urb->number_of_packets) ? true : false ;
		}else{
			urb->actual_length = TGPD_GET_BUF_LEN(gpd);
			qh->offset = TGPD_GET_BUF_LEN(gpd);
			done = true;
		}
		if(done){
			mtk_q_advance_schedule(musb, urb, hw_ep, USB_DIR_IN);			
			urb = next_urb(qh);
			qh->iso_idx = 0;
		}
		
		gpd = TGPD_GET_NEXT(gpd);
		//DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		gpd = gpd_phys_to_virt((dma_addr_t)gpd, RXQ, ep_num);
		DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		if(!gpd) {
			QMU_ERR("[RXD][ERROR]""%s EP%d ,gpd=%p\n", __func__, ep_num, gpd);
			BUG_ON(1);
		}
		DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		Rx_gpd_last[ep_num] = gpd;
		DBG(4, "gpd = %p ep_num = %d\n",gpd,ep_num);
		DBG(4, "hw_ep = %p\n",hw_ep);



#endif
		DBG(4, "\n");
	}

	/* QMU should keep take HWO gpd , so there is error*/
	if(gpd != gpd_current && TGPD_IS_FLAGS_HWO(gpd)) {
		QMU_ERR("[RXD][ERROR]""gpd=%p\n", gpd);

		QMU_ERR("[RXD][ERROR]""EP%d RQCSR=%x, RQSAR=%x, RQCPR=%x, RQLDPR=%x\n",
				ep_num, 
				MGC_ReadQMU32(base, MGC_O_QMU_RQCSR(ep_num)), 
				MGC_ReadQMU32(base, MGC_O_QMU_RQSAR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_RQCPR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_RQLDPR(ep_num))); 

		QMU_ERR("[RXD][ERROR]""QCR0=%x, QCR2=%x, QCR3=%x, QGCSR=%x\n", 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR0), 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR2), 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR3), 
				MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR));

		QMU_ERR("[RXD][ERROR]""HWO=%d, Next_GPD=%p ,DataBufLen=%d, "
			"DataBuf=%p, RecvLen=%d, Endpoint=%d\n",
			(u32)TGPD_GET_FLAG(gpd), TGPD_GET_NEXT(gpd),
			(u32)TGPD_GET_DataBUF_LEN(gpd), TGPD_GET_DATA(gpd),
			(u32)TGPD_GET_BUF_LEN(gpd), (u32)TGPD_GET_EPaddr(gpd));
	}

	QMU_INFO("[RXD]""%s EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Rx_gpd_last[ep_num], Rx_gpd_end[ep_num]);
	DBG(4, "\n");
}

void h_qmu_done_tx(struct musb *musb, u8 ep_num)
{
	void __iomem* base = qmu_base;
	TGPD* gpd = Tx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(unsigned long)MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num));
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_in;
	struct musb_hw_ep    *hw_ep = musb->endpoints + ep_num;         
        struct musb_qh        *qh = hw_ep->out_qh;
	struct urb    *urb=NULL;	
	bool done = true;

	urb = next_urb(qh);			
	DBG(0,"\n");
	if (!urb) {
		DBG(4, "extra TX%d ready\n", ep_num);
		 mtk_qmu_stop(ep_num, USB_DIR_OUT);
		return;
	}

	/*Transfer PHY addr got from QMU register to VIR addr*/
	gpd_current = gpd_phys_to_virt((dma_addr_t)gpd_current, TXQ, ep_num);

	/*
                      gpd or Last       gdp_current
                           |                  |
            |->  GPD1 --> GPD2 --> GPD3 --> GPD4 --> GPD5 -|
            |----------------------------------------------|
	*/

	QMU_INFO("[TXD]""%s EP%d, Last=%p, Current=%p, End=%p\n",
		__func__, ep_num, gpd, gpd_current, Tx_gpd_end[ep_num]);

	/*gpd_current should at least point to the next GPD to the previous last one.*/
	if (gpd == gpd_current) {
		QMU_ERR("[TXD] gpd(%p) == gpd_current(%p)\n", 
				gpd, gpd_current);
		return;
	}

	if(TGPD_IS_FLAGS_HWO(gpd)) {
		QMU_ERR("[TXD] HWO=1, CPR=%x\n", MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));
		BUG_ON(1);
	}

	/* NORMAL EXEC FLOW */
	while (gpd != gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {

		//QMU_INFO("[TXD]""gpd=%p ->HWO=%d, BPD=%d, Next_GPD=%p, DataBuffer=%p, "
		//	"BufferLen=%d urb=%p\n", 
		//	gpd, (u32)TGPD_GET_FLAG(gpd), (u32)TGPD_GET_FORMAT(gpd),
		//	TGPD_GET_NEXT(gpd), TGPD_GET_DATA(gpd), (u32)TGPD_GET_BUF_LEN(gpd), urb);
		
		QMU_INFO("[TXD]""gpd=%p ->HWO=%d, BPD=%d, Next_GPD=%p, DataBuffer=%p, "
			"BufferLen=%d \n", 
			gpd, (u32)TGPD_GET_FLAG(gpd), (u32)TGPD_GET_FORMAT(gpd),
			TGPD_GET_NEXT(gpd), TGPD_GET_DATA(gpd), (u32)TGPD_GET_BUF_LEN(gpd));

		if(!TGPD_GET_NEXT(gpd)) {
			QMU_ERR("[TXD][ERROR]""Next GPD is null!!\n");
			//BUG_ON(1);
			break;
		}

		#if 0
		urb = next_urb(qh);			
		DBG(0,"\n");
		if (!urb) {
			DBG(4, "extra TX%d ready\n", ep_num);
			 mtk_qmu_stop(ep_num, USB_DIR_OUT);
			return;
		}
		#endif
		
		//urb->actual_length = TGPD_GET_BUF_LEN(gpd);
		//urb->status = 0;

				if (!TGPD_GET_NEXT(gpd) || !TGPD_GET_DATA(gpd)) {
					QMU_ERR("[RXD][ERROR] EP%d ,gpd=%p\n", ep_num, gpd);
					BUG_ON(1);
				}
		
		
				if(usb_pipebulk(urb->pipe) && urb->transfer_buffer_length >= QMU_RX_SPLIT_THRE && usb_pipeout(urb->pipe)){
					urb->actual_length += TGPD_GET_BUF_LEN(gpd);
					qh->offset += TGPD_GET_BUF_LEN(gpd);
					qh->iso_idx++;
					done = (qh->iso_idx == urb->number_of_packets) ? true : false ;
				}else{
					urb->actual_length = TGPD_GET_BUF_LEN(gpd);
					qh->offset = TGPD_GET_BUF_LEN(gpd);
					done = true;
				}
				if(done){
					mtk_q_advance_schedule(musb, urb, hw_ep, USB_DIR_OUT);			
					urb = next_urb(qh);
					qh->iso_idx = 0;
				}


		gpd = TGPD_GET_NEXT(gpd);
		gpd = gpd_phys_to_virt((dma_addr_t)gpd, TXQ, ep_num);
		
		Tx_gpd_last[ep_num] = gpd;
		DBG(0, "GET tx USB %p\n", urb);		
		DBG(0, "urb->actual_length set");					
		
		
		DBG(0,"\n");
		//mtk_q_advance_schedule(musb, urb, hw_ep, USB_DIR_OUT);
		DBG(0,"\n");	
	}

	if(gpd!=gpd_current && TGPD_IS_FLAGS_HWO(gpd)) {

		QMU_ERR("[TXD][ERROR]""EP%d TQCSR=%x, TQSAR=%x, TQCPR=%x\n", 
				ep_num, 
				MGC_ReadQMU32(base, MGC_O_QMU_TQCSR(ep_num)), 
				MGC_ReadQMU32(base, MGC_O_QMU_TQSAR(ep_num)),
				MGC_ReadQMU32(base, MGC_O_QMU_TQCPR(ep_num)));

		QMU_ERR("[RXD][ERROR]""QCR0=%x, QCR2=%x, QCR3=%x, QGCSR=%x\n", 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR0), 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR2), 
				MGC_ReadQMU32(base, MGC_O_QMU_QCR3), 
				MGC_ReadQUCS32(base, MGC_O_QUCS_USBGCSR));

		QMU_ERR("[TXD][ERROR]""HWO=%d, BPD=%d, Next_GPD=%p, DataBuffer=%p, "
							"BufferLen=%d, Endpoint=%d\n", 
							(u32)TGPD_GET_FLAG(gpd), (u32)TGPD_GET_FORMAT(gpd), 
							TGPD_GET_NEXT(gpd), TGPD_GET_DATA(gpd), 
							(u32)TGPD_GET_BUF_LEN(gpd), (u32)TGPD_GET_EPaddr(gpd));
	}

	QMU_INFO("[TXD]""%s EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Tx_gpd_last[ep_num], Tx_gpd_end[ep_num]);
}

int mtk_q_schedule(struct musb *musb, struct musb_qh *qh, int  isRx)
{
	int			idle;
	int			best_diff;
	int			best_end, epnum;
	struct musb_hw_ep	*hw_ep = NULL;
	struct list_head	*head = NULL;
	//u8			toggle;
	//u8			txtype;
	//struct urb		*urb = next_urb(qh);
	
	if (!musb->is_active)
		return -ENODEV;


	/* use fixed hardware for control and bulk */
	if (qh->type == USB_ENDPOINT_XFER_CONTROL) {
		head = &musb->control;
		hw_ep = musb->control_ep;
		goto success;
	}

	/* else, periodic transfers get muxed to other endpoints */

	/*
	 * We know this qh hasn't been scheduled, so all we need to do
	 * is choose which hardware endpoint to put it on ...
	 *
	 * REVISIT what we really want here is a regular schedule tree
	 * like e.g. OHCI uses.
	 */
	best_diff = 4096;
	best_end = -1;

	for (epnum = 1, hw_ep = musb->endpoints + 1;
			epnum < musb->nr_endpoints;
			epnum++, hw_ep++) {
		//int	diff;

		if (musb_ep_get_qh(hw_ep, isRx) != NULL)
			continue;

		//if (hw_ep == musb->bulk_ep)
		//	continue;


		hw_ep = musb->endpoints + epnum;//got the right ep
		DBG(3,"musb_schedule:: find a hw_ep%d\n",hw_ep->epnum);
		break;
#if 0
		if (is_in)
			diff = hw_ep->max_packet_sz_rx;
		else
			diff = hw_ep->max_packet_sz_tx;
		diff -= (qh->maxpacket * qh->hb_mult);

		if (diff >= 0 && best_diff > diff) {

			/*
			 * Mentor controller has a bug in that if we schedule
			 * a BULK Tx transfer on an endpoint that had earlier
			 * handled ISOC then the BULK transfer has to start on
			 * a zero toggle.  If the BULK transfer starts on a 1
			 * controller starts the Bulk transfer on a 0 toggle
			 * irrespective of the programming of the toggle bits
			 * in the TXCSR register.  Check for this condition
			 * while allocating the EP for a Tx Bulk transfer.  If
			 * so skip this EP.
			 */
			hw_ep = musb->endpoints + epnum;
			toggle = usb_gettoggle(urb->dev, qh->epnum, !is_in);
			txtype = (musb_readb(hw_ep->regs, MUSB_TXTYPE)
					>> 4) & 0x3;
			if (!is_in && (qh->type == USB_ENDPOINT_XFER_BULK) &&
				toggle && (txtype == USB_ENDPOINT_XFER_ISOC))
				continue;

			best_diff = diff;
			best_end = epnum;
		}
#endif
	}

	if(!hw_ep){
        DBG(0,"musb::error!not find a ep for the urb\r\n");
        return -1;
        }

#if 0
	/* use bulk reserved ep1 if no other ep is free */
	if (best_end < 0 && qh->type == USB_ENDPOINT_XFER_BULK) {
		hw_ep = musb->bulk_ep;
		if (is_in)
			head = &musb->in_bulk;
		else
			head = &musb->out_bulk;

		/* Enable bulk RX/TX NAK timeout scheme when bulk requests are
		 * multiplexed.  This scheme doen't work in high speed to full
		 * speed scenario as NAK interrupts are not coming from a
		 * full speed device connected to a high speed device.
		 * NAK timeout interval is 8 (128 uframe or 16ms) for HS and
		 * 4 (8 frame or 8ms) for FS device.
		 */
		if (qh->dev)
			qh->intv_reg =
				(USB_SPEED_HIGH == qh->dev->speed) ? 8 : 4;
		goto success;
	} else if (best_end < 0) {
		return -ENOSPC;
	}
#endif

	idle = 1;
	qh->mux = 0;
	//hw_ep = musb->endpoints + best_end;
	DBG(4, "qh %p periodic slot %d\n", qh, best_end);
success:
	if (head) {
		idle = list_empty(head);
		list_add_tail(&qh->ring, head);
		qh->mux = 1;
	}
	qh->hw_ep = hw_ep;
	qh->hep->hcpriv = qh;
	if (idle) {
		mtk_kick_CmdQ(musb, isRx, qh);
                DBG(4, "mtk_kick_CmdQ!!!!!!!!\n");
        }
	return 0;
 }
#endif
#endif
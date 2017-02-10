#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/sched.h>

#include <linux/delay.h>
#include <linux/string.h>
#include <net/xfrm.h>
#include <net/esp.h>
#include <linux/crypto.h>

#include <net/ip.h>
#include <asm/io.h>
#include <asm/rt2880/rt_mmap.h>
#include "cs_adapter.h"
#include "basic_defs.h" 
#include "clib.h"
#include "mtk_AdapterInternal.h"
#include "adapter_interrupts.h"

#include <net/mtk_esp.h>
#include "mtk_ipsec.h"
#include <linux/skbuff.h>

unsigned int g_global_bus_burst_size = 1;
unsigned int *pCmdRingBase[DDK_PEC_IF_ID], *pResRingBase[DDK_PEC_IF_ID];
addrHandler_t		gACDataAddrList[DDK_PEC_IF_ID];
#define EIP97_REG_BASE		(ETHDMASYS_CRYPTO_ENGINE_BASE)
static uint32_t *pEip97RegBase = (uint32_t *)EIP97_REG_BASE;

#define HIA_CDR_y_RING_SIZE		0x018
#define HIA_CDR_y_DESC_SIZE		0x01C
#define HIA_CDR_y_CFG			0x020
#define HIA_CDR_y_DMA_CFG		0x024
#define HIA_CDR_y_THRESH		0x028
#define HIA_CDR_y_PREP_COUNT	0x02C
#define HIA_CDR_y_PROC_COUNT	0x030
#define HIA_CDR_y_PREP_PNTR		0x034
#define HIA_CDR_y_PROC_PNTR		0x038
#define HIA_CDR_y_STAT			0x03C

#define HIA_RDR_y_RING_SIZE		0x818
#define HIA_RDR_y_DESC_SIZE		0x81C
#define HIA_RDR_y_CFG			0x820
#define HIA_RDR_y_DMA_CFG		0x824
#define HIA_RDR_y_THRESH		0x828
#define HIA_RDR_y_PREP_COUNT	0x82C
#define HIA_RDR_y_PROC_COUNT	0x830
#define HIA_RDR_y_PREP_PNTR		0x834
#define HIA_RDR_y_PROC_PNTR		0x838
#define HIA_RDR_y_STAT			0x83C


#define HIA_AIC_R0_POL_CTRL		0xE000
#define HIA_AIC_R0_TYPE_CTRL	0xE004
#define HIA_AIC_R0_ENABLE_CTRL	0xE008
#define HIA_AIC_R0_ENABLE_SET	0xE00C


#define PE_EIP96_CONTEXT_CTRL	0x11008
#define PE_EIP96_CUR_SEQNUM		0x11288
#define PE_EIP96_RES_HASH0		0x112C0
#define PE_EIP96_RES_HASH1		0x112C4
#define PE_EIP96_RES_HASH2		0x112C8
#define PE_EIP96_RES_HASH3		0x112CC
#define PE_EIP96_RES_HASH4		0x112D0
#define PE_EIP96_RES_SPI_SSRC	0x112E4
#define PE_EIP96_RES_SEQNUM		0x112E8
#define PE_EIP96_RES_EXT_SEQNUM	0x112EC

#define EIP97_BUFFER_WORDSIZE	10
uint32_t EIP97_CMDRING_WORDSIZE = 128;
static uint32_t EIP97_RESULTRING_WORDSIZE	=	128;
uint32_t EIP97_CMDDESC_WORDOFFSET = 8;
static uint32_t EIP97_RESULTDESC_WORDOFFSET = 8;

#define K1_TO_PHY(x)		(((unsigned int)x) & 0x1fffffff)
#define WORDSWAP(a)     	((((a)>>24)&0xff) | (((a)>>8)&0xff00) | (((a)<<8)&0xff0000) | (((a)<<24)&0xff000000))

static inline unsigned int PaddedSize(unsigned int ByteCount, unsigned int BlockByteCount)
{
    return (ByteCount + BlockByteCount - 1) & ~(BlockByteCount - 1);
}

static inline unsigned int ComputePadBytes(unsigned int PayloadByteCount, unsigned int PadBlockByteCount)
{
    unsigned int PadByteCount;
   
    PadByteCount =  PaddedSize(PayloadByteCount + 2, PadBlockByteCount) - PayloadByteCount; 
    return PadByteCount;
}


static unsigned int cmdRingIdx[DDK_PEC_IF_ID], resRingIdx[DDK_PEC_IF_ID], \
					resPrepRingIdx[DDK_PEC_IF_ID], cmdRingFrontIdx[DDK_PEC_IF_ID];

static spinlock_t 	putlock[DDK_PEC_IF_ID], getlock[DDK_PEC_IF_ID];

#ifdef WORKQUEUE_BH
static void mtk_BH_resultGet0();
static void mtk_BH_resultGet1();
static void mtk_BH_resultGet2();
static void mtk_BH_resultGet3();
static DECLARE_WORK(mtk_interrupt_BH_result0_wq, mtk_BH_resultGet0);
static DECLARE_WORK(mtk_interrupt_BH_result1_wq, mtk_BH_resultGet1);
static DECLARE_WORK(mtk_interrupt_BH_result2_wq, mtk_BH_resultGet2);
static DECLARE_WORK(mtk_interrupt_BH_result3_wq, mtk_BH_resultGet3);
#else
static DECLARE_TASKLET( \
mtk_interrupt_BH_result0_tsk, mtk_BH_handler_resultGet, 0);
static DECLARE_TASKLET( \
mtk_interrupt_BH_result1_tsk, mtk_BH_handler_resultGet, 1);
static DECLARE_TASKLET( \
mtk_interrupt_BH_result2_tsk, mtk_BH_handler_resultGet, 2);
static DECLARE_TASKLET( \
mtk_interrupt_BH_result3_tsk, mtk_BH_handler_resultGet, 3);
#endif
static void mtk_interruptHandler0_done(void);
static void mtk_interruptHandler1_done(void);
static void mtk_interruptHandler2_done(void);
static void mtk_interruptHandler3_done(void);

#include <net/xfrm.h>

#ifdef MCRYPTO_DBG
#define ra_dbg 	printk
#else
#define ra_dbg(fmt, arg...) do {}while(0)
#endif
#ifdef MCRYPTO_DBG
u32 mcrypto_ioread32(uint32_t* addr)
{
	u32 val = ioread32(addr);
	//printk("ReadWord(0x%X, 0x%08X, 0x%08X);\n", addr val, 0xFFFFFFFF);
	printk("MReadI32 0x%08X 1; =0x%08X\n", addr, val);              
	return val;
	
}	
void mcrypto_iowrite32(u32 val, u32* addr)
{
	//printk("WriteWord(0x%X, 0x%08X, 0x%08X);\n", addr, val, 0xFFFFFFFF);
	printk("D.S SD:0x%08X %%LE %%LONG 0x%08X;\n", addr, val);	
	iowrite32(val, addr);
	return;
}	
#else
#define mcrypto_ioread32(x) ioread32(x)
#define mcrypto_iowrite32(data, x) iowrite32(data, x)
#endif

static int DMAAlign = 0;

#ifdef MCRYPTO_DBG
static void skb_dump(struct sk_buff* sk, const char* func,int line) {
        unsigned int i;

        printk("(%d)skb_dump: [%s] with len %d (%08X) headroom=%d tailroom=%d sk->data=%08X\n",
                line,func,sk->len,sk,
                skb_headroom(sk),skb_tailroom(sk),sk->data);

        for(i=(unsigned int)sk->head;i<=(unsigned int)sk->data + sk->len + 64;i++) {
                if((i % 16) == 0)
                        printk("\n");
                if(i==(unsigned int)sk->data) printk("{");
                printk("%02x ",*((unsigned char*)i));
                if(i==(unsigned int)(sk->tail)-1) printk("}");
        }
        printk("\n");
}
#else
#define skb_dump(x,y,z) do {}while(0)
#endif

/************************************************************************
*              P R I V A T E     F U N C T I O N S
*************************************************************************
*/
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
static void 
mtk_cmdHandler_free(
	void *cmdHandler_ptr
)
{
	eip97DescpHandler_t *cmdHandler;
	uint32_t *saRecord;
	uint32_t *TokenData;//, *pTCRData;
	dma_addr_t	saPhyAddr, statePhyAddr, TokenDataPhyAddr;
	
	if (cmdHandler_ptr)
		cmdHandler = (eip97DescpHandler_t *)cmdHandler_ptr;
	else
	{	
		printk("==[%s] free null cmd ===\n",__func__);
		return;
	}
	saRecord = (uint32_t *)cmdHandler->saAddr.addr;
	saPhyAddr = (uint32_t *)cmdHandler->saAddr.phyAddr;
	TokenData = (uint32_t *)cmdHandler->ACDataAddr.addr;
	TokenDataPhyAddr = (dma_addr_t)cmdHandler->ACDataAddr.phyAddr;	
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)	
	dma_free_coherent(NULL, SAPOOLSIZE, saRecord, saPhyAddr);
	
	if (cmdHandler->pTCRData)
		kfree(cmdHandler->pTCRData);
	kfree(cmdHandler);
#endif	
}

/*_______________________________________________________________________
**function name: ipsec_addrsDigestPreCompute_free
**
**description:
*   free those structions that are created for Hash Digest Pre-Compute!
*	Those sturctures won't be used anymore during encryption/decryption!
**parameters:
*   currAdapterPtr -- point to the structure that stores the addresses
*		for those structures for Hash Digest Pre-Compute.
**global:
*   none
**return:
*   none
**call:
*   none
**revision:
*   1.Qwert 20150520
**_______________________________________________________________________*/
static void 
mtk_addrsDigestPreCompute_free(
	ipsecEip93Adapter_t *currAdapterPtr
)
{
	unsigned int *ipad, *opad, *hashKeyTank;
	unsigned int *pIDigest, *pODigest;
	unsigned int blkSize;
	uint32_t *saRecord, *saRecord2, *TokenData, *TokenData2;
	dma_addr_t	ipadPhyAddr, opadPhyAddr, saPhyAddr, saPhyAddr2, TokenDataPhyAddr, TokenDataPhyAddr2;
	eip97DescpHandler_t *cmdHandler;
	addrsDigestPreCompute_t *addrsPreCompute;

	addrsPreCompute = currAdapterPtr->addrsPreCompute;
	
	if(addrsPreCompute == NULL)
		return;
	
	hashKeyTank = addrsPreCompute->hashKeyTank;
	ipad		= (unsigned int *)addrsPreCompute->ipadHandler.addr;
	ipadPhyAddr = addrsPreCompute->ipadHandler.phyAddr;
	opad		= (unsigned int *)addrsPreCompute->opadHandler.addr;
	opadPhyAddr = addrsPreCompute->opadHandler.phyAddr;
	blkSize 	= addrsPreCompute->blkSize;
	cmdHandler 	= addrsPreCompute->cmdHandler;
	saRecord 	= (uint32_t *)addrsPreCompute->saHandler.addr;
	saPhyAddr 	= addrsPreCompute->saHandler.phyAddr;
	saRecord2 	= (uint32_t *)addrsPreCompute->saHandler2.addr;
	saPhyAddr2 	= addrsPreCompute->saHandler2.phyAddr;
	TokenData = (uint32_t *)addrsPreCompute->ACDataAddr.addr;
	TokenDataPhyAddr = (uint32_t *)addrsPreCompute->ACDataAddr.phyAddr;
	TokenData2 = (uint32_t *)addrsPreCompute->ACDataAddr2.addr;
	TokenDataPhyAddr2 = (uint32_t *)addrsPreCompute->ACDataAddr2.phyAddr;

	pIDigest 	= addrsPreCompute->pIDigest;
	pODigest 	= addrsPreCompute->pODigest;		
#if !defined(CONFIG_HWCRYPTO_MEMPOOL)
	kfree(pODigest);
	kfree(pIDigest);

	dma_free_coherent(NULL, addrsPreCompute->RecPoolHandler.size, addrsPreCompute->RecPoolHandler.addr, addrsPreCompute->RecPoolHandler.phyAddr);	
	kfree(cmdHandler);	
	kfree(hashKeyTank);
	addrsPreCompute->pODigest = NULL;
	addrsPreCompute->pIDigest = NULL;
	kfree(addrsPreCompute);
	currAdapterPtr->addrsPreCompute = NULL;
#endif	
}
#endif
static void 
mtk_hashDigests_get(
	ipsecEip93Adapter_t *currAdapterPtr
)
{

	eip97DescpHandler_t *cmdHandler;
	uint32_t *ContextRec;
	addrsDigestPreCompute_t* addrsPreCompute;
	unsigned int i;
	unsigned int* pIDigest, *pODigest, *pData;

	cmdHandler = (eip97DescpHandler_t *)(currAdapterPtr->cmdHandler);

	addrsPreCompute = currAdapterPtr->addrsPreCompute;

	pIDigest = (unsigned int*)(cmdHandler->saAddr.addr+8+cmdHandler->KeySizeDW*4);
	pODigest = pIDigest+addrsPreCompute->digestWord;
	
	ra_dbg("pIDigest=%08X pODigest=%08X KeySizeDW=%d cmd->digestWord=%d\n",\
			pIDigest ,pODigest,cmdHandler->KeySizeDW,cmdHandler->digestWord);	
	for (i = 0; i < (addrsPreCompute->digestWord); i++)
	{		
		pIDigest[i] = addrsPreCompute->pIDigest[i];
		pODigest[i] = addrsPreCompute->pODigest[i];
	}
	ra_dbg("%s IDigest[%d]= {\n",__func__,addrsPreCompute->digestWord);
	pData = pIDigest;
	for(i=0; i<addrsPreCompute->digestWord; i++)
	{
		ra_dbg("0x%08X ",pData[i]);
		if (i%4==3) ra_dbg("\n");
		if(i==(addrsPreCompute->digestWord-1)) ra_dbg("}\n");
	}
	ra_dbg("%s ODigest[%d]= {\n",__func__,addrsPreCompute->digestWord);
	pData = pODigest;
	for(i=0; i<addrsPreCompute->digestWord; i++)
	{
		ra_dbg("0x%08X ",pData[i]);
		if (i%4==3) ra_dbg("\n");
		if(i==(addrsPreCompute->digestWord-1)) ra_dbg("}\n");
	}

}

static void 
mtk_hashDigests_set(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int isInOrOut
)
{
	//resDescpHandler only has physical addresses, so we have to get saState's virtual address from addrsPreCompute.
	addrsDigestPreCompute_t *addrsPreCompute;
	uint32_t* digest, *pData;
	unsigned int i, digestWord;
	
	
	addrsPreCompute = (addrsDigestPreCompute_t*) currAdapterPtr->addrsPreCompute;
	digestWord = addrsPreCompute->digestWord;
	
	if (isInOrOut == 1) //for Inner Digests
	{		
		ra_dbg("IPad[%d] = {",digestWord);
		pData = addrsPreCompute->ipadHandler.addr;
		for (i = 0; i < digestWord; i++)
		{
			ra_dbg("0x%08X,",pData[i]);
		}
		ra_dbg("}\n");

		digest = (uint32_t*)(addrsPreCompute->saHandler.addr+8);		

		ra_dbg("IDigest[%d] = {",digestWord);
		for (i = 0; i < digestWord; i++)
		{
			//digest[i] = WORDSWAP(digest[i]);
			addrsPreCompute->pIDigest[i] = (digest[i]);
			ra_dbg("0x%08X,",addrsPreCompute->pIDigest[i]);
		}
		ra_dbg("}\n");
	}
	else if (isInOrOut == 2) //for Outer Digests
	{		
		ra_dbg("OPad[%d] = {",digestWord);
		pData = addrsPreCompute->opadHandler.addr;
		for (i = 0; i < digestWord; i++)
		{
			ra_dbg("0x%08X,",pData[i]);
		}
		ra_dbg("}\n");
	
		digest = (uint32_t*)(addrsPreCompute->saHandler2.addr+8);	
		ra_dbg("ODigest[%d] = {",digestWord);
		for (i = 0; i < digestWord; i++)
		{
			//digest[i] = WORDSWAP(digest[i]);
			addrsPreCompute->pODigest[i] = (digest[i]);
			ra_dbg("0x%08X,",addrsPreCompute->pODigest[i]);
		}
		ra_dbg("}\n");
	}
}

static unsigned int 
mtk_eip97UserId_get(
	void *resHandler
)
{
/* In our case, during hash digest pre-compute, the userId will be
 * currAdapterPtr; but during encryption/decryption, the userId
 * will be skb
 */

	return ((eip97DescpHandler_t*)resHandler)->userId;
}

static unsigned int 
mtk_pktLength_get(
	void *resHandler
)
{
	return ((eip97DescpHandler_t*)resHandler)->peRData.peRDataW0.bits.Packet_Length;
}	

static unsigned char 
mtk_espNextHeader_get(
	void *resHandler
)
{
	return ((eip97DescpHandler_t*)resHandler)->peRData.peRDataW3.bits.Next_Header;
}

static void 
mtk_espNextHeader_set(
	void *cmdHandler_ptr, 
	unsigned char protocol	
)
{
	//ipsec esp's next-header which is IPPROTO_IPIP for tunnel or ICMP/TCP/UDP for transport mode
	eip97DescpHandler_t* cmdHandler = (eip97DescpHandler_t* )cmdHandler_ptr;
	cmdHandler->nexthdr = protocol;
}

static int 
mtk_preComputeIn_cmdDescp_set(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int direction
)
{
	addrsDigestPreCompute_t* addrsPreCompute = currAdapterPtr->addrsPreCompute;
	eip97DescpHandler_t *cmdHandler;
	uint32_t* *saState;
	dma_addr_t	statePhyAddr;
	int errVal=1;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	cmdHandler = addrsPreCompute->cmdHandler;
#else
	unsigned int flags = 0;

	if (in_atomic())
		flags |= GFP_ATOMIC;
	else
		flags |= GFP_KERNEL;
	cmdHandler = (eip97DescpHandler_t *) kzalloc(sizeof(eip97DescpHandler_t), flags);
	if (unlikely(cmdHandler == NULL))
	{
		printk("\n\n !!kmalloc for cmdHandler failed!! \n\n");
		return -ENOMEM;
	}
	addrsPreCompute->cmdHandler = cmdHandler;
#endif
	saState = addrsPreCompute->RecPoolHandler.addr + 1024 + STATE_OFFSET;
	statePhyAddr = addrsPreCompute->RecPoolHandler.phyAddr + 1024 + STATE_OFFSET;

	memset(saState, 0, addrsPreCompute->blkSize);
	addrsPreCompute->stateHandler.addr = (unsigned int)saState;
	addrsPreCompute->stateHandler.phyAddr = statePhyAddr;	
	addrsPreCompute->stateHandler.size = addrsPreCompute->blkSize;

	cmdHandler->srcAddr.phyAddr = addrsPreCompute->ipadHandler.phyAddr;

	cmdHandler->dstAddr.addr = addrsPreCompute->stateHandler.addr;
	cmdHandler->dstAddr.phyAddr = addrsPreCompute->stateHandler.phyAddr;
	cmdHandler->dstAddr.size = addrsPreCompute->stateHandler.size;
    	
	//save needed info in EIP97's userID, so the needed info can be used by the tasklet which is raised by interrupt.
	cmdHandler->userId = (unsigned int)currAdapterPtr;
	errVal = PreComputeDigestToken(currAdapterPtr, direction, addrsPreCompute->blkSize, 1);
	if (errVal < 0 )
		printk("preComputeIn faile to call BuildBasicAlgToken.\n");

	return errVal;
}

static int 
mtk_preComputeOut_cmdDescp_set(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int direction
)
{
	addrsDigestPreCompute_t* addrsPreCompute = currAdapterPtr->addrsPreCompute;	
	uint32_t *saState2;
	dma_addr_t	statePhyAddr2;
	int errVal;
	eip97DescpHandler_t *cmdHandler = (eip97DescpHandler_t *)addrsPreCompute->cmdHandler;
	unsigned int flags = 0;

	saState2 = addrsPreCompute->RecPoolHandler.addr + STATE_OFFSET;
	statePhyAddr2 = addrsPreCompute->RecPoolHandler.phyAddr + STATE_OFFSET; 
	memset(saState2, 0, addrsPreCompute->blkSize);
	addrsPreCompute->stateHandler2.addr = (unsigned int)saState2;
	addrsPreCompute->stateHandler2.phyAddr = statePhyAddr2;	
	addrsPreCompute->stateHandler2.size = addrsPreCompute->blkSize;
	
	cmdHandler->srcAddr.phyAddr = addrsPreCompute->opadHandler.phyAddr;
	cmdHandler->dstAddr.addr = addrsPreCompute->stateHandler2.addr;
	cmdHandler->dstAddr.phyAddr = addrsPreCompute->stateHandler2.phyAddr;
	cmdHandler->dstAddr.size = addrsPreCompute->stateHandler2.size;

	errVal = PreComputeDigestToken(currAdapterPtr, direction, addrsPreCompute->blkSize, 0);
	if (errVal < 0 )
		printk("preComputeOut faile to call BuildBasicAlgToken.\n");
	else		
		return 1;

	return errVal;
}

static int 
mtk_cmdHandler_cmdDescp_set(
	ipsecEip93Adapter_t *currAdapterPtr, 
	unsigned int direction,
	unsigned int cipherAlg, 
	unsigned int hashAlg,
	unsigned int digestWord, 
	unsigned int cipherMode, 
	unsigned int enHmac, 
	unsigned int aesKeyLen, 
	unsigned int *cipherKey, 
	unsigned int keyLen, 
	unsigned int spi, 
	unsigned int padCrtlStat
)
{
	eip97DescpHandler_t *cmdHandler;
	int errVal;
	unsigned int keyWord, i;
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	unsigned int flags = 0;

	if (in_atomic())
		flags |= GFP_ATOMIC;
	else
		flags |= GFP_KERNEL;

	cmdHandler = (eip97DescpHandler_t *) kzalloc(sizeof(eip97DescpHandler_t), flags);
	if (unlikely(cmdHandler == NULL))
	{
		printk("\n\n !!kmalloc for cmdHandler_prepare failed!! \n\n");
		return -ENOMEM;
	}
	cmdHandler->pIDigest = kzalloc(512, GFP_KERNEL);
	if (unlikely(cmdHandler->pIDigest == NULL))
	{
		printk("\n\n !!kmalloc for cmdHandler_prepare pIDigest failed!! \n\n");
		goto free_cmdHandler;
	}
	cmdHandler->pODigest = kzalloc(512, GFP_KERNEL);
	if (unlikely(cmdHandler->pODigest == NULL))
	{
		printk("\n\n !!kmalloc for cmdHandler_prepare pODigest failed!! \n\n");
		goto free_pIDigest;
	}
#endif
	ra_dbg("--[%d] %s dir=%d cipherAlg=%d hashAlg=%d digestWord=%d cipherMode=%d ,keyLen=%d spi=%x --\n" \
			,__LINE__,__func__,direction,cipherAlg,hashAlg,\
			digestWord,cipherMode,keyLen,spi);
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	/* restore cmdHandler for later use */
	currAdapterPtr->cmdHandler = cmdHandler;
#else
	memset(((eip97DescpHandler_t *)(currAdapterPtr->cmdHandler))->pIDigest, 0 , 512);
	memset(((eip97DescpHandler_t *)(currAdapterPtr->cmdHandler))->pODigest, 0 , 512);	
#endif
	errVal = BuildESPToken(currAdapterPtr, direction, cipherAlg, hashAlg, digestWord, cipherMode, cipherKey, keyLen);
	if (errVal==0)
	{
		printk("==	BuildESPToken errVal=0 ==\n");
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
goto EXIT;
#else
		goto free_pODigest;
#endif
	}

	return 1;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
EXIT:
#else
free_pODigest:
	kfree(cmdHandler->pODigest);
free_pIDigest:
	kfree(cmdHandler->pIDigest);
free_cmdHandler:
	kfree(cmdHandler);
#endif
	return errVal;
}

/*_______________________________________________________________________
**function name: mtk_packet_put
**
**description:
*   put command descriptor into EIP97's Command Descriptor Ring and
*	then kick off EIP97.
**parameters:
*   cmdDescp -- point to the command handler that stores the needed
*		info for the command descriptor.
*	skb -- the packet for encryption/decryption
**global:
*   none
**return:
*   0 -- success.
**call:
*   none
**revision:
*   1.Qwert 20150420
**_______________________________________________________________________*/
static int 
mtk_packet_put(
	void *cmdDescp_ptr, 
	struct sk_buff *skb, //skb == NULL when in digestPreCompute
	unsigned int rdx
)
{
	unsigned int *pCrd = NULL;
	unsigned int *pRrd = NULL;
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int addedLen;
	unsigned int *addrCurrAdapter;
	unsigned long flags;
	u32* pData = NULL;
	u32* token_ptr = NULL;
	dma_addr_t pDataPhy;
	uint32_t data;
	unsigned int PadBytes = 0;
	unsigned int checksum = 0;
	eip97DescpHandler_t *cmdDescp = (eip97DescpHandler_t *)cmdDescp_ptr;

	if(likely(skb != NULL))
	{

		addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
		currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
		if (currAdapterPtr==NULL)
		{	
			printk("!! currAdapterPtr from skb = NULL,skb=%08X!!\n",skb);
			return -1; 
		}
		addedLen = currAdapterPtr->addedLen;

		pCrd = pCmdRingBase[rdx];
		pRrd = pResRingBase[rdx];						
		spin_lock(&currAdapterPtr->lock);
		currAdapterPtr->packet_count++;
		spin_unlock(&currAdapterPtr->lock);		
		pRrd += resPrepRingIdx[rdx];		
		pCrd += cmdRingIdx[rdx];
		token_ptr = gACDataAddrList[rdx].addr +(cmdRingIdx[rdx]>>4)*16*4;

		memcpy(token_ptr, cmdDescp->ACDataAddr.addr, (cmdDescp->TokenWords+1)*sizeof(u32));	
		pCrd[5] = cmdDescp->saAddr.phyAddr;
		
		dma_sync_single_for_device(NULL, virt_to_phys(skb->data), skb->len + addedLen*1, DMA_TO_DEVICE);
		pRrd[1] = pCrd[1] = virt_to_phys(skb->data);			

		if (currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION)
		{	
			PadBytes = ComputePadBytes(skb->len, cmdDescp->blkSize);
			pRrd[0] = BIT_23|BIT_22|(skb->len + addedLen*1);
		}
		else
			pRrd[0] = BIT_23|BIT_22|(skb->len);
	
		data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_PREP_COUNT+0x1000*rdx) >> 2));
		data &= ~(BIT_16-1);
		data |= (1*(EIP97_RESULTDESC_WORDOFFSET<<2));
		mcrypto_iowrite32(data, pEip97RegBase + ((HIA_RDR_y_PREP_COUNT+0x1000*rdx) >> 2));
		
		if (currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION)
		{
			
			token_ptr[1] = (token_ptr[1]&(~0x1ffff))|skb->len;
			token_ptr[2] = (token_ptr[2]&~(0x01ffff))|(cmdDescp->nexthdr<<9)|PadBytes;
	
			pCrd[4] = 0x00ACE000|(unsigned char)(currAdapterPtr->idx);
			token_ptr[cmdDescp->TokenWords+2] = 0x00ACE000|\
					0xFF000000&(((u32)(currAdapterPtr->idx))<<24)|(cmdRingIdx[rdx]&0x00000FFF);
			token_ptr[cmdDescp->TokenWords]  = 0xF0000000|cmdRingIdx[rdx];
			pCrd[0] = ((cmdDescp->TokenWords+3)<<24)|BIT_23|BIT_22|skb->len;

		}
		else
		{	
			struct esp_data *esp = currAdapterPtr->x->data;
			struct ip_esp_hdr *esph = skb->data;
			int ivlen = crypto_aead_ivsize(esp->aead);
			int icvlen = crypto_aead_authsize(esp->aead);
			token_ptr[1] = (token_ptr[1]&(~0x1ffff))|(-8u+skb->len-ivlen-icvlen-0);
	
			pCrd[4] = 0x00DEC000|(unsigned char)(currAdapterPtr->idx);
			token_ptr[cmdDescp->TokenWords+2] = esph->seq_no;
			token_ptr[cmdDescp->TokenWords]  = 0xF0000000|cmdRingIdx[rdx];
			token_ptr[cmdDescp->TokenWords+3] = 0x00DEC000|\
					0xFF000000&(((u32)(cmdDescp->TokenWords+4))<<24)|(cmdRingIdx[rdx]&0x00000FFF);

			pCrd[0] = ((cmdDescp->TokenWords+4)<<24)|BIT_23|BIT_22|skb->len;

		}		

		token_ptr[cmdDescp->TokenWords+1] = skb;
		cmdDescp->TokenHeaderWord = (cmdDescp->TokenHeaderWord&(~0x1ffff))|skb->len;
		pCrd[2] = (u32)(gACDataAddrList[rdx].phyAddr)+(cmdRingIdx[rdx]>>4)*16*4;

	}
	else
	{
		addrsDigestPreCompute_t* addrsDigestPreCompute;
		currAdapterPtr = cmdDescp->userId;
		addrsDigestPreCompute = currAdapterPtr->addrsPreCompute;
		
		pCrd = pCmdRingBase[rdx];
		pRrd = pResRingBase[rdx];

		pRrd += resPrepRingIdx[rdx];		
		pCrd += cmdRingIdx[rdx];
		token_ptr = cmdDescp->ACDataAddr.addr;
		
		pRrd[0] = BIT_23|BIT_22|(addrsDigestPreCompute->blkSize);
		pRrd[1] = cmdDescp->dstAddr.phyAddr;
	
		data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_PREP_COUNT+0x1000*rdx) >> 2));
		data &= ~(BIT_16-1);
		data |= (1*(EIP97_RESULTDESC_WORDOFFSET<<2));
		mcrypto_iowrite32(data, pEip97RegBase + ((HIA_RDR_y_PREP_COUNT+0x1000*rdx) >> 2));

		token_ptr[cmdDescp->TokenWords+1] = cmdDescp->userId;
		pCrd[0] = ((cmdDescp->TokenWords+2)<<24)|BIT_23|BIT_22|addrsDigestPreCompute->blkSize;		
		
		pCrd[1] = cmdDescp->srcAddr.phyAddr;
		if (addrsDigestPreCompute->saHandler2.phyAddr)
		{	
			pCrd[5] = addrsDigestPreCompute->saHandler2.phyAddr;
			pData = addrsDigestPreCompute->saHandler2.addr;
			pData[0] |= 0xa<<16;
			pData[1] |= 0x1;
		}
		else
		{	
			pCrd[5] = addrsDigestPreCompute->saHandler.phyAddr;
			pData = addrsDigestPreCompute->saHandler.addr;
			pData[0] |= 0xa<<16;
			pData[1] |= 0x1;
		}	
		cmdDescp->TokenHeaderWord &= ~(0x3<<20);

		pCrd[4] = 0x00CAD000|resPrepRingIdx[rdx];
		
		pCrd[2] = cmdDescp->ACDataAddr.phyAddr;
		currAdapterPtr->packet_count++;

	}

	pCrd[3] = cmdDescp->TokenHeaderWord;
/*
	ra_dbg("--CmdRing[%d] Desc[%d] [0x%08X] %c--\n", rdx, cmdRingIdx[rdx], \
				pCrd, (currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION) ? 'E': \
					((currAdapterPtr->isEncryptOrDecrypt==CRYPTO_DECRYPTION) ? 'D' : 'A'));

	ra_dbg("--[0x%08X]--\n",pCrd[0]);
	ra_dbg("--[0x%08X]--\n",pCrd[1]);
	ra_dbg("--[0x%08X]--\n",pCrd[2]);
	ra_dbg("--[0x%08X]--\n",pCrd[3]);
	ra_dbg("--[0x%08X]--\n",pCrd[4]);
	ra_dbg("--[0x%08X]--\n",pCrd[5]);
	ra_dbg("--ResRing[%d] Prep Desc[%d] [0x%08X]--\n", rdx,resPrepRingIdx[rdx],pRrd);
	ra_dbg("--[0x%08X]--\n",pRrd[0]);
	ra_dbg("--[0x%08X]--\n",pRrd[1]);

	{
		int i;
		uint32_t* sa = cmdDescp->saAddr.addr;
		ra_dbg("--SA[0x%08X][0x%08X] W=%d--\n", pCrd[5],sa,cmdDescp->saAddr.size>>2);
		for (i=0 ;i < cmdDescp->saAddr.size>>2; i++)
			ra_dbg("--[0x%08X]--\n",sa[i]);
	}
	
	{
		int i;
		uint32_t* token = cmdDescp->ACDataAddr.addr;
		if ((pCrd[4]&0x00FFF000)!=0x00CAD000)
			token = gACDataAddrList[rdx].addr + (cmdRingIdx[rdx]>>4)*16*4;
		ra_dbg("--Token[0x%08X][0x%08X] W=%d--\n", pCrd[2],token,cmdDescp->TokenWords);
		for (i=0 ;i < cmdDescp->TokenWords+4; i++)
			ra_dbg("--[0x%08X]--\n",token[i]);
	}
*/
	data = mcrypto_ioread32(pEip97RegBase + ((HIA_CDR_y_PREP_COUNT+0x1000*rdx) >> 2));
	data &= ~(BIT_16-1);
	//data |= ((1*(EIP97_CMDDESC_WORDOFFSET<<2)) | (1<<29));
	data |= (EIP97_CMDDESC_WORDOFFSET<<2);
	mcrypto_iowrite32(data, pEip97RegBase + ((HIA_CDR_y_PREP_COUNT+0x1000*rdx) >> 2));


	cmdRingIdx[rdx]+=EIP97_CMDDESC_WORDOFFSET;
	if (cmdRingIdx[rdx] == EIP97_CMDRING_WORDSIZE)
	{
		cmdRingIdx[rdx] = 0;
	}
	resPrepRingIdx[rdx]+=EIP97_RESULTDESC_WORDOFFSET;
	if (resPrepRingIdx[rdx] == EIP97_RESULTRING_WORDSIZE)
	{
		resPrepRingIdx[rdx] = 0;
	}
	
	return 0; //success
}

/*_______________________________________________________________________
**function name: mtk_packet_get
**
**description:
*   get result descriptor from EIP97's Result Descriptor Ring.
**parameters:
*   resDescp -- point to the result handler that stores the needed
*		info for the result descriptor.
**global:
*   none
**return:
*   0  -- EIP97 has no result yet.
*   1  -- EIP97 has results ready.
*   -1 -- the current result is wrong!
**call:
*   none
**revision:
*   1.Qwert 20150420
**_______________________________________________________________________*/
static int 
mtk_packet_get(
	void *resDescp_ptr,
	unsigned int rdx
)
{
	unsigned int *pRrd = NULL;
	unsigned int *pCrd = NULL;
	unsigned int done1, done2, buf_sts,err_code, PktCnt, timeCnt = 0;
	unsigned long flags;
	struct sk_buff *skb = NULL;
	int retVal;
	int nTry = -1;
	unsigned int checksum = 0;
	eip97DescpHandler_t *resDescp = (eip97DescpHandler_t*)resDescp_ptr;
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int *addrCurrAdapter;
	uint32_t data,data1, reg;
	uint32_t temp[6];

	if ((rdx >= 4) || (rdx < 0))
		ra_dbg("[error rdx=%d]\n",rdx);

	data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_PROC_COUNT+0x1000*rdx) >> 2));

	PktCnt = (data & (BIT_24 - 1))>>2;
	//don't wait for Crypto Engine in order to speed up!
	if(PktCnt == 0)
	{
		ra_dbg("--PktCnt=0--\n");
		return 0; //no result yet
	}
			
	pRrd = pResRingBase[rdx];
	pRrd += resRingIdx[rdx];

	ra_dbg("--ResRing[%d] Proc Desc[%d]--\n", rdx,resRingIdx[rdx]); 
	ra_dbg("--[0x%08X]--\n",pRrd[0]);
	ra_dbg("--[0x%08X]--\n",pRrd[1]);
	ra_dbg("--[0x%08X]--\n",pRrd[2]);
	ra_dbg("--[0x%08X]--\n",pRrd[3]);
	ra_dbg("--[0x%08X]--\n",pRrd[4]);
	ra_dbg("--[0x%08X]--\n",pRrd[5]);
	ra_dbg("--[0x%08X]--\n",pRrd[6]);
	ra_dbg("--[0x%08X]--\n",pRrd[7]);
	ra_dbg("--[0x%08X]--\n",pRrd[8]);
	ra_dbg("--[0x%08X]--\n",pRrd[9]);
	

	while (1)
	{
		data1 = pRrd[0] + pRrd[1] + pRrd[2] + pRrd[3] + pRrd[4] + pRrd[5];
		if (data1==0)
		{	
			mcrypto_proc.dbg_pt[13]++;
			while (1)
			{
				int done = 0;
				done = (pRrd[0]>>22)&0x3;
				if (done==0x3)
					break;	
			}
		}
		
		resDescp->RxDescW0.word = pRrd[0];
		buf_sts = (resDescp->RxDescW0.word>>20)&0x3;
		err_code = pRrd[2]>>17;
		resDescp->userId = pRrd[7];
		resDescp->peRData.peRDataW0.word = pRrd[2];
		resDescp->peRData.peRDataW1.word = pRrd[3];
		resDescp->peRData.peRDataW3.word = pRrd[5];

		if(buf_sts||err_code)
		{
			mcrypto_proc.dbg_pt[12]++;
			ra_dbg("Get errcode=%x buf_sts=%x reg=%08X pRrd[8]=%08X pRrd[9]=%08X\n",\
					err_code,buf_sts,reg,pRrd[8],pRrd[9]);
			if ((((unsigned int)resDescp->userId>>28)&0x0F)==0x0)	
			{	
				printk("[%d]%s buf_sts=%d err_code=%d userId=%08X\n",\
						__LINE__,__func__,buf_sts,err_code,resDescp->userId);
				retVal = -1;					
				goto EXIT;
			}
			else
			{	
				skb = (struct sk_buff *)resDescp->userId;
				addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
				currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
				printk("\n\n !PE Ring%d[%d] Buf_ov=0x%x ErrCode=0x%x! op=%c qlen=%d packet_count=%d,idx=%d spi=%x\n\n",\
					   	rdx, resRingIdx[rdx], buf_sts, err_code,\
						(currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION) ?\
					   	'E' : 'D' , currAdapterPtr->skbQueue.qlen,currAdapterPtr->packet_count,currAdapterPtr->idx,currAdapterPtr->spi);
				kfree_skb(skb);	   		
				spin_lock(&currAdapterPtr->lock);					
				currAdapterPtr->packet_count--;
				spin_unlock(&currAdapterPtr->lock);						
			}
			retVal = -1;
			goto EXIT;
		}
		else
		{
			if ((pRrd[4]&(0x00FFF000))!=0x00CAD000)
			{					
				struct xfrm_state *x;
				struct ip_esp_hdr *esph;						
				skb = (struct sk_buff *)resDescp->userId;
				if ((((unsigned int)skb>>28)&0x0F)==0x0)	
				{	
					printk("!! skb from userId = %x!!\n",skb);
					retVal = -2;
					goto EXIT;
				}	
				if (skb==NULL)
				{	
					printk("!! skb from userId = NULL!!\n");
					retVal = -2;
					goto EXIT;
				}
					
				addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
				if ((((unsigned int)addrCurrAdapter>>28)&0x0F)==0x0)	
				{
					printk("!! addrCurrAdapter = %x, skb=%08X!!\n",addrCurrAdapter,skb);
					retVal = -2;
					goto EXIT;
				}	
				currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
				if ((((unsigned int)currAdapterPtr>>28)&0x0F)==0x0)	
				{	
					printk("!! currAdapterPtr = %x, skb=%08X!!\n",currAdapterPtr,skb);
					retVal = -2;
					goto EXIT;
				}	
				if (currAdapterPtr==NULL)
				{	
					printk("!! currAdapterPtr from skb = NULL,skb=%08X,pRrd[4]=%08X!!\n",skb,pRrd[4]);
					retVal = -3;
					goto EXIT;
				}
					
				x = currAdapterPtr->x;
				if (((((unsigned int)x)>>28)&0x0F)==0x0)	
				{	
					printk("!! x = %x, skb=%08X!!\n",x,skb);
					retVal = -2;
					currAdapterPtr->packet_count--;
					goto EXIT;
				}
				if (x==NULL)
				{	
					printk("!! x from currAdapterPtr = NULL!!\n");
					retVal = -4;
					currAdapterPtr->packet_count--;
					goto EXIT;
				}
					
				spin_lock(&currAdapterPtr->lock);
				currAdapterPtr->packet_count--;
				spin_unlock(&currAdapterPtr->lock);	

				if (((pRrd[1]>>28)&0x0F)==0x0)
				{
					printk("!! pRrd[1] = %x!!\n",pRrd[1]);
				}
	
				dma_sync_single_for_device(NULL,  pRrd[1], resDescp->peRData.peRDataW0.bits.Packet_Length, \
											DMA_FROM_DEVICE);
				retVal = resDescp->peRData.peRDataW0.bits.Packet_Length;
	
				if (currAdapterPtr->isEncryptOrDecrypt==CRYPTO_DECRYPTION)
				{
					resDescp->seq_no = pRrd[8];
				}	
			}
			else
			{	
				currAdapterPtr = resDescp->userId;
				spin_lock(&currAdapterPtr->lock);
				currAdapterPtr->packet_count--;
				spin_unlock(&currAdapterPtr->lock);
				if (currAdapterPtr->spi==0)
					retVal = -1;
				else	
					retVal = 1;				
			}	
		}
		break; 
	}	//while (1)

EXIT:
	resRingIdx[rdx]+=EIP97_RESULTDESC_WORDOFFSET;
	if (resRingIdx[rdx] == EIP97_RESULTRING_WORDSIZE)
	{
		resRingIdx[rdx] = 0;
	}
	cmdRingFrontIdx[rdx] += EIP97_CMDDESC_WORDOFFSET;		
	if (cmdRingFrontIdx[rdx] == EIP97_CMDRING_WORDSIZE)
		cmdRingFrontIdx[rdx] = 0;
		
	data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_PROC_COUNT+0x1000*rdx) >> 2));
	data &= ~(BIT_16-1);
	data |= (EIP97_RESULTDESC_WORDOFFSET<<2);
	mcrypto_iowrite32(data, pEip97RegBase + ((HIA_RDR_y_PROC_COUNT+0x1000*rdx) >> 2));

	data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_STAT+0x1000*rdx) >> 2));
	if (data&0x0FF)
		mcrypto_iowrite32(data, pEip97RegBase + ((HIA_RDR_y_STAT+0x1000*rdx) >> 2));

	return retVal;
	
}
		

static bool 
mtk_eip97CmdResCnt_check(
	unsigned int rdx
)
{

	int diff,diff2;
	if (cmdRingFrontIdx[rdx] > cmdRingIdx[rdx] )
		diff = (cmdRingFrontIdx[rdx]-cmdRingIdx[rdx]);
	else
		diff = EIP97_CMDRING_WORDSIZE - (cmdRingIdx[rdx]-cmdRingFrontIdx[rdx]);
	if (resRingIdx[rdx] > resPrepRingIdx[rdx] )
		diff2 = (resRingIdx[rdx]-resPrepRingIdx[rdx]);
	else
		diff2 = EIP97_RESULTRING_WORDSIZE - (resPrepRingIdx[rdx]-resRingIdx[rdx]);
	
	return ((diff >= (EIP97_CMDDESC_WORDOFFSET*2)) && (diff2 >= (EIP97_CMDDESC_WORDOFFSET*2)) );
}


static unsigned int 
mtk_espSeqNum_get(
	void *resHandler_ptr
)
{
	return (mcrypto_ioread32(pEip97RegBase + (PE_EIP96_RES_SEQNUM >> 2)));
}
/************************************************************************
*              P U B L I C     F U N C T I O N S
*************************************************************************
*/
void 
mtk_ipsec_init(
	void
)
{
	int i;
	uint32_t Data;
	unsigned int flags;
	DMAAlign = 4;
	
	printk("== IPSEC Crypto Engine Driver : %s %s ==\n",__DATE__,__TIME__);
	for (i = 0 ; i < DDK_PEC_IF_ID ; i++)
	{
		spin_lock_init(&putlock[i]);
		spin_lock_init(&getlock[i]);
	}
	ipsec_eip93_adapters_init();
	ipsec_cryptoLock_init();
	
	for (i = 0 ; i < DDK_PEC_IF_ID ; i++)
	{
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_CDR_y_RING_SIZE+0x1000*i)>>2));
		EIP97_CMDRING_WORDSIZE = (Data & (BIT_24-1))>>2;
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_RING_SIZE+0x1000*i)>>2));
		EIP97_RESULTRING_WORDSIZE = (Data & (BIT_24-1))>>2;
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_CDR_y_DESC_SIZE+0x1000*i)>>2));
		EIP97_CMDDESC_WORDOFFSET = (Data>>16) &0x0FF;
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_DESC_SIZE+0x1000*i)>>2));
		EIP97_RESULTDESC_WORDOFFSET = (Data>>16) &0x0FF;
		
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_CDR_y_CFG+0x1000*i)>>2));
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_CFG+0x1000*i)>>2));
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_CDR_y_DMA_CFG+0x1000*i)>>2));
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_DMA_CFG+0x1000*i)>>2));
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_CDR_y_THRESH+0x1000*i)>>2));
		
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_STAT+0x1000*i) >> 2));
		Data |= 1<<5;//timeout_irq
		mcrypto_iowrite32(Data, pEip97RegBase + ((HIA_RDR_y_STAT+0x1000*i) >> 2));
		
		Data = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*i)>>2));
		printk("-- HIA_RDR_%d_THRESH=0x%08X ---\n",i, Data);
		Data &= ~0x3fffff;
		Data |= (1<<23)|(0x1);
		mcrypto_iowrite32(Data, pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*i)>>2));
	}
		
	Data = mcrypto_ioread32(pEip97RegBase + (PE_EIP96_CONTEXT_CTRL>>2));

	printk("-- EIP97_CMDRING_WORDSIZE=%d 		EIP97_RESULTRING_WORDSIZE=%d --\n",
					EIP97_CMDRING_WORDSIZE, EIP97_RESULTRING_WORDSIZE);
	printk("-- EIP97_CMDDESC_WORDOFFSET=%d 		EIP97_RESULTDESC_WORDOFFSET=%d --\n",
					EIP97_CMDDESC_WORDOFFSET, EIP97_RESULTDESC_WORDOFFSET);				
	//function pointer init
	ipsec_packet_put = mtk_packet_put;
	ipsec_packet_get = mtk_packet_get;

	ipsec_eip93CmdResCnt_check = mtk_eip97CmdResCnt_check;
	ipsec_preComputeIn_cmdDescp_set = mtk_preComputeIn_cmdDescp_set;
	ipsec_preComputeOut_cmdDescp_set = mtk_preComputeOut_cmdDescp_set;
	ipsec_cmdHandler_cmdDescp_set = mtk_cmdHandler_cmdDescp_set;
	ipsec_espNextHeader_set = mtk_espNextHeader_set;
	ipsec_espNextHeader_get = mtk_espNextHeader_get;
	ipsec_pktLength_get = mtk_pktLength_get;
	ipsec_eip93HashFinal_get = NULL;
	ipsec_eip93UserId_get = mtk_eip97UserId_get;
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	ipsec_addrsDigestPreCompute_free = mtk_addrsDigestPreCompute_free;
	ipsec_cmdHandler_free = mtk_cmdHandler_free;
#endif
	ipsec_hashDigests_get = mtk_hashDigests_get;
	ipsec_hashDigests_set = mtk_hashDigests_set;
	
	ipsec_espSeqNum_get = mtk_espSeqNum_get;
	
	for (i = 0 ; i < DDK_PEC_IF_ID ; i++)
	{
		//eip97 info init
		cmdRingFrontIdx[i] = cmdRingIdx[i] = mcrypto_ioread32(pEip97RegBase + ((HIA_CDR_y_PREP_PNTR+0x1000*i) >> 2)) & (BIT_24-1); 
		resPrepRingIdx[i] = resRingIdx[i] = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_PROC_PNTR+0x1000*i) >> 2)) & (BIT_24-1);
		cmdRingFrontIdx[i] = EIP97_CMDRING_WORDSIZE-EIP97_CMDDESC_WORDOFFSET;
	
		printk("-- cmdRingFrontIdx%d=%d cmdRingIdx%d=%d --\n",i,cmdRingFrontIdx[i],i,cmdRingIdx[i]);
		printk("-- resPrepRingIdx%d=%d resRingIdx%d=%d --\n",i,resPrepRingIdx[i],i,resRingIdx[i]);
	}
	
	for (i = 0 ; i < DDK_PEC_IF_ID ; i++)
	{
		flags = 0;
		if (in_atomic())
			flags |= GFP_ATOMIC;
		else
			flags |= GFP_KERNEL;
		gACDataAddrList[i].size = (EIP97_CMDRING_WORDSIZE/EIP97_CMDDESC_WORDOFFSET)*16*4;
		gACDataAddrList[i].addr = (uint32_t *) dma_alloc_coherent(NULL, gACDataAddrList[i].size, &gACDataAddrList[i].phyAddr, flags);
		if (unlikely(gACDataAddrList[i].addr  == NULL))
		{
			printk("ACDataAddrList failed\n");
			dma_free_coherent(NULL, gACDataAddrList[i].size, gACDataAddrList[i].addr, gACDataAddrList[i].phyAddr);
			dma_free_coherent(NULL, gACDataAddrList[i].size , gACDataAddrList[i].addr , gACDataAddrList[i].phyAddr);
		}
		memset(gACDataAddrList[i].addr, 0, gACDataAddrList[i].size);
		printk("gACDataAddrList[%d] [0x%08X 0x%08X]\n",i, gACDataAddrList[i].addr,gACDataAddrList[i].phyAddr );		
	}
	Data = (*((volatile u32 *)(0xfb000014)));
	Data |= (1<<7);
	(*((volatile u32 *)(0xfb000014))) = Data;

#ifdef DRIVER_INTERRUPTS
#ifdef WORKQUEUE_BH
	INIT_WORK(&mtk_interrupt_BH_result0_wq, mtk_BH_resultGet0);
#if (DDK_PEC_IF_ID > 1)	
	INIT_WORK(&mtk_interrupt_BH_result1_wq, mtk_BH_resultGet1);
	INIT_WORK(&mtk_interrupt_BH_result2_wq, mtk_BH_resultGet2);
	INIT_WORK(&mtk_interrupt_BH_result3_wq, mtk_BH_resultGet3);
#endif	
#else
	tasklet_init(&mtk_interrupt_BH_result0_tsk, mtk_BH_handler_resultGet , 0);
#if (DDK_PEC_IF_ID > 1)	
	tasklet_init(&mtk_interrupt_BH_result1_tsk, mtk_BH_handler_resultGet , 1);
	tasklet_init(&mtk_interrupt_BH_result2_tsk, mtk_BH_handler_resultGet , 2);
	tasklet_init(&mtk_interrupt_BH_result3_tsk, mtk_BH_handler_resultGet , 3);
#endif	
#endif
	//eip97 interrupt mode init
	Adapter_Interrupt_SetHandler(IRQ_RDR0, mtk_interruptHandler0_done);
	Adapter_Interrupt_Enable(IRQ_RDR0, 0);
#if (DDK_PEC_IF_ID > 1)	
	Adapter_Interrupt_SetHandler(IRQ_RDR1, mtk_interruptHandler1_done);
	Adapter_Interrupt_Enable(IRQ_RDR1, 0);
	Adapter_Interrupt_SetHandler(IRQ_RDR2, mtk_interruptHandler2_done);
	Adapter_Interrupt_Enable(IRQ_RDR2, 0);
	Adapter_Interrupt_SetHandler(IRQ_RDR3, mtk_interruptHandler3_done);
	Adapter_Interrupt_Enable(IRQ_RDR3, 0);
#endif
#else	
#endif	
	
}

void 
mtk_ipsec_release(
	void
)
{
	int i;
	for (i = 0 ; i < DDK_PEC_IF_ID ; i++)
	{
		printk("gACDataAddrList[%d] free [0x%08X 0x%08X]\n",i, gACDataAddrList[i].addr,gACDataAddrList[i].phyAddr );    
		if (unlikely(gACDataAddrList[i].addr  != NULL))
		{
			dma_free_coherent(NULL, gACDataAddrList[i].size, gACDataAddrList[i].addr, gACDataAddrList[i].phyAddr);
		}		
	}
}


void mtk_BH_resultGet0(
)
{
	unsigned int reg;
	unsigned int data = 0;
	ipsec_BH_handler_resultGet(data);

	reg = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	reg &= ~0x3fffff;
	reg |= (1<<23)|(0x1);
	mcrypto_iowrite32(reg, pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	Adapter_Interrupt_Enable(((data+2)<<1), 0);
}

void mtk_BH_resultGet1(
)
{
	unsigned int reg;
	unsigned int data = 1;
	ipsec_BH_handler_resultGet(data);

	reg = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	reg &= ~0x3fffff;
	reg |= (1<<23)|(0x1);
	mcrypto_iowrite32(reg, pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	Adapter_Interrupt_Enable(((data+2)<<1), 0);
}
void mtk_BH_resultGet2(
)
{
	unsigned int reg;
	unsigned int data = 2;
	ipsec_BH_handler_resultGet(data);

	reg = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	reg &= ~0x3fffff;
	reg |= (1<<23)|(0x1);
	mcrypto_iowrite32(reg, pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	Adapter_Interrupt_Enable(((data+2)<<1), 0);
}	

void mtk_BH_resultGet3(
)
{
	unsigned int reg;
	unsigned int data = 3;
	ipsec_BH_handler_resultGet(data);

	reg = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	reg &= ~0x3fffff;
	reg |= (1<<23)|(0x1);
	mcrypto_iowrite32(reg, pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	Adapter_Interrupt_Enable(((data+2)<<1), 0);
}	
void mtk_BH_handler_resultGet(
	unsigned long data
)
{
	unsigned int reg;

	ipsec_BH_handler_resultGet(data);

	reg = mcrypto_ioread32(pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	reg &= ~0x3fffff;
	reg |= (1<<23)|(0x1);
	mcrypto_iowrite32(reg, pEip97RegBase + ((HIA_RDR_y_THRESH+0x1000*data)>>2));
	Adapter_Interrupt_Enable(((data+2)<<1), 0);
}

static void mtk_interruptHandler0_done(void)
{
#ifdef WORKQUEUE_BH
	schedule_work(&mtk_interrupt_BH_result0_wq);
#else	
	tasklet_hi_schedule(&mtk_interrupt_BH_result0_tsk);
#endif
}

static void mtk_interruptHandler1_done(void)
{
#ifdef WORKQUEUE_BH
	schedule_work(&mtk_interrupt_BH_result1_wq);
#else	
	tasklet_hi_schedule(&mtk_interrupt_BH_result1_tsk);
#endif
}

static void mtk_interruptHandler2_done(void)
{
#ifdef WORKQUEUE_BH
	schedule_work(&mtk_interrupt_BH_result2_wq);
#else	
	tasklet_hi_schedule(&mtk_interrupt_BH_result2_tsk);
#endif
}

static void mtk_interruptHandler3_done(void)
{
#ifdef WORKQUEUE_BH
	schedule_work(&mtk_interrupt_BH_result3_wq);
#else	
	tasklet_hi_schedule(&mtk_interrupt_BH_result3_tsk);
#endif
}

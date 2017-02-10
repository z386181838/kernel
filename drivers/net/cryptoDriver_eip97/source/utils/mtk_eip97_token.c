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


#include "basic_defs.h"
#include "cs_token_builder.h"
#include "sa_builder.h"
#include "sa_builder_basic.h"
#include "sa_builder_ipsec.h"
#include "sa_builder_params_ipsec.h"
#include "token_builder.h"
#include "mtk_esp.h"

#ifdef MCRYPTO_DBG
#define ra_dbg 	printk
#else
#define ra_dbg(fmt, arg...) do {}while(0)
#endif
/* Specific negative test cases, force a specific error that has to be
   detected by the API */
#define TEST_ERROR_KEY_SIZE 1
#define TEST_ERROR_KEY_NULL 2
#define TEST_ERROR_AUTH1_NULL 3
#define TEST_ERROR_AUTH2_NULL 4
#define TEST_ERROR_SAPARMS_NULL_GETSIZE 5
#define TEST_ERROR_SAPARMS_NULL_BUILDSA 6
#define TEST_ERROR_PROTO 7
#define TEST_ERROR_INIT_NULL 8
#define TEST_ERROR_SETPARAMS_NULL 9
#define TEST_ERROR_NONCE_NULL 10
#define TEST_ERROR_IV_NULL 11
#define TEST_ERROR_TKBCTXGETSIZE_NULL 12
#define TEST_ERROR_BUILDCTX_NULL 13
#define TEST_ERROR_TKBGETSIZE_NULL 14
#define TEST_ERROR_BUILDTOKEN_NULL 15
#define TEST_ERROR_PACKET_SIZE 16
#define TEST_ERROR_AAD_SIZE 17


#define WORDSWAP(a)     	((((a)>>24)&0xff) | (((a)>>8)&0xff00) | (((a)<<8)&0xff0000) | (((a)<<24)&0xff000000))

const static uint8_t ExampleSHA1InnerDigest[] =
{
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0x73, 0x74
};

const static uint8_t ExampleSHA1OuterDigest[] =
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39
};

static int idx = 0;
static dma_addr_t AuthKey1_PhyAddr[IPESC_EIP93_ADAPTERS*2];
static dma_addr_t AuthKey2_PhyAddr[IPESC_EIP93_ADAPTERS*2];
static uint8_t* AuthKey1[IPESC_EIP93_ADAPTERS*2];
static uint8_t* AuthKey2[IPESC_EIP93_ADAPTERS*2];

extern uint32_t EIP97_CMDRING_WORDSIZE ;
extern uint32_t EIP97_CMDDESC_WORDOFFSET ;

/*----------------------------------------------------------------------------
 * TestBasicAlgo
 *
 * Create an SA for ESP with the specified properties, then use the
 * token builder to build tokens for various packet sizes. Display a
 * hex dump of the created SA and each of the generated tokens.
 * Destroy all generated data structures before returning.
 *
 * direction (input):
 *    SAV_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND
 * crypto (input)
 *    Crypto algorithm, must be SAB_CRYPTO_NULL, SAB_CRYPTO_AES
 *    or SAB_CRYPTO_ARCFOUR.
 * CryptoMode (input)
 *    Crypto mode to use, must be SAB_CRYPTO_MODE_CBC, SAB_CRYPTO_MODE_CTR,
 *    SAB_CRYPTO_MODE_ECB, SAB_CRYPTO_MODE_STATELESS or
 *    SAB_CRYPTO_MODE_STATEFUL.
 * auth (input).
 *    Authentication algorithm, SAB_AUTH_NULL, SAB_AUTH_HMAC_SHA1 or
 *    SAB_AUTH_HASH_SHA1.
 * IVSrc (input)
 *    IV source, SAB_IV_SRC_INPUT, SAB_IV_SRC_SA or SAB_IV_SRC_TOKEN.
 * ExtAAD (input)
 *    flag that indicates that AAD data will be external.
 * CopyIV (input)
 *    flag that indicates whether the IV must be copied to the output packet.
 * CheckHash (input)
 *    flag that indicates whether the hash (appended to the input packet) must
 *    be extracted and checked.
 * SaveState (input)
 *    flag that indicates whether the hash state should be saved
 */
 
int PreComputeDigestToken(ipsecEip93Adapter_t *currAdapterPtr, unsigned int dir, int datalen, int inDigest)
{
    int rc;
    SABuilder_Params_t params;
    SABuilder_Params_Basic_t BasicParams;
    unsigned int SAWords=0,ARC4Words=0;
    unsigned int *SAData=0, *ARC4Data=0;
    unsigned int TCRWords=0;
    void * TCRData;
    unsigned int TokenWords=0;
    unsigned int TokenHeaderWord=0;
    unsigned int TokenMaxWords=0;
    unsigned int *TokenData;
    unsigned int i,j;
	addrsDigestPreCompute_t* addrsPreCompute = currAdapterPtr->addrsPreCompute;
	SABuilder_Direction_t direction = SAB_DIRECTION_OUTBOUND;
    SABuilder_Crypto_t crypto = SAB_CRYPTO_NULL;
    SABuilder_Crypto_Mode_t CryptoMode = SAB_CRYPTO_MODE_ECB;
    SABuilder_Auth_t auth = SAB_AUTH_NULL;
    SABuilder_IV_Src_t IVSrc = SAB_IV_SRC_INPUT;
    bool ExtAAD = false;
    bool CopyIV = false;
    bool CheckHash = false;
    bool SaveState= true;
    unsigned int ParameterError = 0;
	dma_addr_t SADataPhyAddr, ARC4DataPhyAddr, TokenDataPhyAddr;
	dma_addr_t SAPoolPhyAddr;
	unsigned int *SAPool=0;
    static int TestNr=1;
	unsigned int flags = 0;	
	if (dir == HASH_DIGEST_OUT)
		direction = SAB_DIRECTION_OUTBOUND;
	else
		direction = SAB_DIRECTION_INBOUND;

	if (addrsPreCompute->digestWord == 4)
		auth = SAB_AUTH_HASH_MD5; //md5
	else if (addrsPreCompute->digestWord == 5)
		auth = SAB_AUTH_HASH_SHA1; //sha1
	else if (addrsPreCompute->digestWord == 8)
		auth = SAB_AUTH_HASH_SHA2_256; //sha256
	else if (addrsPreCompute->digestWord == 0)
		auth = SAB_AUTH_NULL;; //null
			
    ra_dbg("\nTest #%d: %s %s %s %s %s %s %s %s %s\n",
           TestNr,
           direction==SAB_DIRECTION_OUTBOUND?"Outbound":
           (direction==SAB_DIRECTION_INBOUND?"Inbound":"unknown"),
           crypto==SAB_CRYPTO_AES?"AES":
           (crypto==SAB_CRYPTO_ARCFOUR?"ARCFOUR":
            (crypto==SAB_CRYPTO_NULL?"Nullcrypto":
             (crypto==SAB_CRYPTO_DES?"DES":"unknown"))),

           CryptoMode==SAB_CRYPTO_MODE_CBC?"CBC":
           (CryptoMode==SAB_CRYPTO_MODE_CTR?"CTR":
            (CryptoMode==SAB_CRYPTO_MODE_STATELESS?"Stateless":
             (CryptoMode==SAB_CRYPTO_MODE_STATEFUL?"Stateful":
              (CryptoMode==SAB_CRYPTO_MODE_CCM?"CCM":
               (CryptoMode==SAB_CRYPTO_MODE_GCM?"GCM":
                (CryptoMode==SAB_CRYPTO_MODE_GMAC?"GMAC":
                 (CryptoMode==SAB_CRYPTO_MODE_ECB?"ECB":"unknown"))))))),
           auth==SAB_AUTH_HMAC_SHA1?"HMAC-SHA1":
           (auth==SAB_AUTH_NULL?"Nullauth":
            (auth==SAB_AUTH_HASH_SHA1?"Hash-SHA1":"combined")),
           IVSrc==SAB_IV_SRC_INPUT?"IV-INPUT":
           (IVSrc==SAB_IV_SRC_SA?"IV-SA":"IV-TOKEN"),
           SaveState?"With hash save state":"",
           CopyIV?"with copy IV":"",
           CheckHash?"extract and check hash":"",
           ExtAAD?" external AAD":"");
    if (ParameterError!=0)
    {
        ra_dbg("With specific parameter error\n");
    }
    TestNr++;

    rc=SABuilder_Init_Basic(ParameterError==TEST_ERROR_INIT_NULL?NULL:&params,
                            &BasicParams, direction);
    ra_dbg("SABuilder_Init_Basic returned %d\n",rc);
    if (rc!=0)
        return;

    params.IVSrc = IVSrc;
    params.flags |= SAB_FLAG_SUPPRESS_PAYLOAD;

    if(SaveState)
        params.flags |= SAB_FLAG_HASH_SAVE;

    {
        params.CryptoAlgo = crypto;
    }

    ra_dbg("Setting Auth algo and keys\n");
    
	params.AuthAlgo = auth;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	params.AuthKey1_p = ExampleSHA1InnerDigest;
	params.AuthKey2_p = ExampleSHA1OuterDigest;
#else	
	flags = 0;
	if (in_atomic())
		flags |= GFP_ATOMIC;
	else
		flags |= GFP_KERNEL;
		
	AuthKey1[idx] =  kmalloc(40, flags);
	AuthKey2[idx] =  AuthKey1[idx] +  20;
		
	memcpy(AuthKey1[idx], ExampleSHA1InnerDigest, 20);
	memcpy(AuthKey2[idx], ExampleSHA1OuterDigest, 20);
		
	params.AuthKey1_p = AuthKey1[idx];
	params.AuthKey2_p = AuthKey2[idx];
		
	//idx++;
	if (idx==32)
	{	
		idx = 0;
    	if (AuthKey1[idx])
		{	
			kfree(AuthKey1[idx]);
				
			AuthKey1[idx] = AuthKey2[idx] = NULL;
			AuthKey1_PhyAddr[idx] = AuthKey2_PhyAddr[idx] = NULL;
		}
    }
#endif
    if (CopyIV)
        params.flags |= SAB_FLAG_COPY_IV;

    if (CheckHash)
        BasicParams.BasicFlags |= SAB_BASIC_FLAG_EXTRACT_ICV;
		
    if(ParameterError==TEST_ERROR_SETPARAMS_NULL)
        params.ProtocolExtension_p = NULL;

    rc=SABuilder_GetSizes(ParameterError==TEST_ERROR_SAPARMS_NULL_GETSIZE?NULL:&params,
                          &SAWords, NULL, &ARC4Words);

    ra_dbg("SABuilder_GetSizes returned %d SA size=%u words ARC4=%u words\n",rc,SAWords,ARC4Words);
    if (rc!=0)
    {
        printk("SA not created because of errors\n");
        return;
    }

    SAData = addrsPreCompute->RecPoolHandler.addr + (1024*inDigest + SA_OFFSET);
    SADataPhyAddr = addrsPreCompute->RecPoolHandler.phyAddr + (1024*inDigest + SA_OFFSET);   

    if (!SAData)
    {
        printk("Malloc failed, bummer\n");
        return;
    }
    rc=SABuilder_BuildSA(ParameterError==TEST_ERROR_SAPARMS_NULL_BUILDSA?NULL:&params,
                         SAData, NULL, ARC4Data);

    ra_dbg("SABuilder_BuildSA returned %d\n",rc);
    if (rc != 0)
    {
        printk("SA not created because of errors\n");
        return;
    }
	
    ra_dbg("PreCompute Digest SA Data:[%08X %08X]\n",SAData ,SADataPhyAddr);
    for(i=0; i<SAWords; i++)
    {
        ra_dbg("0x%08X ",SAData[i]);
        if (i%4==3) ra_dbg("\n");
        if (i==SAWords-1) ra_dbg("}\n");
    }

    rc=TokenBuilder_GetContextSize(ParameterError==TEST_ERROR_TKBCTXGETSIZE_NULL?NULL:&params, &TCRWords);

    ra_dbg("TokenBuilder_GetContextSize returned %d context size %d\n",
           rc,TCRWords);
    if (rc != 0)
    {
        return -1;
    }
	flags = 0;
	if (in_atomic())
		flags |= GFP_ATOMIC;
	else
		flags |= GFP_KERNEL;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	TCRData = addrsPreCompute->LocalPool + PRECOMPUTE_TCRDATA_OFFSET;
	memset(TCRData, 0, 4*TCRWords);
#else
    TCRData = kzalloc(4*TCRWords, flags);
    if (!TCRData)
    {
        printk("Malloc failed, bummer\n");
        return -1;
    }
#endif	
    rc = TokenBuilder_BuildContext(ParameterError==TEST_ERROR_BUILDCTX_NULL?NULL:&params, TCRData);

    ra_dbg("TokenBuilder_BuildContext returned %d\n",rc);
    if (rc != 0)
    {
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
        kfree(TCRData);
#endif
        return -1;
    }

    rc = TokenBuilder_GetSize(ParameterError==TEST_ERROR_TKBGETSIZE_NULL?NULL:TCRData, &TokenMaxWords);
    ra_dbg("TokenBuilder_GetSize returned %d size=%d\n",rc,TokenMaxWords);
    if (rc != 0)
    {
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
        kfree(TCRData);
#endif
        return -1;
    }
		
	TokenData = addrsPreCompute->RecPoolHandler.addr + (1024*inDigest + TOKEN_OFFSET);
	TokenDataPhyAddr = addrsPreCompute->RecPoolHandler.phyAddr + (1024*inDigest + TOKEN_OFFSET);

		
    if (!TokenData)
    {
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
        kfree(TCRData);
        printk("Malloc failed, bummer\n");
#endif
        return -1;
    }
    memset((unsigned char *)TokenData, 0, 4*TokenMaxWords);
	ra_dbg("PreComput : TokenMaxWords = %d\n",TokenMaxWords);

    TokenBuilder_Params_t TokenParams;
	ra_dbg("Token for packet of size %d\n",datalen);

    TokenParams.PacketFlags = 0;

    TokenParams.BypassByteCount = 0;

    if (crypto==SAB_CRYPTO_ARCFOUR && CryptoMode==
        SAB_CRYPTO_MODE_STATEFUL)
        TokenParams.PacketFlags |= TKB_PARCKET_FLAG_ARC4_INIT;

    TokenParams.PacketFlags |= TKB_PACKET_FLAG_HASHFIRST;

    TokenParams.IV_p = NULL;
    TokenParams.AAD_p = NULL;

    if(datalen==12)
        TokenParams.AdditionalValue = 0;
    else if (ParameterError==TEST_ERROR_AAD_SIZE && datalen > 64)
        TokenParams.AdditionalValue = 65;
    else
        TokenParams.AdditionalValue = 16;
        
    rc = TokenBuilder_BuildToken(TCRData,
                                 addrsPreCompute->ipadHandler.addr,
                                 datalen,
                                 &TokenParams,
                                 TokenData,
                                 &TokenWords,
                                 &TokenHeaderWord);
    ra_dbg("TokenBuilder_BuildToken returned %d, size %d, header word %08x\n",
           rc,TokenWords,TokenHeaderWord);

    if(rc!=TKB_STATUS_OK)
    {
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
    	kfree(TCRData);
#endif
        return -1;
    }

    TokenData[TokenWords] = 0xF0000000;

    ra_dbg("Token data [0x%08X 0x%08X]:\n",TokenData,TokenDataPhyAddr );

	for(j=0; j<TokenWords+1; j++)
    {
        ra_dbg("0x%08X ",TokenData[j]);
        if (j%4==3) ra_dbg("\n");
        if (j==TokenWords) ra_dbg("}\n");
    }
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
    kfree(TCRData);
#endif    
    if (inDigest)
    {	
    	eip97DescpHandler_t* cmdHandler = addrsPreCompute->cmdHandler;
    	cmdHandler->ACDataAddr.addr = TokenData;
    	cmdHandler->ACDataAddr.phyAddr = TokenDataPhyAddr;
    	cmdHandler->ACDataAddr.size = TokenMaxWords*4;
		cmdHandler->TokenHeaderWord = TokenHeaderWord;
		cmdHandler->TokenWords = TokenWords;
    	addrsPreCompute->ACDataAddr.addr = TokenData;
    	addrsPreCompute->ACDataAddr.phyAddr = TokenDataPhyAddr;
    	addrsPreCompute->ACDataAddr.size = TokenMaxWords*4;
    	addrsPreCompute->saHandler.addr = SAData;
    	addrsPreCompute->saHandler.phyAddr = SADataPhyAddr;
    	addrsPreCompute->saHandler.size = SAWords*4;
	}
	else
	{
			eip97DescpHandler_t* cmdHandler = addrsPreCompute->cmdHandler;
    	cmdHandler->ACDataAddr.addr = TokenData;
    	cmdHandler->ACDataAddr.phyAddr = TokenDataPhyAddr;
    	cmdHandler->ACDataAddr.size = TokenMaxWords*4;
    	cmdHandler->TokenHeaderWord = TokenHeaderWord;
			cmdHandler->TokenWords = TokenWords;
    	addrsPreCompute->ACDataAddr2.addr = TokenData;
    	addrsPreCompute->ACDataAddr2.phyAddr = TokenDataPhyAddr;
    	addrsPreCompute->ACDataAddr2.size = TokenMaxWords*4;
    	addrsPreCompute->saHandler2.addr = SAData;
    	addrsPreCompute->saHandler2.phyAddr = SADataPhyAddr;
    	addrsPreCompute->saHandler2.size = SAWords*4;
	}
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)		
	if (AuthKey1[idx])
	{
		kfree(AuthKey1[idx]);
	}
#endif
	return 0;
}

int BuildESPToken(ipsecEip93Adapter_t *currAdapterPtr, 	unsigned int dir, unsigned int cipherAlg, \
									unsigned int hashAlg, unsigned int digestWord, unsigned int cipherMode,	unsigned int *cipherKey, \
									unsigned int keyLen)
{
    int rc,j,i;
    SABuilder_Params_t params;
    SABuilder_Params_IPsec_t IPsecParams;
    unsigned int SAWords=0;
    unsigned int *TokenData;
	unsigned int *SAData, *SAPool;
	dma_addr_t SAPoolPhyAddr, SADataPhyAddr, ARC4DataPhyAddr, TokenDataPhyAddr;
    unsigned int TCRWords=0;
    void * TCRData=0;
    unsigned int TokenWords=0;
    unsigned int TokenHeaderWord;
    unsigned int TokenMaxWordsOut=0,TokenMaxWordsIn=0;

    TokenBuilder_Params_t TokenParams;
	unsigned int packet_length = 0;
    unsigned int PktByteCount;
	unsigned int ipsec_mode = SAB_IPSEC_TUNNEL;
	eip97DescpHandler_t *cmdHandler = (eip97DescpHandler_t *)currAdapterPtr->cmdHandler;
	addrsDigestPreCompute_t* addrsPreCompute = currAdapterPtr->addrsPreCompute;
	unsigned int flags = 0;	
	unsigned int icv_len,iv_len;
	if (currAdapterPtr->tunnel==0)
	{
		ra_dbg("==ipsec_mode : SAB_IPSEC_TRANSPORT==\n");
		ipsec_mode = SAB_IPSEC_TRANSPORT;
    }
    // First prepare the SA and Token Context Record for the Outbound SA.

	memset(&params, 0, sizeof(SABuilder_Params_t));
    memset(&IPsecParams, 0, sizeof(SABuilder_Params_IPsec_t));
    memset(&TokenParams, 0, sizeof(TokenBuilder_Params_t));
    rc=SABuilder_Init_ESP(&params, &IPsecParams, WORDSWAP(currAdapterPtr->spi),  ipsec_mode,
                          SAB_IPSEC_IPV4, ((dir==HASH_DIGEST_OUT) ? SAB_DIRECTION_OUTBOUND : SAB_DIRECTION_INBOUND));

    if (rc!=0)
    {
        printk("SABuilder_Init_ESP failed\n");
        goto error_exit;
    }

    // Add crypto key and parameters.
    switch(cipherAlg)
    {
    	case 0:
    				params.CryptoAlgo = SAB_CRYPTO_DES;
    				cmdHandler->blkSize = 8;
    				iv_len = 8;
    				break;
    	case 1:
    				params.CryptoAlgo = SAB_CRYPTO_3DES;
    				cmdHandler->blkSize = 8;
    				iv_len = 8;
    				break;
    	case 3:
    				params.CryptoAlgo = SAB_CRYPTO_AES;
    				cmdHandler->blkSize = 16;
    				iv_len = 16;
    				break;
    	case 0xf:
    				params.CryptoAlgo = SAB_CRYPTO_NULL;
    				cmdHandler->blkSize = 0;			
    				iv_len = 0;
    	default:
    				params.CryptoAlgo = SAB_CRYPTO_AES;
    				cmdHandler->blkSize = 16;
    				iv_len = 16;
    				break;
  	}
  	switch (cipherMode)
  	{	
    
    	case 0:
    				params.CryptoMode = SAB_CRYPTO_MODE_ECB;
    				break;
    	case 1:
	   				params.CryptoMode = SAB_CRYPTO_MODE_CBC;
						break;
			default:
						params.CryptoMode = SAB_CRYPTO_MODE_CBC;
						break;		
  	}
    params.KeyByteCount = keyLen;;
    params.Key_p = cipherKey;
    {
    	unsigned int * pData = cipherKey;
    	ra_dbg("CryptoKey[%d] = {",keyLen>>2);
		for(i=0; i<(keyLen>>2); i++)
	    {
	        ra_dbg("0x%08X ",pData[i]);
	        if (i%4==3) ra_dbg("\n");
	        if(i==(keyLen>>2)-1)
	        	ra_dbg("}\n");
	    }
  	}
    // Add authentication key and paramters.
    switch (hashAlg)
  	{	
    case 0:
    				params.AuthAlgo = SAB_AUTH_HMAC_MD5;
    				icv_len = 12;
    				break;
    case 1:
    				params.AuthAlgo = SAB_AUTH_HMAC_SHA1;
    				icv_len = 12;
    				break;
    case 3:
    				params.AuthAlgo = SAB_AUTH_HMAC_SHA2_256;
    				icv_len = 12;
    				break;
    case 0xf:
    				params.AuthAlgo = SAB_AUTH_NULL;
    				break;
    default:
    				params.AuthAlgo = SAB_AUTH_HMAC_SHA1;
    				icv_len = 12;
						break;
		}
	packet_length = cmdHandler->blkSize*10+8+icv_len+iv_len;					
    cmdHandler->KeySizeDW = keyLen>>2;
    cmdHandler->digestWord = digestWord;		

	params.AuthKey1_p = cmdHandler->pIDigest;
    params.AuthKey2_p = cmdHandler->pODigest;

	params.flags |= SAB_FLAG_HASH_LOAD;
    rc=SABuilder_GetSizes(&params, &SAWords, NULL, NULL);
    if (rc!=0)
    {
        printk("SA not created because of errors\n");
        goto error_exit;
    }
	ra_dbg("[CMD]SABuilder_GetSizes returned %d SA size=%u words\n",rc,SAWords);

	flags = 0;
	if (in_atomic())
		flags |= GFP_ATOMIC;
	else
		flags |= GFP_KERNEL;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	SAPool = cmdHandler->saAddr.addr;
	SAPoolPhyAddr = cmdHandler->saAddr.phyAddr;
#else
	SAPool = (uint32_t *) dma_alloc_coherent(NULL, SAPOOLSIZE, &SAPoolPhyAddr, flags);
#endif
	SAData = SAPool;
	SADataPhyAddr = SAPoolPhyAddr;
	if (unlikely(SAData == NULL))
	{
		goto error_exit;
	}		
	memset(SAData, 0, SAWords*4);
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	cmdHandler->saAddr.addr = (unsigned int)SAData;
	cmdHandler->saAddr.phyAddr = SADataPhyAddr;
#endif
	cmdHandler->saAddr.size = SAWords*4;
	rc=SABuilder_BuildSA(&params, (uint32_t *)SAData, NULL, NULL);
    if (rc != 0)
    {
        printk("SA not created because of errors\n");
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
        dma_free_coherent(NULL, SAWords*4, SAData, SADataPhyAddr);
#endif
        goto error_exit;
    }

	ra_dbg("SA Data[%d]:[%08X %08X] = {\n",SAWords,SAData ,SADataPhyAddr);
    for(i=0; i<SAWords; i++)
    {
        ra_dbg("0x%08X ",SAData[i]);
        if (i%4==3) 
        	ra_dbg("\n");
        if (i==SAWords-1) 
        	ra_dbg("}\n");
    }
    rc=TokenBuilder_GetContextSize(&params, &TCRWords);

    if (rc != 0)
    {
        printk("TokenBuilder_GetContextSize returned errors\n");
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
        dma_free_coherent(NULL, SAPOOLSIZE, SAPool, SAPoolPhyAddr);
#endif
        goto error_exit;
    }

    // The Token Context Record does not need to be allocated
    // in a DMA-safe buffer.
	flags = 0;
	if (in_atomic())
		flags |= GFP_ATOMIC;
	else
		flags |= GFP_KERNEL;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	TCRData = cmdHandler->LocalPool + CMD_TCRDATA_OFFSET;
	memset(TCRData, 0, 4*TCRWords);
#else
    TCRData = kzalloc(4*TCRWords, flags);
    if (!TCRData)
    {
        rc = 1;
        printk("Allocation of outbound TCR failed\n");
        kfree(TCRData);
        dma_free_coherent(NULL, SAPOOLSIZE, SAPool, SAPoolPhyAddr);
        goto error_exit;
    }
#endif
    rc = TokenBuilder_BuildContext(&params, TCRData);

    if (rc != 0)
    {
        printk("TokenBuilder_BuildContext failed\n");
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
        kfree(TCRData);
				dma_free_coherent(NULL, SAPOOLSIZE, SAPool, SAPoolPhyAddr);
#endif
        goto error_exit;
    }

    rc = TokenBuilder_GetSize(TCRData, &TokenMaxWordsOut);
    if (rc != 0)
    {
        printk("TokenBuilder_GetSize failed\n");
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
        kfree(TCRData);
        dma_free_coherent(NULL, SAPOOLSIZE, SAPool, SAPoolPhyAddr);
#endif
        goto error_exit;
    }
    ra_dbg("TokenBuilder_GetSize returned %d size=%d\n",rc,TokenMaxWordsOut);

    // Allocate one buffer for the token and three packet buffers.
	flags = 0;
	if (in_atomic())
			flags |= GFP_ATOMIC;
	else
		flags |= GFP_KERNEL;

	TokenData = SAPool + (256>>2);
	TokenDataPhyAddr = SAPoolPhyAddr + 256;
	if (unlikely(TokenData == NULL))
	{
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
		kfree(TCRData);
		dma_free_coherent(NULL, SAPOOLSIZE, SAPool, SAPoolPhyAddr);
#endif
		goto error_exit;
	}		
	memset(TokenData, 0, 4*MAX(TokenMaxWordsOut, TokenMaxWordsIn)+16);
		
	ra_dbg("BuildESPToken : TokenMaxWords = %d\n",MAX(TokenMaxWordsOut, TokenMaxWordsIn)+4);
		
	cmdHandler->ACDataAddr.addr = (unsigned int)TokenData;
	cmdHandler->ACDataAddr.phyAddr = TokenDataPhyAddr;
	cmdHandler->ACDataAddr.size = 4*MAX(TokenMaxWordsOut, TokenMaxWordsIn)+16;

	cmdHandler->pTCRData = TCRData;
    	
    TokenParams.PacketFlags = 0;
    
    TokenParams.PadByte = 0x04;
    TokenParams.IV_p = NULL;
    TokenParams.BypassByteCount = 0;
    TokenParams.AdditionalValue = 0;
	rc = TokenBuilder_BuildToken(TCRData,
                                     (uint8_t*)0xF0000000,
                                     ((dir==HASH_DIGEST_OUT) ? 64 : packet_length) /*256*/,
                                     &TokenParams,
                                     (uint32_t*)TokenData,
                                     &TokenWords,
                                     &TokenHeaderWord);
    ra_dbg("TokenBuilder_BuildToken returned %d, size %d, header word %08x\n",
               rc,TokenWords,TokenHeaderWord);                                 
    if(rc != TKB_STATUS_OK)
    {
        if(rc == TKB_BAD_PACKET)
        {
           printk("Token not created because packet size is invalid\n");
        }
        else
        {
            printk("Token builder failed\n");
        }
        goto error_exit;
    }
    
    TokenData[TokenWords] = 0xF0000000;
    ra_dbg("Token data[%d] [0x%08X 0x%08X]:\n",TokenWords,TokenData,TokenDataPhyAddr );

	for(j=0; j<TokenWords+1; j++)
    {
        ra_dbg("0x%08X ",TokenData[j]);
        if (j%4==3)  ra_dbg("\n");
        if(j==TokenWords) ra_dbg("}\n");
    }

	cmdHandler->TokenHeaderWord = TokenHeaderWord;
	cmdHandler->TokenWords = TokenWords;

error_exit:
    return rc == 0;		
	
}	


#ifndef EIP97_ADAPTER_INTERNAL_H
#define EIP97_ADAPTER_INTERNAL_H

#include "basic_defs.h"
#include "clib.h"
//#include "mtk_hwDmaAccess.h"          // HWPAL_DMAResource_t
//#include "mtk_dmaBuf.h"             // DMABuf_Handle_t
//#include "mtk_eip93.h"                // EIP93_IOArea_t
//#include "c_adapter.h"
#include "mtk_esp.h"

/*----------------------------------------------------------------------------
 *                           Implementation helper macros
 *----------------------------------------------------------------------------
 */

#define ZEROINIT(_x)  memset(&_x, 0, sizeof(_x))



/*----------------------------------------------------------------------------
 *                           Adapter_EIP93
 *----------------------------------------------------------------------------
 */

//extern EIP93_IOArea_t Adapter_EIP93_IOArea;
//extern unsigned int Adapter_EIP93_MaxDescriptorsInRing;

extern int
Driver97_Init(void);

extern void
Driver97_Exit(void);

extern int PreComputeDigestToken(ipsecEip93Adapter_t *currAdapterPtr, unsigned int dir, int datalen, int inDigest);
extern int BuildESPToken(ipsecEip93Adapter_t *currAdapterPtr, 	unsigned int dir, unsigned int cipherAlg, \
									unsigned int hashAlg, unsigned int digestWord, unsigned int cipherMode,	unsigned int *cipherKey, \
									unsigned int keyLen);

//bool
//Adapter_EIP93_Init(void);

//bool
//Adapter_EIP93_SetMode_Idle(void);

//bool
//Adapter_EIP93_SetMode_ARM(
//        const bool fEnableDynamicSA);


//void
//Adapter_EIP93_UnInit(void);


//void
//Adapter_GetEIP93PhysAddr(
//        DMABuf_Handle_t Handle,
//        HWPAL_DMAResource_Handle_t * const DMAHandle_p,
//        EIP93_DeviceAddress_t * const EIP93PhysAddr_p);

//#ifdef MTK_CRYPTO_DRIVER
extern void mtk_interruptHandler_descriptorDone(void);
extern void mtk_BH_handler_resultGet(unsigned long data);
//#endif

/*----------------------------------------------------------------------------
 * Adapter_EIP93_InterruptHandler_DescriptorDone
 *
 * This function is invoked when the EIP93 has activated the descriptor done
 * interrupt.
 */


/*----------------------------------------------------------------------------
 *                           Adapter_Interrupts
 *----------------------------------------------------------------------------
 */




/*----------------------------------------------------------------------------
 *                           Adapter_DMABuf
 *----------------------------------------------------------------------------
 */

//#define ADAPTER_DMABUF_ALLOCATORREF_KMALLOC 'k'   /* kmalloc */


/*----------------------------------------------------------------------------
 *                           VTBAL Global device
 *----------------------------------------------------------------------------
 */
//extern void *  GlobalVTBALDevice ;

#endif 


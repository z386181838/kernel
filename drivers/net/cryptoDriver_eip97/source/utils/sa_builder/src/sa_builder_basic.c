/* sa_builder_basic.c
 *
 * Basic Crypto/hash specific functions (for initialization of
 * SABuilder_Params_t structures and for building the Basic Crypto/hash
 * specifc part of an SA.).
 */

/*****************************************************************************
* Copyright (c) 2011-2013 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://essoemsupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */
#include "sa_builder_basic.h"
#include "sa_builder_internal.h" /* SABuilder_SetBasicParams */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_sa_builder.h"
#include "basic_defs.h"
#include "log.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * SABuilder_Init_Basic
 *
 * This function initializes the SABuilder_Params_t data structure and
 * its SABuilder_Params_Basic_t extension with sensible defaults for
 * basic crypto and hash processing..
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsBasic_p (output)
 *   Pointer to Basic parameter extension to be filled in
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Both the crypto and the authentication algorithm are initialized to
 * NULL. Either the cipher algorithm or the authentication algorithm
 * (but not both) must be set to one of the supported algorithms for
 * basic crypto or basic hash or HMAC. The crypto mode and IV source
 * can be specified as well.  Any required keys have to be specified
 * as well.
 *
 * Both the SAParams_p and SAParamsBasic_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsBasic_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_Basic(
    SABuilder_Params_t * const SAParams_p,
    SABuilder_Params_Basic_t * const SAParamsBasic_p,
     const SABuilder_Direction_t direction)
{
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL || SAParamsBasic_p == NULL)
    {
        LOG_CRIT("SABuilder_Init_Basic: NULL pointer parameter supplied.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (direction != SAB_DIRECTION_OUTBOUND &&
        direction != SAB_DIRECTION_INBOUND)
    {
        LOG_CRIT("SABuilder_Init_Basic: Invalid direction.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    SAParams_p->protocol = SAB_PROTO_BASIC;
    SAParams_p->direction = direction;
    SAParams_p->ProtocolExtension_p = (void*)SAParamsBasic_p;
    SAParams_p->flags = 0;

    SAParams_p->CryptoAlgo = SAB_CRYPTO_NULL;
    SAParams_p->CryptoMode = SAB_CRYPTO_MODE_CBC;
    SAParams_p->IVSrc = SAB_IV_SRC_DEFAULT;
    SAParams_p->KeyByteCount = 0;
    SAParams_p->Key_p = NULL;
    SAParams_p->IV_p = NULL;
    SAParams_p->Nonce_p = NULL;

    SAParams_p->AuthAlgo = SAB_AUTH_NULL;
    SAParams_p->AuthKey1_p = NULL;
    SAParams_p->AuthKey2_p = NULL;
    SAParams_p->AuthKey3_p = NULL;

    SAParams_p->OffsetARC4StateRecord = 0;
    SAParams_p->CW0 = 0;
    SAParams_p->CW1 = 0;
    SAParams_p->OffsetDigest0 = 0;
    SAParams_p->OffsetDigest1 = 0;
    SAParams_p->OffsetSeqNum = 0;
    SAParams_p->OffsetSeqMask = 0;
    SAParams_p->OffsetIV = 0;
    SAParams_p->OffsetIJPtr = 0;
    SAParams_p->OffsetARC4State = 0;
    SAParams_p->SeqNumWord32Count = 0;
    SAParams_p->SeqMaskWord32Count = 0;
    SAParams_p->IVWord32Count = 0;

    SAParamsBasic_p->BasicFlags = 0;
    SAParamsBasic_p->DigestBlockCount = 0;
    SAParamsBasic_p->ICVByteCount = 0;

    return SAB_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SABuilder_SetBasicParams
 *
 * Fill in Basic Crypto and hash-specific extensions into the SA.
 *
 * SAParams_p (input)
 *   The SA parameters structure from which the SA is derived.
 * SAState_p (input, output)
 *   Variables containing information about the SA being generated.
 * SABuffer_p (input, output).
 *   The buffer in which the SA is built. If NULL, no SA will be built, but
 *   state variables in SAState_p will still be updated.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when SAParams_p is invalid, or if any of
 *    the buffer arguments  is a null pointer while the corresponding buffer
 *    would be required for the operation.
 * SAB_UNSUPPORTED_FEATURE when SAParams_p describes an operations that
 *    is not supported on the hardware for which this SA builder
 *    is configured.
 */
SABuilder_Status_t
SABuilder_SetBasicParams(SABuilder_Params_t *const SAParams_p,
                          SABuilder_State_t * const SAState_p,
                          uint32_t * const SABuffer_p)
{
    SABuilder_Params_Basic_t *SAParamsBasic_p;
    SAParamsBasic_p = (SABuilder_Params_Basic_t *)
        (SAParams_p->ProtocolExtension_p);
    if (SAParamsBasic_p == NULL)
    {
        LOG_CRIT("SABuilder: Basic extension pointer is null\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Use one of the classic hash algorithms, possibly with
       encryption */
    if ((SAParams_p->AuthAlgo == SAB_AUTH_HASH_MD5 ||
         SAParams_p->AuthAlgo == SAB_AUTH_HASH_SHA1 ||
         SAParams_p->AuthAlgo == SAB_AUTH_HASH_SHA2_224 ||
         SAParams_p->AuthAlgo == SAB_AUTH_HASH_SHA2_256 ||
         SAParams_p->AuthAlgo == SAB_AUTH_HASH_SHA2_384 ||
         SAParams_p->AuthAlgo == SAB_AUTH_HASH_SHA2_512) &&
        (SAParams_p->flags & (SAB_FLAG_HASH_LOAD|SAB_FLAG_HASH_SAVE|
                              SAB_FLAG_HASH_INTERMEDIATE)) != 0)
    {
        /* We are doing basic hash (no HMAC) with storing the state.*/
        SAState_p->CW1 |= SAB_CW1_HASH_STORE | SAB_CW1_DIGEST_CNT;
        if(SABuffer_p != NULL)
            SABuffer_p[SAState_p->CurrentOffset] =
                SAParamsBasic_p->DigestBlockCount;
        SAState_p->CurrentOffset += 1;
    }  else if (SAParams_p->AuthAlgo != SAB_AUTH_NULL &&
                (SAParams_p->flags & (SAB_FLAG_HASH_SAVE)) != 0)
    {
        SAState_p->CW1 |= SAB_CW1_HASH_STORE;
    }

    if (SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL)
    {
        /* We are now doing basic encryption/decryption, possibly
           with hash.*/
        if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
            if (SAParams_p->AuthAlgo == SAB_AUTH_NULL)
                SAState_p->CW0 |= SAB_CW0_TOP_ENCRYPT;
            else if (SAParams_p->AuthAlgo==SAB_AUTH_AES_CCM ||
                     SAParams_p->AuthAlgo==SAB_AUTH_AES_GMAC)
                SAState_p->CW0 |= SAB_CW0_TOP_HASH_ENCRYPT;
            else
                SAState_p->CW0 |= SAB_CW0_TOP_ENCRYPT_HASH;
        else
            if (SAParams_p->AuthAlgo == SAB_AUTH_NULL)
                SAState_p->CW0 |= SAB_CW0_TOP_DECRYPT;
            else if (SAParams_p->AuthAlgo==SAB_AUTH_AES_CCM)
                SAState_p->CW0 |= SAB_CW0_TOP_DECRYPT_HASH;
            else
                SAState_p->CW0 |= SAB_CW0_TOP_HASH_DECRYPT;

        /* Check for prohibited algorithms and crypto modes. */
        if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CFB1 ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CFB8)
        {
            LOG_CRIT("SABuilder: crypto algorithm/mode not supported\n");
            return SAB_INVALID_PARAMETER;
        }

        if (SAParams_p->CryptoAlgo != SAB_CRYPTO_ARCFOUR &&
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_ECB)
        {
            /* For ARCFOUR and block ciphers in ECB mode we do not have an IV */
            if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT)
                SAParams_p->IVSrc = SAB_IV_SRC_INPUT;

            if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CTR ||
                SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM ||
                SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GMAC ||
                SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM)
            {
                if (SAParams_p->IVSrc == SAB_IV_SRC_TOKEN)
                {
                    SAState_p->CW1 &= ~0x7; // Clear crypto mode (CTR);
                    SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CTR_LOAD;
                    /* When the CTR mode IV is loaded from token, then
                       load all four IV words, including block counter */
                }
                else
                {   /* else add nonce to SA */
                    SAState_p->CW1 |= SAB_CW1_IV0;

#ifdef SAB_STRICT_ARGS_CHECK
                    if (SAParams_p->Nonce_p == NULL)
                    {
                        LOG_CRIT("SABuilder: NULL pointer nonce.\n");
                        return SAB_INVALID_PARAMETER;
                    }
#endif
                    if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
                    {
                        if (SABuffer_p != NULL)
                            SABuffer_p[SAState_p->CurrentOffset] =
                                (SAParams_p->Nonce_p[0] << 8)  |
                                (SAParams_p->Nonce_p[1] << 16) |
                                (SAParams_p->Nonce_p[2] << 24) | SAB_CCM_FLAG_L4;
                    }
                    else
                    {
                        SABuilderLib_CopyKeyMat(SABuffer_p,
                                                SAState_p->CurrentOffset,
                                                SAParams_p->Nonce_p, 4);
                    }
                    SAState_p->CurrentOffset +=1;
                }
                if (SAParams_p->IVSrc == SAB_IV_SRC_SA)
                {
                    SAState_p->CW1 |= SAB_CW1_IV_CTR | SAB_CW1_IV1 | SAB_CW1_IV2;
#ifdef SAB_STRICT_ARGS_CHECK
                    if (SAParams_p->IV_p == NULL)
                    {
                        LOG_CRIT("SABuilder: NULL pointer IV.\n");
                        return SAB_INVALID_PARAMETER;
                    }
#endif
                    SAParams_p->OffsetIV = SAState_p->CurrentOffset;
                    SAParams_p->IVWord32Count = 2;

                    SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                                            SAParams_p->IV_p, 8);
                    SAState_p->CurrentOffset += 2;
                }
                else
                {
                    SAState_p->CW1 |= SAB_CW1_IV_CTR;
                }
                if (SAParams_p->IVSrc != SAB_IV_SRC_TOKEN &&
                    SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
                {
                    /* Add 0 counter field (IV3) */
                    SAState_p->CW1 |= SAB_CW1_IV3;
                    if(SABuffer_p != NULL)
                        SABuffer_p[SAState_p->CurrentOffset] = 0;
                    SAState_p->CurrentOffset+=1;
                }
            }
            else
            {
                if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_ICM)
                {
                    SAState_p->CW1 &= ~0x7; // Clear crypto mode (CTR or ICM);
                    SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CTR_LOAD;
                    /* When the CTR mode IV is loaded from token, then
                       load all four IV words, including block counter */
                }

                SAState_p->CW1 |= SAB_CW1_IV_FULL;
                if (SAParams_p->IVSrc == SAB_IV_SRC_SA)
                {
                    SAState_p->CW1 |= SAB_CW1_IV0 | SAB_CW1_IV1;
                    if(SAState_p->IVWords == 4)
                        SAState_p->CW1 |= SAB_CW1_IV2 | SAB_CW1_IV3;
#ifdef SAB_STRICT_ARGS_CHECK
                    if (SAParams_p->IV_p == NULL)
                    {
                        LOG_CRIT("SABuilder: NULL pointer IV.\n");
                        return SAB_INVALID_PARAMETER;
                    }
#endif
                    SAParams_p->OffsetIV = SAState_p->CurrentOffset;
                    SAParams_p->IVWord32Count = SAState_p->IVWords;

                    SABuilderLib_CopyKeyMat(SABuffer_p,
                                            SAState_p->CurrentOffset,
                                            SAParams_p->IV_p,
                                            SAState_p->IVWords * 4);
                    SAState_p->CurrentOffset += SAState_p->IVWords;
                    SAState_p->CW1 |= SAB_CW1_CRYPTO_STORE;
                }
            }
        }
    }
    else
    {
        if (SAParams_p->AuthAlgo == SAB_AUTH_NULL)
        {
            LOG_CRIT("SABuilder:  Crypto and hash algorithm cannot both be null\n");
            return SAB_INVALID_PARAMETER;
        }
        /* Basic authentication, use inbound direction when verifying  */

    if ((SAParamsBasic_p->BasicFlags & SAB_BASIC_FLAG_EXTRACT_ICV) !=0)
      SAState_p->CW0 |= SAB_CW0_TOP_HASH_IN;
    else
      SAState_p->CW0 |= SAB_CW0_TOP_HASH_OUT;
    }

    return SAB_STATUS_OK;
}




/* end of file sa_builder_basic.c */

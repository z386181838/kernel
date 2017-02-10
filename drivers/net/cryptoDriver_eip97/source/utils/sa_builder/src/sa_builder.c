/* sa_builder.c
 *
 * Main implementation file of the EIP-96 SA builder.
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
#include "sa_builder.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_sa_builder.h"
#include "basic_defs.h"
#include "log.h"
#include "sa_builder_internal.h"

#ifdef SAB_ENABLE_IPSEC_EXTENDED
#include "sa_builder_extended_internal.h"
#endif

/*----------------------------------------------------------------------------
 * Local variables
 */
static const  uint8_t ZeroWords[32];


/*----------------------------------------------------------------------------
 * SABuilderLib_CopyKeyMat
 *
 * Copy a key into the SA
 *
 * Destination_p (input)
 *   Destination (word-aligned) of the SA record.
 * offset (input)
 *   Word offset of the key in the SA record where it must be stored.
 * Source_p (input)
 *   Source (byte aligned) of the data.
 * KeyByteCount (input)
 *   Size of the key in bytes.
 *
 * Destination_p is allowed to be a null pointer, in which case no key
 * will be written.
 */
void
SABuilderLib_CopyKeyMat(uint32_t * const Destination_p,
                        const unsigned int offset,
                        const uint8_t * const Source_p,
                        const unsigned int KeyByteCount)
{
    uint32_t *dst = Destination_p + offset;
    const uint8_t *src = Source_p;
    unsigned int i,j;
    uint32_t w;
    if (Destination_p == NULL)
        return;
    for(i=0; i<(KeyByteCount+3)/4; i++)
    {
        w=0;
        for(j=0; j<4; j++)
            w=(w>>8)|(*src++ << 24);
        *dst++ = w;
    }
}

/*----------------------------------------------------------------------------
 * SABuilderLib_CopyKeyMatSwap
 *
 * Copy a key into the SA with the words byte-swapped.
 *
 * Destination_p (input)
 *   Destination (word-aligned) to store the data.
 * offset (input)
 *   Word offset of the key in the SA record where it must be stored.
 * Source_p (input)
 *   Source (byte aligned) of the data.
 * KeyByteCount (input)
 *   Size of the key in bytes.
 *
 * Destination_p is allowed to be a null pointer, in which case no key
 * will be written.
 */
void
SABuilderLib_CopyKeyMatSwap(uint32_t * const Destination_p,
                            const unsigned int offset,
                            const uint8_t * const Source_p,
                            const unsigned int KeyByteCount)
{
    uint32_t *dst = Destination_p + offset;
    const uint8_t *src = Source_p;
    unsigned int i,j;
    uint32_t w;
    if (Destination_p == NULL)
        return;
    for(i=0; i<(KeyByteCount+3)/4; i++)
    {
        w=0;
        for(j=0; j<4; j++)
            w=(w<<8)|(*src++);
        *dst++ = w;
    }
}


/*----------------------------------------------------------------------------
 * SABuilder_SetCipherKeys
 *
 * Fill in cipher keys and associated command word fields in SA.
 *
 * SAParams_p (input)
 *   The SA parameters structure from which the SA is derived.
 * SAState_p (input, output)
 *   Variables containing information about the SA being generated/
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
static SABuilder_Status_t
SABuilder_SetCipherKeys(SABuilder_Params_t *const SAParams_p,
                        SABuilder_State_t * const SAState_p,
                         uint32_t * const SABuffer_p)
{
    /* Fill in crypto-algorithm specific parameters */
    switch (SAParams_p->CryptoAlgo)
    {
    case SAB_CRYPTO_NULL: /* Null crypto, do nothing */
        break;
#ifdef SAB_ENABLE_CRYPTO_3DES
    case SAB_CRYPTO_DES:
        SAState_p->CipherKeyWords = 2;
        SAState_p->IVWords = 2;
        SAState_p->CW0 |= SAB_CW0_CRYPTO_DES;
        break;
    case SAB_CRYPTO_3DES:
        SAState_p->CipherKeyWords = 6;
        SAState_p->IVWords = 2;
        SAState_p->CW0 |= SAB_CW0_CRYPTO_3DES;
        break;
#endif
#ifdef SAB_ENABLE_CRYPTO_AES
    case SAB_CRYPTO_AES:
        switch (SAParams_p->KeyByteCount)
        {
        case 16:
            SAState_p->CW0 |= SAB_CW0_CRYPTO_AES_128;
            SAState_p->CipherKeyWords = 4;
            break;
        case 24:
            SAState_p->CW0 |= SAB_CW0_CRYPTO_AES_192;
            SAState_p->CipherKeyWords = 6;
            break;
        case 32:
            SAState_p->CW0 |= SAB_CW0_CRYPTO_AES_256;
            SAState_p->CipherKeyWords = 8;
            break;
        default:
            LOG_CRIT("SABuilder: Bad key size for AES.\n");
            return SAB_INVALID_PARAMETER;
        }
        SAState_p->IVWords = 4;
        break;
#endif
#ifdef SAB_ENABLE_CRYPTO_ARCFOUR
    case SAB_CRYPTO_ARCFOUR:
        SAState_p->CW0 |= SAB_CW0_CRYPTO_ARC4;
        if (SAParams_p->KeyByteCount < 5 ||
            SAParams_p->KeyByteCount > 16)
        {
            LOG_CRIT("SABuilder: Bad key size for ARCFOUR.\n");
            return SAB_INVALID_PARAMETER;
        }
        SAState_p->CW1 |= SAParams_p->KeyByteCount;
        SAState_p->CipherKeyWords =
            ((unsigned int)SAParams_p->KeyByteCount + 3) / 4;

        if (SAParams_p->CryptoMode != SAB_CRYPTO_MODE_STATELESS)
        {
            SAState_p->ARC4State = true;
            SAState_p->CW1 |= SAB_CW1_ARC4_IJ_PTR | SAB_CW1_ARC4_STATE_SEL |
                SAB_CW1_CRYPTO_STORE;
        }

        break;
#endif
    default:
        LOG_CRIT("SABuilder: Unsupported crypto algorithm\n");
        return SAB_UNSUPPORTED_FEATURE;
    }

    /* Check block cipher length against provided key  */
    if (SAParams_p->CryptoAlgo != SAB_CRYPTO_ARCFOUR &&
        SAState_p->CipherKeyWords*4 != SAParams_p->KeyByteCount)
    {
        LOG_CRIT("SABuilder: Bad cipher key size..\n");
        return SAB_INVALID_PARAMETER;
    }
#ifdef SAB_STRICT_ARGS_CHECK
    if ( SAState_p->CipherKeyWords > 0 && SAParams_p->Key_p == NULL)
    {
        LOG_CRIT("SABuilder: NULL pointer for Key_p.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    /* Copy the cipher key */

    SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                            SAParams_p->Key_p, SAParams_p->KeyByteCount);

    if (SAParams_p->CryptoAlgo == SAB_CRYPTO_ARCFOUR)
        SAState_p->CurrentOffset += 4; /* Always use 4 words for key in ARC4*/
    else
        SAState_p->CurrentOffset += SAState_p->CipherKeyWords;

    /* Handle feedback modes */
    if (SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL &&
        SAParams_p->CryptoAlgo != SAB_CRYPTO_ARCFOUR)
    {
        switch (SAParams_p->CryptoMode)
        {
        case SAB_CRYPTO_MODE_ECB:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_ECB;
            SAState_p->IVWords = 0;
            break;
        case SAB_CRYPTO_MODE_CBC:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CBC;
            break;
        case SAB_CRYPTO_MODE_CFB:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CFB;
            break;
        case SAB_CRYPTO_MODE_OFB:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_OFB;
            break;
        case SAB_CRYPTO_MODE_CTR:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CTR;
            break;
        case SAB_CRYPTO_MODE_ICM:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_ICM;
            break;
        case SAB_CRYPTO_MODE_CCM:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CTR_LOAD;
            if (SAParams_p->AuthAlgo != SAB_AUTH_AES_CCM)
            {
                LOG_CRIT("SABuilder: crypto CCM requires auth CCM.\n");
                return SAB_INVALID_PARAMETER;
            }
            break;
        case SAB_CRYPTO_MODE_GCM:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CTR;
            if (SAParams_p->AuthAlgo != SAB_AUTH_AES_GCM)
            {
                LOG_CRIT("SABuilder: crypto GCM requires auth GCM.\n");
                return SAB_INVALID_PARAMETER;
            }
            break;
        case SAB_CRYPTO_MODE_GMAC:
            SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CTR;
            if (SAParams_p->AuthAlgo != SAB_AUTH_AES_GMAC)
            {
                LOG_CRIT("SABuilder: crypto GMAC requires auth GMAC.\n");
                return SAB_INVALID_PARAMETER;
            }
            break;
        default:
            LOG_CRIT("SABuilder: Invalid crypto mode.\n");
            return SAB_INVALID_PARAMETER;
        }
    }
    /* The following crypto modes can only be used with AES */
    if ( (SAParams_p->CryptoMode==SAB_CRYPTO_MODE_CTR ||
          SAParams_p->CryptoMode==SAB_CRYPTO_MODE_ICM ||
          SAParams_p->CryptoMode==SAB_CRYPTO_MODE_CCM ||
          SAParams_p->CryptoMode==SAB_CRYPTO_MODE_GCM ||
          SAParams_p->CryptoMode==SAB_CRYPTO_MODE_GMAC) &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_AES)
    {
        LOG_CRIT("SABuilder: crypto mode requires AES.\n");
        return SAB_INVALID_PARAMETER;
    }
    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_SetAuthKeys
 *
 * Fill in authentication keys and associated command word fields in SA.
 *
 * SAParams_p (input)
 *   The SA parameters structure from which the SA is derived.
 * SAState_p (input, output)
 *   Variables containing information about the SA being generated/
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
static SABuilder_Status_t
SABuilder_SetAuthKeys(SABuilder_Params_t *const SAParams_p,
                         SABuilder_State_t * const SAState_p,
                         uint32_t * const SABuffer_p)
{
    unsigned int Auth1Words = 0;
    unsigned int Auth2Words = 0;

    switch (SAParams_p->AuthAlgo)
    {
    case SAB_AUTH_NULL:
        break;
#ifdef SAB_ENABLE_AUTH_MD5
    case SAB_AUTH_HASH_MD5:
        SAState_p->CW0 |= SAB_CW0_AUTH_HASH_MD5;
        if ((SAParams_p->flags & (SAB_FLAG_HASH_LOAD|SAB_FLAG_HASH_SAVE|
                                  SAB_FLAG_HASH_INTERMEDIATE)) != 0)
        {
            SAState_p->CW0 |= SAB_CW0_HASH_LOAD_DIGEST;
            Auth1Words = 4;
        }
        break;
#endif
#ifdef SAB_ENABLE_AUTH_SHA1
    case SAB_AUTH_HASH_SHA1:
        SAState_p->CW0 |= SAB_CW0_AUTH_HASH_SHA1;
        if ((SAParams_p->flags & (SAB_FLAG_HASH_LOAD|SAB_FLAG_HASH_SAVE|
                                  SAB_FLAG_HASH_INTERMEDIATE)) != 0)
        {
            SAState_p->CW0 |= SAB_CW0_HASH_LOAD_DIGEST;
            Auth1Words = 5;
        }
        break;
#endif
#ifdef SAB_ENABLE_AUTH_SHA2_256
    case SAB_AUTH_HASH_SHA2_224:
        SAState_p->CW0 |= SAB_CW0_AUTH_HASH_SHA2_224;
        if ((SAParams_p->flags & (SAB_FLAG_HASH_LOAD|SAB_FLAG_HASH_SAVE|
                                  SAB_FLAG_HASH_INTERMEDIATE)) != 0)
        {
            SAState_p->CW0 |= SAB_CW0_HASH_LOAD_DIGEST;
            Auth1Words = 8;
        }
        break;
    case SAB_AUTH_HASH_SHA2_256:
        SAState_p->CW0 |= SAB_CW0_AUTH_HASH_SHA2_256;
        if ((SAParams_p->flags & (SAB_FLAG_HASH_LOAD|SAB_FLAG_HASH_SAVE|
                                  SAB_FLAG_HASH_INTERMEDIATE)) != 0)
        {
            SAState_p->CW0 |= SAB_CW0_HASH_LOAD_DIGEST;
            Auth1Words = 8;
        }
        break;
#endif
#ifdef SAB_ENABLE_AUTH_SHA2_512
    case SAB_AUTH_HASH_SHA2_384:
        SAState_p->CW0 |= SAB_CW0_AUTH_HASH_SHA2_384;
        if ((SAParams_p->flags & (SAB_FLAG_HASH_LOAD|SAB_FLAG_HASH_SAVE|
                                  SAB_FLAG_HASH_INTERMEDIATE)) != 0)
        {
            SAState_p->CW0 |= SAB_CW0_HASH_LOAD_DIGEST;
            Auth1Words = 16;
        }
        break;
    case SAB_AUTH_HASH_SHA2_512:
        SAState_p->CW0 |= SAB_CW0_AUTH_HASH_SHA2_512;
        if ((SAParams_p->flags & (SAB_FLAG_HASH_LOAD|SAB_FLAG_HASH_SAVE|
                                  SAB_FLAG_HASH_INTERMEDIATE)) != 0)
        {
            SAState_p->CW0 |= SAB_CW0_HASH_LOAD_DIGEST;
            Auth1Words = 16;
        }
        break;
#endif
#ifdef SAB_ENABLE_AUTH_MD5
    case SAB_AUTH_SSLMAC_MD5:
    case SAB_AUTH_HMAC_MD5:
        SAState_p->CW0 |= SAB_CW0_AUTH_HMAC_MD5;
        Auth1Words = 4;
        Auth2Words = 4;
        break;
#endif
#ifdef SAB_ENABLE_AUTH_SHA1
    case SAB_AUTH_SSLMAC_SHA1:
        SAState_p->CW0 |= SAB_CW0_AUTH_SSLMAC_SHA1;
        Auth1Words = 5;
        break;
    case SAB_AUTH_HMAC_SHA1:
        SAState_p->CW0 |= SAB_CW0_AUTH_HMAC_SHA1;
        Auth1Words = 5;
        Auth2Words = 5;
        break;
#endif
#ifdef SAB_ENABLE_AUTH_SHA2_256
    case SAB_AUTH_HMAC_SHA2_224:
        SAState_p->CW0 |= SAB_CW0_AUTH_HMAC_SHA2_224;
        Auth1Words = 8;
        Auth2Words = 8;
        break;
    case SAB_AUTH_HMAC_SHA2_256:
        SAState_p->CW0 |= SAB_CW0_AUTH_HMAC_SHA2_256;
        Auth1Words = 8;
        Auth2Words = 8;
        break;
#endif
#ifdef SAB_ENABLE_AUTH_SHA2_512
    case SAB_AUTH_HMAC_SHA2_384:
        SAState_p->CW0 |= SAB_CW0_AUTH_HMAC_SHA2_384;
        Auth1Words = 16;
        Auth2Words = 16;
        break;
    case SAB_AUTH_HMAC_SHA2_512:
        SAState_p->CW0 |= SAB_CW0_AUTH_HMAC_SHA2_512;
        Auth1Words = 16;
        Auth2Words = 16;
        break;
#endif
    case SAB_AUTH_AES_XCBC_MAC:
        SAState_p->CW0 |= SAB_CW0_AUTH_CMAC_128;
        Auth1Words = 4;
        Auth2Words = 4;
        break;
    case SAB_AUTH_AES_CCM:
        if (SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CCM)
        {
            LOG_CRIT("SABuilder: auth CCM requires crypto CCM.\n");
            return SAB_INVALID_PARAMETER;
        }
        switch (SAParams_p->KeyByteCount)
        {
        case 16:
            Auth1Words = 4;
            SAState_p->CW0 |= SAB_CW0_AUTH_CMAC_128;
            break;
        case 24:
            Auth1Words = 6;
            SAState_p->CW0 |= SAB_CW0_AUTH_CMAC_192;
            break;
        case 32:
            Auth1Words = 8;
            SAState_p->CW0 |= SAB_CW0_AUTH_CMAC_256;
            break;
        }
        Auth2Words = 4;
        SAState_p->CW1 |= SAB_CW1_ENCRYPT_HASHRES;
        break;
#ifdef SAB_ENABLE_CRYPTO_GCM
    case SAB_AUTH_AES_GCM:
        if (SAParams_p->CryptoMode != SAB_CRYPTO_MODE_GCM)
        {
            LOG_CRIT("SABuilder: auth GCM requires crypto GCM.\n");
            return SAB_INVALID_PARAMETER;
        }
        SAState_p->CW0 |= SAB_CW0_AUTH_GHASH;
        Auth1Words = 4;
        SAState_p->CW1 |= SAB_CW1_ENCRYPT_HASHRES;
        break;
    case SAB_AUTH_AES_GMAC:
        if (SAParams_p->CryptoMode != SAB_CRYPTO_MODE_GMAC)
        {
            LOG_CRIT("SABuilder: auth GMAC requires crypto GMAC.\n");
            return SAB_INVALID_PARAMETER;
        }
        SAState_p->CW0 |= SAB_CW0_AUTH_GHASH;
        Auth1Words = 4;
        SAState_p->CW1 |= SAB_CW1_ENCRYPT_HASHRES;
        break;
#endif
    default:
        LOG_CRIT("SABuilder: Unsupported authentication algorithm\n");
        return SAB_UNSUPPORTED_FEATURE;
    }

    /* Now copy the authentication keys, if applicable */
    if (SAParams_p->AuthAlgo == SAB_AUTH_AES_CCM)
    {
        SAParams_p->OffsetDigest0 = SAState_p->CurrentOffset;
        SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset ,
                                ZeroWords, 32);
        /* Fill zero blocks for the XCBC MAC subkeys */
        SAState_p->CurrentOffset += 8;
        SABuilderLib_CopyKeyMatSwap(SABuffer_p, SAState_p->CurrentOffset,
                                    SAParams_p->Key_p,
                                    SAParams_p->KeyByteCount);
        SAState_p->CurrentOffset += SAState_p->CipherKeyWords;
        if (SAState_p->CipherKeyWords == 6)
        {
            SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                                    ZeroWords, 8);
            SAState_p->CurrentOffset += 2; /* Pad key to 256 bits for CCM-192*/
        }
    }
    else if (SAParams_p->AuthAlgo == SAB_AUTH_AES_XCBC_MAC)
    {
#ifdef SAB_STRICT_ARGS_CHECK
        if (SAParams_p->AuthKey1_p == NULL ||
            SAParams_p->AuthKey2_p == NULL ||
            SAParams_p->AuthKey3_p == NULL)
        {
            LOG_CRIT("SABuilder: NULL pointer AuthKey supplied\n");
            return SAB_INVALID_PARAMETER;
        }
#endif
        SAParams_p->OffsetDigest0 = SAState_p->CurrentOffset;
        SABuilderLib_CopyKeyMatSwap(SABuffer_p, SAState_p->CurrentOffset,
                                    SAParams_p->AuthKey2_p, 16);
        SABuilderLib_CopyKeyMatSwap(SABuffer_p, SAState_p->CurrentOffset + 4,
                                    SAParams_p->AuthKey3_p, 16);
        SABuilderLib_CopyKeyMatSwap(SABuffer_p, SAState_p->CurrentOffset + 8,
                                    SAParams_p->AuthKey1_p, 16);
        SAState_p->CurrentOffset += 12;
    }
    else
    {
        if (Auth1Words > 0)
        {
#ifdef SAB_STRICT_ARGS_CHECK
            if (SAParams_p->AuthKey1_p == NULL)
            {
                LOG_CRIT("SABuilder: NULL pointer AuthKey supplied\n");
                return SAB_INVALID_PARAMETER;
            }
#endif
            SAParams_p->OffsetDigest0 = SAState_p->CurrentOffset;
            SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                                    SAParams_p->AuthKey1_p, Auth1Words*4);
            SAState_p->CurrentOffset += Auth1Words;
            if (SAParams_p->AuthAlgo == SAB_AUTH_SSLMAC_SHA1)
            { /* both inner and outer digest fields must be set, even though
                 only one is used. */
                SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                                        ZeroWords, Auth1Words*4);
                SAState_p->CurrentOffset += Auth1Words;
            }
        }
        if (Auth2Words > 0)
        {
#ifdef SAB_STRICT_ARGS_CHECK
            if (SAParams_p->AuthKey2_p == NULL)
            {
                LOG_CRIT("SABuilder: NULL pointer AuthKey supplied\n");
                return SAB_INVALID_PARAMETER;
            }
#endif
            SAParams_p->OffsetDigest1 = SAState_p->CurrentOffset;
            SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                                    SAParams_p->AuthKey2_p, Auth2Words*4);
            SAState_p->CurrentOffset += Auth2Words;
        }
    }

    return SAB_STATUS_OK;
}



/*----------------------------------------------------------------------------
 * SABuilder_GetSizes();
 *
 * Compute the required sizes in 32-bit words of any of up to three memory
 * areas used by the SA.
 *
 * SAParams_p (input)
 *   Pointer to the SA parameters structure.
 * SAWord32Count_p (output)
 *   The size of the normal SA buffer,
 * SAStateWord32Count_p (output)
 *   The size of any SA state record.
 * ARC4StateWord32Count_p (output) T
 *   The size of any ARCFOUR state buffer (output).
 *
 * When the SA state record or ARCFOUR state buffer are not required by
 * the packet engine for this transform, the corresponding size outputs
 * are returned as zero. The SafeXcel-IP-96 never requires these buffers
 *
 * If any of the output parameters is a null pointer,
 * the corresponding size will not be returned.
 *
 * The SAParams_p structure must be fully filled in: it must have the
 * same contents as it would have when SABuilder_BuildSA is called.
 * This function calls the same routines as SABuilder_BuildSA, but with
 * null pointers instead of the SA pointer, so no actual SA will be built.
 * These functions are only called to obtain the length of the SA.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when the record referenced by SAParams_p is invalid,
 */
SABuilder_Status_t
SABuilder_GetSizes(
    SABuilder_Params_t *const SAParams_p,
    unsigned int *const SAWord32Count_p,
    unsigned int *const SAStateWord32Count_p,
    unsigned int *const ARC4StateWord32Count_p
    )
{
    SABuilder_State_t SAState;
    int rc;

#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL)
    {
        LOG_CRIT("SABuilder_GetSizes: NULL pointer SAParams_p supplied\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    SAState.CurrentOffset = 2; /* Count Control words 0 and 1 */
    SAState.CW0 = 0;
    SAState.CW1 = 0;
    SAState.CipherKeyWords = 0;
    SAState.IVWords = 0;
    SAState.ARC4State = false;

    rc = SABuilder_SetCipherKeys(SAParams_p, &SAState, NULL);
    if (rc != SAB_STATUS_OK)
        return rc;

    rc = SABuilder_SetAuthKeys(SAParams_p, &SAState, NULL);
    if (rc != SAB_STATUS_OK)
        return rc;

    switch ( SAParams_p->protocol)
    {
#ifdef SAB_ENABLE_PROTO_BASIC
    case SAB_PROTO_BASIC:
    {
        rc = SABuilder_SetBasicParams(SAParams_p, &SAState, NULL);
    }
    break;
#endif
#ifdef SAB_ENABLE_PROTO_IPSEC
    case SAB_PROTO_IPSEC:
    {
        rc = SABuilder_SetIPsecParams(SAParams_p, &SAState, NULL);
	}
    break;
#endif
#ifdef SAB_ENABLE_PROTO_SSLTLS
    case SAB_PROTO_SSLTLS:
    {
        rc = SABuilder_SetSSLTLSParams(SAParams_p, &SAState, NULL);
    }
    break;
#endif
#ifdef SAB_ENABLE_PROTO_SRTP
    case SAB_PROTO_SRTP:
    {
        rc = SABuilder_SetSRTPParams(SAParams_p, &SAState, NULL);
    }
    break;
#endif
    default:
        LOG_CRIT("SABuilder_GetSizes: unsupported protocol\n");
        return SAB_INVALID_PARAMETER;
    }
    if (rc != SAB_STATUS_OK)
        return rc;

    if (SAState.ARC4State)
    {
        SAState.CurrentOffset += 2; /* Count IJ pointer and ARC4 State */
    }

#ifdef SAB_ENABLE_FIXED_RECORD_SIZE
#ifdef SAB_ENABLE_TWO_FIXED_RECORD_SIZES
    if (SAState.CurrentOffset <= SAB_LARGE_RECORD_THRESHOLD_WORD_COUNT)
    {
        SAState.CurrentOffset = SAB_RECORD_WORD_COUNT;
    }
    else if (SAState.CurrentOffset <= SAB_RECORD_WORD_COUNT_LARGE)
    {
        SAState.CurrentOffset = SAB_RECORD_WORD_COUNT_LARGE;
    }
    else
    {
        LOG_CRIT("SABuilder_GetSizes: SA filled beyond record size.\n");
        return SAB_INVALID_PARAMETER;
    }
#else
    if (SAState.CurrentOffset > SAB_RECORD_WORD_COUNT)
    {
        LOG_CRIT("SABuilder_GetSizes: SA filled beyond record size.\n");
        return SAB_INVALID_PARAMETER;
    }
    SAState.CurrentOffset = SAB_RECORD_WORD_COUNT;
    /* Make the SA record a fixed size for engines that have
       record caches */
#endif
#endif

    if (SAStateWord32Count_p != NULL)
        *SAStateWord32Count_p = 0;
#ifdef SAB_ARC4_STATE_IN_SA
    if (ARC4StateWord32Count_p != NULL)
        *ARC4StateWord32Count_p = 0;
    if (SAState.ARC4State)
    {
        if (SAParams_p->OffsetARC4StateRecord > 0)
        {
            if (SAParams_p->OffsetARC4StateRecord  < SAState.CurrentOffset)
            {
                LOG_CRIT("SABuilder_GetSizes: OffsetARC4StateRecord too low\n");
                return SAB_INVALID_PARAMETER;
            }
            SAState.CurrentOffset = SAParams_p->OffsetARC4StateRecord + 64;
        }
        else
        {
            SAState.CurrentOffset += 64;
        }
    }
#else
    if (ARC4StateWord32Count_p != NULL)
    {
        if (SAState.ARC4State)
        {
            *ARC4StateWord32Count_p = 64;
        }
        else
        {
            *ARC4StateWord32Count_p = 0;
        }
    }
#endif
    if (SAWord32Count_p != NULL)
        *SAWord32Count_p = SAState.CurrentOffset;

    return SAB_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SABuilder_BuildSA();
 *
 * Construct the SA record for the operation described in SAParams_p in
 * up to three memory buffers.
 *
 * SAParams_p (input)
 *    Pointer to the SA parameters structure.
 * SABuffer_p (output)
 *    Pointer to the the normal SA buffer,
 * SAStateBuffer_p (output)
 *    Pointer to the SA state record buffer,
 * ARC4StateBuffer_p (output)
 *    Pointer to the ARCFOUR state buffer.
 *
 * Each of the Buffer arguments must point to a word-aligned
 * memory buffer whose size in words is at least equal to the
 * corresponding size parameter returned by SABuilder_GetSizes().
 *
 * If any of the three buffers is not required for the SA (the
 * corresponding size in SABuilder_GetSizes() is 0), the corresponding
 * Buffer arguments to this function may be a null pointer.
 * The SafeXcel-IP-96 never requires these buffers.
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
SABuilder_BuildSA(
    SABuilder_Params_t * const SAParams_p,
    uint32_t *const SABuffer_p,
    uint32_t *const SAStateBuffer_p,
    uint32_t *const ARC4StateBuffer_p
    )
{
    SABuilder_State_t SAState;
    int rc;

    IDENTIFIER_NOT_USED(SAStateBuffer_p);
    IDENTIFIER_NOT_USED(ARC4StateBuffer_p);
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL || SABuffer_p == NULL)
    {
        LOG_CRIT("SABuilder: NULL pointer parameter supplied.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    SAState.CurrentOffset = 2; /* Count Control words 0 and 1 */
    SAState.CW0 = 0;
    SAState.CW1 = 0;
    SAState.CipherKeyWords = 0;
    SAState.IVWords = 0;
    SAState.ARC4State = false;

    rc = SABuilder_SetCipherKeys(SAParams_p, &SAState, SABuffer_p);
    if (rc != SAB_STATUS_OK)
        return rc;

    rc = SABuilder_SetAuthKeys(SAParams_p, &SAState, SABuffer_p);
    if (rc != SAB_STATUS_OK)
        return rc;

    switch ( SAParams_p->protocol)
    {
#ifdef SAB_ENABLE_PROTO_BASIC
    case SAB_PROTO_BASIC:
    {
        rc = SABuilder_SetBasicParams(SAParams_p, &SAState, SABuffer_p);
    }
    break;
#endif
#ifdef SAB_ENABLE_PROTO_IPSEC
    case SAB_PROTO_IPSEC:
    {
        rc = SABuilder_SetIPsecParams(SAParams_p, &SAState, SABuffer_p);
    }
    break;
#endif
#ifdef SAB_ENABLE_PROTO_SSLTLS
    case SAB_PROTO_SSLTLS:
    {
        rc = SABuilder_SetSSLTLSParams(SAParams_p, &SAState, SABuffer_p);
    }
    break;
#endif
#ifdef SAB_ENABLE_PROTO_SRTP
    case SAB_PROTO_SRTP:
    {
        rc = SABuilder_SetSRTPParams(SAParams_p, &SAState, SABuffer_p);
    }
    break;
#endif
    default:
        LOG_CRIT("SABuilder_BuildSA: unsupported protocol\n");
        return SAB_UNSUPPORTED_FEATURE;
    }
    if (rc != SAB_STATUS_OK)
        return rc;

    if (SAState.ARC4State)
    {
#ifdef SAB_ARC4_STATE_IN_SA
        if (SAParams_p->OffsetARC4StateRecord > 0)
            SABuffer_p[SAState.CurrentOffset] =
                SAParams_p->OffsetARC4StateRecord * 4;
        else
            SABuffer_p[SAState.CurrentOffset] = (SAState.CurrentOffset + 2) * 4;
#elif defined(SAB_ENABLE_FIXED_RECORD_SIZE)
        SABuffer_p[SAState.CurrentOffset] = SAB_RECORD_WORD_COUNT * 4;
#else
        SABuffer_p[SAState.CurrentOffset] = 0;
#endif
        SABuffer_p[SAState.CurrentOffset + 1] = 0;
        if ( (SAParams_p->flags & SAB_FLAG_ARC4_STATE_LOAD) != 0)
        {
            /* Load the ARC4 state when building the SA.
             Nonce_p[0] is the 'i' variable and
             Nonce_p[1] is the 'j' variable.
             IV_p points to the 256-byte state array.
             The SA Builder will not fill in the ARC4 state pointer. */
            if (SAParams_p->Nonce_p != NULL)
            {
                SABuffer_p[SAState.CurrentOffset + 1] =
                    ((SAParams_p->Nonce_p[0] + 1) & 0xff) |
                    (SAParams_p->Nonce_p[1]<<8);
            }
            if(SAParams_p->IV_p != NULL)
            {
#ifdef SAB_ARC4_STATE_IN_SA
                SABuilderLib_CopyKeyMat(SABuffer_p,
                                        (SAParams_p->OffsetARC4StateRecord>0 ?
                                         SAParams_p->OffsetARC4StateRecord :
                                         SAState.CurrentOffset + 2),
                                        SAParams_p->IV_p, 256);

#else
                SABuilderLib_CopyKeyMat(ARC4StateBuffer_p, 0,
                                        SAParams_p->IV_p, 256);
#endif
            }

        }

        SAParams_p->OffsetIJPtr = SAState.CurrentOffset + 1;
        SAParams_p->OffsetARC4State = SAState.CurrentOffset;

        SAState.CurrentOffset += 2; /* Count IJ pointer and ARC4 State */
    }

    SAState.CW0 |= (SAState.CurrentOffset - 2) << 8;

    SABuffer_p[0] = SAState.CW0;
    SABuffer_p[1] = SAState.CW1;

    SAParams_p->CW0 = SAState.CW0;
    SAParams_p->CW1 = SAState.CW1;

#ifdef SAB_ENABLE_IPSEC_EXTENDED
    if (SAParams_p->protocol == SAB_PROTO_IPSEC)
    {
        SABuilder_Status_t res;
        res = SABuilder_SetExtendedIPsecParams(SAParams_p, &SAState, SABuffer_p);
        if (res != SAB_STATUS_OK)
            return res;
    }
#endif
#ifdef SAB_ENABLE_DTLS_EXTENDED
    if (SAParams_p->protocol == SAB_PROTO_SSLTLS)
    {
        SABuilder_Status_t res;
        res = SABuilder_SetExtendedDTLSParams(SAParams_p, &SAState, SABuffer_p);
        if (res != SAB_STATUS_OK)
            return res;
    }
#endif

#ifdef SAB_ENABLE_FIXED_RECORD_SIZE
#ifdef SAB_ENABLE_TWO_FIXED_RECORD_SIZES
    if (SAState.CurrentOffset > SAB_LARGE_RECORD_THRESHOLD_WORD_COUNT)
    {
        SABuffer_p[0] |= SAB_CW0_SW_IS_LARGE;
    }
#endif
#endif

    return SAB_STATUS_OK;
}


/* end of file sa_builder.c */



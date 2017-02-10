/* sa_builder_internal.h
 *
 * Internal API of the EIP-96 SA Builder.
 * - layout of the control words.
 * - Data structure that represents the SA builder state.
 * - Headers for shared functions and protocol-specific functions.
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


#ifndef SA_BUILDER_INTERNAL_H_
#define SA_BUILDER_INTERNAL_H_
#include "c_sa_builder.h"
#include "sa_builder.h"


/* Type of packet field in Control Word 0 */

#define SAB_CW0_TOP_NULL_OUT        0x00000000
#define SAB_CW0_TOP_NULL_IN         0x00000001
#define SAB_CW0_TOP_HASH_OUT        0x00000002
#define SAB_CW0_TOP_HASH_IN         0x00000003
#define SAB_CW0_TOP_ENCRYPT         0x00000004
#define SAB_CW0_TOP_DECRYPT         0x00000005
#define SAB_CW0_TOP_ENCRYPT_HASH    0x00000006
#define SAB_CW0_TOP_DECRYPT_HASH    0x00000007
#define SAB_CW0_TOP_HASH_ENCRYPT    0x0000000e
#define SAB_CW0_TOP_HASH_DECRYPT    0x0000000f

/* Flags to indicate SA properties to other software components.
   They will be stored inside the per-packet options field in the SA and
   as such they will never be user by hardware. */
#define SAB_CW0_SW_IS_LARGE         0x00000010

/* Cryptographic algorith, field (and key bit) */

#define SAB_CW0_CRYPTO_NULL         0x00000000
#define SAB_CW0_CRYPTO_DES          0x00010000
#define SAB_CW0_CRYPTO_ARC4         0x00030000
#define SAB_CW0_CRYPTO_3DES         0x00050000
#define SAB_CW0_CRYPTO_AES_128      0x000b0000
#define SAB_CW0_CRYPTO_AES_192      0x000d0000
#define SAB_CW0_CRYPTO_AES_256      0x000f0000

/* Hash and authentication algorithm field */

#define SAB_CW0_AUTH_NULL           0x00000000
#define SAB_CW0_AUTH_HASH_MD5       0x00000000
#define SAB_CW0_AUTH_HASH_SHA1      0x01000000
#define SAB_CW0_AUTH_HASH_SHA2_256  0x01800000
#define SAB_CW0_AUTH_HASH_SHA2_224  0x02000000
#define SAB_CW0_AUTH_HASH_SHA2_512  0x02800000
#define SAB_CW0_AUTH_HASH_SHA2_384  0x03000000
#define SAB_CW0_AUTH_SSLMAC_SHA1    0x00e00000
#define SAB_CW0_AUTH_HMAC_MD5       0x00600000
#define SAB_CW0_AUTH_HMAC_SHA1      0x01600000
#define SAB_CW0_AUTH_HMAC_SHA2_256  0x01e00000
#define SAB_CW0_AUTH_HMAC_SHA2_224  0x02600000
#define SAB_CW0_AUTH_HMAC_SHA2_512  0x02e00000
#define SAB_CW0_AUTH_HMAC_SHA2_384  0x03600000
#define SAB_CW0_AUTH_CMAC_128       0x00c00000
#define SAB_CW0_AUTH_CMAC_192       0x01400000
#define SAB_CW0_AUTH_CMAC_256       0x01c00000
#define SAB_CW0_AUTH_GHASH          0x02400000

/* Add this wehn hash value must be loaded from context */
#define SAB_CW0_HASH_LOAD_DIGEST    0x00200000

/* SPI present */
#define SAB_CW0_SPI                 0x08000000

/* Sequence number size */
#define SAB_CW0_SEQNUM_32           0x10000000
#define SAB_CW0_SEQNUM_48           0x20000000
#define SAB_CW0_SEQNUM_64           0x30000000

/* Mask size */
#define SAB_CW0_MASK_64             0x40000000
#define SAB_CW0_MASK_32             0x80000000
#define SAB_CW0_MASK_128            0xc0000000

/* Crypto feedback mode */
#define SAB_CW1_CRYPTO_MODE_ECB     0x00000000
#define SAB_CW1_CRYPTO_MODE_CBC     0x00000001
#define SAB_CW1_CRYPTO_MODE_CTR     0x00000002
#define SAB_CW1_CRYPTO_MODE_ICM     0x00000003
#define SAB_CW1_CRYPTO_MODE_OFB     0x00000004
#define SAB_CW1_CRYPTO_MODE_CFB     0x00000005
#define SAB_CW1_CRYPTO_MODE_CTR_LOAD 0x00000006


/* IV words load */
#define SAB_CW1_IV0                 0x00000020
#define SAB_CW1_IV1                 0x00000040
#define SAB_CW1_IV2                 0x00000080
#define SAB_CW1_IV3                 0x00000100

#define SAB_CW1_DIGEST_CNT          0x00000200

/* IV mode */
#define SAB_CW1_IV_FULL              0x00000000
#define SAB_CW1_IV_CTR               0x00000400
#define SAB_CW1_IV_ORIG_SEQ          0x00000800
#define SAB_CW1_IV_INCR_SEQ          0x00000c00


#define SAB_CW1_CRYPTO_STORE        0x00001000

#define SAB_CW1_PREPKT_OP           0x00002000

/* Pad type */
#define SAB_CW1_PAD_ZERO            0x00000000
#define SAB_CW1_PAD_PKCS7           0x00004000
#define SAB_CW1_PAD_CONST           0x00008000
#define SAB_CW1_PAD_RTP             0x0000c000
#define SAB_CW1_PAD_IPSEC           0x00010000
#define SAB_CW1_PAD_TLS             0x00014000
#define SAB_CW1_PAD_SSL             0x00018000
#define SAB_CW1_PAD_IPSEC_NOCHECK   0x0001c000


#define SAB_CW1_ENCRYPT_HASHRES     0x00020000
#define SAB_CW1_HASH_STORE          0x00080000

#define SAB_CW1_ARC4_IJ_PTR         0x00100000
#define SAB_CW1_ARC4_STATE_SEL      0x00200000
#define SAB_CW1_SEQNUM_STORE        0x00400000
#define SAB_CW1_NO_MASK_UPDATE      0x00800000
#define SAB_CW1_EARLY_SEQNUM_UPDATE 0x40000000
#define SAB_CW1_CNTX_FETCH_MODE     0x80000000

/* Flag byte for CCM. first byte in counter mode IV, equal to L-1.
 */
#define SAB_CCM_FLAG_L4             0x3

/* This structure represents the internal state of the SA builder, shared
   between all sub-functions, e.g. for encryption, hash and protocol-specific
   extensions.
 */
typedef struct
{
    unsigned int CurrentOffset; /* Current word offset within the SA */
    uint32_t CW0; /* Control word 0 */
    uint32_t CW1; /* Control word 1 */
    unsigned int CipherKeyWords; /* Size of the cipher key in words */
    unsigned int IVWords; /* Size of the IV in words */
    bool ARC4State; /* Is ARC4 state used? */
} SABuilder_State_t;


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
                        const unsigned int KeyByteCount);


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
                            const unsigned int KeyByteCount);



#ifdef SAB_ENABLE_PROTO_BASIC
/*----------------------------------------------------------------------------
 * SABuilder_SetSSLTLSParams
 *
 * Fill in Basic Crypto and hash specific extensions into the SA.
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
SABuilder_Status_t
SABuilder_SetBasicParams(SABuilder_Params_t *const SAParams_p,
                          SABuilder_State_t * const SAState_p,
                          uint32_t * const SABuffer_p);

#endif


#ifdef SAB_ENABLE_PROTO_IPSEC
/*----------------------------------------------------------------------------
 * SABuilder_SetIPsecParams
 *
 * Fill in IPsec-specific extensions into the SA.
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
SABuilder_Status_t
SABuilder_SetIPsecParams(SABuilder_Params_t *const SAParams_p,
                         SABuilder_State_t * const SAState_p,
                         uint32_t * const SABuffer_p);

#endif

#ifdef SAB_ENABLE_PROTO_SSLTLS
/*----------------------------------------------------------------------------
 * SABuilder_SetSSLTLSParams
 *
 * Fill in SSL/TLS/DTLS-specific extensions into the SA.
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
SABuilder_Status_t
SABuilder_SetSSLTLSParams(SABuilder_Params_t *const SAParams_p,
                          SABuilder_State_t * const SAState_p,
                          uint32_t * const SABuffer_p);

#endif


#ifdef SAB_ENABLE_PROTO_SRTP
/*----------------------------------------------------------------------------
 * SABuilder_SetSRTPParams
 *
 * Fill in SRTP-specific extensions into the SA.
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
SABuilder_Status_t
SABuilder_SetSRTPParams(SABuilder_Params_t *const SAParams_p,
                          SABuilder_State_t * const SAState_p,
                          uint32_t * const SABuffer_p);

#endif



#endif /* SA_BUILDER_INTERNAL_H_ */


/* end of file sa_builder_internal.h */

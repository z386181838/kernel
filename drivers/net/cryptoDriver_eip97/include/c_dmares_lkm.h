/* c_dmares_lkm.h
 *
 * Configuration Handling for Driver Framework DMAResource API implementation.
 * A build-level configuration file is included and sanity-checked.
 * Do not edit this file. Edit cs_hwpal_lkm.h instead.
 *
 */

/*****************************************************************************
* Copyright (c) 2010-2013 INSIDE Secure B.V. All Rights Reserved.
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

/*----------------------------------------------------------------
 * get configuration settings from product config file
 */
#include "cs_hwpal_lkm.h"

// choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#ifndef HWPAL_LOG_SEVERITY
#define HWPAL_LOG_SEVERITY  LOG_SEVERITY_INFO
#endif

#ifndef HWPAL_DMA_NRESOURCES
#error "Expected HWPAL_DMA_NRESOURCES defined by cs_hwpal_lkm.h"
#endif

#if HWPAL_DMA_NRESOURCES < 8
#error "HWPAL_DMA_NRESOURCES too small"
#endif


/*----------------------------------------------------------------
 * Other configuration parameters that can be set in a top level
 * configuration
 */

/* HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT
 *
 * Enable this parameter in order to use the coherent Linux DMA mappings,
 * all allocated DMA buffers will be cache-coherent.
 */

/* HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL
 *
 * Enable this parameter in order to use the full Linux DMA API,
 * this will also allow the Linux Kernel to perform more sanity checks
 * on the provided DMA buffers
 */

#if defined(HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT) && \
    defined(HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL)
#error "Error: Cannot define both HWPAL_DMARESOURCE_ALLOC_CACHE_COHERENT and" \
       " HWPAL_DMARESOURCE_MINIMUM_CACHE_CONTROL at the same time"
#endif

// Note: D-cache line size,
//       this can be customized for some platforms to other value.
//       The implementation will ask OS for this parameter if not defined.
#if 0
#ifndef HWPAL_DMARESOURCE_DCACHE_LINE_SIZE
#ifdef HWPAL_64BIT_HOST
// Note: D-cache line size,
//       this can be customized for some platforms to other value
#define HWPAL_DMARESOURCE_DCACHE_LINE_SIZE          64 // 64 bytes
#else
#define HWPAL_DMARESOURCE_DCACHE_LINE_SIZE          32 // 32 bytes
#endif
#endif // !HWPAL_DMARESOURCE_DCACHE_LINE_SIZE
#endif


/* end of file c_dmares_lkm.h */

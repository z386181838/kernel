/* eip97_global_fsm_stub.c
 *
 * EIP-97 Global Control Driver Library API State Machine Internal Interface
 * stub implementation
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

#include "eip97_global_fsm.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                // IDENTIFIER_NOT_USED

// EIP-97 Driver Library Types API
#include "eip97_global_types.h"        // EIP97_Global_* types

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP97_Global_State_Set
 *
 */
EIP97_Global_Error_t
EIP97_Global_State_Set(
        EIP97_Global_State_t * const CurrentState,
        const EIP97_Global_State_t NewState)
{
    IDENTIFIER_NOT_USED(CurrentState);
    IDENTIFIER_NOT_USED((bool)NewState);

    return EIP97_GLOBAL_NO_ERROR;
}


/* end of file eip97_global_fsm_stub.c */

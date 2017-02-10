/* eip202_rdr_fsm.c
 *
 * EIP-202 Ring Control Driver Library API
 * State Machine Internal Interface implementation for RDR
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

#include "eip202_rdr_fsm.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"              // IDENTIFIER_NOT_USED

// EIP-202 Ring Control Driver Library Types API
#include "eip202_ring_types.h"        // EIP202_Ring_* types


/*----------------------------------------------------------------------------
 * EIP202_RDR_State_Set
 *
 */
EIP202_Ring_Error_t
EIP202_RDR_State_Set(
        volatile EIP202_RDR_State_t * const CurrentState,
        const EIP202_RDR_State_t NewState)
{
#ifdef EIP202_RING_DEBUG_FSM
    switch(*CurrentState)
    {
        case EIP202_RDR_STATE_UNKNOWN:
            switch(NewState)
            {
                case EIP202_RDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

         case EIP202_RDR_STATE_UNINITIALIZED:
            switch(NewState)
            {
                case EIP202_RDR_STATE_INITIALIZED:
                   *CurrentState = NewState;
                   break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_RDR_STATE_INITIALIZED:
            switch(NewState)
            {
                case EIP202_RDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FREE:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FULL:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_RDR_STATE_FREE:
            switch(NewState)
            {
                case EIP202_RDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FULL:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FREE:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_RDR_STATE_FULL:
            switch(NewState)
            {
                case EIP202_RDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FREE:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                case EIP202_RDR_STATE_FULL:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_RDR_STATE_FATAL_ERROR:
            switch(NewState)
            {
                case EIP202_RDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        default:
            return EIP202_RING_ILLEGAL_IN_STATE;
    }
#else
    IDENTIFIER_NOT_USED(CurrentState);
    IDENTIFIER_NOT_USED(NewState);
#endif // EIP202_RING_DEBUG_FSM

    return EIP202_RING_NO_ERROR;
}


/* end of file eip202_rdr_fsm.c */

/* adapter_pecdev_dma.h
 *
 * Interface to device-specific layer of the PEC implementation.
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

#ifndef ADAPTER_PECDEV_DMA_H_
#define ADAPTER_PECDEV_DMA_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_adapter_pec.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool

// SLAD PEC API
#include "api_pec.h"

// SLAD DMABuf API
#include "api_dmabuf.h"

#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
#include "adapter_interrupts.h"
#endif


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Capabilities_Get
 *
 * This routine returns a structure that describes the capabilities of the
 * implementation. See description of PEC_Capabilities_t for details.
 *
 * Capabilities_p
 *     Pointer to the capabilities structure to fill in.
 *
 * This function is re-entrant.
 */
PEC_Status_t
Adapter_PECDev_Capabilities_Get(
        PEC_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Control_Write
 *
 * Write the Control1 and Control2 engine-specific fields in the
 * Command Descriptor The other fields (such as SrcPkt_ByteCount and
 * Bypass_WordCount must have been filled in already.
 *
 * Command_p (input, output)
 *     Command descriptor whose Control1 and Control2 fields must be filled in.
 *
 * PacketParams_p (input)
 *     Per-packet parameters.
 *
 * This function is not implemented for all engine types.
 */
PEC_Status_t
Adapter_PECDev_CD_Control_Write(
        PEC_CommandDescriptor_t *Command_p,
        const PEC_PacketParams_t *PacketParams_p);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_RD_Status_Read
 *
 * Read the engine-specific Status1 and Status2 fields from a Result Descriptor
 * and convert them to an engine-independent format.
 *
 * Result_p (input)
 *     Result descriptor.
 *
 * ResultStatus_p (output)
 *     Engine-independent status information.
 *
 * Not all error conditions can occur on all engine types.
 */
PEC_Status_t
Adapter_PECDev_RD_Status_Read(
        const PEC_ResultDescriptor_t * const Result_p,
        PEC_ResultStatus_t * const ResultStatus_p);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Init
 *
 * This function must be used to initialize the service. No API function may
 * be used before this function has returned.
 */
PEC_Status_t
Adapter_PECDev_Init(
        const unsigned int InterfaceId,
        const PEC_InitBlock_t * const InitBlock_p);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_UnInit
 *
 * This call un-initializes the service. Use only when there are no pending
 * transforms. The caller must make sure that no API function is used while or
 * after this function executes.
 */
PEC_Status_t
Adapter_PECDev_UnInit(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SA_Prepare
 *
 * This function performs device-specific preparation of the SA record(s).
 *
 * At this point the SA Record is stored in memory in the byte order
 * required by the hardware, which may be different from the host byte order.
 * Hence any accesses must be done with DMAResource_Read32() and
 * DMAResource_Write32().
 *
 * Note: The resource is not owned by the hardware yet.
 *
 * SAHandle (input)
 *   DMA handle representing the main SA Record.
 *
 * StateHandle (input)
 *   DMA handle representing the State Record (null handle if not in use)
 *
 * ARC4Handle (input)
 *   DMA handle representing the ARC4 State Record (null handle if not in use)
 *
 * Return code
 *   See PEC_Status_t
 */
PEC_Status_t
Adapter_PECDev_SA_Prepare(
        const DMABuf_Handle_t SAHandle,
        const DMABuf_Handle_t StateHandle,
        const DMABuf_Handle_t ARC4Handle);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SA_Remove
 *
 * This function performs device-specific removal of the SA record(s).
 *
 * SAHandle (input)
 *   DMA handle representing the main SA Record.
 *
 * StateHandle (input)
 *   DMA handle representing the State Record (null handle if not in use)
 *
 * ARC4Handle (input)
 *   DMA handle representing the ARC4 State Record (null handle if not in use)
 *
 * At this point the SA record is stored in memory in the byte order
 * required by the hardware, which may be different from the host byte order.
 * Hence any accesses must be done with DMAResource_Read32() and
 * DMAResource_Read32().
 *
 * Note: The record is not owned by the hardware anymore.
 *
 * Return code
 *   See PEC_Status_t
 */
PEC_Status_t
Adapter_PECDev_SA_Remove(
        const DMABuf_Handle_t SAHandle,
        const DMABuf_Handle_t StateHandle,
        const DMABuf_Handle_t ARC4Handle);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_GetFreeSpace
 *
 * Return the number of free slots in the indicated ring. When
 * Adapter_PECDev_PacketPut is called with no more than that many
 * valid command descriptors and none of these require scatter/gather,
 * the operation is guaranteed to succeed, unless a fatal error
 * happens in the meantime.
 */
unsigned int
Adapter_PECDev_GetFreeSpace(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_PacketPut
 *
 * Submit one or more non-scatter-gather packets to the device.
 *
 * InterfaceId (input)
 *     Represents the ring to which packets are submitted.
 *
 * Commands_p (input)
 *     Pointer to one (or an array of command descriptors, each describe one
 *     transform request.
 *
 * CommandsCount (input)
 *     The number of command descriptors pointed to by Commands_p.
 *
 * PutCount_p (output)
 *     This parameter is used to return the actual number of descriptors that
 *     was queued up for processing (0..CommandsCount).
 *
 * If bounce buffers were required for any of the DMA resources, the
 * supplied handles contain the BounceHandle field in their DMA
 * resource records and the data has been copied to bounce
 * buffers. The packet token is stored in the DMA buffer in the
 * required byte order (this is the case for all DMA resources). All
 * DMA resources are owned by the hardware.
 */
PEC_Status_t
Adapter_PECDev_Packet_Put(
        const unsigned int InterfaceId,
        const PEC_CommandDescriptor_t * Commands_p,
        const unsigned int CommandsCount,
        unsigned int * const PutCount_p);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Packet_Get
 *
 * Retrieve zero or more non-scatter-gather packets from the device.
 *
 * InterfaceId (input)
 *     Represents the ring to which packets are submitted.
 *
 * Results_p (input)
 *     Pointer to the result descriptor, or array of result descriptors, that
 *     will be populated by the service based on completed transform requests.
 *
 * ResultsLimit (input)
 *     The number of result descriptors available from Results_p and onwards.
 *
 * GetCount_p (output)
 *     The actual number of result descriptors that were populated is returned
 *     in this parameter.
 *
 * Only the DstPkt_ByteCount, Status1 and Status2 fields will be filled in for
 * each result descriptor. All DMA resources are still owned by the hardware.
 */
PEC_Status_t
Adapter_PECDev_Packet_Get(
        const unsigned int InterfaceId,
        PEC_ResultDescriptor_t * Results_p,
        const unsigned int ResultsLimit,
        unsigned int * const GetCount_p);


#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
/*----------------------------------------------------------------------------
 * Adapter_PECDev_TestSG
 *
 * Test whether a single packet with the indicated number of gather
 * particles and scatter particles can be processed successfully on
 * the indicated ring.
 *
 * If GatherParticleCount is zero, the packet does not use Gather.
 * If ScatterParticleCount is zero, the packet does not use Scatter.
 */
bool
Adapter_PECDev_TestSG(
        const unsigned int InterfaceId,
        const unsigned int GatherParticleCount,
        const unsigned int ScatterParticleCount);
#endif // ADAPTER_PEC_ENABLE_SCATTERGATHER


#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * Adapter_PECDev_IRQToInteraceID
 *
 * Convert the interrupt number to the Interface ID for which this
 * interrupt was generated.
 */
unsigned int
Adapter_PECDev_IRQToInferfaceId(
        const int nIRQ);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Enable_ResultIRQ
 *
 * Enable the interrupt or interrupts for available result packets on the
 * specified interface.
 */
void
Adapter_PECDev_Enable_ResultIRQ(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Disable_ResultIRQ
 *
 * Disable the interrupt or interrupts for available result packets on the
 * specified interface.
 */
void
Adapter_PECDev_Disable_ResultIRQ(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Enable_CommandIRQ
 *
 * Enable the interrupt or interrupts that indicate that the specified
 * interface can accept commands.
 */
void
Adapter_PECDev_Enable_CommandIRQ(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Disable_CommandIRQ
 *
 * Disable the interrupt or interrupts that indicate that the specified
 * interface can accept commands.
 */
void
Adapter_PECDev_Disable_CommandIRQ(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SetResultHandler
 *
 * Set a handler function for the result interrupt on the indicated
 * interface.
 */
void Adapter_PECDev_SetResultHandler(
    const unsigned int InterfaceId,
    Adapter_InterruptHandler_t HandlerFunction);


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SetCommandHandler
 *
 * Set a handler function for the command interrupt on the indicated
 * interface.
 */
void Adapter_PECDev_SetCommandHandler(
        const unsigned int InterfaceId,
        Adapter_InterruptHandler_t HandlerFunction);
#endif // ADAPTER_PEC_INTERRUPTS_ENABLE


#endif /* ADAPTER_PECDEV_DMA_H_ */


/* end of file adapter_pecdev_dma.h */

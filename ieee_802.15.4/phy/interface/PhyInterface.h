/*! *********************************************************************************
* \defgroup PhyInterface PHY Interface
*
* The PHY sublayer provides two services: the PHY data service, and the PHY management service interfacing to the PHY sublayer management entity (PLME) service access point (SAP) (known as PLME-SAP). 
* The PHY data service enables the transmission and reception of PHY protocol data units (PSDUs) over the media (radio).
* The PHY Layer interfaces to the MAC Layer through function calls and function callbacks.
* If the interface primitives are implemented as function calls, the MAC Layer calls the exposed functions (provided by the PHY Layer) to issue commands / requests.
*
* @{
********************************************************************************** */
/*!
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
*
* \file
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _PHY_INTERFACE_H
#define _PHY_INTERFACE_H


/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "PhyTypes.h"
#include "PhyMessages.h"
#include "Messaging.h"


/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

/*! Number of PHY instances (Only for PHYs which support multiple instances) */
#ifndef gPhyInstancesCnt_c
#define gPhyInstancesCnt_c  1
#endif

/*! The PHY task stack size (Only for PHYs which use OS task) */
#ifndef gPhyTaskStackSize_c
#define gPhyTaskStackSize_c 600
#endif

/*! The PHY task priority (Only for PHYs which use OS task) 
    The PHY Task must have the highest priority! */
#ifndef gPhyTaskPriority_c
#define gPhyTaskPriority_c  0
#endif

/*! The following define is used to enable the Automatic Frequency Correction (MKW01 only) */
#ifndef gAfcEnabled_d
#define gAfcEnabled_d		0
#endif

/*! Configure the maximum number of PHY timers/events */
#ifndef gMaxPhyTimers_c
#define gMaxPhyTimers_c                 (5)
#endif

/*! This define is used as a start time to signal that a current sequence is handled as soon as possible by the PHY layer.  */
#define gPhySeqStartAsap_c              ((phyTime_t)(-1))

/*! The MemManager Poll Id used by the PHY layer */
#ifndef gPhyPoolId_d
#define gPhyPoolId_d 0
#endif

/*! \cond DOXY_SKIP_TAG */
#define Phy_BufferAlloc(size) MEM_BufferAllocWithId(size,gPhyPoolId,(void*)__get_LR())
#define Phy_BufferAllocForever(size) MEM_BufferAllocForever(size,gPhyPoolId)
/*! \endcond */


/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/*! PHY power states. These modes are hardware specific.
    Some modes may not be available on certain HW platforms. */
typedef enum{
    gPhyPwrIdle_c,
    gPhyPwrAutodoze_c,
    gPhyPwrDoze_c,
    gPhyPwrHibernate_c,
    gPhyPwrDSM_c,
    gPhyPwrReset_c
}phyPwrMode_t;

/*! The PD-DATA.request primitive is generated by the MAC Layer when a MAC data frame (MPDU) is ready to be transferred to the PHY Layer, becoming payload for the PHY frame (PSDU). 
    Upon the reception of this primitive, the PHY Layer will arm either a T or TRxAck sequence, depending on the Acknowledgement Request sub-field bit included in the Frame Control field, which is part of the MHR. */
typedef struct pdDataReq_tag
{ 
    phyTime_t               startTime;  /*!< The absolute start time of the Data Request sequence. A value of gPhySeqStartAsap_c to start immediately.  */  
    uint32_t                txDuration; /*!< The computed duration for the Data Request frame. */
    phySlottedMode_t        slottedTx;
    phyCCAType_t            CCABeforeTx; /*!< Indicates, whether the CCA is used before this transmission. */
    phyAckRequired_t        ackRequired; /*!< Indicates, whether the ACK is required for this transmission. */
    uint8_t                 psduLength;  /*!< The number of octets contained in the PSDU to be transmitted by the PHY Layer, without the last two octets containing the FCS field. */
#ifdef gPHY_802_15_4g_d	
    phyPHR_t                phyHeader;    /*!< Sub-1 GHz PHY only. Used to form the PHY header before sending. */
    uint8_t                 macDataIndex; /*!< Sub-1 GHz PHY only. Used to store the index of the currently sent byte. */
    uint8_t                 fillFifoBlockLength; /*!< Sub-1 GHz PHY only. The block length to be pre-filled in transceiverís FIFO. */
#endif  /* gPHY_802_15_4g_d */	
    uint8_t *               pPsdu;        /*!< A pointer to the set of octets forming the PSDU to be transmitted by the PHY Layer. */
} pdDataReq_t;

/*! The PD-DATA.confirm primitive reports the result of a request to transfer a data MAC frame (MPDU). 
    The status returned by PD-DATA.confirm can be SUCCESS, indicating that the request to transmit was successful, an error code of BUSY if the PHY Layer 
    was not in the idle state (I sequence) when the PD-DATA.request was issued, or an error code of CHANNEL_BUSY if all CCA sequences indicated that the channel was busy. 
    If the transmission occurred successfully but no valid ACK frame was received (assuming that it was requested), then an error code of NO_ACK is used. */
typedef struct pdDataCnf_tag
{
    phyStatus_t             status;
} pdDataCnf_t;

/*! The PD-DATA.indication primitive is generated by the PHY Layer when an MPDU is ready to be transferred to the MAC Layer. 
    Besides the PSDU itself, the primitive also returns the LQI value measured during the reception. */
typedef struct pdDataInd_tag
{
    phyTime_t               timeStamp;       /*!< The timestamp when the reception started. */
    uint8_t                 ppduLinkQuality; /*!< Link quality (LQI) value measured during the reception of the PPDU. */
    uint8_t                 psduLength;      /*!< The number of octets contained in the PSDU received by the PHY Layer. */
    uint8_t *               pPsdu;           /*!< The pointer to the set of octets forming the PSDU received by the PHY Layer. */
#ifdef gSnifferCRCEnabled_d	
    uint32_t                crcValue;
    bool_t                  crcValid;
#endif
} pdDataInd_t;

/*! The PD-INDQUEUEINSERT.Request primitive is generated by the MAC Layer when a packet is inserted into the MAC indirect queue. 
    A 16-bit checksum derived from the Destination Address and Destination PAN ID is passed to the PHY Layer. 
    The PHY Layer, both in hardware implementations and emulated in software, must keep a database of checksums, and facilitate writing into it through the use of this primitive. */
typedef struct pdIndQueueInsertReq_tag
{
    uint8_t                 index;    /*!< Index into the indirect table */
    uint16_t                checksum;
} pdIndQueueInsertReq_t;

/*! Unused! The request is synchronous. */
typedef struct pdIndQueueInsertCnf_tag
{
    phyStatus_t             status;
} pdIndQueueInsertCnf_t;

/*! The PD-INDQUEUEREMOVE.Request primitive is generated by the MAC Layer when a packet is removed from the MAC indirect queue, and the index, at which the packetís checksum is stored, gets passed on to the PHY Layer. 
    The PHY Layer, both in hardware implementations and emulated in software, must facilitate erasing entries from its database of checksums through the use of this primitive. */
typedef struct pdIndQueueRemoveReq_tag
{
    uint8_t                 index; /*!< Index into the indirect table */
} pdIndQueueRemoveReq_t;

/*! The PLME-ED.request primitive is generated by the MAC Layer when an ED measurement must be performed by the PHY Layer. */
typedef struct plmeEdReq_tag
{
    phyTime_t               startTime;  /*!< absolute time */
} plmeEdReq_t;

/*! The PLME-CCA.request primitive is generated by the MAC Layer when a CCA operation must be performed, and it is passed to the PHY Layer. */
typedef struct plmeCcaReq_tag
{
    phyCCAType_t            ccaType;
    phyContCCAMode_t        contCcaMode; /*!< Enable continuous CCA mode */
} plmeCcaReq_t;

/*! The PLME-CCA.confirm primitive is generated by the PHY Layer after the C sequence completes, and returns the response of a previous PLME-CCA.request to the MAC Layer. 
    The status returned can be IDLE if the channel is idle, RX_ON if the transceiver is receiving, or BUSY if there is an ongoing T sequence or the channel assessment process determined that the channel is busy. */
typedef struct plmeCcaCnf_tag
{
    phyStatus_t             status;
} plmeCcaCnf_t;

/*! The PLME-ED.confirm primitive is generated by the PHY Layer after the C sequence completes, and returns the response of a previous PLME-ED.request to the MAC Layer. 
    The status returned can be SUCCESS if the measurement was successful, an error code of TX_ON if there is an ongoing T sequence, or RX_ON if the transceiver is receiving. 
    The PLME-ED.confirm primitive returns the value of the ED measurement. */
typedef struct plmeEdCnf_tag
{
    phyStatus_t             status;
    uint8_t                 energyLevel;   /*!< The ED level for the current channel. If the status is not SUCCESS, the value of this parameter will be ignored. */
    uint8_t                 energyLeveldB; /*!< The ED level for the current channel in dBm value. */
} plmeEdCnf_t;

/*! The PLME-SET-TRX-STATE.request primitive is generated by the MAC Layer when the transceiver state must be changed by the PHY Layer, which then arms either an I or R sequence. 
    This primitive is also used to cancel any ongoing sequence by setting the state to FORCE_TRX_OFF. If this primitive is issued with an RX_ON or TRX_OFF argument and the PHY is busy transmitting a PPDU, a state change will occur at the end of the transmission. 
    If this primitive is issued with TRX_OFF, and the PHY is in the RX_ON state and has already received a valid SFD, the state change will occur at the end of reception of the PPDU.
    The slottedTx parameter is used by the PHY Layer during R sequence to determine whether the ensuing transmit acknowledge frame (if any) must be synchronized to a backoff slot boundary. */
typedef struct plmeSetTRxStateReq_tag
{
    phyState_t              state;       /*!< The new state in which to configure the transceiver */
    phySlottedMode_t        slottedMode; /*!< Slotted/non-slotted mode */
    phyTime_t               startTime;   /*!< Absolute time */
    uint32_t                rxDuration;  /*!< If the requested state is Rx, then Rx will be enabled for rxDuration symbols. */
} plmeSetTRxStateReq_t;

/*! PHY Event */
typedef struct phyTimeEvent_tag
{
    phyTime_t          timestamp; /*!< Absolute time of the event */
    phyTimeCallback_t  callback;  /*!< Callback function to handle the event */
    uint32_t           parameter; /*!< Parameter to be specified to the callback function */
}phyTimeEvent_t;

/*! PLME-SET-TRX-STATE.Confirm */
typedef struct plmeSetTRxStateCnf_tag
{
    phyStatus_t             status;
} plmeSetTRxStateCnf_t;

/*! The PLME-SET.request primitive is generated by the MAC Layer to modify a PIB attribute in the PHY Layer. 
    This primitive requires the identifier of the PIB attribute to set its value. 
    This request is synchronous. The PIB value will be set inside the call to the PLME SAP. */
typedef struct plmeSetReq_tag
{
    phyPibId_t              PibAttribute;
    uint64_t                PibAttributeValue;
} plmeSetReq_t;

/*! PLME-SET.Confirm - Unused! The request is synchronous. */
typedef struct plmeSetCnf_tag
{
    phyStatus_t             status;
    phyPibId_t              PibAttribute;
} plmeSetCnf_t;

/*! The PLME-GET.request primitive is generated by the MAC Layer to request information about a PIB attribute in the PHY Layer. 
    This primitive requires the identifier of the PIB attribute to read. 
    This request is synchronous. The PIB value will be available after the call to the PLME SAP. */
typedef struct plmeGetReq_tag
{
    phyPibId_t              PibAttribute;
    uint64_t *              pPibAttributeValue;
} plmeGetReq_t;

/*! PLME-GET.Confirm - Unused! The request is synchronous. */
typedef struct plmeGetCnf_tag
{
    phyStatus_t             status;
    phyPibId_t              PibAttribute;
    uint64_t                PibAttributeValue;
} plmeGetCnf_t;

/*! PHY Management Requests messages */
typedef struct macToPlmeMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        plmeEdReq_t                edReq;
        plmeCcaReq_t               ccaReq;
        plmeSetTRxStateReq_t       setTRxStateReq;
        plmeSetReq_t               setReq;
        plmeGetReq_t               getReq;
    }msgData;
} macToPlmeMessage_t;

/*! PHY Data Requests messages */
typedef struct macToPdDataMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        pdDataReq_t             dataReq;
        pdIndQueueInsertReq_t   indQueueInsertReq;
        pdIndQueueRemoveReq_t   indQueueRemoveReq;
    }msgData;
} macToPdDataMessage_t;

/*! PHY Management Confirms/Indications messages */
typedef struct plmeToMacMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        plmeCcaCnf_t            ccaCnf;
        plmeEdCnf_t             edCnf;
#if 0
        plmeSetTRxStateCnf_t    setTRxStateCnf;
        plmeSetCnf_t            setCnf;
        plmeGetCnf_t            getCnf;
#endif
    }msgData;
} plmeToMacMessage_t;

/*! PHY Data Confirms/Indications messages */
typedef struct pdDataToMacMessage_tag
{
    phyMessageId_t             msgType;
    uint8_t                    macInstance;
    union
    {
        pdDataCnf_t             dataCnf;
        pdDataInd_t             dataInd;
#if 0
        pdIndQueueInsertCnf_t   indQueueInsertCnf;
#endif
    }msgData;
} pdDataToMacMessage_t;

/*! Common PHY message header */
typedef struct phyMessageHeader_tag
{
    phyMessageId_t             msgType;     /*!< The Id of the PHY message */
    uint8_t                    macInstance; /*!< The instance of the upper layer */
} phyMessageHeader_t;

/*! PHY RX parameter */
typedef struct phyRxParams_tag {
    phyTime_t   timeStamp;  /*!< [symbols] : Rx startTime / DataIndication timestamp */
    uint8_t     psduLength; /*!< bytes received */
    uint8_t     linkQuality;/*!< LQI value */
#ifdef gPHY_802_15_4g_d	    
    uint8_t     headerLength; 
    uint8_t     macDataIndex;
    uint8_t     fifoBlockLen;
    phyPHR_t    phyHeader;
#else
    uint32_t              duration; /*!< [symbols] */
    pdDataToMacMessage_t *pRxData;  /*!< Pointer where received data will be stored */
    pdDataToMacMessage_t *pDataPending; /*!< Pointer to pending received data */
    phySlottedMode_t      phyRxMode;/*!< Slotted/Non-slotted RX */
#endif  /* gPHY_802_15_4g_d */  
} phyRxParams_t;

/*! PHY Channel parameters */
typedef struct phyChannelParams_tag {
    union{
        phyStatus_t channelStatus; /*!< Channel busy or idle */
        uint8_t     energyLeveldB; /*!< Channel energy level in dBm */
    };
#ifdef gPHY_802_15_4g_d
    uint8_t     ccaParam;
#endif   /* gPHY_802_15_4g_d */	
} phyChannelParams_t;

/*! Passed by the MAC layer to specify that a standalone CCA must be used, or that the ACK is required for the sequence. */
typedef struct phyTxParams_tag {
  uint8_t           numOfCca;    /*!< The number of CCA sequences before TX */
  phyAckRequired_t  ackRequired; /*!< Specify if PHY should wait for ACK */
} phyTxParams_t;

/*! PHY data service callback type */
typedef phyStatus_t ( * PD_MAC_SapHandler_t)(pdDataToMacMessage_t * pMsg, instanceId_t instanceId);

/*! PHY management service callback type */
typedef phyStatus_t ( * PLME_MAC_SapHandler_t)(plmeToMacMessage_t * pMsg, instanceId_t instanceId);

#ifdef __cplusplus
extern "C" {
#endif 

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/*! \cond DOXY_SKIP_TAG */
extern const uint8_t gPhyIndirectQueueSize_c;
extern const uint8_t gPhyPoolId;
/*! \endcond */

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief initialize the PHY module
 *
 ********************************************************************************** */
void Phy_Init( void );

/*! *********************************************************************************
 * \brief This function creates a logical binding with the next available 
 *        (un-binded) PHY instance.
 *
 * \param macInstance Upper layer instance id
 *
 * \return instance of the PHY
 *
 ********************************************************************************** */
instanceId_t BindToPHY( instanceId_t macInstance );

/*! *********************************************************************************
 * \brief Register upper layer callback function to handle the PHY confirms/indications
 *
 * \param pPD_MAC_SapHandler    Upper layer callback function for Data messages
 * \param pPLME_MAC_SapHandler  Upper layer callback function for Management messages
 * \param instanceId            Instance of the PHY layer
 *
 ********************************************************************************** */
void Phy_RegisterSapHandlers(PD_MAC_SapHandler_t pPD_MAC_SapHandler, PLME_MAC_SapHandler_t pPLME_MAC_SapHandler, instanceId_t instanceId);

/*! *********************************************************************************
 * \brief This is the entry point for the PHY data service requests
 *
 * \param pMsg        Pointer to the request
 * \param instanceId  Instance of the PHY layer
 *
 * \return phyStatus_t
 *
 ********************************************************************************** */
phyStatus_t MAC_PD_SapHandler(macToPdDataMessage_t * pMsg, instanceId_t phyInstance);

/*! *********************************************************************************
 * \brief This is the entry point for the PHY management service requests
 *
 * \param pMsg        Pointer to the request
 * \param instanceId  Instance of the PHY layer
 *
 * \return phyStatus_t
 *
 ********************************************************************************** */
phyStatus_t MAC_PLME_SapHandler(macToPlmeMessage_t * pMsg, instanceId_t phyInstance);

/*! *********************************************************************************
 * \brief This function will schedule the next event to expire
 *
 ********************************************************************************** */
void PhyTime_Maintenance ( void );

/*! *********************************************************************************
 * \brief This function will run the callback of the next event scheduled
 *
 ********************************************************************************** */
void PhyTime_RunCallback ( void );

/*! *********************************************************************************
 * \brief Function to be called from the PHY timer compare match ISR
 *
 ********************************************************************************** */
void PhyTime_ISR         ( void );

/*! *********************************************************************************
 * \brief Initialize the PHY timer module
 *
 * \param cb Callback function used to handle a timer compare match event.
 *           If NULL, the PhyTime_RunCallback() and the PhyTime_Maintenance()
 *           functions will be called from ISR context
 *
 * \return phyTimeStatus_t
 *
 ********************************************************************************** */
phyTimeStatus_t    PhyTime_TimerInit ( void (*cb)(void) );

/*! *********************************************************************************
 * \brief Return a 64-bit time-stamp in symbols
 *
 * \return time-stamp
 *
 ********************************************************************************** */
phyTime_t          PhyTime_GetTimestamp ( void );

/*! *********************************************************************************
 * \brief This function schedules a timed event. 
 *        The event context is given by the configuration structure.
 *
 * \param pEvent Pointer to the event data
 *
 * \return Id of the event, or gInvalidTimerId_c if the event cannot be scheduled 
 *
 ********************************************************************************** */
phyTimeTimerId_t   PhyTime_ScheduleEvent( phyTimeEvent_t *pEvent );

/*! *********************************************************************************
 * \brief Cancel a specific scheduled event
 *
 * \param timerId The id of the event to schedule
 *
 * \return phyTimeStatus_t
 *
 ********************************************************************************** */
phyTimeStatus_t    PhyTime_CancelEvent  ( phyTimeTimerId_t timerId );

/*! *********************************************************************************
 * \brief Cancel all schedules events which has a specific parameter specified
 *
 * \param param Parameter to match the event(s) to be cancel
 *
 * \return phyTimeStatus_t
 *
 ********************************************************************************** */
phyTimeStatus_t    PhyTime_CancelEventsWithParam ( uint32_t param );

#ifdef gPHY_802_15_4g_d
/*! *********************************************************************************
 * \brief Return a 64-bit time-stamp in microseconds
 *
 * \return time-stamp
 *
 ********************************************************************************** */
phyTime_t PhyTime_GetTimestampUs(void);

/*! *********************************************************************************
 * \brief Converts PHY symbols to microseconds
 *
 * \return time-stamp
 *
 ********************************************************************************** */
phyTime_t PhyTime_GetSymbolsToUs(phyTime_t tsSymbols);

/*! *********************************************************************************
 * \brief Converts microseconds to PHY symbols
 *
 * \return time-stamp
 *
 ********************************************************************************** */
phyTime_t PhyTime_GetUsToSymbols(phyTime_t tsUs);
#endif

/*! *********************************************************************************
 * \brief Set the low-power state of the PHY
 *
 * \param state  The new power state
 *
 * \return phyStatus_t
 *
 ********************************************************************************** */
phyStatus_t        PhyPlmeSetPwrState( uint8_t state );

#ifdef __cplusplus
}
#endif 
    
#endif  /* _PHY_INTERFACE_H */
/*! *********************************************************************************
* @}
********************************************************************************** */
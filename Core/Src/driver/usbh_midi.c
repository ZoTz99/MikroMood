/**
  ******************************************************************************
  * @file    usbh_midi.c
  * @author  Bianchi Davide
  * @brief   This file is the MIDI Layer Handlers for USB Host MIDI class.
  *
  ******************************************************************************
**/

/* Includes ------------------------------------------------------------------*/
#include "driver/usbh_midi.h"

/*------------------------------------------------------------------------------------------------------------------------------*/


/** @defgroup USBH_MIDI_CORE_Private_FunctionPrototypes
  * @{
  */
    static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost);
    static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost);
    static USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost);
    static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost);
    static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost);
    static void MIDI_ProcessTransmission(USBH_HandleTypeDef *phost);
    static void MIDI_ProcessReception(USBH_HandleTypeDef *phost);

    USBH_ClassTypeDef  MIDI_Class =
    {
        "MIDI",
		USB_AUDIO_CLASS,
        USBH_MIDI_InterfaceInit,
        USBH_MIDI_InterfaceDeInit,
        USBH_MIDI_ClassRequest,
        USBH_MIDI_Process,
        USBH_MIDI_SOFProcess,
        NULL,
    };
/* Per tastiera LIM
    USBH_ClassTypeDef  MIDI_Class =
   {
	   "MIDI",
	   0x0A,
	   USBH_MIDI_InterfaceInit,
	   USBH_MIDI_InterfaceDeInit,
	   USBH_MIDI_ClassRequest,
	   USBH_MIDI_Process,
	   USBH_MIDI_SOFProcess,
	   NULL,
   };
*/
/**
  * @}
  */

/** @defgroup USBH_MIDI_CORE_Private_Functions
  * @{
  */

/**
  * @brief  USBH_MIDI_InterfaceInit
  *         The function init the MIDI class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status;
    uint8_t interface;
    MIDI_HandleTypeDef *MIDI_Handle;

    // Step_1: find the right interface to handle usb data (this function returns the index of the interface)
    interface = USBH_FindInterface(phost, USB_AUDIO_CLASS, USB_MIDISTREAMING_SUBCLASS, 0xFF);

    // interface = USBH_FindInterface(phost, 0x0A, 1, 0xFF); // Per tastiera LIM

    if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) /* No Valid Interface */
    {
        USBH_DbgLog("Cannot Find the interface for MIDI Interface Class.", phost->pActiveClass->Name);
        return USBH_FAIL;
    }

    // Step_2: select the interface given the index (aka the 'interface variable')
    status = USBH_SelectInterface(phost, interface);
    if (status != USBH_OK)
    {
        return USBH_FAIL;
    }

    // Step_3: in the USBH_HandleTypeDef associate to the active class the handle of the MIDI (MIDI_HandleTypeDef)
    phost->pActiveClass->pData = (MIDI_HandleTypeDef *)USBH_malloc(sizeof(MIDI_HandleTypeDef));
    MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;
    if (MIDI_Handle == NULL)
    {
        USBH_DbgLog("Cannot allocate memory for MIDI Handle");
        return USBH_FAIL;
    }
    /* Initialize MIDI handler */
    (void)USBH_memset(MIDI_Handle, 0, sizeof(MIDI_HandleTypeDef));


    // Step_4: Collect the class specific endpoint address and length
    // Check if endpoint_0 is in/out
    if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) != 0U)  // 0x80 = 1000_0000 see pag. 25 USB_MIDI_STANDARD
    {
        // In this branch bEndAdd = 1000_0000: this means bit 7 is set to 1 and in pag.25 the D7 is the direction bit: 0->out_endpoint; 1->in_endpoint

        // InEp = input endpoint_0 address
        MIDI_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
        // InEpSize = max_packet_size of the in_endpoint_0 aka standard size of the endpoint
        MIDI_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
    }
    else    // If D7 is 0 -> out_endpoint (pag. 25)
    {
        // OutEp = output endpoint_0 address
        MIDI_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
        // OutEpSize = max_packet_size of the out_endpoint_0 aka standard size of the endpoint
        MIDI_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
    }

    // Check if endpoint_1 is in/out
    if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80U) != 0U)
    {
        // InEp = input endpoint_1 address
        MIDI_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
        // InEpSize = max_packet_size of the in_endpoint_1 aka standard size of the endpoint
        MIDI_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
    }
    else
    {
        // OutEp = output endpoint_1 address
        MIDI_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
        // OutEpSize = max_packet_size of the out_endpoint_1 aka standard size of the endpoint
        MIDI_Handle->DataItf.OutEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
    }

    // Step_5: allocating and opening the pipe
    /*Allocate the length for host channel number out*/
    MIDI_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, MIDI_Handle->DataItf.OutEp);   // Returns the index of the allocated pipe

    /*Allocate the length for host channel number in*/
    MIDI_Handle->DataItf.InPipe = USBH_AllocPipe(phost, MIDI_Handle->DataItf.InEp);

    /* Open channel for OUT endpoint */
    (void)USBH_OpenPipe(phost, MIDI_Handle->DataItf.OutPipe, MIDI_Handle->DataItf.OutEp,
                        phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                        MIDI_Handle->DataItf.OutEpSize);

    /* Open channel for IN endpoint */
    (void)USBH_OpenPipe(phost, MIDI_Handle->DataItf.InPipe, MIDI_Handle->DataItf.InEp,
                        phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                        MIDI_Handle->DataItf.InEpSize);

    // Step_6: setting the state to IDLE to start the communication process
    MIDI_Handle->state = MIDI_IDLE_STATE;

    (void)USBH_LL_SetToggle(phost, MIDI_Handle->DataItf.OutPipe, 0U);   // Activate the toggle on the out_pipe
    (void)USBH_LL_SetToggle(phost, MIDI_Handle->DataItf.InPipe, 0U);    // Activate the toggle on the in_pipe

    return USBH_OK;
}

/**
  * @brief  USBH_MIDI_InterfaceDeInit
  *         The function DeInit the Pipes used for the MIDI class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;

    if ((MIDI_Handle->DataItf.InPipe) != 0U)
    {
        (void)USBH_ClosePipe(phost, MIDI_Handle->DataItf.InPipe);
        (void)USBH_FreePipe(phost, MIDI_Handle->DataItf.InPipe);
        MIDI_Handle->DataItf.InPipe = 0U;     /* Reset the Channel as Free */
    }

    if ((MIDI_Handle->DataItf.OutPipe) != 0U)
    {
        (void)USBH_ClosePipe(phost, MIDI_Handle->DataItf.OutPipe);
        (void)USBH_FreePipe(phost, MIDI_Handle->DataItf.OutPipe);   // Normal free in C
        MIDI_Handle->DataItf.OutPipe = 0U;    /* Reset the Channel as Free */
    }

    if ((phost->pActiveClass->pData) != NULL)
    {
        USBH_free(phost->pActiveClass->pData);
        phost->pActiveClass->pData = 0U;
    }

    return USBH_OK;
}

/**
  * @brief  USBH_MIDI_ClassRequest
  *         The function is responsible for handling Standard requests for MIDI class.
  *
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost)
{
    phost->pUser(phost, HOST_USER_CLASS_ACTIVE);   // Pointer to the USBH_UserProcess in the usb_host.c
    return USBH_OK;
}


/**
  * @brief  USBH_MIDI_Process
  *         The function is for managing state machine for MIDI data transfers. It is called from phost->pActiveClass->BgndProcess(phost); (usbh_core.c)
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_BUSY;
    USBH_StatusTypeDef req_status = USBH_OK;
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;

    switch (MIDI_Handle->state)
    {
        case MIDI_IDLE_STATE:
            status = USBH_OK;
        break;

        case MIDI_TRANSFER_DATA:
            //MIDI_ProcessTransmission(phost);
            MIDI_ProcessReception(phost);
        break;

        case MIDI_ERROR_STATE:
            req_status = USBH_ClrFeature(phost, 0x00U);
            if (req_status == USBH_OK)
            {
                /*Change the state to waiting*/
                MIDI_Handle->state = MIDI_IDLE_STATE;
            }
        break;

        default:
        break;

    }

    return status;
}

/**
  * @brief  USBH_MIDI_SOFProcess
  *         The function is for managing SOF callback (Start of frame: used for synchronization)
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(phost);
    return USBH_OK;
}


/**
  * @brief  USBH_MIDI_Stop
  *         Stop current MIDI Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_MIDI_Stop(USBH_HandleTypeDef *phost)
{
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;

    if (phost->gState == HOST_CLASS)    // called in the USBH_Process in usbh_core.c
    {
        MIDI_Handle->state = MIDI_IDLE_STATE;
        (void)USBH_ClosePipe(phost, MIDI_Handle->DataItf.InPipe);       // Close the input pipe
        (void)USBH_ClosePipe(phost, MIDI_Handle->DataItf.OutPipe);      // Close the output pipe
    }
    return USBH_OK;
}

/**
  * @brief  USBH_MIDI_GetLastReceivedDataSize
            This function return last received data size
  * @param  None
  * @retval None
  */
uint16_t USBH_MIDI_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
    uint32_t dataSize;
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;

    if (phost->gState == HOST_CLASS)
    {
        dataSize = USBH_LL_GetLastXferSize(phost, MIDI_Handle->DataItf.InPipe);
    }
    else
    {
        dataSize =  0U;
    }

    return (uint16_t)dataSize;
}

/**
  * @brief  USBH_MIDI_Transmit
            This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef USBH_MIDI_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
    USBH_StatusTypeDef Status = USBH_BUSY;
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;

    if ((MIDI_Handle->state == MIDI_IDLE_STATE) || (MIDI_Handle->state == MIDI_TRANSFER_DATA))
    {
        MIDI_Handle->pTxData = pbuff;
        MIDI_Handle->TxDataLength = length;
        MIDI_Handle->state = MIDI_TRANSFER_DATA;
        MIDI_Handle->data_tx_state = MIDI_SEND_DATA;
        Status = USBH_OK;
    }
    return Status;
}


/**
  * @brief  USBH_MIDI_Receive
  *         This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef USBH_MIDI_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
    USBH_StatusTypeDef Status = USBH_BUSY;
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;

    if ((MIDI_Handle->state == MIDI_IDLE_STATE) || (MIDI_Handle->state == MIDI_TRANSFER_DATA))
    {
        MIDI_Handle->pRxData = pbuff;
        MIDI_Handle->RxDataLength = length;
        MIDI_Handle->state = MIDI_TRANSFER_DATA;
        MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA;
        Status = USBH_OK;
    }
    return Status;
}

/**
  * @brief  MIDI_ProcessTransmission
  *         The function is responsible for sending data to the device
  * @param  pdev: Selected device
  * @retval None
  */
static void MIDI_ProcessTransmission(USBH_HandleTypeDef *phost)
{
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;
    USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;    // URB = USB Request Block Status

    switch (MIDI_Handle->data_tx_state)
    {
        case MIDI_SEND_DATA:
            if (MIDI_Handle->TxDataLength > MIDI_Handle->DataItf.OutEpSize) // Data to be transmitted is too big for the endpoint (it must be trasmitted in more packet)
            {
                (void)USBH_BulkSendData(phost,
                                        MIDI_Handle->pTxData,
                                        MIDI_Handle->DataItf.OutEpSize, // Size of the packet trasmitted is the maximum size of the endpoint
                                        MIDI_Handle->DataItf.OutPipe,
                                        1U);
            }
            else    // The packet is smaller than the maximum endpoint size
            {
                (void)USBH_BulkSendData(phost,
                                        MIDI_Handle->pTxData,
                                        (uint16_t)MIDI_Handle->TxDataLength,    // Size of the packet trasmitted is the size of the data
                                        MIDI_Handle->DataItf.OutPipe,
                                        1U);
            }

            MIDI_Handle->data_tx_state = MIDI_SEND_DATA_WAIT;   // After sending the packet of data, turn into wait stage
        break;

        case MIDI_SEND_DATA_WAIT:

            URB_Status = USBH_LL_GetURBState(phost, MIDI_Handle->DataItf.OutPipe);  // Getting the USB status of the block trasmitted
            /* Check the status done for transmission */
            if (URB_Status == USBH_URB_DONE)    // Block of data trasmitted with success
            {
                if (MIDI_Handle->TxDataLength > MIDI_Handle->DataItf.OutEpSize) // If the size of the whole data to be trasmitted if bigger than the max_endpoint_size
                {
                    MIDI_Handle->TxDataLength -= MIDI_Handle->DataItf.OutEpSize;    // Calculate the lenght remaining part to be trasmitted
                    MIDI_Handle->pTxData += MIDI_Handle->DataItf.OutEpSize;         // The pointers is moved ahead of OutEpSize bytes so it can point to the next bytes to be trasmitted
                }
                else    // This means the last packet of bytes trasmitted was the last of the sequence
                {
                    MIDI_Handle->TxDataLength = 0U;
                }

                if (MIDI_Handle->TxDataLength > 0U) // There are still more bytes of data to be trasmitted
                {
                    MIDI_Handle->data_tx_state = MIDI_SEND_DATA;    // The trasmission must continue
                }
                else    // All the bytes of data were trasmitted
                {
                    MIDI_Handle->data_tx_state = MIDI_IDLE; // Return to the idle state
                    USBH_MIDI_TransmitCallback(phost);  // To be implemented
                }
            }
            else
            {
                if (URB_Status == USBH_URB_NOTREADY)    // Block of Data not trasmitted so it must be trasmitted again (MIDI_SEND_DATA)
                {
                    MIDI_Handle->data_tx_state = MIDI_SEND_DATA;    // Send the remaining data
                }
            }
        break;

        default:
        break;
    }
}

/**
  * @brief  MIDI_ProcessReception
  *         This function responsible for reception of data from the device
  * @param  pdev: Selected device
  * @retval None
  */
static void MIDI_ProcessReception(USBH_HandleTypeDef *phost)
{
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;
    USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
    uint32_t length;
    //MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA_WAIT;
/*
    USBH_StatusTypeDef status = USBH_InterruptReceiveData(phost,
                        MIDI_Handle->pRxData,           // Pointer to data to be received
                        MIDI_Handle->DataItf.InEpSize,  // The size of the ep is always the maximum
                        MIDI_Handle->DataItf.InPipe);   // Number of the pipe used for communication
  */

    switch (MIDI_Handle->data_rx_state)
    {
        case MIDI_RECEIVE_DATA:

            (void)USBH_BulkReceiveData(phost,
                                       MIDI_Handle->pRxData,           // Pointer to data to be received
                                       MIDI_Handle->DataItf.InEpSize,  // The size of the ep is always the maximum
                                       MIDI_Handle->DataItf.InPipe);   // Number of the pipe used for communication

            MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA_WAIT;        // After receiving the packet of data, turn into wait stage

        break;

        case MIDI_RECEIVE_DATA_WAIT:

            URB_Status = USBH_LL_GetURBState(phost, MIDI_Handle->DataItf.InPipe);   // Checking the state of the reception

            /*Check the status done for reception*/

            if (URB_Status == USBH_URB_DONE)    // Block of data received with success
            {

                length = USBH_LL_GetLastXferSize(phost, MIDI_Handle->DataItf.InPipe);   // Getting the size of the last block of bytes received

                if (((MIDI_Handle->RxDataLength - length) > 0U) && (length > MIDI_Handle->DataItf.InEpSize))    // If length of the data trasmitted is less than the total data and the length of the data is more than the total size
                {
                    MIDI_Handle->RxDataLength -= length;                // Calculate the length of the remaining part to be received
                    MIDI_Handle->pRxData += length;                     // Pointer to the data received
                    MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA;     // The reception must continue because more bytes need to be sent by the device
                }

                else // This means the last packet of bytes received was the last of the sequence
                {
                    MIDI_Handle->data_rx_state = MIDI_IDLE; // Return to the idle state
                    USBH_MIDI_ReceiveCallback(phost);       // to be implemented
                }
            }
        break;


        default:
        break;
    }
}

/**
  * @brief  USBH_MIDI_TransmitCallback
  *         The function informs user that data have been received
  * @param  pdev: Selected device
  * @retval None
  */
__weak void USBH_MIDI_TransmitCallback(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(phost);
}

/**
  * @brief  USBH_MIDI_ReceiveCallback
  *         The function informs user that data have been sent
  * @param  pdev: Selected device
  * @retval None
  */
__weak void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(phost);
}

/**
  * @}
  */

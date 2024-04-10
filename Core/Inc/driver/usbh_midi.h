/**
  ******************************************************************************
  * @file    usbh_midi.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the usbh_midi.c
  ******************************************************************************
**/

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef INC_DRIVER_USBH_MIDI_H_
#define INC_DRIVER_USBH_MIDI_H_

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"


/** @defgroup USBH_MIDI_CORE_Exported_Types
  * @{
  */

    /* States for MIDI State Machine */
    typedef enum
    {
        MIDI_IDLE = 0U,
        MIDI_SEND_DATA,
        MIDI_SEND_DATA_WAIT,
        MIDI_RECEIVE_DATA,
        MIDI_RECEIVE_DATA_WAIT,
    }
    MIDI_DataStateTypeDef;

    /******************************************/

    typedef enum
    {
        MIDI_IDLE_STATE = 0U,
        MIDI_TRANSFER_DATA,
        MIDI_ERROR_STATE,
    }
    MIDI_StateTypeDef;

    /******************************************/

    typedef struct
    {
        uint8_t              OutPipe;           // Save the index of the output pipe
        uint8_t              InPipe;            // Save the index of the input pipe
        uint8_t              OutEp;             // Save the address of the output endpoint
        uint8_t              InEp;              // Save the address of the input endpoint
        uint16_t             OutEpSize;         // Save the size of the output endpoint
        uint16_t             InEpSize;          // Save the size of the input endpoint
    }
    MIDI_DataItfTypedef;

    /******************************************/

    /* Structure for MIDI process */
    typedef struct _MIDI_Process
    {
        MIDI_DataItfTypedef                DataItf;
        uint8_t                           *pTxData;
        uint8_t                           *pRxData;
        uint32_t                           TxDataLength;
        uint32_t                           RxDataLength;
        MIDI_StateTypeDef                  state;
        MIDI_DataStateTypeDef              data_tx_state;
        MIDI_DataStateTypeDef              data_rx_state;
        uint8_t                            Rx_Poll;
    }
    MIDI_HandleTypeDef;

    /******************************************/

    typedef enum {
        NoteOff         = 0x8,
        NoteOn          = 0x9,
        PolyKeyPressure = 0xa,
        ControlChange   = 0xb,
        ProgramChange   = 0xc,
        ChannelPressure = 0xd,
        PitchBendChange = 0xe
    } midi_event_t;

    /******************************************/

    typedef struct {
        uint8_t usb_byte;       // cable_number [0:3], code_index_number [4:7]
        uint8_t status_byte;    // message_type [0:3], channel_number [4:7]
        uint8_t data_byte_1;
        uint8_t data_byte_2;
    } midi_package_t;

/**
  * @}
  */

/** @defgroup USBH_MIDI_CORE_Exported_Defines (AND VARIABLES)
 * @{
 */
    #define USB_AUDIO_CLASS                                 0x01
    #define USB_MIDISTREAMING_SUBCLASS                      0x03
    #define USB_MIDI_DESC_SIZE                                 9
    #define USBH_MIDI_CLASS    &MIDI_Class
    extern USBH_ClassTypeDef  MIDI_Class;

    // Buffer size (should be at least >= MIOS32_USB_MIDI_DESC_DATA_*_SIZE/4) -> The max is 128 bit so 128/4 = 32 (64 bit chosen) pag. 17/18 USB 2.0 Standard
    #define USB_MIDI_RX_BUFFER_SIZE   64    // Max number of packages that can be received
    #define USB_MIDI_TX_BUFFER_SIZE   64    // Max number of packages that can be transmitted
    // Endpoint assignments
    #define USB_MIDI_DATA_OUT_EP 0x01   // Pag 42 USB 2.0 Standard (bEndpointAddress)
    #define USB_MIDI_DATA_IN_EP  0x81   // Pag 40 USB 2.0 Standard (bEndpointAddress)
/**
  * @}
  */

/** @defgroup USBH_MIDI_CORE_Exported_FunctionsPrototype
  * @{
  */

    USBH_StatusTypeDef  USBH_MIDI_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length);
    USBH_StatusTypeDef  USBH_MIDI_Receive (USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length);
    uint16_t            USBH_MIDI_GetLastReceivedDataSize(USBH_HandleTypeDef *phost);
    USBH_StatusTypeDef  USBH_MIDI_Stop(USBH_HandleTypeDef *phost);
    void 				USBH_MIDI_TransmitCallback(USBH_HandleTypeDef *phost);
    void 				USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost);
/**
  * @}
  */

#endif /* INC_DRIVER_USBH_MIDI_H_ */


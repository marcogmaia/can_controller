#pragma once

#include <stdint.h>

#define CAN_CONFIGS_DEFAULT                                                                                          \
    {                                                                                                                \
        .StdId = 0x7FF, .ExtId = 0x1FFFFFFF, .SRR = 1, .IDE = 0, .RTR = 1, .r0 = 1, .r1 = 1, .DLC = 0, .data = NULL, \
        .CRC = 0                                                                                                     \
    }

typedef struct CAN_message {
    uint8_t bitarray[256];
    uint8_t length;
} CAN_message_typedef;


typedef struct CAN_configs {
    uint32_t StdId; /* Specifies the standard identifier.
                          This parameter must be a number between Min_Data = 0 and
                        Max_Data = 0x7FF. */
    uint32_t ExtId; /* Specifies the extended identifier.
                         This parameter must be a number between Min_Data = 0 and
                       Max_Data = 0x3FFFF. */
    uint8_t SRR;
    uint8_t IDE;    /* Specifies the type of frame for the message that will be
                        transmitted. */
    uint8_t RTR;    /* Specifies the type of frame for the message that will be
                        transmitted.*/
    uint8_t r0, r1; /* Reserved bits */
    uint8_t DLC;    /* Specifies the length of the frame that will be
                        transmitted. This parameter must be a number between Min_Data
                        = 0 and Max_Data = 8. */
    uint8_t *data;  /* Pointer to data to be transmitted */
    uint32_t CRC;
} CAN_configs_typedef;

typedef enum enum_can_err {
    CAN_OK = 0,
    CAN_ERROR_BIT,
    CAN_ERROR_STUFFING,
    CAN_ERROR_FRAME,
    CAN_ERROR_ACK,
    CAN_ERROR_CRC,
} can_err;
#pragma once

#include "can_utils.h"
#include "can_controller.h"

typedef enum enum_decoder_fsm {
    SOF = 0,
    ID_A,
    RTR,
    IDE,
    DLC,
    ID_B,
    R0,
    R1,
    DATA,
    CRC,
    /* a partir daqui não há mais stuffing \/ */
    CRC_DL,
    ACK,
    ACK_DL,
    CAN_EOF,
    INTERFRAME_SPACING,
    ERROR_STATE
} decoder_fsm_t;
extern decoder_fsm_t decoder_state;

/**
 * @param p_config_dst pointer to struct where the decoded config will be saved
 * @param p_encoded_message pointer to encoded message, which will be decoded
 */
void decoder_decoded_message_to_configs(CAN_configs_t *p_configs_dst, uint8_t *p_decoded_message);
// void decoder_bit_destuff(uint8_t sample_bit, CAN_message_t *destuffed_bitarr);
// uint8_t *decoder_decode_msg(CAN_message_t *p_encoded_message);

CAN_err_t decoder_decode_msg(/* CAN_configs_t *p_config_dst,  */ uint8_t sampled_bit);
void decoder_get_msg(CAN_configs_t *p_configs_dst);
// void decoder_task(void *ignore);
void decoder_init();
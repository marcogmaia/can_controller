#pragma once
#include "can_controller.h"


uint8_t transmitter_transmit_bit(uint8_t transmit_bit);
void transmitter_send_error_frame() ;
void transmitter_send_interframe_space() ;
uint8_t transmitter_send_message(const CAN_message_t *p_message);
void transmitter_init();


#pragma once
#include "can_controller.h"

void transmitter_transmit(uint8_t transmit_bit);
uint8_t transmitter_transmit_message(const CAN_message_t *p_message);
void transmitter_init();
#pragma once
#include "can_utils.h"
#include "can_controller.h"


void encoder_encode_msg(CAN_configs_t *p_config, CAN_message_t *encoded_message);
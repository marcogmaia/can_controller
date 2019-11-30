#pragma once

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "decoder.h"
#include "bittiming.h"
#include "can_controller.h"


void receiver_init();
uint8_t receiver_receive_bit();
#pragma once

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "decoder.h"
#include "bittiming.h"
#include "can_controller.h"

extern SemaphoreHandle_t sem_receive_msg;
extern uint8_t receiver_error_count;
void receiver_init();
uint8_t receiver_receive_bit();
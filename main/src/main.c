#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#include "bittiming.h"
#include "can_controller.h"
#include "decoder.h"
#include "encoder.h"
#include "transmitter.h"
#include "receiver.h"

void app_main(void) {
    static const bittiming_configs_t timing_configs = {
        .time_tq   = 5000,  // 5ms
        .size_seg1 = 14,    // sync_seg == 1, 75% sample point
        .size_seg2 = 5,
        .SJW       = 3,
    };

    static const CAN_pins_t can_pins = {
        .rx_pin = GPIO_NUM_18,
        .tx_pin = GPIO_NUM_19,
    };

    bittiming_setup(&timing_configs, &can_pins);
    transmitter_init();
    receiver_init();
}

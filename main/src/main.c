#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#include "bittiming.h"
#include "can_controller.h"
#include "decoder.h"


void app_main(void) {
    static const bittiming_configs_t timing_configs = {
        .time_tq   = 50000,  // 50ms
        .size_seg1 = 14,     // sync_seg == 1, 75% sample point
        .size_seg2 = 5,
        .SJW       = 3,
    };

    static const CAN_pins_t can_pins = {
        .rx_pin = GPIO_NUM_18,
        .tx_pin = GPIO_NUM_19,
    };

    bittiming_setup(&timing_configs, &can_pins);
    xTaskCreate(decoder_task, "decoderTask", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
    // xTaskCreate(encoder_task, "encoderTask", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}

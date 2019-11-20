#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#include "bittming.h"


void app_main(void) {
    static const bittming_configs_t timing_configs = {
        .time_tq   = 50000,  // 50ms
        .size_seg1 = 14,     // sync_seg == 1, 75% sample point
        .size_seg2 = 5,
        .SJW       = 3,
    };

    bittiming_setup(&timing_configs);
}

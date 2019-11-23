
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"


#include "transmitter.h"
#include "can_utils.h"
#include "encoder.h"

void transmitter_transmit(uint8_t transmit_bit) {
    // static
    if(xSemaphoreTake(sem_write_pt, portMAX_DELAY)) {
        gpio_set_level(p_can_pins->tx_pin, transmit_bit);
        printf("%d", transmit_bit);
        fflush(stdout);
    }
}

uint8_t transmitter_transmit_message(const CAN_message_t *p_message) {
    char bitarr_buf[256];
    bitarray_to_str(bitarr_buf, p_message->bitarray, p_message->length);
    ESP_LOGI("TRASMITTER", "%s", bitarr_buf);

    for(uint32_t i = 0; i < p_message->length; ++i) {
        transmitter_transmit(p_message->bitarray[i]);
    }
    /* Interframe spacing */
    transmitter_transmit(1);
    transmitter_transmit(1);
    transmitter_transmit(1);
    
    printf("\n");
    return 1;
}

static void transmitter_task(void *ignore) {
    CAN_configs_t can_configs;
    can_configs.StdId     = 0x123;
    can_configs.IDE       = 0;
    static uint8_t data[] = {0};
    can_configs.data      = data;
    can_configs.DLC       = sizeof data;
    can_configs.RTR       = 0;

    static CAN_message_t can_message;
    while(true) {
        encoder_encode_msg(&can_configs, &can_message);
        transmitter_transmit_message(&can_message);
        data[0] += 1;
    }
}

void transmitter_init() {
    xTaskCreate(transmitter_task, "trasmTask", configMINIMAL_STACK_SIZE * 3, NULL, 7, NULL);
}
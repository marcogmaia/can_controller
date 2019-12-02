#include "esp_log.h"
#include "receiver.h"
#include "transmitter.h"


uint8_t receiver_error_count = 0;
uint8_t receiver_receive_bit() {
    static uint8_t sampled_bit;
    /* ErrorFrame detection */

    if(xSemaphoreTake(sem_sample_pt, portMAX_DELAY)) {
        sampled_bit = gpio_get_level(p_can_pins->rx_pin);

        receiver_error_count = sampled_bit ? 0 : receiver_error_count + 1;
        if(receiver_error_count >= 6) {
            decoder_state = ERROR_STATE;
            ESP_LOGW("RECEIVER", "ERROR_COUNT");
        }

        decoder_decode_msg(sampled_bit);
        // printf();
    }
    return sampled_bit;
}

SemaphoreHandle_t sem_receive_msg = NULL;
void receiver_receive_message(CAN_configs_t *p_configs) {
    CAN_configs_t received_configs;

    if(xSemaphoreTake(sem_receive_msg, portMAX_DELAY)) {
        decoder_get_msg(&received_configs);
    }

    memcpy(p_configs, &received_configs, sizeof received_configs);
}

static void receiver_task(void *ignore) {
    while(true) {
        receiver_receive_bit();

        static CAN_configs_t msg_configs;

        if(xSemaphoreTake(sem_receive_msg, pdMS_TO_TICKS(20))) {
            decoder_get_msg(&msg_configs);

            if(transmitter_sending == false) {
                ESP_LOGI("RECEIVER",
                         "DECODE:\n"
                         "ID_A: 0x%X\n"
                         "DLC: %d\n"
                         "RTR: %d\n"
                         "IDE: %d\n"
                         "ID_B: 0x%X\n"
                         "DATA: %d\n"
                         "CRC: %d",
                         msg_configs.StdId, msg_configs.DLC, msg_configs.RTR, msg_configs.IDE, msg_configs.ExtId,
                         msg_configs.data[0], msg_configs.CRC);
            }
        }
    }
}

void receiver_init() {
    sem_receive_msg = xSemaphoreCreateBinary();
    xTaskCreate(receiver_task, "rcvTask", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}

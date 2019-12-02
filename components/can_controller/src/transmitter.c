
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp32/rom/ets_sys.h"

#include "transmitter.h"
#include "receiver.h"
#include "can_utils.h"
#include "encoder.h"

static const char *TAG = "TRANSMITTER";

uint8_t transmitter_transmit_bit(uint8_t transmit_bit) {
    uint8_t tx_bit = CAN_RECESSIVE;
    if(xSemaphoreTake(sem_write_pt, portMAX_DELAY)) {
        tx_bit = transmit_bit;
        gpio_set_level(p_can_pins->tx_pin, transmit_bit);
    }
    return tx_bit;
}
void transmitter_send_interframe_space() {
    ESP_LOGD(TAG, "INTERFRAME_SPACING");
    transmitter_transmit_bit(CAN_RECESSIVE);
    transmitter_transmit_bit(CAN_RECESSIVE);
    transmitter_transmit_bit(CAN_RECESSIVE);
    transmitter_transmit_bit(CAN_RECESSIVE);
    transmitter_transmit_bit(CAN_RECESSIVE);
}

// void transmitter_send_error_frame() {
//     CAN_message_t error_frame = {
//         .bitarray = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
//         .length   = 14,
//     };

//     for(uint32_t i = 0; i < error_frame.length; ++i) {
//         transmitter_transmit_bit(error_frame.bitarray[i]);
//     }
// }

bool can_arb_lost        = false;
bool transmitter_sending = false;
uint8_t transmitter_send_message(const CAN_message_t *p_message) {
    transmitter_sending = true;
    char bitarr_buf[256];
    bitarray_to_str(bitarr_buf, p_message->bitarray, p_message->length);
    ESP_LOGI(TAG, "%s", bitarr_buf);

    for(uint32_t i = 0; i < p_message->length; ++i) {
        if(decoder_state == ERROR_STATE) {
            return 0;
        }

        uint8_t tx_bit, rx_bit;
        tx_bit = transmitter_transmit_bit(p_message->bitarray[i]);
        ets_delay_us(50);
        rx_bit = gpio_get_level(p_can_pins->rx_pin);

        ESP_LOGD(TAG, "tx: %d | rx: %d", tx_bit, rx_bit);
        /* arbitração */
        if(tx_bit == CAN_RECESSIVE && rx_bit == CAN_DOMINANT && (decoder_state == ID_A || decoder_state == ID_B)) {
            can_arb_lost = true;
            ESP_LOGW("TRANSMITTER", "\nARBITRATION_LOST");
            return 0;
        }
    }

    /* posso sempre enviar recessivos */
    transmitter_send_interframe_space();


    transmitter_sending = false;
    ESP_LOGI(TAG, "MSG SENT");
    return 0;
}

bool transmitter_can_send = true;
static void transmitter_task(void *ignore) {
    if(decoder_state == SOF) {
        CAN_configs_t can_configs;
        can_configs.StdId   = esp_random() % 0x7FF;
        can_configs.ExtId   = esp_random() % 0x3FFFF;
        can_configs.DLC     = 1;
        can_configs.data[0] = 0;

        CAN_message_t can_message;

        memset(&can_message, 0, sizeof can_message);
        encoder_encode_msg(&can_configs, &can_message);
        transmitter_send_message(&can_message);
    }
    vTaskDelete(NULL);
}

void transmitter_init() {
    xTaskCreate(transmitter_task, "trnsmtTask", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}

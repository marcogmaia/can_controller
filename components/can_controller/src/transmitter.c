
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "transmitter.h"
#include "receiver.h"
#include "can_utils.h"
#include "encoder.h"


uint8_t transmitter_transmit_bit(uint8_t transmit_bit) {
    uint8_t tx_bit = CAN_RECESSIVE;
    if(xSemaphoreTake(sem_write_pt, portMAX_DELAY)) {
        tx_bit = transmit_bit;
        gpio_set_level(p_can_pins->tx_pin, transmit_bit);
    }
    return tx_bit;
}
void transmitter_send_interframe_space() {
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

bool can_arb_lost = false;
uint8_t transmitter_send_message(const CAN_message_t *p_message) {
    if(!can_arb_lost) {
        char bitarr_buf[256];
        bitarray_to_str(bitarr_buf, p_message->bitarray, p_message->length);
        ESP_LOGI("TRASMITTER", "%s", bitarr_buf);

        for(uint32_t i = 0; i < p_message->length; ++i) {
            uint8_t tx_bit, rx_bit;
            tx_bit = transmitter_transmit_bit(p_message->bitarray[i]);
            rx_bit = receiver_receive_bit();

            if(tx_bit == CAN_RECESSIVE && rx_bit == CAN_DOMINANT) {
                /* perdi a arbitração */
                can_arb_lost = true;
                /* setar uma flag pra fazer o "resend" */
                return 1;
            }
            else if(tx_bit == CAN_DOMINANT && rx_bit == CAN_RECESSIVE) {
                decoder_state = ERROR_STATE;
                break;
            }
        }
        /* Interframe spacing */
        transmitter_send_interframe_space();
    }

    // printf("\n");
    return 0;
}

static void transmitter_task(void *ignore) {
    // CAN_configs_t can_configs;
    // can_configs.StdId     = 0x123;
    // can_configs.IDE       = 1;
    // can_configs.ExtId     = 0x3F123;
    // static uint8_t data[] = {0};
    // can_configs.data      = data;
    // can_configs.DLC       = sizeof data;
    // can_configs.RTR       = 0;

    // static CAN_message_t can_message;
    // while(true) {
    //     memset(&can_message, 0, sizeof can_message);
    //     encoder_encode_msg(&can_configs, &can_message);
    //     transmitter_send_message(&can_message);
    //     data[0] += 1;
    // }
}

void transmitter_init() {
    xTaskCreate(transmitter_task, "trnsmtTask", configMINIMAL_STACK_SIZE * 3, NULL, 7, NULL);
}

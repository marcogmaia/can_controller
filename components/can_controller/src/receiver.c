#include "receiver.h"
#include "transmitter.h"


static void receiver_task(void *ignore) {
    // static CAN_configs_t decoded_configs;
    // memset(&decoded_configs, 0, sizeof decoded_configs);

    // while(true) {
    //     // if(xSemaphoreTake(sem_sample_pt, portMAX_DELAY)) {
    //     static uint8_t sampled_bit;
    //     sampled_bit   = gpio_get_level(p_can_pins->rx_pin);
    //     CAN_err_t ret = decoder_decode_msg(sampled_bit);

    //     /* tratar os erros no switch */
    //     switch(ret) {
    //         case CAN_DECODED: {
    //             printf(
    //                 "\n"
    //                 "DECODE:\n"
    //                 "ID_A: 0x%X\n"
    //                 "DLC: %d\n"
    //                 "RTR: %d\n"
    //                 "IDE: %d\n"
    //                 "ID_B: 0x%X\n"
    //                 "DATA: %d\n"
    //                 "CRC: %d\n",
    //                 decoded_configs.StdId, decoded_configs.DLC, decoded_configs.RTR, decoded_configs.IDE,
    //                 decoded_configs.ExtId, decoded_configs.data[0], decoded_configs.CRC);

    //             /* reset configs  */
    //             memset(&decoded_configs, 0, sizeof decoded_configs);
    //         } break;

    //         case CAN_ACK: {
    //             /* envia pro transmitter o ACK pra botar no barramento */
    //             transmitter_transmit_bit(CAN_DOMINANT);
    //         } break;

    //         case CAN_IDLE: {
    //         } break;

    //         case CAN_OK: {
    //             // printf("bit decoded\n");
    //         } break;

    //         default:
    //             break;
    //     }
    //     // }
    // }
}

uint8_t receiver_receive_bit() {
    static uint8_t sampled_bit;
    if(xSemaphoreTake(sem_sample_pt, portMAX_DELAY)) {
        sampled_bit = gpio_get_level(p_can_pins->rx_pin);
        decoder_decode_msg(sampled_bit);
    }
    return sampled_bit;
}

CAN_configs_t receiver_receive_message() {
    static CAN_configs_t received_configs;

    return received_configs;
}

void receiver_init() {
    xTaskCreate(receiver_task, "rcvTask", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}

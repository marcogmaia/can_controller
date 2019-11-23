#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "decoder.h"
#include "bittiming.h"
#include "can_controller.h"

#include "esp_log.h"
const char *TAG = "DECODER";

/**
 * https://www.kvaser.com/about-can/the-can-protocol/can-error-handling/
 *
 * os tipos de erros que tem são 5:
 * 1- Bit Monitoring. (manda um bit, recebe outro)
 * 2- Bit Stuffing. (não foi feito o bit sutffing corretamente, preciso ter só um bit pra dar "assert"
 *      no sexto bit pra ver se é oposto aos 5 anteriores)
 * 3- Frame Check. (CRC_DEL, ACK_DEL, EOF, esses campos são fixos)
 * 4- Acknowledgement Check.
 * 5- Cyclic Redundancy Check.
 */
CAN_err_t decoder_decode_msg(CAN_configs_t *p_config_dst, uint8_t sampled_bit) {
    printf("%d", sampled_bit);
    fflush(stdout);
    enum decoder_fsm {
        SOF = 0,
        ID_A,
        RTR,
        IDE,
        DLC,
        ID_B,
        R0,
        R1,
        DATA,
        CRC,
        /* a partir daqui não há mais stuffing */
        CRC_DL,
        ACK,
        ACK_DL,
        CAN_EOF,
        INTERFRAME_SPACING,
    };

    static enum decoder_fsm state = SOF;

    static uint8_t buffer[256];
    static uint8_t size;

    /* "estado" que trata do destuff */
    {
        static uint8_t curr_bit_count = 0;
        static uint8_t last_bit       = 0xFF;
        static bool is_stuffed_bit    = false;
        /* só há bitstuffing até o CRC */
        if(state < CRC_DL) {
            /* tem que matar o sexto bit depois de 5 iguais */
            is_stuffed_bit = (curr_bit_count == 5) ? true : false;
            curr_bit_count = (last_bit == sampled_bit) ? (curr_bit_count + 1) : 1;
            last_bit       = sampled_bit;

            if(is_stuffed_bit) {
                /* se é o bitstuffed e não mudou o bit, então ocorreu o erro */
                curr_bit_count = 1;
                is_stuffed_bit = false;
                return CAN_ERROR_STUFFING;
            }
        }
        else {
            /* reseta o estado caso não haja mais bitstuffing */
            curr_bit_count = 0;
            last_bit       = 0xFF;
            is_stuffed_bit = false;
        }
    }


    /* start State Machine */
    static uint8_t state_cnt = 0;
    CAN_err_t ret            = CAN_OK;
    switch(state) {
        case SOF: {
            /* se SoF faz o setup */
            if(sampled_bit == 0) {
                // hardsync_flag = 1;
                memset(buffer, 0xFF, sizeof buffer);
                memset(p_config_dst, 0, sizeof *p_config_dst);
                static uint8_t data[8];
                memset(data, 0, sizeof data);
                p_config_dst->data = data;

                size           = 0;
                buffer[size++] = sampled_bit;
                state          = ID_A;
            }

        } break;

        case ID_A: {
            buffer[size++] = sampled_bit;
            ++state_cnt;
            if(state_cnt == 11) {
                state_cnt           = 0;
                p_config_dst->StdId = bitarray_to_int(&buffer[size - 11], 11);
                state               = RTR;
            }
        } break;

        case RTR: {
            buffer[size++]    = sampled_bit;
            p_config_dst->RTR = sampled_bit;

            if(p_config_dst->SRR == 1) {
                state = R1;
            }
            else {
                state = IDE;
            }
        } break;

        case IDE: {
            buffer[size++]    = sampled_bit;
            p_config_dst->IDE = sampled_bit;

            if(p_config_dst->IDE == 0) {
                state = R0;
            }
            /* IDE == 1 */
            else {
                p_config_dst->SRR = p_config_dst->RTR;
                state             = ID_B;
            }
        } break;

        case R0: {
            buffer[size++] = sampled_bit;
            state          = DLC;
        } break;

        case R1: {
            buffer[size++] = sampled_bit;
            state          = R0;
        } break;

        case ID_B: {
            buffer[size++] = sampled_bit;
            ++state_cnt;
            if(state_cnt == 18) {
                state_cnt           = 0;
                p_config_dst->ExtId = bitarray_to_int(&buffer[size - 18], 18);
                state               = RTR;
            }
        } break;

        case DLC: {
            buffer[size++] = sampled_bit;
            ++state_cnt;
            if(state_cnt == 4) {
                state_cnt         = 0;
                p_config_dst->DLC = (uint8_t)bitarray_to_int(&buffer[size - 4], 4);
                if(p_config_dst->RTR == 1) {
                    state = CRC;
                }
                else {
                    state = DATA;
                }
            }
        } break;

        case DATA: {
            // static uint8_t state_cnt = 0;
            uint8_t data_size = (p_config_dst->DLC > 8) ? 8 : p_config_dst->DLC;

            buffer[size++] = sampled_bit;
            ++state_cnt;

            if(state_cnt == (data_size * 8)) {
                state_cnt = 0;
                for(uint32_t i = 0; i < data_size; ++i) {
                    p_config_dst->data[i] = bitarray_to_int(&buffer[size - (data_size - i) * 8], 8);
                }
                state = CRC;
            }
        } break;

        case CRC: {
            buffer[size++] = sampled_bit;
            ++state_cnt;
            if(state_cnt == 15) {
                state_cnt         = 0;
                p_config_dst->CRC = bitarray_to_int(buffer + size - 15, 15);
                state             = CRC_DL;
            }
        } break;

        case CRC_DL: {
            buffer[size++] = sampled_bit;
            state          = ACK;
        } break;

        case ACK: {
            buffer[size++] = sampled_bit;
            state          = ACK_DL;
            ret            = CAN_ACK;
        } break;

        case ACK_DL: {
            if(p_config_dst->CRC != crc15(buffer, size - 15 - 2)) {
                ESP_LOGW(TAG, "ERROR CRC");
                ret = CAN_ERROR_CRC;
            }
            buffer[size++] = sampled_bit;
            state          = CAN_EOF;
        } break;

        /**
         * TODO: arrumar o EOF e o INTERFRAME
         * é pra functionar com state_cnt = 7
         */
        case CAN_EOF: {
            if(sampled_bit == 0) {
                ESP_LOGW(TAG, "ERROR Frame EOF\n");
                state_cnt = 0;
                ret       = CAN_ERROR_FRAME;
            }
            else {
                buffer[size++] = sampled_bit;
                ++state_cnt;
            }

            if(state_cnt == 7) {
                // state_cnt = 0;
                state_cnt = 0;
                state     = INTERFRAME_SPACING;  // state = INTERFRAME_SPACING
                ret       = CAN_DECODED;
            }
        } break;

        case INTERFRAME_SPACING: {
            if(sampled_bit == 0) {
                ESP_LOGW(TAG, "ERROR Frame INTERFRAME_SPACING\n");
                ret = CAN_ERROR_FRAME;
            }
            else {
                buffer[size++] = sampled_bit;
                ++state_cnt;
            }
            if(state_cnt == 3) {
                ret           = CAN_IDLE;
                hardsync_flag = 1;
                state_cnt     = 0;
                state         = SOF; /* vai ficar aqui até acontecer o SoF */
            }
        } break;
    }

    return ret;
}

static void decoder_task(void *ignore) {
    uint8_t sample_bit;
    static CAN_configs_t decoded_configs;
    memset(&decoded_configs, 0, sizeof decoded_configs);

    while(true) {
        if(xSemaphoreTake(sem_sample_pt, portMAX_DELAY)) {
            sample_bit    = gpio_get_level(p_can_pins->rx_pin);
            CAN_err_t ret = decoder_decode_msg(&decoded_configs, sample_bit);

            /* tratar os erros no switch */
            switch(ret) {
                case CAN_DECODED: {
                    printf(
                        "\n"
                        "DECODE:\n"
                        "ID_A: 0x%X\n"
                        "DLC: %d\n"
                        "RTR: %d\n"
                        "IDE: %d\n"
                        "ID_B: 0x%X\n"
                        "DATA: %d\n"
                        "CRC: %d\n",
                        decoded_configs.StdId, decoded_configs.DLC, decoded_configs.RTR, decoded_configs.IDE,
                        decoded_configs.ExtId, decoded_configs.data[0], decoded_configs.CRC);

                    /* reset configs  */
                    memset(&decoded_configs, 0, sizeof decoded_configs);
                } break;

                case CAN_ACK: {
                    /* envia pro transmitter o ACK pra botar no barramento */
                } break;

                case CAN_IDLE: {
                    hardsync_flag = 1;
                } break;

                case CAN_OK: {
                    // printf("bit decoded\n");
                } break;

                default:
                    break;
            }
        }
    }
}

void decoder_init() {
    xTaskCreate(decoder_task, "decoderTask", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}

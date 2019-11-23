#include <stdio.h>


#include "esp_timer.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "bittiming.h"


typedef enum {
    SYNC = 0,
    PSEG1,
    PSEG2,
    EXTEND_PSEG1,
    // NUM_OF_STATES,
} state_fsm;

/* ===== Definition of private variables ==== */

/* contador do tseg1 (incremental) */
static uint8_t ts1_cnt = 0;
/* contator do tseg2 (decremental) */
static uint8_t ts2_cnt = 0;
/* current state */
// uint8_t state;


uint8_t resync_flag    = 0;
static state_fsm state = SYNC;
bool is_write_pt, is_sample_pt;

/* ==== END VARIABLE DEFINITIONS ========= */
static const bittiming_configs_t *configs;

static void plot_data() {
    printf("%d %d %d\n", state, is_write_pt - 1, is_sample_pt - 1);
}

// static void bittiming_write_task(void *ignore) {
//     while(true) {
//         if(xSemaphoreTake(sem_write_pt, portMAX_DELAY) == pdTRUE) {
//             printf("write task\n");
//         }
//     }
// }
// static void bittimint_sample_task(void *ignore) {
//     while(true) {
//         if(xSemaphoreTake(sem_sample_pt, portMAX_DELAY) == pdTRUE) {
//             printf("sample task\n");
//         }
//     }
// }

static esp_timer_handle_t handle_bittiming_fsm;

static void hard_sync(void *ignore) {
    static int64_t lasttime = 0;
    // timenow = esp_timer_get_time();
    int64_t timenow = esp_timer_get_time();
    if((timenow - lasttime) > 50000) {
        lasttime = timenow;
        state    = SYNC;
        ESP_ERROR_CHECK(esp_timer_stop(handle_bittiming_fsm));
        ESP_ERROR_CHECK(esp_timer_start_periodic(handle_bittiming_fsm, configs->time_tq));
        ts1_cnt = ts2_cnt = 0;
    }
}

static void IRAM_ATTR intr_set_resync_flag(void *ignore) {
    static int64_t lasttime = 0;
    int64_t timenow         = esp_timer_get_time();
    if((timenow - lasttime) > 200000) { // 200ms
        lasttime    = timenow;
        resync_flag = 1;
    }
}

void bittiming_setup(const bittiming_configs_t *timing_configs, const CAN_pins_t *p_can_pins_conf) {
    configs = timing_configs;
    p_can_pins = p_can_pins_conf;
    

    /* create writePt and samplePt semaphores */
    sem_write_pt  = xSemaphoreCreateBinary();
    sem_sample_pt = xSemaphoreCreateBinary();

    /* setup state_machine */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &update_state_machine,
        .name     = "bittimingUpdateFSM",
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &handle_bittiming_fsm));
    /* The timer has been created but is not running yet */
    /* Start the timers */
    ESP_ERROR_CHECK(esp_timer_start_periodic(handle_bittiming_fsm, timing_configs->time_tq));
    
    /* setup cantx and canrx interrupts */
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << p_can_pins->rx_pin) ;  // GPIO18 e 19
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
    io_conf.intr_type    = GPIO_PIN_INTR_POSEDGE;

    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    // gpio_isr_handler_add(GPIO_NUM_18, intr_hard_sync, NULL);
    gpio_isr_handler_add(p_can_pins->rx_pin, intr_set_resync_flag, NULL);

    gpio_set_direction(p_can_pins->tx_pin, GPIO_MODE_OUTPUT);
}

static bool consume_resync_flag() {
    uint8_t ret = resync_flag;
    if(resync_flag) resync_flag = 0;
    return ret;
}

/* A cada tq a maquina de estados é chamada */
void update_state_machine(void *ignore) {
    static uint8_t extend_count = 0;
    static uint8_t printtq      = 0;


    printtq = !printtq;
    // printf("%d ", printtq - 2);
    // plot_data();

    is_write_pt = is_sample_pt = false;

    switch(state) {
        case SYNC: {
            if(hardsync_flag) {
                hard_sync(NULL);
                hardsync_flag = 0;
            }
            state = PSEG1;
            consume_resync_flag();
        } break;

        case PSEG1: {
            ++ts1_cnt;
            if(ts1_cnt == configs->size_seg1) {
                ts1_cnt      = 0;
                state        = PSEG2;
                is_sample_pt = true;
                xSemaphoreGive(sem_sample_pt);
            }
            else if(consume_resync_flag()) {
                uint8_t min;
                if((configs->size_seg1 - ts1_cnt) < configs->SJW)
                    min = configs->SJW;
                else
                    min = configs->size_seg1 - ts1_cnt;

                state        = EXTEND_PSEG1;
                extend_count = min;
                ts1_cnt      = 0;
            }
        } break;

        case EXTEND_PSEG1: {
            --extend_count;
            /* se acontecer o resync aqui não faz nada */
            consume_resync_flag();
            if(extend_count == 0) {
                is_sample_pt = true;
                xSemaphoreGive(sem_sample_pt);
                state = PSEG2;
            }
        } break;

        case PSEG2: {
            ++ts2_cnt;
            if(ts2_cnt >= configs->size_seg2) {
                ts2_cnt     = 0;
                is_write_pt = true;
                xSemaphoreGive(sem_write_pt);
                state = SYNC;
            }
            else if(consume_resync_flag()) {
                /* encurta no máximo de SJW */
                if((configs->size_seg2 - ts2_cnt) < configs->SJW) {
                    ts2_cnt += configs->SJW;
                }
                else {
                    ts2_cnt     = 0;
                    state       = SYNC;
                    is_write_pt = true;
                    xSemaphoreGive(sem_write_pt);
                }
            }
        } break;
        default:
            break;
    }
}

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "can_controller.h"

typedef struct {
    // const uint8_t BRP;        //= 15;
    const uint8_t SJW;        //= 7;
    const uint32_t time_tq;   //= 50; duração do tq em milisegundos
    const uint8_t size_seg1;  //= 15;  // propseg + phaseseg1
    const uint8_t size_seg2;  //= 5;
} bittiming_configs_t;
/* UNO tem interrupts apenas nos pins 2 e 3 */
// const uint8_t rx_pin = 2;
// const uint8_t hard_sync_pin = 3;
// }  // namespace configs


// enum bittiming_states {
//     SYNC = 0,
//     PSEG1,
//     PSEG2,
//     SKIP_CLOCKS,

//     NUM_OF_STATES,
// };


/* contador do tseg1 (incremental) */
// extern uint8_t ts1_cnt;
/* contator do tseg2 (decremental) */
// extern uint8_t ts2_cnt;
/* current state */
// extern uint8_t state;
// extern bool is_write_pt;
// extern bool is_sample_pt;

extern uint8_t resync_flag;
extern uint8_t hardsync_flag;

/**
 * run in the setup to configure the bittiming
 */
void bittiming_setup(const bittiming_configs_t *p_timing_configs, const CAN_pins_t *p_can_pins_conf);

/**
 * update the state machine based on the current state
 * of the bittiming
 */
void update_state_machine();

/**
 * interrupt to set the resync flag
 */
// void set_resync_flag();

/**
 * @return 1 if resync was set, 0 otherwise
 */
// uint8_t consume_resync_flag();

/* set state */
void hard_sync();

/* @brief plot(print): state, write_pt and sample_pt */
void plot_data();

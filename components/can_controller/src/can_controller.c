#include <stdint.h>
#include "can_controller.h"


// SemaphoreHandle_t sem_write_pt;
// SemaphoreHandle_t sem_sample_pt;
// const CAN_pins_t *can_pins;

SemaphoreHandle_t sem_write_pt;
SemaphoreHandle_t sem_sample_pt;
const CAN_pins_t *p_can_pins;

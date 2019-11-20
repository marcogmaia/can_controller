
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "transmitter.h"

#include "driver/gpio.h"

void transmitter_transmit(uint8_t transmit_bit) {
    // static
    if(xSemaphoreTake(sem_write_pt, portMAX_DELAY)) {
        gpio_set_level(p_can_pins->tx_pin, !!transmit_bit);
    }
}
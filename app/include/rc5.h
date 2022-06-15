#ifndef RC5_H
#define RC5_H

#include <zephyr.h>
#include <zephyr/drivers/gpio.h>

extern struct gpio_dt_spec rc5_decoder;

// stolen from: https://github.com/pinkeen/avr-rc5/blob/master/main.c

#define rc5_get_start_bits(command) ((command & 0x3000) >> 12)
#define rc5_get_toggle_bit(command) ((command & 0x800) >> 11)
#define rc5_get_address_bits(command) ((command & 0x7C0) >> 6)
#define rc5_get_command_bits(command) (command & 0x3F)
#define rc5_get_command_address_bits(command) (command & 0x7FF)

void rc5_init();
void rc5_reset();
bool rc5_new_command_received(int* new_command);

#endif
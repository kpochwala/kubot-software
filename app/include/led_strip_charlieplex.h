#pragma once
#include <zephyr/drivers/led_strip.h>

extern const struct led_rgb kabot_active;
extern const struct led_rgb kabot_inactive;
extern const struct led_rgb kabot_color;
extern const struct led_rgb kabot_init;
extern const struct led_rgb kabot_warning;
extern const struct led_rgb kabot_error;

int led_strip_update_rgb_impl(const struct device *dev, struct led_rgb *pixels, size_t num_pixels);
int led_strip_set_led(const struct device *dev, struct led_rgb pixel, size_t number_of_pixel);
extern struct k_mutex rgb_mutex;

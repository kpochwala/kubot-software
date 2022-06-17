#pragma once
#include <zephyr/drivers/led_strip.h>

extern const struct led_rgb kabot_active;
extern const struct led_rgb kabot_inactive;
extern const struct led_rgb kabot_color;
extern const struct led_rgb kabot_init;
extern const struct led_rgb kabot_warning;
extern const struct led_rgb kabot_error;
extern const struct led_rgb kabot_ok;
extern const struct led_rgb kabot_off;

// commented out because not thread safe
// int led_strip_update_rgb_impl(const struct device *dev, struct led_rgb *pixels, size_t num_pixels);
// int led_strip_set_led(const struct device *dev, struct led_rgb pixel, size_t number_of_pixel);
void set_led(int number, struct led_rgb color);
void set_led_multi(struct led_rgb* pixels, size_t number_of_pixels, size_t offset);
void set_led_all(struct led_rgb color);
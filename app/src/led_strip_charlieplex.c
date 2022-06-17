#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "led_strip_charlieplex.h"

LOG_MODULE_REGISTER(led_strip_charlieplex);
K_MUTEX_DEFINE(rgb_mutex);

////////////////////////////////////////////////// LEDS
#define ALL_LEDS_NUMBER 14
#define PWM_RESOLUTION 0.1

#define PWM_CYCLE_PERIOD_US 10
#define FLASH_TIME_US 250

#define FADE_TIME_US 300000
#define FADE_LOOP_ITERATIONS 5

static const struct pwm_dt_spec pwm_red = PWM_DT_SPEC_GET(DT_ALIAS(pwm_red));
static const struct pwm_dt_spec pwm_green = PWM_DT_SPEC_GET(DT_ALIAS(pwm_green));
static const struct pwm_dt_spec pwm_blue = PWM_DT_SPEC_GET(DT_ALIAS(pwm_blue));

static const struct pwm_dt_spec rgb_components[] = {pwm_red, pwm_green, pwm_blue};

static const struct gpio_dt_spec rgb_en_0 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_0), gpios);
static const struct gpio_dt_spec rgb_en_1 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_1), gpios);
static const struct gpio_dt_spec rgb_en_2 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_2), gpios);
static const struct gpio_dt_spec rgb_en_3 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_3), gpios);
static const struct gpio_dt_spec rgb_en_4 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_4), gpios);
static const struct gpio_dt_spec rgb_en_5 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_5), gpios);
static const struct gpio_dt_spec rgb_en_6 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_6), gpios);
static const struct gpio_dt_spec rgb_en_7 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_7), gpios);
static const struct gpio_dt_spec rgb_en_8 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_8), gpios);
static const struct gpio_dt_spec rgb_en_9 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_9), gpios);

static const struct gpio_dt_spec rgb_en_l0 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l0), gpios);
static const struct gpio_dt_spec rgb_en_l1 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l1), gpios);
static const struct gpio_dt_spec rgb_en_l2 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l2), gpios);
static const struct gpio_dt_spec rgb_en_l3 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l3), gpios);

static const struct gpio_dt_spec rgb_enables[] = {rgb_en_0, rgb_en_1, rgb_en_2, rgb_en_3, rgb_en_4, rgb_en_5, rgb_en_6, rgb_en_7, rgb_en_8, rgb_en_9, rgb_en_l3, rgb_en_l2, rgb_en_l0, rgb_en_l1};

const struct led_rgb kabot_color = {255, 0, 72};
const struct led_rgb kabot_init = {0, 0, 255};
const struct led_rgb kabot_warning = {255, 157, 0};
const struct led_rgb kabot_error = {255, 0, 0};

const struct led_rgb kabot_active = {255, 255, 255};
const struct led_rgb kabot_inactive = {127, 0, 36};


struct rgb {
    float r;
    float g;
    float b;
};

const struct rgb kabot_init_internal = {0.0, 0.0, 1.0};

struct rgb rgbPwmValues[] = {kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal,
                             kabot_init_internal};


void set_leds(struct rgb* rgbPwmValues, int led_number) {

        for(int currentLedNumber = 0; currentLedNumber < led_number; currentLedNumber++){
            struct gpio_dt_spec current_led = rgb_enables[currentLedNumber];
            gpio_pin_set_dt(&current_led, 1);

            if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0){
                pwm_set_pulse_dt(&pwm_red, pwm_red.period * rgbPwmValues[currentLedNumber].r);
                pwm_set_pulse_dt(&pwm_green, pwm_green.period * rgbPwmValues[currentLedNumber].g);
                pwm_set_pulse_dt(&pwm_blue, pwm_blue.period * rgbPwmValues[currentLedNumber].b);

                k_mutex_unlock(&rgb_mutex);
                k_sleep(K_USEC(FLASH_TIME_US));
            }
            
            pwm_set_pulse_dt(&pwm_red, 0);
            pwm_set_pulse_dt(&pwm_green, 0);
            pwm_set_pulse_dt(&pwm_blue, 0);
            gpio_pin_set_dt(&current_led, 0);
        }
}

void led_thread(void){

    for(int i = 0; i < ALL_LEDS_NUMBER; i++){
        gpio_pin_configure_dt(&rgb_enables[i], GPIO_OUTPUT_ACTIVE);
        gpio_pin_set_dt(&rgb_enables[i], 0);
    }

    while(1) {
        set_leds(rgbPwmValues, ALL_LEDS_NUMBER);    
        k_sleep(K_MSEC(1));
    }
}

// todo: make it real driver
int led_strip_update_rgb_impl(const struct device *dev, struct led_rgb *pixels, size_t num_pixels){
    ARG_UNUSED(dev);
    int ret = 0;
    for(size_t i = 0; i < num_pixels; i++){
        
        ret = k_mutex_lock(&rgb_mutex, K_MSEC(100));
        if(ret == 0){
            rgbPwmValues[i].r = pixels[i].r * (1.0/255);
            rgbPwmValues[i].g = pixels[i].g * (1.0/255);
            rgbPwmValues[i].b = pixels[i].b * (1.0/255);
            k_mutex_unlock(&rgb_mutex);
        }else{
            LOG_WRN("Unable aquire mutex: rgb_mutex");
        }
    }

    return ret;
}

int led_strip_set_led(const struct device *dev, struct led_rgb pixel, size_t number_of_pixel){
    static struct led_rgb pixels[ALL_LEDS_NUMBER];
    pixels[number_of_pixel].r = pixel.r;
    pixels[number_of_pixel].g = pixel.g;
    pixels[number_of_pixel].b = pixel.b;

    return led_strip_update_rgb_impl(dev, pixels, ALL_LEDS_NUMBER);
}

void set_led(int number, struct led_rgb color){
    if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0){
        led_strip_set_led(NULL, color, number);
        k_mutex_unlock(&rgb_mutex);
    }else{
        LOG_WRN("Led mutex timeout!");
    }
}

void set_led_multi(struct led_rgb* pixels, size_t number_of_pixels, size_t offset){
    if(number_of_pixels + offset > sizeof(ALL_LEDS_NUMBER)){
        LOG_ERR("Too big size of led array or offset: array: %d offset: %d max: %d", number_of_pixels, offset, ALL_LEDS_NUMBER);
        return;
    }
    if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0){
        for(size_t i = 0; i < number_of_pixels; i++){
            led_strip_set_led(NULL, pixels[i], i+offset);
            k_mutex_unlock(&rgb_mutex);
        }
    }else{
        LOG_WRN("Led mutex timeout!");
    }
}

K_THREAD_DEFINE(led_thread_id, 1024, led_thread, NULL, NULL, NULL, 6, 0, 0);

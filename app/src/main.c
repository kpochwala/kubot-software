#include <zephyr.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <stdio.h>



////////////////////////////////////////////////// LEDS
#define ALL_LEDS_NUMBER 10
#define PWM_RESOLUTION 0.1

#define PWM_CYCLE_PERIOD_US 10
#define FLASH_TIME_US 100

#define FADE_TIME_US 300000
#define FADE_LOOP_ITERATIONS 5

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
         "Console device is not ACM CDC UART device");

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

static const struct gpio_dt_spec rgb_k_r = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_k_r), gpios);
static const struct gpio_dt_spec rgb_k_g = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_k_g), gpios);
static const struct gpio_dt_spec rgb_k_b = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_k_b), gpios);

static const struct gpio_dt_spec rgb_components[3] = {rgb_k_r, rgb_k_g, rgb_k_b};

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

static const struct gpio_dt_spec rgb_enables[10] = {rgb_en_0, rgb_en_1, rgb_en_2, rgb_en_3, rgb_en_4, rgb_en_5, rgb_en_6, rgb_en_7, rgb_en_8, rgb_en_9};

struct rgb {
    float r;
    float g;
    float b;
};

const struct rgb kabot_color = {255/255.0, 0/255.0, 72/255.0};

struct rgb rgbPwmValues[10] = {kabot_color,
                               kabot_color,
                               kabot_color,
                               kabot_color,
                               kabot_color,
                               kabot_color,
                               kabot_color,
                               kabot_color,
                               kabot_color,
                               kabot_color};




struct rgb rgbPwmValues2[10] = {{0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               {0.0, 0.0, 0.0},
                               };

void flash_leds(struct rgb* enabled_leds, int led_number, int whole_flash_time){

    for(int currentLedNumber = 0; currentLedNumber < led_number; currentLedNumber++){
        struct gpio_dt_spec* current_led = &rgb_enables[currentLedNumber];
        gpio_pin_set_dt(&rgb_components[0], enabled_leds[currentLedNumber].r>0.01);
        gpio_pin_set_dt(&rgb_components[1], enabled_leds[currentLedNumber].g>0.01);
        gpio_pin_set_dt(&rgb_components[2], enabled_leds[currentLedNumber].b>0.01);

        gpio_pin_set_dt(current_led, 1);
        k_sleep(K_USEC(whole_flash_time / led_number));
        gpio_pin_set_dt(current_led, 0);
    }
}

void set_leds(struct rgb* rgbPwmValues, int led_number) {
        
        struct rgb rgbBoolValues[ALL_LEDS_NUMBER];

        for(float currentIterationValue = 0; currentIterationValue < 1.0; currentIterationValue+=PWM_RESOLUTION){
            for(int ledNumber = 0; ledNumber < ALL_LEDS_NUMBER; ledNumber++){
                rgbBoolValues[ledNumber].r = rgbPwmValues[ledNumber].r > currentIterationValue;
                rgbBoolValues[ledNumber].g = rgbPwmValues[ledNumber].g > currentIterationValue;
                rgbBoolValues[ledNumber].b = rgbPwmValues[ledNumber].b > currentIterationValue;
            }
            flash_leds(rgbBoolValues, ALL_LEDS_NUMBER, FLASH_TIME_US);
        }
}
////////////////////////////////////////////////// THREADS

#define STACKSIZE 1024
#define PRIORITY 7

void led_thread(void){

    for(int i = 0; i < 3; i++){
        gpio_pin_configure_dt(&rgb_components[i], GPIO_OUTPUT_ACTIVE);
    }

    for(int i = 0; i < 10; i++){
        gpio_pin_configure_dt(&rgb_enables[i], GPIO_OUTPUT_ACTIVE);
    }

    for(int i = 0; i < 3; i++){
        gpio_pin_set_dt(&rgb_components[i], 0);
    }

    for(int i = 0; i < 10; i++){
        gpio_pin_set_dt(&rgb_enables[i], 0);
    }

    k_sleep(K_MSEC(1000));

    while(1) {
        set_leds(rgbPwmValues, ALL_LEDS_NUMBER);
        // k_sleep(K_MSEC(1000));
    }
}

void main_thread(void){
    while(1){
        k_sleep(K_MSEC(1000));
        rgbPwmValues[3].r = 1.0;
        rgbPwmValues[3].g = 1.0;
        rgbPwmValues[3].b = 1.0;
        k_sleep(K_MSEC(1000));
        rgbPwmValues[3].r = 0.0;
        rgbPwmValues[3].g = 0.0;
        rgbPwmValues[3].b = 0.0;
    }
}

K_THREAD_DEFINE(led_thread_id, STACKSIZE, led_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(main_thread_id, STACKSIZE, main_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

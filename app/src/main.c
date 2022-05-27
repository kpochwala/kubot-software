#include <zephyr.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <drivers/adc.h>
#include <stdio.h>

////////////////////////////////////////////////// ADC
#define ADC_RESOLUTION          12
#define ADC_GAIN                ADC_GAIN_1
#define ADC_REFERENCE           ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME    ADC_ACQ_TIME_DEFAULT

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

#define LABEL_AND_COMMA(node_id, prop, idx) \
	DT_LABEL(DT_IO_CHANNELS_CTLR_BY_IDX(node_id, idx)),

/* Labels of ADC controllers referenced by the above io-channels. */
static const char *const adc_labels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     LABEL_AND_COMMA)
};

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

static const struct gpio_dt_spec rgb_components[] = {rgb_k_r, rgb_k_g, rgb_k_b};

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

static const struct gpio_dt_spec ir_led = GPIO_DT_SPEC_GET(DT_NODELABEL(ir_led), gpios);

static const struct gpio_dt_spec rgb_enables[] = {rgb_en_0, rgb_en_1, rgb_en_2, rgb_en_3, rgb_en_4, rgb_en_5, rgb_en_6, rgb_en_7, rgb_en_8, rgb_en_9};

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

K_MUTEX_DEFINE(rgb_mutex);


struct printk_data_t {
    void *fifo_reserved; /* 1st word reserved for use by fifo */
    uint32_t adc_value;
};

K_FIFO_DEFINE(printk_fifo);


#define STACKSIZE 1024
#define PRIORITY 7



void led_thread(void){

    for(int i = 0; i < 3; i++){
        gpio_pin_configure_dt(&rgb_components[i], GPIO_OUTPUT_ACTIVE);
        gpio_pin_set_dt(&rgb_components[i], 0);
    }

    for(int i = 0; i < 10; i++){
        gpio_pin_configure_dt(&rgb_enables[i], GPIO_OUTPUT_ACTIVE);
        gpio_pin_set_dt(&rgb_enables[i], 0);
    }

    while(1) {
        if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0){
            set_leds(rgbPwmValues, ALL_LEDS_NUMBER);
            k_mutex_unlock(&rgb_mutex);
        }        
    }
}

void main_thread(void){
    while(1){
        k_sleep(K_MSEC(1000));

        if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
            rgbPwmValues[3].r = 1.0;
            rgbPwmValues[3].g = 1.0;
            rgbPwmValues[3].b = 1.0;
            k_mutex_unlock(&rgb_mutex);
        }

        k_sleep(K_MSEC(1000));

        if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
            rgbPwmValues[3].r = 0.0;
            rgbPwmValues[3].g = 0.0;
            rgbPwmValues[3].b = 0.0;
            k_mutex_unlock(&rgb_mutex);
        }

        struct printk_data_t tx_data = { .adc_value = 42 };
        size_t size = sizeof(struct printk_data_t);
        char *mem_ptr = k_malloc(size);
        __ASSERT_NO_MSG(mem_ptr != 0);
        memcpy(mem_ptr, &tx_data, size);
        k_fifo_put(&printk_fifo, mem_ptr);

    }
}

void usb_thread(void){
    const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    
    if (usb_enable(NULL)) {
        return;
    }

    /* Poll if the DTR flag was set */
    uint32_t dtr = 0;
    while (!dtr) {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        /* Give CPU resources to low priority threads. */
        k_sleep(K_MSEC(100));
    }

    printk("Hello World! %s\n", CONFIG_ARCH);

    while (1) {
        struct printk_data_t *rx_data = k_fifo_get(&printk_fifo, K_FOREVER);
        printk("Adc value: %d\n", rx_data->adc_value);
        k_free(rx_data);
    }

}

void adc_thread(void){

    gpio_pin_configure_dt(&ir_led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set_dt(&ir_led, 1);

    struct adc_dt_spec *dt_spec = &adc_channels[0];
    struct adc_channel_cfg channel_cfg = {
            .channel_id       = dt_spec->channel_id,
            .gain             = ADC_GAIN,
            .reference        = ADC_REFERENCE,
            .acquisition_time = ADC_ACQUISITION_TIME,
    };

    adc_channel_setup(dt_spec->dev, &channel_cfg);

    int16_t sample_buffer[1];

    while(1){
        struct adc_sequence sequence = {
            .buffer = sample_buffer,
            /* buffer size in bytes, not number of samples */
            .buffer_size = sizeof(sample_buffer),
            .resolution = ADC_RESOLUTION,
            .channels = BIT(dt_spec->channel_id),
            .oversampling = 0,
        };

        adc_read(dt_spec->dev, &sequence);

        struct printk_data_t tx_data = { .adc_value = sample_buffer[0] };
        size_t size = sizeof(struct printk_data_t);
        char *mem_ptr = k_malloc(size);
        __ASSERT_NO_MSG(mem_ptr != 0);
        memcpy(mem_ptr, &tx_data, size);
        k_fifo_put(&printk_fifo, mem_ptr);

        k_sleep(K_MSEC(100));
    }

}

K_THREAD_DEFINE(led_thread_id, STACKSIZE, led_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(adc_thread_id, STACKSIZE, adc_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(main_thread_id, STACKSIZE, main_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(usb_thread_id, STACKSIZE, usb_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

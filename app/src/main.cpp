#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/eeprom.h>
#include <stdio.h>
#include <zephyr/logging/log.h>

#include <vector>
#include <memory>

#define DISTANCE_THRESHOLD (CONFIG_VL53L0X_PROXIMITY_THRESHOLD/1000.0)

LOG_MODULE_REGISTER(app);

class SomeClass {

};

std::vector<std::shared_ptr<SomeClass>> someVector;

#ifdef __cplusplus
extern "C" {
#endif

static const struct device *get_eeprom_device(void)
{
	const struct device *dev = DEVICE_DT_GET(DT_ALIAS(eeprom_0));

	if (!device_is_ready(dev)) {
		printk("\nError: Device \"%s\" is not ready; "
		       "check the driver initialization logs for errors.\n",
		       dev->name);
		return NULL;
	}

	printk("Found EEPROM device \"%s\"\n", dev->name);
	return dev;
}


////////////////////////////////////////////////// ADC
#define ADC_RESOLUTION          12
#define ADC_GAIN                ADC_GAIN_1
#define ADC_REFERENCE           ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME    ADC_ACQ_TIME_DEFAULT
#define BL_RGB_OFFSET           10
#define BL_NUMBER               4

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
#define ALL_LEDS_NUMBER 14
#define PWM_RESOLUTION 0.1

#define PWM_CYCLE_PERIOD_US 10
#define FLASH_TIME_US 1\

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

static const struct gpio_dt_spec rgb_en_l0 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l0), gpios);
static const struct gpio_dt_spec rgb_en_l1 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l1), gpios);
static const struct gpio_dt_spec rgb_en_l2 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l2), gpios);
static const struct gpio_dt_spec rgb_en_l3 = GPIO_DT_SPEC_GET(DT_NODELABEL(rgb_en_l3), gpios);


static const struct gpio_dt_spec ir_led = GPIO_DT_SPEC_GET(DT_NODELABEL(ir_led), gpios);

static const struct gpio_dt_spec rgb_enables[] = {rgb_en_0, rgb_en_1, rgb_en_2, rgb_en_3, rgb_en_4, rgb_en_5, rgb_en_6, rgb_en_7, rgb_en_8, rgb_en_9, rgb_en_l3, rgb_en_l2, rgb_en_l0, rgb_en_l1};

// static const struct device *g_eeprom = DEVICE_DT_GET(DT_ALIAS(eeprom_0));
// static const struct device *eeprom = NULL;

static const char* tof_labels[] = {"V0", "V1", "V2", "V3", "V4", "V5", "V6", "V7", "V8", "V9"};
struct rgb {
    float r;
    float g;
    float b;
};

const struct rgb kabot_color = {255/255.0, 0/255.0, 72/255.0};
// const struct rgb kabot_color = {0/255.0, 0/255.0, 72/255.0};
const struct rgb kabot_init = {0/255.0, 0/255.0, 255/255.0};
const struct rgb kabot_warning = {255/255.0, 157/255.0, 0/255.0};
const struct rgb kabot_error = {255/255.0, 0/255.0, 0/255.0};

struct rgb rgbPwmValues[] = {kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init,
                             kabot_init};




struct rgb rgbPwmValues2[] = {{0.0, 0.0, 0.0},
                              {0.0, 0.0, 0.0},
                              {0.0, 0.0, 0.0},
                              {0.0, 0.0, 0.0},
                              {0.0, 0.0, 0.0},
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
////////////////////////////////////////////////// EEPROM

/*
 * Get a device structure from a devicetree node with alias eeprom0
 */


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

    for(int i = 0; i < ALL_LEDS_NUMBER; i++){
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

    k_sleep(K_MSEC(3000));

    const struct device *eeprom = get_eeprom_device();
    eeprom = get_eeprom_device();
    eeprom = get_eeprom_device();
    eeprom = get_eeprom_device();

    size_t eeprom_size;

    if(eeprom){
        eeprom_size = eeprom_get_size(eeprom);
        LOG_DBG("EEPROM initialized");
    }else{
        LOG_ERR("EEPROM initialization failed");
    }


    while(1){
        k_sleep(K_MSEC(1000));
        // if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
        //     rgbPwmValues[3].r = 1.0;
        //     rgbPwmValues[3].g = 1.0;
        //     rgbPwmValues[3].b = 1.0;
        //     k_mutex_unlock(&rgb_mutex);
        // }

        // k_sleep(K_MSEC(1000));

        // if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
        //     rgbPwmValues[3].r = 0.0;
        //     rgbPwmValues[3].g = 0.0;
        //     rgbPwmValues[3].b = 0.0;
        //     k_mutex_unlock(&rgb_mutex);
        // }

        // struct printk_data_t tx_data = { .adc_value = 42 };
        // size_t size = sizeof(struct printk_data_t);
        // char *mem_ptr = k_malloc(size);
        // __ASSERT_NO_MSG(mem_ptr != 0);
        // memcpy(mem_ptr, &tx_data, size);
        // k_fifo_put(&printk_fifo, mem_ptr);

    }
}

void usb_thread(void){
    const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (!device_is_ready(dev) || usb_enable(NULL)) {
		return;
	}
    
    dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
       
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

    for(int i = 0; i < BL_NUMBER; i++){
        struct adc_dt_spec *dt_spec = &adc_channels[i];
        struct adc_channel_cfg channel_cfg = {
                .gain             = ADC_GAIN,
                .reference        = ADC_REFERENCE,
                .acquisition_time = ADC_ACQUISITION_TIME,
                .channel_id       = dt_spec->channel_id,
        };
        adc_channel_setup(dt_spec->dev, &channel_cfg);
    }

    int16_t sample_buffer[1];

    struct adc_sequence sequence = {
        .buffer = sample_buffer,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(sample_buffer),
        .resolution = ADC_RESOLUTION,
        .oversampling = 0,
    };
    
    int adc_readings[BL_NUMBER];

    while(1){
        printk("BL: ");
        for(int i = 0; i < BL_NUMBER; i++){
            struct adc_dt_spec *dt_spec = &adc_channels[i];
            sequence.channels = BIT(dt_spec->channel_id);
            gpio_pin_set_dt(&ir_led, 1);
            k_sleep(K_MSEC(1));
            adc_read(dt_spec->dev, &sequence);
            gpio_pin_set_dt(&ir_led, 0);

            adc_readings[i] = sample_buffer[0];
            adc_read(dt_spec->dev, &sequence);
            adc_readings[i] -= sample_buffer[0];
            printk("BL%4d: %4d ", i, adc_readings[i]);

            if(adc_readings[i] < -10){
                if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
                    rgbPwmValues[BL_RGB_OFFSET + i] = kabot_warning;
                    k_mutex_unlock(&rgb_mutex);
                }
            }else{
                    if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
                    rgbPwmValues[BL_RGB_OFFSET + i] = kabot_color;
                    k_mutex_unlock(&rgb_mutex);
                }
            }
        }
        printk("\n");

        // struct printk_data_t tx_data = { .adc_value = sample_buffer[0] };
        // size_t size = sizeof(struct printk_data_t);
        // char *mem_ptr = k_malloc(size);
        // __ASSERT_NO_MSG(mem_ptr != 0);
        // memcpy(mem_ptr, &tx_data, size);
        // k_fifo_put(&printk_fifo, mem_ptr);

        k_sleep(K_MSEC(1));
    }

}

void fetch_tof(void){

    k_sleep(K_MSEC(1000));

    struct sensor_value values[10];
    struct sensor_value value;
    for(int i = 0; i < 10; i++) {
        // struct device* tof_1 = device_get_binding(DT_LABEL(VL53L0X));
        struct device* tof = device_get_binding(tof_labels[i]);
        
        int retval = 0;
        retval = sensor_sample_fetch(tof);
        
        printk("Sensor:  %s\n", tof_labels[i]);
        
        if(retval != 0){
            if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
                rgbPwmValues[i] = kabot_error;
                k_mutex_unlock(&rgb_mutex);
            }
            continue;
        }
    }

    while(1){
         k_sleep(K_MSEC(1));

        for(int i = 0; i < 10; i++) {
            // struct device* tof_1 = device_get_binding(DT_LABEL(VL53L0X));
            int retval = 0;
            struct device* tof = device_get_binding(tof_labels[i]);
            struct sensor_value value;
            values[i] = value;

            retval = sensor_sample_fetch(tof);
            if(retval != 0){
                if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
                    rgbPwmValues[i] = kabot_warning;
                    k_mutex_unlock(&rgb_mutex);
                }
                printk("Error fetching value from %s error code: %d\n", tof_labels[i], retval);
                continue;
            }
            retval = sensor_channel_get(tof, SENSOR_CHAN_DISTANCE, &value);
            if(retval != 0){
                if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
                    rgbPwmValues[i] = kabot_warning;
                    k_mutex_unlock(&rgb_mutex);
                }
                printk("Error getting value from %s error code: %d\n", tof_labels[i], retval);
                continue;
            }


            struct rgb active = {.r = 1.0, .g = 1.0, .b = 1.0};
            struct rgb inactive = kabot_color;

            if(sensor_value_to_double(&value) < DISTANCE_THRESHOLD){
                if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
                    rgbPwmValues[i] = active;
                    k_mutex_unlock(&rgb_mutex);
                }
            }else{
                if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0) {
                    rgbPwmValues[i] = inactive;
                    k_mutex_unlock(&rgb_mutex);
                }
            }
        }

        printk("Sensor values: ");
        for(int i=0; i<10; i++) {
            printk("%s: ", tof_labels[i]);
            printk("%.3fm ", sensor_value_to_double(&values[i]));
        }
        printk("\n");
    }

    // while(1){
    //     for(int i = 0; i < 5; i++) {
    //         // struct device* tof_1 = device_get_binding(DT_LABEL(VL53L0X));
    //         struct device* tof = device_get_binding(tof_labels[i]);
    //         sensor_sample_fetch(tof);
    //         sensor_channel_get(tof, SENSOR_CHAN_PROX, &value);
    //         printk("distance is %.3fm\n", sensor_value_to_double(&value));
    //     }
    //     k_sleep(K_MSEC(100));
    // }
}

K_THREAD_DEFINE(fetch_tof_id, STACKSIZE, fetch_tof, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(led_thread_id, STACKSIZE, led_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(adc_thread_id, STACKSIZE, adc_thread, NULL, NULL, NULL, PRIORITY+1, 0, 0);
K_THREAD_DEFINE(main_thread_id, STACKSIZE, main_thread, NULL, NULL, NULL, PRIORITY+5, 0, 0);
K_THREAD_DEFINE(usb_thread_id, STACKSIZE, usb_thread, NULL, NULL, NULL, PRIORITY+2, 0, 0);




static int cmd_foo(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(shell, "bar");

	return 0;
}
SHELL_CMD_REGISTER(foo, NULL, "This prints bar.", cmd_foo);

static int tof1_handler(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(shell, "tof1_handler");
	return 0;
}

SHELL_SUBCMD_SET_CREATE(sub_tof, (tof, tof1));
SHELL_SUBCMD_ADD((sub_tof), tof1, &sub_tof, "Help for tof1", tof1_handler, 1, 0);
SHELL_CMD_REGISTER(tof, &sub_tof, "Tof calibration commands", NULL);

#ifdef __cplusplus
}
#endif

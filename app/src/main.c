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
#include "sensor_thread.h"
#include "led_strip_charlieplex.h"


LOG_MODULE_REGISTER(app);


// std::vector<std::shared_ptr<SomeClass>> someVector;

// #ifdef __cplusplus
// extern "C" {
// #endif

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



BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
         "Console device is not ACM CDC UART device");

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);


static const struct gpio_dt_spec ir_led = GPIO_DT_SPEC_GET(DT_NODELABEL(ir_led), gpios);

// static const struct device *g_eeprom = DEVICE_DT_GET(DT_ALIAS(eeprom_0));
// static const struct device *eeprom = NULL;

////////////////////////////////////////////////// EEPROM

/*
 * Get a device structure from a devicetree node with alias eeprom0
 */


////////////////////////////////////////////////// THREADS




struct printk_data_t {
    void *fifo_reserved; /* 1st word reserved for use by fifo */
    uint32_t adc_value;
};

K_FIFO_DEFINE(printk_fifo);


#define STACKSIZE 1024
#define PRIORITY 7




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
        k_sleep(K_MSEC(1 ));
        for(int i = 0; i < ALL_SENSORS_NUMBER; i++){

            if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0){
                // struct rgb *current_led = &rgbPwmValues[i];

                if(k_mutex_lock(&tof_measurements_mutex, K_MSEC(100)) == 0){
                    struct distance_measurement *current_measurement = &tof_measurements[i];

                    if(current_measurement->err){
                        led_strip_set_led(NULL, kabot_warning, i);
                        continue;
                    }

                    if(current_measurement->in_range){
                        led_strip_set_led(NULL, kabot_active, i);
                    }else{
                        led_strip_set_led(NULL, kabot_inactive, i);
                    }
                    k_mutex_unlock(&tof_measurements_mutex);
                }
                k_mutex_unlock(&rgb_mutex);
            }
        }


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

                led_strip_set_led(NULL, kabot_warning, BL_RGB_OFFSET + i);
            }else{
                led_strip_set_led(NULL, kabot_color, BL_RGB_OFFSET + i);
            }
        }
        printk("\n");

        // struct printk_data_t tx_data = { .adc_value = sample_buffer[0] };
        // size_t size = sizeof(struct printk_data_t);
        // char *mem_ptr = k_malloc(size);
        // __ASSERT_NO_MSG(mem_ptr != 0);
        // memcpy(mem_ptr, &tx_data, size);
        // k_fifo_put(&printk_fifo, mem_ptr);

        k_sleep(K_MSEC(100));
    }

}

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

// #ifdef __cplusplus
// }
// #endif

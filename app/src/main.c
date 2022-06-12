#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/eeprom.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
#include "sensor_thread.h"
#include "led_strip_charlieplex.h"
#include "line_sensor.h"

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




BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
         "Console device is not ACM CDC UART device");

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);



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
        for(int i = 0; i < ALL_LINE_SENSOR_NUMBER; i++){
            if(k_mutex_lock(&line_measurements_mutex, K_MSEC(100)) == 0) {
                struct line_measurement *current_measurement = &line_measurements[i];
                if(current_measurement->white_line_detected){
                    led_strip_set_led(NULL, kabot_active, 10+i);
                }else{
                    led_strip_set_led(NULL, kabot_inactive, 10+i);
                }
                k_mutex_unlock(&line_measurements_mutex);
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

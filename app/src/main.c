#include <zephyr.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <stdio.h>


BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);


void main(void)
{
	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return;
	}

	if (!device_is_ready(led0.port)) {
		return;
	}

	if (!device_is_ready(led1.port)) {
		return;
	}

	if (gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) < 0) {
		return;
	}

	if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE) < 0) {
		return;
	}

	if (gpio_pin_toggle_dt(&led0) < 0) {
		return;
	}

	/* Poll if the DTR flag was set */
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
	}

	// set up VL53L0x

	dev = device_get_binding(DT_LABEL(DT_INST(0, st_vl53l0x)));
	struct sensor_value value;

	if (dev == NULL) {
		printk("Could not get VL53L0X device\n");
		return;
	}




	while (1) {
		printk("Hello World! %s\n", CONFIG_ARCH);
		k_sleep(K_MSEC(10));

		// if (gpio_pin_toggle_dt(&led0) < 0) {
		// 	return;
		// }


		int ret = 0;
		ret = sensor_sample_fetch(dev);
		if (ret) {
			printk("sensor_sample_fetch failed ret %d\n", ret);
			return;
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_PROX, &value);
		printk("prox is %d\n", value.val1);
		
		gpio_pin_set_dt(&led1, value.val1);


		ret = sensor_channel_get(dev,
					 SENSOR_CHAN_DISTANCE,
					 &value);
		printf("distance is %.3fm\n", sensor_value_to_double(&value));

	}
}

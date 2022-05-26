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
	bool r;
	bool g;
	bool b;
};

void main(void)
{
	for(int i = 0; i < 3; i++){
		gpio_pin_configure_dt(&rgb_components[i], GPIO_OUTPUT_ACTIVE);
	}

	for(int i = 0; i < 10; i++){
		gpio_pin_configure_dt(&rgb_enables[i], GPIO_OUTPUT_ACTIVE);
	}

	k_sleep(K_MSEC(1000));


	while(1){

		for(int i = 0; i < 3; i++){
			gpio_pin_set_dt(&rgb_components[i], 0);
		}

		for(int i = 0; i < 10; i++){
			gpio_pin_set_dt(&rgb_enables[i], 0);
		}

		struct gpio_dt_spec* previous_led = NULL;
		struct gpio_dt_spec* previous_color = NULL;

		for(int i = 0; i < 3; i++) {
			// k_sleep(K_MSEC(1000));

			struct gpio_dt_spec* current_color = &rgb_components[i];

			if(previous_color != NULL){
				gpio_pin_set_dt(previous_color, 0);
			}
			gpio_pin_set_dt(current_color, 1);
			previous_color = current_color;

			////////////////////////////////////////////////
			// for(int i = 0; i < 3; i++){
			// 	gpio_pin_set_dt(&rgb_components[i], 1);
			// }	
			////////////////////////////////////////////////


			gpio_pin_set_dt(&rgb_components[0], 1);
			gpio_pin_set_dt(&rgb_components[1], 1);
			gpio_pin_set_dt(&rgb_components[2], 1);

			for (int i = 0; i < 10; i++) {
				struct gpio_dt_spec* current_led = &rgb_enables[i];

				if(previous_led != NULL){
					gpio_pin_set_dt(previous_led, 0);
				}
				gpio_pin_set_dt(current_led, 1);
				previous_led = current_led;
				k_sleep(K_MSEC(2));
			}
		}
	}

	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

	// gpio_pin_set_dt(&rgb_k_r, 1);
	// gpio_pin_set_dt(&rgb_k_r, 1);
	// k_sleep(K_MSEC(100));
	// gpio_pin_set_dt(&rgb_k_r, 0);
	// k_sleep(K_MSEC(100));




	if (usb_enable(NULL)) {
		return;
	}

	if (!device_is_ready(led0.port)) {
		return;
	}

	if (!device_is_ready(led1.port)) {
		return;
	}

	// if (gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) < 0) {
	// 	return;
	// }

	// if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE) < 0) {
	// 	return;
	// }

	// if (gpio_pin_toggle_dt(&led0) < 0) {
	// 	return;
	// }

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

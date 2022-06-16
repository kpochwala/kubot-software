#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>

#define RING_BUF_SIZE 1024
uint8_t ring_buffer[RING_BUF_SIZE];

struct ring_buf ringbuf;

LOG_MODULE_REGISTER(kabot_usb);

// BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
//          "Console device is not ACM CDC UART device");

struct printk_data_t {
    void *fifo_reserved; /* 1st word reserved for use by fifo */
    uint32_t adc_value;
};

K_FIFO_DEFINE(printk_fifo);

void usb_thread(void){
    const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (!device_is_ready(dev) || usb_enable(NULL)) {
		return;
	}
    
    dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

    ring_buf_init(&ringbuf, sizeof(ring_buffer), ring_buffer);
       
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


#define PRIORITY 9
#define STACKSIZE KB(4)
K_THREAD_DEFINE(usb_thread_id, STACKSIZE, usb_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

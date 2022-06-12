#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include "led_strip_charlieplex.h"
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(line_sensor);

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

static const struct gpio_dt_spec ir_led = GPIO_DT_SPEC_GET(DT_NODELABEL(ir_led), gpios);

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

#define STACKSIZE 1024
#define PRIORITY 7
K_THREAD_DEFINE(adc_thread_id, STACKSIZE, adc_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

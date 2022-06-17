#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include "led_strip_charlieplex.h"
#include <zephyr/drivers/gpio.h>
#include "robot_sensors/line_sensor.h"

LOG_MODULE_REGISTER(line_sensor);
K_MUTEX_DEFINE(line_measurements_mutex);

struct line_measurement line_measurements[ALL_LINE_SENSOR_NUMBER];

////////////////////////////////////////////////// ADC
#define ADC_RESOLUTION          12
#define ADC_GAIN                ADC_GAIN_1
#define ADC_REFERENCE           ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME    ADC_ACQ_TIME_DEFAULT
#define BL_RGB_OFFSET           10

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
    
    struct line_measurement* current_measurement;
    
    for(int i = 0; i < ALL_LINE_SENSOR_NUMBER; i++){
        // initialize ADC
        struct adc_dt_spec *dt_spec = &adc_channels[i];
        struct adc_channel_cfg channel_cfg = {
                .gain             = ADC_GAIN,
                .reference        = ADC_REFERENCE,
                .acquisition_time = ADC_ACQUISITION_TIME,
                .channel_id       = dt_spec->channel_id,
        };
        adc_channel_setup(dt_spec->dev, &channel_cfg);
        if(k_mutex_lock(&line_measurements_mutex, K_MSEC(1000)) == 0) {
            // initialize line readings
            current_measurement = &line_measurements[i];
            current_measurement->threshold = -10;
            current_measurement->white_line_detected = false;   
            k_mutex_unlock(&line_measurements_mutex);
        }
    }

    int16_t sample_buffer[1];

    struct adc_sequence sequence = {
        .buffer = sample_buffer,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(sample_buffer),
        .resolution = ADC_RESOLUTION,
        .oversampling = 0,
    };
    
    int adc_readings[ALL_LINE_SENSOR_NUMBER];
    
    while(1){
        for(int i = 0; i < ALL_LINE_SENSOR_NUMBER; i++){

            struct adc_dt_spec *dt_spec = &adc_channels[i];
            sequence.channels = BIT(dt_spec->channel_id);
            gpio_pin_set_dt(&ir_led, 1);
            k_sleep(K_MSEC(1));
            adc_read(dt_spec->dev, &sequence);
            gpio_pin_set_dt(&ir_led, 0);

            adc_readings[i] = sample_buffer[0];
            adc_read(dt_spec->dev, &sequence);
            adc_readings[i] -= sample_buffer[0];

            if(k_mutex_lock(&line_measurements_mutex, K_MSEC(100)) == 0) {
                current_measurement = &line_measurements[i];
                current_measurement->raw = adc_readings[i];
                if(adc_readings[i] < current_measurement->threshold){
                    current_measurement->white_line_detected = true;
                }else{
                    current_measurement->white_line_detected = false;
                }            
                k_mutex_unlock(&line_measurements_mutex);
            }

        }
        k_sleep(K_MSEC(1));
    }

}

static struct line_measurement copy[ALL_LINE_SENSOR_NUMBER];
struct line_measurement* get_line(){
    if(k_mutex_lock(&line_measurements_mutex, K_MSEC(100)) == 0){
        memcpy(copy, line_measurements, sizeof(copy));
        k_mutex_unlock(&line_measurements_mutex);
    }
    return copy;
}

#define STACKSIZE KB(2)
#define PRIORITY 7
K_THREAD_DEFINE(adc_thread_id, STACKSIZE, adc_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

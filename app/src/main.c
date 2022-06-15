#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/eeprom.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

#include "sensor_thread.h"
#include "led_strip_charlieplex.h"
#include "line_sensor.h"

#include "rc5.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

#include "arm_math.h"
arm_pid_instance_f32 PID;

LOG_MODULE_REGISTER(app);


// std::vector<std::shared_ptr<SomeClass>> someVector;

// #ifdef __cplusplus
// extern "C" {
// #endif




static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);



// static const struct device *g_eeprom = DEVICE_DT_GET(DT_ALIAS(eeprom_0));
// static const struct device *eeprom = NULL;

////////////////////////////////////////////////// EEPROM

/*
 * Get a device structure from a devicetree node with alias eeprom0
 */


////////////////////////////////////////////////// THREADS

static const struct gpio_dt_spec m1_in1 = GPIO_DT_SPEC_GET(DT_ALIAS(m1_in1), gpios);
static const struct gpio_dt_spec m1_in2 = GPIO_DT_SPEC_GET(DT_ALIAS(m1_in2), gpios);
static const struct gpio_dt_spec m2_in1 = GPIO_DT_SPEC_GET(DT_ALIAS(m2_in1), gpios);
static const struct gpio_dt_spec m2_in2 = GPIO_DT_SPEC_GET(DT_ALIAS(m2_in2), gpios);

static const struct pwm_dt_spec m1_d1 = PWM_DT_SPEC_GET(DT_ALIAS(m1_d1));
static const struct pwm_dt_spec m1_d2 = PWM_DT_SPEC_GET(DT_ALIAS(m1_d2));
static const struct pwm_dt_spec m2_d1 = PWM_DT_SPEC_GET(DT_ALIAS(m2_d1));
static const struct pwm_dt_spec m2_d2 = PWM_DT_SPEC_GET(DT_ALIAS(m2_d2));


int set_m1_d1_pwm(float pwm){
    return pwm_set_pulse_dt(&m1_d1, m1_d1.period * pwm);
}

int set_m1_d2_pwm(float pwm){
    return pwm_set_pulse_dt(&m1_d2, m1_d2.period * pwm);
}

int set_m2_d1_pwm(float pwm){
    return pwm_set_pulse_dt(&m2_d1, m2_d1.period * pwm);
}

int set_m2_d2_pwm(float pwm){
    return pwm_set_pulse_dt(&m2_d2, m2_d2.period * pwm);
}

int init_m1_gpio(){
    int ret = 0;
    ret = gpio_pin_configure_dt(&m1_in1, GPIO_OUTPUT_INACTIVE);
    if(ret){
        return ret;
    }

    ret = gpio_pin_configure_dt(&m1_in2, GPIO_OUTPUT_INACTIVE);
    if(ret){
        return ret;
    }

    return ret;
}

int init_m2_gpio(){
    int ret = 0;
    ret = gpio_pin_configure_dt(&m2_in1, GPIO_OUTPUT_INACTIVE);
    if(ret){
        return ret;
    }

    ret = gpio_pin_configure_dt(&m2_in2, GPIO_OUTPUT_INACTIVE);
    if(ret){
        return ret;
    }

    return ret;
}

int set_m1_forward(){
    gpio_pin_set_dt(&m1_in1, 1);
    gpio_pin_set_dt(&m1_in2, 0);
}
int set_m1_backward(){
    gpio_pin_set_dt(&m1_in1, 0);
    gpio_pin_set_dt(&m1_in2, 1);
}
int set_m2_forward(){
    gpio_pin_set_dt(&m2_in1, 1);
    gpio_pin_set_dt(&m2_in2, 0);
}
int set_m2_backward(){
    gpio_pin_set_dt(&m2_in1, 0);
    gpio_pin_set_dt(&m2_in2, 1);
}

int set_m1_pwm(float pwm){
    set_m1_d1_pwm(pwm);
    set_m1_d2_pwm(1.0);
}
int set_m2_pwm(float pwm){
    set_m2_d1_pwm(pwm);
    set_m2_d2_pwm(1.0);
}

int set_m1(float speed){
    bool dir = speed > 0;
    if(dir){
        set_m1_pwm(speed);
        set_m1_forward();
    }else{
        set_m1_pwm(-speed);
        set_m1_backward();
    }
}
int set_m2(float speed){
    bool dir = speed > 0;
    if(dir){
        set_m2_pwm(speed);
        set_m2_forward();
    }else{
        set_m2_pwm(-speed);
        set_m2_backward();
    }
}

int set_motors(float speed_m1, float speed_m2){
    set_m1(speed_m1);
    set_m2(speed_m2);
}

int init_motors(){
    init_m1_gpio();
    init_m2_gpio();
}


struct vec3f {
    float x;
    float y;
    float z;
};

int set_motors_vec(struct vec3f* translation, struct vec3f* rotation) {

    float m1 = translation->x + 0.5 * rotation->z;
    float m2 = translation->x - 0.5 * rotation->z;

    return set_motors(m1, m2);
}

#define STACKSIZE 1024
#define PRIORITY 7




void main_thread(void){

    k_sleep(K_MSEC(1000));
    init_motors();
    rc5_init();

    // struct vec3f t;
    // struct vec3f r;

    // t.x = 0.5;
    // r.z = 1.0;
    // set_motors_vec(&t, &r);
    // k_sleep(K_MSEC(1000));

    // t.x = -0.5;
    // r.z = 1.0;
    // set_motors_vec(&t, &r);
    // k_sleep(K_MSEC(1000));

    // t.x = 0.0;
    // r.z = 2.0;
    // set_motors_vec(&t, &r);
    // k_sleep(K_MSEC(1000));
    
    // t.x = 0.0;
    // r.z = -2.0;
    // set_motors_vec(&t, &r);
    // k_sleep(K_MSEC(1000));

    set_motors(0.0, 0.0);

    // for(float f = 0; f <= 1.0; f += 0.01){
    //     set_motors(f, f);
    //     k_sleep(K_MSEC(20));
    // }
    // for(float f = 1.0; f >= -1.0; f -= 0.01){
    //     set_motors(f, f);
    //     k_sleep(K_MSEC(20));
    // }
    // for(float f = -1.0; f <= 0.0; f += 0.01){
    //     set_motors(f, f);
    //     k_sleep(K_MSEC(20));
    // }

    // k_sleep(K_MSEC(1000));
    // set_motors(-1.0, -1.0);
    // k_sleep(K_MSEC(1000));
    // set_motors(0.0, 0.0);


	// const struct device *imu = DEVICE_DT_GET(DT_ALIAS(imu));
	// if (!imu) {
	// 	LOG_ERR("Failed to find sensor %s\n", "IMU");
    //     led_strip_set_led(NULL, kabot_error, 5);
	// 	return;
	// }
    // if (!device_is_ready(imu)) {
	// 	LOG_ERR("Failed to find sensor %s\n", "IMU");
    //     led_strip_set_led(NULL, kabot_warning, 5);
	// 	return;
	// }


    while(1){
        k_sleep(K_MSEC(100));

        int command;
        if(rc5_new_command_received(&command)){
            rc5_reset();
            LOG_DBG("RC5 received");
            LOG_DBG("Start bits:   0x%x", rc5_get_start_bits(command));
            LOG_DBG("Toggle bits:  0x%x",rc5_get_toggle_bit(command));
            LOG_DBG("Address bits: 0x%x",rc5_get_address_bits(command));
            LOG_DBG("Command bits: 0x%x",rc5_get_command_bits(command));
        }


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




K_THREAD_DEFINE(main_thread_id, STACKSIZE, main_thread, NULL, NULL, NULL, PRIORITY+5, 0, 0);





// #ifdef __cplusplus
// }
// #endif

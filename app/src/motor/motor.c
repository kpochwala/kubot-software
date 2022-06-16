#include "motor/motor.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

static const struct gpio_dt_spec m1_in1 = GPIO_DT_SPEC_GET(DT_ALIAS(m1_in1), gpios);
static const struct gpio_dt_spec m1_in2 = GPIO_DT_SPEC_GET(DT_ALIAS(m1_in2), gpios);
static const struct gpio_dt_spec m2_in1 = GPIO_DT_SPEC_GET(DT_ALIAS(m2_in1), gpios);
static const struct gpio_dt_spec m2_in2 = GPIO_DT_SPEC_GET(DT_ALIAS(m2_in2), gpios);

static const struct pwm_dt_spec m1_d1 = PWM_DT_SPEC_GET(DT_ALIAS(m1_d1));
static const struct pwm_dt_spec m1_d2 = PWM_DT_SPEC_GET(DT_ALIAS(m1_d2));
static const struct pwm_dt_spec m2_d1 = PWM_DT_SPEC_GET(DT_ALIAS(m2_d1));
static const struct pwm_dt_spec m2_d2 = PWM_DT_SPEC_GET(DT_ALIAS(m2_d2));

const float speed_multiplier = 0.5;

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
  
    set_m1(speed_m1 * speed_multiplier);
    set_m2(speed_m2 * speed_multiplier);
}

int motors_init(){
    init_m1_gpio();
    init_m2_gpio();
}


int set_motors_vec(struct vec3f* translation, struct vec3f* rotation) {

    float m1 = translation->x + 0.5 * rotation->z;
    float m2 = translation->x - 0.5 * rotation->z;

    return set_motors(m1, m2);
}

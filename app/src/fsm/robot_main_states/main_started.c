#include "fsm/start_module_fsm.h"

#include "led_strip_charlieplex.h"
#include "robot_sensors/sensor_thread.h"
#include "robot_sensors/line_sensor.h"
#include <string.h>
#include <zephyr/logging/log.h>
#include "motor/motor.h"

LOG_MODULE_REGISTER(main_started);


static struct distance_measurement tof_measurements[ALL_SENSORS_NUMBER];
static struct line_measurement line_measurements[ALL_LINE_SENSOR_NUMBER];


double tof_weights[] = {
    -0.8,
    -1.0,
    -0.3,
    -0.5,
    -0.1,
     0.1,
     0.5,
     0.3,
     1.0,
     0.8,
};

struct sensor_data {
    double angle;
    int count_of_active;
};

struct sensor_data calculate_angle(){
    struct sensor_data data;
    data.angle = 0.0;
    data.count_of_active = 0;

    for(int i = 0; i < ALL_SENSORS_NUMBER; i++){
        if(tof_measurements[i].in_range){
            data.angle += tof_weights[i];
            data.count_of_active += 1;
        }
    }

    if(data.count_of_active == 0){
        data.angle = -1.0;
    }else{
        data.angle /= data.count_of_active;
    }
    
    return data;
}


void main_started(){

    memcpy(tof_measurements, get_tof(), sizeof(tof_measurements));
    memcpy(line_measurements, get_line(), sizeof(line_measurements));

    struct sensor_data sensor_d = calculate_angle();
    
    double left_motor = 1.0 + sensor_d.angle;
    double right_motor = 1.0 - sensor_d.angle;

    set_motors(left_motor, right_motor);

    LOG_INF("Measured angle: %6f", sensor_d.angle);
    LOG_INF("Count of active: %6d", sensor_d.count_of_active);



    for(int i = 0; i < ALL_SENSORS_NUMBER; i++){

        if(tof_measurements[i].err){
            set_led(i, kabot_warning);
            continue;
        }

        if(tof_measurements[i].in_range){
            LOG_INF("Distance for sensor %d is %d", i, tof_measurements[i].distance_mm);
            set_led(i, kabot_active);
        }else{
            set_led(i, kabot_inactive);
        }

    }
    for(int i = 0; i < ALL_LINE_SENSOR_NUMBER; i++){
        if(line_measurements[i].white_line_detected){
            set_led(ALL_SENSORS_NUMBER+i, kabot_active);
        }else{
            set_led(10+i, kabot_inactive);
        }
    }
}

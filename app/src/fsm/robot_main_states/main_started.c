#include "fsm/start_module_fsm.h"

#include "led_strip_charlieplex.h"
#include "robot_sensors/sensor_thread.h"
#include "robot_sensors/line_sensor.h"
#include <string.h>

struct distance_measurement tof_measurements[ALL_SENSORS_NUMBER];

void main_started(){

    memcpy(tof_measurements, get_tof(), sizeof(tof_measurements));

    for(int i = 0; i < ALL_SENSORS_NUMBER; i++){
        if(k_mutex_lock(&rgb_mutex, K_MSEC(100)) == 0){

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

#include "fsm/start_module_fsm.h"

#include "led_strip_charlieplex.h"
#include "robot_sensors/sensor_thread.h"
#include "robot_sensors/line_sensor.h"
#include <string.h>

struct distance_measurement tof_measurements[ALL_SENSORS_NUMBER];

void main_started(){

    memcpy(tof_measurements, get_tof(), sizeof(tof_measurements));

    for(int i = 0; i < ALL_SENSORS_NUMBER; i++){

        struct distance_measurement *current_measurement = &tof_measurements[i];
        if(current_measurement->err){
            set_led(i, kabot_warning);
            continue;
        }

        if(current_measurement->in_range){
            set_led(i, kabot_active);
        }else{
            set_led(i, kabot_inactive);
        }


    }
    for(int i = 0; i < ALL_LINE_SENSOR_NUMBER; i++){
        struct line_measurement *current_measurement = &line_measurements[i];
        if(current_measurement->white_line_detected){
            set_led(10+i, kabot_active);
        }else{
            set_led(10+i, kabot_inactive);
        }
    }
}

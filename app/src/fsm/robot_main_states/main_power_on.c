#include "fsm/start_module_fsm.h"
#include "led_strip_charlieplex.h"
#include "robot_sensors/sensor_thread.h"
#include "robot_sensors/line_sensor.h"
#include <string.h>

static struct distance_measurement tof_measurements[ALL_SENSORS_NUMBER];
static struct line_measurement line_measurements[ALL_LINE_SENSOR_NUMBER];

void main_power_on(){

    memcpy(tof_measurements, get_tof(), sizeof(tof_measurements));
    memcpy(line_measurements, get_line(), sizeof(line_measurements));

    for(int i = 0; i < ALL_SENSORS_NUMBER; i++){

        if(tof_measurements[i].err){
            set_led(i, kabot_warning);
            continue;
        }

        if(tof_measurements[i].in_range){
            set_led(i, kabot_active);
        }else{
            set_led(i, kabot_off);
        }

    }
    for(int i = 0; i < ALL_LINE_SENSOR_NUMBER; i++){
        if(line_measurements[i].white_line_detected){
            set_led(ALL_SENSORS_NUMBER+i, kabot_active);
        }else{
            set_led(10+i, kabot_off);
        }
    }
}

#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 
#include "eeprom/eeprom_helper.h"
#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"
#include "led_strip_charlieplex.h"
#include "robot_sensors/sensor_thread.h"

LOG_MODULE_REGISTER(state_select_sensor);

static int current_sensor = 0;

static void increment_sensor(){
    if(current_sensor == ALL_SENSORS_NUMBER){
        current_sensor = 0;
    }else{
        current_sensor++;
    }
}
static void decrement_sensor(){
    if(current_sensor == 0){
        current_sensor = ALL_SENSORS_NUMBER;
    }else{
        current_sensor--;
    }
}

void select_sensor_entry(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
}
void select_sensor_exit(void *o){
    ARG_UNUSED(o);  
    LOG_DBG("");
    set_led_all(kabot_off);
}
void select_sensor_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    set_led_all(kabot_inactive);
    set_led(current_sensor, kabot_warning);

    struct rc5_data data = fetch_rc5_data(&s_obj);

    if(is_kabot_custom_command(data.address, data.command)){
        switch(data.command){
            case KABOT_UP:
                increment_sensor();
                LOG_WRN("Increment.");
            break;
            case KABOT_DOWN:
                decrement_sensor();
                LOG_WRN("Decrement.");
            break;
            case KABOT_RED:
                s_obj.current_selected_sensor = current_sensor;
                smf_set_state(SMF_CTX(&s_obj), &start_module_states[CALIBRATE_THRESHOLD]);
            break;
            case KABOT_GREEN:
                s_obj.current_selected_sensor = current_sensor;
                smf_set_state(SMF_CTX(&s_obj), &start_module_states[CALIBRATE_OFFSET]);
            break;
            case KABOT_EXIT:
                smf_set_state(SMF_CTX(&s_obj), &start_module_states[STOPPED]);
            break;
            default:
                LOG_WRN("Unsupported command: 0x%x", data.command);
            break;
        }
    }

    k_sleep(K_MSEC(200)); // debounce
}

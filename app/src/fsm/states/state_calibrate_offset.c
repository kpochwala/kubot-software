#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 
#include "eeprom/eeprom_helper.h"
#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"
#include "led_strip_charlieplex.h"
#include "robot_sensors/sensor_thread.h"

LOG_MODULE_REGISTER(state_calibrate_offset);

static int current_sensor = 0;

void calibrate_offset_entry(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    
    current_sensor = s_obj.current_selected_sensor;
}
void calibrate_offset_exit(void *o){
    ARG_UNUSED(o);  
    LOG_DBG("");
    set_led_all(kabot_off);
}
void calibrate_offset_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    set_led_all(kabot_ok);
    set_led(current_sensor, kabot_active);

    struct rc5_data data = fetch_rc5_data(&s_obj);
    if(is_kabot_custom_command(data.address, data.command)){
        switch(data.command){
            case KABOT_OK:
                // run calibration
                // blink
                // save calibration to eeprom
                LOG_WRN("Calibrate offset");
                smf_set_state(SMF_CTX(&s_obj), &start_module_states[SELECT_SENSOR]);
            break;
            case KABOT_EXIT:
                smf_set_state(SMF_CTX(&s_obj), &start_module_states[SELECT_SENSOR]);
            break;
            default:
                LOG_WRN("Unsupported command: 0x%x", data.command);
            break;
        }
    }

    k_sleep(K_MSEC(200)); // debounce
}

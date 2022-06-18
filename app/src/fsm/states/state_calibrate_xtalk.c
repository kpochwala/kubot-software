#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 

#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"
#include "led_strip_charlieplex.h"
#include "robot_sensors/sensor_thread.h"

#include "eeprom/eeprom_helper.h"
#include "eeprom/eeprom_structs.h"

LOG_MODULE_REGISTER(state_calibrate_xtalk);
#define CALIBRATION_DISTANCE_MM (80+150)


static int current_sensor = 0;

static struct led_rgb color;

static const struct device* sensor_device;
static uint32_t xtalk_compensation_megacps;

static void blink_save(){
    
    set_led_all(kabot_off);
    k_sleep(K_MSEC(100));
    set_led_all(color);
    set_led(current_sensor, kabot_active);
    k_sleep(K_MSEC(100));
    set_led_all(kabot_off);
    k_sleep(K_MSEC(100));
    set_led_all(color);
    set_led(current_sensor, kabot_active);
}

void calibrate_xtalk_entry(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    color = kabot_warning;
    current_sensor = s_obj.current_selected_sensor;
    sensor_device = get_tof_device(current_sensor);
}
void calibrate_xtalk_exit(void *o){
    ARG_UNUSED(o);  
    LOG_DBG("");
    set_led_all(kabot_off);
}
void calibrate_xtalk_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    set_led_all(color);
    set_led(current_sensor, kabot_active);

    struct rc5_data data = fetch_rc5_data(&s_obj);
    if(is_kabot_custom_command(data.address, data.command)){
        switch(data.command){
            case KABOT_OK:
                // run calibration
                // blink
                // save calibration to eeprom
                if (k_sem_take(&tof_semaphore, K_MSEC(1000)) == 0){
                    LOG_WRN("Calibrate xtalk");
                    smf_set_state(SMF_CTX(&s_obj), &start_module_states[SELECT_SENSOR]);

                    vl53l0x_extra_calibrate_xtalk(sensor_device, CALIBRATION_DISTANCE_MM, &xtalk_compensation_megacps);
                    LOG_DBG("xtalk after calibration: %d", xtalk_compensation_megacps);
                    
                    struct eeprom_view copy;
                    read_eeprom_into(&copy);
                    copy.tof.sensor[current_sensor].xtalk_compensation_megacps = xtalk_compensation_megacps;
                    write_eeprom(&copy);

                    blink_save();
                }else{
                    LOG_WRN("Could not take tof semaphore");
                }
                k_sem_give(&tof_semaphore);

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

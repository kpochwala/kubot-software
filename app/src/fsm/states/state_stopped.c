#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 
#include "eeprom/eeprom_helper.h"
#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"

LOG_MODULE_REGISTER(state_stopped);

void stopped_entry(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    set_start(false);
    set_kill(true);

    s_obj.rc5_command = -1;
    s_obj.rc5_address = -1;
}
void stopped_exit(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
}
void stopped_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    if(is_kabot_custom_command(s_obj.rc5_address, s_obj.rc5_command)){
        switch(s_obj.rc5_command){
            case KABOT_POWER:
                smf_set_state(SMF_CTX(&s_obj), &start_module_states[POWER_ON]);
            break;
            case KABOT_OK:
                smf_set_state(SMF_CTX(&s_obj), &start_module_states[SELECT_SENSOR]);
            break;
            default:
                LOG_WRN("Unsupported command: 0x%x", s_obj.rc5_command);
            break;
        }
    }
    main_stopped();
}

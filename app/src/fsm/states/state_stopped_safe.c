
#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 
#include "eeprom/eeprom_helper.h"
#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"

LOG_MODULE_REGISTER(state_stopped_safe);

void stopped_safe_entry(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    set_start(false);
    set_kill(true);

    s_obj.rc5_command = -1;
    s_obj.rc5_address = -1;
}
void stopped_safe_exit(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
}
void stopped_safe_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    // delay 1000ms
    // set led flashing
    smf_set_state(SMF_CTX(&s_obj), &start_module_states[STOPPED]);
    main_stopped_safe();
}

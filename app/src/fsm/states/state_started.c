#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 
#include "eeprom/eeprom_helper.h"
#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"

LOG_MODULE_REGISTER(state_started);

void started_entry(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    set_start(true);
    set_kill(false);

    s_obj.rc5_command = -1;
    s_obj.rc5_address = -1;
}
void started_exit(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
}
void started_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    if(is_stop_command(s_obj.rc5_address, s_obj.rc5_command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[STOPPED_SAFE]);
    }
    // if(is_program_command(s_obj.rc5_address, s_obj.rc5_command)){
    //     smf_set_state(SMF_CTX(&s_obj), &start_module_states[PROGRAMMING]);
    // }
    main_started();
}

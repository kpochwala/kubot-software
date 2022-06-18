#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 
#include "eeprom/eeprom_helper.h"
#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"

LOG_MODULE_REGISTER(state_programming);

void programming_entry(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    set_start(false);
    set_kill(true);
    save_dohyo_address(s_obj.new_dohyo_address);
}
void programming_exit(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
}
void programming_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
    //todo: flash twice
    smf_set_state(SMF_CTX(&s_obj), &start_module_states[POWER_ON]);
    main_programming();
}

#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h" 
#include "eeprom/eeprom_helper.h"
#include <zephyr/logging/log.h>
#include "fsm/robot_main_states.h"

LOG_MODULE_REGISTER(state_power_on);

void power_on_entry(void *o){
    LOG_DBG("");
    set_start(false);
    set_kill(true);

    struct eeprom_view copy;
    read_eeprom_into(&copy);

    k_sleep(K_MSEC(1000));

    s_obj.current_dohyo_address = copy.start_module.dohyo_address;
    LOG_DBG("dohyo address: 0x%x", s_obj.current_dohyo_address);
    // todo: set leds
}
void power_on_exit(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");
}
void power_on_run(void *o){
    ARG_UNUSED(o);
    LOG_DBG("");

    struct rc5_data data = fetch_rc5_data(&s_obj);

    if(is_start_command(data.address, data.command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[STARTED]);
    }
    if(is_stop_command(data.address, data.command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[STOPPED_SAFE]);
    }
    if(is_program_command(data.address, data.command)){
        s_obj.new_dohyo_address = data.command;
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[PROGRAMMING]);
    }
    main_power_on();
}

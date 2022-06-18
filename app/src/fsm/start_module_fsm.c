#include "fsm/start_module_fsm.h"
#include "rc5/rc5.h"

#include <zephyr/logging/log.h>
#include "eeprom/eeprom_helper.h"
#include "fsm/robot_main_states.h"
#include "fsm/states.h"

LOG_MODULE_REGISTER(start_module_fsm);

static bool is_initialized;

static const int start_command = 1;
static const int stop_command = 0;
static const int program_command_address = 0xb;
static const int start_stop_command_address = 0x7;
static const int kabot_custom_command_rc5_address = 0x00;
static const int kabot_reset_start_module_rc5_command = 0x0c;

struct smf_state start_module_states[] = {
    [POWER_ON] = SMF_CREATE_STATE(power_on_entry, power_on_run, power_on_exit),
    [PROGRAMMING] = SMF_CREATE_STATE(programming_entry, programming_run, programming_exit),
    [STARTED] = SMF_CREATE_STATE(started_entry, started_run, started_exit),
    [STOPPED_SAFE] = SMF_CREATE_STATE(stopped_safe_entry, stopped_safe_run, stopped_safe_exit),
    [STOPPED] = SMF_CREATE_STATE(stopped_entry, stopped_run, stopped_exit),
    [SELECT_SENSOR] = SMF_CREATE_STATE(select_sensor_entry, select_sensor_run, select_sensor_exit),
    [CALIBRATE_THRESHOLD] = SMF_CREATE_STATE(calibrate_threshold_entry, calibrate_threshold_run, calibrate_threshold_exit),
    [CALIBRATE_OFFSET] = SMF_CREATE_STATE(calibrate_offset_entry, calibrate_offset_run, calibrate_offset_exit),
    [CALIBRATE_XTALK] = SMF_CREATE_STATE(calibrate_xtalk_entry, calibrate_xtalk_run, calibrate_xtalk_exit),
};



struct rc5_data fetch_rc5_data(struct s_object* object){
    struct rc5_data command;
    command.address = object->rc5_address;
    command.command = object->rc5_command;
    object->rc5_command = -1;
    object->rc5_address = -1;
    return command;
}

struct s_object s_obj;

// module machine state commands:


void set_start(bool value){
    LOG_DBG("Setting start to: %d", value);
}

void set_kill(bool value){
    LOG_DBG("Setting kill to: %d", value);
}

void save_dohyo_address(int address){
    LOG_DBG("Setting dohyo address to 0x%x", address);
    s_obj.current_dohyo_address = address;
    volatile struct eeprom_view copy;
    read_eeprom_into(&copy);
    copy.start_module.dohyo_address = address;
    write_eeprom(&copy);
}

bool is_start_command(int rc5_address, int rc5_command){
    if(rc5_address != start_stop_command_address){
        return false;
    }
    if(rc5_command == start_command + s_obj.current_dohyo_address) {
        return true;
    }
    return false;
}

bool is_stop_command(int rc5_address, int rc5_command){
    if(rc5_address != start_stop_command_address){
        return false;
    }
    if(rc5_command == stop_command + s_obj.current_dohyo_address) {
        return true;
    }
    return false;
}

bool is_program_command(int rc5_address, int rc5_command){
    return rc5_address == program_command_address;
}

bool is_kabot_custom_command(int rc5_address, int rc5_command){
    return rc5_address == kabot_custom_command_rc5_address;
}


void fsm_start_module_init(){
    smf_set_initial(SMF_CTX(&s_obj), &start_module_states[POWER_ON]);
    is_initialized = true;
}

int fsm_start_module_run(){
    int ret = 0;
    if(is_initialized == false){
        LOG_INF("FSM not initialized, initializing");
        fsm_start_module_init();
    }
    ret = smf_run_state(SMF_CTX(&s_obj));
    if(ret){
        LOG_ERR("FSM returned non-zero code: %d", ret);
    }
    return ret;
}

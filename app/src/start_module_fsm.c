#include "start_module_fsm.h"
#include "rc5.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(start_module_fsm);

static bool is_initialized;

static int dohyo_address;
static const int start_command = 1;
static const int stop_command = 0;
static const int program_command_address = 0xb;
static const int start_stop_command_address = 0x7;
static const int kabot_custom_command_rc5_address = 0xC;
static const int kabot_reset_start_module_rc5_command = 0x0;


static const struct smf_state start_module_states[];

enum kabot_custom_command {
    RESET_START_MODULE = 0x00,
};

static const  kabot_custom_command_fn kabot_custom_commands[];


enum start_module_state{POWER_ON, PROGRAMMING, STARTED, STOPPED_SAFE, STOPPED};

struct s_object s_obj;


// custom commands:

void reset_start_module(void *o){
    smf_set_state(SMF_CTX(&s_obj), &start_module_states[POWER_ON]);
}





// module machine state commands:

void set_start(bool value){
    LOG_DBG("Setting start to: %d", value);
}

void set_kill(bool value){
    LOG_DBG("Setting kill to: %d", value);
}

void save_dohyo_address(int address){
    LOG_DBG("Setting dohyo address to 0x%x", address);
    dohyo_address = address;
}

bool is_start_command(int rc5_address, int rc5_command){
    if(rc5_address != start_stop_command_address){
        return false;
    }
    if(rc5_command == start_command + dohyo_address) {
        return true;
    }
    return false;
}

bool is_stop_command(int rc5_address, int rc5_command){
    if(rc5_address != start_stop_command_address){
        return false;
    }
    if(rc5_command == stop_command + dohyo_address) {
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

static void power_on_entry(void *o){
    LOG_DBG("");
    set_start(false);
    set_kill(true);
    // todo: set leds
}
static void power_on_exit(void *o){
    LOG_DBG("");
}
static void power_on_run(void *o){
    LOG_DBG("");
    if(is_start_command(s_obj.rc5_address, s_obj.rc5_command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[STARTED]);
    }
    if(is_stop_command(s_obj.rc5_address, s_obj.rc5_command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[STOPPED_SAFE]);
    }
    if(is_program_command(s_obj.rc5_address, s_obj.rc5_command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[PROGRAMMING]);
    }
}

static void programming_entry(void *o){
    LOG_DBG("");
    set_start(false);
    set_kill(true);
    save_dohyo_address(s_obj.rc5_command);
}
static void programming_exit(void *o){
    LOG_DBG("");
}
static void programming_run(void *o){
    LOG_DBG("");
    //todo: flash twice
    smf_set_state(SMF_CTX(&s_obj), &start_module_states[POWER_ON]);
}

static void started_entry(void *o){
    LOG_DBG("");
    set_start(true);
    set_kill(false);
}
static void started_exit(void *o){
    LOG_DBG("");
}
static void started_run(void *o){
    LOG_DBG("");
    if(is_stop_command(s_obj.rc5_address, s_obj.rc5_command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[STOPPED_SAFE]);
    }
    if(is_program_command(s_obj.rc5_address, s_obj.rc5_command)){
        smf_set_state(SMF_CTX(&s_obj), &start_module_states[PROGRAMMING]);
    }
}

static void stopped_safe_entry(void *o){
    LOG_DBG("");
    set_start(false);
    set_kill(true);
}
static void stopped_safe_exit(void *o){
    LOG_DBG("");
}
static void stopped_safe_run(void *o){
    LOG_DBG("");
    // delay 1000ms
    // set led flashing
    smf_set_state(SMF_CTX(&s_obj), &start_module_states[STOPPED]);
}

static void stopped_entry(void *o){
    LOG_DBG("");
    set_start(false);
    set_kill(true);
    // set led flashing
    // use command reset to go back into power_on
}
static void stopped_exit(void *o){
    LOG_DBG("");
}
static void stopped_run(void *o){
    LOG_DBG("");
    if(is_kabot_custom_command(s_obj.rc5_address, s_obj.rc5_command)){
        kabot_custom_commands[s_obj.rc5_command](o);
    }
}

static const struct smf_state start_module_states[] = {
    [POWER_ON] = SMF_CREATE_STATE(power_on_entry, power_on_run, power_on_exit),
    [PROGRAMMING] = SMF_CREATE_STATE(programming_entry, programming_run, programming_exit),
    [STARTED] = SMF_CREATE_STATE(started_entry, started_run, started_exit),
    [STOPPED_SAFE] = SMF_CREATE_STATE(stopped_safe_entry, stopped_safe_run, stopped_safe_exit),
    [STOPPED] = SMF_CREATE_STATE(stopped_entry, stopped_run, stopped_exit),
};

static const kabot_custom_command_fn kabot_custom_commands[] = {
    [RESET_START_MODULE] = reset_start_module,
};

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

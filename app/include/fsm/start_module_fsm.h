#pragma once
#include <zephyr/smf.h>
#include "fsm/states.h"
struct s_object {
    struct smf_ctx ctx;
    int rc5_address;
    int rc5_command;
    int new_dohyo_address;
    int current_dohyo_address;
    int current_selected_sensor;
};

enum start_module_state{
    POWER_ON, 
    PROGRAMMING, 
    STARTED, 
    STOPPED_SAFE, 
    STOPPED,
    SELECT_SENSOR,
    CALIBRATE_THRESHOLD,
    CALIBRATE_OFFSET,
    CALIBRATE_XTALK,
};

enum kabot_custom_command {

    KABOT_POWER = 0x0C,
    KABOT_UP = 0x10,
    KABOT_DOWN = 0x11,
    KABOT_LEFT = 0x15,
    KABOT_RIGHT = 0x16,
    KABOT_OK = 0x17,
    KABOT_EXIT = 0x12,
    KABOT_RED = 0x2B,
    KABOT_GREEN = 0x2C,
    KABOT_YELLOW = 0x2D,
    KABOT_BLUE = 0x2E,
    KABOT_MUTE = 0xD,
};
extern struct smf_state start_module_states[];

struct rc5_data fetch_rc5_data(struct s_object* object);

typedef void (*kabot_custom_command_fn)(void *obj);

extern struct s_object s_obj;
int fsm_start_module_run();


bool is_start_command(int rc5_address, int rc5_command);
bool is_stop_command(int rc5_address, int rc5_command);
bool is_program_command(int rc5_address, int rc5_command);
bool is_kabot_custom_command(int rc5_address, int rc5_command);

void set_start(bool value);
void set_kill(bool value);
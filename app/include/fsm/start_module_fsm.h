#pragma once
#include <zephyr/smf.h>
struct s_object {
    struct smf_ctx ctx;
    int rc5_address;
    int rc5_command;
};

typedef void (*kabot_custom_command_fn)(void *obj);

extern struct s_object s_obj;
int fsm_start_module_run();

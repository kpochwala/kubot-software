#include "start_module_fsm.h"
#include "rc5.h"

#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(start_module_fsm);

static bool is_initialized;
static const struct smf_state demo_states[];

enum demo_state{S0, S1, S2};

struct s_object {
    struct smf_ctx ctx;
    
}s_obj;

static void s0_entry(void *o){

}

static void s0_run(void *o){
    smf_set_state(SMF_CTX(&s_obj), &demo_states[S1]);
    LOG_DBG("s0 run");
}

static void s0_exit(void *o){
    
}

static void s1_entry(void *o){
    
}

static void s1_run(void *o){
    smf_set_state(SMF_CTX(&s_obj), &demo_states[S2]);
    LOG_DBG("s1 run");
}

static void s1_exit(void *o){
    
}

static void s2_entry(void *o){
    
}

static void s2_run(void *o){
    smf_set_state(SMF_CTX(&s_obj), &demo_states[S0]);
    LOG_DBG("s2 run");
}

static void s2_exit(void *o){
    
}

static const struct smf_state demo_states[] = {
    [S0] = SMF_CREATE_STATE(s0_entry, s0_run, s0_exit),
    [S1] = SMF_CREATE_STATE(s1_entry, s1_run, s1_exit),
    [S2] = SMF_CREATE_STATE(s2_entry, s2_run, s2_exit),
};

void fsm_start_module_init(){
    smf_set_initial(SMF_CTX(&s_obj), &demo_states[S0]);
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

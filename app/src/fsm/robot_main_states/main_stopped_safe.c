#include "fsm/start_module_fsm.h"
#include "motor/motor.h"

void main_stopped_safe(){
    set_motors(0,0);
}

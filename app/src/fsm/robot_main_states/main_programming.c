#include "fsm/start_module_fsm.h"
#include "led_strip_charlieplex.h"

void main_programming(){
    set_led_all(kabot_active);
    k_sleep(K_MSEC(100));
    set_led_all(kabot_inactive);
    k_sleep(K_MSEC(100));
    set_led_all(kabot_active);
    k_sleep(K_MSEC(100));
    set_led_all(kabot_inactive);
    k_sleep(K_MSEC(100));    
}

#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <stdio.h>


// #include "robot_sensors/sensor_thread.h"
// #include "robot_sensors/line_sensor.h"
// #include "led_strip_charlieplex.h"
#include "eeprom/eeprom_helper.h"

#include "motor/motor.h"
#include "rc5/rc5.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include "fsm/start_module_fsm.h"

#include "arm_math.h"
arm_pid_instance_f32 PID;

LOG_MODULE_REGISTER(app);


#define STACKSIZE KB(64)
#define PRIORITY 7

K_SEM_DEFINE(main_semaphore, 0, 1);


void main_thread(void){
    k_sleep(K_MSEC(1000));

    // initialize_kubot_eeprom();

    if(k_sem_take(&main_semaphore, K_MSEC(20000)) != 0){
        LOG_ERR("Unable to acquire main semaphore!");
    } // wait for up to 20s for initialization

    motors_init();
    rc5_init();

    set_motors(0.0, 0.0);


    while(1){
        k_sleep(K_MSEC(100));

        int command;
        if(rc5_new_command_received(&command)){
            rc5_reset();
            LOG_DBG("RC5 received");
            LOG_DBG("Start bits:   0x%x", rc5_get_start_bits(command));
            LOG_DBG("Toggle bits:  0x%x",rc5_get_toggle_bit(command));
            LOG_DBG("Address bits: 0x%x",rc5_get_address_bits(command));
            LOG_DBG("Command bits: 0x%x",rc5_get_command_bits(command));

            s_obj.rc5_address = rc5_get_address_bits(command);
            s_obj.rc5_command = rc5_get_command_bits(command);
        }

        fsm_start_module_run();

    }
}


K_THREAD_DEFINE(main_thread_id, STACKSIZE, main_thread, NULL, NULL, NULL, PRIORITY+5, 0, 0);





// #ifdef __cplusplus
// }
// #endif

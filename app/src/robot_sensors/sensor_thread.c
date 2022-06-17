#include "robot_sensors/sensor_thread.h"
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#define DISTANCE_THRESHOLD (CONFIG_VL53L0X_PROXIMITY_THRESHOLD)
#define REFRESH_TIME_MS 20

LOG_MODULE_REGISTER(sensor_thread);
K_MUTEX_DEFINE(tof_measurements_mutex);

static const char* tof_labels[] = {"V0", "V1", "V2", "V3", "V4", "V5", "V6", "V7", "V8", "V9"};


static struct distance_measurement tof_measurements[ALL_SENSORS_NUMBER];

void fetch_tof(void){

    struct sensor_value value;
    struct distance_measurement* current_sensor_measurement;
    const struct device* tof_devices[ALL_SENSORS_NUMBER];

    for(int i = 0; i < ALL_SENSORS_NUMBER; i++) {
        tof_devices[i] = device_get_binding(tof_labels[i]);

        if(k_mutex_lock(&tof_measurements_mutex, K_MSEC(1000)) == 0) {        
            current_sensor_measurement = &tof_measurements[i];
            current_sensor_measurement->err = sensor_sample_fetch(tof_devices[i]);
            if(current_sensor_measurement->err){
                LOG_ERR("Error during initialization of sensor %s", tof_devices[i]->name);
            }
            k_mutex_unlock(&tof_measurements_mutex);
        }
    }
    
    int64_t fetch_start = k_uptime_get();
    int64_t fetch_total_time = 0;
    int64_t calculated_sleep = 0;

    while(1){
        
        fetch_total_time = k_uptime_get() - fetch_start;
        fetch_start = k_uptime_get(); // reset fetch timer

        calculated_sleep = REFRESH_TIME_MS - fetch_total_time;      
        
        if((calculated_sleep) < 1){ // make sure we yield at least for 1 ms for other tasks
            k_sleep(K_MSEC(1)) ;
        }else{
            k_sleep(K_MSEC(calculated_sleep)); // sleep for time that is REFRESH_TIME_MS - measurement fetch time
        }

        for(int i = 0; i < ALL_SENSORS_NUMBER; i++) {

            if(k_mutex_lock(&tof_measurements_mutex, K_MSEC(100)) == 0) {
                
                current_sensor_measurement = &tof_measurements[i];
    
                current_sensor_measurement->err = sensor_sample_fetch(tof_devices[i]);
                if(current_sensor_measurement->err){
                    LOG_ERR("Error fetching value from sensor %s", tof_devices[i]->name);
                    goto unlock;
                }
                
                current_sensor_measurement->err = sensor_channel_get(tof_devices[i], SENSOR_CHAN_DISTANCE, &value);
                if(current_sensor_measurement->err){
                    LOG_ERR("Error getting value from measurement for %s", tof_devices[i]->name);
                    goto unlock;
                }
    
                current_sensor_measurement->distance_mm = sensor_value_to_double(&value) * 1000;
                current_sensor_measurement->in_range = current_sensor_measurement->distance_mm < DISTANCE_THRESHOLD;

                unlock:
                k_mutex_unlock(&tof_measurements_mutex);
            }
        }
    }
}


struct distance_measurement copy[ALL_SENSORS_NUMBER];
struct distance_measurement* get_tof(){
    if(k_mutex_lock(&tof_measurements_mutex, K_MSEC(100)) == 0){
        memcpy(copy, tof_measurements, sizeof(copy));
        k_mutex_unlock(&tof_measurements_mutex);
    }
    return copy;
}

#define STACK_SIZE KB(16)
#define PRIORITY 7

K_THREAD_DEFINE(fetch_tof_id, STACK_SIZE, fetch_tof, NULL, NULL, NULL, PRIORITY, 0, 0);

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/drivers/sensor.h>

#define ALL_SENSORS_NUMBER 10

struct distance_measurement {
    bool in_range;
    unsigned int distance_mm;
    int err;
};

struct distance_measurement* get_tof();
extern struct k_sem tof_semaphore; 

//todo: this is uglyyyyyy hack
int vl53l0x_extra_calibrate_xtalk(const struct device *dev, int distance_mm, uint32_t *xtalk_output);
int vl53l0x_extra_save_xtalk(const struct device *dev, uint32_t xtalk_data);
int vl53l0x_extra_calibrate_offset(const struct device *dev, int distance_mm, int32_t *offset_micrometer);
int vl53l0x_extra_save_offset(const struct device *dev, int32_t offset_micrometer);
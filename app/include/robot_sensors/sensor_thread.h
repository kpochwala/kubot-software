#pragma once
#include <stdbool.h>

#define ALL_SENSORS_NUMBER 10

extern struct k_mutex tof_measurements_mutex;

struct distance_measurement {
    bool in_range;
    unsigned int distance_mm;
    int err;
};

struct distance_measurement* get_tof();

// extern struct distance_measurement tof_measurements[];
#pragma once
#include <stdbool.h>

#define ALL_SENSORS_NUMBER 10

extern struct k_mutex tof_measurements_mutex;

struct distance_measurement {
    bool in_range;
    unsigned int distance_mm;
    int err;
};

extern struct distance_measurement tof_measurements[];
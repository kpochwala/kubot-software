#pragma once
#include <stdbool.h>

#define ALL_SENSORS_NUMBER 10

struct distance_measurement {
    bool in_range;
    unsigned int distance_mm;
    int err;
};

struct distance_measurement* get_tof();
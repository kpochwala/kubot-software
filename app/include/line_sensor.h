#pragma once
#include <stdbool.h>

#define ALL_LINE_SENSOR_NUMBER 4

struct line_measurement {
    bool white_line_detected;
    int threshold;
    int err;
};

extern struct line_measurement line_measurements[];
extern struct k_mutex line_measurements_mutex;

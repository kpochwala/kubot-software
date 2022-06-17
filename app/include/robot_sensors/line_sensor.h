#pragma once
#include <stdbool.h>

#define ALL_LINE_SENSOR_NUMBER 4

struct line_measurement {
    bool white_line_detected;
    int threshold;
    int raw;
    int err;
};

struct line_measurement* get_line();

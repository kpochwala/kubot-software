#pragma once

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define LINE_SENSOR_COUNT 4
#define TOF_SENSOR_COUNT 10
#define SENSOR_NAME_LENGTH 16

#define __eeprom__ __attribute__((packed, aligned(16)))

struct __eeprom__ start_module_eeprom {
    int saved_state;
    int dohyo_address;
};

struct __eeprom__ line_sensor_eeprom {
    char sensor_name[SENSOR_NAME_LENGTH];
    int32_t wants_initialization;
    int32_t enabled;
    int32_t threshold;
};

struct __eeprom__ tof_sensor_eeprom {
    char sensor_name[SENSOR_NAME_LENGTH];
    bool wants_initialization;
    bool enabled;
    uint32_t xtalk_compensation_megacps;
    int32_t offset_micrometer;
};

struct __eeprom__ line_sensors_eeprom {
    uint32_t sensor_count;
    struct line_sensor_eeprom sensor[LINE_SENSOR_COUNT];
};

struct __eeprom__ tof_sensors_eeprom {
    uint32_t sensor_count;
    struct tof_sensor_eeprom sensor[TOF_SENSOR_COUNT];
};

struct __eeprom__ eeprom_configuration {
    bool is_eeprom_initialized;
    uint32_t magic_number_of_initialization;
};

struct __eeprom__ eeprom_view {
    struct eeprom_configuration config;
    struct start_module_eeprom start_module;
    struct tof_sensors_eeprom tof;
    struct line_sensors_eeprom line;
};
#pragma once

void power_on_entry(void *o);
void power_on_run(void *o);
void power_on_exit(void *o);

void programming_entry(void *o);
void programming_run(void *o);
void programming_exit(void *o);

void started_entry(void *o);
void started_run(void *o);
void started_exit(void *o);

void stopped_safe_entry(void *o);
void stopped_safe_run(void *o);
void stopped_safe_exit(void *o);

void stopped_entry(void *o);
void stopped_run(void *o);
void stopped_exit(void *o);

void select_sensor_entry(void *o);
void select_sensor_run(void *o);
void select_sensor_exit(void *o);

void calibrate_threshold_entry(void *o);
void calibrate_threshold_run(void *o);
void calibrate_threshold_exit(void *o);

void calibrate_offset_entry(void *o);
void calibrate_offset_run(void *o);
void calibrate_offset_exit(void *o);

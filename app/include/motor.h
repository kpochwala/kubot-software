#pragma once

extern const float speed_multiplier;

struct vec3f {
    float x;
    float y;
    float z;
};

int motors_init();
int set_motors(float speed_m1, float speed_m2);
int set_motors_vec(struct vec3f* translation, struct vec3f* rotation);

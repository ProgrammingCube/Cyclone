#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

// Simple 3D vector, used everywhere
typedef struct {
    float x, y, z;
} vec3;

// Simple 4D vector, used for quaternions
typedef struct {
    float x, y, z, w;
} vec4;

// Enum for the four gravity directions
typedef enum {
    GRAVITY_DOWN,
    GRAVITY_UP,
    GRAVITY_LEFT,
    GRAVITY_RIGHT
} GravityDirection;

#endif // TYPES_H
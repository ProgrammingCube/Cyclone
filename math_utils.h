#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "types.h"
#include <math.h>

// Represents a Quaternion for 3D rotations
typedef vec4 quat;

// --- Quaternion Functions ---

static void quat_to_mat4(float* m, quat q) {
    float x2 = q.x * q.x;
    float y2 = q.y * q.y;
    float z2 = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    m[0] = 1.0f - 2.0f * (y2 + z2);
    m[1] = 2.0f * (xy + wz);
    m[2] = 2.0f * (xz - wy);
    m[3] = 0.0f;

    m[4] = 2.0f * (xy - wz);
    m[5] = 1.0f - 2.0f * (x2 + z2);
    m[6] = 2.0f * (yz + wx);
    m[7] = 0.0f;

    m[8] = 2.0f * (xz + wy);
    m[9] = 2.0f * (yz - wx);
    m[10] = 1.0f - 2.0f * (x2 + y2);
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

// Creates a quaternion from an axis and an angle (in radians)
static quat quat_from_axis_angle(vec3 axis, float angle) {
    quat q;
    float half_angle = angle / 2.0f;
    float s = sin(half_angle);
    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;
    q.w = cos(half_angle);
    return q;
}

// Multiplies two quaternions
static quat quat_multiply(quat q1, quat q2) {
    quat result;
    result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    return result;
}

// Rotates a 3D vector by a quaternion
static vec3 quat_rotate_vec3(quat q, vec3 v) {
    vec3 u = {q.x, q.y, q.z};
    float s = q.w;
    vec3 cross_uv;
    cross_uv.x = u.y * v.z - u.z * v.y;
    cross_uv.y = u.z * v.x - u.x * v.z;
    cross_uv.z = u.x * v.y - u.y * v.x;
    
    vec3 cross_u_cross_uv;
    cross_u_cross_uv.x = u.y * cross_uv.z - u.z * cross_uv.y;
    cross_u_cross_uv.y = u.z * cross_uv.x - u.x * cross_uv.z;
    cross_u_cross_uv.z = u.x * cross_uv.y - u.y * cross_uv.x;

    vec3 result;
    result.x = v.x + 2.0f * (s * cross_uv.x + cross_u_cross_uv.x);
    result.y = v.y + 2.0f * (s * cross_uv.y + cross_u_cross_uv.y);
    result.z = v.z + 2.0f * (s * cross_uv.z + cross_u_cross_uv.z);
    return result;
}

// Spherical Linear Interpolation between two quaternions
static quat quat_slerp(quat q1, quat q2, float t) {
    float dot = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

    if (dot < 0.0f) {
        q2.x = -q2.x; q2.y = -q2.y; q2.z = -q2.z; q2.w = -q2.w;
        dot = -dot;
    }

    if (dot > 0.9995f) { // If quaternions are very close, use linear interpolation
        quat result = {
            q1.x + t * (q2.x - q1.x),
            q1.y + t * (q2.y - q1.y),
            q1.z + t * (q2.z - q1.z),
            q1.w + t * (q2.w - q1.w)
        };
        float len = sqrt(result.x*result.x + result.y*result.y + result.z*result.z + result.w*result.w);
        result.x /= len; result.y /= len; result.z /= len; result.w /= len;
        return result;
    }

    float theta_0 = acos(dot);
    float theta = theta_0 * t;
    float sin_theta = sin(theta);
    float sin_theta_0 = sin(theta_0);

    float s1 = cos(theta) - dot * sin_theta / sin_theta_0;
    float s2 = sin_theta / sin_theta_0;

    quat result = {
        (s1 * q1.x) + (s2 * q2.x),
        (s1 * q1.y) + (s2 * q2.y),
        (s1 * q1.z) + (s2 * q2.z),
        (s1 * q1.w) + (s2 * q2.w)
    };
    return result;
}

#endif // MATH_UTILS_H
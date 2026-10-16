#pragma once

#include "types.hpp"

mat4 mat4_identity(void);
quat quat_identity(void);
quat quat_from_axis_angle(float axis_x, float axis_y, float axis_z,
                          float angle_radians);
mat4 quat_to_mat4(quat q);

mat4 mat4_multiply(const mat4 &a, const mat4 &b);
mat4 mat4_translation(float x, float y, float z);

// Quaternion math
quat quat_conjugate(quat q);
quat quat_multiply(quat q1, quat q2);

// Projection Matrices
mat4 mat4_perspective_opengl(float fov_y, float aspect, float z_near,
                             float z_far);
mat4 mat4_perspective_vulkan(float fov_y, float aspect, float z_near,
                             float z_far);

quat quat_normalize(quat q);

void quat_rotate_vec3(quat q, const float in[3], float out[3]);
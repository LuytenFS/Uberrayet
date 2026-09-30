#pragma once

#include "types.hpp"

mat4 mat4_identity(void);
quat quat_identity(void);
quat quat_from_axis_angle(float axis_x, float axis_y, float axis_z,
                          float angle_radians);
mat4 quat_to_mat4(quat q);

// Quaternion math
quat quat_conjugate(quat q);
quat quat_multiply(quat q1, quat q2);

// Projection Matrices
mat4 mat4_perspective_opengl(float fov_y, float aspect, float z_near,
                             float z_far);
mat4 mat4_perspective_vulkan(float fov_y, float aspect, float z_near,
                             float z_far);
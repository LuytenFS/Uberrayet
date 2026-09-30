#include "umath.hpp"
#include <cmath>

mat4 mat4_identity(void) {
  mat4 result = {{{0}}};
  result.m[0][0] = 1.0f;
  result.m[1][1] = 1.0f;
  result.m[2][2] = 1.0f;
  result.m[3][3] = 1.0f;
  return result;
}

quat quat_identity(void) {
  return (quat){.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f};
}

quat quat_from_axis_angle(float axis_x, float axis_y, float axis_z,
                          float angle_radians) {
  float half_angle = angle_radians * 0.5f;
  float sin_half = sinf(half_angle);

  quat q;
  q.x = axis_x * sin_half;
  q.y = axis_y * sin_half;
  q.z = axis_z * sin_half;
  q.w = cosf(half_angle);
  return q;
}

quat quat_conjugate(quat q) { return quat{-q.x, -q.y, -q.z, q.w}; }

quat quat_multiply(quat q1, quat q2) {
  return quat{q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
              q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
              q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
              q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z};
}

mat4 quat_to_mat4(quat q) {
  mat4 r = mat4_identity();

  float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
  float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
  float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

  r.m[0][0] = 1.0f - 2.0f * (yy + zz);
  r.m[0][1] = 2.0f * (xy + wz);
  r.m[0][2] = 2.0f * (xz - wy);

  r.m[1][0] = 2.0f * (xy - wz);
  r.m[1][1] = 1.0f - 2.0f * (xx + zz);
  r.m[1][2] = 2.0f * (yz + wx);

  r.m[2][0] = 2.0f * (xz + wy);
  r.m[2][1] = 2.0f * (yz - wx);
  r.m[2][2] = 1.0f - 2.0f * (xx + yy);

  return r;
}

// OpenGL Perspective Matrix (Z range: [-1, 1], Y-axis: Up)
mat4 mat4_perspective_opengl(float fov_y, float aspect, float z_near,
                             float z_far) {
  mat4 result = {{{0}}};
  float tan_half_fov = tanf(fov_y * 0.5f);

  result.m[0][0] = 1.0f / (aspect * tan_half_fov);
  result.m[1][1] = 1.0f / tan_half_fov;
  result.m[2][2] = -(z_far + z_near) / (z_far - z_near);
  result.m[2][3] = -1.0f;
  result.m[3][2] = -(2.0f * z_far * z_near) / (z_far - z_near);

  return result;
}

// Vulkan Perspective Matrix (Z range: [0, 1], Y-axis: Down)
mat4 mat4_perspective_vulkan(float fov_y, float aspect, float z_near,
                             float z_far) {
  mat4 result = {{{0}}};
  float tan_half_fov = tanf(fov_y * 0.5f);

  result.m[0][0] = 1.0f / (aspect * tan_half_fov);
  result.m[1][1] =
      -1.0f / tan_half_fov; // Inverted Y for Vulkan coordinate system
  result.m[2][2] = z_far / (z_near - z_far);
  result.m[2][3] = -1.0f;
  result.m[3][2] = (z_near * z_far) / (z_near - z_far);

  return result;
}
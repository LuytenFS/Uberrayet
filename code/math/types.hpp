#pragma once

#include <cmath>

// Three Dimensional Vector
typedef struct {
  union {
    struct {
      float x, y, z;
    };
    float v[3];
  };
} vec_3d;

// Two Dimensional Vector
typedef struct {
  union {
    struct {
      float x, y;
    };
    float v[2];
  };
} vec_2d;

// 3x3 Matrix: Standard for 2D transformations or 3D rotations/normals
typedef struct {
  union {
    float m[3][3];
    float data[9];
  };
} mat3;

// 4x4 Matrix: Standard for 3D transformations (Scale, Rotate, Translate,
// Projection)
typedef struct {
  union {
    float m[4][4];
    float data[16];
  };
} mat4;

// Quaternion
typedef struct {
  float x;
  float y;
  float z;
  float w; // Scalar / real part
} quat;

mat4 mat4_identity(void);

// Returns an identity quaternion (no rotation)
quat quat_identity(void);

// Create a quaternion representing a rotation around a normalized 3D axis
// vector
quat quat_from_axis_angle(float axis_x, float axis_y, float axis_z,
                          float angle_radians);

// Convert a quaternion into a 4x4 matrix for rendering on the GPU
mat4 quat_to_mat4(quat q);
#pragma once

#include "math/types.hpp"

class Camera {
public:
  Camera(float fov_degrees, float aspect_ratio, float near_plane,
         float far_plane);

  // Movement & Orientation Controls
  void SetPosition(double x, double y, double z) {
    m_world_x = x;
    m_world_y = y;
    m_world_z = z;
  }

  void Rotate(quat delta_rotation);
  void MoveLocal(double right, double up, double forward);
  void SetAspect(float aspect);
  // Matrix Operations
  mat4 GetViewMatrix() const;

  // Projection Matrix (Vulkan)
  mat4 GetProjectionMatrixVulkan() const;

  // Projection Matrix (OpenGL)
  mat4 GetProjectionMatrixOpenGL() const;

  // Getters for Floating Origin calculation
  void GetPosition(double &x, double &y, double &z) const;
private:
  // 64-bit precision position for space scale
  double m_world_x;
  double m_world_y;
  double m_world_z;

  // 6DOF Orientation
  quat m_orientation;

  // Projection / Frustum properties
  float m_fov_y;
  float m_aspect_ratio;
  float m_near_plane;
  float m_far_plane;
};
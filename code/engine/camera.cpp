#include "camera.hpp"
#include "math/umath.hpp"

Camera::Camera(float fov_degrees, float aspect_ratio, float near_plane,
               float far_plane)
    : m_world_x(0.0), m_world_y(0.0), m_world_z(0.0),
      m_orientation(quat_identity()),
      m_fov_y(fov_degrees * 0.0174532925f), // Convert to radians
      m_aspect_ratio(aspect_ratio), m_near_plane(near_plane),
      m_far_plane(far_plane) {}

void Camera::GetPosition(double &x, double &y, double &z) const{
  x = m_world_x;
  y = m_world_y;
  z = m_world_z;
}

mat4 Camera::GetProjectionMatrixOpenGL() const {
  return mat4_perspective_opengl(m_fov_y, m_aspect_ratio, m_near_plane,
                                 m_far_plane);
}

mat4 Camera::GetProjectionMatrixVulkan() const {
  return mat4_perspective_vulkan(m_fov_y, m_aspect_ratio, m_near_plane,
                                 m_far_plane);
}

mat4 Camera::GetViewMatrix() const {
  // Conjugate quaternion represents inverse rotation for camera view
  quat inv_orient = quat_conjugate(m_orientation);
  return quat_to_mat4(inv_orient);
}

void Camera::Rotate(quat delta_rotation) {
  // Multiply current orientation by delta rotation for 6DOF movement
  m_orientation = quat_multiply(m_orientation, delta_rotation);
}

void Camera::MoveLocal(double right, double up, double forward) {
  // -Z is forward (OpenGL convention)
  const float local[3] = {static_cast<float>(right), static_cast<float>(up),
                          static_cast<float>(-forward)};
  float world[3];
  quat_rotate_vec3(m_orientation, local, world);

  m_world_x += world[0];
  m_world_y += world[1];
  m_world_z += world[2];
}

void Camera::SetAspect(float a) { m_aspect_ratio = a; }
#version 450
#extension GL_ARB_separate_shader_objects : enable
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform UniformBufferObject {
    vec3 observation_point;
    float r_max;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vCartesianPosition; // origin al position
layout(location = 1) out vec4 vSphericalPosition;
layout(location = 2) out vec3 vColor;

vec4 project(vec3 position) {
  float r, phi, theta;
  vec3 d = position - ubo.observation_point;

  r = length(d);
  theta = atan(position.y, position.x);
  phi = (r == 0) ?  0 : acos(position.z / r);

  float gamma = 1.0 / M_PI;
  return vec4(theta * gamma, (2*phi - M_PI)*gamma, r / ubo.r_max, 1);
}

void main() {
  // Position
  vSphericalPosition = project(inPosition);
  gl_Position = vSphericalPosition;

  // Color
  vColor = vec3(vSphericalPosition[2], vSphericalPosition[2], vSphericalPosition[2]);

  // Original position
  vCartesianPosition = inPosition;
}

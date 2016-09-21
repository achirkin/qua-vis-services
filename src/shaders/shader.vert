#version 450
#extension GL_ARB_separate_shader_objects : enable
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform UniformBufferObject {
    vec3 observation_point;
    float r_max;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  float r, phi, theta;
  vec3 d = inPosition - ubo.observation_point;

  r = length(d);
  theta = atan(inPosition.y, inPosition.x);
  phi = r == 0 ?  0 : acos(inPosition.z / r);

  float gamma = ubo.r_max / M_PI;
  gl_Position = vec4(theta * gamma, (2*phi - M_PI)*gamma, r, ubo.r_max);
  float rc = r/ubo.r_max;
  fragColor = vec3(rc, rc, rc);
}

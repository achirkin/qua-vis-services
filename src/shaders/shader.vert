#version 450

layout(push_constant) uniform PushConsts {
  vec3 observation_point;
  float r_max;
  float alpha_max;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vCartesianPosition;
layout(location = 1) out vec3 vColor;

void main() {
  // Compute vector from observer to vertex
  vCartesianPosition = inPosition - ubo.observation_point;
  vColor = inColor;
}

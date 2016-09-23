#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    vec3 observation_point;
    float r_max;
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

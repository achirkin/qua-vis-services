#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform UniformBufferObject {
  mat4 projection;
  mat4 view;
  mat4 model;
  vec3 observation_point;
  float r_max;
  float alpha_max;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec4 vCartesianPosition;
layout(location = 1) out vec3 vColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
  vCartesianPosition = vec4(inPosition - ubo.observation_point, 1.0);
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
  vColor = vec3(length(inPosition - ubo.observation_point)/ubo.r_max, 0, 0);
}

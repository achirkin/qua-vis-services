#version 450
#extension GL_ARB_tessellation_shader : enable
#define ID gl_InvocationID

layout(binding = 0) uniform UniformBufferObject {
  vec3 observation_point;
  float r_max;
  float alpha_max;
} ubo;

layout(location = 0) in vec3 vCartesianPosition[];
layout(location = 1) in vec3 vColor[];

layout (vertices = 3) out;
layout(location = 0) out vec3 tcCartesianPosition[];
layout(location = 1) out vec3 tcColor[];

void main()
{
  tcCartesianPosition[ID] = vCartesianPosition[ID];
  tcColor[ID] = vColor[ID];

  float l0 = length(vCartesianPosition[0]),
        l1 = length(vCartesianPosition[1]),
        l2 = length(vCartesianPosition[2]),
        l01 = max(1.0f , acos( dot(vCartesianPosition[0],vCartesianPosition[1]) / (l0 * l1) ) / ubo.alpha_max),
        l02 = max(1.0f , acos( dot(vCartesianPosition[0],vCartesianPosition[2]) / (l0 * l2) ) / ubo.alpha_max),
        l12 = max(1.0f , acos( dot(vCartesianPosition[1],vCartesianPosition[2]) / (l1 * l2) ) / ubo.alpha_max);

  gl_TessLevelInner[0] = max(l01, max(l02,l12));
  gl_TessLevelOuter[0] = l12;
  gl_TessLevelOuter[1] = l02;
  gl_TessLevelOuter[2] = l01;
}

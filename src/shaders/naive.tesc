#version 450
#extension GL_ARB_tessellation_shader : enable
#define ID gl_InvocationID

layout(push_constant) uniform PushConsts {
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
  gl_TessLevelInner[0] = 1;
  gl_TessLevelOuter[0] = 1;
  gl_TessLevelOuter[1] = 1;
  gl_TessLevelOuter[2] = 1;
}

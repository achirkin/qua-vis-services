#version 450
#extension GL_ARB_tessellation_shader : enable
#define ID gl_InvocationID

layout(push_constant) uniform UniformBufferObject {
  vec3 observation_point;
  float r_max;
  float alpha_max;
  mat4 projection;
  mat4 view;
  mat4 model;
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

  // compute angles between vectors
  float l[3];
  l[0] = length(vCartesianPosition[0]);
  l[1] = length(vCartesianPosition[1]);
  l[2] = length(vCartesianPosition[2]);

  vec3 c[3];
  c[0] = cross(vCartesianPosition[0],vCartesianPosition[1]);
  c[1] = cross(vCartesianPosition[1],vCartesianPosition[2]);
  c[2] = cross(vCartesianPosition[2],vCartesianPosition[0]);

  float d[3];
  d[0] = length(c[0]);
  d[1] = length(c[1]);
  d[2] = length(c[2]);

  float alpha[3];
  alpha[0] = asin(d[0] / l[0] / l[1]);
  alpha[1] = asin(d[1] / l[1] / l[2]);
  alpha[2] = asin(d[2] / l[2] / l[0]);

  // tessellation level computed using maximum-angle heuristic
  int tl[4];
  tl[0] = max(1, int(ceil(alpha[0] / ubo.alpha_max)));
  tl[1] = max(1, int(ceil(alpha[1] / ubo.alpha_max)));
  tl[2] = max(1, int(ceil(alpha[2] / ubo.alpha_max)));
  tl[3] = max(1, max(max(tl[0], max(tl[1], tl[2]))/2, min(tl[0], min(tl[1], tl[2]))-1));

  gl_TessLevelInner[0] = tl[3];

  gl_TessLevelOuter[2] = tl[0];
  gl_TessLevelOuter[0] = tl[1];
  gl_TessLevelOuter[1] = tl[2];
}

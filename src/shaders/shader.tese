#version 450
#extension GL_ARB_tessellation_shader : enable

layout(triangles) in;
layout(location = 0) in vec3 tcCartesianPosition[];
layout(location = 1) in vec3 tcColor[];

layout(location = 0) out vec3 teCartesianPosition;
layout(location = 1) out vec3 teColor;

void main()
{
  // interpolated cartesian position
  vec3 q0 = gl_TessCoord.x * tcCartesianPosition[0];
  vec3 q1 = gl_TessCoord.y * tcCartesianPosition[1];
  vec3 q2 = gl_TessCoord.z * tcCartesianPosition[2];
  teCartesianPosition = q0 + q1 + q2;

  // interpolated color
  vec3 c0 = gl_TessCoord.x * tcColor[0];
  vec3 c1 = gl_TessCoord.y * tcColor[1];
  vec3 c2 = gl_TessCoord.z * tcColor[2];
  teColor = c0 + c1 + c2;
}

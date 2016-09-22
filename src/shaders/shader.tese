#version 450
#extension GL_ARB_tessellation_shader : enable

layout(triangles) in;
layout(location = 0) in vec4 tcPosition[];
layout(location = 1) in vec3 tcColor[];

layout(location = 0) out vec4 tePosition;
layout(location = 1) out vec3 teColor;
layout(location = 2) out vec3 tePatchDistance;

void main()
{
  tePatchDistance = gl_TessCoord;

  vec4 p0 = gl_TessCoord.x * tcPosition[0];
  vec4 p1 = gl_TessCoord.y * tcPosition[1];
  vec4 p2 = gl_TessCoord.z * tcPosition[2];
  tePosition = p0 + p1 + p2;

  vec3 c0 = gl_TessCoord.x * tcColor[0];
  vec3 c1 = gl_TessCoord.y * tcColor[1];
  vec3 c2 = gl_TessCoord.z * tcColor[2];
  teColor = c0 + c1 + c2;

  gl_Position = tePosition;
}

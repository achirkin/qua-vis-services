#version 450
#extension GL_ARB_tessellation_shader : enable

layout(triangles, equal_spacing, cw) in;
in vec3 tcPosition[];
in vec3 tcColor[];

out vec3 tePosition;
out vec3 tePatchDistance;
out vec3 teColor;

void main()
{
  tePatchDistance = gl_TessCoord;

  vec3 p0 = gl_TessCoord.x * tcPosition[0];
  vec3 p1 = gl_TessCoord.y * tcPosition[1];
  vec3 p2 = gl_TessCoord.z * tcPosition[2];
  tePosition = p0 + p1 + p2;

  vec3 c0 = gl_TessCoord.x * tcColor[0];
  vec3 c1 = gl_TessCoord.y * tcColor[1];
  vec3 c2 = gl_TessCoord.z * tcColor[2];
  teColor = c0 + c1 + c2;

  gl_Position = vec4(tePosition, 1);
}

#version 450
#extension GL_ARB_tessellation_shader : enable

layout(triangles) in;
layout(location = 0) in vec3 tcCartesianPosition[];
layout(location = 1) in vec3 tcColor[];

layout(location = 0) out vec3 teCartesianPosition;
layout(location = 1) out vec3 teColor;

void main()
{
  teCartesianPosition = gl_TessCoord.x*tcCartesianPosition[0] + gl_TessCoord.y*tcCartesianPosition[1] + gl_TessCoord.z*tcCartesianPosition[2];
  teColor = gl_TessCoord.x*tcColor[0] + gl_TessCoord.y*tcColor[1] + gl_TessCoord.z*tcColor[2];
}

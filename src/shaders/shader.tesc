#version 450
#extension GL_ARB_tessellation_shader : enable
#define ID gl_InvocationID
#define M_PI 3.1415926535897932384626433832795

layout(location = 0) in vec3 vCartesianPosition[];
layout(location = 1) in vec4 vSphericalPPosition[];
layout(location = 2) in vec3 vColor[];

layout (vertices = 3) out;
layout(location = 0) out vec3 tcCartesianPosition[];
layout(location = 1) out vec4 tcSphericalPPosition[];
layout(location = 2) out vec3 tcColor[];

void main()
{
  tcCartesianPosition[ID] = vCartesianPosition[ID];
  tcSphericalPPosition[ID] = vSphericalPPosition[ID];
  tcColor[ID] = vColor[ID];

  if (ID == 0) {
    gl_TessLevelInner[0] = 1;

    gl_TessLevelOuter[0] = 1;
    gl_TessLevelOuter[1] = 1;
    gl_TessLevelOuter[2] = 1;
  }
}

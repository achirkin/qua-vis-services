#version 450
#extension GL_ARB_tessellation_shader : enable
#define M_PI 3.1415926535897932384626433832795

layout(location = 0) in vec4 vPosition[];
layout(location = 1) in vec3 vColor[];

layout (vertices = 3) out;
layout(location = 0) out vec4 tcPosition[];
layout(location = 1) out vec3 tcColor[];

#define ID gl_InvocationID

void main()
{
  tcPosition[ID] = vPosition[ID];
  tcColor[ID] = vColor[ID];

  if (ID == 0) {
    float dt[3];
    dt[0] = abs(vPosition[0][0] - vPosition[1][0]);
    dt[1] = abs(vPosition[1][0] - vPosition[2][0]);
    dt[2] = abs(vPosition[2][0] - vPosition[0][0]);

    gl_TessLevelInner[0] = 1;

    gl_TessLevelOuter[0] = (dt[0] > M_PI) ? 2 : 1;
    gl_TessLevelOuter[1] = (dt[1] > M_PI) ? 2 : 1;
    gl_TessLevelOuter[2] = (dt[2] > M_PI) ? 2 : 1;
  }
}

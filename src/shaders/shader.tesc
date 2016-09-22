#version 450
#extension GL_ARB_tessellation_shader : enable
#define ID gl_InvocationID
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform UniformBufferObject {
    vec3 observation_point;
    float r_max;
} ubo;

layout(location = 0) in vec3 voPosition[];
layout(location = 1) in vec4 vPosition[];
layout(location = 2) in vec3 vColor[];

layout (vertices = 3) out;
layout(location = 0) out vec3 tcoPosition[];
layout(location = 1) out vec4 tcPosition[];
layout(location = 2) out vec3 tcColor[];

void main()
{
  tcoPosition[ID] = voPosition[ID];
  tcPosition[ID] = vPosition[ID];
  tcColor[ID] = vColor[ID];

  if (ID == 0) {
    float gamma = ubo.r_max / M_PI;
    float dt[3];
    dt[0] = abs(vPosition[0][0] - vPosition[1][0]) / gamma;
    dt[1] = abs(vPosition[1][0] - vPosition[2][0]) / gamma;
    dt[2] = abs(vPosition[2][0] - vPosition[0][0]) / gamma;

    gl_TessLevelInner[0] = 1;

    gl_TessLevelOuter[0] = (dt[0] > M_PI) ? 2 : 1;
    gl_TessLevelOuter[1] = (dt[1] > M_PI) ? 2 : 1;
    gl_TessLevelOuter[2] = (dt[2] > M_PI) ? 2 : 1;
  }
}

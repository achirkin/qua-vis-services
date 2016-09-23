#version 450
#extension GL_ARB_tessellation_shader : enable
#define ID gl_InvocationID
#define M_PI 3.1415926535897932384626433832795

layout(location = 0) in vec3 vCartesianPosition[];
layout(location = 1) in vec4 vSphericalPosition[];
layout(location = 2) in vec3 vColor[];

layout (vertices = 3) out;
layout(location = 0) out vec3 tcCartesianPosition[];
layout(location = 1) out vec4 tcSphericalPosition[];
layout(location = 2) out vec3 tcColor[];

void main()
{
  tcCartesianPosition[ID] = vCartesianPosition[ID];
  tcColor[ID] = vColor[ID];

  bool xa, xb, xc;
  xa = abs(vSphericalPosition[1][0] - vSphericalPosition[0][0]) > 1;
  xb = abs(vSphericalPosition[2][0] - vSphericalPosition[1][0]) > 1;
  xc = abs(vSphericalPosition[0][0] - vSphericalPosition[2][0]) > 1;

  // correct vertex positions with broken adjacent edges
  int multiplier[3];
  multiplier[0] = 2 * int((xa || xc) && (vSphericalPosition[0][0] < 0));
  multiplier[1] = 2 * int((xa || xb) && (vSphericalPosition[1][0] < 0));
  multiplier[2] = 2 * int((xb || xc) && (vSphericalPosition[2][0] < 0));

  tcSphericalPosition[ID] = vSphericalPosition[ID];
  tcSphericalPosition[ID][0] += multiplier[ID];

  if (ID == 0) {
    gl_TessLevelInner[0] = 1;

    gl_TessLevelOuter[0] = 1;
    gl_TessLevelOuter[1] = 1;
    gl_TessLevelOuter[2] = 1;
  }
}

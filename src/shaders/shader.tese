#version 450
#extension GL_ARB_tessellation_shader : enable
#define M_PI 3.1415926535897932384626433832795
#define ID gl_InvocationID

layout(binding = 0) uniform UniformBufferObject {
    vec3 observation_point;
    float r_max;
} ubo;

layout(triangles) in;
layout(location = 0) in vec3 tcoPosition[]; // original position
layout(location = 1) in vec4 tcPosition[];
layout(location = 2) in vec3 tcColor[];

layout(location = 0) out vec4 tePosition;
layout(location = 1) out vec3 teColor;

vec4 project(vec3 position) {
  float r, phi, theta;
  vec3 d = position - ubo.observation_point;

  r = length(d);
  theta = atan(position.y, position.x);
  phi = (r == 0) ?  0 : acos(position.z / r);

  float gamma = ubo.r_max / M_PI;
  return vec4(theta * gamma, (2*phi - M_PI)*gamma, r, ubo.r_max);
}

vec4 interpolate() {
  // Check whether vertex is an original triangle corner
  if (gl_TessCoord == vec3(1,0,0)) {
    return tcPosition[0];
  }
  if (gl_TessCoord == vec3(0,1,0)) {
    return tcPosition[1];
  }
  if (gl_TessCoord == vec3(0,0,1)) {
    return tcPosition[2];
  }
  else {
    // TODO
    vec4 p0 = gl_TessCoord.x * tcPosition[0];
    vec4 p1 = gl_TessCoord.y * tcPosition[1];
    vec4 p2 = gl_TessCoord.z * tcPosition[2];
    return p0 + p1 + p2;
  }
}

void main()
{
  gl_Position = interpolate();

  // color
  vec3 c0 = gl_TessCoord.x * tcColor[0];
  vec3 c1 = gl_TessCoord.y * tcColor[1];
  vec3 c2 = gl_TessCoord.z * tcColor[2];
  teColor = c0 + c1 + c2;
}

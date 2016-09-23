#version 450
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform UniformBufferObject {
    vec3 observation_point;
    float r_max;
} ubo;

layout(triangles) in;
layout(location = 0) in vec3 teCartesianPosition[3];
layout(location = 1) in vec3 teColor[3];

layout(triangle_strip, max_vertices = 6) out;
layout(location = 0) out vec3 gCartesianPosition;
layout(location = 1) out vec4 gSphericalPosition;
layout(location = 2) out vec3 gColor;

vec4 project(vec3 position) {
  float r, phi, theta;

  r = length(position);
  theta = atan(position.y, position.x);
  phi = (r == 0) ?  0 : acos(position.z / r);

  float gamma = 1.0 / M_PI;
  return vec4(theta * gamma, (2*phi - M_PI)*gamma, r / ubo.r_max, 1);
}

void EmitShiftedTriangle(vec4 coordinates[3]) {
  gCartesianPosition = teCartesianPosition[0];
  gSphericalPosition = coordinates[0];
  gColor = teColor[0];
  gl_Position = gSphericalPosition;
  EmitVertex();

  gCartesianPosition = teCartesianPosition[1];
  gSphericalPosition = coordinates[1];
  gColor = teColor[1];
  gl_Position = gSphericalPosition;
  EmitVertex();

  gCartesianPosition = teCartesianPosition[2];
  gSphericalPosition = coordinates[2];
  gColor = teColor[2];
  gl_Position = gSphericalPosition;
  EmitVertex();

  EndPrimitive();
}


void main() {
  // transform to spherical coordinates
  vec4 sphericalPosition[3];
  sphericalPosition[0] = project(teCartesianPosition[0]);
  sphericalPosition[1] = project(teCartesianPosition[1]);
  sphericalPosition[2] = project(teCartesianPosition[2]);

  bool xa, xb, xc;
  xa = abs(sphericalPosition[1][0] - sphericalPosition[0][0]) > 1;
  xb = abs(sphericalPosition[2][0] - sphericalPosition[1][0]) > 1;
  xc = abs(sphericalPosition[0][0] - sphericalPosition[2][0]) > 1;

  // correct vertex positions with broken adjacent edges
  bool multiplier[3];
  multiplier[0] = (xa || xc) && (sphericalPosition[0][0] < 0);
  multiplier[1] = (xa || xb) && (sphericalPosition[1][0] < 0);
  multiplier[2] = (xb || xc) && (sphericalPosition[2][0] < 0);

  // shift to the right
  sphericalPosition[0][0] += 2 * int(multiplier[0]);
  sphericalPosition[1][0] += 2 * int(multiplier[1]);
  sphericalPosition[2][0] += 2 * int(multiplier[2]);
  EmitShiftedTriangle(sphericalPosition);

  // shift to the left
  if (multiplier[0] || multiplier[1] || multiplier[2]) {
    sphericalPosition[0][0] -= 2;
    sphericalPosition[1][0] -= 2;
    sphericalPosition[2][0] -= 2;
    EmitShiftedTriangle(sphericalPosition);
  }
}

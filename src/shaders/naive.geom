#version 450
#define PI 3.14159265358979311599796346854419
#define INV_PI 0.31830988618379069121644420192752

layout(push_constant) uniform PushConsts {
  vec3 observation_point;
  float r_max;
  float alpha_max;
} ubo;

layout(triangles) in;
layout(location = 0) in vec3 teCartesianPosition[3];
layout(location = 1) in vec3 teNormal[3];

layout(triangle_strip, max_vertices = 10) out;
layout(location = 0) out vec3 gCartesianPosition;
layout(location = 1) out vec4 gSphericalPosition;
layout(location = 2) out vec3 gNormal;

vec4 project(vec3 position) {
  // the original position uses x (front to back), y (left to right), z (bottom to top)
  // and the coordinates are transformed into (phi, theta, r) where
  // phi is in the angle in the x-y-plane
  float r, theta, phi;

  // compute spherical coordinatess
  r = length(position);
  phi = atan(position.y, position.x);
  theta = (r == 0) ?  0 : acos(position.z / r);

  // correct for viewport constraints such that
  // - x,y in [-1, 1]
  // - the distance to the observer (r) is capped at r_max
  //   (0 <= r <= r_max (<=>) 0 <= z <= 1)
  return vec4(phi * INV_PI, 2 * theta * INV_PI - 1, r / ubo.r_max, 1);
}

void EmitSphericalVertex(vec3 cartesian, vec4 spherical, vec3 normal) {
  gCartesianPosition = cartesian;
  gSphericalPosition = spherical;
  gNormal = normal;
  gl_Position = gSphericalPosition;
  EmitVertex();
}

void main() {
  // transform to spherical coordinates
  vec4 sphericalPosition[3];
  sphericalPosition[0] = project(teCartesianPosition[0]);
  sphericalPosition[1] = project(teCartesianPosition[1]);
  sphericalPosition[2] = project(teCartesianPosition[2]);

  EmitSphericalVertex(teCartesianPosition[0], sphericalPosition[0], teNormal[0]);
  EmitSphericalVertex(teCartesianPosition[1], sphericalPosition[1], teNormal[1]);
  EmitSphericalVertex(teCartesianPosition[2], sphericalPosition[2], teNormal[2]);
  EndPrimitive();
}

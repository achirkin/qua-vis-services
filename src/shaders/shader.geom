#version 450
#define PI 3.14159265358979311599796346854419
#define INV_PI 0.31830988618379069121644420192752

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
  // the original position uses x (front to back), y (left to right), z (bottom to top)
  // and the coordinates are transformed into (theta, phi, r) where
  // theta is in the angle in the x-y-plane
  float r, phi, theta;

  // compute spherical coordinatess
  r = length(position);
  theta = atan(position.y, position.x);
  phi = (r == 0) ?  0 : acos(position.z / r);

  // correct for viewport constraints such that
  // - x,y in [-1, 1]
  // - the distance to the observer (r) is capped at r_max
  //   (0 <= r <= r_max (<=>) 0 <= z <= 1)
  return vec4(theta * INV_PI, 2 * phi * INV_PI - 1, r / ubo.r_max, 1);
}

void EmitSphericalVertex(vec3 cartesian, vec4 spherical, vec3 color) {
  gCartesianPosition = cartesian;
  gSphericalPosition = spherical;
  gColor = color;
  gl_Position = gSphericalPosition;

  EmitVertex();
}

void EmitSphericalTriangle(vec4 coordinates[3]) {
  EmitSphericalVertex(teCartesianPosition[0], coordinates[0], teColor[0]);
  EmitSphericalVertex(teCartesianPosition[1], coordinates[1], teColor[1]);
  EmitSphericalVertex(teCartesianPosition[2], coordinates[2], teColor[2]);

  EndPrimitive();
}


void main() {
  // transform to spherical coordinates
  vec4 sphericalPosition[3];
  sphericalPosition[0] = project(teCartesianPosition[0]);
  sphericalPosition[1] = project(teCartesianPosition[1]);
  sphericalPosition[2] = project(teCartesianPosition[2]);

  // check for each edge whether its broken. An edge is broken
  // if its endpoints lie in different halfs of the azimuth-axis
  bool a_broken, b_broken, c_broken;
  a_broken = abs(sphericalPosition[1][0] - sphericalPosition[0][0]) > 1;
  b_broken = abs(sphericalPosition[2][0] - sphericalPosition[1][0]) > 1;
  c_broken = abs(sphericalPosition[0][0] - sphericalPosition[2][0]) > 1;

  // check for each vertex if it has an incident broken edge and whether
  // it's on the left side
  bool has_broken_edge[3];
  has_broken_edge[0] = (sphericalPosition[0][0] < 0) && (a_broken || c_broken);
  has_broken_edge[1] = (sphericalPosition[1][0] < 0) && (a_broken || b_broken);
  has_broken_edge[2] = (sphericalPosition[2][0] < 0) && (b_broken || c_broken);

  // shift to the right:
  // If a vertex is on the left side
  // of the image and it has at least one adjacent edge that goes to the
  // right side of the image, then push this vertex to the right (outside of the image)
  // this creates the left side of the triangle on the right image border
  sphericalPosition[0][0] += 2 * int(has_broken_edge[0]);
  sphericalPosition[1][0] += 2 * int(has_broken_edge[1]);
  sphericalPosition[2][0] += 2 * int(has_broken_edge[2]);
  EmitSphericalTriangle(sphericalPosition);

  // shift to the left
  // If one vertex has been shifted, then the right side of the triangle
  // is currently missing. Therefore, we're copying the whole triangle to the
  // image's left side where the already emitted part of the triangle
  // is not within the image
  if (has_broken_edge[0] || has_broken_edge[1] || has_broken_edge[2]) {
    sphericalPosition[0][0] -= 2;
    sphericalPosition[1][0] -= 2;
    sphericalPosition[2][0] -= 2;
    EmitSphericalTriangle(sphericalPosition);
  }
}

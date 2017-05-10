#version 450
#define PI 3.14159265358979311599796346854419
#define INV_PI 0.31830988618379069121644420192752

layout(binding = 0) uniform UniformBufferObject {
  vec3 observation_point;
  float r_max;
  float alpha_max;
} ubo;

layout(triangles) in;
layout(location = 0) in vec3 teCartesianPosition[3];
layout(location = 1) in vec3 teColor[3];

layout(triangle_strip, max_vertices = 18) out;
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
  a_broken = abs(sphericalPosition[1][0] - sphericalPosition[0][0]) >= 1;
  b_broken = abs(sphericalPosition[2][0] - sphericalPosition[1][0]) >= 1;
  c_broken = abs(sphericalPosition[0][0] - sphericalPosition[2][0]) >= 1;

  // used to distinguish between two degenerate cases
  int sum_broken = int(a_broken) + int(b_broken) + int(c_broken);

  if (sum_broken == 0) {
    EmitSphericalTriangle(sphericalPosition);
  }
  if (sum_broken > 0) {
    // check for each vertex if it has an incident broken edge
    bool has_broken_edge[3];
    has_broken_edge[0] = (a_broken || c_broken);
    has_broken_edge[1] = (a_broken || b_broken);
    has_broken_edge[2] = (b_broken || c_broken);

    // check for each vertex if its on the left side
    bool is_left_sided[3];
    is_left_sided[0] = (sphericalPosition[0][0] < 0);
    is_left_sided[1] = (sphericalPosition[1][0] < 0);
    is_left_sided[2] = (sphericalPosition[2][0] < 0);

    // determine which vertex is non-broken, which is broken left and which is broken right
    int unbroken_index = int(!has_broken_edge[1]) * 1 + int(!has_broken_edge[2]) * 2; // b
    int broken_left_index = int(has_broken_edge[1] && is_left_sided[1]) * 1 + int(has_broken_edge[2] && is_left_sided[2]) * 2; // a
    int broken_right_index = 3 - unbroken_index - broken_left_index;

    if (sum_broken == 1) {
      // determine intersection between triangle and z-axis in cartesian coordinates
      vec2 column1 = vec2(
        teCartesianPosition[broken_left_index].y - teCartesianPosition[broken_right_index].y,
        teCartesianPosition[unbroken_index].y - teCartesianPosition[broken_left_index].y
      );
      vec2 column2 = vec2(
        teCartesianPosition[broken_right_index].x - teCartesianPosition[broken_left_index].x,
        teCartesianPosition[broken_left_index].x - teCartesianPosition[unbroken_index].x
      );
      mat2 M = mat2(column1, column2);
      vec2 st = 1.0/determinant(M) * M * vec2(teCartesianPosition[broken_left_index].x, teCartesianPosition[broken_left_index].y);
      float zq = (1 - st.s - st.t) * teCartesianPosition[broken_left_index].z + st.s * teCartesianPosition[unbroken_index].z + st.t * teCartesianPosition[broken_right_index].z;
      zq /= ubo.r_max; // normalize

      // compute 3 points on pole
      vec4 qmpi = vec4(-1, -1, zq, 1);
      vec4 qb = vec4(sphericalPosition[unbroken_index][1], -1, zq, 1);
      vec4 qpi = vec4(1, -1, zq, 1);

      vec4 sphericalPositionNew[3];
      sphericalPositionNew[0] = sphericalPosition[broken_left_index]; // a
      sphericalPositionNew[1] = sphericalPosition[unbroken_index]; // b
      sphericalPositionNew[2] = sphericalPosition[broken_right_index]; // c

      // emit 4 additional triangles
      sphericalPositionNew[2] = qb; //a,b,qb
      EmitSphericalTriangle(sphericalPositionNew);

      sphericalPositionNew[0] = sphericalPosition[broken_right_index]; //c,b,qb
      EmitSphericalTriangle(sphericalPositionNew);

      sphericalPositionNew[1] = qpi; //c,qpi,qb
      EmitSphericalTriangle(sphericalPositionNew);

      sphericalPositionNew[1] = qmpi; // c,qmpi,qb
      sphericalPositionNew[2] = sphericalPosition[broken_left_index]; //c,qb,a
      EmitSphericalTriangle(sphericalPositionNew);

      //qpi,c,a
      sphericalPositionNew[0] = qpi;
      sphericalPositionNew[1] = sphericalPosition[broken_right_index];
      sphericalPositionNew[2] = sphericalPosition[broken_left_index];

      // right side
      sphericalPositionNew[2][0] += 2; // qpi,c,a'
      EmitSphericalTriangle(sphericalPositionNew);

      // left side
      // qmpi, c', a
      sphericalPositionNew[0][0] -= 2;
      sphericalPositionNew[1][0] -= 2;
      sphericalPositionNew[2][0] -= 2;
      EmitSphericalTriangle(sphericalPositionNew);
    }
    else {
      sphericalPosition[0][0] += 2 * int(has_broken_edge[0] && is_left_sided[0]);
      sphericalPosition[1][0] += 2 * int(has_broken_edge[1] && is_left_sided[1]);
      sphericalPosition[2][0] += 2 * int(has_broken_edge[2] && is_left_sided[2]);
      EmitSphericalTriangle(sphericalPosition);

      sphericalPosition[0][0] -= 2;
      sphericalPosition[1][0] -= 2;
      sphericalPosition[2][0] -= 2;
      EmitSphericalTriangle(sphericalPosition);
    }
  }
}

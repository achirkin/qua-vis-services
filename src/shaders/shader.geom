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

  // check for each edge whether it's broken. An edge is broken
  // if its endpoints lie in different halfs of the azimuth-axis
  // used to distinguish between two degenerate cases
  int sum_broken = int(abs(sphericalPosition[0][0] - sphericalPosition[1][0]) >= 1)
                 + int(abs(sphericalPosition[0][0] - sphericalPosition[2][0]) >= 1)
                 + int(abs(sphericalPosition[1][0] - sphericalPosition[2][0]) >= 1);
  
  if (sum_broken == 0) { // k == 0; regular triangle
    EmitSphericalVertex(teCartesianPosition[0], sphericalPosition[0], teNormal[0]);
    EmitSphericalVertex(teCartesianPosition[1], sphericalPosition[1], teNormal[1]);
    EmitSphericalVertex(teCartesianPosition[2], sphericalPosition[2], teNormal[2]);
    EndPrimitive();
  } else { // we have a bad triangle, let's order vertices so that phi_a > phi_b > phi_c
    int index_a = 0, index_b = 1, index_c = 2;
    switch ( int(sphericalPosition[0][0] > sphericalPosition[1][0])
           | int(sphericalPosition[0][0] > sphericalPosition[2][0])<<1
           | int(sphericalPosition[1][0] > sphericalPosition[2][0])<<2 ) {
      case 7: // 0 > 1 > 2
                                               break;
      case 6: // 1 >= 0 > 2
        index_a = 1; index_b = 0;              break;
      case 4: // 1 > 2 >= 0
        index_a = 1; index_b = 2; index_c = 0; break;
      case 3: // 0 > 2 >= 1
                     index_b = 2; index_c = 1; break;
      case 1: // 2 >= 0 > 1
        index_a = 2; index_b = 0; index_c = 1; break;
      case 0: // 2 >= 1 >= 0
        index_a = 2;              index_c = 0; break;
      // all other variants are impossible
    }
    
    // Now, get the coordinates
    vec3 p_a = teCartesianPosition[index_a],
         p_b = teCartesianPosition[index_b],
         p_c = teCartesianPosition[index_c];

    // spherical coordinates
    vec4 s_a = sphericalPosition[index_a],
         s_b = sphericalPosition[index_b],
         s_c = sphericalPosition[index_c],
         s_cx = s_c + vec4(2,0,0,0), // imaginary c'
         s_ax = s_a - vec4(2,0,0,0); // imaginary a'
    
    if (sum_broken == 2) { // the triangle crosses the hind meridian
      
      // we need to duplicate b in spherical coorindates either on the left or on the right.
      vec4 s_bp = s_b  + vec4(int(s_b[0] < 0)*2,0,0,0),
           s_bm = s_bp - vec4(2,0,0,0);
           
      EmitSphericalVertex(p_a, s_ax, teNormal[index_a]); // a'
      EmitSphericalVertex(p_c, s_c,  teNormal[index_c]); // c
      EmitSphericalVertex(p_b, s_bm, teNormal[index_b]); // b or b'
      EndPrimitive();           
      EmitSphericalVertex(p_a, s_a,  teNormal[index_a]); // a
      EmitSphericalVertex(p_c, s_cx, teNormal[index_c]); // c'
      EmitSphericalVertex(p_b, s_bp, teNormal[index_b]); // b or b'
      EndPrimitive();
      
    } else { // sum_broken == 1: the triangle hovers one of the poles
    
      // determine intersection between triangle and z-axis in cartesian coordinates
      vec2 column1 = vec2( p_a.y - p_c.y
                         , p_b.y - p_a.y ),
           column2 = vec2( p_c.x - p_a.x
                         , p_a.x - p_b.x );
      mat2 M = mat2(column1, column2);
      vec2 st = 1.0/determinant(M) * M * vec2(p_a.x, p_a.y);
      float z_pole = (1 - st.s - st.t) * p_a.z + st.s * p_b.z + st.t * p_c.z;

      // Certesian position and normal for an added pole point
      vec3 p_pole = vec3(0,0,z_pole),
           n_pole = (1 - st.s - st.t) * teNormal[index_a] + st.s * teNormal[index_b] + st.t * teNormal[index_c];

      // Three spherical positions for the pole point
      vec4 s_polep = vec4( 1, -sign(z_pole), z_pole/ubo.r_max, 1),
           s_polem = vec4(-1,     s_polep[1], s_polep[2], s_polep[3]),
           s_poleb = vec4(s_b[0], s_polep[1], s_polep[2], s_polep[3]);
      
      // Finally, emit two triangle strips
      EmitSphericalVertex(p_c   , s_cx   , teNormal[index_c]); // c'
      EmitSphericalVertex(p_pole, s_polep, n_pole           ); // q_pi
      EmitSphericalVertex(p_a   , s_a    , teNormal[index_a]); // a
      EmitSphericalVertex(p_pole, s_poleb, n_pole           ); // q_b
      EmitSphericalVertex(p_b   , s_b    , teNormal[index_b]); // b
      EndPrimitive();
      
      EmitSphericalVertex(p_a   , s_ax   , teNormal[index_a]); // a'
      EmitSphericalVertex(p_pole, s_polem, n_pole           ); // q_-pi
      EmitSphericalVertex(p_c   , s_c    , teNormal[index_c]); // c
      EmitSphericalVertex(p_pole, s_poleb, n_pole           ); // q_b
      EmitSphericalVertex(p_b   , s_b    , teNormal[index_b]); // b
      EndPrimitive();
    }
  }
}

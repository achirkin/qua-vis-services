#ifndef GEOJSON_HPP
#define GEOJSON_HPP

#define EPS 0.00001

#include <float.h> // FLT_MIN
#include <math.h> // sqrt

#include <vector>
#include <numeric> // iota()
#include <algorithm> // sort()
#include <iostream> // TODO: Remove

namespace geojson {
  float abs(float x) {
    return x > 0 ? x : -x;
  }


  /**
   * 2D vector functions
   */
  typedef struct vec2 {
    float x;
    float y;

    vec2 operator*(const float scalar) {
      return {x*scalar, y*scalar};
    }

    vec2 operator/(const float scalar) {
      return {x/scalar, y/scalar};
    }

    vec2 operator-(const vec2 other) {
      return {x-other.x, y-other.y};
    }

    vec2 operator+(const vec2 other) {
      return {x+other.x, y+other.y};
    }

    float operator*(const vec2 other) {
      return x*other.x + y*other.y;
    }

    bool operator==(const vec2 other) {
      return x==other.x && y==other.y;
    }

    operator std::string() const {
      return "{" + std::to_string(x) + "," + std::to_string(y) + "}";
    }
  } vec2;

  float abs(vec2 p) {
    return sqrt(p*p);
  }

  float ccw(vec2 p1, vec2 p2, vec2 p3) {
    return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x);
  }

  /**
   * 3D Vector functions
   */
  typedef struct vec3 {
    float x;
    float y;
    float z;

    vec3 operator*(const float scalar) {
      return {x*scalar, y*scalar, z*scalar};
    }

    vec3 operator/(const float scalar) {
      return {x/scalar, y/scalar, z/scalar};
    }

    vec3 operator-(const vec3 other) {
      return {x-other.x, y-other.y, z-other.z};
    }

    vec3 operator+(const vec3 other) {
      return {x+other.x, y+other.y, z+other.z};
    }

    float operator*(const vec3 other) {
      return x*other.x + y*other.y + z*other.z;
    }

    bool operator==(const vec3 other) {
      return abs(x-other.x) < EPS && abs(y-other.y) < EPS && abs(z-other.z) < EPS;
    }

    operator std::string() const {
      return "{" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + "}";
    }
  } vec3;

  float abs(vec3 p) {
    return sqrt(p*p);
  }

  /**
   * Inverted base change using orthogonal basis of Subvectorspace spanned by polygon
   */
  std::vector<vec3> to3d(std::vector<vec2> points, std::vector<vec3> basis, vec3 origin) {
    std::vector<vec3> points3d (points.size());
    for (size_t i = 0; i < points.size(); i++) {
      // multiply by transposed (=inverted) base change matrix
      points3d[i] = origin + basis[0]*points[i].x + basis[1]*points[i].y;
    }

    return points3d;
  }

  /**
   * Base change using orthogonal basis of Subvectorspace spanned by polygon
   */
  std::vector<vec2> to2d(std::vector<vec3> points, std::vector<vec3> basis, vec3 origin) {
    std::vector<vec2> points2d(points.size());
    for (size_t i = 0; i < points.size(); i++) {
      // multiply by base change matrix (row-wise orthogonal vectors)
      vec3 v = points[i] - origin;
      points2d[i] = {basis[0]*v, basis[1]*v};
    }

    return points2d;
  }

  /**
   * Gram-Schmidt orthogonalization, returns the orthogonal basis of the 2d-subspace
   */
  std::vector<vec3> gramschmidt(std::vector<vec3> points) {
    std::vector<vec3> b(2);
    vec3 v1 = points[2] - points[1];
    b[0] = points[1] - points[0];
    b[1] = v1 - b[0]*((v1*b[0])/(b[0]*b[0]));

    b[0] = b[0] / abs(b[0]);
    b[1] = b[1] / abs(b[1]);

    return b;
  }

  bool intriangle2d(vec2 p, vec2 t1, vec2 t2, vec2 t3) {
    float ccw1, ccw2, ccw3;
    ccw1 = ccw(p, t1, t2);
    ccw2 = ccw(p, t2, t3);
    ccw3 = ccw(p, t3, t1);

    bool b1, b2, b3;
    b1 = ccw1 < 0;
    b2 = ccw2 < 0;
    b3 = ccw3 < 0;

    return ((b1 == b2) && (b2 == b3));
  }

  /**
   * Augments the polygon with edges to create a suitible structure to use the
   * ear-clipping algorithm
   */
  std::vector<vec2> augment(std::vector<vec2> points) {
    // divide the point sequence into one outer polygon and multiple inner polygons (holes)
    std::vector<vec2> outer_polygon = {};
    std::vector<std::vector<vec2>> inner_polygons = {};
    size_t polygon_index = 0;
    for (size_t i = 1; i < points.size(); i++) {
      if (points[i] == points[polygon_index]) {
        if (polygon_index == 0) {
          outer_polygon = std::vector<vec2> {points.begin()+polygon_index, points.begin()+i};
        }
        else {
          inner_polygons.push_back(std::vector<vec2> {points.begin()+polygon_index, points.begin()+i});
        }
        polygon_index = ++i;
      }
    }

    // for each inner polygon, sort its components by x and rotate it such
    // that the rightmost vertex is at the first place
    std::vector<size_t> rightmost_vertex(inner_polygons.size());
    for (size_t i = 0; i < inner_polygons.size(); i++) {

      std::cout << "Inner Ring " << i << std::endl;
      for (size_t j = 0; j < inner_polygons[i].size(); j++) {
        std::cout << std::string(inner_polygons[i][j]) << std::endl;
      }

      size_t max_index = 0;
      float max_x = FLT_MIN;
      for (size_t j = 0; j < inner_polygons[i].size(); j++) {
        if (inner_polygons[i][j].x > max_x) {
          max_x = inner_polygons[i][j].x;
          max_index = j;
        }
      }

      std::cout << "Maximum Index " << max_index << std::endl;

      std::rotate(
        inner_polygons[i].begin(),
        inner_polygons[i].begin()+max_index,
        inner_polygons[i].end()
      );


      std::cout << "Inner Ring Rotated " << i << std::endl;
      for (size_t j = 0; j < inner_polygons[i].size(); j++) {
        std::cout << std::string(inner_polygons[i][j]) << std::endl;
      }
    }
    // sort the inner polygons by their maximum x value (descending)
    std::vector<int> inner_order(inner_polygons.size());
    std::iota(inner_order.begin(), inner_order.end(), 0);
    std::sort(
      inner_order.begin(),
      inner_order.end(),
      [&inner_polygons](size_t i, size_t j) -> bool {
        return inner_polygons[i][0].x > inner_polygons[j][0].x;
      }
    );

    // combine outer polygon P0 with first hole H0 to P1
    // compine P1 with second hole H1 to P2
    // continue until all holes are contained in P
    std::vector<vec2> augmented = outer_polygon;
    for (int i : inner_order) {
      vec2 inner_point = inner_polygons[i][0];

      // find the closest point on the outer polygon
      // that is to the right of the inner point
      // TODO: This can be buggy for certain cases (e.g. a spiral)
      size_t closest_index = 0;
      float min_distance = FLT_MAX;
      for (size_t j = 0; j < augmented.size(); j++) {
        if (augmented[j].x < inner_point.x) continue;

        float distance = abs(inner_point - augmented[j]);
        std::cout << std::string(augmented[j]) << " : " << distance << std::endl;
        if (distance < min_distance) {
          min_distance = distance;
          closest_index = j;
        }
      }

      std::cout << "Rightmost Inner point:" << std::string(inner_point) << std::endl;
      std::cout << "Closest Outer point:" << std::string(augmented[closest_index]) << std::endl;

      // augment the polygon
      augmented.insert(augmented.begin()+closest_index+1, inner_polygons[i].begin(), inner_polygons[i].end());
      augmented.insert(augmented.begin()+closest_index+1+inner_polygons[i].size(), inner_polygons[i][0]);
      augmented.insert(augmented.begin()+closest_index+1+inner_polygons[i].size()+1, augmented[closest_index]);

      std::cout << "Augmented:" << std::endl;
      for (size_t j = 0; j < augmented.size(); j++) {
        std::cout << std::string(augmented[j]) << std::endl;
      }
    }

    return augmented;
  }

  std::vector<vec2> triangulate2d(std::vector<vec2> points) {
    // TODO: Assumes a simple polygon
    std::vector<vec2> triangles = {};
    std::vector<vec2> unmarked { points.begin(), points.end() };

    vec2 p0, p1, p2;
    int index1 = 0;
    int index2 = 1;
    int index3 = 2;
    while (unmarked.size() > 3) {
      p0 = unmarked[index1];
      p1 = unmarked[index2];
      p2 = unmarked[index3];

      bool found_removable_ear = false;
      // check if we found a counter-clockwise triangle
      if (ccw(p0, p1, p2) > 0) {
        found_removable_ear = true;
        // check if triangle is an ear
        for (vec2 pi : unmarked) {
          if (pi == p0 || pi == p1 || pi == p2) continue;

          if (intriangle2d(pi, p0, p1, p2)) {
            found_removable_ear = false;
            break;
          }
        }
      }

      // If it was an ear, we clip it
      if (found_removable_ear) {
        triangles.push_back(p0);
        triangles.push_back(p1);
        triangles.push_back(p2);

        // Now remove the 2nd vertex of this ear and adjust the indices
        unmarked.erase(unmarked.begin() + index2);
        index1 %= unmarked.size();
        index2 %= unmarked.size();
        index3 %= unmarked.size();
      }
      else {
        index1 = index2;
        index2 = index3;
        index3 = (index3 +1 )% unmarked.size();
      }
    }
    triangles.insert(triangles.end(), unmarked.begin(), unmarked.end());

    return triangles;
  }

  std::vector<vec3> triangulate(std::vector<vec3> points) {
    if (points.size() < 3) return points;

    std::vector<vec3> basis = gramschmidt(points);
    std::vector<vec2> points2d = to2d(points, basis, points[0]);
    std::vector<vec2> augmented = augment(points2d);
    std::vector<vec2> triangles2d = triangulate2d(augmented);
    std::vector<vec3> triangles3d = to3d(triangles2d, basis, points[0]);

    return triangles3d;
  }
}

#endif //GEOJSON_HPP

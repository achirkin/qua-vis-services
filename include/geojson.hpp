#ifndef GEOJSON_HPP
#define GEOJSON_HPP

#define EPS 0.00001

#include <vector>
#include <iostream>
#include <math.h>

namespace geojson {
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
      return x==other.x && y==other.y && z == other.z;
    }

    operator std::string() const {
      return "{" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + "}";
    }
  } vec3;

  float abs(vec3 p) {
    return sqrt(p*p);
  }

  float abs(float x) {
    return x > 0 ? x : -x;
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
      // check if we found a counter-clockwise triangles
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

      if (found_removable_ear) {
        // We found a triangle
        triangles.push_back(p0);
        triangles.push_back(p1);
        triangles.push_back(p2);

        // Now remove p1 and adjust the indices
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
    std::vector<vec2> triangles2d = triangulate2d(points2d);
    std::vector<vec3> triangles3d = to3d(triangles2d, basis, points[0]);

    return triangles3d;
  }
}

#endif //GEOJSON_HPP

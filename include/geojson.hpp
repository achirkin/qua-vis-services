#ifndef GEOJSON_HPP
#define GEOJSON_HPP

#include <vector>
#include <iostream>
#include <math.h>

namespace geojson {
  typedef struct vec2 {
    float x;
    float y;

    vec2 operator-(const vec2 other) {
      return {x-other.x, y-other.y};
    }

    vec2 operator+(const vec2 other) {
      return {x+other.x, y+other.y};
    }

    float operator*(const vec2 other) {
      return x*other.x + y*other.y;
    }

    vec2 operator*(const float scalar) {
      return {x*scalar, y*scalar};
    }

    vec2 operator/(const float scalar) {
      return {x/scalar, y/scalar};
    }

    operator std::string() const {
      return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    }
  } vec2;

  float abs(vec2 p) {
    return sqrt(p*p);
  }

  typedef struct vec3 {
    float x;
    float y;
    float z;

    vec3 operator-(const vec3 other) {
      return {x-other.x, y-other.y, z-other.z};
    }

    vec3 operator+(const vec3 other) {
      return {x+other.x, y+other.y, z+other.z};
    }

    float operator*(const vec3 other) {
      return x*other.x + y*other.y + z*other.z;
    }

    vec3 operator*(const float scalar) {
      return {x*scalar, y*scalar, z*scalar};
    }

    vec3 operator/(const float scalar) {
      return {x/scalar, y/scalar, z/scalar};
    }

    operator std::string() const {
      return "(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ")";
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

  std::vector<vec3> triangulate(std::vector<vec3> points) {
    std::vector<vec3> basis = gramschmidt(points);
    std::vector<vec2> xy = to2d(points, basis, points[0]);
    std::vector<vec3> xyz = to3d(xy, basis, points[0]);

    std::cout << "Original:" << std::endl;
    for (geojson::vec3 p : points) {
      std::cout << std::string(p) << std::endl;
    };

    std::cout << "Orthogonal Basis:" << std::endl;
    for (geojson::vec3 p : basis) {
      std::cout << std::string(p) << std::endl;
    };

    std::cout << "2D:" << std::endl;
    for (geojson::vec2 p : xy) {
      std::cout << std::string(p) << std::endl;
    };

    std::cout << "3D:" << std::endl;
    for (geojson::vec3 p : xyz) {
      std::cout << std::string(p) << std::endl;
    };

    return points;
  }
}

#endif //GEOJSON_HPP

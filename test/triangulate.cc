#include "geojson.hpp"

#include <vector>

void test_square() {
  std::vector<geojson::vec3> polygon = {
    {0,0,0},
    {0,1,0},
    {1,1,0},
    {1,0,0}
  };

  std::vector<geojson::vec3> expected = {
    {0,0,0},
    {0,1,0},
    {1,1,0},
    {0,0,0},
    {1,1,0},
    {1,0,0}
  };

  std::vector<geojson::vec3> result = triangulate(polygon);
  for (size_t i = 0; i < result.size(); i++) {
    if(!(result[i] == expected[i])) throw;
  }
}

void test_polygon_no_holes() {
  std::vector<geojson::vec3> polygon = {
    {0.5,0,0},
    {0.5,-1,0},
    {1,0,0},
    {0.5,1,0},
    {-0.5,1,0},
    {-1,0,0},
    {-0.5,-1,0},
    {-0.5,0,0}
  };

  std::vector<geojson::vec3> expected = {
    {0.500000,0.000000,0.000000},
    {0.500000,-1.000000,0.000000},
    {1.000000,0.000000,0.000000},
    {0.500000,0.000000,0.000000},
    {1.000000,0.000000,0.000000},
    {0.500000,1.000000,0.000000},
    {0.500000,0.000000,0.000000},
    {0.500000,1.000000,0.000000},
    {-0.500000,1.000000,0.000000},
    {-1.000000,0.000000,0.000000},
    {-0.500000,-1.000000,0.000000},
    {-0.500000,0.000000,0.000000},
    {-0.500000,0.000000,0.000000},
    {0.500000,0.000000,0.000000},
    {-0.500000,1.000000,0.000000},
    {-0.500000,1.000000,0.000000},
    {-1.000000,0.000000,0.000000},
    {-0.500000,0.000000,0.000000}
  };

  std::vector<geojson::vec3> result = triangulate(polygon);
  for (size_t i = 0; i < result.size(); i++) {
    if(!(result[i] == expected[i])) throw;
  }
}

int main() {
  test_polygon_no_holes();
  test_square();
}

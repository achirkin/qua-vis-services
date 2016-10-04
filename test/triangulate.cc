#include "geojson.hpp"

#include <vector>

int main() {
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

  triangulate(polygon);
}

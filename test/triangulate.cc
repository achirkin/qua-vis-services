#include "geojson.hpp"

#include <vector>

int main() {
  std::vector<geojson::vec3> polygon = {
    {0,0,0},
    {0,6,1},
    {0,1,7}
  };

  triangulate(polygon);
}

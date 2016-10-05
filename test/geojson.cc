#include "quavis/vk/geometry/geometry.h"
#include "quavis/vk/geometry/geojson.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <cfloat>

namespace quavis {
  namespace geojson {
    void triangulate_file(std::string path) {
      std::ifstream fh (path);
      if (fh.is_open()) {
        std::string contents ((std::istreambuf_iterator<char>(fh)), std::istreambuf_iterator<char>());
        std::vector<vec3> vertices = geojson::parse(contents);
        std::vector<int> indices(vertices.size());
        std::iota(indices.begin(), indices.end(), 0);

        for (size_t i = 0; i < vertices.size(); i++) {
          std::cout << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
        }
        for (size_t i = 0; i < indices.size(); i+=3) {
          std::cout << "f " << indices[i]+1 << " " << indices[i+1]+1 << " " << indices[i+2]+1 << std::endl;
        }
      }
    }
  }
}

int main() {
  quavis::geojson::triangulate_file("mooctask.geojson");

}

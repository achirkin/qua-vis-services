#ifndef QUA_VIS_LOADER_H
#define QUA_VIS_LOADER_H

namespace quavis {
  class GeoJsonLoader {
  public:
    GeoJsonLoader(std::string path);

  private:
    std::string filepath;
    
  }
}
#endif

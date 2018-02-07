#ifndef _HITREGION_H_
#define _HITREGION_H_

#include "Path2D.h"
#include <string>

namespace canvas {
  class HitRegion {
  public:
    HitRegion() { }
    HitRegion(const std::string & _id, const Path2D & _path, const std::string & _cursor)
      : id(_id), cursor(_cursor), path(_path) { }

    const std::string & getId() const { return id; }
    const std::string & getCursor() const { return cursor; }
    const Path2D & getPath() const { return path; }
//    bool isInside(float x, float y) const { return path.isInside(x, y); }
    
  private:
    std::string id, cursor;
    Path2D path;
  };
};
#endif

#ifndef _CANVAS_POINT_H_
#define _CANVAS_POINT_H_

namespace canvas {
  class Point {
  public:
  Point() : x(0), y(0) { }
  Point(double _x, double _y) : x(_x), y(_y) { }
    double x, y;
  };
};

#endif

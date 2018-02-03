#ifndef PATH2D_H_
#define PATH2D_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <Context.h>
#include <Image.h>
#include <ImageData.h>
#include <Point.h>
#include <Path2D.h>

using namespace v8;
using namespace node;

class Path2D : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  void MoveTo(double x, double y);
  void LineTo(double x, double y);
  void ClosePath();
  void Arc(double x, double y, double radius, double startAngle, double endAngle, double anticlockwise);
  void ArcTo(double x1, double y1, double x2, double y2, double radius);
  void QuadraticCurveTo(double cpx, double cpy, double x, double y);
  void Clear();

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(MoveTo);
  static NAN_METHOD(LineTo);
  static NAN_METHOD(ClosePath);
  static NAN_METHOD(Arc);
  static NAN_METHOD(ArcTo);
  static NAN_METHOD(QuadraticCurveTo);
  static NAN_METHOD(Clear);

  Path2D();
  virtual ~Path2D();

private:
  canvas::Path2D *path2d;

  friend class CanvasRenderingContext2D;
};

#include "canvas.h"

#endif

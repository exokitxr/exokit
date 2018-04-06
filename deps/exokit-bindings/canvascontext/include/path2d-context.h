#ifndef _CANVASCONTEXT_PATH2D_H_
#define _CANVASCONTEXT_PATH2D_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/Image.h>
#include <canvas/include/ImageData.h>
#include <canvas/include/Point.h>
#include <canvas/include/Path2D.h>
#include <SkPath.h>

using namespace v8;
using namespace node;

class Path2D : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  void MoveTo(float x, float y);
  void LineTo(float x, float y);
  void ClosePath();
  void Arc(float x, float y, float radius, float startAngle, float endAngle, float anticlockwise);
  void ArcTo(float x1, float y1, float x2, float y2, float radius);
  void QuadraticCurveTo(float cpx, float cpy, float x, float y);
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
  SkPath path;

  friend class CanvasRenderingContext2D;
};

#include "canvas-context.h"

#endif

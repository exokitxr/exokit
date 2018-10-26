#ifndef _CANVASCONTEXT_CANVASGRADIENT_H_
#define _CANVASCONTEXT_CANVASGRADIENT_H_

#include <v8.h>
#include <nan.h>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/Image.h>
#include <canvas/include/ImageData.h>
#include <canvas/include/Point.h>
#include <canvas/include/Path2D.h>
#include <SkRefCnt.h>
#include <SkScalar.h>
#include <SkPaint.h>
#include <SkGradientShader.h>

using namespace v8;
using namespace node;

class CanvasGradient : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

  enum GradientType {
    LinearType,
    RadialType,
  };

  sk_sp<SkShader> getShader() const;

protected:
  CanvasGradient(float x0, float y0, float x1, float y1);
  CanvasGradient(float x0, float y0, float r0, float x1, float y1, float r1);
  virtual ~CanvasGradient();

  void AddColorStop(SkScalar offset, SkColor color);

  static NAN_METHOD(New);
  static NAN_METHOD(AddColorStop);

  GradientType type;
  float x0;
  float y0;
  float r0;
  float x1;
  float y1;
  float r1;
  std::vector<SkScalar> positions;
  std::vector<SkColor> colors;

  friend class CanvasRenderingContext2D;
};

#include "canvas-context.h"

#endif

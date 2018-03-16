#ifndef _CANVASCONTEXT_CANVASPATTERN_H_
#define _CANVASCONTEXT_CANVASPATTERN_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <SkRefCnt.h>
#include <SkShader.h>

using namespace v8;
using namespace node;

class CanvasPattern : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  CanvasPattern(sk_sp<SkShader> shader);
  virtual ~CanvasPattern();

  static NAN_METHOD(New);

  sk_sp<SkShader> shader;

  friend class CanvasRenderingContext2D;
};

#include "canvas-context.h"

#endif

#ifndef _CANVASCONTEXT_IMAGEDATA_H_
#define _CANVASCONTEXT_IMAGEDATA_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/ImageData.h>

using namespace v8;
using namespace node;

class CanvasRenderingContext2D;

class ImageData : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  unsigned int GetWidth();
  unsigned int GetHeight();
  unsigned int GetNumChannels();
  unsigned char *GetData();

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(DataGetter);

  ImageData(unsigned int width, unsigned int height);
  virtual ~ImageData();

private:
  canvas::ImageData *imageData;
  Nan::Persistent<Uint8ClampedArray> dataArray;

  friend class CanvasRenderingContext2D;
};

#include "canvas-context.h"

#endif

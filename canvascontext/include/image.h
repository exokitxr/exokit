#ifndef _CANVASCONTEXT_IMAGE_H_
#define _CANVASCONTEXT_IMAGE_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/Image.h>
#include <canvas/include/ImageData.h>

using namespace v8;
using namespace node;

class CanvasRenderingContext2D;

class Image : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate, canvas::ContextFactory *canvasContextFactory);
  unsigned int GetWidth();
  unsigned int GetHeight();
  unsigned int GetNumChannels();
  unsigned char *GetData();
  bool Load(const unsigned char *buffer, size_t size);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(DataGetter);
  static NAN_METHOD(LoadMethod);

  Image();
  virtual ~Image();

private:
  static canvas::ContextFactory *canvasContextFactory;

  canvas::Image *image;

  friend class CanvasRenderingContext2D;
  friend class ImageData;
  friend class ImageBitmap;
};

#include "canvas.h"

#endif

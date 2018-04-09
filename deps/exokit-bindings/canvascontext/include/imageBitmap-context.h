#ifndef _CANVASCONTEXT_IMAGEBITMAP_H_
#define _CANVASCONTEXT_IMAGEBITMAP_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/ImageData.h>
#include <SkBitmap.h>

using namespace v8;
using namespace node;

class CanvasRenderingContext2D;
class Image;

class ImageBitmap : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> imageCons);
  unsigned int GetWidth();
  unsigned int GetHeight();
  unsigned int GetNumChannels();
  // unsigned char *GetData();
  // void Set(canvas::ImageData *imageData);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(DataGetter);

  ImageBitmap();
  ImageBitmap(unsigned int width, unsigned int height, unsigned char *data);
  // ImageBitmap(Image *image, int x, int y, unsigned int width, unsigned int height, bool flipY);
  ImageBitmap(const SkBitmap &bitmap);
  virtual ~ImageBitmap();

private:
  SkBitmap bitmap;
  Nan::Persistent<Uint8ClampedArray> dataArray;

  friend class CanvasRenderingContext2D;
};

#include "canvas-context.h"

#endif

#ifndef IMAGEBITMAP_H_
#define IMAGEBITMAP_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <Context.h>
#include <ImageData.h>

using namespace v8;
using namespace node;

class CanvasRenderingContext2D;
class Image;

class ImageBitmap : public ObjectWrap {
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
  static NAN_METHOD(CreateImageBitmap);

  ImageBitmap(Image *image);
  virtual ~ImageBitmap();

private:
  canvas::ImageData *imageData;

  friend class CanvasRenderingContext2D;
};

#include "canvas.h"

#endif

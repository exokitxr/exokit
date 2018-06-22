#ifndef _CANVASCONTEXT_IMAGE_H_
#define _CANVASCONTEXT_IMAGE_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/Image.h>
#include <canvas/include/ImageData.h>
#include <canvas/include/Resources.h>
#include <SkData.h>
#include <SkImage.h>
#include <nanosvg.h>
#include <nanosvgrast.h>
#include <string>
#include <thread>

using namespace v8;
using namespace node;

class Image : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  unsigned int GetWidth();
  unsigned int GetHeight();
  unsigned int GetNumChannels();
  // unsigned char *GetData();
  static void RunInMainThread(uv_async_t *handle);
  void Load(Local<ArrayBuffer> arrayBuffer, size_t byteOffset, size_t byteLength, Local<Function> cbFn);
  // void Set(canvas::Image *image);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(DataGetter);
  static NAN_METHOD(LoadMethod);

  Image();
  ~Image();

private:
  sk_sp<SkImage> image;
  Nan::Persistent<Uint8ClampedArray> dataArray;

  Nan::Persistent<ArrayBuffer> arrayBuffer;
  Nan::Persistent<Function> cbFn;
  std::string error;

  friend class CanvasRenderingContext2D;
  friend class ImageData;
  friend class ImageBitmap;
};

#include "canvas-context.h"

#endif

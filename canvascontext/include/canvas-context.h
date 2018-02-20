#ifndef _CANVASCONTEXT_CANVAS_H_
#define _CANVASCONTEXT_CANVAS_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/Image.h>
#include <canvas/include/ImageData.h>
#include <canvas/include/web_color.h>
#include <SkRefCnt.h>
#include <SkScalar.h>
#include <SkRect.h>
#include <SkSurface.h>
#include <SkCanvas.h>
#include <SkPath.h>
#include <SkPaint.h>

using namespace v8;
using namespace node;

class Image;
class ImageData;
class ImageBitmap;
class Path2D;

class CanvasRenderingContext2D : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> imageDataCons);
  unsigned int GetWidth();
  unsigned int GetHeight();
  unsigned int GetNumChannels();
  void Scale(double x, double y);
  void Rotate(double angle);
  void Translate(double x, double y);
  void Transform(double a, double b, double c, double d, double e, double f);
  void SetTransform(double a, double b, double c, double d, double e, double f);
  void ResetTransform();
  double MeasureText(const std::string &text);
  void BeginPath();
  void ClosePath();
  void Clip();
  void Stroke();
  void Stroke(const Path2D &path);
  void Fill();
  void Fill(const Path2D &path);
  void MoveTo(double x, double y);
  void LineTo(double x, double y);
  void Arc(float x, float y, float radius, float startAngle, float endAngle, float anticlockwise);
  void ArcTo(double x1, double y1, double x2, double y2, double radius);
  void Rect(float x, float y, float w, float h);
  void FillRect(float x, float y, float w, float h);
  void StrokeRect(float x, float y, float w, float h);
  void ClearRect(float x, float y, float w, float h);
  void FillText(const std::string &text, float x, float y);
  void StrokeText(const std::string &text, float x, float y);
  void Resize(unsigned int w, unsigned int h);
  void DrawImage(const SkImage *image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
  void Save();
  void Restore();

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(DataGetter);
  static NAN_GETTER(LineWidthGetter);
  static NAN_SETTER(LineWidthSetter);
  static NAN_GETTER(FillStyleGetter);
  static NAN_SETTER(FillStyleSetter);
  static NAN_GETTER(StrokeStyleGetter);
  static NAN_SETTER(StrokeStyleSetter);
  static NAN_METHOD(Scale);
  static NAN_METHOD(Rotate);
  static NAN_METHOD(Translate);
  static NAN_METHOD(Transform);
  static NAN_METHOD(SetTransform);
  static NAN_METHOD(ResetTransform);
  static NAN_METHOD(MeasureText);
  static NAN_METHOD(BeginPath);
  static NAN_METHOD(ClosePath);
  static NAN_METHOD(Clip);
  static NAN_METHOD(Stroke);
  static NAN_METHOD(Fill);
  static NAN_METHOD(MoveTo);
  static NAN_METHOD(LineTo);
  static NAN_METHOD(Arc);
  static NAN_METHOD(ArcTo);
  static NAN_METHOD(Rect);
  static NAN_METHOD(FillRect);
  static NAN_METHOD(StrokeRect);
  static NAN_METHOD(ClearRect);
  static NAN_METHOD(FillText);
  static NAN_METHOD(StrokeText);
  static NAN_METHOD(Resize);
  static NAN_METHOD(DrawImage);
  static NAN_METHOD(CreateImageData);
  static NAN_METHOD(GetImageData);
  static NAN_METHOD(PutImageData);
  static NAN_METHOD(Save);
  static NAN_METHOD(Restore);

  CanvasRenderingContext2D(unsigned int width, unsigned int height);
  virtual ~CanvasRenderingContext2D();

public:
  static void InitalizeStatic(canvas::ContextFactory *newCanvasContextFactory);

private:
  static canvas::ContextFactory *canvasContextFactory;
  Nan::Persistent<Uint8ClampedArray> dataArray;

  sk_sp<SkSurface> surface;
  SkPath path;
  SkPaint strokePaint;
  SkPaint fillPaint;
  SkPaint clearPaint;

  friend class Image;
  friend class ImageData;
  friend class ImageBitmap;
  friend class Path2D;
};

#include "image-context.h"
#include "imageData-context.h"
#include "imageBitmap-context.h"
#include "path2d-context.h"

#endif

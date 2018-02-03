#ifndef CANVAS_H_
#define CANVAS_H_

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>
#include <ContextAndroid.h>
#include <Image.h>
#include <ImageData.h>

using namespace v8;
using namespace node;

class Image;
class ImageData;
class ImageBitmap;
class Path2D;

class CanvasRenderingContext2D : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate, canvas::ContextFactory *canvasContextFactory, Local<Value> imageDataCons);
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
  void ResetClip();
  void Stroke();
  void Stroke(Path2D &path2d);
  void Fill();
  void Fill(Path2D &path2d);
  void MoveTo(double x, double y);
  void LineTo(double x, double y);
  void Arc(double x, double y, double radius, double startAngle, double endAngle, double anticlockwise);
  void ArcTo(double x1, double y1, double x2, double y2, double radius);
  void Rect(double x, double y, double w, double h);
  void FillRect(double x, double y, double w, double h);
  void StrokeRect(double x, double y, double w, double h);
  void ClearRect(double x, double y, double w, double h);
  void FillText(const std::string &text, double x, double y);
  void StrokeText(const std::string &text, double x, double y);
  void Resize(unsigned int w, unsigned int h);
  void DrawImage(const canvas::ImageData &imageData, int sx, int sy, unsigned int sw, unsigned int sh, int dx, int dy, unsigned int dw, unsigned int dh);
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
  static NAN_METHOD(ResetClip);
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

private:
  static canvas::ContextFactory *canvasContextFactory;

  canvas::Context *context;

  friend class Image;
  friend class ImageData;
  friend class ImageBitmap;
  friend class Path2D;
};

#include "image.h"
#include "imageData.h"
#include "imageBitmap.h"
#include "path2d.h"

#endif

#ifndef _CANVASCONTEXT_CANVAS_H_
#define _CANVASCONTEXT_CANVAS_H_

#include <v8.h>
#include <nan.h>
#define _USE_MATH_DEFINES
#undef max
#include <cmath>
#include <defines.h>
#include <canvas/include/Context.h>
#include <canvas/include/Image.h>
#include <canvas/include/ImageData.h>
#include <canvas/include/web_color.h>
#include <canvas/include/web_font.h>
#include <SkRefCnt.h>
#include <SkScalar.h>
#include <SkRect.h>
#include <SkTypeface.h>
#include <SkSurface.h>
#include <SkCanvas.h>
#include <SkPath.h>
#include <SkPaint.h>
#include <webglcontext/include/webgl.h>

using namespace v8;
using namespace node;

class Image;
class ImageData;
class ImageBitmap;
class Path2D;
class CanvasGradient;

enum class TextBaseline {
  TOP,
  HANGING,
  MIDDLE,
  ALPHABETIC,
  IDEOGRAPHIC,
  BOTTOM,
};
enum class Direction {
  LEFT_TO_RIGHT,
  RIGHT_TO_LEFT,
};

class CanvasRenderingContext2D : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> imageDataCons, Local<Value> canvasGradientCons, Local<Value> canvasPatternCons);
  unsigned int GetWidth();
  unsigned int GetHeight();
  unsigned int GetNumChannels();
  void Scale(float x, float y);
  void Rotate(float angle);
  void Translate(float x, float y);
  void Transform(float a, float b, float c, float d, float e, float f);
  void SetTransform(float a, float b, float c, float d, float e, float f);
  void ResetTransform();
  float MeasureText(const std::string &text);
  void BeginPath();
  void ClosePath();
  void Clip();
  void Stroke();
  void Stroke(const Path2D &path);
  void Fill();
  void Fill(const Path2D &path);
  void MoveTo(float x, float y);
  void LineTo(float x, float y);
  void Arc(float x, float y, float radius, float startAngle, float endAngle, float anticlockwise);
  void ArcTo(float x1, float y1, float x2, float y2, float radius);
  void QuadraticCurveTo(float cpx, float cpy, float x, float y);
  void BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y);
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

// protected:
  static NAN_METHOD(New);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(TextureGetter);
  static NAN_GETTER(LineWidthGetter);
  static NAN_SETTER(LineWidthSetter);
  static NAN_GETTER(StrokeStyleGetter);
  static NAN_SETTER(StrokeStyleSetter);
  static NAN_GETTER(FillStyleGetter);
  static NAN_SETTER(FillStyleSetter);
  static NAN_GETTER(FontGetter);
  static NAN_SETTER(FontSetter);
  static NAN_GETTER(FontFamilyGetter);
  static NAN_SETTER(FontFamilySetter);
  static NAN_GETTER(FontSizeGetter);
  static NAN_SETTER(FontSizeSetter);
  static NAN_GETTER(FontWeightGetter);
  static NAN_SETTER(FontWeightSetter);
  static NAN_GETTER(LineHeightGetter);
  static NAN_SETTER(LineHeightSetter);
  static NAN_GETTER(FontStyleGetter);
  static NAN_SETTER(FontStyleSetter);
  static NAN_GETTER(FontVariantGetter);
  static NAN_SETTER(FontVariantSetter);
  static NAN_GETTER(TextAlignGetter);
  static NAN_SETTER(TextAlignSetter);
  static NAN_GETTER(TextBaselineGetter);
  static NAN_SETTER(TextBaselineSetter);
  static NAN_GETTER(DirectionGetter);
  static NAN_SETTER(DirectionSetter);
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
  static NAN_METHOD(QuadraticCurveTo);
  static NAN_METHOD(BezierCurveTo);
  static NAN_METHOD(Rect);
  static NAN_METHOD(FillRect);
  static NAN_METHOD(StrokeRect);
  static NAN_METHOD(ClearRect);
  static NAN_METHOD(FillText);
  static NAN_METHOD(StrokeText);
  static NAN_METHOD(CreateLinearGradient);
  static NAN_METHOD(CreateRadialGradient);
  static NAN_METHOD(CreatePattern);
  static NAN_METHOD(Resize);
  static NAN_METHOD(DrawImage);
  static NAN_METHOD(CreateImageData);
  static NAN_METHOD(GetImageData);
  static NAN_METHOD(PutImageData);
  static NAN_METHOD(Save);
  static NAN_METHOD(Restore);
  static NAN_METHOD(ToArrayBuffer);
  static NAN_METHOD(Destroy);
  static NAN_METHOD(GetWindowHandle);
  static NAN_METHOD(SetWindowHandle);
  static NAN_METHOD(SetTexture);

  static bool isImageType(Local<Value> arg);
  static sk_sp<SkImage> getImageFromContext(CanvasRenderingContext2D *ctx);
  static sk_sp<SkImage> getImage(Local<Value> arg);

  CanvasRenderingContext2D();
  virtual ~CanvasRenderingContext2D();

// protected:
  bool live;
  NATIVEwindow *windowHandle;
  GLuint tex;
  sk_sp<GrContext> grContext;
  sk_sp<SkSurface> surface;
  SkPath path;
  SkPaint strokePaint;
  SkPaint fillPaint;
  SkPaint clearPaint;
  float lineHeight;
  std::string textAlign;
  TextBaseline textBaseline;
  Direction direction;

  /* friend class Image;
  friend class ImageData;
  friend class ImageBitmap;
  friend class Path2D;
  friend class CanvasGradient;
  friend class CanvasPattern; */
};

#include "image-context.h"
#include "imageData-context.h"
#include "imageBitmap-context.h"
#include "path2d-context.h"
#include "canvas-gradient.h"
#include "canvas-pattern.h"

#endif

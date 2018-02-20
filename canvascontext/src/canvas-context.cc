#include <canvascontext/include/canvas-context.h>

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> CanvasRenderingContext2D::Initialize(Isolate *isolate, Local<Value> imageDataCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("CanvasRenderingContext2D"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto,"scale", Scale);
  Nan::SetMethod(proto,"rotate", Rotate);
  Nan::SetMethod(proto,"translate", Translate);
  Nan::SetMethod(proto,"transform", Transform);
  Nan::SetMethod(proto,"setTransform", SetTransform);
  Nan::SetMethod(proto,"resetTransform", ResetTransform);
  Nan::SetMethod(proto,"measureText", MeasureText);
  Nan::SetMethod(proto,"beginPath", BeginPath);
  Nan::SetMethod(proto,"closePath", ClosePath);
  Nan::SetMethod(proto,"clip", Clip);
  Nan::SetMethod(proto,"stroke", Stroke);
  Nan::SetMethod(proto,"fill", Fill);
  Nan::SetMethod(proto,"moveTo", MoveTo);
  Nan::SetMethod(proto,"lineTo", LineTo);
  Nan::SetMethod(proto,"arc", Arc);
  Nan::SetMethod(proto,"arcTo", ArcTo);
  Nan::SetMethod(proto,"rect", Rect);
  Nan::SetMethod(proto,"fillRect", FillRect);
  Nan::SetMethod(proto,"strokeRect", StrokeRect);
  Nan::SetMethod(proto,"clearRect", ClearRect);
  Nan::SetMethod(proto,"fillText", FillText);
  Nan::SetMethod(proto,"strokeText", StrokeText);
  Nan::SetMethod(proto,"resize", Resize);
  Nan::SetMethod(proto,"drawImage", DrawImage);
  Nan::SetMethod(proto,"createImageData", CreateImageData);
  Nan::SetMethod(proto,"getImageData", GetImageData);
  Nan::SetMethod(proto,"putImageData", PutImageData);

  Local<Function> ctorFn = ctor->GetFunction();
  ctorFn->Set(JS_STR("ImageData"), imageDataCons);

  return scope.Escape(ctorFn);
}

unsigned int CanvasRenderingContext2D::GetWidth() {
  return surface->width();
}

unsigned int CanvasRenderingContext2D::GetHeight() {
  return surface->height();
}

unsigned int CanvasRenderingContext2D::GetNumChannels() {
  return 4;
}

void CanvasRenderingContext2D::Scale(double x, double y) {
  surface->getCanvas()->scale(x, y);
}

void CanvasRenderingContext2D::Rotate(double angle) {
  surface->getCanvas()->rotate(angle);
}

void CanvasRenderingContext2D::Translate(double x, double y) {
  surface->getCanvas()->translate(x, y);
}

void CanvasRenderingContext2D::Transform(double a, double b, double c, double d, double e, double f) {
  SkScalar affine[] = {a, b, c, d, e, f};
  SkMatrix m;
  m.setAffine(affine);
  surface->getCanvas()->setMatrix(m);
}

void CanvasRenderingContext2D::SetTransform(double a, double b, double c, double d, double e, double f) {
  SkScalar affine[] = {a, b, c, d, e, f};
  SkMatrix m;
  m.setAffine(affine);
  surface->getCanvas()->setMatrix(m);
}

void CanvasRenderingContext2D::ResetTransform() {
  surface->getCanvas()->resetMatrix();
}

double CanvasRenderingContext2D::MeasureText(const std::string &text) {
  SkRect bounds;
  strokePaint.measureText(text.c_str(), text.length(), &bounds);
  return bounds.width();
}

void CanvasRenderingContext2D::BeginPath() {
  path.reset();
}

void CanvasRenderingContext2D::ClosePath() {
  path.close();
}

void CanvasRenderingContext2D::Clip() {
  surface->getCanvas()->clipPath(path);
}

void CanvasRenderingContext2D::Stroke() {
  surface->getCanvas()->drawPath(path, strokePaint);
}

void CanvasRenderingContext2D::Stroke(const Path2D &path) {
  surface->getCanvas()->drawPath(path.path, strokePaint);
}

void CanvasRenderingContext2D::Fill() {
  surface->getCanvas()->drawPath(path, fillPaint);
}

void CanvasRenderingContext2D::Fill(const Path2D &path) {
  surface->getCanvas()->drawPath(path.path, fillPaint);
}

void CanvasRenderingContext2D::MoveTo(double x, double y) {
  path.moveTo(x, y);
}

void CanvasRenderingContext2D::LineTo(double x, double y) {
  path.lineTo(x, y);
}

void CanvasRenderingContext2D::Arc(float x, float y, float radius, float startAngle, float endAngle, float anticlockwise) {
  if (anticlockwise) {
    float temp = startAngle;
    startAngle = endAngle;
    endAngle = temp;
  }
  path.addArc({x - radius/2, y - radius/2, x + radius/2, y + radius/2}, startAngle, endAngle);
}

void CanvasRenderingContext2D::ArcTo(double x1, double y1, double x2, double y2, double radius) {
  path.arcTo(x1, y1, x2 - x1, y2 - y1, radius);
}

void CanvasRenderingContext2D::Rect(float x, float y, float w, float h) {
  path.addRect({x, y, w, h});
}

void CanvasRenderingContext2D::FillRect(float x, float y, float w, float h) {
  SkPath path;
  path.addRect({x, y, w, h});
  surface->getCanvas()->drawPath(path, fillPaint);
}

void CanvasRenderingContext2D::StrokeRect(float x, float y, float w, float h) {
  SkPath path;
  path.addRect({x, y, w, h});
  surface->getCanvas()->drawPath(path, strokePaint);
}

void CanvasRenderingContext2D::ClearRect(float x, float y, float w, float h) {
  SkPath path;
  path.addRect({x, y, w, h});
  surface->getCanvas()->drawPath(path, clearPaint);
}

void CanvasRenderingContext2D::FillText(const std::string &text, float x, float y) {
  surface->getCanvas()->drawText(text.c_str(), text.length(), x, y, fillPaint);
}

void CanvasRenderingContext2D::StrokeText(const std::string &text, float x, float y) {
  surface->getCanvas()->drawText(text.c_str(), text.length(), x, y, strokePaint);
}

void CanvasRenderingContext2D::Resize(unsigned int w, unsigned int h) {
  surface = SkSurface::MakeRasterN32Premul(w, h);
}

void CanvasRenderingContext2D::DrawImage(const SkImage *image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
  SkPaint paint;
  surface->getCanvas()->drawImageRect(image, SkRect{sx, sy, sw, sh}, SkRect{dx, dy, dw, dh}, &paint);
}

void CanvasRenderingContext2D::Save() {
  surface->getCanvas()->save();
}

void CanvasRenderingContext2D::Restore() {
  surface->getCanvas()->restore();
}

NAN_METHOD(CanvasRenderingContext2D::New) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    unsigned int width = info[0]->Uint32Value();
    unsigned int height = info[1]->Uint32Value();
    CanvasRenderingContext2D *context = new CanvasRenderingContext2D(width, height);
    Local<Object> canvasObj = info.This();
    context->Wrap(canvasObj);

    Nan::SetAccessor(canvasObj, JS_STR("width"), WidthGetter);
    Nan::SetAccessor(canvasObj, JS_STR("height"), HeightGetter);
    Nan::SetAccessor(canvasObj, JS_STR("data"), DataGetter);
    Nan::SetAccessor(canvasObj, JS_STR("lineWidth"), LineWidthGetter, LineWidthSetter);
    Nan::SetAccessor(canvasObj, JS_STR("fillStyle"), FillStyleGetter, FillStyleSetter);
    Nan::SetAccessor(canvasObj, JS_STR("strokeStyle"), StrokeStyleGetter, StrokeStyleSetter);

    info.GetReturnValue().Set(canvasObj);
  } else {
    return Nan::ThrowError("Invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::WidthGetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  info.GetReturnValue().Set(JS_INT(context->GetWidth()));
}

NAN_GETTER(CanvasRenderingContext2D::HeightGetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  info.GetReturnValue().Set(JS_INT(context->GetHeight()));
}

NAN_GETTER(CanvasRenderingContext2D::DataGetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

  if (context->dataArray.IsEmpty()) {
    unsigned int width = context->GetWidth();
    unsigned int height = context->GetHeight();

    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), width * height * 4); // XXX link lifetime

    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(width, height);
    bool ok = context->surface->getCanvas()->readPixels(imageInfo, arrayBuffer->GetContents().Data(), width * 4, 0, 0);
    if (ok) {
      // nothing
    } else {
      return info.GetReturnValue().Set(Nan::Null());
    }
  }

  return info.GetReturnValue().Set(Nan::New(context->dataArray));
}

NAN_GETTER(CanvasRenderingContext2D::LineWidthGetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  info.GetReturnValue().Set(JS_FLOAT(context->strokePaint.getStrokeWidth()));
}

NAN_SETTER(CanvasRenderingContext2D::LineWidthSetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  float lineWidth = info.Data()->NumberValue();
  context->strokePaint.setStrokeWidth(lineWidth);
}

NAN_GETTER(CanvasRenderingContext2D::FillStyleGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FillStyleSetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  v8::String::Utf8Value text(info.Data());
  std::string fillStyle(*text, text.length());

  canvas::web_color webColor = canvas::web_color::from_string(fillStyle.c_str());
  context->fillPaint.setColor(((uint32_t)webColor.r << (8 * 3)) | ((uint32_t)webColor.g << (8 * 2)) | ((uint32_t)webColor.g << (8 * 1)) | ((uint32_t)webColor.a << (8 * 0)));
}

NAN_GETTER(CanvasRenderingContext2D::StrokeStyleGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::StrokeStyleSetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  v8::String::Utf8Value text(info.Data());
  std::string strokeStyle(*text, text.length());

  canvas::web_color webColor = canvas::web_color::from_string(strokeStyle.c_str());
  context->strokePaint.setColor(((uint32_t)webColor.r << (8 * 3)) | ((uint32_t)webColor.g << (8 * 2)) | ((uint32_t)webColor.g << (8 * 1)) | ((uint32_t)webColor.a << (8 * 0)));
}

NAN_METHOD(CanvasRenderingContext2D::Scale) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  context->Scale(x, y);
}

NAN_METHOD(CanvasRenderingContext2D::Rotate) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double angle = info[0]->NumberValue();
  context->Rotate(angle);
}

NAN_METHOD(CanvasRenderingContext2D::Translate) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  context->Translate(x, y);
}

NAN_METHOD(CanvasRenderingContext2D::Transform) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double a = info[0]->NumberValue();
  double b = info[1]->NumberValue();
  double c = info[2]->NumberValue();
  double d = info[3]->NumberValue();
  double e = info[4]->NumberValue();
  double f = info[5]->NumberValue();
  context->Transform(a, b, c, d, e, f);
}

NAN_METHOD(CanvasRenderingContext2D::SetTransform) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double a = info[0]->NumberValue();
  double b = info[1]->NumberValue();
  double c = info[2]->NumberValue();
  double d = info[3]->NumberValue();
  double e = info[4]->NumberValue();
  double f = info[5]->NumberValue();
  context->SetTransform(a, b, c, d, e, f);
}

NAN_METHOD(CanvasRenderingContext2D::ResetTransform) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  context->ResetTransform();
}

NAN_METHOD(CanvasRenderingContext2D::MeasureText) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  v8::String::Utf8Value textUtf8(info[0]);
  std::string text(*textUtf8, textUtf8.length());

  Local<Object> result = Object::New(Isolate::GetCurrent());
  result->Set(JS_STR("width"), JS_FLOAT(context->MeasureText(text)));

  info.GetReturnValue().Set(result);
}

NAN_METHOD(CanvasRenderingContext2D::BeginPath) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  context->BeginPath();
}

NAN_METHOD(CanvasRenderingContext2D::ClosePath) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  context->ClosePath();
}

NAN_METHOD(CanvasRenderingContext2D::Clip) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  context->Clip();
}

NAN_METHOD(CanvasRenderingContext2D::Stroke) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

  if (info[0]->BooleanValue() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("Path2D"))) {
    Path2D *path2d = ObjectWrap::Unwrap<Path2D>(Local<Object>::Cast(info[0]));

    context->Stroke(*path2d);
  } else {
    context->Stroke();
  }
}

NAN_METHOD(CanvasRenderingContext2D::Fill) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

  if (info[0]->BooleanValue() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("Path2D"))) {
    Path2D *path2d = ObjectWrap::Unwrap<Path2D>(Local<Object>::Cast(info[0]));

    context->Fill(*path2d);
  } else {
    context->Fill();
  }
}

NAN_METHOD(CanvasRenderingContext2D::MoveTo) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();

  context->MoveTo(x, y);
}

NAN_METHOD(CanvasRenderingContext2D::LineTo) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();

  context->LineTo(x, y);
}

NAN_METHOD(CanvasRenderingContext2D::Arc) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  double radius = info[2]->NumberValue();
  double startAngle = info[3]->NumberValue();
  double endAngle = info[4]->NumberValue();
  double anticlockwise = info[5]->NumberValue();

  context->Arc(x, y, radius, startAngle, endAngle, anticlockwise);
}

NAN_METHOD(CanvasRenderingContext2D::ArcTo) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x1 = info[0]->NumberValue();
  double y1 = info[1]->NumberValue();
  double x2 = info[2]->NumberValue();
  double y2 = info[3]->NumberValue();
  double radius = info[4]->NumberValue();

  context->ArcTo(x1, y1, x2, y2, radius);
}

NAN_METHOD(CanvasRenderingContext2D::Rect) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  double w = info[2]->NumberValue();
  double h = info[3]->NumberValue();

  context->Rect(x, y, w, h);

  // info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_METHOD(CanvasRenderingContext2D::FillRect) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  double w = info[2]->NumberValue();
  double h = info[3]->NumberValue();

  context->FillRect(x, y, w, h);

  // info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_METHOD(CanvasRenderingContext2D::StrokeRect) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  double w = info[2]->NumberValue();
  double h = info[3]->NumberValue();

  context->StrokeRect(x, y, w, h);

  // info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_METHOD(CanvasRenderingContext2D::ClearRect) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  double w = info[2]->NumberValue();
  double h = info[3]->NumberValue();

  context->ClearRect(x, y, w, h);

  // info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_METHOD(CanvasRenderingContext2D::FillText) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  v8::String::Utf8Value text(info[0]);
  std::string string(*text, text.length());
  double x = info[1]->NumberValue();
  double y = info[2]->NumberValue();

  context->FillText(string, x, y);

  // info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_METHOD(CanvasRenderingContext2D::StrokeText) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  v8::String::Utf8Value text(info[0]);
  std::string string(*text, text.length());
  double x = info[1]->NumberValue();
  double y = info[2]->NumberValue();

  context->StrokeText(string, x, y);

  // info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_METHOD(CanvasRenderingContext2D::Resize) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  unsigned int w = info[0]->Uint32Value();
  unsigned int h = info[1]->Uint32Value();

  context->Resize(w, h);

  // info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_METHOD(CanvasRenderingContext2D::DrawImage) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("CanvasRenderingContext2D"))) {
    CanvasRenderingContext2D *otherContext = ObjectWrap::Unwrap<CanvasRenderingContext2D>(Local<Object>::Cast(info[0]));

    SkBitmap bitmap;
    bool ok = otherContext->surface->getCanvas()->readPixels(bitmap, 0, 0);
    if (ok) {
      bitmap.setImmutable();
      sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);

      /* canvas::Surface &surface = otherContext->context->getDefaultSurface();
      std::unique_ptr<canvas::Image> image = surface.createImage(1);
      canvas::ImageData &imageData = image->getData(); */

      int x = info[1]->Int32Value();
      int y = info[2]->Int32Value();
      if (info.Length() > 3) {
        if (info.Length() > 5) {
          unsigned int sw = info[3]->Uint32Value();
          unsigned int sh = info[4]->Uint32Value();
          unsigned int dw = info[5]->Uint32Value();
          unsigned int dh = info[6]->Uint32Value();

          context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
        } else {
          unsigned int dw = info[3]->Uint32Value();
          unsigned int dh = info[4]->Uint32Value();
          unsigned int sw = otherContext->GetWidth();
          unsigned int sh = otherContext->GetHeight();

          // context->DrawImage(imageData, 0, 0, sw, sh, x, y, dw, dh);
          context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
        }
      } else {
        unsigned int sw = otherContext->GetWidth();
        unsigned int sh = otherContext->GetHeight();
        unsigned int dw = sw;
        unsigned int dh = sh;

        // context->DrawImage(imageData, 0, 0, sw, sh, x, y, dw, dh);
        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      return Nan::ThrowError("Failed to read pixels");
    }
  } else if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLImageElement"))) {
    Image *image = ObjectWrap::Unwrap<Image>(Local<Object>::Cast(info[0]->ToObject()->Get(JS_STR("image"))));
    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();

    if (info.Length() > 3) {
      if (info.Length() > 5) {
        unsigned int sw = info[3]->Uint32Value();
        unsigned int sh = info[4]->Uint32Value();
        unsigned int dw = info[5]->Uint32Value();
        unsigned int dh = info[6]->Uint32Value();

        context->DrawImage(image->image.get(), 0, 0, sw, sh, x, y, dw, dh);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = image->GetWidth();
        unsigned int sh = image->GetHeight();

        context->DrawImage(image->image.get(), 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      unsigned int sw = image->GetWidth();
      unsigned int sh = image->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(image->image.get(), 0, 0, sw, sh, x, y, dw, dh);
    }
  } else if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("ImageData"))) {
    ImageData *imageData = ObjectWrap::Unwrap<ImageData>(Local<Object>::Cast(info[0]));
    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();

    sk_sp<SkImage> image = SkImage::MakeFromBitmap(imageData->bitmap);
    if (info.Length() > 3) {
      if (info.Length() > 5) {
        unsigned int sw = info[3]->Uint32Value();
        unsigned int sh = info[4]->Uint32Value();
        unsigned int dw = info[5]->Uint32Value();
        unsigned int dh = info[6]->Uint32Value();

        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = imageData->GetWidth();
        unsigned int sh = imageData->GetHeight();

        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      unsigned int sw = imageData->GetWidth();
      unsigned int sh = imageData->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
    }
  } else if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("ImageBitmap"))) {
    ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(Local<Object>::Cast(info[0]));
    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();

    sk_sp<SkImage> image = SkImage::MakeFromBitmap(imageBitmap->bitmap);
    if (info.Length() > 3) {
      if (info.Length() > 5) {
        unsigned int sw = info[3]->Uint32Value();
        unsigned int sh = info[4]->Uint32Value();
        unsigned int dw = info[5]->Uint32Value();
        unsigned int dh = info[6]->Uint32Value();

        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = imageBitmap->GetWidth();
        unsigned int sh = imageBitmap->GetHeight();

        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      unsigned int sw = imageBitmap->GetWidth();
      unsigned int sh = imageBitmap->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
    }
  }
}

NAN_METHOD(CanvasRenderingContext2D::CreateImageData) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(Local<Object>::Cast(info.This()));
  double w = info[0]->NumberValue();
  double h = info[1]->NumberValue();

  Local<Function> imageDataCons = Local<Function>::Cast(
    Local<Object>::Cast(info.This())->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("ImageData"))
  );
  Local<Value> argv[] = {
    Number::New(Isolate::GetCurrent(), w),
    Number::New(Isolate::GetCurrent(), h),
  };
  Local<Object> imageDataObj = imageDataCons->NewInstance(sizeof(argv)/sizeof(argv[0]), argv);

  info.GetReturnValue().Set(imageDataObj);
}

NAN_METHOD(CanvasRenderingContext2D::GetImageData) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(Local<Object>::Cast(info.This()));
  int x = info[0]->Int32Value();
  int y = info[1]->Int32Value();
  unsigned int w = info[2]->Uint32Value();
  unsigned int h = info[3]->Uint32Value();

  Local<Function> imageDataCons = Local<Function>::Cast(
    Local<Object>::Cast(info.This())->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("ImageData"))
  );
  Local<Value> argv[] = {
    Number::New(Isolate::GetCurrent(), w),
    Number::New(Isolate::GetCurrent(), h),
  };
  Local<Object> imageDataObj = imageDataCons->NewInstance(sizeof(argv)/sizeof(argv[0]), argv);
  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(imageDataObj);
  context->surface->getCanvas()->readPixels(imageData->bitmap, x, y);

  info.GetReturnValue().Set(imageDataObj);
}

NAN_METHOD(CanvasRenderingContext2D::PutImageData) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(Local<Object>::Cast(info.This()));
  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(Local<Object>::Cast(info[0]));
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();

  if (info.Length() > 3) {
    int dirtyX = info[3]->Int32Value();
    int dirtyY = info[4]->Int32Value();
    unsigned int dirtyWidth = info[5]->Uint32Value();
    unsigned int dirtyHeight = info[6]->Uint32Value();
    unsigned int dw = imageData->GetWidth();
    unsigned int dh = imageData->GetHeight();

    sk_sp<SkImage> image = SkImage::MakeFromBitmap(imageData->bitmap);
    context->DrawImage(image.get(), dirtyX, dirtyY, dirtyWidth, dirtyHeight, x, y, dw, dh);
  } else {
    unsigned int sw = imageData->GetWidth();
    unsigned int sh = imageData->GetHeight();
    unsigned int dw = sw;
    unsigned int dh = sh;

    sk_sp<SkImage> image = SkImage::MakeFromBitmap(imageData->bitmap);
    context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh);
  }
}

NAN_METHOD(CanvasRenderingContext2D::Save) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  context->Save();
}

NAN_METHOD(CanvasRenderingContext2D::Restore) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  context->Restore();
}

CanvasRenderingContext2D::CanvasRenderingContext2D(unsigned int width, unsigned int height) {
  surface = SkSurface::MakeRasterN32Premul(width, height);
  strokePaint.setStyle(SkPaint::kStroke_Style);
  fillPaint.setStyle(SkPaint::kFill_Style);
  clearPaint.setStyle(SkPaint::kFill_Style);
}
CanvasRenderingContext2D::~CanvasRenderingContext2D () {}

canvas::ContextFactory *CanvasRenderingContext2D::canvasContextFactory;
void CanvasRenderingContext2D::InitalizeStatic(canvas::ContextFactory *newCanvasContextFactory) {
  CanvasRenderingContext2D::canvasContextFactory = newCanvasContextFactory;
}

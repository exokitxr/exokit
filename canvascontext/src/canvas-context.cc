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
  Nan::SetMethod(proto,"resetClip", ResetClip);
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
  return context->getWidth();
}

unsigned int CanvasRenderingContext2D::GetHeight() {
  return context->getHeight();
}

unsigned int CanvasRenderingContext2D::GetNumChannels() {
  return context->getDefaultSurface().getNumChannels();
}

void CanvasRenderingContext2D::Scale(double x, double y) {
  context->scale(x, y);
}

void CanvasRenderingContext2D::Rotate(double angle) {
  context->rotate(angle);
}

void CanvasRenderingContext2D::Translate(double x, double y) {
  context->translate(x, y);
}

void CanvasRenderingContext2D::Transform(double a, double b, double c, double d, double e, double f) {
  context->transform(a, b, c, d, e, f);
}

void CanvasRenderingContext2D::SetTransform(double a, double b, double c, double d, double e, double f) {
  context->setTransform(a, b, c, d, e, f);
}

void CanvasRenderingContext2D::ResetTransform() {
  context->resetTransform();
}

double CanvasRenderingContext2D::MeasureText(const std::string &text) {
  return context->measureText(text).width;
}

void CanvasRenderingContext2D::BeginPath() {
  context->beginPath();
}

void CanvasRenderingContext2D::ClosePath() {
  context->closePath();
}

void CanvasRenderingContext2D::Clip() {
  context->clip();
}

void CanvasRenderingContext2D::ResetClip() {
  context->resetClip();
}

void CanvasRenderingContext2D::Stroke() {
  context->stroke();
}

void CanvasRenderingContext2D::Stroke(Path2D &path2d) {
  context->stroke(*path2d.path2d);
}

void CanvasRenderingContext2D::Fill() {
  context->fill();
}

void CanvasRenderingContext2D::Fill(Path2D &path2d) {
  context->fill(*path2d.path2d);
}

void CanvasRenderingContext2D::MoveTo(double x, double y) {
  context->moveTo(x, y);
}

void CanvasRenderingContext2D::LineTo(double x, double y) {
  context->lineTo(x, y);
}

void CanvasRenderingContext2D::Arc(double x, double y, double radius, double startAngle, double endAngle, double anticlockwise) {
  context->arc(x, y, radius, startAngle, endAngle, anticlockwise);
}

void CanvasRenderingContext2D::ArcTo(double x1, double y1, double x2, double y2, double radius) {
  context->arcTo(x1, y1, x2, y2, radius);
}

void CanvasRenderingContext2D::Rect(double x, double y, double w, double h) {
  context->rect(x, y, w, h);
}

void CanvasRenderingContext2D::FillRect(double x, double y, double w, double h) {
  context->fillRect(x, y, w, h);
}

void CanvasRenderingContext2D::StrokeRect(double x, double y, double w, double h) {
  context->strokeRect(x, y, w, h);
}

void CanvasRenderingContext2D::ClearRect(double x, double y, double w, double h) {
  context->clearRect(x, y, w, h);
}

void CanvasRenderingContext2D::FillText(const std::string &text, double x, double y) {
  context->fillText(text, x, y);
}

void CanvasRenderingContext2D::StrokeText(const std::string &text, double x, double y) {
  context->strokeText(text, x, y);
}

void CanvasRenderingContext2D::Resize(unsigned int w, unsigned int h) {
  context->resize(w, h);
}

void CanvasRenderingContext2D::DrawImage(const canvas::ImageData &imageData, int sx, int sy, unsigned int sw, unsigned int sh, int dx, int dy, unsigned int dw, unsigned int dh) {
  if (sx != 0 || sy != 0 || sw != imageData.getWidth() || sh != imageData.getHeight()) {
    std::unique_ptr<canvas::ImageData> scaledImageDataPtr(imageData.crop(sx, sy, sw, sh));
    context->drawImage(*scaledImageDataPtr, dx, dy, dw, dh);
  } else {
    context->drawImage(imageData, dx, dy, dw, dh);
  }
}

void CanvasRenderingContext2D::Save() {
  context->save();
}

void CanvasRenderingContext2D::Restore() {
  context->restore();
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
    unsigned int numChannels = context->GetNumChannels();
    if (numChannels == 4) {
      canvas::Surface &surface = context->context->getDefaultSurface();
      unsigned char *data = (unsigned char *)surface.lockMemory(false);

      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), width * height * 4);
      memcpy(arrayBuffer->GetContents().Data(), data, arrayBuffer->ByteLength());

      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      context->dataArray.Reset(uint8ClampedArray);

      surface.releaseMemory();
    } else {
      return Nan::ThrowError("CanvasRenderingContext2D invalid number of channels");
    }
  }

  info.GetReturnValue().Set(Nan::New(context->dataArray));
}

NAN_GETTER(CanvasRenderingContext2D::LineWidthGetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

  info.GetReturnValue().Set(JS_FLOAT(context->context->lineWidth.get()));
}

NAN_SETTER(CanvasRenderingContext2D::LineWidthSetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  float angle = info.Data()->NumberValue();

  context->context->lineWidth = angle;
}

NAN_GETTER(CanvasRenderingContext2D::FillStyleGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FillStyleSetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  v8::String::Utf8Value text(info.Data());
  std::string fillStyle(*text, text.length());

  context->context->fillStyle = fillStyle;
}

NAN_GETTER(CanvasRenderingContext2D::StrokeStyleGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::StrokeStyleSetter) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
  v8::String::Utf8Value text(info.Data());
  std::string strokeStyle(*text, text.length());

  context->context->strokeStyle = strokeStyle;
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
  v8::String::Utf8Value text(info[0]);
  std::string string(*text, text.length());
  double width = context->MeasureText(string);

  Local<Object> result = Object::New(Isolate::GetCurrent());
  result->Set(JS_STR("width"), JS_FLOAT(width));

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

NAN_METHOD(CanvasRenderingContext2D::ResetClip) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

  context->ResetClip();
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
    canvas::Surface &surface = otherContext->context->getDefaultSurface();
    std::unique_ptr<canvas::Image> image = surface.createImage(1);
    canvas::ImageData &imageData = image->getData();

    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();
    if (info.Length() > 3) {
      if (info.Length() > 5) {
        unsigned int sw = info[3]->Uint32Value();
        unsigned int sh = info[4]->Uint32Value();
        unsigned int dw = info[5]->Uint32Value();
        unsigned int dh = info[6]->Uint32Value();

        context->DrawImage(imageData, 0, 0, sw, sh, x, y, dw, dh);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = imageData.getWidth();
        unsigned int sh = imageData.getHeight();

        context->DrawImage(imageData, 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      unsigned int sw = imageData.getWidth();
      unsigned int sh = imageData.getHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(imageData, 0, 0, sw, sh, x, y, dw, dh);
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

        context->DrawImage(image->image->getData(), 0, 0, sw, sh, x, y, dw, dh);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = image->GetWidth();
        unsigned int sh = image->GetHeight();

        context->DrawImage(image->image->getData(), 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      unsigned int sw = image->GetWidth();
      unsigned int sh = image->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(image->image->getData(), 0, 0, sw, sh, x, y, dw, dh);
    }
  } else if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("ImageData"))) {
    ImageData *imageData = ObjectWrap::Unwrap<ImageData>(Local<Object>::Cast(info[0]));
    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();
    if (info.Length() > 3) {
      if (info.Length() > 5) {
        unsigned int sw = info[3]->Uint32Value();
        unsigned int sh = info[4]->Uint32Value();
        unsigned int dw = info[5]->Uint32Value();
        unsigned int dh = info[6]->Uint32Value();

        context->DrawImage(*imageData->imageData, 0, 0, sw, sh, x, y, dw, dh);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = imageData->GetWidth();
        unsigned int sh = imageData->GetHeight();

        context->DrawImage(*imageData->imageData, 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      unsigned int sw = imageData->GetWidth();
      unsigned int sh = imageData->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(*imageData->imageData, 0, 0, sw, sh, x, y, dw, dh);
    }
  } else if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("ImageBitmap"))) {
    ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(Local<Object>::Cast(info[0]));
    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();
    if (info.Length() > 3) {
      if (info.Length() > 5) {
        unsigned int sw = info[3]->Uint32Value();
        unsigned int sh = info[4]->Uint32Value();
        unsigned int dw = info[5]->Uint32Value();
        unsigned int dh = info[6]->Uint32Value();

        context->DrawImage(*imageBitmap->imageData, 0, 0, sw, sh, x, y, dw, dh);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = imageBitmap->GetWidth();
        unsigned int sh = imageBitmap->GetHeight();

        context->DrawImage(*imageBitmap->imageData, 0, 0, sw, sh, x, y, dw, dh);
      }
    } else {
      unsigned int sw = imageBitmap->GetWidth();
      unsigned int sh = imageBitmap->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(*imageBitmap->imageData, 0, 0, sw, sh, x, y, dw, dh);
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
  Local<Object> imageDataObj = imageDataCons->NewInstance(sizeof(argv) / sizeof(argv[0]), argv);

  info.GetReturnValue().Set(imageDataObj);
}

NAN_METHOD(CanvasRenderingContext2D::GetImageData) {
  Nan::HandleScope scope;

  CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(Local<Object>::Cast(info.This()));
  int x = info[0]->Int32Value();
  int y = info[1]->Int32Value();
  unsigned int w = info[2]->Uint32Value();
  unsigned int h = info[3]->Uint32Value();

  canvas::Surface &canvasSurface = context->context->getDefaultSurface();
  std::unique_ptr<canvas::Image> canvasImg(canvasSurface.createImage(1));
  const canvas::ImageData &canvasImageData = canvasImg->getData();
  unique_ptr<canvas::ImageData> croppedImageData(canvasImageData.crop(x, y, w, h));

  Local<Function> imageDataCons = Local<Function>::Cast(
    Local<Object>::Cast(info.This())->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("ImageData"))
  );
  Local<Value> argv[] = {};
  Local<Object> imageDataObj = imageDataCons->NewInstance(sizeof(argv) / sizeof(argv[0]), argv);
  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(imageDataObj);
  imageData->Set(croppedImageData.release());

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

    context->DrawImage(*imageData->imageData, dirtyX, dirtyY, dirtyWidth, dirtyHeight, x, y, dw, dh);
  } else {
    unsigned int sw = imageData->GetWidth();
    unsigned int sh = imageData->GetHeight();
    unsigned int dw = sw;
    unsigned int dh = sh;

    context->DrawImage(*imageData->imageData, 0, 0, sw, sh, x, y, dw, dh);
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
  context = CanvasRenderingContext2D::canvasContextFactory->createContext(width, height, 4).release();
}
CanvasRenderingContext2D::~CanvasRenderingContext2D () {
  delete context;
}

canvas::ContextFactory *CanvasRenderingContext2D::canvasContextFactory;
void CanvasRenderingContext2D::InitalizeStatic(canvas::ContextFactory *newCanvasContextFactory) {
  CanvasRenderingContext2D::canvasContextFactory = newCanvasContextFactory;
}

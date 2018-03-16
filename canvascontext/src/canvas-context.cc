#include <canvascontext/include/canvas-context.h>

using namespace v8;
using namespace node;

void flipCanvasY(SkCanvas *canvas, float height) {
  canvas->translate(0, canvas->imageInfo().height());
  canvas->scale(1.0, -1.0);
}

Handle<Object> CanvasRenderingContext2D::Initialize(Isolate *isolate, Local<Value> imageDataCons, Local<Value> canvasGradientCons) {
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
  Nan::SetMethod(proto,"createLinearGradient", CreateLinearGradient);
  Nan::SetMethod(proto,"createRadialGradient", CreateRadialGradient);
  Nan::SetMethod(proto,"resize", Resize);
  Nan::SetMethod(proto,"drawImage", DrawImage);
  Nan::SetMethod(proto,"createImageData", CreateImageData);
  Nan::SetMethod(proto,"getImageData", GetImageData);
  Nan::SetMethod(proto,"putImageData", PutImageData);

  Local<Function> ctorFn = ctor->GetFunction();
  ctorFn->Set(JS_STR("ImageData"), imageDataCons);
  ctorFn->Set(JS_STR("CanvasGradient"), canvasGradientCons);

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

void CanvasRenderingContext2D::Scale(float x, float y) {
  surface->getCanvas()->scale(x, y);
}

void CanvasRenderingContext2D::Rotate(float angle) {
  surface->getCanvas()->rotate(angle);
}

void CanvasRenderingContext2D::Translate(float x, float y) {
  surface->getCanvas()->translate(x, y);
}

void CanvasRenderingContext2D::Transform(float a, float b, float c, float d, float e, float f) {
  SkScalar affine[] = {a, b, c, d, e, f};
  SkMatrix m;
  m.setAffine(affine);
  surface->getCanvas()->setMatrix(m);
}

void CanvasRenderingContext2D::SetTransform(float a, float b, float c, float d, float e, float f) {
  SkScalar affine[] = {a, b, c, d, e, f};
  SkMatrix m;
  m.setAffine(affine);
  surface->getCanvas()->setMatrix(m);
}

void CanvasRenderingContext2D::ResetTransform() {
  surface->getCanvas()->resetMatrix();
}

float CanvasRenderingContext2D::MeasureText(const std::string &text) {
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

void CanvasRenderingContext2D::MoveTo(float x, float y) {
  path.moveTo(x, y);
}

void CanvasRenderingContext2D::LineTo(float x, float y) {
  path.lineTo(x, y);
}

void CanvasRenderingContext2D::Arc(float x, float y, float radius, float startAngle, float endAngle, float anticlockwise) {
  if (anticlockwise) {
    float temp = startAngle;
    startAngle = endAngle;
    endAngle = temp;
  }
  path.addArc(SkRect::MakeLTRB(x - radius, y - radius, x + radius, y + radius), startAngle / M_PI * 360, endAngle / M_PI * 360);
}

void CanvasRenderingContext2D::ArcTo(float x1, float y1, float x2, float y2, float radius) {
  path.arcTo(x1, y1, x2 - x1, y2 - y1, radius);
}

void CanvasRenderingContext2D::Rect(float x, float y, float w, float h) {
  path.addRect(SkRect::MakeXYWH(x, y, w, h));
}

void CanvasRenderingContext2D::FillRect(float x, float y, float w, float h) {
  SkPath path;
  path.addRect(SkRect::MakeXYWH(x, y, w, h));
  surface->getCanvas()->drawPath(path, fillPaint);
}

void CanvasRenderingContext2D::StrokeRect(float x, float y, float w, float h) {
  SkPath path;
  path.addRect(SkRect::MakeXYWH(x, y, w, h));
  surface->getCanvas()->drawPath(path, strokePaint);
}

void CanvasRenderingContext2D::ClearRect(float x, float y, float w, float h) {
  SkPath path;
  path.addRect(SkRect::MakeXYWH(x, y, w, h));
  surface->getCanvas()->drawPath(path, clearPaint);
}

float getFontBaseline(const SkPaint::FontMetrics &fontMetrics, const TextBaseline &textBaseline, float lineHeight) {
  // If the font is so tiny that the lroundf operations result in two
  // different types of text baselines to return the same baseline, use
  // floating point metrics (crbug.com/338908).
  // If you changed the heuristic here, for consistency please also change it
  // in SimpleFontData::platformInit().
  // TODO(fserb): revisit this.
  switch (textBaseline) {
    case TextBaseline::TOP:
      return fontMetrics.fAscent;
    case TextBaseline::HANGING:
      // According to
      // http://wiki.apache.org/xmlgraphics-fop/LineLayout/AlignmentHandling
      // "FOP (Formatting Objects Processor) puts the hanging baseline at 80% of
      // the ascender height"
      return fontMetrics.fAscent * 80.0f / 100.0f;
    case TextBaseline::BOTTOM:
    case TextBaseline::IDEOGRAPHIC:
      return -fontMetrics.fDescent;
    case TextBaseline::MIDDLE:
      return -fontMetrics.fDescent + fontMetrics.fCapHeight * lineHeight / 2.0f;
    case TextBaseline::ALPHABETIC:
    default:
      // Do nothing.
      break;
  }
  return 0;
}

void CanvasRenderingContext2D::FillText(const std::string &text, float x, float y) {
  surface->getCanvas()->drawText(text.c_str(), text.length(), x, y - getFontBaseline(fontMetrics, textBaseline, lineHeight), fillPaint);
}

void CanvasRenderingContext2D::StrokeText(const std::string &text, float x, float y) {
  surface->getCanvas()->drawText(text.c_str(), text.length(), x, y - getFontBaseline(fontMetrics, textBaseline, lineHeight), strokePaint);
}

void CanvasRenderingContext2D::Resize(unsigned int w, unsigned int h) {
  SkImageInfo info = SkImageInfo::Make(w, h, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
  surface = SkSurface::MakeRaster(info);
  // flipCanvasY(surface->getCanvas());
}

void CanvasRenderingContext2D::DrawImage(const SkImage *image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, bool flipY) {
  if (flipY) {
    surface->getCanvas()->save();
    flipCanvasY(surface->getCanvas(), dy + dh);
  }

  SkPaint paint;
  paint.setColor(0xFFFFFFFF);
  paint.setStyle(SkPaint::kFill_Style);
  paint.setBlendMode(SkBlendMode::kSrcOver);
  surface->getCanvas()->drawImageRect(image, SkRect::MakeXYWH(sx, sy, sw, sh), SkRect::MakeXYWH(dx, surface->getCanvas()->imageInfo().height() - dy - dh, dw, dh), &paint);

  if (flipY) {
    surface->getCanvas()->restore();
  }
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
    Nan::SetAccessor(canvasObj, JS_STR("strokeStyle"), StrokeStyleGetter, StrokeStyleSetter);
    Nan::SetAccessor(canvasObj, JS_STR("fillStyle"), FillStyleGetter, FillStyleSetter);
    Nan::SetAccessor(canvasObj, JS_STR("font"), FontGetter, FontSetter);
    Nan::SetAccessor(canvasObj, JS_STR("fontFamily"), FontFamilyGetter, FontFamilySetter);
    Nan::SetAccessor(canvasObj, JS_STR("fontSize"), FontSizeGetter, FontSizeSetter);
    Nan::SetAccessor(canvasObj, JS_STR("fontSize"), FontVariantGetter, FontVariantSetter);
    Nan::SetAccessor(canvasObj, JS_STR("fontWeight"), FontWeightGetter, FontWeightSetter);
    Nan::SetAccessor(canvasObj, JS_STR("lineHeight"), LineHeightGetter, LineHeightSetter);
    Nan::SetAccessor(canvasObj, JS_STR("fontStyle"), FontStyleGetter, FontStyleSetter);
    Nan::SetAccessor(canvasObj, JS_STR("textAlign"), TextAlignGetter, TextAlignSetter);
    Nan::SetAccessor(canvasObj, JS_STR("textBaseline"), TextBaselineGetter, TextBaselineSetter);
    Nan::SetAccessor(canvasObj, JS_STR("direction"), DirectionGetter, DirectionSetter);

    info.GetReturnValue().Set(canvasObj);
  } else {
    return Nan::ThrowError("CanvasRenderingContext2D: invalid arguments");
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

    SkImageInfo imageInfo = SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
    bool ok = context->surface->getCanvas()->readPixels(imageInfo, arrayBuffer->GetContents().Data(), width * 4, 0, 0);
    if (ok) {
      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      context->dataArray.Reset(uint8ClampedArray);
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

  if (value->IsNumber()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    float lineWidth = value->NumberValue();

    context->strokePaint.setStrokeWidth(lineWidth);
    context->fillPaint.setStrokeWidth(lineWidth);
  } else {
    Nan::ThrowError("lineWidth: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::StrokeStyleGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::StrokeStyleSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    v8::String::Utf8Value text(value);
    std::string strokeStyle(*text, text.length());

    canvas::web_color webColor = canvas::web_color::from_string(strokeStyle.c_str());
    context->strokePaint.setColor(((uint32_t)webColor.a << (8 * 3)) | ((uint32_t)webColor.r << (8 * 2)) | ((uint32_t)webColor.g << (8 * 1)) | ((uint32_t)webColor.b << (8 * 0)));
    context->fillPaint.setShader(nullptr);
  } else if (value->IsObject() && value->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("CanvasGradient"))) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    CanvasGradient *canvasGradient = ObjectWrap::Unwrap<CanvasGradient>(Local<Object>::Cast(value));
    context->fillPaint.setShader(canvasGradient->getShader());
  } else {
    Nan::ThrowError("strokeStyle: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::FillStyleGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FillStyleSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    v8::String::Utf8Value text(value);
    std::string fillStyle(*text, text.length());

    canvas::web_color webColor = canvas::web_color::from_string(fillStyle.c_str());
    context->fillPaint.setColor(((uint32_t)webColor.a << (8 * 3)) | ((uint32_t)webColor.r << (8 * 2)) | ((uint32_t)webColor.g << (8 * 1)) | ((uint32_t)webColor.b << (8 * 0)));
    context->fillPaint.setShader(nullptr);
  } else if (value->IsObject() && value->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("CanvasGradient"))) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    CanvasGradient *canvasGradient = ObjectWrap::Unwrap<CanvasGradient>(Local<Object>::Cast(value));
    context->fillPaint.setShader(canvasGradient->getShader());
  } else {
     Nan::ThrowError("fillStyle: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::FontGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FontSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    Local<Object> contextObj = info.This();

    v8::String::Utf8Value text(value);
    std::string font(*text, text.length());

    canvas::FontDeclaration declaration = canvas::parse_short_font(font);

    contextObj->Set(JS_STR("fontFamily"), JS_STR(declaration.fontFamily));
    contextObj->Set(JS_STR("fontStyle"), JS_STR(declaration.fontStyle));
    contextObj->Set(JS_STR("fontVariant"), JS_STR(declaration.fontVariant));
    contextObj->Set(JS_STR("fontWeight"), JS_STR(declaration.fontWeight));
    contextObj->Set(JS_STR("fontSize"), JS_STR(declaration.fontSize));
    contextObj->Set(JS_STR("lineHeight"), JS_STR(declaration.lineHeight));
  } else {
    Nan::ThrowError("font: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::FontFamilyGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FontFamilySetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    v8::String::Utf8Value text(value);
    std::string fontFamily(*text, text.length());

    SkTypeface *typeface = context->strokePaint.getTypeface();
    SkFontStyle fontStyle = typeface ? typeface->fontStyle() : SkFontStyle();
    context->strokePaint.setTypeface(SkTypeface::MakeFromName(fontFamily.c_str(), fontStyle));
    context->fillPaint.setTypeface(SkTypeface::MakeFromName(fontFamily.c_str(), fontStyle));
    context->strokePaint.getFontMetrics(&context->fontMetrics);
  } else {
    Nan::ThrowError("fontFamily: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::FontSizeGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FontSizeSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber() || value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    double fontSize = value->NumberValue();

    context->strokePaint.setTextSize(fontSize);
    context->fillPaint.setTextSize(fontSize);
    context->strokePaint.getFontMetrics(&context->fontMetrics);
  } else {
    Nan::ThrowError("fontSize: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::FontWeightGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FontWeightSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber() || value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    v8::String::Utf8Value text(value);
    std::string fontStyleString(*text, text.length());

    unsigned int fontWeight;
    if (fontStyleString == "normal") {
      fontWeight = 400;
    } else if (fontStyleString == "bold") {
      fontWeight = 700;
    } else {
      fontWeight = value->IsNumber() ? value->Uint32Value() : 400;
    }

    SkTypeface *typeface = context->strokePaint.getTypeface();
    SkFontStyle oldFontStyle = typeface ? typeface->fontStyle() : SkFontStyle();
    SkFontStyle fontStyle(fontWeight, oldFontStyle.width(), oldFontStyle.slant());

    const char *familyNameString;
    if (typeface) {
      SkString familyName;
      typeface->getFamilyName(&familyName);
      familyNameString = familyName.c_str();
    } else {
      familyNameString = nullptr;
    }
    context->strokePaint.setTypeface(SkTypeface::MakeFromName(familyNameString, fontStyle));
    context->fillPaint.setTypeface(SkTypeface::MakeFromName(familyNameString, fontStyle));
    context->strokePaint.getFontMetrics(&context->fontMetrics);
  } else {
    Nan::ThrowError("fontWeight: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::LineHeightGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::LineHeightSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber() || value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());

    double lineHeight = value->NumberValue();

    context->lineHeight = lineHeight;
  } else {
    Nan::ThrowError("lineHeight: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::FontStyleGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FontStyleSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
    v8::String::Utf8Value text(value);
    std::string fontStyleString(*text, text.length());

    SkFontStyle::Slant slant;
    if (fontStyleString == "normal") {
      slant = SkFontStyle::Slant::kUpright_Slant;
    } else if (fontStyleString == "italic") {
      slant = SkFontStyle::Slant::kItalic_Slant;
    } else if (fontStyleString == "oblique") {
      slant = SkFontStyle::Slant::kOblique_Slant;
    } else {
      slant = SkFontStyle::Slant::kUpright_Slant;
    }

    SkTypeface *typeface = context->strokePaint.getTypeface();
    SkFontStyle oldFontStyle = typeface ? typeface->fontStyle() : SkFontStyle();
    SkFontStyle fontStyle(oldFontStyle.weight(), oldFontStyle.width(), slant);

    const char *familyNameString;
    if (typeface) {
      SkString familyName;
      typeface->getFamilyName(&familyName);
      familyNameString = familyName.c_str();
    } else {
      familyNameString = nullptr;
    }
    context->strokePaint.setTypeface(SkTypeface::MakeFromName(familyNameString, fontStyle));
    context->fillPaint.setTypeface(SkTypeface::MakeFromName(familyNameString, fontStyle));
    context->strokePaint.getFontMetrics(&context->fontMetrics);
  } else {
    Nan::ThrowError("fontStyle: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::FontVariantGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::FontVariantSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    // TODO
  } else {
    Nan::ThrowError("fontStyle: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::TextAlignGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::TextAlignSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
    v8::String::Utf8Value text(value);
    std::string textAlignString(*text, text.length());

    if (textAlignString == "left") {
      context->strokePaint.setTextAlign(SkPaint::kLeft_Align);
      context->fillPaint.setTextAlign(SkPaint::kLeft_Align);
    } else if (textAlignString == "right") {
      context->strokePaint.setTextAlign(SkPaint::kRight_Align);
      context->fillPaint.setTextAlign(SkPaint::kRight_Align);
    } else if (textAlignString == "center") {
      context->strokePaint.setTextAlign(SkPaint::kCenter_Align);
      context->fillPaint.setTextAlign(SkPaint::kCenter_Align);
    } else if (textAlignString == "start") {
      if (context->direction == Direction::LEFT_TO_RIGHT) {
        context->strokePaint.setTextAlign(SkPaint::kLeft_Align);
        context->fillPaint.setTextAlign(SkPaint::kLeft_Align);
      } else {
        context->strokePaint.setTextAlign(SkPaint::kRight_Align);
        context->fillPaint.setTextAlign(SkPaint::kRight_Align);
      }
    } else if (textAlignString == "end") {
      if (context->direction == Direction::LEFT_TO_RIGHT) {
        context->strokePaint.setTextAlign(SkPaint::kRight_Align);
        context->fillPaint.setTextAlign(SkPaint::kRight_Align);
      } else {
        context->strokePaint.setTextAlign(SkPaint::kLeft_Align);
        context->fillPaint.setTextAlign(SkPaint::kLeft_Align);
      }
    } else {
      context->textAlign = "";
    }
  } else {
    Nan::ThrowError("textAlign: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::TextBaselineGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::TextBaselineSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
    v8::String::Utf8Value text(value);
    std::string textBaselineString(*text, text.length());

    if (textBaselineString == "top") {
      context->textBaseline = TextBaseline::TOP;
    } else if (textBaselineString == "hanging") {
      context->textBaseline = TextBaseline::HANGING;
    } else if (textBaselineString == "alphabetic") {
      context->textBaseline = TextBaseline::ALPHABETIC;
    } else if (textBaselineString == "ideographic") {
      context->textBaseline = TextBaseline::IDEOGRAPHIC;
    } else if (textBaselineString == "bottom") {
      context->textBaseline = TextBaseline::BOTTOM;
    } else {
      context->textBaseline = TextBaseline::ALPHABETIC;
    }
  } else {
    Nan::ThrowError("textBaseline: invalid arguments");
  }
}

NAN_GETTER(CanvasRenderingContext2D::DirectionGetter) {
  // nothing
}

NAN_SETTER(CanvasRenderingContext2D::DirectionSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(info.This());
    v8::String::Utf8Value text(value);
    std::string direction(*text, text.length());

    if (direction == "ltr") {
      context->direction = Direction::LEFT_TO_RIGHT;
    } else if (direction == "rtl") {
      context->direction = Direction::RIGHT_TO_LEFT;
    } else {
      context->direction = Direction::LEFT_TO_RIGHT;
    }
  } else {
    Nan::ThrowError("direction: invalid arguments");
  }
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

NAN_METHOD(CanvasRenderingContext2D::CreateLinearGradient) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
    Local<Object> contextObj = Local<Object>::Cast(info.This());
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(contextObj);

    Local<Function> canvasGradientCons = Local<Function>::Cast(contextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("CanvasGradient")));
    Local<Value> argv[] = {
      info[0],
      info[1],
      info[2],
      info[3],
    };
    Local<Object> canvasGradientObj = canvasGradientCons->NewInstance(sizeof(argv)/sizeof(argv[0]), argv);
    info.GetReturnValue().Set(canvasGradientObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(CanvasRenderingContext2D::CreateRadialGradient) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() && info[4]->IsNumber() && info[5]->IsNumber()) {
    Local<Object> contextObj = Local<Object>::Cast(info.This());
    CanvasRenderingContext2D *context = ObjectWrap::Unwrap<CanvasRenderingContext2D>(contextObj);

    Local<Function> canvasGradientCons = Local<Function>::Cast(contextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("CanvasGradient")));
    Local<Value> argv[] = {
      info[0],
      info[1],
      info[2],
      info[3],
      info[4],
      info[5],
    };
    Local<Object> canvasGradientObj = canvasGradientCons->NewInstance(sizeof(argv)/sizeof(argv[0]), argv);
    info.GetReturnValue().Set(canvasGradientObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
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

      int x = info[1]->Int32Value();
      int y = info[2]->Int32Value();
      if (info.Length() > 3) {
        if (info.Length() > 5) {
          unsigned int sw = info[3]->Uint32Value();
          unsigned int sh = info[4]->Uint32Value();
          unsigned int dx = info[5]->Uint32Value();
          unsigned int dy = info[6]->Uint32Value();
          unsigned int dw = info[7]->Uint32Value();
          unsigned int dh = info[8]->Uint32Value();

          context->DrawImage(image.get(), x, y, sw, sh, dx, dy, dw, dh, false);
        } else {
          unsigned int dw = info[3]->Uint32Value();
          unsigned int dh = info[4]->Uint32Value();
          unsigned int sw = otherContext->GetWidth();
          unsigned int sh = otherContext->GetHeight();

          context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh, false);
        }
      } else {
        unsigned int sw = otherContext->GetWidth();
        unsigned int sh = otherContext->GetHeight();
        unsigned int dw = sw;
        unsigned int dh = sh;

        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh, false);
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
        unsigned int dx = info[5]->Uint32Value();
        unsigned int dy = info[6]->Uint32Value();
        unsigned int dw = info[7]->Uint32Value();
        unsigned int dh = info[8]->Uint32Value();

        context->DrawImage(image->image.get(), x, y, sw, sh, dx, dy, dw, dh, false);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = image->GetWidth();
        unsigned int sh = image->GetHeight();

        context->DrawImage(image->image.get(), 0, 0, sw, sh, x, y, dw, dh, false);
      }
    } else {
      unsigned int sw = image->GetWidth();
      unsigned int sh = image->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(image->image.get(), 0, 0, sw, sh, x, y, dw, dh, false);
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
        unsigned int dx = info[5]->Uint32Value();
        unsigned int dy = info[6]->Uint32Value();
        unsigned int dw = info[7]->Uint32Value();
        unsigned int dh = info[8]->Uint32Value();

        context->DrawImage(image.get(), x, y, sw, sh, dx, dy, dw, dh, false);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = imageData->GetWidth();
        unsigned int sh = imageData->GetHeight();

        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh, false);
      }
    } else {
      unsigned int sw = imageData->GetWidth();
      unsigned int sh = imageData->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh, false);
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
        unsigned int dx = info[5]->Uint32Value();
        unsigned int dy = info[6]->Uint32Value();
        unsigned int dw = info[7]->Uint32Value();
        unsigned int dh = info[8]->Uint32Value();

        context->DrawImage(image.get(), x, y, sw, sh, dx, dy, dw, dh, true);
      } else {
        unsigned int dw = info[3]->Uint32Value();
        unsigned int dh = info[4]->Uint32Value();
        unsigned int sw = imageBitmap->GetWidth();
        unsigned int sh = imageBitmap->GetHeight();

        context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh, true);
      }
    } else {
      unsigned int sw = imageBitmap->GetWidth();
      unsigned int sh = imageBitmap->GetHeight();
      unsigned int dw = sw;
      unsigned int dh = sh;

      context->DrawImage(image.get(), 0, 0, sw, sh, x, y, dw, dh, true);
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

  bool ok = context->surface->getCanvas()->readPixels(imageData->bitmap, x, y);
  if (ok) {
    return info.GetReturnValue().Set(imageDataObj);
  } else {
    return Nan::ThrowError("Failed to get pixels");
  }
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

    context->surface->getCanvas()->save();
    flipCanvasY(context->surface->getCanvas(), y + dh);

    sk_sp<SkImage> image = SkImage::MakeFromBitmap(imageData->bitmap);
    context->DrawImage(image.get(), dirtyX, dirtyY, dirtyWidth, dirtyHeight, x, context->surface->getCanvas()->imageInfo().height() - y - dh, dw, dh, false);

    context->surface->getCanvas()->restore();
  } else {
    unsigned int sw = imageData->GetWidth();
    unsigned int sh = imageData->GetHeight();
    unsigned int dw = sw;
    unsigned int dh = sh;

    context->surface->getCanvas()->save();
    flipCanvasY(context->surface->getCanvas(), y + dh);

    sk_sp<SkImage> image = SkImage::MakeFromBitmap(imageData->bitmap);
    context->DrawImage(image.get(), 0, 0, sw, sh, x, context->surface->getCanvas()->imageInfo().height() - y - dh, dw, dh, false);

    context->surface->getCanvas()->restore();
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
  SkImageInfo info = SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
  surface = SkSurface::MakeRaster(info); // XXX can optimize this to not allocate until a width/height is set
  // flipCanvasY(surface->getCanvas());

  strokePaint.setTextSize(12);
  strokePaint.setStyle(SkPaint::kStroke_Style);
  strokePaint.setBlendMode(SkBlendMode::kSrcOver);

  fillPaint.setTextSize(12);
  fillPaint.setStyle(SkPaint::kFill_Style);
  fillPaint.setBlendMode(SkBlendMode::kSrcOver);

  clearPaint.setTextSize(12);
  clearPaint.setColor(0x0);
  clearPaint.setStyle(SkPaint::kFill_Style);
  clearPaint.setBlendMode(SkBlendMode::kSrc);

  lineHeight = 1;

  strokePaint.getFontMetrics(&fontMetrics);
}
CanvasRenderingContext2D::~CanvasRenderingContext2D () {}

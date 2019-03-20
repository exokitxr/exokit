#include <canvascontext/include/path2d-context.h>

using namespace v8;
using namespace node;
// using namespace std;

Local<Object> Path2D::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Path2D"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto,"moveTo", MoveTo);
  Nan::SetMethod(proto,"lineTo", LineTo);
  Nan::SetMethod(proto,"closePath", ClosePath);
  Nan::SetMethod(proto,"arc", Arc);
  Nan::SetMethod(proto,"arcTo", ArcTo);
  Nan::SetMethod(proto,"quadraticCurveTo", QuadraticCurveTo);
  Nan::SetMethod(proto,"clear", Clear);

  return scope.Escape(Nan::GetFunction(ctor).ToLocalChecked());
}

void Path2D::MoveTo(float x, float y) {
  path.moveTo(x, y);
}
void Path2D::LineTo(float x, float y) {
  path.lineTo(x, y);
}
void Path2D::ClosePath() {
  path.close();
}
void Path2D::Arc(float x, float y, float radius, float startAngle, float endAngle, float anticlockwise) {
  if (anticlockwise) {
    float temp = startAngle;
    startAngle = endAngle;
    endAngle = temp;
  }
  path.addArc(SkRect::MakeLTRB(x - radius/2, y - radius/2, x + radius/2, y + radius/2), startAngle, endAngle);
}
void Path2D::ArcTo(float x1, float y1, float x2, float y2, float radius) {
  path.arcTo(x1, y1, x2 - x1, y2 - y1, radius);
}
void Path2D::QuadraticCurveTo(float cpx, float cpy, float x, float y) {
  path.quadTo(cpx, cpy, x, y);
}
void Path2D::Clear() {
  path.reset();
}

NAN_METHOD(Path2D::New) {
  Nan::HandleScope scope;

  Path2D *path2d = new Path2D();
  path2d->Wrap(info.This());
  // registerImage(image);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Path2D::MoveTo) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double x = TO_DOUBLE(info[0]);
  double y = TO_DOUBLE(info[1]);

  path2d->MoveTo(x, y);
}

NAN_METHOD(Path2D::LineTo) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double x = TO_DOUBLE(info[0]);
  double y = TO_DOUBLE(info[1]);

  path2d->LineTo(x, y);
}

NAN_METHOD(Path2D::ClosePath) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());

  path2d->ClosePath();
}

NAN_METHOD(Path2D::Arc) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double x = TO_DOUBLE(info[0]);
  double y = TO_DOUBLE(info[1]);
  double radius = TO_DOUBLE(info[2]);
  double startAngle = TO_DOUBLE(info[3]);
  double endAngle = TO_DOUBLE(info[4]);
  double anticlockwise = TO_DOUBLE(info[5]);

  path2d->Arc(x, y, radius, startAngle, endAngle, anticlockwise);
}

NAN_METHOD(Path2D::ArcTo) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double x1 = TO_DOUBLE(info[0]);
  double y1 = TO_DOUBLE(info[1]);
  double x2 = TO_DOUBLE(info[2]);
  double y2 = TO_DOUBLE(info[3]);
  double radius = TO_DOUBLE(info[4]);

  path2d->ArcTo(x1, y1, x2, y2, radius);
}

NAN_METHOD(Path2D::QuadraticCurveTo) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double cpx = TO_DOUBLE(info[0]);
  double cpy = TO_DOUBLE(info[1]);
  double x = TO_DOUBLE(info[2]);
  double y = TO_DOUBLE(info[3]);

  path2d->QuadraticCurveTo(cpx, cpy, x, y);
}

NAN_METHOD(Path2D::Clear) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());

  path2d->Clear();
}

Path2D::Path2D() {}
Path2D::~Path2D () {}

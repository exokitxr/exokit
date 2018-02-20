#include <canvascontext/include/path2d-context.h>

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> Path2D::Initialize(Isolate *isolate) {
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

  return scope.Escape(ctor->GetFunction());
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
  path.addArc({x - radius/2, y - radius/2, x + radius/2, y + radius/2}, startAngle, endAngle);
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
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();

  path2d->MoveTo(x, y);
}

NAN_METHOD(Path2D::LineTo) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();

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
  double x = info[0]->NumberValue();
  double y = info[1]->NumberValue();
  double radius = info[2]->NumberValue();
  double startAngle = info[3]->NumberValue();
  double endAngle = info[4]->NumberValue();
  double anticlockwise = info[5]->NumberValue();

  path2d->Arc(x, y, radius, startAngle, endAngle, anticlockwise);
}

NAN_METHOD(Path2D::ArcTo) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double x1 = info[0]->NumberValue();
  double y1 = info[1]->NumberValue();
  double x2 = info[2]->NumberValue();
  double y2 = info[3]->NumberValue();
  double radius = info[4]->NumberValue();

  path2d->ArcTo(x1, y1, x2, y2, radius);
}

NAN_METHOD(Path2D::QuadraticCurveTo) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());
  double cpx = info[0]->NumberValue();
  double cpy = info[1]->NumberValue();
  double x = info[2]->NumberValue();
  double y = info[3]->NumberValue();

  path2d->QuadraticCurveTo(cpx, cpy, x, y);
}

NAN_METHOD(Path2D::Clear) {
  Nan::HandleScope scope;

  Path2D *path2d = ObjectWrap::Unwrap<Path2D>(info.This());

  path2d->Clear();
}

Path2D::Path2D() {}
Path2D::~Path2D () {}

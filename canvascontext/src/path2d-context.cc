#include <canvascontext/include/path2d-context.h>

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> Path2D::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  // Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Path2D"));

  // prototype
  // Nan::SetPrototypeMethod(ctor, "save",save);// NODE_SET_PROTOTYPE_METHOD(ctor, "save", save);
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto,"moveTo", MoveTo);
  Nan::SetMethod(proto,"lineTo", LineTo);
  Nan::SetMethod(proto,"closePath", ClosePath);
  Nan::SetMethod(proto,"arc", Arc);
  Nan::SetMethod(proto,"arcTo", ArcTo);
  Nan::SetMethod(proto,"quadraticCurveTo", QuadraticCurveTo);
  Nan::SetMethod(proto,"clear", Clear);
  // Nan::SetAccessor(proto,JS_STR("src"), SrcGetter, SrcSetter);
  // Nan::Set(target, JS_STR("Image"), ctor->GetFunction());

  // constructor_template.Reset(Isolate::GetCurrent(), ctor->GetFunction());

  return scope.Escape(ctor->GetFunction());
}

void Path2D::MoveTo(double x, double y) {
  path2d->moveTo(canvas::Point(x, y));
}
void Path2D::LineTo(double x, double y) {
  path2d->lineTo(canvas::Point(x, y));
}
void Path2D::ClosePath() {
  path2d->closePath();
}
void Path2D::Arc(double x, double y, double radius, double startAngle, double endAngle, double anticlockwise) {
  path2d->arc(canvas::Point(x, y), radius, startAngle, endAngle, anticlockwise);
}
void Path2D::ArcTo(double x1, double y1, double x2, double y2, double radius) {
  path2d->arcTo(canvas::Point(x1, y1), canvas::Point(x2, y2), radius);
}
void Path2D::QuadraticCurveTo(double cpx, double cpy, double x, double y) {
  path2d->quadraticCurveTo(cpx, cpy, x, y, 1);
}
void Path2D::Clear() {
  path2d->clear();
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

Path2D::Path2D() {
  path2d = new canvas::Path2D();
}
Path2D::~Path2D () {
  delete path2d;
}

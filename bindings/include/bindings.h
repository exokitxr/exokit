#ifndef _EXOKIT_BINDINGS_H_
#define _EXOKIT_BINDINGS_H_

#include <v8.h>
#include <node.h>
#include <image-context.h>
#include <imageData-context.h>
#include <imageBitmap-context.h>
#include <canvas-context.h>
#include <path2d-context.h>
#include <webgl.h>
#include <AudioContext.h>

v8::Local<v8::Object> makeGl();
v8::Local<v8::Object> makeImage();
v8::Local<v8::Object> makeImageData();
v8::Local<v8::Object> makeImageBitmap(Local<Value> imageCons);
v8::Local<v8::Object> makeCanvasRenderingContext2D(Local<Value> imageDataCons);
v8::Local<v8::Object> makePath2D();
v8::Local<v8::Object> makeAudio();

#endif

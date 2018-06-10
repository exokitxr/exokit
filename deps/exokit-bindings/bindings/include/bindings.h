#ifndef _EXOKIT_BINDINGS_H_
#define _EXOKIT_BINDINGS_H_

#include <v8.h>
#include <node.h>
#include <image-context.h>
#include <imageData-context.h>
#include <imageBitmap-context.h>
#include <canvas-context.h>
#include <path2d-context.h>
#include <canvas-gradient.h>
#include <canvas-pattern.h>
#include <glfw.h>
#include <webgl.h>
#include <AudioContext.h>
#include <Video.h>
#if _WIN32
#include <leapmotion.h>
#endif
#ifdef MAGICLEAP
#include <magicleap.h>
#endif

v8::Local<v8::Object> makeGl();
v8::Local<v8::Object> makeImage();
v8::Local<v8::Object> makeImageData();
v8::Local<v8::Object> makeImageBitmap();
v8::Local<v8::Object> makeCanvasRenderingContext2D(Local<Value> imageDataCons, Local<Value> canvasGradientCons, Local<Value> canvasPatternCons);
v8::Local<v8::Object> makePath2D();
v8::Local<v8::Object> makeCanvasGradient();
v8::Local<v8::Object> makeCanvasPattern();
v8::Local<v8::Object> makeAudio();
v8::Local<v8::Object> makeVideo(Local<Value> imageDataCons);

#endif

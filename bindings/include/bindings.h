#ifndef BINDINGS_H_
#define BINDINGS_H_

#include <v8.h>
#include <node.h>
#include <image.h>
#include <imageData.h>
#include <imageBitmap.h>
#include <canvas.h>
#include <path2d.h>
#include <webgl.h>

v8::Local<v8::Object> makeGl(node::NodeService *service);
v8::Local<v8::Object> makeImage(node::NodeService *service, canvas::ContextFactory *canvasContextFactory);
v8::Local<v8::Object> makeImageData(node::NodeService *service);
v8::Local<v8::Object> makeImageBitmap(node::NodeService *service);
v8::Local<v8::Object> makeCanvasRenderingContext2D(node::NodeService *service, canvas::ContextFactory *canvasContextFactory, Local<Value> imageDataCons);
v8::Local<v8::Object> makePath2D(node::NodeService *service);

#endif

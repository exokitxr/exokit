#ifndef _WINDOWSYSTEM_H_
#define _WINDOWSYSTEM_H_

#include <webgl.h>

#include <iostream>
#include <vector>

#include <v8.h>
#include <nan.h>
#include <defines.h>

using namespace v8;

// class WebGLRenderingContext;
typedef unsigned int GLuint;
typedef int GLint;

namespace windowsystembase {

class BlitSpec {
public:
  GLuint fbo;
  GLuint msFbo;
  int width;
  int height;
};

class LayerSpec {
public:
  GLuint colorTex;
  GLuint depthTex;
  std::unique_ptr<BlitSpec> blitSpec;
};

void Initialize();
void Decorate(Local<Object> target);
void Compose(WebGLRenderingContext *gl, const std::vector<LayerSpec> &layers);  
NAN_METHOD(Compose);

extern GLuint composeVao;
extern GLuint composeFbo;
extern GLuint composeProgram;
extern GLint positionLocation;
extern GLint uvLocation;
extern GLint colorTexLocation;
extern GLint depthTexLocation;
extern GLuint positionBuffer;
extern GLuint uvBuffer;

}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
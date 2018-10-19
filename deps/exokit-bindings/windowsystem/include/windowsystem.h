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

class ComposeSpec {
public:
  GLuint composeVao;
  // GLuint composeFbo;
  GLuint composeProgram;
  GLint positionLocation;
  GLint uvLocation;
  GLint colorTexLocation;
  GLint depthTexLocation;
  GLuint positionBuffer;
  GLuint uvBuffer;
};

void InitializeLocalGlState(WebGLRenderingContext *gl);
void ComposeLayers(WebGLRenderingContext *gl, const std::vector<LayerSpec> &layers);  
NAN_METHOD(ComposeLayers);
void Decorate(Local<Object> target);

}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
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

class LayerSpec {
public:
  int width;
  int height;
  GLuint msTex;
  GLuint msDepthTex;
  GLuint tex;
  GLuint depthTex;
  float *viewports[2];
  float *modelView[2];
  float *projection[2];
};

class ComposeSpec {
public:
  GLuint composeVao;
  GLuint composeReadFbo;
  GLuint composeWriteFbo;
  GLuint composeProgram;
  GLint positionLocation;
  GLint uvLocation;
  GLint msTexLocation;
  GLint msDepthTexLocation;
  GLint texSizeLocation;
  GLuint positionBuffer;
  GLuint uvBuffer;
  GLuint indexBuffer;
};

class PlaneSpec {
public:
  GLuint planeVao;
  GLuint planeProgram;
  GLint positionLocation;
  GLint uvLocation;
  GLint modelViewMatrixLocation;
  GLint projectionMatrixLocation;
  GLint texLocation;
  GLuint positionBuffer;
  GLuint uvBuffer;
  GLuint indexBuffer;
};

void InitializeLocalGlState(WebGLRenderingContext *gl);
bool CreateRenderTarget(WebGLRenderingContext *gl, int width, int height, GLuint sharedColorTex, GLuint sharedDepthStencilTex, GLuint sharedMsColorTex, GLuint sharedMsDepthStencilTex, GLuint *pfbo, GLuint *pcolorTex, GLuint *pdepthStencilTex, GLuint *pmsFbo, GLuint *pmsColorTex, GLuint *pmsDepthStencilTex);
NAN_METHOD(CreateRenderTarget);
NAN_METHOD(ResizeRenderTarget);
NAN_METHOD(DestroyRenderTarget);
void ComposeLayers(WebGLRenderingContext *gl, const std::vector<LayerSpec> &layers);
NAN_METHOD(ComposeLayers);
void Decorate(Local<Object> target);

}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
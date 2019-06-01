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
  
enum class LayerType {
  NONE,
  IFRAME_3D,
  IFRAME_3D_REPROJECT,
  IFRAME_2D,
  RAW_CANVAS,
};

class LayerSpec {
public:
  LayerType layerType;
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

class ComposeGlShader : public GlShader {
public:
  ComposeGlShader();
  virtual ~ComposeGlShader();

  static GlKey key;

  GLuint blitFbos[2];
  GLuint composeVao;
  GLuint composeProgram;
  GLint positionLocation;
  GLint uvLocation;
  GLint texLocation;
  GLint depthTexLocation;
  GLuint positionBuffer;
  GLuint uvBuffer;
  GLuint indexBuffer;
};

class PlaneGlShader : public GlShader {
public:
  PlaneGlShader();
  virtual ~PlaneGlShader();

  static GlKey key;

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

bool CreateRenderTarget(WebGLRenderingContext *gl, int width, int height, GLuint sharedColorTex, GLuint sharedDepthStencilTex, GLuint sharedMsColorTex, GLuint sharedMsDepthStencilTex, GLuint *pfbo, GLuint *pcolorTex, GLuint *pdepthStencilTex, GLuint *pmsFbo, GLuint *pmsColorTex, GLuint *pmsDepthStencilTex);
NAN_METHOD(CreateRenderTarget);
NAN_METHOD(ResizeRenderTarget);
NAN_METHOD(DestroyRenderTarget);
NAN_METHOD(GetSync);
NAN_METHOD(WaitSync);
NAN_METHOD(DeleteSync);
void ComposeLayers(WebGLRenderingContext *gl, const std::vector<LayerSpec> &layers);
NAN_METHOD(ComposeLayers);
uv_loop_t *GetEventLoop();
// NAN_METHOD(GetEventLoop);
void Decorate(Local<Object> target);

}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif

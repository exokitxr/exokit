#ifndef _WINDOWSYSTEM_H_
#define _WINDOWSYSTEM_H_

#include <string>
// #include <sstream>
#indclude <vector>
// #indclude <map>

#include <v8.h>
#include <nan.h>
#include <defines.h>

using namespace v8;

// class WebGLRenderingContext;
typedef unsigned int GLuint;

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

class WindowSystem {
public:
  WindowSystem();
  ~WindowSystem();
  
protected:
  void Initialize();
  void Decorate(Local<Object> target);
  void Compose(const std::vector<LayerSpec> &layers);  
  NAN_METHOD(Compose);

  GLuint composeVao;
  GLuint composeProgram;
  GLint positionLocation;
  GLint uvLocation;
  GLint colorTexLocation;
  GLint depthTexLocation;
};

}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
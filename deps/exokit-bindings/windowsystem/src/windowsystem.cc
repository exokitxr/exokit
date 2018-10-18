#include <windowsystem.h>

namespace windowsystembase {

float positions[] = {
  -1, 1, 0,
  1, 1, 0,
  -1, -1, 0,
  1, -1, 0,
};
float uvs[] = {
  0, 1,
  1, 1,
  0, 0,
  1, 0,
};

const char *composeVsh = "\
in vec3 position;\n\
in vec2 uv;\n\
out vec2 vUv;\n\
\n\
void main() {\n\
  vUv = uv;\n\
  gl_Position = vec4(position.xy, 0., 1.);\n\
}\n\
";
const char *composeFsh = "\
in vec2 vUv;\n\
out vec4 fragColor;\n\
uniform sampler2D colorTex;\n\
uniform sampler2D depthTex;\n\
\n\
void main() {\n\
  fragColor = texture2D(colorTex, vUv);\n\
  gl_FragDepth = texture2D(depthTex, vUv).r;\n\
}\n\
";

// WindowSystem

WindowSystem::WindowSystem() {}

WindowSystem::~WindowSystem() {}

void WindowSystem::Initialize() {
  // compose shader

  glGenVertexArrays(1, &this->composeVao);
  glBindVertexArray(this->composeVao);

  // vertex Shader
  GLuint composeVertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(composeVertex, 1, &composeVsh, NULL);
  glCompileShader(composeVertex);
  GLint success;
  glGetShaderiv(composeVertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(composeVertex, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    std::cout << "ML compose vertex shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // fragment Shader
  GLuint composeFragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(composeFragment, 1, &composeFsh, NULL);
  glCompileShader(composeFragment);
  glGetShaderiv(composeFragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(composeFragment, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    std::cout << "ML compose fragment shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // shader Program
  this->composeProgram = glCreateProgram();
  glAttachShader(this->composeProgram, composeVertex);
  glAttachShader(this->composeProgram, composeFragment);
  glLinkProgram(this->composeProgram);
  glGetProgramiv(this->composeProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(this->composeProgram, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    std::cout << "ML compose program linking failed\n" << infoLog << std::endl;
    return;
  }

  this->positionLocation = glGetAttribLocation(this->composeProgram, "position");
  if (this->positionLocation == -1) {
    std::cout << "ML compose program failed to get attrib location for 'position'" << std::endl;
    return;
  }
  this->uvLocation = glGetAttribLocation(this->composeProgram, "uv");
  if (this->uvLocation == -1) {
    std::cout << "ML compose program failed to get attrib location for 'uv'" << std::endl;
    return;
  }
  this->colorTexLocation = glGetUniformLocation(this->composeProgram, "colorTex");
  if (this->colorTexLocation == -1) {
    std::cout << "ML compose program failed to get uniform location for 'colorTex'" << std::endl;
    return;
  }
  this->depthTexLocation = glGetUniformLocation(this->composeProgram, "depthTex");
  if (this->depthTexLocation == -1) {
    std::cout << "ML compose program failed to get uniform location for 'depthTex'" << std::endl;
    return;
  }

  // delete the shaders as they're linked into our program now and no longer necessery
  glDeleteShader(composeVertex);
  glDeleteShader(composeFragment);
}

void WindowSystem::Decorate(Local<Object> target) {
  Nan::SetMethod(target, "compose", Compose);
}

void WindowSystem::Compose(const std::vector<LayerSpec> &layers) {

}

NAN_METHOD(WindowSystem::Compose) {
  if (info[0]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[0]);

    std::vector<LayerSpec> layers(array->Size());
    for (size_t i = 0; i < layers.size(); i++) {
      Local<Value> element = array->Get(i);

      if (element->IsObject()) {
        Local<Object> elementObj = Local<Object>::Cast(element);
        Local<Value> colorTexVal = elementObj->Get(JS_STR("colorTex"));
        Local<Value> depthTexVal = elementObj->Get(JS_STR("depthTex"));
        
        if (colorTexVal->IsNumber() && depthTexVal->IsNumber()) {
          GLuint colorTex = colorTexVal->Uint32Value();
          GLuint depthTex = depthTexVal->Uint32Value();
          
          BlitSpec *blitSpec = nullptr;
          Local<Value> blitVal = elementObj->Get(JS_STR("blit"));
          if (blitVal->IsObject()) {
            Local<Value> fboVal = blitVal->Get(JS_STR("fbo"));
            Local<Value> msFboVal = blitVal->Get(JS_STR("msFbo"));
            Local<Value> widthVal = blitVal->Get(JS_STR("width"));
            Local<Value> heightVal = blitVal->Get(JS_STR("height"));
            
            if (fboVal->IsNumber() && msFboVal->IsNumber() && widthVal->IsNumber() && heightVal->IsNumber()) {
              blitSpec = new BlitSpec{
                fbo,
                msFbo,
                width,
                height,
              };
            } else {
              return Nan::ThrowError("WindowSystem::Compose: invalid layer blit");
            }
          }

          layers[i] = LayerSpec{
            colorTex,
            depthTex,
            std::unique_ptr<BlitSpec>(blitSpec),
          };
        }
      } else {
        return Nan::ThrowError("WindowSystem::Compose: invalid layers");
      }
    }
    
    Compose(layers);
  } else {
    Nan::ThrowError("WindowSystem::Compose: invalid arguments");
  }
}

}

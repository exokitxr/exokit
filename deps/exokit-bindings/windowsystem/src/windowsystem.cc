#include <windowsystem.h>

namespace windowsystembase {

const char *composeVsh = "\
in vec2 position;\n\
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

void Initialize() {
  // compose shader

  glGenFramebuffers(1, &composeFbo);

  glGenVertexArrays(1, &composeVao);
  glBindVertexArray(composeVao);

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
  composeProgram = glCreateProgram();
  glAttachShader(composeProgram, composeVertex);
  glAttachShader(composeProgram, composeFragment);
  glLinkProgram(composeProgram);
  glGetProgramiv(composeProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(composeProgram, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    std::cout << "ML compose program linking failed\n" << infoLog << std::endl;
    return;
  }

  positionLocation = glGetAttribLocation(composeProgram, "position");
  if (positionLocation == -1) {
    std::cout << "ML compose program failed to get attrib location for 'position'" << std::endl;
    return;
  }
  uvLocation = glGetAttribLocation(composeProgram, "uv");
  if (uvLocation == -1) {
    std::cout << "ML compose program failed to get attrib location for 'uv'" << std::endl;
    return;
  }
  colorTexLocation = glGetUniformLocation(composeProgram, "colorTex");
  if (colorTexLocation == -1) {
    std::cout << "ML compose program failed to get uniform location for 'colorTex'" << std::endl;
    return;
  }
  depthTexLocation = glGetUniformLocation(composeProgram, "depthTex");
  if (depthTexLocation == -1) {
    std::cout << "ML compose program failed to get uniform location for 'depthTex'" << std::endl;
    return;
  }

  // delete the shaders as they're linked into our program now and no longer necessery
  glDeleteShader(composeVertex);
  glDeleteShader(composeFragment);

  glGenBuffers(1, &positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  static const float positions[] = {
    -1.0f, 1.0f,
    1.0f, 1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(positionLocation);
  glVertexAttribPointer(positionLocation, 2, GL_FLOAT, false, 0, 0);

  glGenBuffers(1, &uvBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
  static const float uvs[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
  glEnableVertexAttribArray(uvLocation);
  glVertexAttribPointer(uvLocation, 2, GL_FLOAT, false, 0, 0);

  // XXX need to do this per gl instance
  /* if (gl->HasVertexArrayBinding()) {
    glBindVertexArray(gl->GetVertexArrayBinding());
  } else {
    glBindVertexArray(gl->defaultVao);
  }
  if (gl->HasProgramBinding()) {
    glUseProgram(gl->GetProgramBinding());
  } else {
    glUseProgram(0);
  }
  if (gl->HasBufferBinding(GL_ARRAY_BUFFER)) {
    glBindBuffer(GL_ARRAY_BUFFER, gl->GetBufferBinding(GL_ARRAY_BUFFER));
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  } */
}

void Decorate(Local<Object> target) {
  Nan::SetMethod(target, "compose", Compose);
}

void Compose(WebGLRenderingContext *gl, const std::vector<LayerSpec> &layers) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, composeFbo);
  glBindVertexArray(composeVao);
  glUseProgram(composeProgram);

  for (size_t i = 0; i < layers.size(); i++) {
    const LayerSpec &layer = layers[i];

    if (layer.blitSpec) {
      const BlitSpec &blitSpec = *layer.blitSpec;

      glBindFramebuffer(GL_READ_FRAMEBUFFER, blitSpec.msFbo);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blitSpec.fbo);

      glBlitFramebuffer(
        0, 0,
        blitSpec.width, blitSpec.height,
        0, 0,
        blitSpec.width, blitSpec.height,
        GL_COLOR_BUFFER_BIT,
        GL_LINEAR);

      glBlitFramebuffer(
        0, 0,
        blitSpec.width, blitSpec.height,
        0, 0,
        blitSpec.width, blitSpec.height,
        GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
        GL_NEAREST);

      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, composeFbo);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, layer.colorTex);
    glUniform1i(colorTexLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, layer.depthTex);
    glUniform1i(depthTexLocation, 1);
  }

  if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasVertexArrayBinding()) {
    glBindVertexArray(gl->GetVertexArrayBinding());
  } else {
    glBindVertexArray(gl->defaultVao);
  }
  if (gl->HasProgramBinding()) {
    glUseProgram(gl->GetProgramBinding());
  } else {
    glUseProgram(0);
  }
  glActiveTexture(GL_TEXTURE0);
  if (gl->HasTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D)) {
    glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glActiveTexture(GL_TEXTURE1);
  if (gl->HasTextureBinding(GL_TEXTURE1, GL_TEXTURE_2D)) {
    glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(GL_TEXTURE1, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glActiveTexture(gl->activeTexture);
}

NAN_METHOD(Compose) {
  if (info[0]->IsObject() && info[1]->IsArray()) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    Local<Array> array = Local<Array>::Cast(info[1]);

    std::vector<LayerSpec> layers(array->Length());
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
            Local<Object> blitObj = Local<Object>::Cast(blitVal);
            Local<Value> fboVal = blitObj->Get(JS_STR("fbo"));
            Local<Value> msFboVal = blitObj->Get(JS_STR("msFbo"));
            Local<Value> widthVal = blitObj->Get(JS_STR("width"));
            Local<Value> heightVal = blitObj->Get(JS_STR("height"));

            if (fboVal->IsNumber() && msFboVal->IsNumber() && widthVal->IsNumber() && heightVal->IsNumber()) {
              GLuint fbo = fboVal->Uint32Value();
              GLuint msFbo = msFboVal->Uint32Value();
              int width = widthVal->Int32Value();
              int height = heightVal->Int32Value();
              
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

    Compose(gl, layers);
  } else {
    Nan::ThrowError("WindowSystem::Compose: invalid arguments");
  }
}

GLuint composeVao;
GLuint composeFbo;
GLuint composeProgram;
GLint positionLocation;
GLint uvLocation;
GLint colorTexLocation;
GLint depthTexLocation;
GLuint positionBuffer;
GLuint uvBuffer;

}

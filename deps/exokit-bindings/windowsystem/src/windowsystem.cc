#include <windowsystem.h>

namespace windowsystembase {

const char *composeVsh = "\
#version 330\n\
\n\
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
#version 330\n\
\n\
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

void InitializeLocalGlState(WebGLRenderingContext *gl) {
  // compose shader
  ComposeSpec *composeSpec = new ComposeSpec();

  glGenFramebuffers(1, &composeSpec->composeFbo);

  glGenVertexArrays(1, &composeSpec->composeVao);
  glBindVertexArray(composeSpec->composeVao);

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
  composeSpec->composeProgram = glCreateProgram();
  glAttachShader(composeSpec->composeProgram, composeVertex);
  glAttachShader(composeSpec->composeProgram, composeFragment);
  glLinkProgram(composeSpec->composeProgram);
  glGetProgramiv(composeSpec->composeProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(composeSpec->composeProgram, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    std::cout << "ML compose program linking failed\n" << infoLog << std::endl;
    return;
  }

  composeSpec->positionLocation = glGetAttribLocation(composeSpec->composeProgram, "position");
  if (composeSpec->positionLocation == -1) {
    std::cout << "ML compose program failed to get attrib location for 'position'" << std::endl;
    return;
  }
  composeSpec->uvLocation = glGetAttribLocation(composeSpec->composeProgram, "uv");
  if (composeSpec->uvLocation == -1) {
    std::cout << "ML compose program failed to get attrib location for 'uv'" << std::endl;
    return;
  }
  composeSpec->colorTexLocation = glGetUniformLocation(composeSpec->composeProgram, "colorTex");
  if (composeSpec->colorTexLocation == -1) {
    std::cout << "ML compose program failed to get uniform location for 'colorTex'" << std::endl;
    return;
  }
  composeSpec->depthTexLocation = glGetUniformLocation(composeSpec->composeProgram, "depthTex");
  if (composeSpec->depthTexLocation == -1) {
    std::cout << "ML compose program failed to get uniform location for 'depthTex'" << std::endl;
    return;
  }

  // delete the shaders as they're linked into our program now and no longer necessery
  glDeleteShader(composeVertex);
  glDeleteShader(composeFragment);

  glGenBuffers(1, &composeSpec->positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, composeSpec->positionBuffer);
  static const float positions[] = {
    -1.0f, 1.0f,
    1.0f, 1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(composeSpec->positionLocation);
  glVertexAttribPointer(composeSpec->positionLocation, 2, GL_FLOAT, false, 0, 0);

  glGenBuffers(1, &composeSpec->uvBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, composeSpec->uvBuffer);
  static const float uvs[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
  glEnableVertexAttribArray(composeSpec->uvLocation);
  glVertexAttribPointer(composeSpec->uvLocation, 2, GL_FLOAT, false, 0, 0);

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
  if (gl->HasBufferBinding(GL_ARRAY_BUFFER)) {
    glBindBuffer(GL_ARRAY_BUFFER, gl->GetBufferBinding(GL_ARRAY_BUFFER));
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

void ComposeLayers(WebGLRenderingContext *gl, const std::vector<LayerSpec> &layers) {
  ComposeSpec *composeSpec = (ComposeSpec *)(gl->keys[GlKey::GL_KEY_COMPOSE]);
  
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, composeSpec->composeFbo);
  glBindVertexArray(composeSpec->composeVao);
  glUseProgram(composeSpec->composeProgram);

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

      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, composeSpec->composeFbo);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, layer.colorTex);
    glUniform1i(composeSpec->colorTexLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, layer.depthTex);
    glUniform1i(composeSpec->depthTexLocation, 1);
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

NAN_METHOD(ComposeLayers) {
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

    ComposeLayers(gl, layers);
  } else {
    Nan::ThrowError("WindowSystem::Compose: invalid arguments");
  }
}

void Decorate(Local<Object> target) {
  Nan::SetMethod(target, "composeLayers", ComposeLayers);
}

}

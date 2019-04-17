#include <windowsystem.h>

#include <exout>

namespace windowsystembase {

const char *composeVsh = ""
#ifdef ANDROID
"#version 300 es\n"
#else
"#version 330\n"
#endif
"\n\
in vec2 position;\n\
in vec2 uv;\n\
out vec2 vUv;\n\
\n\
void main() {\n\
  vUv = uv;\n\
  gl_Position = vec4(position.xy, 0., 1.);\n\
}\n\
";
const char *composeFsh = ""
#ifdef ANDROID
"#version 300 es\n"
#else
"#version 330\n"
#endif
"\n\
in vec2 vUv;\n\
out vec4 fragColor;\n\
int texSamples = 4;\n\
uniform sampler2DMS msTex;\n\
uniform sampler2DMS msDepthTex;\n\
uniform vec4 texSize;\n\
\n\
vec4 textureMultisample(sampler2DMS sampler, vec2 uv) {\n\
  ivec2 iUv = ivec2((uv + texSize.xy) * texSize.zw);\n\
\n\
  vec4 color = vec4(0.0);\n\
  for (int i = 0; i < texSamples; i++) {\n\
    color += texelFetch(sampler, iUv, i);\n\
  }\n\
  color /= float(texSamples);\n\
  return color;\n\
}\n\
\n\
void main() {\n\
  fragColor = textureMultisample(msTex, vUv);\n\
  gl_FragDepth = textureMultisample(msDepthTex, vUv).r;\n\
}\n\
";

ComposeGlShader::ComposeGlShader() {
  glGenVertexArrays(1, &this->composeVao);

  // vertex array
  glBindVertexArray(this->composeVao);

  // vertex shader
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
    exout << "ML compose vertex shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // fragment shader
  GLuint composeFragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(composeFragment, 1, &composeFsh, NULL);
  glCompileShader(composeFragment);
  glGetShaderiv(composeFragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(composeFragment, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    exout << "ML compose fragment shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // shader program
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
    exout << "ML compose program linking failed\n" << infoLog << std::endl;
    return;
  }

  this->positionLocation = glGetAttribLocation(this->composeProgram, "position");
  if (this->positionLocation == -1) {
    exout << "ML compose program failed to get attrib location for 'position'" << std::endl;
    return;
  }
  this->uvLocation = glGetAttribLocation(this->composeProgram, "uv");
  if (this->uvLocation == -1) {
    exout << "ML compose program failed to get attrib location for 'uv'" << std::endl;
    return;
  }
  this->msTexLocation = glGetUniformLocation(this->composeProgram, "msTex");
  if (this->msTexLocation == -1) {
    exout << "ML compose program failed to get uniform location for 'msTex'" << std::endl;
    return;
  }
  this->msDepthTexLocation = glGetUniformLocation(this->composeProgram, "msDepthTex");
  if (this->msDepthTexLocation == -1) {
    exout << "ML compose program failed to get uniform location for 'msDepthTex'" << std::endl;
    return;
  }
  this->texSizeLocation = glGetUniformLocation(this->composeProgram, "texSize");
  if (this->texSizeLocation == -1) {
    exout << "ML compose program failed to get uniform location for 'texSize'" << std::endl;
    return;
  }

  // delete the shaders as they're linked into our program now and no longer necessery
  glDeleteShader(composeVertex);
  glDeleteShader(composeFragment);

  glGenBuffers(1, &this->positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->positionBuffer);
  static const float positions[] = {
    -1.0f, 1.0f,
    1.0f, 1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(this->positionLocation);
  glVertexAttribPointer(this->positionLocation, 2, GL_FLOAT, false, 0, 0);

  glGenBuffers(1, &this->uvBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->uvBuffer);
  static const float uvs[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
  glEnableVertexAttribArray(this->uvLocation);
  glVertexAttribPointer(this->uvLocation, 2, GL_FLOAT, false, 0, 0);

  glGenBuffers(1, &this->indexBuffer);
  static const uint16_t indices[] = {0, 2, 1, 2, 3, 1};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}
ComposeGlShader::~ComposeGlShader() {}
GlKey ComposeGlShader::key = GlKey::GL_KEY_COMPOSE;

const char *planeVsh = ""
#ifdef ANDROID
"#version 300 es\n"
#else
"#version 330\n"
#endif
"\n\
uniform mat4 modelViewMatrix;\n\
uniform mat4 projectionMatrix;\n\
in vec3 position;\n\
in vec2 uv;\n\
out vec2 vUv;\n\
\n\
void main() {\n\
  vUv = uv;\n\
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position.xy, 0., 1.);\n\
}\n\
";
const char *planeFsh = ""
#ifdef ANDROID
"#version 300 es\n"
#else
"#version 330\n"
#endif
"\n\
in vec2 vUv;\n\
out vec4 fragColor;\n\
uniform sampler2D tex;\n\
\n\
void main() {\n\
  fragColor = texture(tex, vUv);\n\
}\n\
";

PlaneGlShader::PlaneGlShader() {
  glGenVertexArrays(1, &this->planeVao);

  // vertex array
  glBindVertexArray(this->planeVao);

  // vertex shader
  GLuint planeVertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(planeVertex, 1, &planeVsh, NULL);
  glCompileShader(planeVertex);
  GLint success;
  glGetShaderiv(planeVertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(planeVertex, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    exout << "plane vertex shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // fragment shader
  GLuint planeFragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(planeFragment, 1, &planeFsh, NULL);
  glCompileShader(planeFragment);
  glGetShaderiv(planeFragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(planeFragment, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    exout << "plane fragment shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // shader program
  this->planeProgram = glCreateProgram();
  glAttachShader(this->planeProgram, planeVertex);
  glAttachShader(this->planeProgram, planeFragment);
  glLinkProgram(this->planeProgram);
  glGetProgramiv(this->planeProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(this->planeProgram, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    exout << "plane program linking failed\n" << infoLog << std::endl;
    return;
  }

  this->positionLocation = glGetAttribLocation(this->planeProgram, "position");
  if (this->positionLocation == -1) {
    exout << "plane program failed to get attrib location for 'position'" << std::endl;
    return;
  }
  this->uvLocation = glGetAttribLocation(this->planeProgram, "uv");
  if (this->uvLocation == -1) {
    exout << "plane program failed to get attrib location for 'uv'" << std::endl;
    return;
  }
  this->modelViewMatrixLocation = glGetUniformLocation(this->planeProgram, "modelViewMatrix");
  if (this->modelViewMatrixLocation == -1) {
    exout << "plane program failed to get uniform location for 'modelViewMatrix'" << std::endl;
    return;
  }
  this->projectionMatrixLocation = glGetUniformLocation(this->planeProgram, "projectionMatrix");
  if (this->projectionMatrixLocation == -1) {
    exout << "plane program failed to get uniform location for 'projectionMatrix'" << std::endl;
    return;
  }
  this->texLocation = glGetUniformLocation(this->planeProgram, "tex");
  if (this->texLocation == -1) {
    exout << "plane program failed to get uniform location for 'tex'" << std::endl;
    return;
  }

  // delete the shaders as they're linked into our program now and no longer necessery
  glDeleteShader(planeVertex);
  glDeleteShader(planeFragment);

  glGenBuffers(1, &this->positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->positionBuffer);
  static const float positions[] = {
    -1.0f, 1.0f,
    1.0f, 1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(this->positionLocation);
  glVertexAttribPointer(this->positionLocation, 2, GL_FLOAT, false, 0, 0);

  glGenBuffers(1, &this->uvBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->uvBuffer);
  static const float uvs[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
  glEnableVertexAttribArray(this->uvLocation);
  glVertexAttribPointer(this->uvLocation, 2, GL_FLOAT, false, 0, 0);

  glGenBuffers(1, &this->indexBuffer);
  static const uint16_t indices[] = {0, 2, 1, 2, 3, 1};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}
PlaneGlShader::~PlaneGlShader() {}
GlKey PlaneGlShader::key = GlKey::GL_KEY_PLANE;

constexpr GLint MAX_TEXTURE_SIZE = 4096;
bool CreateRenderTarget(WebGLRenderingContext *gl, int width, int height, GLuint sharedColorTex, GLuint sharedDepthStencilTex, GLuint sharedMsColorTex, GLuint sharedMsDepthStencilTex, GLuint *pfbo, GLuint *pcolorTex, GLuint *pdepthStencilTex, GLuint *pmsFbo, GLuint *pmsColorTex, GLuint *pmsDepthStencilTex) {
  const int samples = 4;

  GLuint &fbo = *pfbo;
  GLuint &colorTex = *pcolorTex;
  GLuint &depthStencilTex = *pdepthStencilTex;
  GLuint &msFbo = *pmsFbo;
  GLuint &msColorTex = *pmsColorTex;
  GLuint &msDepthStencilTex = *pmsDepthStencilTex;

  // NOTE: we create statically sized multisample textures because we cannot resize them later
  {
    glGenFramebuffers(1, &msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    if (!sharedMsDepthStencilTex) {
      glGenTextures(1, &msDepthStencilTex);
    } else {
      msDepthStencilTex = sharedMsDepthStencilTex;
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE/2, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE/2, true);
#endif
    // glFramebufferTexture2DMultisampleEXT(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, msDepthStencilTex, 0, samples);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    if (!sharedMsColorTex) {
      glGenTextures(1, &msColorTex);
    } else {
      msColorTex = sharedMsColorTex;
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE/2, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE/2, true);
#endif
    // glFramebufferTexture2DMultisampleEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, msColorTex, 0, samples);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);

    glClear(GL_DEPTH_BUFFER_BIT); // initialize to far depth
  }
  {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    if (!sharedDepthStencilTex) {
      glGenTextures(1, &depthStencilTex);
    } else {
      depthStencilTex = sharedDepthStencilTex;
    }
    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);

    if (!sharedColorTex) {
      glGenTextures(1, &colorTex);
    } else {
      colorTex = sharedColorTex;
    }
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    glClear(GL_DEPTH_BUFFER_BIT); // initialize to far depth
  }

  bool framebufferOk = (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
    glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE)) {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE));
  } else {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP)) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP));
  } else {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

  return framebufferOk;
}

NAN_METHOD(CreateRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = TO_INT32(info[1]);
  int height = TO_INT32(info[2]);
  GLuint sharedColorTex = TO_UINT32(info[3]);
  GLuint sharedDepthStencilTex = TO_UINT32(info[4]);
  GLuint sharedMsColorTex = TO_UINT32(info[5]);
  GLuint sharedMsDepthStencilTex = TO_UINT32(info[6]);

  GLuint fbo;
  GLuint colorTex;
  GLuint depthStencilTex;
  GLuint msFbo;
  GLuint msColorTex;
  GLuint msDepthStencilTex;
  bool ok = CreateRenderTarget(gl, width, height, sharedColorTex, sharedDepthStencilTex, sharedMsColorTex, sharedMsDepthStencilTex, &fbo, &colorTex, &depthStencilTex, &msFbo, &msColorTex, &msDepthStencilTex);

  Local<Value> result;
  if (ok) {
    Local<Array> array = Array::New(Isolate::GetCurrent(), 6);
    array->Set(0, JS_INT(fbo));
    array->Set(1, JS_INT(colorTex));
    array->Set(2, JS_INT(depthStencilTex));
    array->Set(3, JS_INT(msFbo));
    array->Set(4, JS_INT(msColorTex));
    array->Set(5, JS_INT(msDepthStencilTex));
    result = array;
  } else {
    result = Null(Isolate::GetCurrent());
  }
  info.GetReturnValue().Set(result);
}

NAN_METHOD(ResizeRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = TO_INT32(info[1]);
  int height = TO_INT32(info[2]);
  GLuint fbo = TO_UINT32(info[3]);
  GLuint colorTex = TO_UINT32(info[4]);
  GLuint depthStencilTex = TO_UINT32(info[5]);
  GLuint msFbo = TO_UINT32(info[6]);
  GLuint msColorTex = TO_UINT32(info[7]);
  GLuint msDepthStencilTex = TO_UINT32(info[8]);

  const int samples = 4;

  /* {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, true);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, true);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);
  } */
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);

    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
  }

  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
    glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE)) {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE));
  } else {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP)) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP));
  } else {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }
}

NAN_METHOD(DestroyRenderTarget) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    GLuint fbo = TO_UINT32(info[0]);
    GLuint tex = TO_UINT32(info[1]);
    GLuint depthTex = TO_UINT32(info[2]);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
    glDeleteTextures(1, &depthTex);
  } else {
    Nan::ThrowError("DestroyRenderTarget: invalid arguments");
  }
}

template <typename T>
T *getGlShader(WebGLRenderingContext *gl) {
  const GlKey &key = T::key;
  auto iter = gl->keys.find(key);
  if (iter != gl->keys.end()) {
    return (T *)iter->second;
  } else {
    T *t = new T();

    {
      if (gl->HasVertexArrayBinding()) {
        glBindVertexArray(gl->GetVertexArrayBinding());
      } else {
        glBindVertexArray(gl->defaultVao);
      }
      if (gl->HasBufferBinding(GL_ARRAY_BUFFER)) {
        glBindBuffer(GL_ARRAY_BUFFER, gl->GetBufferBinding(GL_ARRAY_BUFFER));
      } else {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    gl->keys[key] = t;
    return t;
  }
}

void ComposeLayer(WebGLRenderingContext *gl, const LayerSpec &layer) {
  if (layer.layerType == LayerType::IFRAME_3D || layer.layerType == LayerType::RAW_CANVAS) {
    ComposeGlShader *composeGlShader = getGlShader<ComposeGlShader>(gl);

    glBindVertexArray(composeGlShader->composeVao);
    glUseProgram(composeGlShader->composeProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, layer.msTex);
    glUniform1i(composeGlShader->msTexLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, layer.msDepthTex);
    glUniform1i(composeGlShader->msDepthTexLocation, 1);

    glUniform4f(composeGlShader->texSizeLocation, 0, 0, layer.width, layer.height);

    glViewport(0, 0, layer.width, layer.height);
    // glScissor(0, 0, width, height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  } else {
    PlaneGlShader *planeGlShader = getGlShader<PlaneGlShader>(gl);

    glBindVertexArray(planeGlShader->planeVao);
    glUseProgram(planeGlShader->planeProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, layer.tex);
    glUniform1i(planeGlShader->texLocation, 0);

    {
      glUniformMatrix4fv(planeGlShader->modelViewMatrixLocation, 1, false, layer.modelView[0]);
      glUniformMatrix4fv(planeGlShader->projectionMatrixLocation, 1, false, layer.projection[0]);

      // glViewport(layer.viewports[0][0], layer.viewports[0][1], layer.viewports[0][2], layer.viewports[0][3]);
      glViewport(0, 0, layer.width/2, layer.height);
      // glScissor(0, 0, width, height);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
    {
      glUniformMatrix4fv(planeGlShader->modelViewMatrixLocation, 1, false, layer.modelView[1]);
      glUniformMatrix4fv(planeGlShader->projectionMatrixLocation, 1, false, layer.projection[1]);

      // glViewport(layer.viewports[1][0], layer.viewports[1][1], layer.viewports[1][2], layer.viewports[1][3]);
      glViewport(layer.width/2, 0, layer.width/2, layer.height);
      // glScissor(0, 0, width, height);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
  }
}

void ComposeLayer(WebGLRenderingContext *gl, GLuint *fbos, const LayerSpec &layer) {
  if (layer.layerType == LayerType::IFRAME_3D || layer.layerType == LayerType::RAW_CANVAS) {
    ComposeGlShader *composeGlShader = getGlShader<ComposeGlShader>(gl);

    glBindVertexArray(composeGlShader->composeVao);
    glUseProgram(composeGlShader->composeProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, layer.msTex);
    glUniform1i(composeGlShader->msTexLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, layer.msDepthTex);
    glUniform1i(composeGlShader->msDepthTexLocation, 1);

    glViewport(0, 0, layer.width/2, layer.height);
    // glScissor(0, 0, width, height);

    for (size_t i = 0; i < 2; i++) {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[i]);

      glUniform4f(composeGlShader->texSizeLocation, i*(layer.width/2), 0, layer.width/2, layer.height);

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
  } else {
    PlaneGlShader *planeGlShader = getGlShader<PlaneGlShader>(gl);

    glBindVertexArray(planeGlShader->planeVao);
    glUseProgram(planeGlShader->planeProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, layer.tex);
    glUniform1i(planeGlShader->texLocation, 0);

    {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[0]);

      glUniformMatrix4fv(planeGlShader->modelViewMatrixLocation, 1, false, layer.modelView[0]);
      glUniformMatrix4fv(planeGlShader->projectionMatrixLocation, 1, false, layer.projection[0]);

      // glViewport(layer.viewports[0][0], layer.viewports[0][1], layer.viewports[0][2], layer.viewports[0][3]);
      glViewport(0, 0, layer.width/2, layer.height);
      // glScissor(0, 0, width, height);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
    {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[1]);

      glUniformMatrix4fv(planeGlShader->modelViewMatrixLocation, 1, false, layer.modelView[1]);
      glUniformMatrix4fv(planeGlShader->projectionMatrixLocation, 1, false, layer.projection[1]);

      // glViewport(layer.viewports[1][0], layer.viewports[1][1], layer.viewports[1][2], layer.viewports[1][3]);
      glViewport(layer.width/2, 0, layer.width/2, layer.height);
      // glScissor(0, 0, width, height);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
  }
}

void ComposeLayers(WebGLRenderingContext *gl, size_t numFbos, GLuint *fbos, const std::vector<LayerSpec> &layers) {
  for (size_t i = 0; i < numFbos; i++) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[i]);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  }
  // XXX if there is one layer, we can disable depth test/depth write
  if (numFbos == 1) {
    for (size_t i = 0; i < layers.size(); i++) {
      ComposeLayer(gl, layers[i]);
    }
  } else {
    for (size_t i = 0; i < layers.size(); i++) {
      ComposeLayer(gl, fbos, layers[i]);
    }
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
  if (gl->viewportState.valid) {
    glViewport(gl->viewportState.x, gl->viewportState.y, gl->viewportState.w, gl->viewportState.h);
  } else {
    glViewport(0, 0, 1280, 1024);
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
  if (info[0]->IsObject() && (info[1]->IsNumber() || info[1]->IsArray()) && info[2]->IsArray()) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    Local<Array> array = Local<Array>::Cast(info[2]);

    size_t numFbos;
    GLuint fbos[2];
    if (info[1]->IsNumber()) {
      fbos[0] = TO_UINT32(info[1]);
      numFbos = 1;
    } else {
      Local<Array> fbosArray = Local<Array>::Cast(info[1]);
      fbos[0] = TO_UINT32(fbosArray->Get(0));
      fbos[1] = TO_UINT32(fbosArray->Get(1));
      numFbos = 2;
    }

    std::vector<LayerSpec> layers;
    layers.reserve(8);
    for (size_t i = 0, size = array->Length(); i < size; i++) {
      Local<Value> element = array->Get(i);

      if (element->IsObject()) {
        Local<Object> elementObj = Local<Object>::Cast(element);

        LayerType layerType = LayerType::NONE;
        if (JS_OBJ(elementObj->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLIFrameElement"))) {
          if (
            elementObj->Get(JS_STR("contentDocument"))->IsObject() &&
            JS_OBJ(elementObj->Get(JS_STR("contentDocument")))->Get(JS_STR("framebuffer"))->IsObject()
          ) {
            layerType = LayerType::IFRAME_3D;
          } else if (elementObj->Get(JS_STR("browser"))->IsObject()) {
            layerType = LayerType::IFRAME_2D;
          }
        } else if (JS_OBJ(elementObj->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLCanvasElement"))) {
          if (elementObj->Get(JS_STR("framebuffer"))->IsObject()) {
            layerType = LayerType::RAW_CANVAS;
          }
        }

        switch (layerType) {
          case LayerType::IFRAME_3D: {
            Local<Object> framebufferObj = Local<Object>::Cast(JS_OBJ(elementObj->Get(JS_STR("contentDocument")))->Get(JS_STR("framebuffer")));
            GLuint tex = TO_UINT32(framebufferObj->Get(JS_STR("tex")));
            GLuint depthTex = TO_UINT32(framebufferObj->Get(JS_STR("depthTex")));
            GLuint msTex = TO_UINT32(framebufferObj->Get(JS_STR("msTex")));
            GLuint msDepthTex = TO_UINT32(framebufferObj->Get(JS_STR("msDepthTex")));
            int width = TO_INT32(JS_OBJ(framebufferObj->Get(JS_STR("canvas")))->Get(JS_STR("width")));
            int height = TO_INT32(JS_OBJ(framebufferObj->Get(JS_STR("canvas")))->Get(JS_STR("height")));

            layers.push_back(LayerSpec{
              layerType,
              width,
              height,
              msTex,
              msDepthTex,
              tex,
              depthTex,
              {nullptr,nullptr},
              {nullptr,nullptr}
            });
            break;
          }
          case LayerType::IFRAME_2D: {
            Local<Object> browserObj = Local<Object>::Cast(elementObj->Get(JS_STR("browser")));
            GLuint tex = TO_UINT32(Local<Object>::Cast(browserObj->Get(JS_STR("texture")))->Get(JS_STR("id")));
            /* Local<Array> viewportsArray = Local<Array>::Cast(browserObj->Get(JS_STR("viewports"))); // XXX get rid of this in callers
            Local<Float32Array> viewportsFloat32Arrays[] = {
              Local<Float32Array>::Cast(viewportsArray->Get(0)),
              Local<Float32Array>::Cast(viewportsArray->Get(1)),
            }; */
            Local<Array> modelViewArray = Local<Array>::Cast(browserObj->Get(JS_STR("modelView")));
            Local<Float32Array> modelViewFloat32Arrays[] = {
              Local<Float32Array>::Cast(modelViewArray->Get(0)),
              Local<Float32Array>::Cast(modelViewArray->Get(1)),
            };
            Local<Array> projectionArray = Local<Array>::Cast(browserObj->Get(JS_STR("projection")));
            Local<Float32Array> projectionFloat32Arrays[] = {
              Local<Float32Array>::Cast(projectionArray->Get(0)),
              Local<Float32Array>::Cast(projectionArray->Get(1)),
            };
            int width = TO_INT32(browserObj->Get(JS_STR("width")));
            int height = TO_INT32(browserObj->Get(JS_STR("height")));

            layers.push_back(LayerSpec{
              layerType,
              width,
              height,
              0,
              0,
              tex,
              0,
              { // modelView
                (float *)((char *)(modelViewFloat32Arrays[0]->Buffer()->GetContents().Data()) + modelViewFloat32Arrays[0]->ByteOffset()),
                (float *)((char *)(modelViewFloat32Arrays[1]->Buffer()->GetContents().Data()) + modelViewFloat32Arrays[1]->ByteOffset())
              },
              { // projection
                (float *)((char *)(projectionFloat32Arrays[0]->Buffer()->GetContents().Data()) + projectionFloat32Arrays[0]->ByteOffset()),
                (float *)((char *)(projectionFloat32Arrays[1]->Buffer()->GetContents().Data()) + projectionFloat32Arrays[1]->ByteOffset())
              }
            });
            break;
          }
          case LayerType::RAW_CANVAS: {
            Local<Object> framebufferObj = Local<Object>::Cast(elementObj->Get(JS_STR("framebuffer")));
            GLuint tex = TO_UINT32(framebufferObj->Get(JS_STR("tex")));
            GLuint depthTex = TO_UINT32(framebufferObj->Get(JS_STR("depthTex")));
            GLuint msTex = TO_UINT32(framebufferObj->Get(JS_STR("msTex")));
            GLuint msDepthTex = TO_UINT32(framebufferObj->Get(JS_STR("msDepthTex")));
            int width = TO_INT32(elementObj->Get(JS_STR("width")));
            int height = TO_INT32(elementObj->Get(JS_STR("height")));

            layers.push_back(LayerSpec{
              layerType,
              width,
              height,
              msTex,
              msDepthTex,
              tex,
              depthTex,
              {nullptr,nullptr},
              {nullptr,nullptr}
            });
            break;
          }
          default: {
            // nothing
            break;
          }
        }
      } else {
        return Nan::ThrowError("WindowSystem::ComposeLayers: invalid layer element");
      }
    }

    if (layers.size() > 0) {
      ComposeLayers(gl, numFbos, fbos, layers);
    }
  } else {
    Nan::ThrowError("WindowSystem::ComposeLayers: invalid arguments");
  }
}

thread_local uv_loop_t *eventLoop;
NAN_METHOD(GetEventLoop) {
  info.GetReturnValue().Set(pointerToArray(eventLoop));
}
NAN_METHOD(SetEventLoop) {
  if (info[0]->IsArray()) {
    Local<Array> loopArray = Local<Array>::Cast(info[0]);
    uv_loop_t *loop = (uv_loop_t *)arrayToPointer(loopArray);
    eventLoop = loop;
  } else {
    Nan::ThrowError("WindowSystem::SetEventLoop: invalid arguments");
  }
}

void Decorate(Local<Object> target) {
  Nan::SetMethod(target, "createRenderTarget", CreateRenderTarget);
  Nan::SetMethod(target, "resizeRenderTarget", ResizeRenderTarget);
  Nan::SetMethod(target, "destroyRenderTarget", DestroyRenderTarget);
  // Nan::SetMethod(target, "renderPlane", RenderPlane);
  Nan::SetMethod(target, "composeLayers", ComposeLayers);
  Nan::SetMethod(target, "getEventLoop", GetEventLoop);
  Nan::SetMethod(target, "setEventLoop", SetEventLoop);
}

}

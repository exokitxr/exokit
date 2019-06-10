#include <windowsystem.h>

#include <exout>

namespace windowsystembase {

std::vector<float> multiplyMatrices(const std::vector<float> &a, const std::vector<float> &b) {
  std::vector<float> result(16);
  
  const float *ae = a.data();
  const float *be = b.data();
  float *te = result.data();

  float a11 = ae[ 0 ], a12 = ae[ 4 ], a13 = ae[ 8 ], a14 = ae[ 12 ];
  float a21 = ae[ 1 ], a22 = ae[ 5 ], a23 = ae[ 9 ], a24 = ae[ 13 ];
  float a31 = ae[ 2 ], a32 = ae[ 6 ], a33 = ae[ 10 ], a34 = ae[ 14 ];
  float a41 = ae[ 3 ], a42 = ae[ 7 ], a43 = ae[ 11 ], a44 = ae[ 15 ];

  float b11 = be[ 0 ], b12 = be[ 4 ], b13 = be[ 8 ], b14 = be[ 12 ];
  float b21 = be[ 1 ], b22 = be[ 5 ], b23 = be[ 9 ], b24 = be[ 13 ];
  float b31 = be[ 2 ], b32 = be[ 6 ], b33 = be[ 10 ], b34 = be[ 14 ];
  float b41 = be[ 3 ], b42 = be[ 7 ], b43 = be[ 11 ], b44 = be[ 15 ];

  te[ 0 ] = a11 * b11 + a12 * b21 + a13 * b31 + a14 * b41;
  te[ 4 ] = a11 * b12 + a12 * b22 + a13 * b32 + a14 * b42;
  te[ 8 ] = a11 * b13 + a12 * b23 + a13 * b33 + a14 * b43;
  te[ 12 ] = a11 * b14 + a12 * b24 + a13 * b34 + a14 * b44;

  te[ 1 ] = a21 * b11 + a22 * b21 + a23 * b31 + a24 * b41;
  te[ 5 ] = a21 * b12 + a22 * b22 + a23 * b32 + a24 * b42;
  te[ 9 ] = a21 * b13 + a22 * b23 + a23 * b33 + a24 * b43;
  te[ 13 ] = a21 * b14 + a22 * b24 + a23 * b34 + a24 * b44;

  te[ 2 ] = a31 * b11 + a32 * b21 + a33 * b31 + a34 * b41;
  te[ 6 ] = a31 * b12 + a32 * b22 + a33 * b32 + a34 * b42;
  te[ 10 ] = a31 * b13 + a32 * b23 + a33 * b33 + a34 * b43;
  te[ 14 ] = a31 * b14 + a32 * b24 + a33 * b34 + a34 * b44;

  te[ 3 ] = a41 * b11 + a42 * b21 + a43 * b31 + a44 * b41;
  te[ 7 ] = a41 * b12 + a42 * b22 + a43 * b32 + a44 * b42;
  te[ 11 ] = a41 * b13 + a42 * b23 + a43 * b33 + a44 * b43;
  te[ 15 ] = a41 * b14 + a42 * b24 + a43 * b34 + a44 * b44;
  
  return result;
}

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
uniform sampler2D tex;\n\
uniform sampler2D depthTex;\n\
\n\
void main() {\n\
  fragColor = texture(tex, vUv);\n\
  gl_FragDepth = texture(depthTex, vUv).r;\n\
}\n\
";

ComposeGlShader::ComposeGlShader() {
  glGenFramebuffers(2, this->blitFbos);

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
    exout << "compose vertex shader compilation failed:\n" << infoLog << std::endl;
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
    exout << "compose fragment shader compilation failed:\n" << infoLog << std::endl;
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
    exout << "compose program linking failed\n" << infoLog << std::endl;
    return;
  }

  this->positionLocation = glGetAttribLocation(this->composeProgram, "position");
  if (this->positionLocation == -1) {
    exout << "compose program failed to get attrib location for 'position'" << std::endl;
    return;
  }
  this->uvLocation = glGetAttribLocation(this->composeProgram, "uv");
  if (this->uvLocation == -1) {
    exout << "compose program failed to get attrib location for 'uv'" << std::endl;
    return;
  }
  this->texLocation = glGetUniformLocation(this->composeProgram, "tex");
  if (this->texLocation == -1) {
    exout << "compose program failed to get uniform location for 'tex'" << std::endl;
    return;
  }
  this->depthTexLocation = glGetUniformLocation(this->composeProgram, "depthTex");
  if (this->depthTexLocation == -1) {
    exout << "compose program failed to get uniform location for 'depthTex'" << std::endl;
    return;
  }

  // delete the shaders as they're linked into our program now and no longer necessary
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

/* void InitializeLocalGlState(WebGLRenderingContext *gl) {
  // compose
  {
    ComposeSpec *composeSpec = new ComposeSpec();

    // blit fbos
    glGenFramebuffers(2, composeSpec->blitFbos);

    // vertex array
    glGenVertexArrays(1, &composeSpec->composeVao);
    glBindVertexArray(composeSpec->composeVao);

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
      exout << "compose vertex shader compilation failed:\n" << infoLog << std::endl;
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
      exout << "compose fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader program
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
      exout << "compose program linking failed\n" << infoLog << std::endl;
      return;
    }

    composeSpec->positionLocation = glGetAttribLocation(composeSpec->composeProgram, "position");
    if (composeSpec->positionLocation == -1) {
      exout << "compose program failed to get attrib location for 'position'" << std::endl;
      return;
    }
    composeSpec->uvLocation = glGetAttribLocation(composeSpec->composeProgram, "uv");
    if (composeSpec->uvLocation == -1) {
      exout << "compose program failed to get attrib location for 'uv'" << std::endl;
      return;
    }
    composeSpec->texLocation = glGetUniformLocation(composeSpec->composeProgram, "tex");
    if (composeSpec->texLocation == -1) {
      exout << "compose program failed to get uniform location for 'tex'" << std::endl;
      return;
    }
    composeSpec->depthTexLocation = glGetUniformLocation(composeSpec->composeProgram, "depthTex");
    if (composeSpec->depthTexLocation == -1) {
      exout << "compose program failed to get uniform location for 'depthTex'" << std::endl;
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

    glGenBuffers(1, &composeSpec->indexBuffer);
    static const uint16_t indices[] = {0, 2, 1, 2, 3, 1};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, composeSpec->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    gl->keys[GlKey::GL_KEY_COMPOSE] = composeSpec;
  }

  // plane
  {
    PlaneSpec *planeSpec = new PlaneSpec();

    glGenVertexArrays(1, &planeSpec->planeVao);

    // vertex array
    glBindVertexArray(planeSpec->planeVao);

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
    planeSpec->planeProgram = glCreateProgram();
    glAttachShader(planeSpec->planeProgram, planeVertex);
    glAttachShader(planeSpec->planeProgram, planeFragment);
    glLinkProgram(planeSpec->planeProgram);
    glGetProgramiv(planeSpec->planeProgram, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(planeSpec->planeProgram, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      exout << "plane program linking failed\n" << infoLog << std::endl;
      return;
    }

    planeSpec->positionLocation = glGetAttribLocation(planeSpec->planeProgram, "position");
    if (planeSpec->positionLocation == -1) {
      exout << "plane program failed to get attrib location for 'position'" << std::endl;
      return;
    }
    planeSpec->uvLocation = glGetAttribLocation(planeSpec->planeProgram, "uv");
    if (planeSpec->uvLocation == -1) {
      exout << "plane program failed to get attrib location for 'uv'" << std::endl;
      return;
    }
    planeSpec->modelViewMatrixLocation = glGetUniformLocation(planeSpec->planeProgram, "modelViewMatrix");
    if (planeSpec->modelViewMatrixLocation == -1) {
      exout << "plane program failed to get uniform location for 'modelViewMatrix'" << std::endl;
      return;
    }
    planeSpec->projectionMatrixLocation = glGetUniformLocation(planeSpec->planeProgram, "projectionMatrix");
    if (planeSpec->projectionMatrixLocation == -1) {
      exout << "plane program failed to get uniform location for 'projectionMatrix'" << std::endl;
      return;
    }
    planeSpec->texLocation = glGetUniformLocation(planeSpec->planeProgram, "tex");
    if (planeSpec->texLocation == -1) {
      exout << "plane program failed to get uniform location for 'tex'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(planeVertex);
    glDeleteShader(planeFragment);

    glGenBuffers(1, &planeSpec->positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planeSpec->positionBuffer);
    static const float positions[] = {
      -1.0f, 1.0f,
      1.0f, 1.0f,
      -1.0f, -1.0f,
      1.0f, -1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(planeSpec->positionLocation);
    glVertexAttribPointer(planeSpec->positionLocation, 2, GL_FLOAT, false, 0, 0);

    glGenBuffers(1, &planeSpec->uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planeSpec->uvBuffer);
    static const float uvs[] = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(planeSpec->uvLocation);
    glVertexAttribPointer(planeSpec->uvLocation, 2, GL_FLOAT, false, 0, 0);

    glGenBuffers(1, &planeSpec->indexBuffer);
    static const uint16_t indices[] = {0, 2, 1, 2, 3, 1};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeSpec->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    gl->keys[GlKey::GL_KEY_PLANE] = planeSpec;
  }

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
} */

constexpr GLint MAX_TEXTURE_SIZE = 4096;
constexpr GLint NUM_SAMPLES = 4;
void CreateRenderTarget(WebGLRenderingContext *gl, int width, int height, GLuint *pfbo, GLuint *pcolorTex, GLuint *pdepthStencilTex, GLuint *pmsFbo, GLuint *pmsColorTex, GLuint *pmsDepthStencilTex) {
  GLuint &fbo = *pfbo;
  GLuint &colorTex = *pcolorTex;
  GLuint &depthStencilTex = *pdepthStencilTex;
  GLuint &msFbo = *pmsFbo;
  GLuint &msColorTex = *pmsColorTex;
  GLuint &msDepthStencilTex = *pmsDepthStencilTex;

  // NOTE: we create statically sized multisampled textures because we cannot resize them later
  {
    glGenFramebuffers(1, &msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    glGenTextures(1, &msDepthStencilTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    glGenTextures(1, &msColorTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);

    // glClear(GL_DEPTH_BUFFER_BIT); // initialize to far depth
  }
  {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    glGenTextures(1, &depthStencilTex);
    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    
    // glClear(GL_DEPTH_BUFFER_BIT); // initialize to far depth
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

NAN_METHOD(CreateRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = TO_INT32(info[1]);
  int height = TO_INT32(info[2]);

  GLuint fbo;
  GLuint colorTex;
  GLuint depthStencilTex;
  GLuint msFbo;
  GLuint msColorTex;
  GLuint msDepthStencilTex;

  CreateRenderTarget(gl, width, height, &fbo, &colorTex, &depthStencilTex, &msFbo, &msColorTex, &msDepthStencilTex);

  Local<Array> result = Array::New(Isolate::GetCurrent(), 6);
  result->Set(0, JS_INT(fbo));
  result->Set(1, JS_INT(colorTex));
  result->Set(2, JS_INT(depthStencilTex));
  result->Set(3, JS_INT(msFbo));
  result->Set(4, JS_INT(msColorTex));
  result->Set(5, JS_INT(msDepthStencilTex));
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

  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    glDeleteTextures(1, &msDepthStencilTex);
    glGenTextures(1, &msDepthStencilTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    glDeleteTextures(1, &msColorTex);
    glGenTextures(1, &msColorTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);

    // glClear(GL_DEPTH_BUFFER_BIT); // initialize to far depth
  }

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
  
  Local<Array> result = Array::New(Isolate::GetCurrent(), 6);
  result->Set(0, JS_INT(fbo));
  result->Set(1, JS_INT(colorTex));
  result->Set(2, JS_INT(depthStencilTex));
  result->Set(3, JS_INT(msFbo));
  result->Set(4, JS_INT(msColorTex));
  result->Set(5, JS_INT(msDepthStencilTex));
  info.GetReturnValue().Set(result);
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

void CreateVrTopRenderTarget(int width, int height, GLuint *pfbo, GLuint *pcolorTex, GLuint *pdepthStencilTex, GLuint *pmsFbo, GLuint *pmsColorTex, GLuint *pmsDepthStencilTex) {
  GLuint &fbo = *pfbo;
  GLuint &colorTex = *pcolorTex;
  GLuint &depthStencilTex = *pdepthStencilTex;
  GLuint &msFbo = *pmsFbo;
  GLuint &msColorTex = *pmsColorTex;
  GLuint &msDepthStencilTex = *pmsDepthStencilTex;

  {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    glGenTextures(1, &depthStencilTex);
    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);
  }

  {
    glGenFramebuffers(1, &msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    glGenTextures(1, &msColorTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);

    glGenTextures(1, &msDepthStencilTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);
  }
}

NAN_METHOD(CreateVrTopRenderTarget) {
  int width = TO_INT32(info[0]);
  int height = TO_INT32(info[1]);

  GLuint fbo;
  GLuint colorTex;
  GLuint depthStencilTex;
  GLuint msFbo;
  GLuint msColorTex;
  GLuint msDepthStencilTex;

  CreateVrTopRenderTarget(width, height, &fbo, &colorTex, &depthStencilTex, &msFbo, &msColorTex, &msDepthStencilTex);

  Local<Array> result = Array::New(Isolate::GetCurrent(), 6);
  result->Set(0, JS_INT(fbo));
  result->Set(1, JS_INT(colorTex));
  result->Set(2, JS_INT(depthStencilTex));
  result->Set(3, JS_INT(msFbo));
  result->Set(4, JS_INT(msColorTex));
  result->Set(5, JS_INT(msDepthStencilTex));
  info.GetReturnValue().Set(result);
}

void CreateVrCompositorRenderTarget(WebGLRenderingContext *gl, int width, int height, GLuint *pFbo, GLuint *pmsFbo, GLuint *pmsColorTex, GLuint *pmsDepthStencilTex) {
  GLuint &fbo = *pFbo;
  GLuint &msFbo = *pmsFbo;
  GLuint &msColorTex = *pmsColorTex;
  GLuint &msDepthStencilTex = *pmsDepthStencilTex;

  // NOTE: we create statically sized multisampled textures because we cannot resize them later
  {
    glGenFramebuffers(1, &msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    glGenTextures(1, &msColorTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);

    glGenTextures(1, &msDepthStencilTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
#if !defined(ANDROID) && !defined(LUMIN)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#else
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_DEPTH24_STENCIL8, width, height, true);
#endif
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    // glClear(GL_DEPTH_BUFFER_BIT); // initialize to far depth
  }
  {
    glGenFramebuffers(1, &fbo);
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

NAN_METHOD(CreateVrCompositorRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = TO_INT32(info[1]);
  int height = TO_INT32(info[2]);

  GLuint fbo;
  GLuint msFbo;
  GLuint msColorTex;
  GLuint msDepthStencilTex;

  CreateVrCompositorRenderTarget(gl, width, height, &fbo, &msFbo, &msColorTex, &msDepthStencilTex);

  Local<Array> result = Array::New(Isolate::GetCurrent(), 4);
  result->Set(0, JS_INT(fbo));
  result->Set(1, JS_INT(msFbo));
  result->Set(2, JS_INT(msColorTex));
  result->Set(3, JS_INT(msDepthStencilTex));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(BindVrChildFbo) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  GLuint fbo = TO_UINT32(info[1]);
  GLuint colorTex = TO_UINT32(info[2]);
  GLuint depthStencilTex = TO_UINT32(info[3]);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);

  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
}

NAN_METHOD(BindVrChildMsFbo) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  GLuint msFbo = TO_UINT32(info[1]);
  GLuint msColorTex = TO_UINT32(info[2]);
  GLuint msDepthStencilTex = TO_UINT32(info[3]);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
}

/* NAN_METHOD(CopyRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = TO_INT32(info[1]);
  int height = TO_INT32(info[2]);
  // GLuint srcFbo = TO_UINT32(info[3]);
  GLuint srcMsColorTex = TO_UINT32(info[4]);
  GLuint srcMsDepthStencilTex = TO_UINT32(info[5]);
  // GLuint dstFbo = TO_UINT32(info[6]);
  GLuint dstMsColorTex = TO_UINT32(info[7]);
  GLuint dstMsDepthStencilTex = TO_UINT32(info[8]);

  {
    GLuint fbos[2];
    glGenFramebuffers(2, fbos);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[0]);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, srcMsColorTex, 0);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, srcMsDepthStencilTex, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[1]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, dstMsColorTex, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, dstMsDepthStencilTex, 0);

    glBlitFramebuffer(
      0, 0,
      width, height,
      0, 0,
      width, height,
      GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT,
      GL_NEAREST
    );

    glDeleteFramebuffers(2, fbos);
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
} */

NAN_METHOD(GetSync) {
  GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  glFlush();
  info.GetReturnValue().Set(pointerToArray((void *)sync));
}

NAN_METHOD(WaitSync) {
  // if (info[0]->IsArray()) {
    Local<Array> syncArray = Local<Array>::Cast(info[0]);
    uintptr_t syncPtr = arrayToPointer(syncArray);
    if (syncPtr > 0) { // macOS returns GLsync zero and crashes when used
      GLsync sync = reinterpret_cast<GLsync>(syncPtr);
      glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
    }
    // glDeleteSync(sync);
  /* } else {
    Nan::ThrowError("WaitSync: invalid arguments");
  } */
}

NAN_METHOD(DeleteSync) {
  // if (info[0]->IsArray()) {
    Local<Array> syncArray = Local<Array>::Cast(info[0]);
    uintptr_t syncPtr = arrayToPointer(syncArray);
    if (syncPtr > 0) { // macOS returns GLsync zero and crashes when used
      GLsync sync = reinterpret_cast<GLsync>(syncPtr);
      glDeleteSync(sync);
    }
  /* } else {
    Nan::ThrowError("DeleteSync: invalid arguments");
  } */
}

void BlitLayer(WebGLRenderingContext *gl, const LayerSpec &layer) {
  if (layer.layerType == LayerType::IFRAME_3D || layer.layerType == LayerType::RAW_CANVAS) {
    ComposeGlShader *composeGlShader = getGlShader<ComposeGlShader>(gl);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, composeGlShader->blitFbos[0]);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, layer.msTex, 0);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, layer.msDepthTex, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, composeGlShader->blitFbos[1]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layer.tex, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, layer.depthTex, 0);

    glBlitFramebuffer(
      0, 0,
      layer.width, layer.height,
      0, 0,
      layer.width, layer.height,
      GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT,
      GL_NEAREST
    );
  }
}

void ComposeLayer(WebGLRenderingContext *gl, const LayerSpec &layer) {
  if (layer.layerType == LayerType::IFRAME_3D || layer.layerType == LayerType::IFRAME_3D_REPROJECT || layer.layerType == LayerType::RAW_CANVAS) {
    ComposeGlShader *composeGlShader = getGlShader<ComposeGlShader>(gl);

    glBindVertexArray(composeGlShader->composeVao);
    glUseProgram(composeGlShader->composeProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, layer.tex);
    glUniform1i(composeGlShader->texLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, layer.depthTex);
    glUniform1i(composeGlShader->depthTexLocation, 1);

    glViewport(0, 0, layer.width, layer.height);
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

      glViewport(layer.viewports[0][0], layer.viewports[0][1], layer.viewports[0][2], layer.viewports[0][3]);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
    {
      glUniformMatrix4fv(planeGlShader->modelViewMatrixLocation, 1, false, layer.modelView[1]);
      glUniformMatrix4fv(planeGlShader->projectionMatrixLocation, 1, false, layer.projection[1]);

      glViewport(layer.viewports[1][0], layer.viewports[1][1], layer.viewports[1][2], layer.viewports[1][3]);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
  }
}

void ComposeLayers(WebGLRenderingContext *gl, const std::vector<LayerSpec> &layers) {
  for (size_t i = 0; i < layers.size(); i++) {
    BlitLayer(gl, layers[i]);
  }

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  for (size_t i = 0; i < layers.size(); i++) {
    ComposeLayer(gl, layers[i]);
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
  if (info[0]->IsObject() && info[1]->IsArray() && info[2]->IsObject()) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    Local<Array> array = Local<Array>::Cast(info[1]);
    Local<Object> xrStateObj = Local<Array>::Cast(info[2]);

    std::vector<LayerSpec> layers;
    layers.reserve(8);
    for (size_t i = 0, size = array->Length(); i < size; i++) {
      Local<Value> element = array->Get(i);

      if (element->IsObject()) {
        Local<Object> elementObj = Local<Object>::Cast(element);

        LayerType layerType = LayerType::NONE;
        if (JS_OBJ(elementObj->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLIFrameElement"))) {
          /* if (
            elementObj->Get(JS_STR("contentWindow"))->IsObject() &&
            elementObj->Get(JS_STR("contentDocument"))->IsObject() &&
            JS_OBJ(elementObj->Get(JS_STR("contentDocument")))->Get(JS_STR("framebuffer"))->IsObject()
          ) {
            // layerType = LayerType::IFRAME_3D;
            Local<Object> contentWindowObj = JS_OBJ(elementObj->Get(JS_STR("contentWindow")));
            if (TO_UINT32(contentWindowObj->Get(JS_STR("phase"))) == 2) { // PHASES.RENDERED
              layerType = LayerType::IFRAME_3D;
            } else {
              layerType = LayerType::IFRAME_3D_REPROJECT;
            }
          } else */if (TO_UINT32(elementObj->Get(JS_STR("d"))) == 2 && elementObj->Get(JS_STR("browser"))->IsObject()) {
            layerType = LayerType::IFRAME_2D;
          }
        } /* else if (JS_OBJ(elementObj->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLCanvasElement"))) {
          if (elementObj->Get(JS_STR("framebuffer"))->IsObject()) {
            layerType = LayerType::RAW_CANVAS;
          }
        } */

        switch (layerType) {
          /* case LayerType::IFRAME_3D: {
            Local<Object> windowObj = Local<Object>::Cast(elementObj->Get(JS_STR("contentWindow")));
            int width = TO_INT32(windowObj->Get(JS_STR("width")));
            int height = TO_INT32(windowObj->Get(JS_STR("height")));
            Local<Object> framebufferObj = Local<Object>::Cast(JS_OBJ(elementObj->Get(JS_STR("contentDocument")))->Get(JS_STR("framebuffer")));
            GLuint msTex = TO_UINT32(framebufferObj->Get(JS_STR("msTex")));
            GLuint msDepthTex = TO_UINT32(framebufferObj->Get(JS_STR("msDepthTex")));
            GLuint tex = TO_UINT32(framebufferObj->Get(JS_STR("tex")));
            GLuint depthTex = TO_UINT32(framebufferObj->Get(JS_STR("depthTex")));

            layers.push_back(LayerSpec{
              LayerType::IFRAME_3D,
              width,
              height,
              msTex,
              msDepthTex,
              tex,
              depthTex,
              {nullptr,nullptr},
              {nullptr,nullptr},
              {nullptr,nullptr}
            });
            break;
          }
          case LayerType::IFRAME_3D_REPROJECT: {
            Local<Object> windowObj = Local<Object>::Cast(elementObj->Get(JS_STR("contentWindow")));
            int width = TO_INT32(windowObj->Get(JS_STR("width")));
            int height = TO_INT32(windowObj->Get(JS_STR("height")));
            Local<Object> framebufferObj = Local<Object>::Cast(JS_OBJ(elementObj->Get(JS_STR("contentDocument")))->Get(JS_STR("framebuffer")));
            GLuint tex = TO_UINT32(framebufferObj->Get(JS_STR("tex")));
            GLuint depthTex = TO_UINT32(framebufferObj->Get(JS_STR("depthTex")));

            layers.push_back(LayerSpec{
              LayerType::IFRAME_3D_REPROJECT,
              width,
              height,
              0,
              0,
              tex,
              depthTex,
              {nullptr,nullptr},
              {nullptr,nullptr},
              {nullptr,nullptr}
            });
            break;
          } */
          case LayerType::IFRAME_2D: {
            Local<Object> browserObj = Local<Object>::Cast(elementObj->Get(JS_STR("browser")));
            int width = TO_INT32(browserObj->Get(JS_STR("width")));
            int height = TO_INT32(browserObj->Get(JS_STR("height")));
            GLuint tex = TO_UINT32(Local<Object>::Cast(browserObj->Get(JS_STR("texture")))->Get(JS_STR("id")));

            Local<Float32Array> renderWidthFloat32Array = Local<Float32Array>::Cast(xrStateObj->Get(JS_STR("renderWidth")));
            const float renderWidth = TO_FLOAT(renderWidthFloat32Array->Get(0));
            Local<Float32Array> renderHeightFloat32Array = Local<Float32Array>::Cast(xrStateObj->Get(JS_STR("renderHeight")));
            const float renderHeight = TO_FLOAT(renderHeightFloat32Array->Get(0));
            float leftViewport[] = {
              0,
              0,
              renderWidth,
              renderHeight,
            };
            float rightViewport[] = {
              renderWidth,
              0,
              renderWidth,
              renderHeight,
            };

            Local<Float32Array> leftViewMatrixFloat32Array = Local<Float32Array>::Cast(xrStateObj->Get(JS_STR("leftViewMatrix")));
            std::vector<float> leftViewMatrix(16);
            memcpy(leftViewMatrix.data(), (float *)((char *)leftViewMatrixFloat32Array->Buffer()->GetContents().Data() + leftViewMatrixFloat32Array->ByteOffset()), leftViewMatrix.size() * sizeof(leftViewMatrix[0]));

            Local<Float32Array> rightViewMatrixFloat32Array = Local<Float32Array>::Cast(xrStateObj->Get(JS_STR("rightViewMatrix")));
            std::vector<float> rightViewMatrix(16);
            memcpy(rightViewMatrix.data(), (float *)((char *)rightViewMatrixFloat32Array->Buffer()->GetContents().Data() + rightViewMatrixFloat32Array->ByteOffset()), rightViewMatrix.size() * sizeof(rightViewMatrix[0]));

            Local<Value> xrOffsetValue = elementObj->Get(JS_STR("xrOffset"));
            if (TO_BOOL(xrOffsetValue)) {
              Local<Object> xrOffsetObj = Local<Object>::Cast(xrOffsetValue);
              Local<Float32Array> xrOffsetMatrixFloat32Array = Local<Float32Array>::Cast(xrOffsetObj->Get(JS_STR("matrix")));

              std::vector<float> xrOffsetMatrix(16);
              memcpy(xrOffsetMatrix.data(), (float *)((char *)xrOffsetMatrixFloat32Array->Buffer()->GetContents().Data() + xrOffsetMatrixFloat32Array->ByteOffset()), xrOffsetMatrix.size() * sizeof(xrOffsetMatrix[0]));

              leftViewMatrix = multiplyMatrices(leftViewMatrix, xrOffsetMatrix);
              rightViewMatrix = multiplyMatrices(rightViewMatrix, xrOffsetMatrix);
            }

            Local<Float32Array> leftProjectionMatrixFloat32Array = Local<Float32Array>::Cast(xrStateObj->Get(JS_STR("leftProjectionMatrix")));
            float *leftProjectionMatrix = (float *)((char *)leftProjectionMatrixFloat32Array->Buffer()->GetContents().Data() + leftProjectionMatrixFloat32Array->ByteOffset());
            Local<Float32Array> rightProjectionMatrixFloat32Array = Local<Float32Array>::Cast(xrStateObj->Get(JS_STR("rightProjectionMatrix")));
            float *rightProjectionMatrix = (float *)((char *)rightProjectionMatrixFloat32Array->Buffer()->GetContents().Data() + rightProjectionMatrixFloat32Array->ByteOffset());

            layers.push_back(LayerSpec{
              LayerType::IFRAME_2D,
              width,
              height,
              0,
              0,
              tex,
              0,
              { // viewports
                leftViewport,
                rightViewport,
              },
              { // modelView
                leftViewMatrix.data(),
                rightViewMatrix.data(),
              },
              { // projection
                leftProjectionMatrix,
                rightProjectionMatrix,
              }
            });
            break;
          }
          /* case LayerType::RAW_CANVAS: {
            Local<Object> framebufferObj = Local<Object>::Cast(elementObj->Get(JS_STR("framebuffer")));
            GLuint tex = TO_UINT32(framebufferObj->Get(JS_STR("tex")));
            GLuint depthTex = TO_UINT32(framebufferObj->Get(JS_STR("depthTex")));
            GLuint msTex = TO_UINT32(framebufferObj->Get(JS_STR("msTex")));
            GLuint msDepthTex = TO_UINT32(framebufferObj->Get(JS_STR("msDepthTex")));
            int width = TO_INT32(framebufferObj->Get(JS_STR("width")));
            int height = TO_INT32(framebufferObj->Get(JS_STR("height")));

            layers.push_back(LayerSpec{
              LayerType::RAW_CANVAS,
              width,
              height,
              msTex,
              msDepthTex,
              tex,
              depthTex,
              {nullptr,nullptr},
              {nullptr,nullptr},
              {nullptr,nullptr}
            });
            break;
          } */
          default: {
            // layer cannot be composed
            break;
          }
        }
      } else {
        return Nan::ThrowError("WindowSystem::ComposeLayers: invalid layer element");
      }
    }

    if (layers.size() > 0) {
      ComposeLayers(gl, layers);
    }
  } else {
    Nan::ThrowError("WindowSystem::ComposeLayers: invalid arguments");
  }
}

uv_loop_t *GetEventLoop() {
  return node::GetCurrentEventLoop(Isolate::GetCurrent());
}

NAN_METHOD(ClearFramebuffer) {
  GLuint fbo = TO_UINT32(info[0]);
  
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
}

void Decorate(Local<Object> target) {
  Nan::SetMethod(target, "createRenderTarget", CreateRenderTarget);
  Nan::SetMethod(target, "resizeRenderTarget", ResizeRenderTarget);
  Nan::SetMethod(target, "destroyRenderTarget", DestroyRenderTarget);
  Nan::SetMethod(target, "createVrTopRenderTarget", CreateVrTopRenderTarget);
  Nan::SetMethod(target, "createVrCompositorRenderTarget", CreateVrCompositorRenderTarget);
  Nan::SetMethod(target, "bindVrChildFbo", BindVrChildFbo);
  Nan::SetMethod(target, "bindVrChildMsFbo", BindVrChildMsFbo);
  Nan::SetMethod(target, "getSync", GetSync);
  Nan::SetMethod(target, "waitSync", WaitSync);
  Nan::SetMethod(target, "deleteSync", DeleteSync);
  Nan::SetMethod(target, "composeLayers", ComposeLayers);
  Nan::SetMethod(target, "clearFramebuffer", ClearFramebuffer);
}

}

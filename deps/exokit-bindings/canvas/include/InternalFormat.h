#ifndef _INTERNALFORMAT_H_
#define _INTERNALFORMAT_H_

namespace canvas {
  enum InternalFormat {
    NO_FORMAT = 0,
    R8,
    RG8,
    RGB565,
    RGBA4,
    RGBA8,
    RGB8,
    RED_RGTC1,
    RG_RGTC2,
    RGB_DXT1,
    RGBA_DXT5,
    RGB_ETC1,
    LUMINANCE_ALPHA,
    LA44, // not a real OpenGL format
    R32F,
    RGBA5551
  };
};

#endif

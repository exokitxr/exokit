#ifndef _IMAGEFORMAT_H_
#define _IMAGEFORMAT_H_

#define RGBA_TO_RED(v) (v & 0xff)
#define RGBA_TO_GREEN(v) ((v >> 8) & 0xff)
#define RGBA_TO_BLUE(v) ((v >> 16) & 0xff)
#define RGBA_TO_ALPHA(v) (v >> 24)

#define BGRA_TO_RED(v) ((v >> 16) & 0xff)
#define BGRA_TO_GREEN(v) ((v >> 8) & 0xff)
#define BGRA_TO_BLUE(v) (v & 0xff)
#define BGRA_TO_ALPHA(v) (v >> 24)

#define RGB565_TO_RED(v) ((v >> 11) * 255 / 31)
#define RGB565_TO_GREEN(v) (((v >> 5) & 0x3f) * 255 / 63)
#define RGB565_TO_BLUE(v) ((v & 0x1f) * 255 / 31)

#define PACK_RGB565(r, g, b) ((r) | ((g) << 5) | ((b) << 11))
#define PACK_RGB24(r, g, b) ((r) | ((g) << 8) | ((b) << 16))
#define PACK_RGBA32(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))

#endif

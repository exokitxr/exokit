#include <web_color.h>

canvas::web_color canvas::web_color::from_string(const char *str) {
  CSSColorParser::Color color(CSSColorParser::parse(str));
  return web_color(color.r, color.g, color.b, (unsigned char)(color.a * 255.0));
}
#ifndef _WEB_COLOR_H_
#define _WEB_COLOR_H_

#include "csscolorparser.h"

using namespace std;

namespace canvas
{
  struct web_color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    web_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xFF) : r(r), g(g), b(b), a(a) {}

    web_color() : b(0), g(0), r(0), a(0xFF) {}

    web_color(const web_color& val) {
      this->r = val.r;
      this->g = val.g;
      this->b = val.b;
      this->a = val.a;
    }

    web_color &operator=(const web_color& val)
    {
      this->r = val.r;
      this->g = val.g;
      this->b = val.b;
      this->a = val.a;
      return *this;
    }
    static web_color from_string(const char* str);
  };
}

#endif
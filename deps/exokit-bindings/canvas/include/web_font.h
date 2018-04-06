#ifndef _WEB_FONT_H_
#define _WEB_FONT_H_

#include "web_string.h"

using namespace std;

namespace canvas
{
  struct FontDeclaration {
    std::string fontFamily;
    std::string fontStyle;
    std::string fontVariant;
    std::string fontWeight;
    std::string fontSize;
    std::string lineHeight;
  };

  FontDeclaration parse_short_font(const string &val);
}

#endif
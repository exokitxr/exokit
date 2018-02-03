#include <Color.h>

#include <cassert>

using namespace canvas;

Color Color::BLACK(0.0f, 0.0f, 0.0f, 1.0f);
Color Color::WHITE(1.0f, 1.0f, 1.0f, 1.0f);
Color Color::RED(1.0f, 0.0f, 0.0f, 1.0f);

static int get_xdigit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else {
    return 0;
  }
}

Color &
Color::operator=(const std::string & s) {
  setValue(s);
  return *this;
}

void
Color::setValue(const std::string & s) {
  if (s == "black") {
    red = green = blue = 0;
    alpha = 1.0f;
  } else if (s == "white") {
    red = green = blue = alpha = 1.0f;
  } else if (s.compare(0, 5, "rgba(") == 0) {
    assert(0);    
  } else {
    unsigned int pos = 0;
    if (s.size() && s[0] == '#') pos++;
    if (s.size() >= pos + 6) {
      red = (get_xdigit(s[pos]) * 16 + get_xdigit(s[pos+1])) / 255.0f;
      green = (get_xdigit(s[pos+2]) * 16 + get_xdigit(s[pos+3])) / 255.0f;
      blue = (get_xdigit(s[pos+4]) * 16 + get_xdigit(s[pos+5])) / 255.0f;
    } else if (s.size() >= pos + 3) {
      int r = get_xdigit(s[pos]);
      int g = get_xdigit(s[pos+1]);
      int b = get_xdigit(s[pos+2]);
      red = (r * 16 + r) / 255.0f;
      green = (g * 16 + g) / 255.0f;
      blue = (b * 16 + b) / 255.0f;
    } else {
      red = green = blue = 0;
    }
    alpha = 1.0f;
  }
}

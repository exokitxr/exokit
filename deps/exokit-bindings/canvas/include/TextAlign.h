#ifndef _CANVAS_TEXTALIGN_H_
#define _CANVAS_TEXTALIGN_H_

#include "Attribute.h"

namespace canvas {
  enum TextAlign {
    ALIGN_START = 1,
    ALIGN_END,
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
  };

  class TextAlignAttribute : public AttributeBase {
  public:
  TextAlignAttribute(GraphicsState * _context, TextAlign _value = ALIGN_LEFT) : AttributeBase(_context), value(_value) { }
  TextAlignAttribute(GraphicsState * _context, const std::string & _value) : AttributeBase(_context) { setValue(_value.c_str()); }
  TextAlignAttribute(GraphicsState * _context, const char * _value) : AttributeBase(_context) { setValue(_value); }

  TextAlignAttribute(const TextAlignAttribute & other)
    : AttributeBase(other), value(other.value) { }

    TextAlignAttribute & operator=(const TextAlignAttribute & other) { value = other.value; return *this; }
    TextAlignAttribute & operator=(const TextAlign & other) { value = other; return *this; }
    TextAlignAttribute & operator=(const std::string & _value) { setValue(_value.c_str()); return *this; }
    TextAlignAttribute & operator=(const char * _value) { setValue(_value); return *this; }

    TextAlign get() const { return value; }
    
  private:
    void setValue(const char * _value) {
      if (strcmp(_value, "start") == 0) value = ALIGN_START;
      else if (strcmp(_value, "end") == 0) value = ALIGN_END;
      else if (strcmp(_value, "left") == 0) value = ALIGN_LEFT;
      else if (strcmp(_value, "center") == 0) value = ALIGN_CENTER;
      else if (strcmp(_value, "right") == 0) value = ALIGN_RIGHT;
      else {
	value = ALIGN_START;
      }
    };

    TextAlign value;
  };
};

#endif

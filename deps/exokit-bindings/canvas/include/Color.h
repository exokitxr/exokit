#ifndef _COLOR_H_
#define _COLOR_H_

#include <string>

namespace canvas {
  class Color {
  public:
    static Color BLACK, WHITE, RED;
    
  Color() : red(0.0f), green(0.0f), blue(0.0f), alpha(1.0f) { }
    Color(const std::string & s)
      : red(0.0f), green(0.0f), blue(0.0f), alpha(1.0f) {
      setValue(s);
    }
  Color(float _red, float _green, float _blue, float _alpha)
    : red(_red), green(_green), blue(_blue), alpha(_alpha) { }
    
    Color & operator=(const std::string & s);
    
	Color mix(float f, const Color & other) {
		return Color(f * other.red + (1 - f) * red,
			f * other.green + (1 - f) * green,
			f * other.blue + (1 - f) * blue,
			f * other.alpha + (1 - f) * alpha);
	}
    float red, green, blue, alpha;
    
  private:
    void setValue(const std::string & s);
  };

  // shim for Attribute<Color>
  inline void assignValue(Color & a, const std::string & b) { a = b; }
};

#endif

#ifndef _CANVAS_FONTWEIGHT_H_
#define _CANVAS_FONTWEIGHT_H_

#include <cstring>

class FontWeight {
 public:
  enum Weight {
    NORMAL = 1,
    BOLD,
    BOLDER,
    LIGHTER
#if 0
    100
    200
    300
    400
    500
    600
    700
    800
    900
#endif
  };

 FontWeight() : value(NORMAL) { }
  FontWeight(const std::string & s) {
    setValue(s.c_str());
  }
  FontWeight & operator=(const std::string & _value) { setValue(_value.c_str()); return *this; }
  FontWeight & operator=(const char * _value) { setValue(_value); return *this; }

  Weight getValue() const { return value; }
  bool isBold() const { return value == BOLD || value == BOLDER; }
  
 private:
  void setValue(const char * _value) {
    if (strcmp(_value, "normal") == 0) {
      value = NORMAL;
    } else if (strcmp(_value, "bold") == 0) {
      value = BOLD;
    } else if (strcmp(_value, "bolder") == 0) {
      value = BOLDER;
    } else if (strcmp(_value, "lighter") == 0) {
      value = LIGHTER;
    } else {
      value = NORMAL;
    }    
  }
  
 private:
  Weight value;
};

#endif

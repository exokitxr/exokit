#ifndef _ATTRIBUTE_H_
#define _ATTRIBUTE_H_

#include <string>
#include <stdlib.h>

namespace canvas {
  class GraphicsState;

  inline void assignValue(float & a, const std::string & b) {
    a = atof(b.c_str());
  }
  inline void assignValue(bool & a, const std::string & b) {
    a = !b.empty() && b != "0" && b != "false";
  }

  class AttributeBase {
  public:
    AttributeBase(GraphicsState * _context) : context(_context) { }
    AttributeBase(const AttributeBase & other) : context(other.context) { }

  protected:
    GraphicsState & getContext() { return *context; }
    
  private:
    GraphicsState * context;
  };

  template<class T>
  class Attribute : public AttributeBase {
  public:
    Attribute(GraphicsState * _context) : AttributeBase(_context), value() { }
    Attribute(GraphicsState * _context, T _value) : AttributeBase(_context), value(_value) { }
    Attribute(const Attribute<T> & other) : AttributeBase(other), value(other.value) { }
  
    Attribute<T> & operator=(const Attribute<T> & other) {
      value = other.value;
      return *this;
    }
    Attribute<T> & operator=(T _value) {
      value = _value;
      return *this;
    }
    Attribute<T> & operator=(const std::string & s) {
      assignValue(value, s);
      return *this;
    }
    GraphicsState & operator()(T _value) {
      value = _value;
      return getContext();
    }
    GraphicsState & operator()(const std::string & s) {
      assignValue(value, s);
      return getContext();
    }

    T get() const { return value; }
    
  private:
    T value;
  };
};

#endif

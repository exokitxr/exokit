#ifndef _CANVAS_MATRIX_H_
#define _CANVAS_MATRIX_H_

#include <Point.h>
#include <cmath>

namespace canvas {
  class Matrix {
  public:
  Matrix() : a(1.0), b(0.0), c(0.0), d(1.0), e(0.0), f(0.0) { }
  Matrix(double _a, double _b, double _c, double _d, double _e, double _f)
    : a(_a), b(_b), c(_c), d(_d), e(_e), f(_f) { }
    
    Matrix operator* (const Matrix & other) {
      return multiply(*this, other);    
    }
    
    const Matrix & operator*= (const Matrix & other) {
      *this = multiply(*this, other);
      return *this;
    }
    
    Point multiply(double x, double y) const {
      return Point( x * a + y * c + e,
		    x * b + y * d + f
		    );    
    }
    
    Point multiply(const Point & p) const {
      return multiply(p.x, p.y);
    }
    
    double transformAngle(double alpha) {
      double x = cos(alpha), y = sin(alpha);
      return atan2(x * b + y * d, x * a + y * c);
    }
    
  private:
    static Matrix multiply(const Matrix & A, const Matrix & B) {
      return Matrix( A.a * B.a + A.c * B.b,
		     A.b * B.a + A.d * B.b,
		     A.a * B.c + A.c * B.d,
		     A.b * B.c + A.d * B.d,
		     A.a * B.e + A.c * B.f + A.e,
		     A.b * B.e + A.d * B.f + A.f
		     );
    }
    
    double a, b, c, d, e, f;
  };
};

#endif

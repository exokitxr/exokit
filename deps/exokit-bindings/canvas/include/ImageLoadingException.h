#ifndef _IMAGELOADINGEXCEPTION_H_
#define _IMAGELOADINGEXCEPTION_H_

#include <exception>

namespace canvas {
  class ImageLoadingException : public std::exception {
  public:
    ImageLoadingException(const char * _reason) : reason(_reason) { }
	
    const char * what() const noexcept(true) override { return reason.c_str(); }
	
  private:
    std::string reason;
  };
};

#endif

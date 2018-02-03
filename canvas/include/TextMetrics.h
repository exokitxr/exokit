#ifndef _TEXTMETRICS_H_
#define _TEXTMETRICS_H_

namespace canvas {
  class TextMetrics {
  public:
  TextMetrics() : width(0) { }
  TextMetrics(float _width) : width(_width), fontBoundingBoxDescent(0), fontBoundingBoxAscent(0) { }
  TextMetrics(float _width, float _fontBoundingBoxDescent, float _fontBoundingBoxAscent) :
  	width(_width), fontBoundingBoxDescent(_fontBoundingBoxDescent), fontBoundingBoxAscent(_fontBoundingBoxAscent)  { }
    
    float width;
    float fontBoundingBoxDescent;
    float fontBoundingBoxAscent;

#if 0
    float ideographicBaseline;
    float alphabeticBaseline;
    float hangingBaseline;
    float emHeightDescent;
    float emHeightAscent;
    float actualBoundingBoxDescent;
    float actualBoundingBoxAscent;
    float actualBoundingBoxRight;
    float actualBoundingBoxLeft;
#endif
  };
};

#endif

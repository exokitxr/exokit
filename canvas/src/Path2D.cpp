#include <Path2D.h>

#include <cmath>

using namespace canvas;

void
Path2D::arc(const Point & p, double radius, double sa, double ea, bool anticlockwise) {
  data.push_back(PathComponent(PathComponent::ARC, p.x, p.y, radius, sa, ea, anticlockwise));
  current_point = Point(p.x + radius * cos(ea), p.y + radius * sin(ea));
}

// Implementation by node-canvas (Node canvas is a Cairo backed Canvas implementation for NodeJS)
// Original implementation influenced by WebKit.
void
Path2D::arcTo(const Point & p1, const Point & p2, double radius) {
  Point p0 = current_point; // current point may be modified so make a copy
  
  if ((p1.x == p0.x && p1.y == p0.y) || (p1.x == p2.x && p1.y == p2.y) || radius == 0.f) {
    lineTo(p1);
    // p2?
    return;
  }

  Point p1p0((p0.x - p1.x), (p0.y - p1.y));
  Point p1p2((p2.x - p1.x), (p2.y - p1.y));
  float p1p0_length = sqrt(p1p0.x * p1p0.x + p1p0.y * p1p0.y);
  float p1p2_length = sqrt(p1p2.x * p1p2.x + p1p2.y * p1p2.y);

  double cos_phi = (p1p0.x * p1p2.x + p1p0.y * p1p2.y) / (p1p0_length * p1p2_length);
  // all points on a line logic
  if (-1 == cos_phi) {
    lineTo(p1);
    // p2?
    return;
  }

  if (1 == cos_phi) {
    // add infinite far away point
    unsigned int max_length = 65535;
    double factor_max = max_length / p1p0_length;
    Point ep((p0.x + factor_max * p1p0.x), (p0.y + factor_max * p1p0.y));
    lineTo(ep);
    return;
  }

  double tangent = radius / tan(acos(cos_phi) / 2);
  double factor_p1p0 = tangent / p1p0_length;
  Point t_p1p0((p1.x + factor_p1p0 * p1p0.x), (p1.y + factor_p1p0 * p1p0.y));

  Point orth_p1p0(p1p0.y, -p1p0.x);
  double orth_p1p0_length = sqrt(orth_p1p0.x * orth_p1p0.x + orth_p1p0.y * orth_p1p0.y);
  double factor_ra = radius / orth_p1p0_length;

  double cos_alpha = (orth_p1p0.x * p1p2.x + orth_p1p0.y * p1p2.y) / (orth_p1p0_length * p1p2_length);
  if (cos_alpha < 0.f) {
    orth_p1p0 = Point(-orth_p1p0.x, -orth_p1p0.y);
  }

  Point p((t_p1p0.x + factor_ra * orth_p1p0.x), (t_p1p0.y + factor_ra * orth_p1p0.y));

  orth_p1p0 = Point(-orth_p1p0.x, -orth_p1p0.y);
  double sa = acos(orth_p1p0.x / orth_p1p0_length);
  if (orth_p1p0.y < 0.f) {
    sa = 2 * M_PI - sa;
  }

  bool anticlockwise = false;

  double factor_p1p2 = tangent / p1p2_length;
  Point t_p1p2((p1.x + factor_p1p2 * p1p2.x), (p1.y + factor_p1p2 * p1p2.y));
  Point orth_p1p2((t_p1p2.x - p.x),(t_p1p2.y - p.y));
  double orth_p1p2_length = sqrt(orth_p1p2.x * orth_p1p2.x + orth_p1p2.y * orth_p1p2.y);
  double ea = acos(orth_p1p2.x / orth_p1p2_length);

  if (orth_p1p2.y < 0) ea = 2 * M_PI - ea;
  if ((sa > ea) && ((sa - ea) < M_PI)) anticlockwise = true;
  if ((sa < ea) && ((ea - sa) > M_PI)) anticlockwise = true;

  // cerr << "ARC " << int(t_p1p0.x) << " " << int(t_p1p0.y) << " " << int(p.x) << " " << int(p.y) << " " << radius << " " << int(sa * 180.0 / M_PI) << " " << int(ea * 180.0 / M_PI) << " " << (anticlockwise ? "acw" : "cw") << endl;

  lineTo(t_p1p0);
  arc(p, radius, sa, ea, anticlockwise); // && M_PI * 2 != radius);
  // current_point = p2;
}

static inline float determinant(float x1, float y1, float x2, float y2) {
  return x1 * y2 - x2 * y1;
}

#define MU_PATH_RECURSION_LIMIT 8
#define MU_PATH_DISTANCE_EPSILON 1.0f
#define MU_PATH_COLLINEARITY_EPSILON FLT_EPSILON
#define MU_PATH_MIN_STEPS_FOR_CIRCLE 20.0f
#define MU_PATH_MAX_STEPS_FOR_CIRCLE 64.0f

void Path2D::quadraticCurveTo(float cpx, float cpy, float x, float y, float scale) {
  float distanceTolerance = MU_PATH_DISTANCE_EPSILON / scale;
  distanceTolerance *= distanceTolerance;

  recursiveQuadratic(current_point.x, current_point.y, cpx, cpy, x, y, 0, distanceTolerance);
}

void Path2D::recursiveQuadratic(float x1, float y1, float x2, float y2, float x3, float y3, int level, float distanceTolerance) {
  // Based on http://www.antigrain.com/research/adaptive_bezier/index.html

  // Calculate all the mid-points of the line segments
  float x12 = (x1 + x2) / 2;
  float y12 = (y1 + y2) / 2;
  float x23 = (x2 + x3) / 2;
  float y23 = (y2 + y3) / 2;
  float x123 = (x12 + x23) / 2;
  float y123 = (y12 + y23) / 2;

  float dx = x3 - x1;
  float dy = y3 - y1;
  float d = fabsf(((x2 - x3) * dy - (y2 - y3) * dx));

  if (d > MU_PATH_COLLINEARITY_EPSILON) {
    // Regular care
    if (d * d <= distanceTolerance * (dx * dx + dy * dy)) {
      lineTo(Point(x123, y123));
      return;
    }
  }
  else {
    // Collinear case
    dx = x123 - (x1 + x3) / 2;
    dy = y123 - (y1 + y3) / 2;
    if (dx * dx + dy * dy <= distanceTolerance) {
      lineTo(Point(x123, y123));
      return;
    }
  }

  if (level <= MU_PATH_RECURSION_LIMIT) {
    // Continue subdivision
    recursiveQuadratic(x1, y1, x12, y12, x123, y123, level + 1, distanceTolerance);
    recursiveQuadratic(x123, y123, x23, y23, x3, y3, level + 1, distanceTolerance);
  }
}

#if 0
// returns vector cross product of vectors p1p2 and p1p3 using Cramer's rule
static inline float crossProduct(const glm::vec2 & p1, const glm::vec2 & p2, const glm::vec2 & p3) {
  float det_p2p3 = determinant(p2.x, p2.y, p3.x, p3.y);
  float det_p1p3 = determinant(p1.x, p1.y, p3.x, p3.y);
  float det_p1p2 = determinant(p1.x, p1.y, p2.x, p2.y);
  return det_p2p3 - det_p1p3 + det_p1p2;
}

// The winding number method has been used here. It counts the number
// of times a polygon winds around the point.  If the result is 0, the
// points is outside the polygon.
bool
Path2D::isInside(float x, float y) const {
  glm::vec2 point(x, y);
  int wn = 0;
  for (unsigned int i = 0; i < data.size(); i++) {
    glm::vec2 v1;
    if (i == 0) {
      v1 = glm::vec2( (float)data.back().x0, (float)data.back().y0 );	  
    } else {
      v1 = glm::vec2( (float)data[i - 1].x0, (float)data[i - 1].y0 );	  
    }
    glm::vec2 v2( (float)data[i].x0, (float)data[i].y0 );
    if (v1.y <= point.y) { // start y <= P.y
      if (v2.y > point.y) { // an upward crossing
	if (crossProduct(v1, v2, point) > 0) {
	  // point left of edge
	  wn++; // have a valid up intersect
	}
      }
    } else { // start y > P.y (no test needed)
      if (v2.y <= point.y) { // a downward crossing
	if (crossProduct(v1, v2, point) < 0) {
	  // point right of edge
	  wn--; // have a valid down intersect
	}
      }
    }
  }
  
  bool is_inside = wn != 0 ? true : false;
  
  // fprintf(stderr, "inside test for polygon %Ld: %s\n", id, is_inside ? "true" : "false" );
  return is_inside;
}
#endif

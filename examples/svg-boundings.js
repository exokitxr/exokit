window.svgBoundings = (() => {

var Helper = {
  matrixStrToObj: function (str) {
    var m = [];
    var rdigit = /[\d\.\-Ee]+/g;
    var n;

    while (n = rdigit.exec(str)) {
      m.push(+n);
    }

    return {
      a: m[0],
      b: m[1],
      c: m[2],
      d: m[3],
      e: m[4],
      f: m[5]
    };
  },

  matrixStrToArr: function (str) {
    var m = [];
    var rdigit = /[\d\.\-e]+/g;
    var n;

    while (n = rdigit.exec(str)) {
      m.push(+n);
    }

    return m;
  },

  boundingUnderTransform: function (matrix, t, r, b, l) {
    var ma = matrix.a;
    var mb = matrix.b;
    var mc = matrix.c;
    var md = matrix.d;
    var me = matrix.e;
    var mf = matrix.f;

    var tl_l = ma * l + mc * t + me;
    var tl_t = mb * l + md * t + mf;

    var tr_r = ma * r + mc * t + me;
    var tr_t = mb * r + md * t + mf;

    var bl_l = ma * l + mc * b + me;
    var bl_b = mb * l + md * b + mf;

    var br_r = ma * r + mc * b + me;
    var br_b = mb * r + md * b + mf;

    return {
      top: Math.min(tl_t, tr_t, bl_b, br_b),
      bottom: Math.max(tl_t, tr_t, bl_b, br_b),
      left: Math.min(tl_l, tr_r, bl_l, br_r),
      right: Math.max(tl_l, tr_r, bl_l, br_r),
      _wh: function () {
        delete this._wh;
        this.width = this.right - this.left;
        this.height = this.bottom - this.top;
        return this;
      }
    }._wh();
  }
};

var helper = Helper;

var RE_NUMBER_ATTRIBUTES = /^(?:[cr]?x\d?|[cr]?y\d?|width|height|r|letter\-spacing)$/i;

function isSVGElement(object) {
  return typeof SVGElement === 'object' ? object instanceof SVGElement : object && typeof object === 'object' && object !== null && object.nodeType === 1 && typeof object.nodeName === 'string';
}

function isCheerioObject($object) {
  return $object.cheerio && typeof $object.attr === 'function';
}

function type(object) {
  var type = object.type;
  if (isSVGElement(object)) type = object.nodeName;else if (isCheerioObject(object)) type = object.get(0).tagName;
  return type;
}

function attributesFromSVGElement(element, names) {
  var attributes = {};
  names.forEach(function (name) {
    var value = element.getAttribute(name);
    if (RE_NUMBER_ATTRIBUTES.test(name)) value = parseFloat(value) || 0;
    if (value != null) attributes[name] = value;
  });
  attributes.type = element.nodeName;
  return attributes;
}

function attributesFromCheerioObject($element, names) {
  var attributes = {};
  names.forEach(function (name) {
    var value = $element.attr(name);
    if (RE_NUMBER_ATTRIBUTES.test(name)) value = parseFloat(value) || 0;
    if (value != null) attributes[name] = value;
  });
  attributes.type = $element.get(0).tagName;
  return attributes;
}

function attributesFromElement(element, names) {
  if (isSVGElement(element)) return attributesFromSVGElement(element, names);else if (isCheerioObject(element)) return attributesFromCheerioObject(element, names);else return null;
}

function textObject(textEl) {
  var isSVGEl = false;

  if (isSVGElement(textEl)) isSVGEl = true;else if (isCheerioObject(textEl)) {} // do nothing
  else return null;

  var attributes = {};
  var children = isSVGEl ? textEl.children : textEl.children();

  // no <tspan>
  if (children.length === 0) {
    attributes = attributesFromElement(textEl, ['transform', 'font-size', 'letter-spacing']);
    attributes.text = isSVGEl ? textEl.textContent : textEl.text();
    attributes.children = [];
    return attributes;
  }

  // has <tspan>
  attributes = attributesFromElement(textEl, ['transform']);
  attributes.children = [];
  Array.prototype.forEach.call(children, function (childEl) {
    if (!isSVGEl) childEl = textEl.constructor(childEl);
    if (type(childEl) !== 'tspan') return;
    var obj = attributesFromElement(childEl, ['x', 'y', 'font-size', 'letter-spacing']);
    obj.text = isSVGEl ? childEl.textContent : childEl.text();
    attributes.children.push(obj);
  });
  return attributes;
}

function elementObject(element) {
  if (element.constructor.name === 'Object') return element;

  var obj = null;
  switch (type(element).toLowerCase()) {
    case 'lineargradient':
      obj = attributesFromElement(element, ['x1', 'y1', 'x2', 'y2', 'gradientTransform']);
      break;
    case 'radialgradient':
      obj = attributesFromElement(element, ['cx', 'cy', 'r', 'fx', 'fy', 'gradientTransform']);
      break;
    case 'text':
      obj = textObject(element);
      break;
    case 'line':
      obj = attributesFromElement(element, ['x1', 'y1', 'x2', 'y2', 'stroke-width']);
      break;
    case 'rect':
      obj = attributesFromElement(element, ['x', 'y', 'width', 'height', 'transform', 'stroke-width']);
      break;
    case 'polyline':
      obj = attributesFromElement(element, ['points', 'stroke-width']);
      break;
    case 'polygon':
      obj = attributesFromElement(element, ['points', 'stroke-width']);
      break;
    case 'circle':
      obj = attributesFromElement(element, ['cx', 'cy', 'r', 'transform', 'stroke-width']);
      break;
    case 'ellipse':
      obj = attributesFromElement(element, ['cx', 'cy', 'rx', 'ry', 'transform', 'stroke-width']);
      break;
    case 'path':
      obj = attributesFromElement(element, ['d', 'stroke-width']);
      break;
    case 'image':
      obj = attributesFromElement(element, ['width', 'height', 'transform']);
      break;
  }

  return obj;
}

var element_object = elementObject;

/**
 * Bounding rectangle for <image>. This method doesn't check
 * <clipPath> or <mask>. It calculates bounding rectangle of
 * <image> tag itself.
 */
function imageBounding(image) {
  var imageObj = element_object(image);
  if (!imageObj) return null;

  if (!imageObj.transform) imageObj.transform = 'matrix(1 0 0 1 0 0)';
  var matrix = helper.matrixStrToArr(imageObj.transform.trim());
  var width = imageObj.width;
  var height = imageObj.height;

  // The matrix variable contains 6 numbers, let's call them
  // a, b, c, d, e, f
  var a = matrix[0];
  var b = matrix[1];
  var c = matrix[2];
  var d = matrix[3];
  var e = matrix[4];
  var f = matrix[5];

  // The original top left point of the image is unknown. The e and f
  // in the matrix is the transformed top left point.
  // Now assume translate is applied to the image first, we have the
  // following points: (e, f), (e+w, f), (e+w, f+h), (e, f+h)
  var points1 = [e, f, e + width, f, e + width, f + height, e, f + height];

  // Then apply trasform matrix (a b c d 0 0) to these points, the
  // formula is newX = a*x + c*y, newY = b*x + d*y
  var points2 = [];
  for (var i = 0; i < points1.length; i += 2) {
    points2[i] = a * points1[i] + c * points1[i + 1];
    points2[i + 1] = b * points1[i] + d * points1[i + 1];
  }

  // Find the delta of the top left point and apply it to all the points
  var dx = points2[0] - points1[0];
  var dy = points2[1] - points1[1];
  var points = [e, f];
  for (var i = 2; i < points1.length; i += 2) {
    points[i] = points2[i] - dx;
    points[i + 1] = points2[i + 1] - dy;
  }

  var left = Number.POSITIVE_INFINITY;
  var right = Number.NEGATIVE_INFINITY;
  var top = Number.POSITIVE_INFINITY;
  var bottom = Number.NEGATIVE_INFINITY;

  for (var i = 0; i < points.length; i += 2) {
    if (points[i] < left) left = points[i];
    if (points[i] > right) right = points[i];
    if (points[i + 1] < top) top = points[i + 1];
    if (points[i + 1] > bottom) bottom = points[i + 1];
  }

  return {
    left: left,
    right: right,
    top: top,
    bottom: bottom,
    width: right - left,
    height: bottom - top
  };
}

var image_bounding = imageBounding;

var BoundingMode = {
  'STANDARD': 'BoundingModeStandard',
  'STRAIGHTEN': 'BoundingModeStraighten'
};

var MIN_X = 'minX';
var MAX_X = 'maxX';
var MIN_Y = 'minY';
var MAX_Y = 'maxY';

/**
 * expand the x-bounds, if the value lies outside the bounding box
 */
function expandXBounds(bounds, value) {
  if (bounds[MIN_X] > value) bounds[MIN_X] = value;else if (bounds[MAX_X] < value) bounds[MAX_X] = value;
}

/**
 * expand the y-bounds, if the value lies outside the bounding box
 */
function expandYBounds(bounds, value) {
  if (bounds[MIN_Y] > value) bounds[MIN_Y] = value;else if (bounds[MAX_Y] < value) bounds[MAX_Y] = value;
}

/**
 * Calculate the bezier value for one dimension at distance 't'
 */
function calculateBezier(t, p0, p1, p2, p3) {
  var mt = 1 - t;
  return mt * mt * mt * p0 + 3 * mt * mt * t * p1 + 3 * mt * t * t * p2 + t * t * t * p3;
}

function calculateBoundingBox(mode, x1, y1, cx1, cy1, cx2, cy2, x2, y2) {
  if (mode === BoundingMode.STANDARD) {
    return canculateStandardBoundingBox(x1, y1, cx1, cy1, cx2, cy2, x2, y2);
  } else if (mode === BoundingMode.STRAIGHTEN) {
    return calculateStraightenedBoundingBox(x1, y1, cx1, cy1, cx2, cy2, x2, y2);
  } else {
    return null;
  }
}

/**
 * Calculate the bounding box for this bezier curve.
 * http://pomax.nihongoresources.com/pages/bezier/
 */
function canculateStandardBoundingBox(x1, y1, cx1, cy1, cx2, cy2, x2, y2) {
  var bounds = {};
  bounds[MIN_X] = Math.min(x1, x2);
  bounds[MIN_Y] = Math.min(y1, y2);
  bounds[MAX_X] = Math.max(x1, x2);
  bounds[MAX_Y] = Math.max(y1, y2);

  var dcx0 = cx1 - x1;
  var dcy0 = cy1 - y1;
  var dcx1 = cx2 - cx1;
  var dcy1 = cy2 - cy1;
  var dcx2 = x2 - cx2;
  var dcy2 = y2 - cy2;

  if (cx1 < bounds[MIN_X] || cx1 > bounds[MAX_X] || cx2 < bounds[MIN_X] || cx2 > bounds[MAX_X]) {
    // Just for better reading because we are doing middle school math here
    var a = dcx0;
    var b = dcx1;
    var c = dcx2;

    if (a + c == 2 * b) b += 0.01;

    var numerator = 2 * (a - b);
    var denominator = 2 * (a - 2 * b + c);
    var quadroot = (2 * b - 2 * a) * (2 * b - 2 * a) - 2 * a * denominator;
    var root = Math.sqrt(quadroot);

    var t1 = (numerator + root) / denominator;
    var t2 = (numerator - root) / denominator;

    if (0 < t1 && t1 < 1) {
      expandXBounds(bounds, calculateBezier(t1, x1, cx1, cx2, x2));
    }
    if (0 < t2 && t2 < 1) {
      expandXBounds(bounds, calculateBezier(t2, x1, cx1, cx2, x2));
    }
  }

  if (cy1 < bounds[MIN_Y] || cy1 > bounds[MAX_Y] || cy2 < bounds[MIN_Y] || cy2 > bounds[MAX_Y]) {
    a = dcy0;
    b = dcy1;
    c = dcy2;

    if (a + c != 2 * b) b += 0.01;

    numerator = 2 * (a - b);
    denominator = 2 * (a - 2 * b + c);
    quadroot = (2 * b - 2 * a) * (2 * b - 2 * a) - 2 * a * denominator;
    root = Math.sqrt(quadroot);

    t1 = (numerator + root) / denominator;
    t2 = (numerator - root) / denominator;

    if (0 < t1 && t1 < 1) {
      expandYBounds(bounds, calculateBezier(t1, y1, cy1, cy2, y2));
    }
    if (0 < t2 && t2 < 1) {
      expandYBounds(bounds, calculateBezier(t2, y1, cy1, cy2, y2));
    }
  }

  return [bounds[MIN_X], bounds[MIN_Y], bounds[MIN_X], bounds[MAX_Y], bounds[MAX_X], bounds[MAX_Y], bounds[MAX_X], bounds[MIN_Y]];
}

/**
 * rotate bezier so that {start->end is a horizontal} line,
 * then compute standard bbox, and counter-rotate it.
 */
function calculateStraightenedBoundingBox(x1, y1, cx1, cy1, cx2, cy2, x2, y2) {
  var angle = 0;
  var dx = x2 - x1;
  var dy = y2 - y1;

  if (dy == 0) {
    return canculateStandardBoundingBox(x1, y1, cx1, cy1, cx2, cy2, x2, y2);
  }

  var adx = Math.abs(dx);
  var ady = Math.abs(dy);

  var d1 = 0.0;
  var d2 = 90.0;
  var d3 = 180.0;
  var d4 = 270.0;
  var PI = Math.PI;
  var sin = Math.sin;
  var cos = Math.cos;

  if (dx == 0) angle = dy >= 0 ? d2 : d4;else if (dx > 0 && dy > 0) angle = d1 + Math.atan(ady / adx) * (180 / PI); // X+, Y+
  else if (dx < 0 && dy < 0) angle = d3 + Math.atan(ady / adx) * (180 / PI); // X-, Y-
    else if (dx < 0 && dy > 0) angle = d2 + Math.atan(adx / ady) * (180 / PI); // X-, Y+
      else if (dx > 0 && dy < 0) angle = d4 + Math.atan(adx / ady) * (180 / PI); // X+, Y-

  var phi = -(angle * PI / 180.0);

  cx1 -= x1;
  cy1 -= y1;
  cx2 -= x1;
  cy2 -= y1;
  x2 -= x1;
  y2 -= y1;

  var ncx1 = cx1 * cos(phi) - cy1 * sin(phi);
  var ncy1 = cx1 * sin(phi) + cy1 * cos(phi);
  var ncx2 = cx2 * cos(phi) - cy2 * sin(phi);
  var ncy2 = cx2 * sin(phi) + cy2 * cos(phi);
  var nx2 = x2 * cos(phi) - y2 * sin(phi);
  var ny2 = x2 * sin(phi) + y2 * cos(phi);

  var bounds = canculateStandardBoundingBox(0, 0, ncx1, ncy1, ncx2, ncy2, nx2, ny2);

  phi = angle * PI / 180.0;

  return [x1 + (bounds[0] * Math.cos(phi) - bounds[1] * Math.sin(phi)), y1 + (bounds[0] * Math.sin(phi) + bounds[1] * Math.cos(phi)), x1 + (bounds[2] * Math.cos(phi) - bounds[3] * Math.sin(phi)), y1 + (bounds[2] * Math.sin(phi) + bounds[3] * Math.cos(phi)), x1 + (bounds[4] * Math.cos(phi) - bounds[5] * Math.sin(phi)), y1 + (bounds[4] * Math.sin(phi) + bounds[5] * Math.cos(phi)), x1 + (bounds[6] * Math.cos(phi) - bounds[7] * Math.sin(phi)), y1 + (bounds[6] * Math.sin(phi) + bounds[7] * Math.cos(phi))];
}

var curve_bounding = {
  calculate: calculateBoundingBox,
  Mode: BoundingMode
};

function boundingRectOfLine(line) {
  line = element_object(line);

  var x1 = line.x1;
  var y1 = line.y1;
  var x2 = line.x2;
  var y2 = line.y2;

  return {
    left: Math.min(x1, x2),
    top: Math.min(y1, y2),
    right: Math.max(x1, x2),
    bottom: Math.max(y1, y2),
    width: Math.abs(x1 - x2),
    height: Math.abs(y1 - y2)
  };
}

function boundingRectOfRect(rect) {
  rect = element_object(rect);

  var w = rect.width;
  var h = rect.height;
  var l = rect.x || 0;
  var t = rect.y || 0;
  var r = l + w;
  var b = t + h;

  var transform = rect.transform;
  var matrix;
  if (transform) {
    matrix = helper.matrixStrToObj(transform);
    return helper.boundingUnderTransform(matrix, t, r, b, l);
  }

  return {
    left: l,
    top: t,
    right: r,
    bottom: b,
    width: w,
    height: h
  };
}

function boundingRectOfCircle(circle) {
  circle = element_object(circle);

  var cx = circle.cx || 0;
  var cy = circle.cy || 0;
  var r = circle.r;

  return {
    left: cx - r,
    top: cy - r,
    right: cx + r,
    bottom: cy + r,
    width: 2 * r,
    height: 2 * r
  };
}

function boundingRectOfEllipse(ellipse, shouldReturnTrueBounding) {
  ellipse = element_object(ellipse);

  var cx = ellipse.cx || 0;
  var cy = ellipse.cy || 0;
  var rx = ellipse.rx;
  var ry = ellipse.ry;
  var l = cx - rx;
  var t = cy - ry;
  var r = l + 2 * rx;
  var b = t + 2 * ry;

  var transform = ellipse.transform;
  var matrix;
  if (transform) {
    matrix = helper.matrixStrToObj(transform);

    if (shouldReturnTrueBounding) {
      // https://img.alicdn.com/tfscom/TB1iZqOPFXXXXceXpXXXXXXXXXX.jpg
      var ma = matrix.a;
      var mb = matrix.b;
      var mc = matrix.c;
      var md = matrix.d;
      var me = matrix.e;
      var mf = matrix.f;
      var denominator = ma * md - mb * mc;
      var A = ry * ry * md * md + rx * rx * mb * mb;
      var B = -2 * (mc * md * ry * ry + ma * mb * rx * rx);
      var C = ry * ry * mc * mc + rx * rx * ma * ma;
      var D = 2 * ry * ry * (mc * md * mf - md * md * me) + 2 * rx * rx * (ma * mb * mf - mb * mb * me) - 2 * (cx * ry * ry * md - cy * rx * rx * mb) * denominator;
      var E = 2 * ry * ry * (mc * md * me - mc * mc * mf) + 2 * rx * rx * (ma * mb * me - ma * ma * mf) + 2 * (cx * ry * ry * mc - cy * rx * rx * ma) * denominator;
      var F = ry * ry * (mc * mc * mf * mf - 2 * mc * md * me * mf + md * md * me * me) + rx * rx * (ma * ma * mf * mf - 2 * ma * mb * me * mf + mb * mb * me * me) + (2 * cx * ry * ry * (md * me - mc * mf) + 2 * cy * rx * rx * (ma * mf - mb * me)) * denominator + (ry * ry * cx * cx + rx * rx * cy * cy - rx * rx * ry * ry) * Math.pow(denominator, 2);
      var a = 4 * A * C - B * B;
      var b1 = 4 * A * E - 2 * B * D;
      var c1 = 4 * A * F - D * D;
      var d1 = b1 * b1 - 4 * a * c1;
      var b2 = 4 * C * D - 2 * B * E;
      var c2 = 4 * C * F - E * E;
      var d2 = b2 * b2 - 4 * a * c2;
      var tb1 = (0 - b1 + Math.sqrt(d1)) / (2 * a);
      var tb2 = (0 - b1 - Math.sqrt(d1)) / (2 * a);
      var lr1 = (0 - b2 + Math.sqrt(d2)) / (2 * a);
      var lr2 = (0 - b2 - Math.sqrt(d2)) / (2 * a);
      return {
        left: Math.min(lr1, lr2),
        top: Math.min(tb1, tb2),
        right: Math.max(lr1, lr2),
        bottom: Math.max(tb1, tb2),
        _wh: function () {
          delete this._wh;
          this.width = this.right - this.left;
          this.height = this.bottom - this.top;
          return this;
        }
      }._wh();
    } else return helper.boundingUnderTransform(matrix, t, r, b, l);
  }

  return {
    left: l,
    top: t,
    right: r,
    bottom: b,
    width: 2 * rx,
    height: 2 * ry
  };
}

function boundingRectOfPolygon(polygon) {
  polygon = element_object(polygon);

  var points = polygon.points.trim().replace(/\r\n|\n|\r/gm, ',').replace(/\s+/g, ',').split(',').map(parseFloat);

  var l = Number.POSITIVE_INFINITY;
  var r = Number.NEGATIVE_INFINITY;
  var t = Number.POSITIVE_INFINITY;
  var b = Number.NEGATIVE_INFINITY;

  for (var i = 0; i < points.length; i += 2) {
    if (l > points[i]) l = points[i];
    if (r < points[i]) r = points[i];
    if (t > points[i + 1]) t = points[i + 1];
    if (b < points[i + 1]) b = points[i + 1];
  }

  return {
    left: l,
    top: t,
    right: r,
    bottom: b,
    width: r - l,
    height: b - t
  };
}

function boundingRectOfPolyline(polyline) {
  polyline = element_object(polyline);
  return boundingRectOfPolygon(polyline);
}

// This method returns the bounding box of the path.
// Unless shouldReturnTrueBounding is set to a truthy value,
// it only checks each point, not the actual drawn path,
// meaning the bounding box may be larger than the actual
// bounding box. The reason is:
// 1. we don't need the exact bounding box;
// 2. all the browsers calculate this way;
// 3. it is easier to calculate.
// This method assumes the d property of the path is valid.
// Since SVG is exported from Illustrator, I assume this condition
// is always met.
// Things ignored:
// 1. the xAxisRotation property of A/a command;
// 2. M/m command checking.
// Because Illustrator doesn't export A/a command as well as useless
// M/m commands, we are good here.
function boundingRectOfPath(path, shouldReturnTrueBounding) {
  path = element_object(path);
  var d = path.d.replace(/\r\n|\n|\r/gm, '');
  var x = 0,
      y = 0;
  var commands = [];
  var params, potentialCp; // cp for control point

  var l = Number.POSITIVE_INFINITY;
  var r = Number.NEGATIVE_INFINITY;
  var t = Number.POSITIVE_INFINITY;
  var b = Number.NEGATIVE_INFINITY;

  // Helper - get arguments of a path drawing command
  var getArgs = function (str) {
    var output = [];
    var idx = 0;
    var c, num;

    var nextNumber = function () {
      var chars = [];

      while (/[^-\d\.]/.test(str.charAt(idx))) {
        // skip the non-digit characters
        idx++;
      }

      if ('-' === str.charAt(idx)) {
        chars.push('-');
        idx++;
      }

      while ((c = str.charAt(idx)) && /[\d\.Ee]/.test(c)) {
        chars.push(c);
        idx++;
      }

      return parseFloat(chars.join(''));
    };

    while (!isNaN(num = nextNumber())) output.push(num);

    return output;
  };

  var checkX = function (val) {
    if (val < l) l = val;
    if (val > r) r = val;
  };

  var checkY = function (val) {
    if (val < t) t = val;
    if (val > b) b = val;
  };

  // Get all commands first
  var i = 0,
      c = '';
  while (c = d.charAt(i++)) {
    if (/[mlhvaqtcsz]/i.test(c)) commands.push(c);
  }

  // The shift() is used to throw away strings come before the first command
  params = d.replace(/[mlhvaqtcsz]/ig, '#').split('#');
  params.shift();
  params.forEach(function (str, idx) {
    var command = commands[idx];
    if (/z/i.test(command)) return;

    // Get arguments of each command
    var args = getArgs(str);

    // Different commands have different arguments
    // Here's a quick review
    // M m - x y
    // L l - x y
    // H h - x
    // V v - y
    // A a - rx ry xAxisRotation largeArc sweep x y
    // Q q - x1 y1 x y
    // T t - x y
    // C c - x1 y1 x2 y2 x y
    // S s - x2 y2 x y
    // S/s needs access to the points of previous C/c command
    // T/t needs access to the points of previous Q/q command
    // Here "previous" means right before the target command
    var i, trueBounds, cpx1, cpy1, cpx2, cpy2;

    if (/[ML]/.test(command)) {
      for (i = 0; i < args.length; i += 2) {
        x = args[i];
        y = args[i + 1];
        checkX(x);
        checkY(y);
      }
    } else if (/[ml]/.test(command)) {
      for (i = 0; i < args.length; i += 2) {
        x += args[i];
        y += args[i + 1];
        checkX(x);
        checkY(y);
      }
    } else if (command === 'C') {
      for (i = 0; i < args.length; i += 6) {
        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, args[i], args[i + 1], args[i + 2], args[i + 3], args[i + 4], args[i + 5]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(args[i]);
          checkY(args[i + 1]);
          checkX(args[i + 2]);
          checkY(args[i + 3]);
          checkX(args[i + 4]);
          checkY(args[i + 5]);
        }

        potentialCp = [args[i + 4] * 2 - args[i + 2], args[i + 5] * 2 - args[i + 3]];
        x = args[i + 4];
        y = args[i + 5];
      }
    } else if (command === 'c') {
      for (i = 0; i < args.length; i += 6) {
        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, x + args[i], y + args[i + 1], x + args[i + 2], y + args[i + 3], x + args[i + 4], y + args[i + 5]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(x + args[i + 0]);
          checkY(y + args[i + 1]);
          checkX(x + args[i + 2]);
          checkY(y + args[i + 3]);
          checkX(x + args[i + 4]);
          checkY(y + args[i + 5]);
        }

        potentialCp = [2 * (x + args[i + 4]) - (x + args[i + 2]), 2 * (y + args[i + 5]) - (y + args[i + 3])];
        x += args[i + 4];
        y += args[i + 5];
      }
    } else if (command === 'S') {
      if (shouldReturnTrueBounding) {
        if (/[cs]/i.test(commands[idx - 1])) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, potentialCp[0], potentialCp[1], args[0], args[1], args[2], args[3]);
        } else {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, x, y, args[0], args[1], args[2], args[3]);
        }
        checkX(trueBounds[0]); // MIN_X
        checkX(trueBounds[4]); // MAX_X
        checkY(trueBounds[1]); // MIN_Y
        checkY(trueBounds[5]); // MAX_Y
      } else {
        if (/[cs]/i.test(commands[idx - 1])) {
          checkX(potentialCp[0]);
          checkY(potentialCp[1]);
        }
        checkX(args[0]);
        checkY(args[1]);
        checkX(args[2]);
        checkY(args[3]);
      }

      potentialCp = [2 * args[2] - args[0], 2 * args[3] - args[1]];

      x = args[2];
      y = args[3];

      for (i = 4; i < args.length; i += 4) {
        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, potentialCp[0], potentialCp[1], args[i], args[i + 1], args[i + 2], args[i + 3]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(potentialCp[0]);
          checkY(potentialCp[1]);
          checkX(args[i]);
          checkY(args[i + 1]);
          checkX(args[i + 2]);
          checkY(args[i + 3]);
        }

        potentialCp = [2 * args[i + 2] - args[i], 2 * args[i + 3] - args[i + 1]];
        x = args[i + 2];
        y = args[i + 3];
      }
    } else if (command === 's') {
      if (shouldReturnTrueBounding) {
        if (/[cs]/i.test(commands[idx - 1])) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, potentialCp[0], potentialCp[1], x + args[0], y + args[1], x + args[2], y + args[3]);
        } else {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, x, y, x + args[0], y + args[1], x + args[2], y + args[3]);
        }
        checkX(trueBounds[0]); // MIN_X
        checkX(trueBounds[4]); // MAX_X
        checkY(trueBounds[1]); // MIN_Y
        checkY(trueBounds[5]); // MAX_Y
      } else {
        if (/[cs]/i.test(commands[idx - 1])) {
          checkX(potentialCp[0]);
          checkY(potentialCp[1]);
        }
        checkX(x + args[0]);
        checkY(y + args[1]);
        checkX(x + args[2]);
        checkY(y + args[3]);
      }

      potentialCp = [2 * (x + args[2]) - (x + args[0]), 2 * (y + args[3]) - (y + args[1])];
      x += args[2];
      y += args[3];

      for (i = 4; i < args.length; i += 4) {
        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, potentialCp[0], potentialCp[1], x + args[i], y + args[i + 1], x + args[i + 2], y + args[i + 3]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(potentialCp[0]);
          checkY(potentialCp[1]);
          checkX(x + args[i]);
          checkY(y + args[i + 1]);
          checkX(x + args[i + 2]);
          checkY(y + args[i + 3]);
        }

        potentialCp = [2 * (x + args[i + 2]) - (x + args[i]), 2 * (y + args[i + 3]) - (y + args[i + 1])];
        x += args[i + 2];
        y += args[i + 3];
      }
    } else if (command === 'H') {
      for (i = 0; i < args.length; i++) {
        x = args[i];
        checkX(x);
      }
    } else if (command === 'h') {
      for (i = 0; i < args.length; i++) {
        x += args[i];
        checkX(x);
      }
    } else if (command === 'V') {
      for (i = 0; i < args.length; i++) {
        y = args[i];
        checkY(y);
      }
    } else if (command === 'v') {
      for (i = 0; i < args.length; i++) {
        y += args[i];
        checkY(y);
      }
    } else if (command === 'Q') {
      for (i = 0; i < args.length; i += 4) {
        // convert the one quadratic curve control point to
        // two bezier curve control points using the formula
        // cubicControlX1 = quadraticStartX + 2/3 * (quadraticControlX - quadraticStartX)
        // cubicControlY1 = quadraticStartY + 2/3 * (quadraticControlY - quadraticStartY)
        // cubicControlX2 = quadraticEndX + 2/3 * (quadraticControlX - quadraticEndX)
        // cubicControlY2 = quadraticEndY + 2/3 * (quadraticControlY - quadraticEndY)

        cpx1 = x + 2 / 3 * (args[i] - x);
        cpy1 = y + 2 / 3 * (args[i + 1] - y);
        cpx2 = args[i + 2] + 2 / 3 * (args[i] - args[i + 2]);
        cpy2 = args[i + 3] + 2 / 3 * (args[i + 1] - args[i + 3]);

        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, cpx1, cpy1, cpx2, cpy2, args[i + 2], args[i + 3]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(cpx1);
          checkY(cpy1);
          checkX(cpx2);
          checkY(cpy2);
          checkX(args[i + 2]);
          checkY(args[i + 3]);
        }

        potentialCp = [2 * args[i + 2] - args[i], 2 * args[i + 3] - args[i + 1]];
        x = args[i + 2];
        y = args[i + 3];
      }
    } else if (command === 'q') {
      for (i = 0; i < args.length; i += 4) {
        cpx1 = x + 2 / 3 * args[i];
        cpy1 = y + 2 / 3 * args[i + 1];
        cpx2 = x + args[i + 2] + 2 / 3 * (args[i] - args[i + 2]);
        cpy2 = y + args[i + 3] + 2 / 3 * (args[i + 1] - args[i + 3]);

        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, cpx1, cpy1, cpx2, cpy2, x + args[i + 2], y + args[i + 3]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(cpx1);
          checkY(cpy1);
          checkX(cpx2);
          checkY(cpy2);
          checkX(x + args[i + 2]);
          checkY(y + args[i + 3]);
        }

        potentialCp = [2 * (x + args[i + 2]) - (x + args[i]), 2 * (y + args[i + 3]) - (y + args[i + 1])];
        x += args[i + 2];
        y += args[i + 3];
      }
    } else if (command === 'T') {
      if (/[qt]/i.test(commands[idx - 1])) {
        cpx1 = x + 2 / 3 * (potentialCp[0] - x);
        cpy1 = y + 2 / 3 * (potentialCp[1] - y);
        cpx2 = args[0] + 2 / 3 * (potentialCp[0] - args[0]);
        cpy2 = args[1] + 2 / 3 * (potentialCp[1] - args[1]);

        potentialCp = [2 * args[0] - potentialCp[0], 2 * args[1] - potentialCp[1]];
      } else {
        cpx1 = x;
        cpy1 = y;
        cpx2 = args[0] + 2 / 3 * (x - args[0]);
        cpy2 = args[1] + 2 / 3 * (y - args[1]);

        potentialCp = [2 * args[0] - x, 2 * args[1] - y];
      }

      if (shouldReturnTrueBounding) {
        trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, cpx1, cpy1, cpx2, cpy2, args[0], args[1]);
        checkX(trueBounds[0]); // MIN_X
        checkX(trueBounds[4]); // MAX_X
        checkY(trueBounds[1]); // MIN_Y
        checkY(trueBounds[5]); // MAX_Y
      } else {
        checkX(cpx1);
        checkY(cpy1);
        checkX(cpx2);
        checkY(cpy2);
        checkX(args[0]);
        checkY(args[1]);
      }

      x = args[0];
      y = args[1];

      for (i = 2; i < args.length; i += 2) {
        cpx1 = x + 2 / 3 * (potentialCp[0] - x);
        cpy1 = y + 2 / 3 * (potentialCp[1] - y);
        cpx2 = args[i] + 2 / 3 * (potentialCp[0] - args[i]);
        cpy2 = args[i + 1] + 2 / 3 * (potentialCp[1] - args[i + 1]);

        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, cpx1, cpy1, cpx2, cpy2, args[i], args[i + 1]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(cpx1);
          checkY(cpy1);
          checkX(cpx2);
          checkY(cpy2);
          checkX(args[i]);
          checkY(args[i + 1]);
        }

        potentialCp = [2 * args[i] - potentialCp[0], 2 * args[i + 1] - potentialCp[1]];
        x = args[i];
        y = args[i + 1];
      }
    } else if (command === 't') {
      if (/[qt]/i.test(commands[idx - 1])) {
        cpx1 = x + 2 / 3 * (potentialCp[0] - x);
        cpy1 = y + 2 / 3 * (potentialCp[1] - y);
        cpx2 = x + args[0] + 2 / 3 * (potentialCp[0] - x - args[0]);
        cpy2 = y + args[1] + 2 / 3 * (potentialCp[1] - y - args[1]);

        potentialCp = [2 * (x + args[0]) - potentialCp[0], 2 * (y + args[1]) - potentialCp[1]];
      } else {
        cpx1 = x;
        cpy1 = y;
        cpx2 = x + args[0] - 2 / 3 * args[0];
        cpy2 = y + args[1] - 2 / 3 * args[1];

        potentialCp = [2 * (x + args[0]) - x, 2 * (y + args[1]) - y];
      }

      if (shouldReturnTrueBounding) {
        trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, cpx1, cpy1, cpx2, cpy2, x + args[0], y + args[1]);
        checkX(trueBounds[0]); // MIN_X
        checkX(trueBounds[4]); // MAX_X
        checkY(trueBounds[1]); // MIN_Y
        checkY(trueBounds[5]); // MAX_Y
      } else {
        checkX(cpx1);
        checkY(cpy1);
        checkX(cpx2);
        checkY(cpy2);
        checkX(x + args[0]);
        checkY(y + args[1]);
      }

      x += args[0];
      y += args[1];

      for (i = 2; i < args.length; i += 2) {
        cpx1 = x + 2 / 3 * (potentialCp[0] - x);
        cpy1 = y + 2 / 3 * (potentialCp[1] - y);
        cpx2 = x + args[i] + 2 / 3 * (potentialCp[0] - x - args[i]);
        cpy2 = y + args[i + 1] + 2 / 3 * (potentialCp[1] - y - args[i + 1]);

        if (shouldReturnTrueBounding) {
          trueBounds = curve_bounding.calculate(curve_bounding.Mode.STANDARD, x, y, cpx1, cpy1, cpx2, cpy2, x + args[i], y + args[i + 1]);
          checkX(trueBounds[0]); // MIN_X
          checkX(trueBounds[4]); // MAX_X
          checkY(trueBounds[1]); // MIN_Y
          checkY(trueBounds[5]); // MAX_Y
        } else {
          checkX(cpx1);
          checkY(cpy1);
          checkX(cpx2);
          checkY(cpy2);
          checkX(x + args[i]);
          checkY(y + args[i + 1]);
        }

        potentialCp = [2 * (x + args[i]) - potentialCp[0], 2 * (y + args[i + 1]) - potentialCp[1]];
        x += args[i];
        y += args[i + 1];
      }
    } else if (command === 'A') {
      for (var i = 0; i < args.length; i += 7) {
        x = args[i + 5];
        y = args[i + 6];
        checkX(x);
        checkY(y);
      }
    } else if (command === 'a') {
      for (var i = 0; i < args.length; i += 7) {
        x += args[i + 5];
        y += args[i + 6];
        checkX(x);
        checkY(y);
      }
    }
  });

  return {
    left: l,
    top: t,
    right: r,
    bottom: b,
    width: r - l,
    height: b - t
  };
}

function boundingRectOfShape(shape, needTrueBounding) {
  var elementObj = element_object(shape);
  if (!elementObj) return null;

  var bounding;
  switch (elementObj.type) {
    case 'path':
      bounding = boundingRectOfPath(elementObj, needTrueBounding);
      break;
    case 'polygon':
      bounding = boundingRectOfPolygon(elementObj);
      break;
    case 'rect':
      bounding = boundingRectOfRect(elementObj);
      break;
    case 'ellipse':
      bounding = boundingRectOfEllipse(elementObj, needTrueBounding);
      break;
    case 'circle':
      bounding = boundingRectOfCircle(elementObj);
      break;
    case 'polyline':
      bounding = boundingRectOfPolyline(elementObj);
      break;
    case 'line':
      bounding = boundingRectOfLine(elementObj);
      break;
  }

  return bounding;
}

var shape_bounding = {
  line: boundingRectOfLine,
  rect: boundingRectOfRect,
  circle: boundingRectOfCircle,
  ellipse: boundingRectOfEllipse,
  polygon: boundingRectOfPolygon,
  polyline: boundingRectOfPolyline,
  path: boundingRectOfPath,
  shape: boundingRectOfShape
};

function getUnicodeLength(str) {
  var length = 0;
  if (!str) return 0;
  for (var i = 0; i < str.length; i++) {
    if (str[i] === '%') length += 2;else length += str.charCodeAt(i) > 255 ? 2 : 1;
  }
  return length;
}

function textBoundingBox(text) {
  var textObj = element_object(text);
  if (!textObj) return null;

  var bounding;
  if (textObj.children.length > 0) bounding = multiLineTextBoundingBox(textObj);else bounding = singleLineTextBoundingBox(textObj);
  return bounding;
}

function singleLineTextBoundingBox(textObj) {
  var x = 0;
  var y = 0;
  var fontSize = parseFloat(textObj['font-size']);
  var letterSpacing = textObj['letter-spacing'];
  var unicodeLength = getUnicodeLength(textObj.text);
  var matrix;
  if (/matrix/i.test(textObj.transform)) {
    matrix = helper.matrixStrToArr(textObj.transform.trim());
    x = matrix[4];
    y = matrix[5];
  }
  if (typeof textObj.x === 'number') x = textObj.x;
  if (typeof textObj.y === 'number') y = textObj.y;

  // By setting y value of an text object to Math.round(0.8808*fontSize - 0.3333)
  // it just snaps to the top of the SVG wrapper
  // The formula comes from curve fitting tool in Matlab
  // https://img.alicdn.com/tps/TB1CJu.PpXXXXXcaXXXXXXXXXXX-2053-1236.jpg
  return {
    top: y - Math.round(0.8808 * fontSize - 0.3333),
    left: x,
    width: unicodeLength / 2 * fontSize + (textObj.text.length - 1) * letterSpacing,
    height: fontSize,
    _init: function () {
      delete this._init;
      this.right = this.left + this.width;
      this.bottom = this.top + this.height;
      return this;
    }
  }._init();
}

function multiLineTextBoundingBox(textObj) {
  var top = Number.POSITIVE_INFINITY;
  var left = Number.POSITIVE_INFINITY;
  var bottom = Number.NEGATIVE_INFINITY;
  var right = Number.NEGATIVE_INFINITY;
  var matrix = [1, 0, 0, 1, 0, 0];
  var firstLineFontSize;
  var lastY = Number.POSITIVE_INFINITY;
  if (/matrix/i.test(textObj.transform)) {
    matrix = helper.matrixStrToArr(textObj.transform.trim());
  }

  textObj.children.forEach(function (tspanObj) {
    var fontSize = parseFloat(tspanObj['font-size'].trim());
    var letterSpacing = tspanObj['letter-spacing'];
    var unicodeLength = getUnicodeLength(tspanObj.text);
    var w = unicodeLength / 2 * fontSize + (tspanObj.text.length - 1) * letterSpacing;
    var h = fontSize;
    var t = tspanObj.y;
    var l = tspanObj.x;
    var b = t + h;
    var r = l + w;
    if (t < top) top = t;
    if (l < left) left = l;
    if (b > bottom) bottom = b;
    if (r > right) right = r;
    if (lastY > t) {
      lastY = t;
      firstLineFontSize = fontSize;
    }
  });

  return {
    left: matrix[0] * left + matrix[2] * top + matrix[4],
    top: matrix[1] * left + matrix[3] * top + matrix[5] - Math.round(0.8808 * firstLineFontSize - 0.3333),
    right: matrix[0] * right + matrix[2] * bottom + matrix[4],
    bottom: matrix[1] * right + matrix[3] * bottom + matrix[5] - Math.round(0.8808 * firstLineFontSize - 0.3333),
    _wh: function () {
      delete this._wh;
      this.width = this.right - this.left;
      this.height = this.bottom - this.top;
      return this;
    }
  }._wh();
}

var text_bounding = textBoundingBox;

function radialGradientBoundingBox(gradientObj) {
  return {
    left: gradientObj.cx - gradientObj.r,
    top: gradientObj.cy - gradientObj.r,
    right: gradientObj.cx + gradientObj.r,
    bottom: gradientObj.cy + gradientObj.r,
    width: gradientObj.r * 2,
    height: gradientObj.r * 2
  };
}

function linearGradientBoundingBox(gradientObj) {
  return {
    left: Math.min(gradientObj.x1, gradientObj.x2),
    top: Math.min(gradientObj.y1, gradientObj.y2),
    right: Math.max(gradientObj.x1, gradientObj.x2),
    bottom: Math.max(gradientObj.y1, gradientObj.y2),
    width: Math.abs(gradientObj.x1 - gradientObj.x2),
    height: Math.abs(gradientObj.y1 - gradientObj.y2)
  };
}

function gradientBoundingBox(gradient) {
  var gradientObj = element_object(gradient);
  if (!gradientObj) return null;

  if (/^linearGradient$/i.test(gradientObj.type)) return linearGradientBoundingBox(gradientObj);else if (/^radialGradient$/i.test(gradientObj.type)) return radialGradientBoundingBox(gradientObj);else return null;
}

var gradient_bounding = gradientBoundingBox;

var svgBoundings = {
  line: shape_bounding.line,
  rect: shape_bounding.rect,
  circle: shape_bounding.circle,
  ellipse: shape_bounding.ellipse,
  polygon: shape_bounding.polygon,
  polyline: shape_bounding.polyline,
  path: shape_bounding.path,
  shape: shape_bounding.shape,
  image: image_bounding,
  text: text_bounding,
  gradient: gradient_bounding
};
var svgBoundings_1 = svgBoundings.line;
var svgBoundings_2 = svgBoundings.rect;
var svgBoundings_3 = svgBoundings.circle;
var svgBoundings_4 = svgBoundings.ellipse;
var svgBoundings_5 = svgBoundings.polygon;
var svgBoundings_6 = svgBoundings.polyline;
var svgBoundings_7 = svgBoundings.path;
var svgBoundings_8 = svgBoundings.shape;
var svgBoundings_9 = svgBoundings.image;
var svgBoundings_10 = svgBoundings.text;
var svgBoundings_11 = svgBoundings.gradient;

return svgBoundings;

})();

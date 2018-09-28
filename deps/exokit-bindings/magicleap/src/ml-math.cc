#ifdef LUMIN

#include <ml-math.h>

namespace ml {

// util

MLVec3f addVectors(const MLVec3f &a, const MLVec3f &b) {
  return MLVec3f{
    a.x + b.x,
    a.y + b.y,
    a.z + b.z
  };
}

MLVec3f subVectors(const MLVec3f &a, const MLVec3f &b) {
  return MLVec3f{
    a.x - b.x,
    a.y - b.y,
    a.z - b.z
  };
}

MLVec3f multiplyVector(const MLVec3f &v, float l) {
  return MLVec3f{
    v.x * l,
    v.y * l,
    v.z * l
  };
}

MLVec3f divideVector(const MLVec3f &v, float l) {
  return MLVec3f{
    v.x / l,
    v.y / l,
    v.z / l
  };
}

float dotVectors(const MLVec3f &a, const MLVec3f &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

MLVec3f crossVectors(const MLVec3f &a, const MLVec3f &b) {
	return MLVec3f{
    a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
  };
}

float vectorLengthSq(const MLVec3f &v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

float vectorLength(const MLVec3f &v) {
  return sqrt(vectorLengthSq(v));
}

MLVec3f normalizeVector(const MLVec3f &v) {
  return divideVector(v, vectorLength(v));
}

MLVec3f applyVectorQuaternion(const MLVec3f &v, const MLQuaternionf &q) {
  // calculate quat * vector

  const float ix = q.w * v.x + q.y * v.z - q.z * v.y;
  const float iy = q.w * v.y + q.z * v.x - q.x * v.z;
  const float iz = q.w * v.z + q.x * v.y - q.y * v.x;
  const float iw = -q.x * v.x - q.y * v.y - q.z * v.z;

  // calculate result * inverse quat

  return MLVec3f{
    ix * q.w + iw * - q.x + iy * - q.z - iz * - q.y,
    iy * q.w + iw * - q.y + iz * - q.x - ix * - q.z,
    iz * q.w + iw * - q.z + ix * - q.y - iy * - q.x
  };
}

float quaternionLength(const MLQuaternionf &q) {
  return sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

MLQuaternionf normalizeQuaternion(const MLQuaternionf &q) {
  float l = quaternionLength(q);

  if (l == 0) {
    return MLQuaternionf{
      0,
      0,
      0,
      1
    };
  } else {
    l = 1 / l;
    return MLQuaternionf{
      q.x * l,
      q.y * l,
      q.z * l,
      q.w * l
    };
  }
}

MLQuaternionf multiplyQuaternions(const MLQuaternionf &qa, const MLQuaternionf &qb) {
  return MLQuaternionf{
    qa.x * qb.w + qa.w * qb.x + qa.y * qb.z - qa.z * qb.y,
    qa.y * qb.w + qa.w * qb.y + qa.z * qb.x - qa.x * qb.z,
    qa.z * qb.w + qa.w * qb.z + qa.x * qb.y - qa.y * qb.x,
    qa.w * qb.w - qa.x * qb.x - qa.y * qb.y - qa.z * qb.z
  };
}

MLQuaternionf getQuaternionFromUnitVectors(const MLVec3f &vFrom, const MLVec3f &vTo) {
  constexpr float EPS = 0.000001;

  MLVec3f v1;
  float r = dotVectors(vFrom, vTo) + 1;
  if (r < EPS) {
    r = 0;

    if (std::abs(vFrom.x) > std::abs(vFrom.z)) {
      v1 = MLVec3f{-vFrom.y, vFrom.x, 0};
    } else {
      v1 = MLVec3f{0, -vFrom.z, vFrom.y};
    }
  } else {
    v1 = crossVectors(vFrom, vTo);
  }

  MLQuaternionf result{
    v1.x,
    v1.y,
    v1.z,
    r
  };
  return normalizeQuaternion(result);
}

MLVec3f getTriangleNormal(const MLVec3f &a, const MLVec3f &b, const MLVec3f &c) {
  MLVec3f target = subVectors(c, b);
  MLVec3f v0 = subVectors(a, b);
  target = crossVectors(target, v0);

  float targetLengthSq = vectorLengthSq(target);
  if (targetLengthSq > 0) {
    return multiplyVector(target, 1 / sqrt(targetLengthSq));
  } else {
    return MLVec3f{0, 0, 0};
  }
}

void orthonormalizeVectors(MLVec3f &normal, MLVec3f &tangent) {
	normal = normalizeVector(normal);
	tangent = normalizeVector(tangent);
	
  const MLVec3f &projection = multiplyVector(normal, dotVectors(normal, tangent));
	tangent = subVectors(tangent, projection);
  tangent = normalizeVector(tangent);
}

MLQuaternionf getLookAtQuaternion(const MLVec3f &lookAt, const MLVec3f &upDirection) {
	MLVec3f forward = multiplyVector(lookAt, -1);
  MLVec3f up = upDirection;
  orthonormalizeVectors(forward, up);
  
	const MLVec3f &right = crossVectors(up, forward);

	MLQuaternionf ret;
	ret.w = sqrt(1.0f + right.x + up.y + forward.z) * 0.5f;
	const float w4_recip = 1.0f / (4.0f * ret.w);
	ret.x = (up.z - forward.y) * w4_recip;
	ret.y = (forward.x - right.z) * w4_recip;
	ret.z = (right.y - up.x) * w4_recip;
	return ret;
}

MLQuaternionf getQuaternionFromRotationMatrix(const MLMat4f &m) {
  // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
  // assumes the upper 3x3 of m is a pure rotation matrix (i.e, unscaled)

  const float *te = m.matrix_colmajor;

  const float m11 = te[ 0 ], m12 = te[ 4 ], m13 = te[ 8 ],
    m21 = te[ 1 ], m22 = te[ 5 ], m23 = te[ 9 ],
    m31 = te[ 2 ], m32 = te[ 6 ], m33 = te[ 10 ];

  float trace = m11 + m22 + m33,
    s;

  if (trace > 0.0f) {
    s = 0.5f / sqrt(trace + 1.0f);

    return MLQuaternionf{
      (m32 - m23) * s,
      (m13 - m31) * s,
      (m21 - m12) * s,
      0.25f / s
    };
  } else if (m11 > m22 && m11 > m33) {
    s = 2.0f * sqrt(1.0f + m11 - m22 - m33 );

    return MLQuaternionf{
      0.25f * s,
      (m12 + m21) / s,
      (m13 + m31) / s,
      (m32 - m23) / s
    };
  } else if (m22 > m33) {
    s = 2.0f * sqrt(1.0f + m22 - m11 - m33);

    return MLQuaternionf{
      (m12 + m21) / s,
      0.25f * s,
      (m23 + m32) / s,
      (m13 - m31) / s
    };
  } else {
    s = 2.0f * sqrt(1.0f + m33 - m11 - m22);

    return MLQuaternionf{
      (m13 + m31) / s,
      (m23 + m32) / s,
      0.25f * s,
      (m21 - m12) / s
    };
  }
}

MLMat4f multiplyMatrices(const MLMat4f &a, const MLMat4f &b) {
  const float *ae = a.matrix_colmajor;
  const float *be = b.matrix_colmajor;
  
  MLMat4f result;
  float *te = result.matrix_colmajor;

  const float a11 = ae[ 0 ], a12 = ae[ 4 ], a13 = ae[ 8 ], a14 = ae[ 12 ];
  const float a21 = ae[ 1 ], a22 = ae[ 5 ], a23 = ae[ 9 ], a24 = ae[ 13 ];
  const float a31 = ae[ 2 ], a32 = ae[ 6 ], a33 = ae[ 10 ], a34 = ae[ 14 ];
  const float a41 = ae[ 3 ], a42 = ae[ 7 ], a43 = ae[ 11 ], a44 = ae[ 15 ];

  const float b11 = be[ 0 ], b12 = be[ 4 ], b13 = be[ 8 ], b14 = be[ 12 ];
  const float b21 = be[ 1 ], b22 = be[ 5 ], b23 = be[ 9 ], b24 = be[ 13 ];
  const float b31 = be[ 2 ], b32 = be[ 6 ], b33 = be[ 10 ], b34 = be[ 14 ];
  const float b41 = be[ 3 ], b42 = be[ 7 ], b43 = be[ 11 ], b44 = be[ 15 ];

  te[ 0 ] = a11 * b11 + a12 * b21 + a13 * b31 + a14 * b41;
  te[ 4 ] = a11 * b12 + a12 * b22 + a13 * b32 + a14 * b42;
  te[ 8 ] = a11 * b13 + a12 * b23 + a13 * b33 + a14 * b43;
  te[ 12 ] = a11 * b14 + a12 * b24 + a13 * b34 + a14 * b44;

  te[ 1 ] = a21 * b11 + a22 * b21 + a23 * b31 + a24 * b41;
  te[ 5 ] = a21 * b12 + a22 * b22 + a23 * b32 + a24 * b42;
  te[ 9 ] = a21 * b13 + a22 * b23 + a23 * b33 + a24 * b43;
  te[ 13 ] = a21 * b14 + a22 * b24 + a23 * b34 + a24 * b44;

  te[ 2 ] = a31 * b11 + a32 * b21 + a33 * b31 + a34 * b41;
  te[ 6 ] = a31 * b12 + a32 * b22 + a33 * b32 + a34 * b42;
  te[ 10 ] = a31 * b13 + a32 * b23 + a33 * b33 + a34 * b43;
  te[ 14 ] = a31 * b14 + a32 * b24 + a33 * b34 + a34 * b44;

  te[ 3 ] = a41 * b11 + a42 * b21 + a43 * b31 + a44 * b41;
  te[ 7 ] = a41 * b12 + a42 * b22 + a43 * b32 + a44 * b42;
  te[ 11 ] = a41 * b13 + a42 * b23 + a43 * b33 + a44 * b43;
  te[ 15 ] = a41 * b14 + a42 * b24 + a43 * b34 + a44 * b44;
  
  return result;
}

MLMat4f getLookAtMatrix(const MLVec3f &eye, const MLVec3f &target, const MLVec3f &up) {
  MLMat4f result;
  float *te = result.matrix_colmajor;

  MLVec3f x;
  MLVec3f y;
  MLVec3f z;

  z = subVectors(eye, target);

  if (vectorLengthSq(z) == 0) {
    // eye and target are in the same position
    z.z = 1.0;
  }
  
  z = normalizeVector(z);
  x = crossVectors(up, z);

  if (vectorLengthSq(x) == 0.0) {
    // up and z are parallel

    if (std::abs(up.z) == 1.0f) {
      z.x += 0.0001f;
    } else {
      z.z += 0.0001f;
    }

    z = normalizeVector(z);
    x = crossVectors(up, z);
  }

  x = normalizeVector(x);
  y = crossVectors(z, x);

  te[ 0 ] = x.x; te[ 4 ] = y.x; te[ 8 ] = z.x;  te[ 12 ] = 0;
  te[ 1 ] = x.y; te[ 5 ] = y.y; te[ 9 ] = z.y;  te[ 13 ] = 0;
  te[ 2 ] = x.z; te[ 6 ] = y.z; te[ 10 ] = z.z; te[ 14 ] = 0;
  te[ 3 ] = 0;   te[ 7 ] = 0;   te[ 11 ] = 0;   te[ 15 ] = 1;

  return result;
}

MLMat4f composeMatrix(
  const MLVec3f &position,
  const MLQuaternionf &quaternion,
  const MLVec3f &scale
) {
  MLMat4f result;

  float	*te = result.matrix_colmajor;

  float x = quaternion.x, y = quaternion.y, z = quaternion.z, w = quaternion.w;
  float x2 = x + x,	y2 = y + y, z2 = z + z;
  float xx = x * x2, xy = x * y2, xz = x * z2;
  float yy = y * y2, yz = y * z2, zz = z * z2;
  float wx = w * x2, wy = w * y2, wz = w * z2;

  float sx = scale.x, sy = scale.y, sz = scale.z;

  te[ 0 ] = ( 1 - ( yy + zz ) ) * sx;
  te[ 1 ] = ( xy + wz ) * sx;
  te[ 2 ] = ( xz - wy ) * sx;
  te[ 3 ] = 0;

  te[ 4 ] = ( xy - wz ) * sy;
  te[ 5 ] = ( 1 - ( xx + zz ) ) * sy;
  te[ 6 ] = ( yz + wx ) * sy;
  te[ 7 ] = 0;

  te[ 8 ] = ( xz + wy ) * sz;
  te[ 9 ] = ( yz - wx ) * sz;
  te[ 10 ] = ( 1 - ( xx + yy ) ) * sz;
  te[ 11 ] = 0;

  te[ 12 ] = position.x;
  te[ 13 ] = position.y;
  te[ 14 ] = position.z;
  te[ 15 ] = 1;

  return result;
}

MLMat4f invertMatrix(const MLMat4f &matrix) {
  MLMat4f result;

  float	*te = result.matrix_colmajor;
  const float *me = matrix.matrix_colmajor;
  float n11 = me[ 0 ], n21 = me[ 1 ], n31 = me[ 2 ], n41 = me[ 3 ],
    n12 = me[ 4 ], n22 = me[ 5 ], n32 = me[ 6 ], n42 = me[ 7 ],
    n13 = me[ 8 ], n23 = me[ 9 ], n33 = me[ 10 ], n43 = me[ 11 ],
    n14 = me[ 12 ], n24 = me[ 13 ], n34 = me[ 14 ], n44 = me[ 15 ],

    t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44,
    t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44,
    t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44,
    t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

  float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;

  if ( det == 0 ) {

    // std::cout << "ML can't invert matrix, determinant is 0" << std::endl;

    return MLMat4f{
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1,
    };

  }

  float detInv = 1 / det;

  te[ 0 ] = t11 * detInv;
  te[ 1 ] = ( n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44 ) * detInv;
  te[ 2 ] = ( n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44 ) * detInv;
  te[ 3 ] = ( n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43 ) * detInv;

  te[ 4 ] = t12 * detInv;
  te[ 5 ] = ( n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44 ) * detInv;
  te[ 6 ] = ( n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44 ) * detInv;
  te[ 7 ] = ( n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43 ) * detInv;

  te[ 8 ] = t13 * detInv;
  te[ 9 ] = ( n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44 ) * detInv;
  te[ 10 ] = ( n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44 ) * detInv;
  te[ 11 ] = ( n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43 ) * detInv;

  te[ 12 ] = t14 * detInv;
  te[ 13 ] = ( n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34 ) * detInv;
  te[ 14 ] = ( n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34 ) * detInv;
  te[ 15 ] = ( n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33 ) * detInv;

  return result;
}

MLPlanef makePlaneFromComponents(float x, float y, float z, float w) {
  return MLPlanef{
    MLVec3f{x, y, z},
    w
  };
}

MLPlanef normalizePlane(const MLPlanef &p) {
  float inverseNormalLength = 1.0f / vectorLength(p.normal);
  return MLPlanef{
    multiplyVector(p.normal, inverseNormalLength),
    p.constant * inverseNormalLength
  };
}

float distanceFromPlaneToPoint(const MLPlanef &plane, const MLVec3f &point) {
  return dotVectors(plane.normal, point) + plane.constant;
}

MLFrustumf makeFrustumFromMatrix(const MLMat4f &m) {
  const float *me = m.matrix_colmajor;
  const float me0 = me[ 0 ], me1 = me[ 1 ], me2 = me[ 2 ], me3 = me[ 3 ];
  const float me4 = me[ 4 ], me5 = me[ 5 ], me6 = me[ 6 ], me7 = me[ 7 ];
  const float me8 = me[ 8 ], me9 = me[ 9 ], me10 = me[ 10 ], me11 = me[ 11 ];
  const float me12 = me[ 12 ], me13 = me[ 13 ], me14 = me[ 14 ], me15 = me[ 15 ];

  MLFrustumf result;
  result.planes[ 0 ] = normalizePlane(makePlaneFromComponents( me3 - me0, me7 - me4, me11 - me8, me15 - me12 ));
  result.planes[ 1 ] = normalizePlane(makePlaneFromComponents( me3 + me0, me7 + me4, me11 + me8, me15 + me12 ));
  result.planes[ 2 ] = normalizePlane(makePlaneFromComponents( me3 + me1, me7 + me5, me11 + me9, me15 + me13 ));
  result.planes[ 3 ] = normalizePlane(makePlaneFromComponents( me3 - me1, me7 - me5, me11 - me9, me15 - me13 ));
  result.planes[ 4 ] = normalizePlane(makePlaneFromComponents( me3 - me2, me7 - me6, me11 - me10, me15 - me14 ));
  result.planes[ 5 ] = normalizePlane(makePlaneFromComponents( me3 + me2, me7 + me6, me11 + me10, me15 + me14 ));
  return result;
}

bool frustumIntersectsSphere(const MLFrustumf &frustum, const MLSphere &sphere) {
  const MLPlanef *planes = frustum.planes;
  const MLVec3f &center = sphere.center;
  const float negRadius = -sphere.radius;

  for (int i = 0; i < 6; i++) {
    float distance = distanceFromPlaneToPoint(planes[i], center);
    if (distance < negRadius) {
      return false;
    }
  }
  return true;
}

// hands

bool getHandBone(MLVec3f &position, int handIndex, float wristBones[2][4][1 + 3], float fingerBones[2][5][4][1 + 3]) {
  for (size_t i = 0; i < 4; i++) {
    if (*(uint32_t *)&wristBones[handIndex][i][0]) {
      return true;
    }
  }
  for (size_t i = 0; i < 5; i++) {
    for (size_t j = 0; j < 4; j++) {
      if (*(uint32_t *)&fingerBones[handIndex][i][j][0]) {
        position = {
          fingerBones[handIndex][i][j][1],
          fingerBones[handIndex][i][j][2],
          fingerBones[handIndex][i][j][3]
        };
        return true;
      }
    }
  }
  return false;
}

bool getFingerRayTransform(MLTransform &transform, std::vector<std::vector<float *>> &fingers, const MLVec3f &normal) {
  return std::any_of(fingers.begin(), fingers.end(), [&](std::vector<float *> &bones) -> bool {
    std::vector<float *> validBones(bones);
    validBones.erase(std::remove_if(validBones.begin(), validBones.end(), [&](float *bone) -> bool {
      if (*(uint32_t *)&bone[0]) {
        return false; // keep
      } else {
        return true; // remove
      }
    }), validBones.end());
    if (validBones.size() >= 2) {
      float *startBoneArray = validBones[0];
      const MLVec3f startBone = {
        startBoneArray[1],
        startBoneArray[2],
        startBoneArray[3]
      };
      float *endBoneArray = validBones[validBones.size() - 1];
      const MLVec3f endBone = {
        endBoneArray[1],
        endBoneArray[2],
        endBoneArray[3]
      };
      // const MLVec3f &direction = normalizeVector(subVectors(endBone, startBone));
      // const MLMat4f &lookMatrix = getLookAtMatrix(startBone, endBone, normal);
      // const MLQuaternionf &rotation = getQuaternionFromRotationMatrix(lookMatrix);
      const MLQuaternionf &rotation = getLookAtQuaternion(subVectors(endBone, startBone), normal);

      transform.position = endBone;
      transform.rotation = rotation;

      return true;
    } else {
      return false;
    }
  });
}

bool getHandTransform(MLVec3f &center, MLVec3f &normal, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3], bool left) {
  float (&thumb)[4][1 + 3] = fingerBones[0];
  float *thumbTip = thumb[3];

  if (*(uint32_t *)&thumbTip[0]) {
    float *thumbBase = nullptr;

    for (size_t i = 0; i < 3; i++) {
      if (*(uint32_t *)&thumb[i][0]) {
        thumbBase = thumb[i];
      }
    }
    if (thumbBase) {
      float *pointerSemiTip = nullptr;

      for (size_t i = 1; i < 5; i++) {
        for (size_t j = 0; j < 3; j++) {
          if (*(uint32_t *)&fingerBones[i][j][0]) {
            pointerSemiTip = fingerBones[i][j];
          }
        }
      }

      if (pointerSemiTip) {
        MLVec3f a = {
          thumbTip[1],
          thumbTip[2],
          thumbTip[3]
        };
        MLVec3f b = {
          pointerSemiTip[1],
          pointerSemiTip[2],
          pointerSemiTip[3]
        };
        MLVec3f c = {
          thumbBase[1],
          thumbBase[2],
          thumbBase[3]
        };

        center = {
          (a.x + b.x + c.x) / 3,
          (a.y + b.y + c.y) / 3,
          (a.z + b.z + c.z) / 3
        };
        normal = getTriangleNormal(a, b, c);
        if (!left) {
          normal = multiplyVector(normal, -1);
        }
        
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool getHandPointerTransform(MLTransform &transform, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3], const MLVec3f &normal) {
  std::vector<std::vector<float *>> fingers = {
    { // index
      fingerBones[1][0],
      fingerBones[1][1],
      fingerBones[1][2],
      fingerBones[1][3],
    },
    { // middle
      fingerBones[2][0],
      fingerBones[2][1],
      fingerBones[2][2],
      fingerBones[2][3],
    },
    { // thumb
      fingerBones[0][0],
      fingerBones[0][1],
      fingerBones[0][2],
      fingerBones[0][3],
    },
  };
  return getFingerRayTransform(transform, fingers, normal);
}

bool getHandGripTransform(MLTransform &transform, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3], const MLVec3f &normal) {
  std::vector<std::vector<float *>> fingers = {
    { // wrist center, middleFinger
      wristBones[0],
      fingerBones[2][0],
      fingerBones[2][1],
      fingerBones[2][2],
      fingerBones[2][3],
    },
    { // thumb
      fingerBones[0][0],
      fingerBones[0][1],
      fingerBones[0][2],
      fingerBones[0][3],
    },
  };
  return getFingerRayTransform(transform, fingers, normal);
}

}

#endif
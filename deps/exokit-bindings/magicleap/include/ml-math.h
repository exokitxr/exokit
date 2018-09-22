#ifndef _ML_MATH_H_
#define _ML_MATH_H_

#if defined(LUMIN)

#include <magicleap.h>

namespace ml {

// util

MLVec3f addVectors(const MLVec3f &a, const MLVec3f &b);
MLVec3f subVectors(const MLVec3f &a, const MLVec3f &b);
MLVec3f multiplyVector(const MLVec3f &v, float l);
MLVec3f divideVector(const MLVec3f &v, float l);
float dotVectors(const MLVec3f &a, const MLVec3f &b);
MLVec3f crossVectors(const MLVec3f &a, const MLVec3f &b);
float vectorLengthSq(const MLVec3f &v);
float vectorLength(const MLVec3f &v);
MLVec3f normalizeVector(const MLVec3f &v);
float quaternionLength(const MLQuaternionf &q);
MLQuaternionf normalizeQuaternion(const MLQuaternionf &q);
MLQuaternionf getQuaternionFromUnitVectors(const MLVec3f &vFrom, const MLVec3f &vTo);
MLVec3f getTriangleNormal(const MLVec3f &a, const MLVec3f &b, const MLVec3f &c);
void orthonormalizeVectors(MLVec3f &normal, MLVec3f &tangent);
MLQuaternionf getLookAtQuaternion(const MLVec3f &lookAt, const MLVec3f &upDirection);
MLQuaternionf getQuaternionFromRotationMatrix(const MLMat4f &m);
MLMat4f getLookAtMatrix(const MLVec3f &eye, const MLVec3f &target, const MLVec3f &up);
MLMat4f composeMatrix(
  const MLVec3f &position = MLVec3f{0,0,0},
  const MLQuaternionf &quaternion = MLQuaternionf{0,0,0,1},
  const MLVec3f &scale = MLVec3f{1,1,1}
);
MLMat4f invertMatrix(const MLMat4f &matrix);

// hands

bool getHandBone(MLVec3f &position, int handIndex, float wristBones[2][4][1 + 3], float fingerBones[2][5][4][1 + 3]);
bool getFingerRayTransform(MLTransform &transform, std::vector<std::vector<float *>> &fingers, const MLVec3f &normal);
bool getHandTransform(MLVec3f &center, MLVec3f &normal, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3], bool left);
bool getHandPointerTransform(MLTransform &transform, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3], const MLVec3f &normal);
bool getHandGripTransform(MLTransform &transform, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3], const MLVec3f &normal);

}

#endif
#endif

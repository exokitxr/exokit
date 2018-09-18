// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//  Enhanced RGB->YUV conversion functions
//
// Author: Skal (pascal.massimino@gmail.com)

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <vector>
using std::vector;

#define SJPEG_NEED_ASM_HEADERS
#include "sjpegi.h"

namespace sjpeg {

// We could use SFIX=0 and only uint8_t for fixed_y_t, but it produces some
// banding sometimes. Better use extra precision.
#define SFIX 2                // fixed-point precision of RGB and Y/W
#define SHALF (1 << SFIX >> 1)
#define MAX_Y_T ((256 << SFIX) - 1)
typedef int16_t fixed_t;      // signed type with extra SFIX precision for UV
typedef uint16_t fixed_y_t;   // unsigned type with extra SFIX precision for W

static fixed_y_t clip_y(int y) {
  return (!(y & ~MAX_Y_T)) ? (fixed_y_t)y : (y < 0) ? 0 : MAX_Y_T;
}

////////////////////////////////////////////////////////////////////////////////
// Helper functions for Y/U/V fixed-point calculations.

// The following functions convert r/g/b values in SFIX fixed-point precision
// to 8b values, clipped:
#define YUV_FIX 16
#define TFIX (YUV_FIX + SFIX)
#define TROUNDER (1 << TFIX >> 1)

static uint8_t clip_8b(int v) {
  return (!(v & ~0xff)) ? (uint8_t)v : (v < 0) ? 0u : 255u;
}

static uint8_t ConvertRGBToY(int r, int g, int b) {
  const int luma = 19595 * r + 38469 * g + 7471 * b + TROUNDER;
  return clip_8b(luma >> TFIX);
}

static uint8_t ConvertRGBToU(int r, int g, int b) {
  const int u =  -11058 * r - 21709 * g + 32768 * b + TROUNDER;
  return clip_8b(128 + (u >> TFIX));
}

static uint8_t ConvertRGBToV(int r, int g, int b) {
  const int v = +32768 * r - 27439 * g - 5328 * b + TROUNDER;
  return clip_8b(128 + (v >> TFIX));
}

// convert to luma using 16b precision:
static void ConvertRowToY(const uint8_t* row, int w, uint8_t* const dst) {
  for (int i = 0; i < w; i += 1, row += 3) {
    const int r = row[0], g = row[1], b = row[2];
    const int y = 19595 * r + 38469 * g + 7471 * b;
    dst[i] = (y + (1 << YUV_FIX >> 1)) >> YUV_FIX;
  }
}

static void ConvertRowToUV(const uint8_t* row1, const uint8_t* row2,
                           int w, uint8_t* u, uint8_t* v) {
  for (int i = 0; i < (w & ~1); i += 2, row1 += 6, row2 += 6) {
    const int r = row1[0] + row1[3] + row2[0] + row2[3];
    const int g = row1[1] + row1[4] + row2[1] + row2[4];
    const int b = row1[2] + row1[5] + row2[2] + row2[5];
    *u++ = ConvertRGBToU(r, g, b);
    *v++ = ConvertRGBToV(r, g, b);
  }
  if (w & 1) {
    const int r = 2 * (row1[0] + row2[0]);
    const int g = 2 * (row1[1] + row2[1]);
    const int b = 2 * (row1[2] + row2[2]);
    *u++ = ConvertRGBToU(r, g, b);
    *v++ = ConvertRGBToV(r, g, b);
  }
}

#undef TFIX
#undef ROUNDER

////////////////////////////////////////////////////////////////////////////////
// Sharp RGB->YUV conversion

static const int kNumIterations = 4;
static const int kMinDimensionIterativeConversion = 4;

// size of the interpolation table for linear-to-gamma
#define GAMMA_TABLE_SIZE 32
static uint32_t kLinearToGammaTab[GAMMA_TABLE_SIZE + 2];
#define GAMMA_TO_LINEAR_BITS 14
static uint32_t kGammaToLinearTab[MAX_Y_T + 1];   // size scales with Y_FIX

static void InitGammaTablesF(void) {
  static bool done = false;
  assert(2 * GAMMA_TO_LINEAR_BITS < 32);  // we use uint32_t intermediate values
  if (!done) {
    int v;
    const double norm = 1. / MAX_Y_T;
    const double scale = 1. / GAMMA_TABLE_SIZE;
    const double a = 0.099;
    const double thresh = 0.018;
    const double gamma = 1. / 0.45;
    const double final_scale = 1 << GAMMA_TO_LINEAR_BITS;
    for (v = 0; v <= MAX_Y_T; ++v) {
      const double g = norm * v;
      double value;
      if (g <= thresh * 4.5) {
        value = g / 4.5;
      } else {
        const double a_rec = 1. / (1. + a);
        value = pow(a_rec * (g + a), gamma);
      }
      kGammaToLinearTab[v] = static_cast<uint32_t>(value * final_scale + .5);
    }
    for (v = 0; v <= GAMMA_TABLE_SIZE; ++v) {
      const double g = scale * v;
      double value;
      if (g <= thresh) {
        value = 4.5 * g;
      } else {
        value = (1. + a) * pow(g, 1. / gamma) - a;
      }
      // we already incorporate the 1/2 rounding constant here
      kLinearToGammaTab[v] =
          static_cast<uint32_t>(MAX_Y_T * value)
            + (1 << GAMMA_TO_LINEAR_BITS >> 1);
    }
    // to prevent small rounding errors to cause read-overflow:
    kLinearToGammaTab[GAMMA_TABLE_SIZE + 1] =
        kLinearToGammaTab[GAMMA_TABLE_SIZE];
    done = true;
  }
}

// return value has a fixed-point precision of GAMMA_TO_LINEAR_BITS
static uint32_t GammaToLinear(int v) { return kGammaToLinearTab[v]; }

static uint32_t LinearToGamma(uint32_t value) {
  // 'value' is in GAMMA_TO_LINEAR_BITS fractional precision
  const uint32_t v = value * GAMMA_TABLE_SIZE;
  const uint32_t tab_pos = v >> GAMMA_TO_LINEAR_BITS;
  // fractional part, in GAMMA_TO_LINEAR_BITS fixed-point precision
  const uint32_t x = v - (tab_pos << GAMMA_TO_LINEAR_BITS);  // fractional part
  // v0 / v1 are in GAMMA_TO_LINEAR_BITS fixed-point precision (range [0..1])
  const uint32_t v0 = kLinearToGammaTab[tab_pos + 0];
  const uint32_t v1 = kLinearToGammaTab[tab_pos + 1];
  // Final interpolation. Note that rounding is already included.
  const uint32_t v2 = (v1 - v0) * x;    // note: v1 >= v0.
  const uint32_t result = v0 + (v2 >> GAMMA_TO_LINEAR_BITS);
  return result;
}

//------------------------------------------------------------------------------

static uint64_t SharpUpdateY_C(const uint16_t* ref, const uint16_t* src,
                               uint16_t* dst, int len) {
  uint64_t diff = 0;
  for (int i = 0; i < len; ++i) {
    const int diff_y = ref[i] - src[i];
    const int new_y = static_cast<int>(dst[i]) + diff_y;
    dst[i] = clip_y(new_y);
    diff += (uint64_t)abs(diff_y);
  }
  return diff;
}

static void SharpUpdateRGB_C(const int16_t* ref, const int16_t* src,
                             int16_t* dst, int len) {
  for (int i = 0; i < len; ++i) {
    const int diff_uv = ref[i] - src[i];
    dst[i] += diff_uv;
  }
}

static void SharpFilterRow_C(const int16_t* A, const int16_t* B, int len,
                             const uint16_t* best_y, uint16_t* out) {
  for (int i = 0; i < len; ++i, ++A, ++B) {
    const int v0 = (A[0] * 9 + A[1] * 3 + B[0] * 3 + B[1] + 8) >> 4;
    const int v1 = (A[1] * 9 + A[0] * 3 + B[1] * 3 + B[0] + 8) >> 4;
    out[2 * i + 0] = clip_y(best_y[2 * i + 0] + v0);
    out[2 * i + 1] = clip_y(best_y[2 * i + 1] + v1);
  }
}

#if defined(SJPEG_USE_SSE2)

#define LOAD_16(P) (_mm_loadu_si128(reinterpret_cast<const __m128i*>(P)))
#define STORE_16(P, V) (_mm_storeu_si128(reinterpret_cast<__m128i*>(P), (V)))

static uint64_t SharpUpdateY_SSE2(const uint16_t* ref, const uint16_t* src,
                                  uint16_t* dst, int len) {
  uint64_t diff = 0;
  uint32_t tmp[4];
  int i;
  const __m128i zero = _mm_setzero_si128();
  const __m128i max = _mm_set1_epi16(MAX_Y_T);
  const __m128i one = _mm_set1_epi16(1);
  __m128i sum = zero;

  for (i = 0; i + 8 <= len; i += 8) {
    const __m128i A = LOAD_16(ref + i);
    const __m128i B = LOAD_16(src + i);
    const __m128i C = LOAD_16(dst + i);
    const __m128i D = _mm_sub_epi16(A, B);       // diff_y
    const __m128i E = _mm_cmpgt_epi16(zero, D);  // sign (-1 or 0)
    const __m128i F = _mm_add_epi16(C, D);       // new_y
    const __m128i G = _mm_or_si128(E, one);      // -1 or 1
    const __m128i H = _mm_max_epi16(_mm_min_epi16(F, max), zero);
    const __m128i I = _mm_madd_epi16(D, G);      // sum(abs(...))
    STORE_16(dst + i, H);
    sum = _mm_add_epi32(sum, I);
  }
  STORE_16(tmp, sum);
  diff = tmp[3] + tmp[2] + tmp[1] + tmp[0];
  for (; i < len; ++i) {
    const int diff_y = ref[i] - src[i];
    const int new_y = static_cast<int>(dst[i]) + diff_y;
    dst[i] = clip_y(new_y);
    diff += (uint64_t)abs(diff_y);
  }
  return diff;
}

static void SharpUpdateRGB_SSE2(const int16_t* ref, const int16_t* src,
                                int16_t* dst, int len) {
  int i = 0;
  for (i = 0; i + 8 <= len; i += 8) {
    const __m128i A = LOAD_16(ref + i);
    const __m128i B = LOAD_16(src + i);
    const __m128i C = LOAD_16(dst + i);
    const __m128i D = _mm_sub_epi16(A, B);   // diff_uv
    const __m128i E = _mm_add_epi16(C, D);   // new_uv
    STORE_16(dst + i, E);
  }
  for (; i < len; ++i) {
    const int diff_uv = ref[i] - src[i];
    dst[i] += diff_uv;
  }
}

static void SharpFilterRow_SSE2(const int16_t* A, const int16_t* B, int len,
                                const uint16_t* best_y, uint16_t* out) {
  int i;
  const __m128i kCst8 = _mm_set1_epi16(8);
  const __m128i max = _mm_set1_epi16(MAX_Y_T);
  const __m128i zero = _mm_setzero_si128();
  for (i = 0; i + 8 <= len; i += 8) {
    const __m128i a0 = LOAD_16(A + i + 0);
    const __m128i a1 = LOAD_16(A + i + 1);
    const __m128i b0 = LOAD_16(B + i + 0);
    const __m128i b1 = LOAD_16(B + i + 1);
    const __m128i a0b1 = _mm_add_epi16(a0, b1);
    const __m128i a1b0 = _mm_add_epi16(a1, b0);
    const __m128i a0a1b0b1 = _mm_add_epi16(a0b1, a1b0);  // A0+A1+B0+B1
    const __m128i a0a1b0b1_8 = _mm_add_epi16(a0a1b0b1, kCst8);
    const __m128i a0b1_2 = _mm_add_epi16(a0b1, a0b1);    // 2*(A0+B1)
    const __m128i a1b0_2 = _mm_add_epi16(a1b0, a1b0);    // 2*(A1+B0)
    const __m128i c0 = _mm_srai_epi16(_mm_add_epi16(a0b1_2, a0a1b0b1_8), 3);
    const __m128i c1 = _mm_srai_epi16(_mm_add_epi16(a1b0_2, a0a1b0b1_8), 3);
    const __m128i d0 = _mm_add_epi16(c1, a0);
    const __m128i d1 = _mm_add_epi16(c0, a1);
    const __m128i e0 = _mm_srai_epi16(d0, 1);
    const __m128i e1 = _mm_srai_epi16(d1, 1);
    const __m128i f0 = _mm_unpacklo_epi16(e0, e1);
    const __m128i f1 = _mm_unpackhi_epi16(e0, e1);
    const __m128i g0 = LOAD_16(best_y + 2 * i + 0);
    const __m128i g1 = LOAD_16(best_y + 2 * i + 8);
    const __m128i h0 = _mm_add_epi16(g0, f0);
    const __m128i h1 = _mm_add_epi16(g1, f1);
    const __m128i i0 = _mm_max_epi16(_mm_min_epi16(h0, max), zero);
    const __m128i i1 = _mm_max_epi16(_mm_min_epi16(h1, max), zero);
    STORE_16(out + 2 * i + 0, i0);
    STORE_16(out + 2 * i + 8, i1);
  }
  for (; i < len; ++i) {
    //   (9 * A0 + 3 * A1 + 3 * B0 + B1 + 8) >> 4 =
    // = (8 * A0 + 2 * (A1 + B0) + (A0 + A1 + B0 + B1 + 8)) >> 4
    // We reuse the common sub-expressions.
    const int a0b1 = A[i + 0] + B[i + 1];
    const int a1b0 = A[i + 1] + B[i + 0];
    const int a0a1b0b1 = a0b1 + a1b0 + 8;
    const int v0 = (8 * A[i + 0] + 2 * a1b0 + a0a1b0b1) >> 4;
    const int v1 = (8 * A[i + 1] + 2 * a0b1 + a0a1b0b1) >> 4;
    out[2 * i + 0] = clip_y(best_y[2 * i + 0] + v0);
    out[2 * i + 1] = clip_y(best_y[2 * i + 1] + v1);
  }
}
#undef STORE_16
#undef LOAD_16

#elif defined(SJPEG_USE_NEON)

static uint64_t SharpUpdateY_NEON(const uint16_t* ref, const uint16_t* src,
                                  uint16_t* dst, int len) {
  int i;
  const int16x8_t zero = vdupq_n_s16(0);
  const int16x8_t max = vdupq_n_s16(MAX_Y_T);
  uint64x2_t sum = vdupq_n_u64(0);

  for (i = 0; i + 8 <= len; i += 8) {
    const int16x8_t A = vreinterpretq_s16_u16(vld1q_u16(ref + i));
    const int16x8_t B = vreinterpretq_s16_u16(vld1q_u16(src + i));
    const int16x8_t C = vreinterpretq_s16_u16(vld1q_u16(dst + i));
    const int16x8_t D = vsubq_s16(A, B);       // diff_y
    const int16x8_t F = vaddq_s16(C, D);       // new_y
    const uint16x8_t H =
        vreinterpretq_u16_s16(vmaxq_s16(vminq_s16(F, max), zero));
    const int16x8_t I = vabsq_s16(D);          // abs(diff_y)
    vst1q_u16(dst + i, H);
    sum = vpadalq_u32(sum, vpaddlq_u16(vreinterpretq_u16_s16(I)));
  }
  uint64_t diff = vgetq_lane_u64(sum, 0) + vgetq_lane_u64(sum, 1);
  for (; i < len; ++i) {
    const int diff_y = ref[i] - src[i];
    const int new_y = static_cast<int>(dst[i]) + diff_y;
    dst[i] = clip_y(new_y);
    diff += static_cast<uint64_t>(abs(diff_y));
  }
  return diff;
}

static void SharpUpdateRGB_NEON(const int16_t* ref, const int16_t* src,
                                int16_t* dst, int len) {
  int i;
  for (i = 0; i + 8 <= len; i += 8) {
    const int16x8_t A = vld1q_s16(ref + i);
    const int16x8_t B = vld1q_s16(src + i);
    const int16x8_t C = vld1q_s16(dst + i);
    const int16x8_t D = vsubq_s16(A, B);   // diff_uv
    const int16x8_t E = vaddq_s16(C, D);   // new_uv
    vst1q_s16(dst + i, E);
  }
  for (; i < len; ++i) {
    const int diff_uv = ref[i] - src[i];
    dst[i] += diff_uv;
  }
}

static void SharpFilterRow_NEON(const int16_t* A, const int16_t* B, int len,
                                const uint16_t* best_y, uint16_t* out) {
  int i;
  const int16x8_t max = vdupq_n_s16(MAX_Y_T);
  const int16x8_t zero = vdupq_n_s16(0);
  for (i = 0; i + 8 <= len; i += 8) {
    const int16x8_t a0 = vld1q_s16(A + i + 0);
    const int16x8_t a1 = vld1q_s16(A + i + 1);
    const int16x8_t b0 = vld1q_s16(B + i + 0);
    const int16x8_t b1 = vld1q_s16(B + i + 1);
    const int16x8_t a0b1 = vaddq_s16(a0, b1);
    const int16x8_t a1b0 = vaddq_s16(a1, b0);
    const int16x8_t a0a1b0b1 = vaddq_s16(a0b1, a1b0);  // A0+A1+B0+B1
    const int16x8_t a0b1_2 = vaddq_s16(a0b1, a0b1);    // 2*(A0+B1)
    const int16x8_t a1b0_2 = vaddq_s16(a1b0, a1b0);    // 2*(A1+B0)
    const int16x8_t c0 = vshrq_n_s16(vaddq_s16(a0b1_2, a0a1b0b1), 3);
    const int16x8_t c1 = vshrq_n_s16(vaddq_s16(a1b0_2, a0a1b0b1), 3);
    const int16x8_t d0 = vaddq_s16(c1, a0);
    const int16x8_t d1 = vaddq_s16(c0, a1);
    const int16x8_t e0 = vrshrq_n_s16(d0, 1);
    const int16x8_t e1 = vrshrq_n_s16(d1, 1);
    const int16x8x2_t f = vzipq_s16(e0, e1);
    const int16x8_t g0 = vreinterpretq_s16_u16(vld1q_u16(best_y + 2 * i + 0));
    const int16x8_t g1 = vreinterpretq_s16_u16(vld1q_u16(best_y + 2 * i + 8));
    const int16x8_t h0 = vaddq_s16(g0, f.val[0]);
    const int16x8_t h1 = vaddq_s16(g1, f.val[1]);
    const int16x8_t i0 = vmaxq_s16(vminq_s16(h0, max), zero);
    const int16x8_t i1 = vmaxq_s16(vminq_s16(h1, max), zero);
    vst1q_u16(out + 2 * i + 0, vreinterpretq_u16_s16(i0));
    vst1q_u16(out + 2 * i + 8, vreinterpretq_u16_s16(i1));
  }
  for (; i < len; ++i) {
    const int a0b1 = A[i + 0] + B[i + 1];
    const int a1b0 = A[i + 1] + B[i + 0];
    const int a0a1b0b1 = a0b1 + a1b0 + 8;
    const int v0 = (8 * A[i + 0] + 2 * a1b0 + a0a1b0b1) >> 4;
    const int v1 = (8 * A[i + 1] + 2 * a0b1 + a0a1b0b1) >> 4;
    out[2 * i + 0] = clip_y(best_y[2 * i + 0] + v0);
    out[2 * i + 1] = clip_y(best_y[2 * i + 1] + v1);
  }
}

#endif    // SJPEG_USE_NEON

static uint64_t (*kSharpUpdateY)(const uint16_t* src, const uint16_t* ref,
                                 uint16_t* dst, int len);
static void (*kSharpUpdateRGB)(const int16_t* src, const int16_t* ref,
                               int16_t* dst, int len);
static void (*kSharpFilterRow)(const int16_t* A, const int16_t* B,
                               int len, const uint16_t* best_y, uint16_t* out);

static void InitFunctionPointers() {
  static bool done = false;
  if (!done) {
    kSharpUpdateY = SharpUpdateY_C;
    kSharpUpdateRGB = SharpUpdateRGB_C;
    kSharpFilterRow = SharpFilterRow_C;
#if defined(SJPEG_USE_SSE2)
    if (sjpeg::SupportsSSE2()) {
      kSharpUpdateY = SharpUpdateY_SSE2;
      kSharpUpdateRGB = SharpUpdateRGB_SSE2;
      kSharpFilterRow = SharpFilterRow_SSE2;
    }
#endif
#if defined(SJPEG_USE_NEON)
    if (sjpeg::SupportsNEON()) {
      kSharpUpdateY = SharpUpdateY_NEON;
      kSharpUpdateRGB = SharpUpdateRGB_NEON;
      kSharpFilterRow = SharpFilterRow_NEON;
    }
#endif
    done = true;
  }
}

//------------------------------------------------------------------------------

static uint32_t RGBToGray(uint32_t r, uint32_t g, uint32_t b) {
  const uint32_t luma = 13933 * r + 46871 * g + 4732 * b + (1u << YUV_FIX >> 1);
  return (luma >> YUV_FIX);
}

static uint32_t ScaleDown(int a, int b, int c, int d) {
  const uint32_t A = GammaToLinear(a);
  const uint32_t B = GammaToLinear(b);
  const uint32_t C = GammaToLinear(c);
  const uint32_t D = GammaToLinear(d);
  return LinearToGamma((A + B + C + D + 2) >> 2);
}

static void UpdateChroma(const fixed_y_t* src1, const fixed_y_t* src2,
                         fixed_t* dst, size_t uv_w) {
  for (size_t i = 0; i < uv_w; ++i) {
    const uint32_t r = ScaleDown(src1[0 * uv_w + 0], src1[0 * uv_w + 1],
                                 src2[0 * uv_w + 0], src2[0 * uv_w + 1]);
    const uint32_t g = ScaleDown(src1[2 * uv_w + 0], src1[2 * uv_w + 1],
                                 src2[2 * uv_w + 0], src2[2 * uv_w + 1]);
    const uint32_t b = ScaleDown(src1[4 * uv_w + 0], src1[4 * uv_w + 1],
                                 src2[4 * uv_w + 0], src2[4 * uv_w + 1]);
    const int W = RGBToGray(r, g, b);
    dst[0 * uv_w] = (fixed_t)(r - W);
    dst[1 * uv_w] = (fixed_t)(g - W);
    dst[2 * uv_w] = (fixed_t)(b - W);
    dst  += 1;
    src1 += 2;
    src2 += 2;
  }
}

static void UpdateW(const fixed_y_t* src, fixed_y_t* dst, int w) {
  for (int i = 0; i < w; ++i) {
    const uint32_t R = GammaToLinear(src[0 * w + i]);
    const uint32_t G = GammaToLinear(src[1 * w + i]);
    const uint32_t B = GammaToLinear(src[2 * w + i]);
    const uint32_t Y = RGBToGray(R, G, B);
    dst[i] = (fixed_y_t)LinearToGamma(Y);
  }
}

static void StoreGray(const fixed_y_t* const rgb, fixed_y_t* const y, int w) {
  for (int i = 0; i < w; ++i) {
    y[i] = RGBToGray(rgb[0 * w + i], rgb[1 * w + i], rgb[2 * w + i]);
  }
}

//------------------------------------------------------------------------------

static fixed_y_t Filter2(int A, int B, int W0) {
  const int v0 = (A * 3 + B + 2) >> 2;
  return clip_y(v0 + W0);
}

//------------------------------------------------------------------------------

static fixed_y_t UpLift(uint8_t a) {  // 8bit -> SFIX
  return ((fixed_y_t)a << SFIX) | SHALF;
}

static void ImportOneRow(const uint8_t* const rgb, int pic_width,
                         fixed_y_t* const dst) {
  const int w = (pic_width + 1) & ~1;
  for (int i = 0; i < pic_width; ++i) {
    const int off = i * 3;
    dst[i + 0 * w] = UpLift(rgb[off + 0]);
    dst[i + 1 * w] = UpLift(rgb[off + 1]);
    dst[i + 2 * w] = UpLift(rgb[off + 2]);
  }
  if (pic_width & 1) {  // replicate rightmost pixel
    dst[pic_width + 0 * w] = dst[pic_width + 0 * w - 1];
    dst[pic_width + 1 * w] = dst[pic_width + 1 * w - 1];
    dst[pic_width + 2 * w] = dst[pic_width + 2 * w - 1];
  }
}

static void InterpolateTwoRows(const fixed_y_t* const best_y,
                               const fixed_t* prev_uv,
                               const fixed_t* cur_uv,
                               const fixed_t* next_uv,
                               int w,
                               fixed_y_t* out1, fixed_y_t* out2) {
  const int uv_w = w >> 1;
  const int len = (w - 1) >> 1;   // length to filter
  for (int k = 3; k > 0; --k) {  // process each R/G/B segments in turn
    // special boundary case for i==0
    out1[0] = Filter2(cur_uv[0], prev_uv[0], best_y[0]);
    out2[0] = Filter2(cur_uv[0], next_uv[0], best_y[w]);

    kSharpFilterRow(cur_uv, prev_uv, len, best_y + 0 + 1, out1 + 1);
    kSharpFilterRow(cur_uv, next_uv, len, best_y + w + 1, out2 + 1);

    // special boundary case for i == w - 1 when w is even
    if (!(w & 1)) {
      out1[w - 1] = Filter2(cur_uv[uv_w - 1], prev_uv[uv_w - 1],
                            best_y[w - 1 + 0]);
      out2[w - 1] = Filter2(cur_uv[uv_w - 1], next_uv[uv_w - 1],
                            best_y[w - 1 + w]);
    }
    out1 += w;
    out2 += w;
    prev_uv += uv_w;
    cur_uv  += uv_w;
    next_uv += uv_w;
  }
}

static void ConvertWRGBToYUV(const fixed_y_t* best_y,
                             const fixed_t* best_uv,
                             int width, int height,
                             uint8_t* y_plane,
                             uint8_t* u_plane, uint8_t* v_plane) {
  const int w = (width + 1) & ~1;
  const int h = (height + 1) & ~1;
  const int uv_w = w >> 1;
  const int uv_h = h >> 1;
  for (int j = 0; j < height; ++j) {
    const int off = (j >> 1) * 3 * uv_w;
    for (int i = 0; i < width; ++i) {
      const int W = best_y[i + j * w];
      const int r = best_uv[off + (i >> 1) + 0 * uv_w] + W;
      const int g = best_uv[off + (i >> 1) + 1 * uv_w] + W;
      const int b = best_uv[off + (i >> 1) + 2 * uv_w] + W;
      y_plane[i] = ConvertRGBToY(r, g, b);
    }
    y_plane += width;
  }
  for (int j = 0; j < uv_h; ++j) {
    for (int i = 0; i < uv_w; ++i) {
      const int off = i + j * 3 * uv_w;
      const int r = best_uv[off + 0 * uv_w];
      const int g = best_uv[off + 1 * uv_w];
      const int b = best_uv[off + 2 * uv_w];
      u_plane[i] = ConvertRGBToU(r, g, b);
      v_plane[i] = ConvertRGBToV(r, g, b);
    }
    u_plane += uv_w;
    v_plane += uv_w;
  }
}

//------------------------------------------------------------------------------
// Main function

static void PreprocessARGB(const uint8_t* const rgb,
                           int width, int height, size_t stride,
                           uint8_t* y_plane,
                           uint8_t* u_plane, uint8_t* v_plane) {
  // we expand the right/bottom border if needed
  const int w = (width + 1) & ~1;
  const int h = (height + 1) & ~1;
  const int uv_w = w >> 1;
  const int uv_h = h >> 1;
  uint64_t prev_diff_y_sum = ~0;

  InitGammaTablesF();
  InitFunctionPointers();

  // TODO(skal): allocate one big memory chunk instead.
  vector<fixed_y_t> tmp_buffer(w * 3 * 2);
  vector<fixed_y_t> best_y(w * h);
  vector<fixed_y_t> target_y(w * h);
  vector<fixed_y_t> best_rgb_y(w * 2);
  vector<fixed_t> best_uv(uv_w * 3 * uv_h);
  vector<fixed_t> target_uv(uv_w * 3 * uv_h);
  vector<fixed_t> best_rgb_uv(uv_w * 3 * 1);
  const uint64_t diff_y_threshold = static_cast<uint64_t>(3.0 * w * h);

  assert(width >= kMinDimensionIterativeConversion);
  assert(height >= kMinDimensionIterativeConversion);

  // Import RGB samples to W/RGB representation.
  for (int j = 0; j < height; j += 2) {
    const int is_last_row = (j == height - 1);
    fixed_y_t* const src1 = &tmp_buffer[0 * w];
    fixed_y_t* const src2 = &tmp_buffer[3 * w];
    const int rgb_off = j * stride;
    const int y_off = j * w;
    const int uv_off = (j >> 1) * 3 * uv_w;

    // prepare two rows of input
    ImportOneRow(rgb + rgb_off, width, src1);
    if (!is_last_row) {
      ImportOneRow(rgb + rgb_off + stride, width, src2);
    } else {
      memcpy(src2, src1, 3 * w * sizeof(*src2));
    }
    StoreGray(src1, &best_y[y_off + 0], w);
    StoreGray(src2, &best_y[y_off + w], w);
    UpdateW(src1, &target_y[y_off + 0], w);
    UpdateW(src2, &target_y[y_off + w], w);
    UpdateChroma(src1, src2, &target_uv[uv_off], uv_w);
    memcpy(&best_uv[uv_off], &target_uv[uv_off], 3 * uv_w * sizeof(best_uv[0]));
  }

  // Iterate and resolve clipping conflicts.
  for (int iter = 0; iter < kNumIterations; ++iter) {
    const fixed_t* cur_uv = &best_uv[0];
    const fixed_t* prev_uv = &best_uv[0];
    uint64_t diff_y_sum = 0;

    for (int j = 0; j < h; j += 2) {
      const int uv_off = (j >> 1) * 3 * uv_w;
      fixed_y_t* const src1 = &tmp_buffer[0 * w];
      fixed_y_t* const src2 = &tmp_buffer[3 * w];
      const fixed_t* const next_uv = cur_uv + ((j < h - 2) ? 3 * uv_w : 0);
      InterpolateTwoRows(&best_y[j * w], prev_uv, cur_uv, next_uv,
                         w, src1, src2);
      prev_uv = cur_uv;
      cur_uv = next_uv;

      UpdateW(src1, &best_rgb_y[0 * w], w);
      UpdateW(src2, &best_rgb_y[1 * w], w);
      UpdateChroma(src1, src2, &best_rgb_uv[0], uv_w);

      // update two rows of Y and one row of RGB
      diff_y_sum += kSharpUpdateY(&target_y[j * w],
                                  &best_rgb_y[0], &best_y[j * w], 2 * w);
      kSharpUpdateRGB(&target_uv[uv_off],
                      &best_rgb_uv[0], &best_uv[uv_off], 3 * uv_w);
    }
    // test exit condition
    if (iter > 0) {
      if (diff_y_sum < diff_y_threshold) break;
      if (diff_y_sum > prev_diff_y_sum) break;
    }
    prev_diff_y_sum = diff_y_sum;
  }
  // final reconstruction
  ConvertWRGBToYUV(&best_y[0], &best_uv[0], width, height,
                   y_plane, u_plane, v_plane);
}

}  // namespace sjpeg

////////////////////////////////////////////////////////////////////////////////
// Entry point

void sjpeg::ApplySharpYUVConversion(const uint8_t* const rgb,
                                    int W, int H, int stride,
                                    uint8_t* y_plane,
                                    uint8_t* u_plane, uint8_t* v_plane) {
  if (W <= kMinDimensionIterativeConversion ||
      H <= kMinDimensionIterativeConversion) {
    const int uv_w = (W + 1) >> 1;
    for (int y = 0; y < H; y += 2) {
      const uint8_t* const rgb1 = rgb + y * stride;
      const uint8_t* const rgb2 = (y < H - 1) ? rgb1 + stride : rgb1;
      ConvertRowToY(rgb1, W, &y_plane[y * W]);
      if (y < H - 1) {
        ConvertRowToY(rgb2, W, &y_plane[(y + 1) * W]);
      }
      ConvertRowToUV(rgb1, rgb2, W,
                     &u_plane[(y >> 1) * uv_w],
                     &v_plane[(y >> 1) * uv_w]);
    }
  } else {
    PreprocessARGB(rgb, W, H, stride, y_plane, u_plane, v_plane);
  }
}

////////////////////////////////////////////////////////////////////////////////

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
//  RGB -> YUV conversion
//
// Y       = ( 19595 * r + 38469 * g +  7471 * b + HALF) >> FRAC
// U - 128 = (-11059 * r - 21709 * g + 32768 * b + HALF) >> FRAC
// V - 128 = ( 32768 * r - 27439 * g -  5329 * b + HALF) >> FRAC
//
// Author: Skal (pascal.massimino@gmail.com)

#include <string.h>

#define SJPEG_NEED_ASM_HEADERS
#include "sjpegi.h"

namespace sjpeg {

// global fixed-point precision
enum { FRAC = 16, HALF = 1 << FRAC >> 1,
       ROUND_UV = (HALF << 2), ROUND_Y = HALF - (128 << FRAC) };

#if defined(SJPEG_USE_SSE2)

// Load eight 16b-words from *src.
#define LOAD_16(src) _mm_loadu_si128(reinterpret_cast<const __m128i*>(src))
// Store eight 16b-words into *dst
#define STORE_16(V, dst) _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), (V))

// Convert 8 packed RGB samples to r[], g[], b[]
static inline void RGB24PackedToPlanar(const uint8_t* const rgb,
                                       __m128i* const r,
                                       __m128i* const g,
                                       __m128i* const b) {
  const __m128i zero = _mm_setzero_si128();
  // in0: r0 g0 b0 r1 | g1 b1 r2 g2 | b2 r3 g3 b3 | r4 g4 b4 r5
  // in1: b2 r3 g3 b3 | r4 g4 b4 r5 | g5 b5 r6 g6 | b6 r7 g7 b7
  const __m128i in0 = LOAD_16(rgb + 0);
  const __m128i in1 = LOAD_16(rgb + 8);
  // A0: | r2 g2 b2 r3 | g3 b3 r4 g4 | b4 r5 ...
  // A1:                   ... b2 r3 | g3 b3 r4 g4 | b4 r5 g5 b5 |
  const __m128i A0 = _mm_srli_si128(in0, 6);
  const __m128i A1 = _mm_slli_si128(in1, 6);
  // B0: r0 r2 g0 g2 | b0 b2 r1 r3 | g1 g3 b1 b3 | r2 r4 b2 b4
  // B1: g3 g5 b3 b5 | r4 r6 g4 g6 | b4 b6 r5 r7 | g5 g7 b5 b7
  const __m128i B0 = _mm_unpacklo_epi8(in0, A0);
  const __m128i B1 = _mm_unpackhi_epi8(A1, in1);
  // C0: r1 r3 g1 g3 | b1 b3 r2 r4 | b2 b4 ...
  // C1:                 ... g3 g5 | b3 b5 r4 r6 | g4 g6 b4 b6
  const __m128i C0 = _mm_srli_si128(B0, 6);
  const __m128i C1 = _mm_slli_si128(B1, 6);
  // D0: r0 r1 r2 r3 | g0 g1 g2 g3 | b0 b1 b2 b3 | r1 r2 r3 r4
  // D1: b3 b4 b5 b6 | r4 r5 r6 r7 | g4 g5 g6 g7 | b4 b5 b6 b7 |
  const __m128i D0 = _mm_unpacklo_epi8(B0, C0);
  const __m128i D1 = _mm_unpackhi_epi8(C1, B1);
  // r4 r5 r6 r7 | g4 g5 g6 g7 | b4 b5 b6 b7 | 0
  const __m128i D2 = _mm_srli_si128(D1, 4);
  // r0 r1 r2 r3 | r4 r5 r6 r7 | g0 g1 g2 g3 | g4 g5 g6 g7
  const __m128i E0 = _mm_unpacklo_epi32(D0, D2);
  // b0 b1 b2 b3 | b4 b5 b6 b7 | r1 r2 r3 r4 | 0
  const __m128i E1 = _mm_unpackhi_epi32(D0, D2);
  // g0 g1 g2 g3 | g4 g5 g6 g7 | 0
  const __m128i E2 = _mm_srli_si128(E0, 8);
  const __m128i F0 = _mm_unpacklo_epi8(E0, zero);  // -> R
  const __m128i F1 = _mm_unpacklo_epi8(E1, zero);  // -> B
  const __m128i F2 = _mm_unpacklo_epi8(E2, zero);  // -> G
  *r = F0;
  *b = F1;
  *g = F2;
}

// This macro computes (RG * MULT_RG + GB * MULT_GB + ROUNDER) >> DESCALE_FIX
// It's a macro and not a function because we need to use immediate values with
// srai_epi32, e.g.
#define TRANSFORM(RG_LO, RG_HI, GB_LO, GB_HI, MULT_RG, MULT_GB, \
                  ROUNDER, DESCALE_FIX, ADD_OR_SUB, OUT) do {   \
  const __m128i V0_lo = _mm_madd_epi16(RG_LO, MULT_RG);         \
  const __m128i V0_hi = _mm_madd_epi16(RG_HI, MULT_RG);         \
  const __m128i V1_lo = _mm_madd_epi16(GB_LO, MULT_GB);         \
  const __m128i V1_hi = _mm_madd_epi16(GB_HI, MULT_GB);         \
  const __m128i V2_lo = ADD_OR_SUB(V0_lo, V1_lo);               \
  const __m128i V2_hi = ADD_OR_SUB(V0_hi, V1_hi);               \
  const __m128i V3_lo = _mm_add_epi32(V2_lo, ROUNDER);          \
  const __m128i V3_hi = _mm_add_epi32(V2_hi, ROUNDER);          \
  const __m128i V5_lo = _mm_srai_epi32(V3_lo, DESCALE_FIX);     \
  const __m128i V5_hi = _mm_srai_epi32(V3_hi, DESCALE_FIX);     \
  (OUT) = _mm_packs_epi32(V5_lo, V5_hi);                        \
} while (0)

#define MK_CST_16(A, B) _mm_set_epi16((B), (A), (B), (A), (B), (A), (B), (A))

inline void ConvertRGBToY(const __m128i* const R,
                          const __m128i* const G,
                          const __m128i* const B,
                          int offset,
                          __m128i* const Y) {
  const __m128i kRG_y = MK_CST_16(19595, 38469 - 16384);
  const __m128i kGB_y = MK_CST_16(16384, 7471);
  const __m128i kROUND_Y = _mm_set1_epi32(HALF + (offset << FRAC));
  const __m128i RG_lo = _mm_unpacklo_epi16(*R, *G);
  const __m128i RG_hi = _mm_unpackhi_epi16(*R, *G);
  const __m128i GB_lo = _mm_unpacklo_epi16(*G, *B);
  const __m128i GB_hi = _mm_unpackhi_epi16(*G, *B);
  TRANSFORM(RG_lo, RG_hi, GB_lo, GB_hi, kRG_y, kGB_y, kROUND_Y, FRAC,
            _mm_add_epi32, *Y);
}

inline void ConvertRGBToUV(const __m128i* const R,
                           const __m128i* const G,
                           const __m128i* const B,
                           int offset,
                           __m128i* const U, __m128i* const V) {
  // Warning! 32768 is overflowing int16, so we're actually multiplying
  // by -32768 instead of 32768. We compensate by subtracting the result
  // instead of adding, thus restoring the sign.
  const __m128i kRG_u = MK_CST_16(-11059, -21709);
  const __m128i kGB_u = MK_CST_16(0, -32768);
  const __m128i kRG_v = MK_CST_16(-32768, 0);
  const __m128i kGB_v = MK_CST_16(-27439, -5329);
  const __m128i kRound = _mm_set1_epi32((offset << FRAC) + HALF);

  const __m128i RG_lo = _mm_unpacklo_epi16(*R, *G);
  const __m128i RG_hi = _mm_unpackhi_epi16(*R, *G);
  const __m128i GB_lo = _mm_unpacklo_epi16(*G, *B);
  const __m128i GB_hi = _mm_unpackhi_epi16(*G, *B);

  // _mm_sub_epi32 -> sign restore!
  TRANSFORM(RG_lo, RG_hi, GB_lo, GB_hi, kRG_u, kGB_u, kRound, FRAC,
            _mm_sub_epi32, *U);
  // note! GB and RG are inverted, for sign-restoration
  TRANSFORM(GB_lo, GB_hi, RG_lo, RG_hi, kGB_v, kRG_v, kRound, FRAC,
            _mm_sub_epi32, *V);
}

// This version takes four accumulated R/G/B samples. Hence, the
// descaling factor is FRAC + 2.
inline void ConvertRGBToUVAccumulated(const __m128i* const R,
                                      const __m128i* const G,
                                      const __m128i* const B,
                                      __m128i* const U, __m128i* const V) {
  // Warning! 32768 is overflowing int16, so we're actually multiplying
  // by -32768 instead of 32768. We compensate by subtracting the result
  // instead of adding, thus restoring the sign.
  const __m128i kRG_u = MK_CST_16(-11059, -21709);
  const __m128i kGB_u = MK_CST_16(0, -32768);
  const __m128i kRG_v = MK_CST_16(-32768, 0);
  const __m128i kGB_v = MK_CST_16(-27439, -5329);
  const __m128i kRound = _mm_set1_epi32(ROUND_UV);

  const __m128i RG_lo = _mm_unpacklo_epi16(*R, *G);
  const __m128i RG_hi = _mm_unpackhi_epi16(*R, *G);
  const __m128i GB_lo = _mm_unpacklo_epi16(*G, *B);
  const __m128i GB_hi = _mm_unpackhi_epi16(*G, *B);

  // _mm_sub_epi32 -> sign restore!
  TRANSFORM(RG_lo, RG_hi, GB_lo, GB_hi, kRG_u, kGB_u,
            kRound, FRAC + 2, _mm_sub_epi32, *U);
  // note! GB and RG are inverted, for sign-restoration
  TRANSFORM(GB_lo, GB_hi, RG_lo, RG_hi, kGB_v, kRG_v,
            kRound, FRAC + 2, _mm_sub_epi32, *V);
}

#undef MK_CST_16
#undef TRANSFORM

// Convert 8 RGB samples to YUV. out[] points to a 3*64 data block.
static inline void ToYUV_8(const __m128i* const r,
                           const __m128i* const g,
                           const __m128i* const b,
                           int16_t* const out) {
  __m128i Y, U, V;
  ConvertRGBToY(r, g, b, -128, &Y);
  ConvertRGBToUV(r, g, b, 0, &U, &V);
  STORE_16(Y, out + 0 * 64);
  STORE_16(U, out + 1 * 64);
  STORE_16(V, out + 2 * 64);
}

static void Get8x8Block_SSE2(const uint8_t* data, int step, int16_t* out) {
  for (int y = 8; y > 0; --y) {
    __m128i r, g, b;
    RGB24PackedToPlanar(data, &r, &g, &b);
    ToYUV_8(&r, &g, &b, out);
    out += 8;
    data += step;
  }
}

// Convert 16x16 RGB samples to YUV420
static inline void ToY_16x16(const __m128i* const r,
                             const __m128i* const g,
                             const __m128i* const b,
                             int16_t* const y_out,
                             __m128i* const R_acc,
                             __m128i* const G_acc,
                             __m128i* const B_acc,
                             bool do_add) {
  __m128i Y;
  ConvertRGBToY(r, g, b, -128, &Y);
  STORE_16(Y, y_out);
  if (do_add) {
    *R_acc = _mm_add_epi16(*R_acc, *r);
    *G_acc = _mm_add_epi16(*G_acc, *g);
    *B_acc = _mm_add_epi16(*B_acc, *b);
  } else {  // just store
    *R_acc = *r;
    *G_acc = *g;
    *B_acc = *b;
  }
}

static inline void ToUV_8x8(const __m128i* const R,
                            const __m128i* const G,
                            const __m128i* const B,
                            int16_t* const uv_out) {
  __m128i U, V;
  ConvertRGBToUVAccumulated(R, G, B, &U, &V);
  STORE_16(U, uv_out + 0 * 64);
  STORE_16(V, uv_out + 1 * 64);
}

static void Condense16To8(const __m128i* const acc1, __m128i* const acc2) {
  const __m128i one = _mm_set1_epi16(1);
  const __m128i tmp1 = _mm_madd_epi16(*acc1, one);
  const __m128i tmp2 = _mm_madd_epi16(*acc2, one);
  *acc2 = _mm_packs_epi32(tmp1, tmp2);
}

// convert two 16x8 RGB blocks into two blocks of luma, and 2 blocks of U/V
static void Get16x8_SSE2(const uint8_t* src1, int src_stride,
                         int16_t y[4 * 64], int16_t uv[2 * 64]) {
  for (int i = 4; i > 0; --i, src1 += 2 * src_stride) {
    __m128i r_acc1, r_acc2, g_acc1, g_acc2, b_acc1, b_acc2;
    __m128i r, g, b;
    const uint8_t* const src2 = src1 + src_stride;
    RGB24PackedToPlanar(src1 + 0 * 8, &r, &g, &b);
    ToY_16x16(&r, &g, &b, y + 0 * 64 + 0, &r_acc1, &g_acc1, &b_acc1, false);
    RGB24PackedToPlanar(src1 + 3 * 8, &r, &g, &b);
    ToY_16x16(&r, &g, &b, y + 1 * 64 + 0, &r_acc2, &g_acc2, &b_acc2, false);
    RGB24PackedToPlanar(src2 + 0 * 8, &r, &g, &b);
    ToY_16x16(&r, &g, &b, y + 0 * 64 + 8, &r_acc1, &g_acc1, &b_acc1, true);
    RGB24PackedToPlanar(src2 + 3 * 8, &r, &g, &b);
    ToY_16x16(&r, &g, &b, y + 1 * 64 + 8, &r_acc2, &g_acc2, &b_acc2, true);
    Condense16To8(&r_acc1, &r_acc2);
    Condense16To8(&g_acc1, &g_acc2);
    Condense16To8(&b_acc1, &b_acc2);
    ToUV_8x8(&r_acc2, &g_acc2, &b_acc2, uv);
    y += 2 * 8;
    uv += 8;
  }
}

static void Get16x16Block_SSE2(const uint8_t* data, int step, int16_t* blocks) {
  Get16x8_SSE2(data + 0 * step, step, blocks + 0 * 64, blocks + 4 * 64 + 0 * 8);
  Get16x8_SSE2(data + 8 * step, step, blocks + 2 * 64, blocks + 4 * 64 + 4 * 8);
}

#undef LOAD_16
#undef STORE_16

#endif    // SJPEG_USE_SSE2

///////////////////////////////////////////////////////////////////////////////
// NEON-version for 8x8 and 16x16 blocks

#if defined(SJPEG_USE_NEON)

static const int16_t kCoeff1[4] = { (int16_t)38469, 19595, 7471, 0 };
static const int16_t kCoeff2[4] = { 21709, 11059, 27439, 5329 };

// Convert 8 packed RGB or BGR samples to r[], g[], b[]
static void RGB24PackedToPlanar(const uint8_t* const rgb,
                                int16x8_t* const r,
                                int16x8_t* const g,
                                int16x8_t* const b) {
  const uint8x8x3_t in = vld3_u8(rgb);
  *r = vreinterpretq_s16_u16(vmovl_u8(in.val[0]));
  *g = vreinterpretq_s16_u16(vmovl_u8(in.val[1]));
  *b = vreinterpretq_s16_u16(vmovl_u8(in.val[2]));
}

// s16->s32 widening multiply with large (>=32768) coeff requires special care:
#define MULT_S32_S16_LARGE(S16, COEFF, LANE)                                 \
  vreinterpretq_s32_u32(vmull_lane_u16(vreinterpret_u16_s16(S16),            \
                                       vreinterpret_u16_s16(COEFF), (LANE)))

static inline void ConvertRGBToY(const int16x8_t R,
                                 const int16x8_t G,
                                 const int16x8_t B,
                                 const int16x4_t coeffs,
                                 int16x8_t* const Y) {
  int32x4_t lo = MULT_S32_S16_LARGE(vget_low_s16(G), coeffs, 0);
  int32x4_t hi = MULT_S32_S16_LARGE(vget_high_s16(G), coeffs, 0);
  lo = vmlal_lane_s16(lo, vget_low_s16(R), coeffs, 1);
  hi = vmlal_lane_s16(hi, vget_high_s16(R), coeffs, 1);
  lo = vmlal_lane_s16(lo, vget_low_s16(B), coeffs, 2);
  hi = vmlal_lane_s16(hi, vget_high_s16(B), coeffs, 2);
  const int16x8_t V0 = vcombine_s16(vqmovn_s32(vrshrq_n_s32(lo, FRAC)),
                                    vqmovn_s32(vrshrq_n_s32(hi, FRAC)));
  *Y = vsubq_s16(V0, vdupq_n_s16(128));
}

// Compute ((V0<<15) - V1 * C1 - V2 * C2 + round) >> SHIFT
#define DOT_PROD_PREAMBLE(V0, V1, V2, COEFF, LANE1, LANE2)                  \
  int32x4_t lo, hi;                                                         \
  do {                                                                      \
    lo = vshll_n_s16(vget_low_s16(V0), 15);                                 \
    hi = vshll_n_s16(vget_high_s16(V0), 15);                                \
    lo = vmlsl_lane_s16(lo, vget_low_s16(V1), COEFF, LANE1);                \
    hi = vmlsl_lane_s16(hi, vget_high_s16(V1), COEFF, LANE1);               \
    lo = vmlsl_lane_s16(lo, vget_low_s16(V2), COEFF, LANE2);                \
    hi = vmlsl_lane_s16(hi, vget_high_s16(V2), COEFF, LANE2);               \
} while (0)

// This version assumes SHIFT <= 16
#define DOT_PROD1(V0, V1, V2, COEFF, LANE1, LANE2, SHIFT, OUT) do {         \
  assert(SHIFT <= 16);                                                      \
  DOT_PROD_PREAMBLE(V0, V1, V2, COEFF, LANE1, LANE2);                       \
  (OUT) = vcombine_s16(vrshrn_n_s32(lo, SHIFT), vrshrn_n_s32(hi, SHIFT));   \
} while (0)

// alternate version for SHIFT > 16
#define DOT_PROD2(V0, V1, V2, COEFF, LANE1, LANE2, SHIFT, OUT) do {         \
  assert(SHIFT > 16);                                                       \
  DOT_PROD_PREAMBLE(V0, V1, V2, COEFF, LANE1, LANE2);                       \
  (OUT) = vcombine_s16(vqmovn_s32(vrshrq_n_s32(lo, SHIFT)),                 \
                       vqmovn_s32(vrshrq_n_s32(hi, SHIFT)));                \
} while (0)

static inline void ConvertRGBToUV(const int16x8_t R,
                                  const int16x8_t G,
                                  const int16x8_t B,
                                  const int16x4_t coeffs,
                                  int16x8_t* const U, int16x8_t* const V) {
  DOT_PROD1(B, G, R, coeffs, 0, 1, FRAC, *U);
  DOT_PROD1(R, G, B, coeffs, 2, 3, FRAC, *V);
}

static inline void ConvertRGBToUVAccumulated(const int16x8_t R,
                                             const int16x8_t G,
                                             const int16x8_t B,
                                             const int16x4_t coeffs,
                                             int16x8_t* const U,
                                             int16x8_t* const V) {
  DOT_PROD2(B, G, R, coeffs, 0, 1, FRAC + 2, *U);
  DOT_PROD2(R, G, B, coeffs, 2, 3, FRAC + 2, *V);
}

// Convert 8 RGB samples to YUV. out[] points to a 3*64 data block.
static void ToYUV_8(const int16x8_t r, const int16x8_t g, const int16x8_t b,
                    const int16x4_t coeffs1, const int16x4_t coeffs2,
                    int16_t* const out) {
  int16x8_t Y, U, V;
  ConvertRGBToY(r, g, b, coeffs1, &Y);
  ConvertRGBToUV(r, g, b, coeffs2, &U, &V);
  vst1q_s16(out + 0 * 64, Y);
  vst1q_s16(out + 1 * 64, U);
  vst1q_s16(out + 2 * 64, V);
}

static void Get8x8Block_NEON(const uint8_t* data, int step, int16_t* out) {
  const int16x4_t kC1 = vld1_s16(kCoeff1);
  const int16x4_t kC2 = vld1_s16(kCoeff2);
  for (int y = 8; y > 0; --y) {
    int16x8_t r, g, b;
    RGB24PackedToPlanar(data, &r, &g, &b);
    ToYUV_8(r, g, b, kC1, kC2, out);
    out += 8;
    data += step;
  }
}

// Convert 16x16 RGB samples to YUV420
static inline void ToY_16x16(const int16x8_t r,
                             const int16x8_t g,
                             const int16x8_t b,
                             int16_t* const y_out,
                             int16x8_t* const R_acc,
                             int16x8_t* const G_acc,
                             int16x8_t* const B_acc,
                             const int16x4_t coeffs,
                             bool do_add) {
  int16x8_t Y;
  ConvertRGBToY(r, g, b, coeffs, &Y);
  vst1q_s16(y_out, Y);
  if (do_add) {
    *R_acc = vaddq_s16(*R_acc, r);
    *G_acc = vaddq_s16(*G_acc, g);
    *B_acc = vaddq_s16(*B_acc, b);
  } else {  // just store
    *R_acc = r;
    *G_acc = g;
    *B_acc = b;
  }
}

static inline void ToUV_8x8(const int16x8_t R,
                            const int16x8_t G,
                            const int16x8_t B,
                            const int16x4_t coeffs,
                            int16_t* const uv_out) {
  int16x8_t U, V;
  ConvertRGBToUVAccumulated(R, G, B, coeffs, &U, &V);
  vst1q_s16(uv_out + 0 * 64, U);
  vst1q_s16(uv_out + 1 * 64, V);
}

static void Condense16To8(const int16x8_t acc1, int16x8_t* const acc2) {
  const int32x4_t lo = vpaddlq_s16(acc1);
  const int32x4_t hi = vpaddlq_s16(*acc2);
  *acc2 = vcombine_s16(vqmovn_s32(lo), vqmovn_s32(hi));  // pack-saturate
}

// convert two 16x8 RGB blocks into two blocks of luma, and 2 blocks of U/V
static void Get16x8_NEON(const uint8_t* src1, int src_stride,
                         int16_t y[4 * 64], int16_t uv[2 * 64]) {
  const int16x4_t kC1 = vld1_s16(kCoeff1);
  const int16x4_t kC2 = vld1_s16(kCoeff2);
  for (int i = 4; i > 0; --i, src1 += 2 * src_stride) {
    int16x8_t r_acc1, r_acc2, g_acc1, g_acc2, b_acc1, b_acc2;
    int16x8_t r, g, b;
    const uint8_t* const src2 = src1 + src_stride;
    RGB24PackedToPlanar(src1 + 0 * 8, &r, &g, &b);
    ToY_16x16(r, g, b, y + 0 * 64 + 0, &r_acc1, &g_acc1, &b_acc1, kC1, false);
    RGB24PackedToPlanar(src1 + 3 * 8, &r, &g, &b);
    ToY_16x16(r, g, b, y + 1 * 64 + 0, &r_acc2, &g_acc2, &b_acc2, kC1, false);
    RGB24PackedToPlanar(src2 + 0 * 8, &r, &g, &b);
    ToY_16x16(r, g, b, y + 0 * 64 + 8, &r_acc1, &g_acc1, &b_acc1, kC1, true);
    RGB24PackedToPlanar(src2 + 3 * 8, &r, &g, &b);
    ToY_16x16(r, g, b, y + 1 * 64 + 8, &r_acc2, &g_acc2, &b_acc2, kC1, true);
    Condense16To8(r_acc1, &r_acc2);
    Condense16To8(g_acc1, &g_acc2);
    Condense16To8(b_acc1, &b_acc2);
    ToUV_8x8(r_acc2, g_acc2, b_acc2, kC2, uv);
    y += 2 * 8;
    uv += 8;
  }
}

static void Get16x16Block_NEON(const uint8_t* data, int step, int16_t* yuv) {
  int16_t* const uv = yuv + 4 * 64;
  Get16x8_NEON(data + 0 * step, step, yuv + 0 * 64, uv + 0 * 8);
  Get16x8_NEON(data + 8 * step, step, yuv + 2 * 64, uv + 4 * 8);
}

#undef MULT_S32_S16_LARGE
#undef DOT_PROD_PREAMBLE
#undef DOT_PROD1
#undef DOT_PROD2

#endif    // SJPEG_USE_NEON

///////////////////////////////////////////////////////////////////////////////
// C-version

// convert rgb_in[3] RGB triplet to Y, and accumulate in *rgb_sum
static inline int16_t ToY(const uint8_t* const rgb_in, int* const rgb_sum) {
  const int r = rgb_in[0];
  const int g = rgb_in[1];
  const int b = rgb_in[2];
  rgb_sum[0] += r;
  rgb_sum[1] += g;
  rgb_sum[2] += b;
  const int y = 19595 * r + 38469 * g + 7471 * b + ROUND_Y;
  return static_cast<int16_t>(y >> FRAC);
}

// convert sum of four rgb triplets to U
static inline int16_t ToU(const int* const rgb) {
  const int u = -11059 * rgb[0] - 21709 * rgb[1] + 32768 * rgb[2] + ROUND_UV;
  return static_cast<int16_t>(u >> (FRAC + 2));
}

// convert sum of four rgb triplets to V
static inline int16_t ToV(const int* const rgb) {
  const int v = 32768 * rgb[0] - 27439 * rgb[1] -  5329 * rgb[2] + ROUND_UV;
  return static_cast<int16_t>(v >> (FRAC + 2));
}

// for 4:4:4 conversion: convert rgb[3] to yuv
static inline void ToYUV(const uint8_t* const rgb, int16_t* const out) {
  const int r = rgb[0];
  const int g = rgb[1];
  const int b = rgb[2];
  const int y =  19595 * r + 38469 * g +  7471 * b + ROUND_Y;
  const int u = -11059 * r - 21709 * g + 32768 * b + HALF;
  const int v =  32768 * r - 27439 * g -  5329 * b + HALF;
  out[0 * 64] = static_cast<int16_t>(y >> FRAC);
  out[1 * 64] = static_cast<int16_t>(u >> FRAC);
  out[2 * 64] = static_cast<int16_t>(v >> FRAC);
}

static void Get8x8Block_C(const uint8_t* data, int step, int16_t* out) {
  for (int y = 8; y > 0; --y) {
    for (int x = 0; x < 8; ++x) {
      ToYUV(data + 3 * x, out + x);
    }
    out += 8;
    data += step;
  }
}

void Get16x8Block_C(const uint8_t* src1, int src_stride,
                    int16_t yblock[4 * 64], int16_t uvblock[2 * 64]) {
  for (int y = 8; y > 0; y -= 2) {
    const uint8_t* const src2 = src1 + src_stride;
    for (int x = 0; x < 4; ++x) {
      int rgb[2][3];
      memset(rgb, 0, sizeof(rgb));
      yblock[2 * x    ] = ToY(src1 + 6 * x,     rgb[0]);
      yblock[2 * x + 1] = ToY(src1 + 6 * x + 3, rgb[0]);
      yblock[2 * x + 8] = ToY(src2 + 6 * x,     rgb[0]);
      yblock[2 * x + 9] = ToY(src2 + 6 * x + 3, rgb[0]);
      uvblock[0 * 64 + x] = ToU(rgb[0]);
      uvblock[1 * 64 + x] = ToV(rgb[0]);
      yblock[2 * x     + 64] = ToY(src1 + 3 * 8 + 6 * x,     rgb[1]);
      yblock[2 * x + 1 + 64] = ToY(src1 + 3 * 8 + 6 * x + 3, rgb[1]);
      yblock[2 * x + 8 + 64] = ToY(src2 + 3 * 8 + 6 * x,     rgb[1]);
      yblock[2 * x + 9 + 64] = ToY(src2 + 3 * 8 + 6 * x + 3, rgb[1]);
      uvblock[0 * 64 + x + 4] = ToU(rgb[1]);
      uvblock[1 * 64 + x + 4] = ToV(rgb[1]);
    }
    yblock += 2 * 8;
    uvblock += 8;
    src1 += 2 * src_stride;
  }
}

static void Get16x16Block_C(const uint8_t* rgb, int step, int16_t* yuv) {
  Get16x8Block_C(rgb + 0 * step, step, yuv + 0 * 64, yuv + 4 * 64 + 0 * 8);
  Get16x8Block_C(rgb + 8 * step, step, yuv + 2 * 64, yuv + 4 * 64 + 4 * 8);
}

///////////////////////////////////////////////////////////////////////////////

RGBToYUVBlockFunc GetBlockFunc(bool use_444) {
#if defined(SJPEG_USE_SSE2)
  if (SupportsSSE2()) return use_444 ? Get8x8Block_SSE2
                                     : Get16x16Block_SSE2;
#elif defined(SJPEG_USE_NEON)
  if (SupportsNEON()) return use_444 ? Get8x8Block_NEON
                                     : Get16x16Block_NEON;
#endif
  return use_444 ? Get8x8Block_C : Get16x16Block_C;  // default
}

///////////////////////////////////////////////////////////////////////////////

namespace {

uint8_t clip_8b(int v) {
  return (!(v & ~0xff)) ? (uint8_t)v : (v < 0) ? 0u : 255u;
}

int ToY(int r, int g, int b) {
  const int luma = 19595 * r + 38469 * g + 7471 * b;
  return (luma + HALF) >> FRAC;  // no need to clip
}

uint32_t clip_uv(int v) {
  return clip_8b(128 + ((v + HALF) >> FRAC));
}

uint32_t ToU(int r, int g, int b) {
  const int u = -11059 * r - 21709 * g + 32768 * b;
  return clip_uv(u);
}

uint32_t ToV(int r, int g, int b) {
  const int v = 32768 * r - 27439 * g - 5329 * b;
  return clip_uv(v);
}

// (X * 0x0101 >> 16) ~= X / 255
uint32_t Convert(uint32_t v) {
  return (v * (0x0101u * (sjpeg::kRGBSize - 1))) >> 16;
}

int ConvertToYUVIndex(const uint8_t* const rgb) {
  const int r = rgb[0];
  const int g = rgb[1];
  const int b = rgb[2];
  const uint32_t y = Convert(ToY(r, g, b));
  const uint32_t u = Convert(ToU(r, g, b));
  const uint32_t v = Convert(ToV(r, g, b));
  return (y + u * sjpeg::kRGBSize + v * sjpeg::kRGBSize * sjpeg::kRGBSize);
}

void RowToIndexC(const uint8_t* rgb, int width, uint16_t* dst) {
  for (int i = 0; i < width; ++i, rgb += 3) {
    dst[i] = ConvertToYUVIndex(rgb);
  }
}

#if defined(SJPEG_USE_SSE2)
void RowToIndexSSE2(const uint8_t* rgb, int width, uint16_t* dst) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i mult = _mm_set1_epi16(0x0101u * (sjpeg::kRGBSize - 1));
  const __m128i mult1 = _mm_set1_epi16(sjpeg::kRGBSize);
  const __m128i mult2 = _mm_set1_epi16(sjpeg::kRGBSize * sjpeg::kRGBSize);
  const __m128i k255 = _mm_set1_epi16(255);
  while (width >= 8) {
    __m128i r, g, b;
    __m128i Y, U, V;
    RGB24PackedToPlanar(rgb, &r, &g, &b);
    ConvertRGBToY(&r, &g, &b, 0, &Y);
    ConvertRGBToUV(&r, &g, &b, 128, &U, &V);
    // clamping to [0, 255]
    const __m128i y1 = _mm_min_epi16(_mm_max_epi16(Y, zero), k255);
    const __m128i u1 = _mm_min_epi16(_mm_max_epi16(U, zero), k255);
    const __m128i v1 = _mm_min_epi16(_mm_max_epi16(V, zero), k255);
    // convert to idx
    const __m128i y2 = _mm_mulhi_epi16(y1, mult);
    const __m128i u2 = _mm_mulhi_epi16(u1, mult);
    const __m128i v2 = _mm_mulhi_epi16(v1, mult);
    // store final idx
    const __m128i u3 = _mm_mullo_epi16(u2, mult1);
    const __m128i v3 = _mm_mullo_epi16(v2, mult2);
    const __m128i tmp = _mm_add_epi16(y2, u3);
    const __m128i idx = _mm_add_epi16(tmp, v3);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), idx);

    rgb += 3 * 8;
    dst += 8;
    width -= 8;
  }
  if (width > 0) RowToIndexC(rgb, width, dst);
}
#elif defined(SJPEG_USE_NEON)
void RowToIndexNEON(const uint8_t* rgb, int width, uint16_t* dst) {
  const int16x8_t k128 = vdupq_n_s16(128);
  const uint8x8_t mult = vdup_n_u8(sjpeg::kRGBSize - 1);
  const uint16x8_t mult1 = vdupq_n_u16(sjpeg::kRGBSize);
  const uint16x8_t mult2 = vdupq_n_u16(sjpeg::kRGBSize * sjpeg::kRGBSize);
  const int16x4_t coeffs1 = vld1_s16(kCoeff1);
  const int16x4_t coeffs2 = vld1_s16(kCoeff2);
  while (width >= 8) {
    int16x8_t r, g, b;
    int16x8_t Y, U, V;
    RGB24PackedToPlanar(rgb, &r, &g, &b);
    ConvertRGBToY(r, g, b, coeffs1, &Y);
    ConvertRGBToUV(r, g, b, coeffs2, &U, &V);
    // clamping to [0, 255]
    const uint8x8_t y1 = vqmovun_s16(vaddq_s16(Y, k128));
    const uint8x8_t u1 = vqmovun_s16(vaddq_s16(U, k128));
    const uint8x8_t v1 = vqmovun_s16(vaddq_s16(V, k128));
    // convert to idx
    const uint16x8_t y2 = vmull_u8(y1, mult);
    const uint16x8_t u2 = vmull_u8(u1, mult);
    const uint16x8_t v2 = vmull_u8(v1, mult);
    // divide by 255 using v/255 = (v * 0x0101) >> 16
    const uint16x8_t y3 = vshrq_n_u16(vsraq_n_u16(y2, y2, 8), 8);
    const uint16x8_t u3 = vshrq_n_u16(vsraq_n_u16(u2, u2, 8), 8);
    const uint16x8_t v3 = vshrq_n_u16(vsraq_n_u16(v2, v2, 8), 8);
    // store final idx
    const uint16x8_t tmp = vmlaq_u16(y3, u3, mult1);
    const uint16x8_t idx = vmlaq_u16(tmp, v3, mult2);
    vst1q_u16(dst, idx);

    rgb += 3 * 8;
    dst += 8;
    width -= 8;
  }
  if (width > 0) RowToIndexC(rgb, width, dst);
}
#endif    // SJPEG_USE_NEON

}  // namespace


RGBToIndexRowFunc GetRowFunc() {
#if defined(SJPEG_USE_SSE2)
  if (SupportsSSE2()) return RowToIndexSSE2;
#elif defined(SJPEG_USE_NEON)
  if (SupportsNEON()) return RowToIndexNEON;
#endif
  return RowToIndexC;
}

}   // namespace sjpeg

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
//  forward DCT
//
// fdct output is kept scaled by 16, to retain maximum 16bit precision
//
// Author: Skal (pascal.massimino@gmail.com)

#define SJPEG_NEED_ASM_HEADERS
#include "sjpegi.h"

namespace sjpeg {

///////////////////////////////////////////////////////////////////////////////
// Cosine table: C(k) = cos(k.pi/16)/sqrt(2), k = 1..7 using 15 bits signed
const int16_t kTable04[7] = { 22725, 21407, 19266, 16384, 12873,  8867, 4520 };
// rows #1 and #7 are pre-multiplied by 2.C(1) before the 2nd pass.
// This multiply is merged in the table of constants used during 1rst pass:
const int16_t kTable17[7] = { 31521, 29692, 26722, 22725, 17855, 12299, 6270 };
// rows #2 and #6 are pre-multiplied by 2.C(2):
const int16_t kTable26[7] = { 29692, 27969, 25172, 21407, 16819, 11585, 5906 };
// rows #3 and #5 are pre-multiplied by 2.C(3):
const int16_t kTable35[7] = { 26722, 25172, 22654, 19266, 15137, 10426, 5315 };

///////////////////////////////////////////////////////////////////////////////
// Constants and C/SSE2 macros for IDCT vertical pass

#define kTan1   (13036)   // = tan(pi/16)
#define kTan2   (27146)   // = tan(2.pi/16) = sqrt(2) - 1.
#define kTan3m1 (-21746)  // = tan(3.pi/16) - 1
#define k2Sqrt2 (23170)   // = 1 / 2.sqrt(2)

  // performs: {a,b} <- {a-b, a+b}, without saturation
#define BUTTERFLY(a, b) do {   \
  SUB((a), (b));               \
  ADD((b), (b));               \
  ADD((b), (a));               \
} while (0)

///////////////////////////////////////////////////////////////////////////////
// Constants for DCT horizontal pass

// Note about the CORRECT_LSB macro:
// using 16bit fixed-point constants, we often compute products like:
// p = (A*x + B*y + 32768) >> 16 by adding two sub-terms q = (A*x) >> 16
// and r = (B*y) >> 16 together. Statistically, we have p = q + r + 1
// in 3/4 of the cases. This can be easily seen from the relation:
//   (a + b + 1) >> 1 = (a >> 1) + (b >> 1) + ((a|b)&1)
// The approximation we are doing is replacing ((a|b)&1) by 1.
// In practice, this is a slightly more involved because the constants A and B
// have also been rounded compared to their exact floating point value.
// However, all in all the correction is quite small, and CORRECT_LSB can
// be defined empty if needed.

#define COLUMN_DCT8(in) do { \
  LOAD(m0, (in)[0 * 8]);     \
  LOAD(m2, (in)[2 * 8]);     \
  LOAD(m7, (in)[7 * 8]);     \
  LOAD(m5, (in)[5 * 8]);     \
                             \
  BUTTERFLY(m0, m7);         \
  BUTTERFLY(m2, m5);         \
                             \
  LOAD(m3, (in)[3 * 8]);     \
  LOAD(m4, (in)[4 * 8]);     \
  BUTTERFLY(m3, m4);         \
                             \
  LOAD(m6, (in)[6 * 8]);     \
  LOAD(m1, (in)[1 * 8]);     \
  BUTTERFLY(m1, m6);         \
  BUTTERFLY(m7, m4);         \
  BUTTERFLY(m6, m5);         \
                             \
  /* RowIdct() needs 15bits fixed-point input, when the output from   */ \
  /* ColumnIdct() would be 12bits. We are better doing the shift by 3 */ \
  /* now instead of in RowIdct(), because we have some multiplies to  */ \
  /* perform, that can take advantage of the extra 3bits precision.   */ \
  LSHIFT(m4, 3);             \
  LSHIFT(m5, 3);             \
  BUTTERFLY(m4, m5);         \
  STORE16((in)[0 * 8], m5);  \
  STORE16((in)[4 * 8], m4);  \
                             \
  LSHIFT(m7, 3);             \
  LSHIFT(m6, 3);             \
  LSHIFT(m3, 3);             \
  LSHIFT(m0, 3);             \
                             \
  LOAD_CST(m4, kTan2);       \
  m5 = m4;                   \
  MULT(m4, m7);              \
  MULT(m5, m6);              \
  SUB(m4, m6);               \
  ADD(m5, m7);               \
  STORE16((in)[2 * 8], m5);  \
  STORE16((in)[6 * 8], m4);  \
                             \
  /* We should be multiplying m6 by C4 = 1/sqrt(2) here, but we only have */ \
  /* the k2Sqrt2 = 1/(2.sqrt(2)) constant that fits into 15bits. So we    */ \
  /* shift by 4 instead of 3 to compensate for the additional 1/2 factor. */ \
  LOAD_CST(m6, k2Sqrt2);     \
  LSHIFT(m2, 3 + 1);         \
  LSHIFT(m1, 3 + 1);         \
  BUTTERFLY(m1, m2);         \
  MULT(m2, m6);              \
  MULT(m1, m6);              \
  BUTTERFLY(m3, m1);         \
  BUTTERFLY(m0, m2);         \
                             \
  LOAD_CST(m4, kTan3m1);     \
  LOAD_CST(m5, kTan1);       \
  m7 = m3;                   \
  m6 = m1;                   \
  MULT(m3, m4);              \
  MULT(m1, m5);              \
                             \
  ADD(m3, m7);               \
  ADD(m1, m2);               \
  CORRECT_LSB(m1);           \
  CORRECT_LSB(m3);           \
  MULT(m4, m0);              \
  MULT(m5, m2);              \
  ADD(m4, m0);               \
  SUB(m0, m3);               \
  ADD(m7, m4);               \
  SUB(m5, m6);               \
                             \
  STORE16((in)[1 * 8], m1);  \
  STORE16((in)[3 * 8], m0);  \
  STORE16((in)[5 * 8], m7);  \
  STORE16((in)[7 * 8], m5);  \
} while (0)

///////////////////////////////////////////////////////////////////////////////
// Plain-C implementation, bit-wise equivalent to the SSE2 version

// these are the macro required by COLUMN_*
#define LOAD_CST(dst, src) (dst) = (src)
#define LOAD(dst, src) (dst) = (src)
#define MULT(a, b)  (a) = (((a) * (b)) >> 16)
#define ADD(a, b)   (a) = (a) + (b)
#define SUB(a, b)   (a) = (a) - (b)
#define LSHIFT(a, n) (a) = ((a) << (n))
#define STORE16(a, b) (a) = (b)
#define CORRECT_LSB(a) (a) += 1

// DCT vertical pass

void ColumnDct(int16_t* in) {
  for (int i = 0; i < 8; ++i) {
    int32_t m0, m1, m2, m3, m4, m5, m6, m7;
    COLUMN_DCT8(in + i);
  }
}

// DCT horizontal pass

// We don't really need to round before descaling, since we
// still have 4 bits of precision left as final scaled output.
#define DESCALE(a)  static_cast<int16_t>((a) >> 16)

static void RowDct(int16_t* in, const int16_t*table) {
  // The Fourier transform is an unitary operator, so we're basically
  // doing the transpose of RowIdct()
  const int a0 = in[0] + in[7];
  const int b0 = in[0] - in[7];
  const int a1 = in[1] + in[6];
  const int b1 = in[1] - in[6];
  const int a2 = in[2] + in[5];
  const int b2 = in[2] - in[5];
  const int a3 = in[3] + in[4];
  const int b3 = in[3] - in[4];

  // even part
  const int C2 = table[1];
  const int C4 = table[3];
  const int C6 = table[5];
  const int c0 = a0 + a3;
  const int c1 = a0 - a3;
  const int c2 = a1 + a2;
  const int c3 = a1 - a2;

  in[0] = DESCALE(C4 * (c0 + c2));
  in[4] = DESCALE(C4 * (c0 - c2));
  in[2] = DESCALE(C2 * c1 + C6 * c3);
  in[6] = DESCALE(C6 * c1 - C2 * c3);

  // odd part
  const int C1 = table[0];
  const int C3 = table[2];
  const int C5 = table[4];
  const int C7 = table[6];
  in[1] = DESCALE(C1 * b0 + C3 * b1 + C5 * b2 + C7 * b3);
  in[3] = DESCALE(C3 * b0 - C7 * b1 - C1 * b2 - C5 * b3);
  in[5] = DESCALE(C5 * b0 - C1 * b1 + C7 * b2 + C3 * b3);
  in[7] = DESCALE(C7 * b0 - C5 * b1 + C3 * b2 - C1 * b3);
}
#undef DESCALE

#undef LOAD_CST
#undef LOAD
#undef MULT
#undef ADD
#undef SUB
#undef LSHIFT
#undef STORE16
#undef CORRECT_LSB

///////////////////////////////////////////////////////////////////////////////
// SSE2 implementation

#if defined(SJPEG_USE_SSE2)

// Tables and macros

#define CST(v) { { v, v, v, v, v, v, v, v } }
static const union {
  const int16_t s[8];
  const __m128i m;
} CST_kTan1 = CST(kTan1),
  CST_kTan2 = CST(kTan2),
  CST_kTan3m1 = CST(kTan3m1),
  CST_k2Sqrt2 = CST(k2Sqrt2),
  CST_kfRounder1 = CST(1);  // rounders for fdct
#undef CST

static const union {
  const uint16_t s[4 * 8];
  const __m128i m[4];
} kfTables_SSE2[4] = {
    // Tables for fdct, roughly the transposed of the above, shuffled
    { { 0x4000, 0x4000, 0x58c5, 0x4b42, 0xdd5d, 0xac61, 0xa73b, 0xcdb7,
        0x4000, 0x4000, 0x3249, 0x11a8, 0x539f, 0x22a3, 0x4b42, 0xee58,
        0x4000, 0xc000, 0x3249, 0xa73b, 0x539f, 0xdd5d, 0x4b42, 0xa73b,
        0xc000, 0x4000, 0x11a8, 0x4b42, 0x22a3, 0xac61, 0x11a8, 0xcdb7 } },
    { { 0x58c5, 0x58c5, 0x7b21, 0x6862, 0xcff5, 0x8c04, 0x84df, 0xba41,
        0x58c5, 0x58c5, 0x45bf, 0x187e, 0x73fc, 0x300b, 0x6862, 0xe782,
        0x58c5, 0xa73b, 0x45bf, 0x84df, 0x73fc, 0xcff5, 0x6862, 0x84df,
        0xa73b, 0x58c5, 0x187e, 0x6862, 0x300b, 0x8c04, 0x187e, 0xba41 } },
    { { 0x539f, 0x539f, 0x73fc, 0x6254, 0xd2bf, 0x92bf, 0x8c04, 0xbe4d,
        0x539f, 0x539f, 0x41b3, 0x1712, 0x6d41, 0x2d41, 0x6254, 0xe8ee,
        0x539f, 0xac61, 0x41b3, 0x8c04, 0x6d41, 0xd2bf, 0x6254, 0x8c04,
        0xac61, 0x539f, 0x1712, 0x6254, 0x2d41, 0x92bf, 0x1712, 0xbe4d } },
    { { 0x4b42, 0x4b42, 0x6862, 0x587e, 0xd746, 0x9dac, 0x979e, 0xc4df,
        0x4b42, 0x4b42, 0x3b21, 0x14c3, 0x6254, 0x28ba, 0x587e, 0xeb3d,
        0x4b42, 0xb4be, 0x3b21, 0x979e, 0x6254, 0xd746, 0x587e, 0x979e,
        0xb4be, 0x4b42, 0x14c3, 0x587e, 0x28ba, 0x9dac, 0x14c3, 0xc4df } } };

#define LOAD_CST(x, y)  (x) = (CST_ ## y).m
#define LOAD(x, y)      \
    (x) = _mm_load_si128(reinterpret_cast<const __m128i*>(&(y)))
#define MULT(x, y)      (x) = _mm_mulhi_epi16((x), (y))
#define ADD(x, y)       (x) = _mm_add_epi16((x), (y))
#define SUB(x, y)       (x) = _mm_sub_epi16((x), (y))
#define LSHIFT(x, n)    (x) = _mm_slli_epi16((x), (n))
#define STORE16(a, b) _mm_store_si128(reinterpret_cast<__m128i*>(&(a)), (b))
#define CORRECT_LSB(a) (a) = _mm_adds_epi16((a), CST_kfRounder1.m)

// DCT vertical pass

static void ColumnDct_SSE2(int16_t* in) {
  __m128i m0, m1, m2, m3, m4, m5, m6, m7;
  COLUMN_DCT8(in);
}

// DCT horizontal pass

static void RowDct_SSE2(int16_t* in, const __m128i* table1,
                        const __m128i* table2) {
  // load row [0123|4567] as [0123|7654]
  __m128i m0 =
      _mm_shufflehi_epi16(*reinterpret_cast<__m128i*>(in + 0 * 8), 0x1b);
  __m128i m2 =
      _mm_shufflehi_epi16(*reinterpret_cast<__m128i*>(in + 1 * 8), 0x1b);

  // we process two rows in parallel
  __m128i m4 = m0;
  // => x0 x1 x2 x3 | x0' x1' x2' x3'
  m0 = (__m128i)_mm_shuffle_ps((__m128)m0, (__m128)m2, 0x44);
  // => x7 x6 x5 x4 | x7' x6' x5' x4'
  m4 = (__m128i)_mm_shuffle_ps((__m128)m4, (__m128)m2, 0xee);

  // initial butterfly
  m2 = m0;
  m0 = _mm_add_epi16(m0, m4);  // a0=x0+x7 | a1=x1+x6 | a2=x2+x5 | a3=x3+x4
  m2 = _mm_sub_epi16(m2, m4);  // b0=x0-x7 | b1=x1-x6 | b2=x2-x5 | b3=x3-x4

  // prepare for scalar products which are performed using four madd_epi16
  __m128i m6;
  m4 = m0;
  m0 = _mm_unpacklo_epi32(m0, m2);   // a0 a1 | b0 b1 | a2 a3 | b2 b3
  m4 = _mm_unpackhi_epi32(m4, m2);
  m2 = _mm_shuffle_epi32(m0, 0x4e);  // a2 a3 | b2 b3 | a0 a1 | b0 b1
  m6 = _mm_shuffle_epi32(m4, 0x4e);

  __m128i m1, m3, m5, m7;
  m1 = _mm_madd_epi16(m2, table1[1]);
  m3 = _mm_madd_epi16(m0, table1[2]);
  m5 = _mm_madd_epi16(m6, table2[1]);
  m7 = _mm_madd_epi16(m4, table2[2]);

  m2 = _mm_madd_epi16(m2, table1[3]);
  m0 = _mm_madd_epi16(m0, table1[0]);
  m6 = _mm_madd_epi16(m6, table2[3]);
  m4 = _mm_madd_epi16(m4, table2[0]);

  // add the sub-terms
  m0 = _mm_add_epi32(m0, m1);
  m4 = _mm_add_epi32(m4, m5);
  m2 = _mm_add_epi32(m2, m3);
  m6 = _mm_add_epi32(m6, m7);

  // descale
  m0 = _mm_srai_epi32(m0, 16);
  m4 = _mm_srai_epi32(m4, 16);
  m2 = _mm_srai_epi32(m2, 16);
  m6 = _mm_srai_epi32(m6, 16);

  m0 = _mm_packs_epi32(m0, m2);
  m4 = _mm_packs_epi32(m4, m6);

  _mm_store_si128(reinterpret_cast<__m128i*>(in + 0 * 8), m0);
  _mm_store_si128(reinterpret_cast<__m128i*>(in + 1 * 8), m4);
}

#undef LOAD_CST
#undef LOAD
#undef MULT
#undef ADD
#undef SUB
#undef LSHIFT
#undef STORE16
#undef CORRECT_LSB

#endif    // SJPEG_USE_SSE2

// done with the macros

#undef BUTTERFLY
#undef COLUMN_DCT8

///////////////////////////////////////////////////////////////////////////////
// NEON implementation

#if defined(SJPEG_USE_NEON)

// multiply by scalar
#define MULT(A, kC) (vqdmulhq_n_s16((A), (kC) >> 1))
// V0 = r0 - r1, V1 = r0 + r1
#define BUTTERFLY(V0, V1, r0, r1)                  \
  const int16x8_t V0 = vsubq_s16((r0), (r1));      \
  const int16x8_t V1 = vaddq_s16((r0), (r1))

// collect the 16b hi-words of 32bit words into a packed 16b one
static int16x8_t PackS32(const int32x4_t lo, const int32x4_t hi) {
  return vuzpq_s16(vreinterpretq_s16_s32(lo),
                   vreinterpretq_s16_s32(hi)).val[1];
}

#define MULT_DCL_32(LO, HI, A, CST)                                \
  int32x4_t LO = vmull_s16(vget_low_s16(A), vget_low_s16(CST));    \
  int32x4_t HI = vmull_s16(vget_high_s16(A), vget_high_s16(CST))
#define MULT_ADD_32(LO, HI, A, CST) do {                           \
  LO = vmlal_s16(LO, vget_low_s16(A), vget_low_s16(CST));          \
  HI = vmlal_s16(HI, vget_high_s16(A), vget_high_s16(CST));        \
} while (0)
#define MULT_SUB_32(LO, HI, A, CST) do {                           \
  LO = vmlsl_s16(LO, vget_low_s16(A), vget_low_s16(CST));          \
  HI = vmlsl_s16(HI, vget_high_s16(A), vget_high_s16(CST));        \
} while (0)

#define MK_TABLE_CST(A, B, C, D) { (A), (B), (C), (D), (A), (D), (C), (B) }

// s64 transposing helper:
//   *out0 = lo(v0) | hi(v1)
//   *out1 = lo(v1) | hi(v0)
static void vtrn_s64(const int32x4_t v0, const int32x4_t v1,
                     int16x8_t* out0, int16x8_t* out1) {
  *out0 = vreinterpretq_s16_s64(
      vcombine_s64(vreinterpret_s64_s32(vget_low_s32(v0)),
                   vreinterpret_s64_s32(vget_low_s32(v1))));
  *out1 = vreinterpretq_s16_s64(
      vcombine_s64(vreinterpret_s64_s32(vget_high_s32(v0)),
                   vreinterpret_s64_s32(vget_high_s32(v1))));
}

void Transpose8x8(int16x8_t* const A0, int16x8_t* const A1,
                  int16x8_t* const A2, int16x8_t* const A3,
                  int16x8_t* const A4, int16x8_t* const A5,
                  int16x8_t* const A6, int16x8_t* const A7) {
  const int16x8x2_t row01 = vtrnq_s16(*A0, *A1);
  const int16x8x2_t row23 = vtrnq_s16(*A2, *A3);
  const int16x8x2_t row45 = vtrnq_s16(*A4, *A5);
  const int16x8x2_t row67 = vtrnq_s16(*A6, *A7);

  const int32x4x2_t row02 = vtrnq_s32(vreinterpretq_s32_s16(row01.val[0]),
                                      vreinterpretq_s32_s16(row23.val[0]));
  const int32x4x2_t row13 = vtrnq_s32(vreinterpretq_s32_s16(row01.val[1]),
                                      vreinterpretq_s32_s16(row23.val[1]));
  const int32x4x2_t row46 = vtrnq_s32(vreinterpretq_s32_s16(row45.val[0]),
                                      vreinterpretq_s32_s16(row67.val[0]));
  const int32x4x2_t row57 = vtrnq_s32(vreinterpretq_s32_s16(row45.val[1]),
                                      vreinterpretq_s32_s16(row67.val[1]));

  vtrn_s64(row02.val[0], row46.val[0], A0, A4);
  vtrn_s64(row02.val[1], row46.val[1], A2, A6);
  vtrn_s64(row13.val[0], row57.val[0], A1, A5);
  vtrn_s64(row13.val[1], row57.val[1], A3, A7);
}

static void Dct_NEON(int16_t* in) {
  ////////////////////
  // vertical pass
  ////////////////////
  const int16x8_t m0 = vld1q_s16(in + 0 * 8);
  const int16x8_t m1 = vld1q_s16(in + 1 * 8);
  const int16x8_t m2 = vld1q_s16(in + 2 * 8);
  const int16x8_t m3 = vld1q_s16(in + 3 * 8);
  const int16x8_t m4 = vld1q_s16(in + 4 * 8);
  const int16x8_t m5 = vld1q_s16(in + 5 * 8);
  const int16x8_t m6 = vld1q_s16(in + 6 * 8);
  const int16x8_t m7 = vld1q_s16(in + 7 * 8);

  BUTTERFLY(A0, A7, m0, m7);
  BUTTERFLY(A2, A5, m2, m5);
  BUTTERFLY(A3, A4, m3, m4);
  BUTTERFLY(A1, A6, m1, m6);

  BUTTERFLY(B7, B4, A7, A4);
  BUTTERFLY(B6, B5, A6, A5);

  // see comment in COLUMN_DCT8
  const int16x8_t C4 = vshlq_n_s16(B4, 3);
  const int16x8_t C5 = vshlq_n_s16(B5, 3);
  const int16x8_t C7 = vshlq_n_s16(B7, 3);
  const int16x8_t C6 = vshlq_n_s16(B6, 3);
  const int16x8_t C3 = vshlq_n_s16(A3, 3);
  const int16x8_t C0 = vshlq_n_s16(A0, 3);

  // BUTTERFLY(tmp4, tmp0, C4, C5)
  int16x8_t tmp0 = vaddq_s16(C4, C5);
  int16x8_t tmp4 = vsubq_s16(C4, C5);
  int16x8_t tmp6 = vsubq_s16(MULT(C7, kTan2), C6);
  int16x8_t tmp2 = vaddq_s16(MULT(C6, kTan2), C7);

  // see comment in COLUMN_DCT8
  const int16x8_t E2 = vshlq_n_s16(A2, 3 + 1);
  const int16x8_t E1 = vshlq_n_s16(A1, 3 + 1);
  BUTTERFLY(F1, F2, E1, E2);
  const int16x8_t G2 = MULT(F2, k2Sqrt2);
  const int16x8_t G1 = MULT(F1, k2Sqrt2);
  BUTTERFLY(H3, H1, C3, G1);
  BUTTERFLY(H0, H2, C0, G2);

  const int16x8_t G3 = vaddq_s16(MULT(H3, kTan3m1), H3);
  const int16x8_t G6 = vaddq_s16(MULT(H1, kTan1), H2);

  // CORRECT_LSB
  const int16x8_t kOne = vdupq_n_s16(1);
  const int16x8_t I3 = vaddq_s16(G3, kOne);
  const int16x8_t G4 = vaddq_s16(MULT(H0, kTan3m1), H0);

  int16x8_t tmp1 = vaddq_s16(G6, kOne);
  int16x8_t tmp3 = vsubq_s16(H0, I3);
  int16x8_t tmp5 = vaddq_s16(H3, G4);
  int16x8_t tmp7 = vsubq_s16(MULT(H2, kTan1), H1);

  Transpose8x8(&tmp0, &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &tmp7);

  ////////////////////
  // Horizontal pass
  ////////////////////
  BUTTERFLY(b0, a0, tmp0, tmp7);
  BUTTERFLY(b1, a1, tmp1, tmp6);
  BUTTERFLY(b2, a2, tmp2, tmp5);
  BUTTERFLY(b3, a3, tmp3, tmp4);

  BUTTERFLY(c1, c0, a0, a3);
  BUTTERFLY(c3, c2, a1, a2);

  const int16_t kTable0[] = MK_TABLE_CST(22725, 31521, 29692, 26722);
  const int16_t kTable1[] = MK_TABLE_CST(21407, 29692, 27969, 25172);
  const int16_t kTable2[] = MK_TABLE_CST(19266, 26722, 25172, 22654);
  const int16_t kTable3[] = MK_TABLE_CST(16384, 22725, 21407, 19266);
  const int16_t kTable4[] = MK_TABLE_CST(12873, 17855, 16819, 15137);
  const int16_t kTable5[] = MK_TABLE_CST(8867, 12299, 11585, 10426);
  const int16_t kTable6[] = MK_TABLE_CST(4520, 6270, 5906, 5315);

  // even part
  const int16x8_t kC2 = vld1q_s16(kTable1);
  const int16x8_t kC4 = vld1q_s16(kTable3);
  const int16x8_t kC6 = vld1q_s16(kTable5);

  MULT_DCL_32(out0_lo, out0_hi, c0, kC4);
  MULT_DCL_32(out4_lo, out4_hi, c0, kC4);
  MULT_ADD_32(out0_lo, out0_hi, c2, kC4);
  MULT_SUB_32(out4_lo, out4_hi, c2, kC4);
  MULT_DCL_32(out2_lo, out2_hi, c1, kC2);
  MULT_DCL_32(out6_lo, out6_hi, c1, kC6);
  MULT_ADD_32(out2_lo, out2_hi, c3, kC6);
  MULT_SUB_32(out6_lo, out6_hi, c3, kC2);

  int16x8_t out0 = PackS32(out0_lo, out0_hi);
  int16x8_t out4 = PackS32(out4_lo, out4_hi);
  int16x8_t out2 = PackS32(out2_lo, out2_hi);
  int16x8_t out6 = PackS32(out6_lo, out6_hi);

  // odd part
  const int16x8_t kC1 = vld1q_s16(kTable0);
  const int16x8_t kC3 = vld1q_s16(kTable2);
  const int16x8_t kC5 = vld1q_s16(kTable4);
  const int16x8_t kC7 = vld1q_s16(kTable6);

  MULT_DCL_32(out1_lo, out1_hi, b0, kC1);
  MULT_DCL_32(out3_lo, out3_hi, b0, kC3);
  MULT_DCL_32(out5_lo, out5_hi, b0, kC5);
  MULT_DCL_32(out7_lo, out7_hi, b0, kC7);

  MULT_ADD_32(out1_lo, out1_hi, b1, kC3);
  MULT_SUB_32(out3_lo, out3_hi, b1, kC7);
  MULT_SUB_32(out5_lo, out5_hi, b1, kC1);
  MULT_SUB_32(out7_lo, out7_hi, b1, kC5);

  MULT_ADD_32(out1_lo, out1_hi, b2, kC5);
  MULT_SUB_32(out3_lo, out3_hi, b2, kC1);
  MULT_ADD_32(out5_lo, out5_hi, b2, kC7);
  MULT_ADD_32(out7_lo, out7_hi, b2, kC3);

  MULT_ADD_32(out1_lo, out1_hi, b3, kC7);
  MULT_SUB_32(out3_lo, out3_hi, b3, kC5);
  MULT_ADD_32(out5_lo, out5_hi, b3, kC3);
  MULT_SUB_32(out7_lo, out7_hi, b3, kC1);

  int16x8_t out1 = PackS32(out1_lo, out1_hi);
  int16x8_t out3 = PackS32(out3_lo, out3_hi);
  int16x8_t out5 = PackS32(out5_lo, out5_hi);
  int16x8_t out7 = PackS32(out7_lo, out7_hi);

  // final transpose
  Transpose8x8(&out0, &out1, &out2, &out3, &out4, &out5, &out6, &out7);

  // and storage.
  vst1q_s16(&in[0 * 8], out0);
  vst1q_s16(&in[1 * 8], out1);
  vst1q_s16(&in[2 * 8], out2);
  vst1q_s16(&in[3 * 8], out3);
  vst1q_s16(&in[4 * 8], out4);
  vst1q_s16(&in[5 * 8], out5);
  vst1q_s16(&in[6 * 8], out6);
  vst1q_s16(&in[7 * 8], out7);
}

static void FdctNEON(int16_t* coeffs, int num_blocks) {
  while (num_blocks-- > 0) {
    Dct_NEON(coeffs);
    coeffs += 64;
  }
}
#undef MULT
#undef BUTTERFLY
#undef MULT_DCL_32
#undef MULT_ADD_32
#undef MULT_SUB_32
#undef MK_TABLE_CST

#endif    // SJPEG_USE_NEON

#undef kTan1
#undef kTan2
#undef kTan3m1
#undef k2Sqrt2

///////////////////////////////////////////////////////////////////////////////
// visible FDCT callable functions

static void FdctC(int16_t* coeffs, int num_blocks) {
  while (num_blocks-- > 0) {
    ColumnDct(coeffs);
    RowDct(coeffs + 0 * 8, kTable04);
    RowDct(coeffs + 1 * 8, kTable17);
    RowDct(coeffs + 2 * 8, kTable26);
    RowDct(coeffs + 3 * 8, kTable35);
    RowDct(coeffs + 4 * 8, kTable04);
    RowDct(coeffs + 5 * 8, kTable35);
    RowDct(coeffs + 6 * 8, kTable26);
    RowDct(coeffs + 7 * 8, kTable17);
    coeffs += 64;
  }
}

#if defined(SJPEG_USE_SSE2)
static void FdctSSE2(int16_t* coeffs, int num_blocks) {
  while (num_blocks-- > 0) {
    ColumnDct_SSE2(coeffs);
    RowDct_SSE2(coeffs + 0 * 8, kfTables_SSE2[0].m, kfTables_SSE2[1].m);
    RowDct_SSE2(coeffs + 2 * 8, kfTables_SSE2[2].m, kfTables_SSE2[3].m);
    RowDct_SSE2(coeffs + 4 * 8, kfTables_SSE2[0].m, kfTables_SSE2[3].m);
    RowDct_SSE2(coeffs + 6 * 8, kfTables_SSE2[2].m, kfTables_SSE2[1].m);
    coeffs += 64;
  }
}
#endif  // SJPEG_USE_SSE2

FdctFunc GetFdct() {
#if defined(SJPEG_USE_SSE2)
  if (SupportsSSE2()) return FdctSSE2;
#elif defined(SJPEG_USE_NEON)
  if (SupportsNEON()) return FdctNEON;
#endif
  return FdctC;  // default
}

///////////////////////////////////////////////////////////////////////////////

}     // namespace sjpeg

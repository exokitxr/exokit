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
//  Misc tools for quickly parsing JPEG data
//
// Author: Skal (pascal.massimino@gmail.com)

#include <string.h>   // for memset
#include <vector>

#include "sjpegi.h"

///////////////////////////////////////////////////////////////////////////////
// Dimensions (SOF)

namespace {
// This function will quickly locate the first appareance of an SOF marker in
// the passed JPEG buffer. It assumes the streams starts wih an SOI marker,
// like any valid JPEG should. Returned value points to the beginning of the
// marker and is guarantied to contain a least 8 bytes of valid data.
const uint8_t* GetSOFData(const uint8_t* src, int size) {
  if (src == NULL) return NULL;
  const uint8_t* const end = src + size - 8;   // 8 bytes of safety, for marker
  src += 2;   // skip M_SOI
  for (; src < end && *src != 0xff; ++src) { /* search first 0xff marker */ }
  while (src < end) {
    const uint32_t marker = static_cast<uint32_t>((src[0] << 8) | src[1]);
    if (marker == M_SOF0 || marker == M_SOF1) return src;
    const size_t s = 2 + ((src[2] << 8) | src[3]);
    src += s;
  }
  return NULL;   // No SOF marker found
}
}   // anonymous namespace

bool SjpegDimensions(const uint8_t* src0, size_t size,
                     int* width, int* height, int* is_yuv420) {
  if (width == NULL || height == NULL) return false;
  const uint8_t* src = GetSOFData(src0, size);
  const size_t left_over = size - (src - src0);
  if (src == NULL || left_over < 8 + 3 * 1) return false;
  if (height != NULL) *height = (src[5] << 8) | src[6];
  if (width != NULL) *width = (src[7] << 8) | src[8];
  if (is_yuv420 != NULL) {
    const size_t nb_comps = src[9];
    *is_yuv420 = (nb_comps == 3);
    if (left_over < 11 + 3 * nb_comps) return false;
    for (int c = 0; *is_yuv420 && c < 3; ++c) {
      const int expected_dim = (c == 0 ? 0x22 : 0x11);
      *is_yuv420 &= (src[11 + c * 3] == expected_dim);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Quantizer marker (DQT)

int SjpegFindQuantizer(const uint8_t* src, size_t size,
                       uint8_t quant[2][64]) {
  memset(quant[0], 0, sizeof(quant[0]));
  memset(quant[1], 0, sizeof(quant[1]));
  // minimal size for 64 coeffs and the markers (5 bytes)
  if (src == NULL || size < 69 || src[0] != 0xff || src[1] != 0xd8) {
    return 0;
  }
  const uint8_t* const end = src + size - 8;   // 8 bytes of safety, for marker
  src += 2;   // skip over the initial M_SOI
  for (; src < end && *src != 0xff; ++src) { /* search first 0xff marker */ }
  int nb_comp = 0;
  while (src < end) {
    const uint32_t marker = static_cast<uint32_t>((src[0] << 8) | src[1]);
    const int chunk_size = 2 + ((src[2] << 8) | src[3]);
    if (src + chunk_size > end) {
      break;
    }
    if (marker == M_SOS) {
      // we can stop searching at the first SOS marker encountered, to avoid
      // parsing the whole data
      break;
    } else if (marker == M_DQT) {
      // Jump over packets of 1 index + 64 coeffs
      int i = 4;
      while (i + 1 < chunk_size) {
        const int Pq = src[i] >> 4;
        const int Tq = src[i] & 0x0f;
        if (Pq > 1 || Tq > 3) return 0;    // invalid bitstream. See B.4.
        const int m_size = 64 * Pq + 65;
        if (i + m_size > chunk_size) return 0;
        if (Tq < 2) {
          for (int j = 0; j < 64; ++j) {
            int v;
            if (Pq == 0) {
              v = src[i + 1 + j];
            } else {
              // convert 16b->8b by clamping
              v = ((int)src[i + 1 + 2 * j + 0] << 8)
                      | src[i + 1 + 2 * j + 1];
              v = (v > 255) ? 255 : v;
            }
            quant[Tq][sjpeg::kZigzag[j]] = (v < 1) ? 1u : (uint8_t)v;
          }
        } else {
          // we don't store the pointer, but we record the component
        }
        nb_comp |= 1 << Tq;
        i += m_size;
      }
    }
    src += chunk_size;
  }
  return ((nb_comp & 1) != 0) + ((nb_comp & 2) != 0)
       + ((nb_comp & 4) != 0) + ((nb_comp & 8) != 0);
}

///////////////////////////////////////////////////////////////////////////////

void SjpegQuantMatrix(float quality, bool for_chroma, uint8_t matrix[64]) {
  const float q_factor = sjpeg::GetQFactor(quality) / 100.f;
  const uint8_t* const matrix0 = sjpeg::kDefaultMatrices[for_chroma];
  for (int i = 0; i < 64; ++i) {
    const int v = static_cast<int>(matrix0[i] * q_factor + .5f);
    matrix[i] = (v < 1) ? 1u : (v > 255) ? 255u : v;
  }
}

float SjpegEstimateQuality(const uint8_t matrix[64], bool for_chroma) {
  // There's a lot of way to speed up this search (dichotomy, Newton, ...)
  // but also a lot of way to fabricate a twisted input to fool it.
  // So we're better off trying all the 100 possibilities since it's not
  // a lot after all.
  int best_quality = 0;
  float best_score = 256 * 256 * 64 + 1;
  for (int quality = 0; quality <= 100; ++quality) {
    uint8_t m[64];
    SjpegQuantMatrix(quality, for_chroma, m);
    float score = 0;
    for (size_t i = 0; i < 64; ++i) {
      const float diff = m[i] - matrix[i];
      score += diff * diff;
      if (score > best_score) {
        break;
      }
    }
    if (score < best_score) {
      best_score = score;
      best_quality = quality;
    }
  }
  return best_quality;
}

////////////////////////////////////////////////////////////////////////////////
// Bluriness risk evaluation and YUV420 / sharp-YUV420 / YUV444 decision

static const int kNoiseLevel = 4;
static const double kThreshYU420 = 40.0;
static const double kThreshSharpYU420 = 70.0;

SjpegYUVMode SjpegRiskiness(const uint8_t* rgb,
                            int width, int height, int stride, float* risk) {
  const sjpeg::RGBToIndexRowFunc cvrt_func = sjpeg::GetRowFunc();

  std::vector<uint16_t> row1(width), row2(width);
  double total_score = 0;
  double count = 0;
  const int kRGB3 = sjpeg::kRGBSize * sjpeg::kRGBSize * sjpeg::kRGBSize;

  cvrt_func(rgb, width, &row2[0]);  // convert first row ahead
  for (int j = 1; j < height; ++j) {
    rgb += stride;
    std::swap(row1, row2);
    cvrt_func(rgb, width, &row2[0]);  // this is the row below
    for (int i = 0; i < width - 1; ++i) {
      const int idx0 = row1[i + 0];
      const int idx1 = row1[i + 1];
      const int idx2 = row2[i];
      const int score = sjpeg::kSharpnessScore[idx0 + kRGB3 * idx1]
                      + sjpeg::kSharpnessScore[idx0 + kRGB3 * idx2]
                      + sjpeg::kSharpnessScore[idx1 + kRGB3 * idx2];
      if (score > kNoiseLevel) {
        total_score += score;
        count += 1.0;
      }
    }
  }
  if (count > 0) total_score /= count;
  // number of pixels evaluated
  const double frac = 100. * count / (width * height);
  // if less than 1% of pixels were evaluated -> below noise level.
  if (frac < 1.) total_score = 0.;

  // recommendation (TODO(skal): tune thresholds)
  total_score = (total_score > 25.) ? 100. : total_score * 100. / 25.;
  if (risk != NULL) *risk = (float)total_score;

  const SjpegYUVMode recommendation =
      (total_score < kThreshYU420) ?      SJPEG_YUV_420 :
      (total_score < kThreshSharpYU420) ? SJPEG_YUV_SHARP :
                                          SJPEG_YUV_444;
  return recommendation;
}

namespace sjpeg {

// (X * 0x0101 >> 16) ~= X / 255
static uint32_t Convert(uint32_t v) {
  return (v * (0x0101u * (sjpeg::kRGBSize - 1))) >> 16;
}

// Convert 8b values y/u/v to index entry.
int YUVToRiskIdx(int16_t y, int16_t u, int16_t v) {
  const int idx = Convert(y + 128)
                + Convert(u + 128) * sjpeg::kRGBSize
                + Convert(v + 128) * sjpeg::kRGBSize * sjpeg::kRGBSize;
  return idx;
}

// return riskiness score on an 8x8 block. Input is YUV444 block
// of DCT coefficients (Y/U/V).
double DCTRiskinessScore(const int16_t yuv[3 * 8], int16_t scores[8 * 8]) {
  uint16_t idx[64];
  for (int k = 0; k < 64; ++k) {
    idx[k] = YUVToRiskIdx(yuv[k + 0 * 64], yuv[k + 1 * 64],  yuv[k + 2 * 64]);
  }
  const int kRGB3 = sjpeg::kRGBSize * sjpeg::kRGBSize * sjpeg::kRGBSize;
  double total_score = 0;
  double count = 0;
  for (size_t J = 0; J <= 7; ++J) {
    for (size_t I = 0; I <= 7; ++I) {
      const int k = I + J * 8;
      const int idx0 = idx[k + 0];
      const int idx1 = idx[k + (I < 7 ? 1 : -1)];
      const int idx2 = idx[k + (J < 7 ? 8 : -8)];
      int score = sjpeg::kSharpnessScore[idx0 + kRGB3 * idx1]
                + sjpeg::kSharpnessScore[idx0 + kRGB3 * idx2]
                + sjpeg::kSharpnessScore[idx1 + kRGB3 * idx2];
      if (score <= kNoiseLevel) {
        score = 0;
      } else {
        total_score += score;
        count += 1.0;
      }
      scores[I + J * 8] = static_cast<int16_t>(score);
    }
  }
  if (count > 0) total_score /= count;
  total_score = (total_score > 25.) ? 100. : total_score * 100. / 25.;
  return total_score;
}

// This function returns the raw per-pixel riskiness scores. The input rgb[]
// samples is a 8x8 block, the output is a 8x8 block.
// Not an official API, because a little too specific. But still accessible.
double BlockRiskinessScore(const uint8_t* rgb, int stride,
                           int16_t scores[8 * 8]) {
  const RGBToYUVBlockFunc get_block = GetBlockFunc(true);
  int16_t yuv444[3 * 64];
  get_block(rgb, stride, yuv444);
  return DCTRiskinessScore(yuv444, scores);
}

}   // namespace sjpeg

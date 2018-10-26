// Copyright 2018 Google Inc.
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
//  Dichotomy loop and default search implementation
//
// Author: Skal (pascal.massimino@gmail.com)

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sjpegi.h"

using namespace sjpeg;

////////////////////////////////////////////////////////////////////////////////
// dichotomy

#define DBG_PRINT 0

// convergence is considered reached if |dq| < kdQLimit.
static const float kdQLimit = 0.15;

static float Clamp(float v, float min, float max) {
  return (v < min) ? min : (v > max) ? max : v;
}

bool SearchHook::Setup(const EncoderParam& param) {
  for_size = (param.target_mode == EncoderParam::TARGET_SIZE);
  target = param.target_value;
  tolerance = param.tolerance / 100.;
  qmin = (param.qmin < 0) ? 0 : param.qmin;
  qmax = (param.qmax > 100) ? 100 :
         (param.qmax < param.qmin) ? param.qmin : param.qmax;
  q = Clamp(SjpegEstimateQuality(param.GetQuantMatrix(0), false), qmin, qmax);
  value = 0;   // undefined for at this point
  pass = 0;
  return true;
}

bool SearchHook::Update(float result) {
  value = result;
  bool done = (fabs(value - target) < tolerance * target);
  if (done) return true;

  if (value > target) {
    qmax = q;
  } else {
    qmin = q;
  }
  const float last_q = q;
  q = (qmin + qmax) / 2.;
  done = (fabs(q - last_q) < kdQLimit);
  if (DBG_PRINT) {
    printf(" -> next-q=%.2f\n", last_q);
  }
  return done;
}

void SearchHook::NextMatrix(int idx, uint8_t dst[64]) {
  SetQuantMatrix(kDefaultMatrices[idx], GetQFactor(q), dst);
}

////////////////////////////////////////////////////////////////////////////////
// Helpers for the search

void Encoder::StoreRunLevels(DCTCoeffs* coeffs) {
  assert(use_extra_memory_);
  assert(reuse_run_levels_);

  const QuantizeBlockFunc quantize_block = use_trellis_ ? TrellisQuantizeBlock
                                                        : quantize_block_;
  if (use_trellis_) InitCodes(true);

  ResetDCs();
  nb_run_levels_ = 0;
  int16_t* in = in_blocks_;
  for (int n = 0; n < mb_w_ * mb_h_; ++n) {
    CheckBuffers();
    for (int c = 0; c < nb_comps_; ++c) {
      for (int i = 0; i < nb_blocks_[c]; ++i) {
        RunLevel* const run_levels = all_run_levels_ + nb_run_levels_;
        const int dc = quantize_block(in, c, &quants_[quant_idx_[c]],
                                      coeffs, run_levels);
        coeffs->dc_code_ = GenerateDCDiffCode(dc, &DCs_[c]);
        nb_run_levels_ += coeffs->nb_coeffs_;
        ++coeffs;
        in += 64;
      }
    }
  }
}

void Encoder::LoopScan() {
  assert(use_extra_memory_);
  assert(reuse_run_levels_);

  if (use_adaptive_quant_) {
    CollectHistograms();
  } else {
    CollectCoeffs();   // we just need the coeffs
  }

  const size_t nb_mbs = mb_w_ * mb_h_ * mcu_blocks_;
  DCTCoeffs* const base_coeffs = Alloc<DCTCoeffs>(nb_mbs);
  if (base_coeffs == nullptr) return;

  uint8_t opt_quants[2][64];

  // Dichotomy passes
  float best = 0.;     // best distance
  float best_q = 0.;  // informative value to return to the user
  float best_result = 0.;
  for (int p = 0; p < passes_; ++p) {
    search_hook_->pass = p;
    // set new matrices to evaluate
    for (int c = 0; c < 2; ++c) {
      search_hook_->NextMatrix(c, quants_[c].quant_);
      FinalizeQuantMatrix(&quants_[c], q_bias_);
    }
    if (use_adaptive_quant_) {
      AnalyseHisto();   // adjust quant_[] matrices
    }

    float result;
    if (search_hook_->for_size) {
      // compute pass to store coeffs / runs / dc_code_
      StoreRunLevels(base_coeffs);
      if (optimize_size_) {
        StoreOptimalHuffmanTables(nb_mbs, base_coeffs);
        if (use_trellis_) InitCodes(true);
      }
      result = ComputeSize(base_coeffs);
    } else {
      // if we're just targeting PSNR, we don't need to compute the
      // run/levels within the loop. We just need to quantize the coeffs
      // and measure the distortion.
      result = ComputePSNR();
    }
    if (DBG_PRINT) printf("pass #%d: q=%.2f value:%.2f ",
                          search_hook_->pass, search_hook_->q, result);

    if (p == 0 || fabs(result - search_hook_->target) < best) {
      // save the matrices for later, if they are better
      for (int c = 0; c < 2; ++c) {
        CopyQuantMatrix(quants_[c].quant_, opt_quants[c]);
      }
      best = fabs(result - search_hook_->target);
      best_q = search_hook_->q;
      best_result = result;
    }
    if (search_hook_->Update(result)) break;
  }
  // transfer back the final matrices
  SetQuantMatrices(opt_quants);
  for (int c = 0; c < 2; ++c) FinalizeQuantMatrix(&quants_[c], q_bias_);

  // return informative values to the user
  search_hook_->q = best_q;
  search_hook_->value = best_result;

  // optimize Huffman table now, if we haven't already during the search
  if (!search_hook_->for_size) {
    StoreRunLevels(base_coeffs);
    if (optimize_size_) {
      StoreOptimalHuffmanTables(nb_mbs, base_coeffs);
    }
  }

  // finish bitstream
  WriteDQT();
  WriteSOF();
  WriteDHT();
  WriteSOS();
  FinalPassScan(nb_mbs, base_coeffs);

  Free(base_coeffs);
}

////////////////////////////////////////////////////////////////////////////////
// Size & PSNR computation, mostly for dichotomy

size_t Encoder::HeaderSize() const {
  size_t size = 0;
  size += 20;    // APP0
  size += app_markers_.size();
  if (exif_.size() > 0) {
    size += 8 + exif_.size();
  }
  if (iccp_.size() > 0) {
    const size_t chunk_size_max = 0xffff - 12 - 4;
    const size_t num_chunks = (iccp_.size() - 1) / chunk_size_max + 1;
    size += num_chunks * (12 + 4 + 2);
    size += iccp_.size();
  }
  if (xmp_.size() > 0) {
    size += 2 + 2 + 29 + xmp_.size();
  }
  size += 2 * 65 + 2 + 2;         // DQT
  size += 8 + 3 * nb_comps_ + 2;  // SOF
  size += 6 + 2 * nb_comps_ + 2;  // SOS
  size += 2;                      // EOI
  // DHT:
  for (int c = 0; c < (nb_comps_ == 1 ? 1 : 2); ++c) {   // luma, chroma
    for (int type = 0; type <= 1; ++type) {               // dc, ac
      const HuffmanTable* const h = Huffman_tables_[type * 2 + c];
      size += 2 + 3 + 16 + h->nb_syms_;
    }
  }
  return size * 8;
}

void Encoder::BlocksSize(int nb_mbs, const DCTCoeffs* coeffs,
                         const RunLevel* rl,
                         BitCounter* const bc) const {
  for (int n = 0; n < nb_mbs; ++n) {
    const DCTCoeffs& c = coeffs[n];
    const int idx = c.idx_;
    const int q_idx = quant_idx_[idx];

    // DC
    const int dc_len = c.dc_code_ & 0x0f;
    const uint32_t code = dc_codes_[q_idx][dc_len];
    bc->AddPackedCode(code);
    if (dc_len) bc->AddBits(c.dc_code_ >> 4, dc_len);

    // AC
    const uint32_t* const codes = ac_codes_[q_idx];
    for (int i = 0; i < c.nb_coeffs_; ++i) {
      int run = rl[i].run_;
      while (run & ~15) {        // escapes
        bc->AddPackedCode(codes[0xf0]);
        run -= 16;
      }
      const uint32_t suffix = rl[i].level_;
      const size_t nbits = suffix & 0x0f;
      const int sym = (run << 4) | nbits;
      bc->AddPackedCode(codes[sym]);
      bc->AddBits(suffix >> 4, nbits);
    }
    if (c.last_ < 63) bc->AddPackedCode(codes[0x00]);  // EOB
    rl += c.nb_coeffs_;
  }
}

float Encoder::ComputeSize(const DCTCoeffs* coeffs) {
  InitCodes(false);
  size_t size = HeaderSize();
  BitCounter bc;
  BlocksSize(mb_w_ * mb_h_ * mcu_blocks_, coeffs, all_run_levels_, &bc);
  size += bc.Size();
  return size / 8.f;
}

////////////////////////////////////////////////////////////////////////////////

static float GetPSNR(uint64_t err, uint64_t size) {
  // This expression is written such that it gives the same result on ARM
  // and x86 (for large values of err/size in particular). Don't change it!
  return (err > 0 && size > 0) ? 4.3429448f * log(size / (err / 255. / 255.))
                               : 99.f;
}

float Encoder::ComputePSNR() const {
  uint64_t error = 0;
  const int16_t* in = in_blocks_;
  const size_t nb_mbs = mb_w_ * mb_h_;
  for (size_t n = 0; n < nb_mbs; ++n) {
    for (int c = 0; c < nb_comps_; ++c) {
      const Quantizer* const Q = &quants_[quant_idx_[c]];
      for (int i = 0; i < nb_blocks_[c]; ++i) {
        error += quantize_error_(in, Q);
        in += 64;
      }
    }
  }
  return GetPSNR(error, 64ull * nb_mbs * mcu_blocks_);
}

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
//  Fast & simple JPEG encoder. Internal header.
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef SJPEG_JPEGI_H_
#define SJPEG_JPEGI_H_

#include "sjpeg.h"
#include "bit_writer.h"

#ifndef NULL
#define NULL 0
#endif

#if defined(__SSE2__)
#define SJPEG_USE_SSE2
#endif

#if defined(__ARM_NEON__) || defined(__aarch64__)
#define SJPEG_USE_NEON
#endif

#if defined(SJPEG_NEED_ASM_HEADERS)
#if defined(SJPEG_USE_SSE2)
#include <emmintrin.h>
#endif

#if defined(SJPEG_USE_NEON)
#include <arm_neon.h>
#endif
#endif    // SJPEG_NEED_ASM_HEADERS

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////

namespace sjpeg {

extern bool SupportsSSE2();
extern bool SupportsNEON();

// Constants below are marker codes defined in JPEG spec
// ISO/IEC 10918-1 : 1993(E) Table B.1
// See also: http://www.w3.org/Graphics/JPEG/itu-t81.pdf

#define M_SOF0  0xffc0
#define M_SOF1  0xffc1
#define M_DHT   0xffc4
#define M_SOI   0xffd8
#define M_EOI   0xffd9
#define M_SOS   0xffda
#define M_DQT   0xffdb

// Forward 8x8 Fourier transforms, in-place.
typedef void (*FdctFunc)(int16_t *coeffs, int num_blocks);
FdctFunc GetFdct();

// these are the default luma/chroma matrices (JPEG spec section K.1)
extern const uint8_t kDefaultMatrices[2][64];
extern const uint8_t kZigzag[64];

// scoring tables in score_7.cc
extern const int kRGBSize;
extern const uint8_t kSharpnessScore[];

// internal riskiness scoring functions:
extern double DCTRiskinessScore(const int16_t yuv[3 * 8],
                                int16_t scores[8 * 8]);
extern double BlockRiskinessScore(const uint8_t* rgb, int stride,
                                  int16_t scores[8 * 8]);
extern int YUVToRiskIdx(int16_t y, int16_t u, int16_t v);

///////////////////////////////////////////////////////////////////////////////
// RGB->YUV conversion

// convert 16x16 RGB block into YUV420, or 8x8 RGB block into YUV444
typedef void (*RGBToYUVBlockFunc)(const uint8_t* src, int src_stride,
                                  int16_t* blocks);
extern RGBToYUVBlockFunc GetBlockFunc(bool use_444);

// convert a row of RGB samples to YUV444
typedef void (*RGBToIndexRowFunc)(const uint8_t* src, int width,
                                  uint16_t* dst);
extern RGBToIndexRowFunc GetRowFunc();

// Enhanced slower RGB->YUV conversion:
//  y_plane[] has dimension W x H, whereas u_plane[] and v_plane[] have
//  dimension (W + 1)/2 x (H + 1)/2.
void ApplySharpYUVConversion(const uint8_t* const rgb,
                             int W, int H, int stride,
                             uint8_t* y_plane,
                             uint8_t* u_plane, uint8_t* v_plane);

///////////////////////////////////////////////////////////////////////////////
// some useful helper functions around quant matrices

extern float GetQFactor(float q);   // convert quality factor -> scale factor
extern void CopyQuantMatrix(const uint8_t in[64], uint8_t out[64]);
extern void SetQuantMatrix(const uint8_t in[64], float q_factor,
                           uint8_t out[64]);
extern void SetMinQuantMatrix(const uint8_t* const m, uint8_t out[64],
                              int tolerance);

////////////////////////////////////////////////////////////////////////////////
// main structs

// Huffman tables
struct HuffmanTable {
  uint8_t bits_[16];     // number of symbols per bit count
  const uint8_t* syms_;  // symbol map, in increasing bit length
  uint8_t nb_syms_;      // cached value of sum(bits_[])
};

// quantizer matrices
struct Quantizer {
  uint8_t quant_[64];      // direct quantizer matrix
  uint8_t min_quant_[64];  // min quantizer value allowed
  uint16_t iquant_[64];    // precalc'd reciprocal for divisor
  uint16_t qthresh_[64];   // minimal absolute value that produce non-zero coeff
  uint16_t bias_[64];      // bias, for coring
  const uint32_t* codes_;  // codes for bit-cost calculation
};

// compact Run/Level storage, separate from DCTCoeffs infos
// Run/Level Information is not yet entropy-coded, but just stored
struct RunLevel {
  int16_t run_;
  uint16_t level_;     // 4bits for length, 12bits for mantissa
};

// short infos about the block of quantized coefficients
struct DCTCoeffs {
  int16_t last_;       // last position (inclusive) of non-zero coeff
  int16_t nb_coeffs_;  // total number of non-zero AC coeffs
  uint16_t dc_code_;   // DC code (4bits for length, 12bits for suffix)
  int8_t idx_;         // component idx
  int8_t bias_;        // perceptual bias
};

// Histogram of transform coefficients, for adaptive quant matrices
// * HSHIFT controls the trade-off between storage size for counts[]
//   and precision: the fdct doesn't descale and returns coefficients as
//   signed 16bit value. We are only interested in the absolute values
//   of coefficients that are less than MAX_HISTO_DCT_COEFF, which are our
//   best contributors.
//   Still, storing histogram up to MAX_HISTO_DCT_COEFF can be costly, so
//   we further aggregate the statistics in bins of size 1 << HSHIFT to save
//   space.
// * HLAMBDA roughly measures how much you are willing to trade in distortion
//   for a 1-bit gain in filesize.
// * QDELTA_MIN / QDELTA_MAX control how much we allow wandering around the
//   initial point. This helps reducing the CPU cost, as long as keeping the
//   optimization around the initial desired quality-factor (HLAMBDA also
//   serve this purpose).
enum { HSHIFT = 2,                       // size of bins is (1 << HSHIFT)
       HHALF = 1 << (HSHIFT - 1),
       MAX_HISTO_DCT_COEFF = (1 << 7),   // max coefficient, descaled by HSHIFT
       HLAMBDA = 0x80,
       // Limits on range of alternate quantizers explored around
       // the initial value.  (see details in AnalyseHisto())
       QDELTA_MIN = -12, QDELTA_MAX = 12,
       QSIZE = QDELTA_MAX + 1 - QDELTA_MIN,
};

struct Histo {
  // Reserve one extra entry for counting all coeffs greater than
  // MAX_HISTO_DCT_COEFF. Result isn't used, but it makes the loop easier.
  int counts_[64][MAX_HISTO_DCT_COEFF + 1];
};

////////////////////////////////////////////////////////////////////////////////

struct Encoder {
 public:
  Encoder(int W, int H, int step, const uint8_t* rgb, ByteSink* sink);
  virtual ~Encoder();
  bool Ok() const { return ok_; }

  // setters
  void SetQuality(float q);
  void SetCompressionMethod(int method);

  // all-in-one init from EncoderParam.
  bool InitFromParam(const EncoderParam& param);

  // Main call. Return false in case of parameter error (setting empty output).
  bool Encode();

  // these are colorspace-dependant.
  virtual void InitComponents() = 0;
  // return MCU samples at macroblock position (mb_x, mb_y)
  // clipped is true if the MCU is clipped and needs replication
  virtual void GetSamples(int mb_x, int mb_y, bool clipped,
                          int16_t* out_blocks) = 0;

 private:
  // setters
  void SetQuantMatrices(const uint8_t m[2][64]);
  void SetMinQuantMatrices(const uint8_t m[2][64], int tolerance);
  void SetDefaultMinQuantMatrices();

  void SetQuantizationBias(int bias, bool use_adaptive);
  void SetQuantizationDeltas(int qdelta_luma, int qdelta_chroma);

  typedef enum { ICC, EXIF, XMP, MARKERS } MetadataType;
  void SetMetadata(const std::string& data, MetadataType type);

 private:
  bool CheckBuffers();  // returns false in case of memory alloc error

  void WriteAPP0();
  bool WriteAPPMarkers(const std::string& data);
  bool WriteEXIF(const std::string& data);
  bool WriteICCP(const std::string& data);
  bool WriteXMP(const std::string& data);
  void WriteDQT();
  void WriteSOF();
  void WriteDHT();
  void WriteSOS();
  void WriteEOI();

  void ResetDCs();

  // collect transformed coeffs (unquantized) only
  void CollectCoeffs();

  // 2-pass Huffman optimizing scan
  void ResetEntropyStats();
  void AddEntropyStats(const DCTCoeffs* const coeffs,
                       const RunLevel* const run_levels);
  void CompileEntropyStats();
  void StoreOptimalHuffmanTables(size_t nb_mbs, const DCTCoeffs* coeffs);

  void SinglePassScan();           // finalizing scan
  void SinglePassScanOptimized();  // optimize the Huffman table + finalize scan

  // quantize and compute run/levels from already stored coeffs
  void StoreRunLevels(DCTCoeffs* coeffs);
  // just write already stored run_levels & coeffs:
  void FinalPassScan(size_t nb_mbs, const DCTCoeffs* coeffs);

  // dichotomy loop
  void LoopScan();

  // Histogram pass
  void CollectHistograms();

  void BuildHuffmanCodes(const HuffmanTable* const tab,
                         uint32_t* const codes);

  typedef int (*QuantizeBlockFunc)(const int16_t in[64], int idx,
                                   const Quantizer* const Q,
                                   DCTCoeffs* const out, RunLevel* const rl);
  static QuantizeBlockFunc quantize_block_;
  static QuantizeBlockFunc GetQuantizeBlockFunc();

  static int TrellisQuantizeBlock(const int16_t in[64], int idx,
                                  const Quantizer* const Q,
                                  DCTCoeffs* const out,
                                  RunLevel* const rl);

  typedef uint32_t (*QuantizeErrorFunc)(const int16_t in[64],
                                        const Quantizer* const Q);
  static QuantizeErrorFunc quantize_error_;
  static QuantizeErrorFunc GetQuantizeErrorFunc();

  void CodeBlock(const DCTCoeffs* const coeffs, const RunLevel* const rl);
  // returns DC code (4bits for length, 12bits for suffix), updates DC_predictor
  static uint16_t GenerateDCDiffCode(int DC, int* const DC_predictor);

  static void FinalizeQuantMatrix(Quantizer* const q, int bias);
  void SetCostCodes(int idx);
  void InitCodes(bool only_ac);

  size_t HeaderSize() const;
  void BlocksSize(int nb_mbs, const DCTCoeffs* coeffs,
                  const RunLevel* rl, sjpeg::BitCounter* const bc) const;
  float ComputeSize(const DCTCoeffs* coeffs);
  float ComputePSNR() const;

 protected:
  bool SetError();   // sets ok_ to true

  // format-specific parameters, set by virtual InitComponents()
  enum { MAX_COMP = 3 };
  int nb_comps_;
  int quant_idx_[MAX_COMP];       // indices for quantization matrices
  int nb_blocks_[MAX_COMP];       // number of 8x8 blocks per components
  uint8_t block_dims_[MAX_COMP];  // component dimensions (8-pixels units)
  int block_w_, block_h_;         // maximum mcu width / height
  int mcu_blocks_;                // total blocks in mcu (= sum of nb_blocks_[])

  // data accessible to sub-classes implementing alternate input format
  int W_, H_, step_;    // width, height, stride
  int mb_w_, mb_h_;     // width / height in units of mcu
  const uint8_t* const rgb_;   // samples

  // Replicate an RGB source sub_w x sub_h block, expanding it to w x h size.
  const uint8_t* GetReplicatedSamples(const uint8_t* rgb,    // block source
                                      int rgb_step,          // stride in source
                                      int sub_w, int sub_h,  // sub-block size
                                      int w, int h);         // size of mcu
  // Replicate an YUV sub-block similarly.
  const uint8_t* GetReplicatedYUVSamples(const uint8_t* in, int step,
                                         int sub_w, int sub_h, int w, int h);
  // set blocks that are totally outside of the picture to an average value
  void AverageExtraLuma(int sub_w, int sub_h, int16_t* out);
  uint8_t replicated_buffer_[3 * 16 * 16];   // tmp buffer for replication

  sjpeg::RGBToYUVBlockFunc get_yuv_block_;
  static sjpeg::RGBToYUVBlockFunc get_yuv444_block_;
  void SetYUVFormat(bool use_444) {
    get_yuv_block_ = sjpeg::GetBlockFunc(use_444);
  }
  bool adaptive_bias_;   // if true, use per-block perceptual bias modulation

  // Memory management
  template<class T> T* Alloc(size_t num) {
    assert(memory_hook_ != nullptr);
    T* const ptr = reinterpret_cast<T*>(memory_hook_->Alloc(sizeof(T) * num));
    if (ptr == nullptr) SetError();
    return ptr;
  }
  template<class T> void Free(T* const ptr) {
    memory_hook_->Free(reinterpret_cast<void*>(ptr));
  }

 private:
  bool ok_;                // set to false if a new[] fails
  sjpeg::BitWriter bw_;    // output buffer

  std::string iccp_, xmp_, exif_, app_markers_;   // metadata

  // compression tools. See sjpeg.h for description of methods.
  bool optimize_size_;        // Huffman-optimize the codes  (method 0, 3)
  bool use_adaptive_quant_;   // modulate the quant matrix   (method 3-8)
  bool use_extra_memory_;     // save the unquantized coeffs (method 3, 4)
  bool reuse_run_levels_;     // save quantized run/levels   (method 1, 4, 5)
  bool use_trellis_;          // use trellis-quantization    (method 7, 8)

  int q_bias_;           // [0..255]: rounding bias for quant. of AC coeffs.
  Quantizer quants_[2];  // quant matrices
  int DCs_[3];           // DC predictors

  // DCT coefficients storage, aligned
  static const size_t ALIGN_CST = 15;
  uint8_t* in_blocks_base_;   // base memory for blocks
  int16_t* in_blocks_;        // aligned pointer to in_blocks_base_
  bool have_coeffs_;          // true if the Fourier coefficients are stored
  bool AllocateBlocks(size_t num_blocks);  // returns false in case of error
  void DesallocateBlocks();

  // these are for regular compression methods 0 or 2.
  RunLevel base_run_levels_[64];

  // this is the extra memory for compression method 1
  RunLevel* all_run_levels_;
  size_t nb_run_levels_, max_run_levels_;

  // Huffman_tables_ indices:
  //  0: luma dc, 1: chroma dc, 2: luma ac, 3: chroma ac
  const HuffmanTable *Huffman_tables_[4];
  uint32_t ac_codes_[2][256];
  uint32_t dc_codes_[2][12];

  // histograms for dynamic codes. Could be temporaries.
  uint32_t freq_ac_[2][256 + 1];  // frequency distribution for AC coeffs
  uint32_t freq_dc_[2][12 + 1];   // frequency distribution for DC coeffs
  uint8_t opt_syms_ac_[2][256];   // optimal table for AC symbols
  uint8_t opt_syms_dc_[2][12];    // optimal table for DC symbols
  HuffmanTable opt_tables_ac_[2];
  HuffmanTable opt_tables_dc_[2];

  // Limits on how much we will decrease the bitrate in the luminance
  // and chrominance channels (respectively).
  int qdelta_max_luma_;
  int qdelta_max_chroma_;

  // Histogram handling

  // This function aggregates each 63 unquantized AC coefficients into an
  // histogram for further analysis.
  typedef void (*StoreHistoFunc)(const int16_t in[64], Histo* const histos,
                                 int nb_blocks);
  static StoreHistoFunc store_histo_;
  static StoreHistoFunc GetStoreHistoFunc();  // select between the above.

  // Provided the AC histograms have been stored with StoreHisto(), this
  // function will analyze impact of varying the quantization scales around
  // initial values, trading distortion for bit-rate in a controlled way.
  void AnalyseHisto();
  void ResetHisto();  // initialize histos_[]
  Histo histos_[2];

  // multi-pass parameters
  int passes_;
  SearchHook default_hook_;
  SearchHook* search_hook_;

  // lower memory management
  MemoryManager* memory_hook_;

  static const float kHistoWeight[QSIZE];

  static void (*fDCT_)(int16_t* in, int num_blocks);
  static void InitializeStaticPointers();
};

////////////////////////////////////////////////////////////////////////////////

}   // namespace sjpeg

#endif    // SJPEG_JPEGI_H_

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
//  Fast & simple JPEG encoder.
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef SJPEG_JPEG_H_
#define SJPEG_JPEG_H_

#include <inttypes.h>
#include <memory>
#include <string>
#include <vector>

#define SJPEG_VERSION 0x000100   // 0.1.0

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

// Returns the library's version.
uint32_t SjpegVersion();

// Main function
//  This is the simplest possible call. There is only one parameter (quality)
//  and most decisions will be made automatically (YUV420/YUV444/etc...).
//  Returns the compressed size, and fills *out_data with the bitstream.
//  This returned buffer is allocated with 'new[]' operator. It must be
//  deallocated by using 'delete[]' or SjpegFreeBuffer() calls.
//  Input data 'rgb' are the samples in sRGB format, in R/G/B memory order.
//  Picture dimension is width x height.
//  Returns 0 in case of error.
size_t SjpegCompress(const uint8_t* rgb, int width, int height, float quality,
                     uint8_t** out_data);

// Parameter 'yuv_mode': decides which colorspace to use. Possible values:
//   * YUV_AUTO  (0): automated decision between YUV 4:2:0 / sharp / 4:4:4
//   * YUV_420   (1): YUV 4:2:0
//   * YUV_SHARP (2): YUV 4:2:0 with 'sharp' conversion
//   * YUV_444   (3): YUV 4:4:4
typedef enum {
  SJPEG_YUV_AUTO = 0,
  SJPEG_YUV_420,
  SJPEG_YUV_SHARP,
  SJPEG_YUV_444
} SjpegYUVMode;

// Encodes an RGB picture to JPEG.
//
//  the dimension of the picture pointed to by 'rgb', is W * H, with stride
//  'stride' (must be greater or equal to 3*W). The dimensions must be strictly
//  positive.
//
// The compressed bytes are made available in *out_data, which is a buffer
// allocated with new []. This buffer must be disallocated using 'delete []',
// or by calling SjpegFreeBuffer().
//
// Return parameter -if positive- is the size of the JPEG string,
// or 0 if an error occurred.
//
// Parameter 'quality' correspond to the usual quality factor in JPEG:
//     0=very bad, 100=very good.
// Parameter 'compression_method' refer to the efforts and resources spent
//  trying to compress better. Default (fastest) method should be 0. Method 1
//  will optimize the size at the expense of using more RAM. Method 2 does
//  the same as method #1, but but without any additional RAM (but using twice
//  more CPU). Methods 3, 4, 5, and 6 behave like methods 0, 1, and 2, except
//  that the quantization matrices are fine-tuned to the source's content using
//  histogram. This requires an additional pass, and is hence slower, but can
//  give substantial filesize reduction, especially for hi-quality settings.
//  Method 5 will try to not use extra RAM to store the Fourier-transformed
//  coefficients, at the expense of being ~15% slower, but will still use some
//  memory for the Huffman size-optimization. Eventually, method 6 will use
//  a minimal amount of RAM, but will be must slower.
//  To recap:
//     method                     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
//     ---------------------------+---+---+---+---+---+---+---+---+---|
//     Huffman size-optimization  |   | x | x |   | x | x | x | x | x |
//     Adaptive quantization      |   |   |   | x | x | x | x | x | x |
//     Extra RAM for Huffman pass |   | x |   |   | x | x |   | x |   |
//     Extra RAM for histogram    |   |   |   | x | x |   |   | x |   |
//     Trellis-based quantization |   |   |   |   |   |   |   | x | x |
//
//  Methods sorted by decreasing speed: 0 > 1 > 2 > 3 > 4 > 5 > 6
//  Sorted by increasing efficiency: 0 < [1|2] < 3 < [4|5|6]
//
//  If you don't have any strict requirements on CPU and memory, you should
//  probably use method #4.
//
size_t SjpegEncode(const uint8_t* rgb,
                   int width, int height, int stride,
                   uint8_t** out_data,
                   float quality,
                   int compression_method,
                   SjpegYUVMode yuv_mode);

// Deallocate a compressed bitstream that were returned by SjpegEncode(),
// SjpegCompress() or sjpeg::Encode(). Useful for non-C++ bindings.
void SjpegFreeBuffer(const uint8_t* buffer);

////////////////////////////////////////////////////////////////////////////////
// JPEG-parsing tools

// Decode the dimensions of a JPEG bitstream, doing as few read operations as
// possible. Return false if an error occurred (invalid bitstream, invalid
// parameter...).
// The pointers 'width', 'height', 'is_yuv420' can be passed NULL.
bool SjpegDimensions(const uint8_t* data, size_t size,
                     int* width, int* height, int* is_yuv420);

// Finds the location of the first two quantization matrices within a JPEG
// 'data' bitstream. Matrices are 64 coefficients stored as uint8_t.
// The matrices are returned in natural order (not zigzag order).
// Note that the input can be truncated to include the headers only, but still
// must start as a valid JPEG with an 0xffd8 marker.
// Returns the number of matrices detected.
// Returns 0 in case of bitstream error, or if the DQT chunk is missing.
int SjpegFindQuantizer(const uint8_t* data, size_t size,
                       uint8_t quant[2][64]);

// Returns an estimation of the quality factor that would best approximate
// the quantization coefficients in matrix[].
// Note that matrix[] must be in natural order (not the zigzag order used
// in the byte stream). With this restriction, one can then pass the result
// of SjpegFindQuantizer() directly to SjpegEstimateQuality().
float SjpegEstimateQuality(const uint8_t matrix[64], bool for_chroma);

// Generate a default quantization matrix for the given quality factor,
// in a libjpeg-6b fashion.
void SjpegQuantMatrix(float quality, bool for_chroma, uint8_t matrix[64]);

// Returns the favored conversion mode to use (YUV420 / sharp-YUV420 / YUV444)
// Return values: SJPEG_YUV_420, SJPEG_YUV_SHARP or SJPEG_YUV_444
// If risk is not NULL, the riskiness score (between 0 and 100) is returned.
SjpegYUVMode SjpegRiskiness(const uint8_t* rgb, int width, int height,
                            int stride, float* risk);

#if defined(__cplusplus) || defined(c_plusplus)
}    // extern "C"
#endif

////////////////////////////////////////////////////////////////////////////////
// Variant of the function above, but using std::string as interface.

bool SjpegCompress(const uint8_t* rgb,
                   int width, int height, float quality, std::string* output);

bool SjpegDimensions(const std::string& jpeg_data,
                     int* width, int* height, int* is_yuv420);

int SjpegFindQuantizer(const std::string& jpeg_data, uint8_t quant[2][64]);


////////////////////////////////////////////////////////////////////////////////
// Advanced API, C++ only.
//    . Fine control over the encoding parameters using EncoderParam
//    . Interfaces to customize the codec
////////////////////////////////////////////////////////////////////////////////

namespace sjpeg {

// Forward declaration of internal struct:
struct Encoder;

// interfaces to customize the codec:
struct SearchHook;
struct ByteSink;
struct MemoryManager;

// Structure for holding encoding parameter, to be passed to the unique
// call to SjpegEncode() below. For a more detailed description of some fields,
// see SjpegEncode()'s doc above.
struct EncoderParam {
  EncoderParam();
  explicit EncoderParam(float quality_factor);

  // Sets the compression factor. 0 = lowest quality, 100 = best quality.
  // The call will actually initialize quant[][].
  void SetQuality(float quality_factor);

  // Reduce the output size by a factor 'reduction' in [0, 100]:
  //  reduction ~= 100 -> small size reduction
  //  reduction ~=   1 -> large size reduction
  // Note: 'reduction' can be larger than 100.
  // This function is incompatible with SetQuality()
  void SetQuantization(const uint8_t m[2][64], float reduction = 100.f);
  const uint8_t* GetQuantMatrix(int idx) const { return quant_[idx]; }

  // Limit the quantization by setting up some minimal quantization matrices
  // based on the current content of quant_[][] matrices.
  // Hence, this function must be called after SetQuality() or SetQuantMatrix().
  void SetLimitQuantization(bool limit_quantization = true, int tolerance = 0);

  // Set the minimal quantization matrices directly, irrespective of the value
  // of quant_[][].
  void SetMinQuantization(const uint8_t m[2][64], int min_quant_tolerance = 0);

  // main compression parameters
  SjpegYUVMode yuv_mode;        // YUV-420...444 decisions
  bool Huffman_compress;        // if true, use optimized Huffman tables.
  bool adaptive_quantization;   // if true, use optimized quantizer matrices.
  bool adaptive_bias;           // if true, use perceptual bias adaptation
  bool use_trellis;             // if true, use trellis-based optimization

  // target size or distortion
  typedef enum {
    TARGET_NONE = 0,
    TARGET_SIZE = 1,
    TARGET_PSNR = 2,
  } TargetMode;
  TargetMode target_mode;
  float target_value;           // size, psnr or SSIM
  int passes;                   // max number of passes to try and converge
  float tolerance;              // percentage of distance-to-target allowed
  float qmin, qmax;             // Limits for the search quality values.
                                // If set, min_quant_[] matrices will take
                                // precedence and limit qmax further.

  // fine-grained control over compression parameters
  int quantization_bias;    // [0..255] Rounding bias for quantization.
  int qdelta_max_luma;      // [0..12] How much to hurt luma in adaptive quant
  int qdelta_max_chroma;    // [0..12] How much to hurt chroma in adaptive quant
                            // A higher value might be useful for images
                            // encoded without chroma subsampling.

  // if null, a default implementation will be used
  sjpeg::SearchHook* search_hook;

  // metadata: extra EXIF/XMP/ICCP data that will be embedded in
  // APP1 or APP2 markers. They should contain only the raw payload and not
  // the prefixes ("Exif\0", "ICC_PROFILE", etc...). These will be added
  // automatically during encoding.
  // Conversely, the content of app_markers is written as is, right after APP0.
  std::string exif;
  std::string xmp;
  std::string iccp;
  std::string app_markers;
  void ResetMetadata();      // clears the above

  // Memory manager used by the codec. If null, default one will be used.
  sjpeg::MemoryManager* memory;

 protected:
  uint8_t quant_[2][64];         // quantization matrices to use
  uint8_t min_quant_[2][64];     // If limit_quantization is true, these
                                 // pointers should direct to the minimum
                                 // quantizer values allowed for luma / chroma.
  bool use_min_quant_;           // True if min_quant_[][] has been set.
  int min_quant_tolerance_;      // Tolerance going over min_quant_ ([0..100])

 protected:
  void Init(float quality_factor);
  friend struct sjpeg::Encoder;
};

// Same as the first version of SjpegEncode(), except encoding parameters are
// passed in a EncoderParam. Upon failure (memory allocation or
// invalid parameter), the function returns false.
bool Encode(const uint8_t* rgb, int width, int height, int stride,
            const EncoderParam& param, std::string* output);

// This version returns data in *out_data. Returns 0 in case of error.
size_t Encode(const uint8_t* rgb, int width, int height, int stride,
              const EncoderParam& param, uint8_t** out_data);

// Generic call taking a byte-sink for emitting the compressed data.
//   Same as SjpegEncode(), except encoding parameters are passed in a
//   EncoderParam. Upon failure (memory allocation or invalid parameter),
//   the function returns false.
bool Encode(const uint8_t* rgb, int width, int height, int stride,
            const EncoderParam& param, sjpeg::ByteSink* sink);

////////////////////////////////////////////////////////////////////////////////
// Some interfaces for customizing the core codec

// Custom search loop
struct SearchHook {
  float q;                // this is the current parameter used
  float qmin, qmax;       // this is the current bracket for q
  float target;           // target value (PSNR or size)
  float tolerance;        // relative tolerance for reaching the 'target' value
  bool for_size;          // true if we're searching for size
  float value;            // result for the search after Update() is called
  int pass;               // pass number (0-based) during search (informative)

  // Returns false in case of initialization error.
  // Should always be called by sub-classes.
  virtual bool Setup(const EncoderParam& param);
  // Set up the next matrices to try, corresponding to the current q value.
  // 'idx' is 0 for luma, 1 for chroma
  virtual void NextMatrix(int idx, uint8_t dst[64]);
  // return true if the search is finished
  virtual bool Update(float result);
  virtual ~SearchHook() {}
};

////////////////////////////////////////////////////////////////////////////////
// Generic byte-sink: custom streaming output of compressed data
//
// Protocol:
//  . Commit(used_size, extra_size, buffer): specify that 'used_size' bytes
//       were used since the last call to Commit(). Also reserve 'extra_size'
//       bytes for the next cycle and make *data point to the corresponding
//       memory. 'extra_size' can be 0, in which case *buffer does not need
//       to point to a valid memory area. Most of the time (except during
//       header writing), 'extra_size' will be less than 2048.
//       Returns false in case of error (both flushing used_size, or allocating
//       extra_size).
//  . Finalize(): indicates that calls to Commit() are finished until the
//       destruction (and the assembled byte-stream can be grabbed).
//       Returns false in case of I/O error.
//  . Reset(): releases resources (called in case of error or at destruction).

struct ByteSink {
 public:
  virtual ~ByteSink() {}
  virtual bool Commit(size_t used_size, size_t extra_size, uint8_t** data) = 0;
  virtual bool Finalize() = 0;
  virtual void Reset() = 0;
};

// Some useful factories
std::shared_ptr<ByteSink> MakeByteSink(std::string* output);
// Vector-based template, specialized for uint8_t
template<typename T>
std::shared_ptr<ByteSink> MakeByteSink(std::vector<T>* output);
template<> std::shared_ptr<ByteSink> MakeByteSink(std::vector<uint8_t>* output);

////////////////////////////////////////////////////////////////////////////////
// Memory manager (for internal allocation)

struct MemoryManager {
 public:
  virtual ~MemoryManager() {}
  virtual void* Alloc(size_t size) = 0;     // same semantic as malloc()
  virtual void Free(void* const ptr) = 0;   // same semantic as free()
};

}  // namespace sjpeg

#endif    // SJPEG_JPEG_H_

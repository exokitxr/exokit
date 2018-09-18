// Copyright 2017 Google, Inc.
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
//  Few utilities for the examples
//
// Author: Mikolaj Zalewski (mikolajz@google.com)
//         Skal (pascal.massimino@gmail.com)

#include "./utils.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>

#ifdef SJPEG_HAVE_PNG
#include <png.h>
#include <setjmp.h>   // note: this must be included *after* png.h
#endif    // SJPEG_HAVE_PNG

using std::vector;
using sjpeg::EncoderParam;

std::string ReadFile(const char filename[]) {
  std::string data;
  int ok = 0;
  FILE* const f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "ERROR: could not open file %s for reading.\n", filename);
  } else {
    do {
      if (fseek(f, 0, SEEK_END) < 0) break;
      const int file_size = ftell(f);
      if (file_size < 0) break;
      if (fseek(f, 0, SEEK_SET) < 0) break;
      data.resize(file_size);
      ok = (fread(&data[0], file_size, 1, f) == 1);
    } while (0);
  }
  if (f != NULL) fclose(f);
  if (!ok) {
    data.clear();
    fprintf(stderr, "ERROR while reading file %s.\n", filename);
  }
  return data;
}

int SaveFile(const char* name, const std::string& out, bool quiet) {
  bool ok = false;
  if (name != NULL) {
    FILE* const f = fopen(name, "wb");
    if (f != NULL && fwrite(out.data(), out.size(), 1, f) == 1) {
      if (!quiet) fprintf(stdout, "Saved file: %s\n", name);
      ok = true;
    } else {
      fprintf(stderr, "Error while saving %s\n", name);
    }
    if (f != NULL) fclose(f);
  }
  return ok;
}

#if defined _WIN32 && !defined __GNUC__
#include <windows.h>

double GetStopwatchTime() {
  LARGE_INTEGER watch;
  LARGE_INTEGER freq;
  if (!QueryPerformanceCounter(&watch))
    return 0.0;
  if (!QueryPerformanceFrequency(&freq))
    return 0.0;
  if (freq.QuadPart == 0)
    return 0.0;
  return watch.QuadPart / static_cast<double>(freq.QuadPart);
}

#else    /* !_WIN32 */
#include <string.h>  // memcpy
#include <sys/time.h>

double GetStopwatchTime() {
  struct timeval watch;
  gettimeofday(&watch, NULL);
  const double sec = static_cast<double>(watch.tv_sec);
  const double usec = static_cast<double>(watch.tv_usec);
  return sec + usec / 1000000.0;
}

#endif   /* _WIN32 */

////////////////////////////////////////////////////////////////////////////////

uint32_t GetCRC32(const std::string& data, uint32_t crc) {
  // CRC32's polynomial 0x4c11db7, reversed
  const uint32_t kPolynomial = 0xedb88320u;
  crc = ~crc;
  for (size_t n = 0; n < data.size(); ++n) {
    crc ^= static_cast<uint8_t>(data[n]);
    for (size_t j = 0; j < 8; ++j) {
      crc = (crc >> 1) ^ ((crc & 1) ? kPolynomial : 0u);
    }
  }
  return ~crc;
}

////////////////////////////////////////////////////////////////////////////////
// JPEG reading

#ifdef SJPEG_HAVE_JPEG
#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

// -----------------------------------------------------------------------------
// Metadata processing

#ifndef JPEG_APP1
# define JPEG_APP1 (JPEG_APP0 + 1)
#endif
#ifndef JPEG_APP2
# define JPEG_APP2 (JPEG_APP0 + 2)
#endif

typedef struct {
  const uint8_t* data;
  size_t data_length;
  int seq;  // this segment's sequence number [1, 255] for use in reassembly.
} ICCPSegment;

static void SaveMetadataMarkers(j_decompress_ptr dinfo) {
  const unsigned int max_marker_length = 0xffff;
  jpeg_save_markers(dinfo, JPEG_APP1, max_marker_length);  // Exif/XMP
  jpeg_save_markers(dinfo, JPEG_APP2, max_marker_length);  // ICC profile
}

static int CompareICCPSegments(const void* a, const void* b) {
  const ICCPSegment* const s1 = static_cast<const ICCPSegment*>(a);
  const ICCPSegment* const s2 = static_cast<const ICCPSegment*>(b);
  return s1->seq - s2->seq;
}

// Extract ICC profile segments from the marker list in 'dinfo', reassembling
// and storing them in 'iccp'.
// Returns true on success and false for memory errors and corrupt profiles.
static int StoreICCP(j_decompress_ptr dinfo, std::string* const iccp) {
  // ICC.1:2010-12 (4.3.0.0) Annex B.4 Embedding ICC Profiles in JPEG files
  static const char kICCPSignature[] = "ICC_PROFILE";
  static const size_t kICCPSignatureLength = 12;  // signature includes '\0'
  static const size_t kICCPSkipLength = 14;  // signature + seq & count
  int expected_count = 0;
  int actual_count = 0;
  int seq_max = 0;
  size_t total_size = 0;
  ICCPSegment iccp_segments[255];
  jpeg_saved_marker_ptr marker;

  memset(iccp_segments, 0, sizeof(iccp_segments));
  for (marker = dinfo->marker_list; marker != NULL; marker = marker->next) {
    if (marker->marker == JPEG_APP2 &&
        marker->data_length > kICCPSkipLength &&
        !memcmp(marker->data, kICCPSignature, kICCPSignatureLength)) {
      // ICC_PROFILE\0<seq><count>; 'seq' starts at 1.
      const int seq = marker->data[kICCPSignatureLength];
      const int count = marker->data[kICCPSignatureLength + 1];
      const size_t segment_size = marker->data_length - kICCPSkipLength;
      ICCPSegment* segment;

      if (segment_size == 0 || count == 0 || seq == 0) {
        fprintf(stderr, "[ICCP] size (%d) / count (%d) / sequence number (%d)"
                        " cannot be 0!\n",
                static_cast<int>(segment_size), seq, count);
        return 0;
      }

      if (expected_count == 0) {
        expected_count = count;
      } else if (expected_count != count) {
        fprintf(stderr, "[ICCP] Inconsistent segment count (%d / %d)!\n",
                expected_count, count);
        return 0;
      }

      segment = iccp_segments + seq - 1;
      if (segment->data_length != 0) {
        fprintf(stderr, "[ICCP] Duplicate segment number (%d)!\n" , seq);
        return 0;
      }

      segment->data = marker->data + kICCPSkipLength;
      segment->data_length = segment_size;
      segment->seq = seq;
      total_size += segment_size;
      if (seq > seq_max) seq_max = seq;
      ++actual_count;
    }
  }

  if (actual_count == 0) return 1;
  if (seq_max != actual_count) {
    fprintf(stderr, "[ICCP] Discontinuous segments, expected: %d actual: %d!\n",
            actual_count, seq_max);
    return 0;
  }
  if (expected_count != actual_count) {
    fprintf(stderr, "[ICCP] Segment count: %d does not match expected: %d!\n",
            actual_count, expected_count);
    return 0;
  }

  // The segments may appear out of order in the file, sort them based on
  // sequence number before assembling the payload.
  qsort(iccp_segments, actual_count, sizeof(*iccp_segments),
        CompareICCPSegments);

  iccp->resize(total_size);
  size_t offset = 0;
  for (int i = 0; i < seq_max; ++i) {
    memcpy(&(*iccp)[offset],
           iccp_segments[i].data, iccp_segments[i].data_length);
    offset += iccp_segments[i].data_length;
  }
  return 1;
}

// Returns true on success and false for memory errors and corrupt profiles.
// The caller must use MetadataFree() on 'metadata' in all cases.
static int ExtractMetadataFromJPEG(j_decompress_ptr dinfo,
                                   EncoderParam* const param) {
  if (param == NULL) return true;
  param->ResetMetadata();
  const struct {
    int marker;
    const char* signature;
    size_t signature_length;
    std::string* data;
  } metadata_map[] = {
    // Exif 2.2 Section 4.7.2 Interoperability Structure of APP1 ...
    { JPEG_APP1, "Exif\0",                        6, &param->exif },
    // XMP Specification Part 3 Section 3 Embedding XMP Metadata ... #JPEG
    // TODO(jzern) Add support for 'ExtendedXMP'
    { JPEG_APP1, "http://ns.adobe.com/xap/1.0/", 29, &param->xmp },
    { 0, NULL, 0, 0 },
  };
  jpeg_saved_marker_ptr marker;
  // Treat ICC profiles separately as they may be segmented and out of order.
  if (!StoreICCP(dinfo, &param->iccp)) return 0;

  for (marker = dinfo->marker_list; marker != NULL; marker = marker->next) {
    int i;
    for (i = 0; metadata_map[i].marker != 0; ++i) {
      if (marker->marker == metadata_map[i].marker &&
          marker->data_length > metadata_map[i].signature_length &&
          !memcmp(marker->data, metadata_map[i].signature,
                  metadata_map[i].signature_length)) {
        std::string* const payload = metadata_map[i].data;

        if (payload->size() == 0) {
          const char* marker_data =
              reinterpret_cast<const char*>(marker->data) +
              metadata_map[i].signature_length;
          const size_t marker_data_length =
              marker->data_length - metadata_map[i].signature_length;
          payload->append(marker_data, marker_data_length);
        } else {
          fprintf(stderr, "Ignoring additional '%s' marker\n",
                  metadata_map[i].signature);
        }
      }
    }
  }
  return 1;
}

#undef JPEG_APP1
#undef JPEG_APP2

// -----------------------------------------------------------------------------
// JPEG decoding

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

static void my_error_exit(j_common_ptr dinfo) {
  struct my_error_mgr* myerr =
      reinterpret_cast<struct my_error_mgr*>(dinfo->err);
  dinfo->err->output_message(dinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

typedef struct {
  struct jpeg_source_mgr pub;
  const uint8_t* data;
  size_t data_size;
} JPEGReadContext;

static void ContextInit(j_decompress_ptr cinfo) {
  JPEGReadContext* const ctx = reinterpret_cast<JPEGReadContext*>(cinfo->src);
  ctx->pub.next_input_byte = ctx->data;
  ctx->pub.bytes_in_buffer = ctx->data_size;
}

static boolean ContextFill(j_decompress_ptr cinfo) {
  // we shouldn't get here.
  ERREXIT(cinfo, JERR_FILE_READ);
  return FALSE;
}

static void ContextSkip(j_decompress_ptr cinfo, long jump_size) {  // NOLINT
  JPEGReadContext* const ctx = reinterpret_cast<JPEGReadContext*>(cinfo->src);
  size_t jump = (size_t)jump_size;
  if (jump > ctx->pub.bytes_in_buffer) {  // Don't overflow the buffer.
    jump = ctx->pub.bytes_in_buffer;
  }
  ctx->pub.bytes_in_buffer -= jump;
  ctx->pub.next_input_byte += jump;
}

static void ContextTerm(j_decompress_ptr cinfo) {
  (void)cinfo;
}

static void ContextSetup(volatile struct jpeg_decompress_struct* const cinfo,
                         JPEGReadContext* const ctx) {
  cinfo->src = reinterpret_cast<struct jpeg_source_mgr*>(ctx);
  ctx->pub.init_source = ContextInit;
  ctx->pub.fill_input_buffer = ContextFill;
  ctx->pub.skip_input_data = ContextSkip;
  ctx->pub.resync_to_restart = jpeg_resync_to_restart;
  ctx->pub.term_source = ContextTerm;
  ctx->pub.bytes_in_buffer = 0;
  ctx->pub.next_input_byte = NULL;
}

vector<uint8_t> ReadJPEG(const std::string& in,
                         int* const width, int* const height,
                         EncoderParam* const param) {
  volatile int ok = 0;
  int64_t stride;
  volatile struct jpeg_decompress_struct dinfo;
  struct my_error_mgr jerr;
  vector<uint8_t> rgb;
  JSAMPROW buffer[1];
  JPEGReadContext ctx;

  assert(in.size() != 0);
  if (width != NULL) *width = 0;
  if (height != NULL) *height = 0;

  memset(&ctx, 0, sizeof(ctx));
  ctx.data = (const uint8_t*)in.data();
  ctx.data_size = in.size();

  memset((j_decompress_ptr)&dinfo, 0, sizeof(dinfo));   // for setjmp sanity
  dinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
 Error:
    jpeg_destroy_decompress((j_decompress_ptr)&dinfo);
    fprintf(stderr, "Error during JPEG decompression.\n");
    goto End;
  }

  jpeg_create_decompress((j_decompress_ptr)&dinfo);
  ContextSetup(&dinfo, &ctx);
  SaveMetadataMarkers((j_decompress_ptr)&dinfo);
  jpeg_read_header((j_decompress_ptr)&dinfo, TRUE);

  dinfo.out_color_space = JCS_RGB;
  dinfo.do_fancy_upsampling = TRUE;

  jpeg_start_decompress((j_decompress_ptr)&dinfo);

  if (dinfo.output_components != 3) {
    fprintf(stderr, "JPEG is not using an RGB colorspace.\n");
    goto Error;
  }

  stride = (int64_t)dinfo.output_width
         * dinfo.output_components * sizeof(rgb[0]);

  if (stride == 0 || stride != static_cast<int>(stride) ||
    static_cast<uint64_t>(dinfo.output_height) >
        (1ull << 31) / static_cast<uint64_t>(stride)) {
    goto End;
  }
  rgb.resize((size_t)stride * dinfo.output_height);
  buffer[0] = static_cast<JSAMPLE*>(&rgb[0]);

  while (dinfo.output_scanline < dinfo.output_height) {
    if (jpeg_read_scanlines((j_decompress_ptr)&dinfo, buffer, 1) != 1) {
      goto End;
    }
    buffer[0] += stride;
  }

  if (!ExtractMetadataFromJPEG((j_decompress_ptr)&dinfo, param)) {
    fprintf(stderr, "Error extracting JPEG metadata!\n");
    goto Error;
  }

  jpeg_finish_decompress(const_cast<j_decompress_ptr>(&dinfo));
  jpeg_destroy_decompress(const_cast<j_decompress_ptr>(&dinfo));
  if (width != NULL) *width = dinfo.output_width;
  if (height != NULL) *height = dinfo.output_height;

  ok = 1;

 End:
  if (!ok) {
    rgb.clear();
    if (param != NULL) param->ResetMetadata();
  }
  return rgb;
}
#else  // !SJPEG_HAVE_JPEG
vector<uint8_t> ReadJPEG(const std::string& in,
                         int* const width, int* const height,
                         EncoderParam* const param) {
  (void)in.size();
  (void)param;
  if (width != NULL) *width = 0;
  if (height != NULL) *height = 0;
  fprintf(stderr, "JPEG support not compiled. Please install the libjpeg "
                  "development package before building.\n");
  return vector<uint8_t>();
}
#endif  // SJPEG_HAVE_JPEG

////////////////////////////////////////////////////////////////////////////////
// PNG reading

#ifdef SJPEG_HAVE_PNG
static void PNGAPI error_function(png_structp png, png_const_charp error) {
  if (error != NULL) fprintf(stderr, "libpng error: %s\n", error);
  longjmp(png_jmpbuf(png), 1);
}

// Converts the NULL terminated 'hexstring' which contains 2-byte character
// representations of hex values to raw data.
// 'hexstring' may contain values consisting of [A-F][a-f][0-9] in pairs,
// e.g., 7af2..., separated by any number of newlines.
// 'expected_length' is the anticipated processed size.
// On success the raw buffer is returned with its length equivalent to
// 'expected_length'. NULL is returned if the processed length is less than
// 'expected_length' or any character aside from those above is encountered.
// The returned buffer must be freed by the caller.
static std::string HexStringToBytes(const char* hexstring,
                                    size_t expected_length) {
  const char* src = hexstring;
  size_t actual_length = 0;
  std::string raw_data;
  raw_data.resize(expected_length);
  char* dst = const_cast<char*>(raw_data.data());

  for (; actual_length < expected_length && *src != '\0'; ++src) {
    char* end;
    char val[3];
    if (*src == '\n') continue;
    val[0] = *src++;
    val[1] = *src;
    val[2] = '\0';
    *dst++ = static_cast<uint8_t>(strtol(val, &end, 16));
    if (end != val + 2) break;
    ++actual_length;
  }

  if (actual_length != expected_length) {
    raw_data.clear();
  }
  return raw_data;
}

static bool ProcessRawProfile(const char* profile, size_t profile_len,
                              std::string* const metadata) {
  const char* src = profile;
  char* end;
  int expected_length;

  if (profile == NULL || profile_len == 0) return false;

  // ImageMagick formats 'raw profiles' as
  // '\n<name>\n<length>(%8lu)\n<hex payload>\n'.
  if (*src != '\n') {
    fprintf(stderr, "Malformed raw profile, expected '\\n' got '\\x%.2X'\n",
            *src);
    return false;
  }
  ++src;
  // skip the profile name and extract the length.
  while (*src != '\0' && *src++ != '\n') {}
  expected_length = static_cast<int>(strtol(src, &end, 10));
  if (*end != '\n') {
    fprintf(stderr, "Malformed raw profile, expected '\\n' got '\\x%.2X'\n",
            *end);
    return false;
  }
  ++end;

  // 'end' now points to the profile payload.
  const std::string payload = HexStringToBytes(end, expected_length);
  if (payload.size() == 0) return false;
  metadata->append(payload);
  return true;
}

static bool ProcessCopy(const char* profile, size_t profile_len,
                        std::string* const metadata) {
  if (profile == NULL || profile_len == 0) return false;
  metadata->append(profile, profile_len);
  return true;
}

static const struct {
  const char* name;
  bool (*process)(const char* profile, size_t profile_len,
                  std::string* const metadata);
  bool is_exif;   // otherwise: XMP
} kPNGMetadataMap[] = {
  // http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/PNG.html#TextualData
  // See also: ExifTool on CPAN.
  { "Raw profile type exif", ProcessRawProfile, true },    // exif
  { "Raw profile type xmp",  ProcessRawProfile, false },   // xmp
  // Exiftool puts exif data in APP1 chunk, too.
  { "Raw profile type APP1", ProcessRawProfile, true },    // exif
  // XMP Specification Part 3, Section 3 #PNG
  { "XML:com.adobe.xmp",     ProcessCopy      , false  },  // xmp
  { NULL, NULL, false },
};

// Looks for metadata at both the beginning and end of the PNG file, giving
// preference to the head.
// Returns true on success. The caller must use MetadataFree() on 'metadata' in
// all cases.
static bool ExtractMetadataFromPNG(png_structp png,
                                   png_infop const head_info,
                                   png_infop const end_info,
                                   EncoderParam* const param) {
  if (param == NULL) return true;
  param->ResetMetadata();
  for (int p = 0; p < 2; ++p)  {
    png_infop const info = (p == 0) ? head_info : end_info;
    png_textp text = NULL;
    const png_uint_32 num = png_get_text(png, info, &text, NULL);
    // Look for EXIF / XMP metadata.
    for (png_uint_32 i = 0; i < num; ++i, ++text) {
      for (int j = 0; kPNGMetadataMap[j].name != NULL; ++j) {
        if (!strcmp(text->key, kPNGMetadataMap[j].name)) {
          std::string* metadata =
              kPNGMetadataMap[j].is_exif ? &param->exif : &param->xmp;
          png_size_t text_length;
          switch (text->compression) {
#ifdef PNG_iTXt_SUPPORTED
            case PNG_ITXT_COMPRESSION_NONE:
            case PNG_ITXT_COMPRESSION_zTXt:
              text_length = text->itxt_length;
              break;
#endif
            case PNG_TEXT_COMPRESSION_NONE:
            case PNG_TEXT_COMPRESSION_zTXt:
            default:
              text_length = text->text_length;
              break;
          }
          if (metadata->size() > 0) {
            fprintf(stderr, "Ignoring additional '%s'\n", text->key);
          } else if (!kPNGMetadataMap[j].process(text->text, text_length,
                                                 metadata)) {
            fprintf(stderr, "Failed to process: '%s'\n", text->key);
            return 0;
          }
          break;
        }
      }
    }
    // Look for an ICC profile.
    {
      png_charp name;
      int comp_type;
#if ((PNG_LIBPNG_VER_MAJOR << 8) | PNG_LIBPNG_VER_MINOR << 0) < \
    ((1 << 8) | (5 << 0))
      png_charp profile;
#else  // >= libpng 1.5.0
      png_bytep profile;
#endif
      png_uint_32 len;

      if (png_get_iCCP(png, info,
                       &name, &comp_type, &profile, &len) == PNG_INFO_iCCP) {
        if (!ProcessCopy(reinterpret_cast<const char*>(profile), len,
                         &param->iccp)) {
          return 0;
        }
        fprintf(stderr, "[%s : %d bytes]\n", "ICCP", static_cast<int>(len));
      }
    }
  }
  return 1;
}

typedef struct {
  const uint8_t* data;
  size_t data_size;
  png_size_t offset;
} PNGReadContext;

static void ReadFunc(png_structp png_ptr, png_bytep data, png_size_t length) {
  PNGReadContext* const ctx =
      static_cast<PNGReadContext*>(png_get_io_ptr(png_ptr));
  if (ctx->data_size - ctx->offset < length) {
    png_error(png_ptr, "ReadFunc: invalid read length (overflow)!");
  }
  memcpy(data, ctx->data + ctx->offset, length);
  ctx->offset += length;
}

vector<uint8_t> ReadPNG(const std::string& input,
                        int* const width_ptr, int* const height_ptr,
                        EncoderParam* const param) {
  volatile png_structp png = NULL;
  volatile png_infop info = NULL;
  volatile png_infop end_info = NULL;
  PNGReadContext context = { NULL, 0, 0 };
  int color_type, bit_depth, interlaced;
  int has_alpha;
  int num_passes;
  int p;
  png_uint_32 width, height, y;
  int64_t stride;
  vector<uint8_t> rgb;

  assert(input.size() > 0);

  context.data = (const uint8_t*)input.data();
  context.data_size = input.size();

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (png == NULL) goto End;

  png_set_error_fn(png, 0, error_function, NULL);
  if (setjmp(png_jmpbuf(png))) {
 Error:
    if (param != NULL) param->ResetMetadata();
    rgb.clear();
    goto End;
  }

  info = png_create_info_struct(png);
  if (info == NULL) goto Error;
  end_info = png_create_info_struct(png);
  if (end_info == NULL) goto Error;

  png_set_read_fn(png, &context, ReadFunc);
  png_read_info(png, info);
  if (!png_get_IHDR(png, info,
                    &width, &height, &bit_depth, &color_type, &interlaced,
                    NULL, NULL)) goto Error;

  png_set_strip_16(png);
  png_set_packing(png);
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png);
  }
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (bit_depth < 8) {
      png_set_expand_gray_1_2_4_to_8(png);
    }
    png_set_gray_to_rgb(png);
  }
  if (png_get_valid(png, info, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png);
    has_alpha = 1;
  } else {
    has_alpha = !!(color_type & PNG_COLOR_MASK_ALPHA);
  }

  if (has_alpha) {
    png_set_strip_alpha(png);
    has_alpha = 0;
  }

  // Apply gamma correction if needed.
  {
    double image_gamma = 1 / 2.2, screen_gamma = 2.2;
    int srgb_intent;
    if (png_get_sRGB(png, info, &srgb_intent) ||
        png_get_gAMA(png, info, &image_gamma)) {
      png_set_gamma(png, screen_gamma, image_gamma);
    }
  }

  num_passes = png_set_interlace_handling(png);
  png_read_update_info(png, info);

  stride = static_cast<int64_t>(3 * width) * sizeof(rgb[0]);
  if (stride != static_cast<int>(stride) ||
        static_cast<uint64_t>(height) >
            ((1ull << 31) / static_cast<uint64_t>(stride))) {
    goto Error;
  }

  rgb.resize((size_t)stride * height);
  for (p = 0; p < num_passes; ++p) {
    png_bytep row = &rgb[0];
    for (y = 0; y < height; ++y) {
      png_read_rows(png, &row, NULL, 1);
      row += stride;
    }
  }
  png_read_end(png, end_info);

  if (!ExtractMetadataFromPNG(png, info, end_info, param)) {
    fprintf(stderr, "Error extracting PNG metadata!\n");
    goto Error;
  }

  if (width_ptr != NULL) *width_ptr = static_cast<int>(width);
  if (height_ptr != NULL) *height_ptr = static_cast<int>(height);

 End:
  if (png != NULL) {
    png_destroy_read_struct((png_structpp)&png,
                            (png_infopp)&info, (png_infopp)&end_info);
  }
  return rgb;
}

#else  // !SJPEG_HAVE_PNG
vector<uint8_t> ReadPNG(const std::string& input,
                        int* const width, int* const height,
                        EncoderParam* const param) {
  (void)input;
  (void)param;
  if (width != NULL) *width = 0;
  if (height != NULL) *height = 0;
  fprintf(stderr, "PNG support not compiled. Please install the libpng "
                  "development package before building.\n");
  return vector<uint8_t>();
}
#endif  // SJPEG_HAVE_PNG

////////////////////////////////////////////////////////////////////////////////
// Simple PPM fallback

#define MAX_LINE_SIZE 1024

static size_t ReadLine(const std::string& input, size_t* const off,
                       char out[MAX_LINE_SIZE + 1]) {
  size_t i = 0;
 redo:
  for (i = 0; i < MAX_LINE_SIZE && *off < input.size(); ++i) {
    out[i] = input[(*off)++];
    if (out[i] == '\n') break;
  }
  if (*off < input.size()) {
    if (i == 0) goto redo;         // empty line
    if (out[0] == '#') goto redo;  // skip comment
  }
  out[i] = 0;   // safety sentinel
  return i;
}

vector<uint8_t> ReadPPM(const std::string& input,
                        int* const width, int* const height,
                        EncoderParam* const param) {
  vector<uint8_t> rgb;
  size_t offset = 0;
  char out[MAX_LINE_SIZE + 1];
  int type = 0;
  int max_value = 0;
  int W = 0, H = 0;
  if (width != NULL) *width = 0;
  if (height != NULL) *height = 0;
  if (param != NULL) param->ResetMetadata();

  if (ReadLine(input, &offset, out) == 0 || sscanf(out, "P%d", &type) != 1) {
    return rgb;
  }
  if (type != 6) {
    fprintf(stderr, "PPM not in RGB format (P6)\n");
    return rgb;
  }
  if (ReadLine(input, &offset, out) == 0 ||
      sscanf(out, "%d %d", &W, &H) != 2) {
    return rgb;
  }
  if (ReadLine(input, &offset, out) == 0 ||
      sscanf(out, "%d", &max_value) != 1 ||
      max_value > 255) {
    return rgb;
  }
  const size_t rgb_size = W * H * 3;
  if (input.size() - offset < rgb_size) return rgb;
  rgb.resize(rgb_size);
  if (rgb_size) memcpy(&rgb[0], &input[offset], rgb_size);
  if (width != NULL) *width = W;
  if (height != NULL) *height = H;
  return rgb;
}

////////////////////////////////////////////////////////////////////////////////

const char* ImageTypeName(ImageType type) {
  switch (type) {
    case SJPEG_JPEG: return "JPG";
    case SJPEG_PNG: return "PNG";
    case SJPEG_PPM: return "PPM";
    default: return "???";
  }
}

// default reader, returning a systematic error
std::vector<uint8_t> ReadFail(const std::string& in,
                              int* const width, int* const height,
                              EncoderParam* const param) {
  (void)in;
  (void)width;
  (void)height;
  if (param != NULL) param->ResetMetadata();
  return std::vector<uint8_t>();
}

ImageType GuessImageType(const std::string& input) {
  if (input.size() >= 3) {
    const uint32_t sig =
        ((uint8_t)input[0] << 24) | ((uint8_t)input[1] << 16) |
        ((uint8_t)input[2] <<  8) | ((uint8_t)input[3] <<  0);
    if ((sig >> 8) == 0xffd8ff) return SJPEG_JPEG;
    if (sig == 0x89504e47) return SJPEG_PNG;
    if ((sig >> 16) == 0x5036) return SJPEG_PPM;  // "P6"
  }
  return SJPEG_UNKNOWN;
}

ImageReader GuessImageReader(const std::string& input) {
  switch (GuessImageType(input)) {
    case SJPEG_JPEG: return ReadJPEG;
    case SJPEG_PNG: return ReadPNG;
    case SJPEG_PPM: return ReadPPM;
    default: return ReadFail;
  }
}

std::vector<uint8_t> ReadImageQuick(const std::string& in,
                                    int* const width, int* const height,
                                    EncoderParam* const param) {
  return GuessImageReader(in)(in, width, height, param);
}

std::vector<uint8_t> ReadImage(const std::string& in,
                               int* const width, int* const height,
                               EncoderParam* const param) {
  vector<uint8_t> rgb = ReadImageQuick(in, width, height, param);
  // quick attempt failed, try the rest in order
  if (rgb.size() == 0) rgb = ReadJPEG(in, width, height, param);
  if (rgb.size() == 0) rgb = ReadPNG(in, width, height, param);
  if (rgb.size() == 0) rgb = ReadPPM(in, width, height, param);
  return rgb;
}

////////////////////////////////////////////////////////////////////////////////

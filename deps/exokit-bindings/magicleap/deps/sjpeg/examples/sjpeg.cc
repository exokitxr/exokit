// Copyright 2017 Google, Inc.
//
//  simple JPEG compressor or re-compressor
//
// usage:
//   sjpeg input.{jpg,png} [-o output.jpg] [-q quality]
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
// Author: Skal (pascal.massimino@gmail.com)
//

#include "sjpeg.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "./utils.h"

using std::vector;
using sjpeg::EncoderParam;

#if !defined(ALT_HOOK_CLASS)
#define ALT_HOOK_CLASS sjpeg::SearchHook   // fall back to default search
#endif

///////////////////////////////////////////////////////////////////////////////

static void PrintMatrix(const char name[], const uint8_t m[64],
                        bool for_chroma) {
  fprintf(stdout, " %s quantization matrix (estimated quality: %.1f)\n",
          name, SjpegEstimateQuality(m, for_chroma));
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) fprintf(stdout, "%3d ", m[i + j * 8]);
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "------\n");
}

static void PrintMetadataInfo(const EncoderParam& param) {
  if (!param.iccp.empty()) {
    fprintf(stdout, "ICCP:       %6u bytes (CRC32: 0x%.8x)\n",
            static_cast<uint32_t>(param.iccp.size()), GetCRC32(param.iccp));
  }
  if (!param.exif.empty()) {
    fprintf(stdout, "EXIF:       %6u bytes (CRC32: 0x%.8x)\n",
            static_cast<uint32_t>(param.exif.size()), GetCRC32(param.exif));
  }
  if (!param.xmp.empty()) {
    fprintf(stdout, "XMP:        %6u bytes (CRC32: 0x%.8x)\n",
            static_cast<uint32_t>(param.xmp.size()), GetCRC32(param.xmp));
  }
}

///////////////////////////////////////////////////////////////////////////////

static const char* kYUVModeNames[4] = {
  "automatic", "YUV420", "SharpYUV420", "YUV444"
};
static const char* kNoYes[2] = { "no", "yes" };

int main(int argc, char * argv[]) {
  const char* input_file = nullptr;
  const char* output_file = nullptr;
  EncoderParam param;
  float reduction = 100;
  float quality = 75;
  bool use_reduction = true;  // until '-q' is used...
  bool no_metadata = false;
  bool estimate = false;
  bool limit_quantization = true;
  int info = 0;
  bool quiet = false;
  bool short_output = false;
  bool print_crc = false;
  float riskiness = 0;
  SjpegYUVMode yuv_mode_rec = SJPEG_YUV_AUTO;
  const char* const usage =
    "sjpeg: Commandline utility to recompress or compress pictures to JPEG.\n"
    "Usage:  sjpeg infile [-o outfile.jpg] [-q quality] ...\n"
    "  -q <float> ...... Quality factor in [0..100] range.\n"
    "                    Value of 100 gives the best quality, largest file.\n"
    "                    Default value is 75.\n"
    "  -r <float> ...... Reduction factor in [0..100] range.\n"
    "                    Default value is 100. Lower value will reduce the \n"
    "                    file size.\n"
    "  -o filename ..... specifies the output file name.\n"
    "  -size <int> ..... target size\n"
    "  -psnr <float> ... target YUV-PSNR\n"
    "  -estimate ....... Just estimate and print the JPEG source quality.\n"
    "  -i .............. Just print some information about the input file.\n"
    "  -version ........ Print the version and exit.\n"
    "  -quiet .......... Quiet mode. Just save the file.\n"
    "  -short .......... Print shorter 1-line info.\n"
    "  -crc ............ Just print the output checksum and exit.\n"
    "\n"
    "Advanced options:\n"
    "  -yuv_mode .......... YUV mode to use:\n"
    "                       0: automatic decision (default)\n"
    "                       1: use YUV 4:2:0\n"
    "                       2: use 'Sharp' YUV 4:2:0 conversion\n"
    "                       3: use YUV 4:4:4 (full resolution for U/V planes)\n"
    "  -no_limit .......... If true, allow the quality factor to be larger\n"
    "                       than the original (JPEG input only).\n"
    "  -no_optim .......... Don't use Huffman optimization (=faster)\n"
    "  -no_adapt .......... Don't use adaptive quantization (=faster)\n"
    "  -trellis ........... use trellis-based quantization (=slower)\n"
    "  -no_metadata ....... Ignore metadata from the source.\n"
    "  -pass <int> ........ number of passes for -size or -psnr (default: 10\n"
    "  -qmin <float> ...... minimum acceptable quality factor during search\n"
    "  -qmax <float> ...... maximum acceptable quality factor during search\n"
    "  -tolerance <float> . tolerance for convergence during search\n"
    "\n"
    "  -444 ............... shortcut for '-yuv_mode 3'\n"
    "  -sharp ............. shortcut for '-yuv_mode 2'\n"
    "  -420 ............... shortcut for '-yuv_mode 1'\n"
    "\n"
    "If the input format is JPEG, the recompression will not go beyond the\n"
    "original quality, *except* if '-no_limit' option is used."
    "\n";

  // in order to gather information, plug a search hook
  ALT_HOOK_CLASS hook;
  param.search_hook = &hook;

  // parse command line
  if (argc <= 1) {
    fprintf(stderr, usage);
    return 0;
  }
  for (int c = 1; c < argc; ++c) {
    if (!strcmp(argv[c], "-h") || !strcmp(argv[c], "--help")) {
      fprintf(stdout, usage);
      return 0;
    } else if (!strcmp(argv[c], "-o") && c + 1 < argc) {
      output_file = argv[++c];
    } else if (!strcmp(argv[c], "-q") && c + 1 < argc) {
      quality = atof(argv[++c]);
      use_reduction = false;
      if (quality < 0 || quality > 100) {
        fprintf(stdout, "Error: invalid range for option '%s': %s\n",
                argv[c - 1], argv[c]);
        return 1;
      }
    } else if (!strcmp(argv[c], "-r") && c + 1 < argc) {
      reduction = atof(argv[++c]);
      use_reduction = true;
      if (reduction <= 0 || reduction > 100) {
        fprintf(stdout, "Error: invalid range for option '%s': %s\n",
                argv[c - 1], argv[c]);
        return 1;
      }
    } else if (!strcmp(argv[c], "-estimate")) {
      estimate = true;
    } else if (!strcmp(argv[c], "-no_limit")) {
      limit_quantization = false;
    } else if (!strcmp(argv[c], "-no_adapt")) {
      param.adaptive_quantization = false;
    } else if (!strcmp(argv[c], "-no_optim")) {
      param.Huffman_compress = false;
    } else if (!strcmp(argv[c], "-adapt_bias")) {
      param.adaptive_bias = true;
    } else if (!strcmp(argv[c], "-trellis")) {
      param.use_trellis = true;
    } else if (!strcmp(argv[c], "-psnr") && c + 1 < argc) {
      param.target_mode = EncoderParam::TARGET_PSNR;
      param.target_value = atof(argv[++c]);
    } else if (!strcmp(argv[c], "-tolerance")) {
      param.tolerance = atof(argv[++c]);
    } else if (!strcmp(argv[c], "-qmin")) {
      param.qmin = atof(argv[++c]);
    } else if (!strcmp(argv[c], "-qmax")) {
      param.qmax = atof(argv[++c]);
    } else if (!strcmp(argv[c], "-size") && c + 1 < argc) {
      param.target_mode = EncoderParam::TARGET_SIZE;
      param.target_value = atof(argv[++c]);
    } else if (!strcmp(argv[c], "-pass") && c + 1 < argc) {
      param.passes = atoi(argv[++c]);
    } else if (!strcmp(argv[c], "-no_metadata")) {
      no_metadata = true;
    } else if (!strcmp(argv[c], "-yuv_mode") && c + 1 < argc) {
      const int mode = atoi(argv[++c]);
      if (mode < 0 || mode > 3) {
        fprintf(stdout, "Error: invalid range for option '%s': %s\n",
                argv[c - 1], argv[c]);
        return 1;
      }
      param.yuv_mode = (SjpegYUVMode)mode;
    } else if (!strcmp(argv[c], "-444")) {
      param.yuv_mode = SJPEG_YUV_444;
    } else if (!strcmp(argv[c], "-sharp")) {
      param.yuv_mode = SJPEG_YUV_SHARP;
    } else if (!strcmp(argv[c], "-420")) {
      param.yuv_mode = SJPEG_YUV_420;
    } else if (!strcmp(argv[c], "-i") || !strcmp(argv[c], "-info")) {
      info = true;
    } else if (!strcmp(argv[c], "-quiet")) {
      quiet = true;
    } else if (!strcmp(argv[c], "-short")) {
      short_output = true;
    } else if (!strcmp(argv[c], "-crc")) {
      print_crc = true;
    } else if (!strcmp(argv[c], "-version")) {
      const uint32_t version = SjpegVersion();
      fprintf(stdout, "%d.%d.%d\n",
              (version >> 16) & 0xff,
              (version >>  8) & 0xff,
              (version >>  0) & 0xff);
      return 0;
    } else {
      input_file = argv[c];
    }
  }
  if (input_file == nullptr) {
    fprintf(stderr, "Missing input file.\n");
    if (!quiet) fprintf(stderr, usage);
    return -1;
  }
  // finish param set up
  const bool use_search = (param.target_mode != EncoderParam::TARGET_NONE);
  if (use_search && param.passes <= 1) {
    param.passes = 10;
  }
  // Read input file into the buffer in_bytes[]
  std::string input = ReadFile(input_file);
  if (input.size() == 0) return 1;

  const ImageType input_type = GuessImageType(input);
  uint8_t quant_matrices[2][64];
  const int nb_matrices =
    (input_type == SJPEG_JPEG) ? SjpegFindQuantizer(input, quant_matrices)
                               : 0;
  const bool is_jpeg = (input_type == SJPEG_JPEG) && (nb_matrices > 0);
  if (use_reduction && !is_jpeg) {
    if (!quiet && !short_output) {
      fprintf(stdout, "Warning! reduction factor (-r option) disabled"
                      " (only applies to JPEG source).\n");
      fprintf(stdout, "         Please use the -q option to set the"
                      " quality factor.\n\n");
    }
    use_reduction = false;
  }
  if (use_reduction) {   // use 'reduction' factor for JPEG source
    param.SetQuantization(quant_matrices, reduction);
    param.SetLimitQuantization(true);
  } else {    // the '-q' option has been used.
    param.SetQuality(quality);
    if (is_jpeg) {
      param.SetMinQuantization(quant_matrices);
    } else {
      param.SetLimitQuantization(false);
    }
  }

  if (estimate) {
    const int q = is_jpeg ? SjpegEstimateQuality(quant_matrices[0], 0) : 100;
    fprintf(stdout, "%d\n", q);
    return 0;
  }
  int W, H;
  vector<uint8_t> in_bytes = ReadImage(input, &W, &H, &param);
  if (in_bytes.size() == 0) return 1;

  if (!short_output && !quiet && !print_crc) {
    fprintf(stdout, "Input [%s]: %s (%u bytes, %d x %d)\n",
            ImageTypeName(input_type), input_file,
            static_cast<uint32_t>(input.size()),
            W, H);
    if (info) {
      yuv_mode_rec = SjpegRiskiness(&in_bytes[0], W, H, 3 * W, &riskiness);
      fprintf(stdout, "Riskiness:   %.1f (recommended yuv_mode: %s)\n",
              riskiness, kYUVModeNames[yuv_mode_rec]);

      if (is_jpeg) {
        fprintf(stdout, "Input is JPEG w/ %d matrices:\n", nb_matrices);
        if (nb_matrices > 0) {
          PrintMatrix("Luma", quant_matrices[0], false);
        }
        if (nb_matrices > 1) {
          PrintMatrix("UV-chroma", quant_matrices[1], true);
        }
      }
      PrintMetadataInfo(param);
    }
  }
  if (info && !print_crc) return 0;   // done

  // finish setting up the quantization matrices
  if (limit_quantization == false) param.SetLimitQuantization(false);

  if (no_metadata) param.ResetMetadata();

  const double start = GetStopwatchTime();
  std::string out;
  const bool ok = sjpeg::Encode(&in_bytes[0], W, H, 3 * W, param, &out);
  const double encode_time = GetStopwatchTime() - start;

  if (!ok) {
    fprintf(stderr, "ERROR: call to sjpeg::Encode() failed.\n");
    return -1;
  }

  if (print_crc) {
    printf("0x%.8x\n", GetCRC32(out));
    return 0;
  }

  if (!short_output && !quiet) {
    const bool show_reduction = use_reduction && !use_search;
    yuv_mode_rec = SjpegRiskiness(&in_bytes[0], W, H, 3 * W, &riskiness);
    fprintf(stdout, "new size:    %u bytes (%.2lf%% of original)\n"
                    "%s%.1f (adaptive: %s, Huffman: %s)\n"
                    "yuv mode:    %s (riskiness: %.1lf%%)\n"
                    "elapsed:     %d ms\n",
                    static_cast<uint32_t>(out.size()),
                    100. * out.size() / input.size(),
                    show_reduction ? "reduction:   r=" : "quality:     q=",
                    show_reduction ? reduction : quality,
                    kNoYes[param.adaptive_quantization],
                    kNoYes[param.Huffman_compress],
                    kYUVModeNames[yuv_mode_rec], riskiness,
                    static_cast<int>(1000. * encode_time));
    if (use_search) {  // print final values
      fprintf(stdout, "passes:      %d\n", hook.pass + 1);
      fprintf(stdout, "final value: %.1f\n", hook.value);
      fprintf(stdout, "final q:     %.2f\n", hook.q);
    }
    PrintMetadataInfo(param);
  } else if (!quiet) {
    fprintf(stdout, "%u %u %.2lf %%\n",
            static_cast<uint32_t>(input.size()),
            static_cast<uint32_t>(out.size()),
            100. * out.size() / input.size());
  }

  // Save the result.
  if (output_file != nullptr && !SaveFile(output_file, out, quiet)) return 1;

  return 0;     // ok.
}

///////////////////////////////////////////////////////////////////////////////

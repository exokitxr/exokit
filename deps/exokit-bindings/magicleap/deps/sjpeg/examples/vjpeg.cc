// Copyright 2017 Google, Inc.
//
//  SJPEG visualization tool
//
// usage: vjpeg input.png [-q quality]
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

#if defined(__unix__) || defined(__CYGWIN__)
#define _POSIX_C_SOURCE 200112L  // for setenv
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <vector>
using std::vector;

#if defined(SJPEG_HAVE_OPENGL) && \
    (defined(HAVE_GLUT_GLUT_H) || defined(HAVE_GL_GLUT_H))
#if defined(HAVE_GLUT_GLUT_H)
#include <GLUT/glut.h>
#elif defined(HAVE_GL_GLUT_H)
#include <GL/glut.h>
#else
#error "GLUT is not supported."
#endif
#ifdef FREEGLUT
#include <GL/freeglut.h>
#endif

#include "./utils.h"

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif

// Unfortunate global variables. Gathered into a struct for comfort.
struct Params {
  ImageType type;
  int done;
  bool error;
  int show;  // 0 = compressed, 1 = original, 2 = info, 3 = help
             // 4 = error map, 5 = riskiness map, 6 = alt
  int fade;  // [0..100]

  size_t current_file;
  std::vector<std::string> files;
  std::string input;     // input file
  std::string jpeg;      // currently encoded file
  float estimated_quality;

  float riskiness;
  SjpegYUVMode yuv_mode_rec;
  uint32_t elapsed;      // encoding time in ms
  float quality;
  uint8_t quant[2][64];
  bool limit_quantization;
  sjpeg::EncoderParam param;
  vector<uint8_t> rgb;       // original samples
  vector<uint8_t> out_rgb;   // recompressed samples
  vector<uint8_t> map;       // error map
  vector<uint8_t> alt;       // alternate comparison picture
  size_t alt_size;           // size of the alternate picture
  int width, height;
  int is_yuv420;
  int viewport_width, viewport_height;

  Params() : current_file(~0u) {
    limit_quantization = true;
    quality = 75;
  }
  bool SetCurrentFile(size_t file_number);
  bool SetAltFile(const char* const file_name);
};
static Params kParams;

//------------------------------------------------------------------------------
// Message printing

static void PrintString(const std::string& text) {
  void* const font = GLUT_BITMAP_9_BY_15;
  for (size_t i = 0; i < text.size(); ++i) glutBitmapCharacter(font, text[i]);
}

static void PrintMessages(const vector<std::string>& msg, float color[4],
                          bool draw_background = false) {
  if (msg.empty()) return;

  const float pix_incr_x = 2.f / kParams.viewport_width;
  const float pix_incr_y = 2.f / kParams.viewport_height;
  const float line_height = 19 * pix_incr_y;
  const float line_start = 1.f - 5 * pix_incr_y - line_height;
  const float left_start = -1. + 10 * pix_incr_x;

  if (draw_background) {
    const float y_top = line_start + line_height;
    const float x_left = left_start - 5 * pix_incr_x;
    float HY = msg.size() * line_height;
    float HX = 0;
    for (size_t i = 0; i < msg.size(); ++i) {
     if (msg[i].size() > HX) HX = msg[i].size();
    }
    HX *= 9 * pix_incr_x;
    HX += 10 * pix_incr_x;
    HY += 10 * pix_incr_x;
    glColor4f(.4, .6, .8, (color[3] > 0.6) ? 0.6 : color[3]);
    glRectf(x_left, y_top, x_left + HX, y_top - HY);
  }

  // draw a black outline
  glColor4f(0., 0., 0., color[3]);
  for (int dy = -1; dy <= 1; dy += 2) {
    for (int dx = -1; dx <= 1; dx += 2) {
      for (size_t i = 0; i < msg.size(); ++i) {
        const float position = line_start - i * line_height;
        glRasterPos2f(left_start + dx * pix_incr_x, position + dy * pix_incr_y);
        PrintString(msg[i]);
      }
    }
  }
  // draw the text
  glColor4f(color[0], color[1], color[2], color[3]);
  for (size_t i = 0; i < msg.size(); ++i) {
    const float position = line_start - i * line_height;
    glRasterPos2f(left_start, position);
    PrintString(msg[i]);
  }
}

static const int kFadeMax = 100;
static const int kFadeDecr = 5;
static const int kFadeMs = 3000 / kFadeDecr;

static void PrintInfo() {
  vector<std::string> msg;
  float color[4] = { 0.90f, 0.90f, 0.90f, 1.0f };

  if (kParams.show == 3) {
    msg.push_back("Keyboard shortcuts:");
    msg.push_back("  'i' ............... overlay file information");
    msg.push_back(
        "  ' ' ............... show the original uncompressed picture");
    msg.push_back(
        "  up/down ........... change the compression factor by +/- 1 units");
    msg.push_back(
        "  left/right ........ change the compression factor by +/- 10 units");
    msg.push_back(
        "  return ............ show alternate picture (if specified)");
    msg.push_back("  0/1/2/3 ........... change the yuv_mode to ");
    msg.push_back("                      auto/yuv420/sharp-yuv420/yuv444");
    msg.push_back("  'o' ............... toggle Huffman optimization");
    msg.push_back("  'a' ............... toggle adaptive quantization");
    msg.push_back("  'b' ............... toggle adaptive bias");
    msg.push_back("  'l' ............... toggle quantization limitation");
    msg.push_back("  't' ............... toggle trellis-based limitation");
    msg.push_back("  'e' ............... show error map");
    msg.push_back("  'r' ............... show riskiness map");
    msg.push_back("  '+'/'-' ........... go to next/previous file");
    msg.push_back("  'h' ............... show this help message");
    msg.push_back("  'q' / 'Q' / ESC ... quit");
  } else if (kParams.show == 1) {
    msg.push_back("- O R I G I N A L -");
  } else if (kParams.show == 2) {
    char tmp[80];

    color[0] = 0.99f;
    color[1] = 0.99f;
    color[2] = 0.30f;
    color[3] = sqrt(1.f * kParams.fade / kFadeMax);

    msg.push_back(kParams.files[kParams.current_file]);

    snprintf(tmp, sizeof(tmp), "Dimension: %d x %d%s",
             kParams.width, kParams.height,
             kParams.is_yuv420 ? " (yuv420)" : "");
    msg.push_back(tmp);
    if (kParams.type == SJPEG_JPEG) {
      snprintf(tmp, sizeof(tmp), "  (estimated quality: %.1f)",
               kParams.estimated_quality);
      msg.back() += tmp;
    }

    snprintf(tmp, sizeof(tmp), "Quality: %.1f", kParams.quality);
    msg.push_back(tmp);

    if (!kParams.param.Huffman_compress) {
      msg.back() += " (no Huffman optim)";
    }
    if (!kParams.param.adaptive_quantization) {
      msg.back() += " (no adaptive quantization)";
    }
    if (kParams.param.adaptive_bias) {
      msg.back() += " (adaptive bias)";
    }
    if (kParams.param.use_trellis) {
      msg.back() += " (trellis)";
    }

    snprintf(tmp, sizeof(tmp), "Size: %ld [%.2f bpp] (%u ms)",
             static_cast<long int>(kParams.jpeg.size()),
             8.f * kParams.jpeg.size() / (kParams.width * kParams.height),
             kParams.elapsed);
    msg.push_back(tmp);
    if (kParams.type == SJPEG_JPEG) {
      snprintf(tmp, sizeof(tmp), " (%.1f%% of original)",
               100. * kParams.jpeg.size() / kParams.input.size());
      msg.back() += tmp;
    }

    const char* kYUVModeStrings[4] =
        { "Auto", "YUV420", "Sharp-YUV420", "YUV444" };
    snprintf(tmp, sizeof(tmp), "YUV-mode: %s",
             kYUVModeStrings[kParams.param.yuv_mode]);
    msg.push_back(tmp);

    snprintf(tmp, sizeof(tmp), " (riskiness: %.2f suggested YUV-mode: %s)",
             kParams.riskiness, kYUVModeStrings[kParams.yuv_mode_rec]);
    msg.push_back(tmp);
  } else if (kParams.show == 4) {
    msg.push_back("- E R R O R   M A P -");
  } else if (kParams.show == 5) {
    msg.push_back("- Riskiness Map -");
  } else if (kParams.show == 6) {
    char tmp[80];
    snprintf(tmp, sizeof(tmp), "- Alt Pic (%ld bytes) -",
             static_cast<long int>(kParams.alt_size));
    msg.push_back(tmp);
  }

  PrintMessages(msg, color, kParams.show != 1);
}

//------------------------------------------------------------------------------
// Timer callbacks

static void StopPrintInfo(int what) {
  // drop this timer if a newer one is in-flight
  if (what < kParams.fade) return;

  if (what == 0) {
    kParams.show = 0;
  } else {
    what -= kFadeDecr;
    if (what < 0) what = 0;
    kParams.fade = what;
    glutTimerFunc(kFadeMs, StopPrintInfo, kParams.fade);
    PrintInfo();
  }
  glutPostRedisplay();
}

static void SetPrintInfoTimer() {
  kParams.fade = kFadeMax;
  glutTimerFunc(kFadeMs, StopPrintInfo, kParams.fade);
  glutPostRedisplay();
}

//------------------------------------------------------------------------------
// error maps

static void ComputeErrorMap() {
  if (kParams.show != 4) {
    const int stride = 3 * kParams.width;
    kParams.map.resize(stride * kParams.height);
    uint8_t* dst = &kParams.map[0];
    const uint8_t* src1 = &kParams.rgb[0];
    const uint8_t* src2 = &kParams.out_rgb[0];
    for (int j = 0; j < kParams.height; ++j) {
      for (int i = 0; i < stride; ++i) {
        const uint32_t err = 4 * abs(src1[i] - src2[i]);
        dst[i] = (err > 255) ? 255 : err;
      }
      dst += stride;
      src1 += stride;
      src2 += stride;
    }
    kParams.show = 4;
  }
}

namespace sjpeg {
// undocumented function
extern double BlockRiskinessScore(const uint8_t* rgb, int stride,
                                  int16_t score[8 * 8]);
}

static void ComputeRiskinessMap() {
  if (kParams.show != 5) {
    const size_t width = kParams.width;
    const size_t height = kParams.height;
    const size_t stride = 3 * width;
    kParams.map.resize(stride * height);

    uint8_t* dst = &kParams.map[0];
    const uint8_t* src = &kParams.rgb[0];
    // We use 7x7 block iteration to avoid the oddity at right and bottom
    // of the 8x8 block returned by BlockRiskinessScore().
    for (size_t j = 0; j + 7 <= height; j += 7) {
      for (size_t i = 0; i + 7 <= width; i += 7) {
        int16_t score_8x8[8 * 8];
        sjpeg::BlockRiskinessScore(src + i * 3, stride, score_8x8);
        for (size_t k = 0, J = 0; J < 7; ++J) {
          for (size_t I = 0; I < 7; ++I, ++k) {
            int score = score_8x8[I + J * 8];
            score *= 2;
            if (score > 255) score = 255;
            const size_t off = 3 * (i + I) + J * stride;
            dst[off + 0] = dst[off + 1] = dst[off + 2] = score;
          }
        }
      }
      dst += 7 * stride;
      src += 7 * stride;
    }
    kParams.show = 5;
  }
}

//------------------------------------------------------------------------------
// compression

static bool EncodeAndDecode() {
  const double start = GetStopwatchTime();
  kParams.param.SetQuality(kParams.quality);
  kParams.param.SetLimitQuantization(kParams.limit_quantization);
  if (!sjpeg::Encode(&kParams.rgb[0],
                     kParams.width, kParams.height, kParams.width * 3,
                     kParams.param, &kParams.jpeg)) {
    fprintf(stderr, "Encoding error!\n");
    kParams.error = true;
    return false;
  }
  kParams.elapsed = static_cast<uint32_t>(1000. * (GetStopwatchTime() - start));
  kParams.out_rgb = ReadJPEG(kParams.jpeg, NULL, NULL, NULL);
  assert(kParams.jpeg.size() > 0);
  assert(kParams.out_rgb.size() > 0);
  if (kParams.show == 4) {
    kParams.show = 0;  // force recomputation
    ComputeErrorMap();
  } else if (kParams.show == 5) {
    kParams.show = 0;  // force recomputation
    ComputeRiskinessMap();
  }
  return true;
}

static void FullRedraw() {
  EncodeAndDecode();
  SetPrintInfoTimer();
}

bool Params::SetCurrentFile(size_t file) {
  if (file >= files.size()) file = files.size() - 1;
  if (file != current_file) {
    current_file = file;
    const std::string& file_name = files[file];
    input = ReadFile(file_name.c_str());
    error = input.empty();
    if (error) return false;

    type = GuessImageType(input);
    rgb = ReadImage(input, &width, &height, &param);
    error = rgb.empty();
    if (error) {
      fprintf(stderr, "Could not decode the input file %s\n",
              file_name.c_str());
      return false;
    }

    if (type == SJPEG_JPEG) {
      if (SjpegFindQuantizer(input, quant)) {
        estimated_quality = SjpegEstimateQuality(quant[0], false);
      }
      int w, h;
      if (!SjpegDimensions(input, &w, &h, &is_yuv420)) {
        fprintf(stderr, "Could not retrieve dimensions\n");
        return false;
      }
      assert(w == width && h == height);
    }
    yuv_mode_rec =
        SjpegRiskiness(&rgb[0], width, height, 3 * width, &riskiness);
    EncodeAndDecode();
  }
  return !error;
}

bool Params::SetAltFile(const char* const file_name) {
  alt.clear();
  alt_size = 0;
  const std::string data = ReadFile(file_name);
  if (data.empty()) return false;
  int w, h;
  alt = ReadImage(data, &w, &h, NULL);
  if (alt.empty()) {
    fprintf(stderr, "Could not decode the alternate file %s\n",
            file_name);
    return false;
  }
  if (w != width || h != height) {
    alt.clear();
    fprintf(stderr, "Alternate picture has incompatible dimensions "
                    " (%dx%d vs expected %dx%d)\n",
            w, h, width, height);
    return false;
  }
  alt_size = data.size();
  return true;
}

//------------------------------------------------------------------------------

void PrintMatrix(const char name[], const uint8_t m[64], bool for_chroma) {
  printf(" %s quantization matrix (estimated quality: %.1f)\n",
         name, SjpegEstimateQuality(m, for_chroma));
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) printf("%3d ", m[i + j * 8]);
    printf("\n");
  }
  printf("------\n");
}

void PrintMatrices() {
  uint8_t quants[2][64];
  const int nb = SjpegFindQuantizer(kParams.jpeg, quants);
  if (nb == 0) return;
  PrintMatrix("- Luma -", quants[0], false);
  if (nb > 1) PrintMatrix("- Chroma -", quants[1], true);
}

//------------------------------------------------------------------------------
// Handlers

static void HandleKey(unsigned char key, int pos_x, int pos_y) {
  (void)pos_x;
  (void)pos_y;
  if (key == 'q' || key == 'Q' || key == 27 /* Esc */) {
#ifdef FREEGLUT
    glutLeaveMainLoop();
#else
    exit(0);
#endif
  } else if (key == ' ') {
    kParams.show = 1;
    glutPostRedisplay();
  } else if (key == 13) {  // return
    if (!kParams.alt.empty()) {
      kParams.show = 6;
      glutPostRedisplay();
    }
  } else if (key == 'E' || key == 'e') {
    ComputeErrorMap();
    glutPostRedisplay();
  } else if (key == 'R' || key == 'r') {
    ComputeRiskinessMap();
    glutPostRedisplay();
  } else if (key == '+') {
    if (kParams.current_file + 1 < kParams.files.size()) {
      kParams.SetCurrentFile(kParams.current_file + 1);
      kParams.show = 0;
      glutReshapeWindow(kParams.width, kParams.height);
      glutPostRedisplay();
    }
  } else if (key == '-') {
    if (kParams.current_file > 0) {
      kParams.SetCurrentFile(kParams.current_file - 1);
      kParams.show = 0;
      glutReshapeWindow(kParams.width, kParams.height);
      glutPostRedisplay();
    }
  } else if (key == 'I' || key == 'i') {
    kParams.fade = kFadeMax;
    kParams.show = 2;
    glutPostRedisplay();
  } else if (key == 'h') {
    kParams.fade = kFadeMax;
    kParams.show = 3;
    glutPostRedisplay();
  } else if (key >= '0' && key <= '3') {
    static const SjpegYUVMode kMap[] = {
      SJPEG_YUV_AUTO, SJPEG_YUV_420, SJPEG_YUV_SHARP, SJPEG_YUV_444
    };
    kParams.param.yuv_mode = kMap[key - '0'];
    FullRedraw();
  } else if (key == 'o') {
    kParams.param.Huffman_compress = !kParams.param.Huffman_compress;
    FullRedraw();
  } else if (key == 'a') {
    kParams.param.adaptive_quantization = !kParams.param.adaptive_quantization;
    FullRedraw();
  } else if (key == 'b') {
    kParams.param.adaptive_bias = !kParams.param.adaptive_bias;
    FullRedraw();
  } else if (key == 'l') {
    kParams.limit_quantization = !kParams.limit_quantization;
    FullRedraw();
  } else if (key == 't') {
    kParams.param.use_trellis = !kParams.param.use_trellis;
    FullRedraw();
  } else if (key == 'm') {
    PrintMatrices();
  }
}

static void HandleKeyUp(unsigned char key, int pos_x, int pos_y) {
  (void)pos_x;
  (void)pos_y;
  if (key == ' ') {
    kParams.show = 0;
    glutPostRedisplay();
  } else if (key == 13) {
    kParams.show = 0;
    glutPostRedisplay();
  } else if (key == 'E' || key == 'e' || key == 'R' || key == 'r') {
    kParams.show = 0;
    kParams.map.clear();
    glutPostRedisplay();
  } else if (key == 'i' || key == 'I') {
    kParams.show = 2;
    SetPrintInfoTimer();
  }
}

static void SetQuality(int incr) {
  float q = kParams.quality + incr;
  if (q < 0) q = 0;
  else if (q > 100) q = 100;
  if (kParams.quality == q) return;
  kParams.quality = q;
  FullRedraw();
}

static void HandleSpecialKeys(int key, int pos_x, int pos_y) {
  (void)pos_x;
  (void)pos_y;
  switch (key) {
    default: return;
    case GLUT_KEY_UP: SetQuality(1); break;
    case GLUT_KEY_DOWN: SetQuality(-1); break;
    case GLUT_KEY_RIGHT: SetQuality(10); break;
    case GLUT_KEY_LEFT: SetQuality(-10); break;
  }
}

static void HandleReshape(int width, int height) {
  // TODO(skal): should we preserve aspect ratio?
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  kParams.viewport_width = width;
  kParams.viewport_height = height;
}


static void HandleDisplay(void) {
  if (kParams.out_rgb.size() == 0) return;
  glPushMatrix();
  glPixelZoom((GLfloat)(+1. / kParams.width * kParams.viewport_width),
              (GLfloat)(-1. / kParams.height * kParams.viewport_height));
  glRasterPos2f(-1.f, 1.f);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, kParams.width);

  const uint8_t* src;
  if (kParams.show == 1) {
    src = &kParams.rgb[0];
  } else if (kParams.show == 4 || kParams.show == 5) {
    src = &kParams.map[0];
  } else if (kParams.show == 6 && !kParams.alt.empty()) {
    src = &kParams.alt[0];
  } else {
    src = &kParams.out_rgb[0];
  }
  glDrawPixels(kParams.width, kParams.height,
               GL_RGB, GL_UNSIGNED_BYTE,
               reinterpret_cast<GLvoid*>(const_cast<uint8_t*>(src)));
  kParams.map.clear();
  PrintInfo();
  glPopMatrix();
  glutSwapBuffers();
}

static void StartDisplay(void) {
  const int width = kParams.width;
  const int height = kParams.height;
  const int swidth = glutGet(GLUT_SCREEN_WIDTH);
  const int sheight = glutGet(GLUT_SCREEN_HEIGHT);
//  printf("%d x %d  vs  %d x %d\n", width, height, swidth, sheight);
  (void)swidth;
  (void)sheight;
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(width, height);
  glutCreateWindow("SimpleJPEG viewer");
  glutDisplayFunc(HandleDisplay);
  glutReshapeFunc(HandleReshape);
  glutIdleFunc(NULL);
  glutKeyboardFunc(HandleKey);
  glutKeyboardUpFunc(HandleKeyUp);
  glutSpecialFunc(HandleSpecialKeys);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(0., 0., 0., 1.);
  glClear(GL_COLOR_BUFFER_BIT);
}

//------------------------------------------------------------------------------
// Main

static void Help(void) {
  printf("Usage: vjpeg in_file [options]\n\n"
         "Visualizer for SJPEG (re-)compression, using OpenGL\n"
         "Options are:\n"
         "  -q quality ........ Quality factor in [0..100] range.\n"
         "                      Value of 100 gives the best quality\n"
         "                      Default value is 75.\n"
         "  -version .......... print version number and exit\n"
         "  -info ............. print info overlay\n"
         "  -h ................ this help message\n"
         "\n"
         "Keyboard shortcuts:\n"
         "  'i' ............... overlay file information\n"
         "  ' ' ............... show the original uncompressed picture\n"
         "  up/down ........... change the compression factor by +/- 1 units\n"
         "  left/right ........ change the compression factor by +/- 10 units\n"
         "  return ............ show alternate picture (if specified)\n"
         "  0/1/2/3 ........... change the yuv_mode to "
                                "auto/yuv420/sharp-yuv420/yuv444\n"
         "  'o' ............... toggle Huffman optimization\n"
         "  'a' ............... toggle adaptive quantization\n"
         "  'l' ............... toggle quantization limitation\n"
         "  't' ............... toggle trellis-based quantization\n"
         "  'e' ............... show error map\n"
         "  'r' ............... show riskiness map\n"
         "  '+'/'-' ........... go to next/previous file\n"
         "  'm' ............... print the output quantization matrices\n"
         "  'h' ............... show this help message\n"
         "  'q' / 'Q' / ESC ... quit\n"
        );
}

int main(int argc, char *argv[]) {
  const char* alt_name = NULL;
  for (int c = 1; c < argc; ++c) {
    int parse_error = 0;
    if (!strcmp(argv[c], "-h") || !strcmp(argv[c], "-help")) {
      Help();
      return 0;
    } else if (!strcmp(argv[c], "-info")) {
      kParams.show = 2;
    } else if (!strcmp(argv[c], "-version")) {
      const uint32_t version = SjpegVersion();
      printf("SJPEG version: %d.%d.%d\n",
             (version >> 16) & 0xff, (version >> 8) & 0xff,
             (version >>  0) & 0xff);
      return 0;
    } else if (!strcmp(argv[c], "-alt")) {
      if (c < argc - 1) alt_name = argv[++c];
    } else if (!strcmp(argv[c], "--")) {
      if (c < argc - 1) kParams.files.push_back(argv[++c]);
    } else if (!strcmp(argv[c], "-q")) {
      if (c < argc - 1) kParams.quality = atof(argv[++c]);
    } else if (argv[c][0] == '-') {
      printf("Unknown option '%s'\n", argv[c]);
      parse_error = 1;
    } else {
      kParams.files.push_back(argv[c]);
    }

    if (parse_error) {
      Help();
      return -1;
    }
  }

  if (kParams.files.empty()) {
    printf("missing input file(s)!!\n");
    Help();
    return 0;
  }

  kParams.fade = kFadeMax;
  if (!kParams.SetCurrentFile(0)) return 1;

  if (alt_name != NULL) kParams.SetAltFile(alt_name);

#if defined(__unix__) || defined(__CYGWIN__)
  // Work around GLUT compositor bug.
  // https://bugs.launchpad.net/ubuntu/+source/freeglut/+bug/369891
  setenv("XLIB_SKIP_ARGB_VISUALS", "1", 1);
#endif

  // Start display (and timer)
  glutInit(&argc, argv);
#ifdef FREEGLUT
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
#endif
  StartDisplay();

  glutMainLoop();

  // Should only be reached when using FREEGLUT:
  return 0;
}

#else   // !SJPEG_HAVE_OPENGL

int main(int argc, const char *argv[]) {
  fprintf(stderr, "OpenGL support not enabled in %s.\n", argv[0]);
  (void)argc;
  return 0;
}

#endif

//------------------------------------------------------------------------------

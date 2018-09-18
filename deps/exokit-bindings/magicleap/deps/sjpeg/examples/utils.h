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

#ifndef SJPEG_EXAMPLES_UTILS_H_
#define SJPEG_EXAMPLES_UTILS_H_

#include <string>
#include <vector>
#include "sjpeg.h"

// reads a disk file
extern std::string ReadFile(const char filename[]);
// save data to disk file
extern int SaveFile(const char* name, const std::string& out, bool quiet);

// returns the current time in seconds.
extern double GetStopwatchTime();

// try to systematically decode the input as a JPEG, PNG, PPM (in this order)
extern std::vector<uint8_t> ReadImage(const std::string& in,
                                      int* const width, int* const height,
                                      sjpeg::EncoderParam* const param);

// Return CRC32 signature for data block. 'crc' is the current checksum value.
extern uint32_t GetCRC32(const std::string& data, uint32_t crc = 0);

///////////////////////////////////////////////////////////////////////////////
// guessed image types

typedef enum { SJPEG_UNKNOWN = 0,
               SJPEG_JPEG,
               SJPEG_PNG,
               SJPEG_PPM } ImageType;

// returns a printable 3-letter name for the image type
extern const char* ImageTypeName(ImageType type);

// try to guess the image format based on few header bytes.
// Even if SJPEG_UNKNOWN is returned, some of the ReadXXX functions below
// can succeed reading the file.
extern ImageType GuessImageType(const std::string& input);

// signature for the function reading a image format
typedef std::vector<uint8_t> (*ImageReader)(const std::string& in,
                                            int* const width,
                                            int* const height,
                                            sjpeg::EncoderParam* const param);

// returns a callable function associated with the detected image type.
extern ImageReader GuessImageReader(const std::string& input);

// quickly try to guess the image format and read it.
extern std::vector<uint8_t> ReadImageQuick(const std::string& in,
                                           int* const width, int* const height,
                                           sjpeg::EncoderParam* const param);

// Directly callable functions, in case auto-detection didn't work.
extern std::vector<uint8_t> ReadJPEG(const std::string& in,
                                     int* const width, int* const height,
                                     sjpeg::EncoderParam* const param);
extern std::vector<uint8_t> ReadPNG(const std::string& input,
                                    int* const width_ptr, int* const height_ptr,
                                    sjpeg::EncoderParam* const param);
extern std::vector<uint8_t> ReadPPM(const std::string& input,
                                    int* const width, int* const height,
                                    sjpeg::EncoderParam* const param);

#endif  /* SJPEG_EXAMPLES_UTILS_H_ */

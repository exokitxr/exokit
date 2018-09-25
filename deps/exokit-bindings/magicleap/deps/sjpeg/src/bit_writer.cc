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
//  Utility for writing bits
//
// Author: Skal (pascal.massimino@gmail.com)

#include "bit_writer.h"

#include <string.h>

#include "sjpeg.h"

namespace sjpeg {

///////////////////////////////////////////////////////////////////////////////
// MemorySink

MemorySink::MemorySink(size_t expected_size)
    : buf_(nullptr), pos_(0), max_pos_(0) {
  // The following call can fail, with no harm: it'll be reported during the
  // next call to Commit() if needed (we don't want to fail in a constructor).
  (void)Commit(0, expected_size, &buf_);
}

MemorySink::~MemorySink() { Reset(); }

void MemorySink::Reset() {
  delete[] buf_;
  buf_ = nullptr;
  pos_ = 0;
  max_pos_ = 0;
}

void MemorySink::Release(uint8_t** buf_ptr, size_t* size_ptr) {
  *buf_ptr = buf_;
  *size_ptr = pos_;
  buf_ = nullptr;
  Reset();
}

bool MemorySink::Commit(size_t used_size, size_t extra_size, uint8_t** data) {
  pos_ += used_size;
  assert(pos_ <= max_pos_);
  size_t new_size = pos_ + extra_size;
  if (new_size > max_pos_) {
  // TODO(skal): the x2 growth is probably over-shooting. Need to tune
  // depending on use-case (ie.: what is the expected average final size?)
    new_size += 256;
    if (new_size < 2 * max_pos_) {
      new_size = 2 * max_pos_;
  }
    uint8_t* const new_buf = new (std::nothrow) uint8_t[new_size];
    if (new_buf == nullptr) return false;

    if (pos_ > 0) memcpy(new_buf, buf_, pos_);
  delete[] buf_;
  buf_ = new_buf;
    max_pos_ = new_size;
  }
  *data = buf_ + pos_;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Sink factories

std::shared_ptr<ByteSink> MakeByteSink(std::string* const output) {
  return std::shared_ptr<ByteSink>(new (std::nothrow) StringSink(output));
}

// specialization for vector<uint8_t>
template<>
std::shared_ptr<ByteSink> MakeByteSink(std::vector<uint8_t>* const output) {
  return std::shared_ptr<ByteSink>(new (std::nothrow) VectorSink(output));
}

///////////////////////////////////////////////////////////////////////////////
// BitWriter

BitWriter::BitWriter(ByteSink* const sink) : sink_(sink), buf_(nullptr) {
  nb_bits_ = 0;
  bits_ = 0x00000000U;
  byte_pos_ = 0;
}

void BitWriter::Flush() {
  // align and pad the bitstream
  // nb_pad is the number of '1' bits we need to insert to reach a byte-aligned
  // position. So nb_pad is 1,2..,7 when nb_bits_= 7,...2,1
  const int nb_pad = (-nb_bits_) & 7;
  if (nb_pad) {
    PutBits((1 << nb_pad) - 1, nb_pad);
  }
  FlushBits();
}

///////////////////////////////////////////////////////////////////////////////

void BitCounter::AddBits(const uint32_t bits, size_t nbits) {
  size_ += nbits;
  bit_pos_ += nbits;
  bits_ |= bits << (32 - bit_pos_);
  while (bit_pos_ >= 8) {
    size_ += ((bits_ >> 24) == 0xff) ? 8 : 0;
    bits_ <<= 8;
    bit_pos_ -= 8;
  }
}

}   // namespace sjpeg

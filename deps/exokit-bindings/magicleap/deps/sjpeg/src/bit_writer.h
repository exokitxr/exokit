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

#ifndef SJPEG_BIT_WRITER_H_
#define SJPEG_BIT_WRITER_H_

#include <assert.h>
#include <stdint.h>
#include <string.h>   // for memcpy

#include <string>
#include <vector>

#include "sjpeg.h"

namespace sjpeg {

///////////////////////////////////////////////////////////////////////////////
// Memory-Sink

class MemorySink : public ByteSink {
 public:
  explicit MemorySink(size_t expected_size);
  virtual ~MemorySink();
  virtual bool Commit(size_t used_size, size_t extra_size, uint8_t** data);
  virtual bool Finalize() { /* nothing to do */ return true; }
  virtual void Reset();
  void Release(uint8_t** buf_ptr, size_t* size_ptr);

 private:
  uint8_t* buf_;
  size_t pos_, max_pos_;
};

///////////////////////////////////////////////////////////////////////////////
// Sink for generic container
//   Container must supply .resize() and [], and be byte-based.

template<class T> class Sink : public ByteSink {
 public:
  explicit Sink(T* const output) : ptr_(output), pos_(0) {}
  virtual ~Sink() {}
  virtual bool Commit(size_t used_size, size_t extra_size, uint8_t** data) {
    pos_ += used_size;
    assert(pos_ <= ptr_->size());
    ptr_->resize(pos_ + extra_size);
    if (ptr_->size() != pos_ + extra_size) return false;
    *data = extra_size ? reinterpret_cast<uint8_t*>(&(*ptr_)[pos_]) : nullptr;
    return true;
  }
  virtual bool Finalize() { ptr_->resize(pos_); return true; }
  virtual void Reset() { ptr_->clear(); }

 protected:
  T* const ptr_;
  size_t pos_;
};

typedef Sink<std::string> StringSink;
typedef Sink<std::vector<uint8_t> > VectorSink;

///////////////////////////////////////////////////////////////////////////////
// BitWriter

class BitWriter {
 public:
  explicit BitWriter(ByteSink* const sink);

  // Verifies the that output buffer can store at least 'size' more bytes.
  // Also flushes the previously written data.
  bool Reserve(size_t size) {
    const bool ok = sink_->Commit(byte_pos_, size, &buf_);
    if (!ok) sink_->Reset();
    byte_pos_ = 0;
    return ok;
  }

  // Make sure we can write 24 bits by flushing the past ones.
  // WARNING! There's no check for buffer overwrite. Use Reserve() before
  // calling this function.
  void FlushBits() {
    // worst case: 3 escaped codes = 6 bytes
    while (nb_bits_ >= 8) {
      const uint8_t tmp = bits_ >> 24;
      buf_[byte_pos_++] = tmp;
      if (tmp == 0xff) {   // escaping
        buf_[byte_pos_++] = 0x00;
      }
      bits_ <<= 8;
      nb_bits_ -= 8;
    }
  }
  // Writes the sequence 'bits' of length 'nb_bits' (less than 24).
  // WARNING! There's no check for buffer overwrite. Use Reserve() before
  // calling this function.
  void PutBits(uint32_t bits, int nb) {
    assert(nb <= 24 && nb > 0);
    assert((bits & ~((1 << nb) - 1)) == 0);
    FlushBits();    // make room for a least 24bits
    nb_bits_+= nb;
    bits_ |= bits << (32 - nb_bits_);
  }
  // Append one byte to buffer. FlushBits() must have been called before.
  // WARNING! There's no check for buffer overwrite. Use Reserve() before
  // calling this function.
  // Also: no 0xff escaping is performed by this function.
  void PutByte(uint8_t value) {
    assert(nb_bits_ == 0);
    buf_[byte_pos_++] = value;
  }
  // Same as multiply calling PutByte().
  void PutBytes(const uint8_t* buf, size_t size) {
    assert(nb_bits_ == 0);
    assert(buf != NULL);
    assert(size > 0);
    memcpy(buf_ + byte_pos_, buf, size);
    byte_pos_ += size;
  }

  // Handy helper to write a packed code in one call.
  void PutPackedCode(uint32_t code) { PutBits(code >> 16, code & 0xff); }

  // Write pending bits, and align bitstream with extra '1' bits.
  void Flush();

  // To be called last.
  bool Finalize() { return Reserve(0) && sink_->Finalize(); }

 private:
  ByteSink* sink_;

  int nb_bits_;      // number of unwritten bits
  uint32_t bits_;    // accumulator for unwritten bits
  size_t byte_pos_;  // write position, in bytes
  uint8_t* buf_;     // destination buffer (don't access directly!)
};

// Class for counting bits, including the 0xff escape
struct BitCounter {
  BitCounter() : bits_(0), bit_pos_(0), size_(0) {}

  void AddPackedCode(const uint32_t code) { AddBits(code >> 16, code & 0xff); }
  void AddBits(const uint32_t bits, size_t nbits);
  size_t Size() const { return size_; }

 private:
  uint32_t bits_;
  size_t bit_pos_;
  size_t size_;
};

}   // namespace sjpeg

#endif    // SJPEG_BIT_WRITER_H_

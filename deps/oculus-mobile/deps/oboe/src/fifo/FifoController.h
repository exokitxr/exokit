/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NATIVEOBOE_FIFOCONTROLLER_H
#define NATIVEOBOE_FIFOCONTROLLER_H

#include <sys/types.h>
#include "FifoControllerBase.h"
#include <atomic>

namespace oboe {

/**
 * A FifoControllerBase with counters contained in the class.
 */
class FifoController : public FifoControllerBase
{
public:
    FifoController(uint32_t bufferSize, uint32_t threshold);
    virtual ~FifoController();

    // TODO review use atomics or memory barriers
    virtual uint64_t getReadCounter() override {
        return mReadCounter.load(std::memory_order_acquire);
    }
    virtual void setReadCounter(uint64_t n) override {
        mReadCounter.store(n, std::memory_order_release);
    }
    virtual uint64_t getWriteCounter() override {
        return mWriteCounter.load(std::memory_order_acquire);
    }
    virtual void setWriteCounter(uint64_t n) override {
        mWriteCounter.store(n, std::memory_order_release);
    }

private:
    std::atomic<uint64_t> mReadCounter;
    std::atomic<uint64_t> mWriteCounter;
};

} // namespace oboe

#endif //NATIVEOBOE_FIFOCONTROLLER_H

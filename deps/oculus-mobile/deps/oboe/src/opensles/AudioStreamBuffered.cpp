/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <memory>

#include "oboe/Oboe.h"

#include "opensles/AudioStreamBuffered.h"
#include "common/AudioClock.h"

namespace oboe {

constexpr int kDefaultBurstsPerBuffer = 16;  // arbitrary, allows dynamic latency tuning

/*
 * AudioStream with a FifoBuffer
 */
AudioStreamBuffered::AudioStreamBuffered(const AudioStreamBuilder &builder)
        : AudioStream(builder) {
}

void AudioStreamBuffered::allocateFifo() {
    // If the caller does not provide a callback use our own internal
    // callback that reads data from the FIFO.
    if (usingFIFO()) {
        // FIFO is configured with the same format and channels as the stream.
        int32_t capacity = getBufferCapacityInFrames();
        if (capacity == oboe::kUnspecified) {
            capacity = getFramesPerBurst() * kDefaultBurstsPerBuffer;
            mBufferCapacityInFrames = capacity;
        }
        // TODO consider using std::make_unique if we require c++14
        mFifoBuffer.reset(new FifoBuffer(getBytesPerFrame(), capacity));
    }
}

void AudioStreamBuffered::updateFramesWritten() {
    if (mFifoBuffer) {
        mFramesWritten = static_cast<int64_t>(mFifoBuffer->getWriteCounter());
    } // or else it will get updated by processBufferCallback()
}

void AudioStreamBuffered::updateFramesRead() {
    if (mFifoBuffer) {
        mFramesRead = static_cast<int64_t>(mFifoBuffer->getReadCounter());
    } // or else it will get updated by processBufferCallback()
}

// This is called by the OpenSL ES callback to read or write the back end of the FIFO.
DataCallbackResult AudioStreamBuffered::onDefaultCallback(void *audioData, int numFrames) {
    int32_t framesTransferred  = 0;

    if (getDirection() == oboe::Direction::Output) {
        // Read from the FIFO and write to audioData, clear part of buffer if not enough data.
        framesTransferred = mFifoBuffer->readNow(audioData, numFrames);
    } else {
        // Read from audioData and write to the FIFO
        framesTransferred = mFifoBuffer->write(audioData, numFrames); // There is no writeNow()
    }

    if (framesTransferred < numFrames) {
        // TODO If we do not allow FIFO to wrap then our timestamps will drift when there is an XRun!
        incrementXRunCount();
    }
    markCallbackTime(numFrames); // so foreground knows how long to wait.
    return DataCallbackResult::Continue;
}

void AudioStreamBuffered::markCallbackTime(int numFrames) {
    mLastBackgroundSize = numFrames;
    mBackgroundRanAtNanoseconds = AudioClock::getNanoseconds();
}

int64_t AudioStreamBuffered::predictNextCallbackTime() {
    if (mBackgroundRanAtNanoseconds == 0) {
        return 0;
    }
    int64_t nanosPerBuffer = (kNanosPerSecond * mLastBackgroundSize) / getSampleRate();
    const int64_t margin = 200 * kNanosPerMicrosecond; // arbitrary delay so we wake up just after
    return mBackgroundRanAtNanoseconds + nanosPerBuffer + margin;
}

// Common code for read/write.
// @return Result::OK with frames read/written, or Result::Error*
ResultWithValue<int32_t> AudioStreamBuffered::transfer(void *buffer,
                                      int32_t numFrames,
                                      int64_t timeoutNanoseconds) {
    // Validate arguments.
    if (buffer == nullptr) {
        LOGE("AudioStreamBuffered::%s(): buffer is NULL", __func__);
        return ResultWithValue<int32_t>(Result ::ErrorNull);
    }
    if (numFrames < 0) {
        LOGE("AudioStreamBuffered::%s(): numFrames is negative", __func__);
        return ResultWithValue<int32_t>(Result::ErrorOutOfRange);
    } else if (numFrames == 0) {
        return ResultWithValue<int32_t>(numFrames);
    }
    if (timeoutNanoseconds < 0) {
        LOGE("AudioStreamBuffered::%s(): timeoutNanoseconds is negative", __func__);
        return ResultWithValue<int32_t>(Result::ErrorOutOfRange);
    }

    int32_t result = 0;
    uint8_t *data = reinterpret_cast<uint8_t *>(buffer);
    int32_t framesLeft = numFrames;
    int64_t timeToQuit = 0;
    bool repeat = true;

    // Calculate when to timeout.
    if (timeoutNanoseconds > 0) {
        timeToQuit = AudioClock::getNanoseconds() + timeoutNanoseconds;
    }

    // Loop until we get the data, or we have an error, or we timeout.
    do {
        // read or write
        if (getDirection() == Direction::Input) {
            result = mFifoBuffer->read(data, framesLeft);
        } else {
            result = mFifoBuffer->write(data, framesLeft);
        }
        if (result > 0) {
            data += mFifoBuffer->convertFramesToBytes(result);
            framesLeft -= result;
        }

        // If we need more data then sleep and try again.
        if (framesLeft > 0 && result >= 0 && timeoutNanoseconds > 0) {
            int64_t timeNow = AudioClock::getNanoseconds();
            if (timeNow >= timeToQuit) {
                LOGE("AudioStreamBuffered::%s(): TIMEOUT", __func__);
                repeat = false; // TIMEOUT
            } else {
                // Figure out how long to sleep.
                int64_t sleepForNanos;
                int64_t wakeTimeNanos = predictNextCallbackTime();
                if (wakeTimeNanos <= 0) {
                    // No estimate available. Sleep for one burst.
                    sleepForNanos = (getFramesPerBurst() * kNanosPerSecond) / getSampleRate();
                } else {
                    // Don't sleep past timeout.
                    if (wakeTimeNanos > timeToQuit) {
                        wakeTimeNanos = timeToQuit;
                    }
                    sleepForNanos = wakeTimeNanos - timeNow;
                    // Avoid rapid loop with no sleep.
                    const int64_t minSleepTime = kNanosPerMillisecond; // arbitrary
                    if (sleepForNanos < minSleepTime) {
                        sleepForNanos = minSleepTime;
                    }
                }

                AudioClock::sleepForNanos(sleepForNanos);
            }

        } else {
            repeat = false;
        }
    } while(repeat);

    if (result < 0) {
        return ResultWithValue<int32_t>(static_cast<Result>(result));
    } else {
        int32_t framesWritten = numFrames - framesLeft;
        return ResultWithValue<int32_t>(framesWritten);
    }
}

// Write to the FIFO so the callback can read from it.
ResultWithValue<int32_t> AudioStreamBuffered::write(const void *buffer,
                                   int32_t numFrames,
                                   int64_t timeoutNanoseconds) {
    if (getState() == StreamState::Closed){
        return ResultWithValue<int32_t>(Result::ErrorClosed);
    }

    if (getDirection() == Direction::Input) {
        return ResultWithValue<int32_t>(Result::ErrorUnavailable); // TODO review, better error code?
    }
    updateServiceFrameCounter();
    return transfer(const_cast<void *>(buffer), numFrames, timeoutNanoseconds);
}

// Read data from the FIFO that was written by the callback.
ResultWithValue<int32_t> AudioStreamBuffered::read(void *buffer,
                                  int32_t numFrames,
                                  int64_t timeoutNanoseconds) {
    if (getState() == StreamState::Closed){
        return ResultWithValue<int32_t>(Result::ErrorClosed);
    }

    if (getDirection() == Direction::Output) {
        return ResultWithValue<int32_t>(Result::ErrorUnavailable); // TODO review, better error code?
    }
    updateServiceFrameCounter();
    return transfer(buffer, numFrames, timeoutNanoseconds);
}

// Only supported when we are not using a callback.
ResultWithValue<int32_t> AudioStreamBuffered::setBufferSizeInFrames(int32_t requestedFrames)
{
    if (getState() == StreamState::Closed){
        return ResultWithValue<int32_t>(Result::ErrorClosed);
    }

    if (mFifoBuffer) {
        if (requestedFrames > mFifoBuffer->getBufferCapacityInFrames()) {
            requestedFrames = mFifoBuffer->getBufferCapacityInFrames();
        } else if (requestedFrames < getFramesPerBurst()) {
            requestedFrames = getFramesPerBurst();
        }
        mFifoBuffer->setThresholdFrames(requestedFrames);
        return ResultWithValue<int32_t>(requestedFrames);
    } else {
        return ResultWithValue<int32_t>(Result::ErrorUnimplemented);
    }
}

int32_t AudioStreamBuffered::getBufferSizeInFrames() {
    if (mFifoBuffer) {
        mBufferSizeInFrames = mFifoBuffer->getThresholdFrames();
    }
    return mBufferSizeInFrames;
}

int32_t AudioStreamBuffered::getBufferCapacityInFrames() const {
    if (mFifoBuffer) {
        return mFifoBuffer->getBufferCapacityInFrames();
    } else {
        return AudioStream::getBufferCapacityInFrames();
    }
}

bool AudioStreamBuffered::isXRunCountSupported() const {
    // XRun count is only supported if we're using blocking I/O (not callbacks)
    return (getCallback() == nullptr);
}

} // namespace oboe
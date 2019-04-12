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

#include <sys/types.h>
#include <pthread.h>
#include <thread>
#include <oboe/AudioStream.h>
#include "OboeDebug.h"
#include <oboe/Utilities.h>

namespace oboe {

/*
 * AudioStream
 */
AudioStream::AudioStream(const AudioStreamBuilder &builder)
        : AudioStreamBase(builder) {
}

Result AudioStream::open() {
    // Parameters are validated by the underlying API.
    return Result::OK;
}

Result AudioStream::close() {
    // Update local counters so they can be read after the close.
    updateFramesWritten();
    updateFramesRead();
    return Result::OK;
}

DataCallbackResult AudioStream::fireDataCallback(void *audioData, int32_t numFrames) {
    if (!isDataCallbackEnabled()) {
        LOGW("AudioStream::%s() called with data callback disabled!", __func__);
        return DataCallbackResult::Stop; // We should not be getting called any more.
    }

    int scheduler = sched_getscheduler(0) & ~SCHED_RESET_ON_FORK; // for current thread
    if (scheduler != mPreviousScheduler) {
        LOGD("AudioStream::%s() scheduler = %s", __func__,
             ((scheduler == SCHED_FIFO) ? "SCHED_FIFO" :
             ((scheduler == SCHED_OTHER) ? "SCHED_OTHER" :
             ((scheduler == SCHED_RR) ? "SCHED_RR" : "UNKNOWN")))
        );
        mPreviousScheduler = scheduler;
    }

    DataCallbackResult result;
    if (mStreamCallback == nullptr) {
        result = onDefaultCallback(audioData, numFrames);
    } else {
        result = mStreamCallback->onAudioReady(this, audioData, numFrames);
    }
    // On Oreo, we might get called after returning stop.
    // So block there here.
    setDataCallbackEnabled(result == DataCallbackResult::Continue);

    return result;
}

Result AudioStream::waitForStateTransition(StreamState startingState,
                                           StreamState endingState,
                                           int64_t timeoutNanoseconds)
{
    StreamState state;
    {
        std::lock_guard<std::mutex> lock(mLock);
        state = getState();
        if (state == StreamState::Closed) {
            return Result::ErrorClosed;
        } else if (state == StreamState::Disconnected) {
            return Result::ErrorDisconnected;
        }
    }

    StreamState nextState = state;
    // TODO Should this be a while()?!
    if (state == startingState && state != endingState) {
        Result result = waitForStateChange(state, &nextState, timeoutNanoseconds);
        if (result != Result::OK) {
            return result;
        }
    }

    if (nextState != endingState) {
        return Result::ErrorInvalidState;
    } else {
        return Result::OK;
    }
}

Result AudioStream::start(int64_t timeoutNanoseconds)
{
    Result result = requestStart();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    return waitForStateTransition(StreamState::Starting,
                                  StreamState::Started, timeoutNanoseconds);
}

Result AudioStream::pause(int64_t timeoutNanoseconds)
{
    Result result = requestPause();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    return waitForStateTransition(StreamState::Pausing,
                                  StreamState::Paused, timeoutNanoseconds);
}

Result AudioStream::flush(int64_t timeoutNanoseconds)
{
    Result result = requestFlush();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    return waitForStateTransition(StreamState::Flushing,
                                  StreamState::Flushed, timeoutNanoseconds);
}

Result AudioStream::stop(int64_t timeoutNanoseconds)
{
    Result result = requestStop();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    return waitForStateTransition(StreamState::Stopping,
                                  StreamState::Stopped, timeoutNanoseconds);
}

int32_t AudioStream::getBytesPerSample() const {
    return convertFormatToSizeInBytes(mFormat);
}

int64_t AudioStream::getFramesRead() {
    updateFramesRead();
    return mFramesRead;
}

int64_t AudioStream::getFramesWritten() {
    updateFramesWritten();
    return mFramesWritten;
}

static void oboe_stop_thread_proc(AudioStream *oboeStream) {
    LOGD("%s() called ----)))))", __func__);
    if (oboeStream != nullptr) {
        oboeStream->requestStop();
    }
    LOGD("%s() returning (((((----", __func__);
}

void AudioStream::launchStopThread() {
    // Stop this stream on a separate thread
    // std::thread t(requestStop);
    std::thread t(oboe_stop_thread_proc, this);
    t.detach();
}

} // namespace oboe

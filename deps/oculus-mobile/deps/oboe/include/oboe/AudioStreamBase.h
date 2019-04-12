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

#ifndef OBOE_STREAM_BASE_H_
#define OBOE_STREAM_BASE_H_

#include <memory>
#include "oboe/AudioStreamCallback.h"
#include "oboe/Definitions.h"

namespace oboe {

/**
 * Base class containing parameters for audio streams and builders.
 **/
class AudioStreamBase {
public:

    AudioStreamBase() {}

    virtual ~AudioStreamBase() = default;

    // This class only contains primitives so we can use default constructor and copy methods.

    /**
     * Default copy constructor
     */
    AudioStreamBase(const AudioStreamBase&) = default;

    /**
     * Default assignment operator
     */
    AudioStreamBase& operator=(const AudioStreamBase&) = default;

    /**
     * @return number of channels, for example 2 for stereo, or kUnspecified
     */
    int32_t getChannelCount() const { return mChannelCount; }

    /**
     * @return Direction::Input or Direction::Output
     */
    Direction getDirection() const { return mDirection; }

    /**
     * @return sample rate for the stream or kUnspecified
     */
    int32_t getSampleRate() const { return mSampleRate; }

    /**
     * @return the number of frames in each callback or kUnspecified.
     */
    int32_t getFramesPerCallback() const { return mFramesPerCallback; }

    /**
     * @return the audio sample format (e.g. Float or I16)
     */
    AudioFormat getFormat() const { return mFormat; }

    /**
     * Query the maximum number of frames that can be filled without blocking.
     * If the stream has been closed the last known value will be returned.
     *
     * @return buffer size
     */
    virtual int32_t getBufferSizeInFrames() { return mBufferSizeInFrames; }

    /**
     * @return capacityInFrames or kUnspecified
     */
    virtual int32_t getBufferCapacityInFrames() const { return mBufferCapacityInFrames; }

    /**
     * @return the sharing mode of the stream.
     */
    SharingMode getSharingMode() const { return mSharingMode; }

    /**
     * @return the performance mode of the stream.
     */
    PerformanceMode getPerformanceMode() const { return mPerformanceMode; }

    /**
     * @return the device ID of the stream.
     */
    int32_t getDeviceId() const { return mDeviceId; }

    /**
     * @return the callback object for this stream, if set.
     */
    AudioStreamCallback* getCallback() const {
        return mStreamCallback;
    }

    /**
     * @return the usage for this stream.
     */
    Usage getUsage() const { return mUsage; }

    /**
     * @return the stream's content type.
     */
    ContentType getContentType() const { return mContentType; }

    /**
     * @return the stream's input preset.
     */
    InputPreset getInputPreset() const { return mInputPreset; }

    /**
     * @return the stream's session ID allocation strategy (None or Allocate).
     */
    SessionId getSessionId() const { return mSessionId; }

protected:

    /** The callback which will be fired when new data is ready to be read/written **/
    AudioStreamCallback            *mStreamCallback = nullptr;
    /** Number of audio frames which will be requested in each callback */
    int32_t                         mFramesPerCallback = kUnspecified;
    /** Stream channel count */
    int32_t                         mChannelCount = kUnspecified;
    /** Stream sample rate */
    int32_t                         mSampleRate = kUnspecified;
    /** Stream audio device ID */
    int32_t                         mDeviceId = kUnspecified;
    /** Stream buffer capacity specified as a number of audio frames */
    int32_t                         mBufferCapacityInFrames = kUnspecified;
    /** Stream buffer size specified as a number of audio frames */
    int32_t                         mBufferSizeInFrames = kUnspecified;
    /**
     * Number of frames which will be copied to/from the audio device in a single read/write
     * operation
     */
    int32_t                         mFramesPerBurst = kUnspecified;

    /** Stream sharing mode */
    SharingMode                     mSharingMode = SharingMode::Shared;
    /** Format of audio frames */
    AudioFormat                     mFormat = AudioFormat::Unspecified;
    /** Stream direction */
    Direction                       mDirection = Direction::Output;
    /** Stream performance mode */
    PerformanceMode                 mPerformanceMode = PerformanceMode::None;

    /** Stream usage. Only active on Android 28+ */
    Usage                           mUsage = Usage::Media;
    /** Stream content type. Only active on Android 28+ */
    ContentType                     mContentType = ContentType::Music;
    /** Stream input preset. Only active on Android 28+ */
    InputPreset                     mInputPreset = InputPreset::VoiceRecognition;
    /** Stream session ID allocation strategy. Only active on Android 28+ */
    SessionId                       mSessionId = SessionId::None;
};

} // namespace oboe

#endif /* OBOE_STREAM_BASE_H_ */

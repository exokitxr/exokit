#include <AudioDestinationGenericImpl.h>

namespace webaudio {

#ifdef ANDROID

void processBuffers(AudioDestinationGenericImpl *audioDestination) {
  while (audioDestination->outputBuffers.size() == 0 || audioDestination->inputBuffers.size() > 0) {
    std::vector<float> outputBuffer(lab::AudioNode::ProcessingSizeInFrames * 2);
    std::vector<float> *inputBuffer;
    std::vector<float> inputBufferCache;
    if (audioDestination->inputBuffers.size() > 0) {
      inputBuffer = &audioDestination->inputBuffers.front();
    } else {
      inputBufferCache.resize(lab::AudioNode::ProcessingSizeInFrames);
      inputBuffer = &inputBufferCache;
    }

    audioDestination->render(lab::AudioNode::ProcessingSizeInFrames, outputBuffer.data(), inputBuffer->data());

    audioDestination->outputBuffers.push_back(std::move(outputBuffer));

    if (audioDestination->inputBuffers.size() > 0) {
      audioDestination->inputBuffers.pop_front();
    }
  }
}

OutputCallback::OutputCallback(AudioDestinationGenericImpl *audioDestination) : audioDestination(audioDestination) {}
oboe::DataCallbackResult OutputCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) {
  // output needs data

  if (numFrames != lab::AudioNode::ProcessingSizeInFrames) {
    std::cerr << "audio output: failed to match number of frames: " << numFrames << " " << lab::AudioNode::ProcessingSizeInFrames << std::endl;
  }

  {
    std::lock_guard<std::mutex> lock(audioDestination->mutex);

    processBuffers(audioDestination);

    if (audioDestination->outputBuffers.size() > 0) {
      std::vector<float> &outputBuffer = audioDestination->outputBuffers.front();
      for (int32_t i = 0; i < numFrames; i++) {
        ((float *)audioData)[i*2] = outputBuffer[i];
        ((float *)audioData)[i*2 + 1] = outputBuffer[numFrames + i];
      }
      audioDestination->outputBuffers.pop_front();
    } else {
      memset(audioData, 0, numFrames*2*sizeof(float));
    }
  }

  return oboe::DataCallbackResult::Continue;
}

InputCallback::InputCallback(AudioDestinationGenericImpl *audioDestination) : audioDestination(audioDestination) {}
oboe::DataCallbackResult InputCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) {
  // input has data

  if (numFrames != lab::AudioNode::ProcessingSizeInFrames) {
    std::cerr << "audio input: failed to match number of frames: " << numFrames << " " << lab::AudioNode::ProcessingSizeInFrames << std::endl;
  }

  std::vector<float> inputBuffer(numFrames);
  memcpy(inputBuffer.data(), audioData, numFrames*sizeof(float));

  {
    std::lock_guard<std::mutex> lock(audioDestination->mutex);

    audioDestination->inputBuffers.push_back(std::move(inputBuffer));
    processBuffers(audioDestination);
  }

  return oboe::DataCallbackResult::Continue;
}

/* void outputErrorCallback(AAudioStream *stream, void *userData, aaudio_result_t error) {
  // AudioDestinationGenericImpl *audioDestination = (AudioDestinationGenericImpl *)userData;

  std::cerr << "audio output error callback: " << error << std::endl;
}

void inputErrorCallback(AAudioStream *stream, void *userData, aaudio_result_t error) {
  // AudioDestinationGenericImpl *audioDestination = (AudioDestinationGenericImpl *)userData;

  std::cerr << "audio input error callback: " << error << std::endl;
} */

AudioDestinationGenericImpl::AudioDestinationGenericImpl(float sampleRate, std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn) :
  renderFn(renderFn)
{
  // output
  {
    oboe::AudioStreamBuilder builder;

    builder.setDirection(oboe::Direction::Output);
    // builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    // builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSampleRate(lab::DefaultSampleRate);
    builder.setChannelCount(oboe::ChannelCount::Stereo);
    // builder.setBufferCapacityInFrames(lab::AudioNode::ProcessingSizeInFrames * 2);
    builder.setFramesPerCallback(lab::AudioNode::ProcessingSizeInFrames);
    outputCallback = new OutputCallback(this);
    builder.setCallback(outputCallback);

    oboe::Result result = builder.openStream(&outputStream);
    if (result != oboe::Result::OK) {
      std::cerr << "failed to open output stream: " << oboe::convertToText(result) << std::endl;
    }
  }
  // input
  {
    oboe::AudioStreamBuilder builder;

    builder.setDirection(oboe::Direction::Input);
    // builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    // builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSampleRate(lab::DefaultSampleRate);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    // builder.setBufferCapacityInFrames(lab::AudioNode::ProcessingSizeInFrames * 2);
    builder.setFramesPerCallback(lab::AudioNode::ProcessingSizeInFrames);
    inputCallback = new InputCallback(this);
    builder.setCallback(inputCallback);

    oboe::Result result = builder.openStream(&inputStream);
    if (result != oboe::Result::OK) {
      std::cerr << "failed to open input stream: " << oboe::convertToText(result) << std::endl;
    }
  }
}

AudioDestinationGenericImpl::~AudioDestinationGenericImpl() {
  outputStream->close();
  inputStream->close();
  delete outputCallback;
  delete inputCallback;
}

bool AudioDestinationGenericImpl::start() {
  oboe::Result result = outputStream->requestStart();
  if (result == oboe::Result::OK) {
    return true;
  } else {
    return false;
  }
}

bool AudioDestinationGenericImpl::stop() {
  oboe::Result result = outputStream->requestStop();
  if (result == oboe::Result::OK) {
    return true;
  } else {
    return false;
  }
}

bool AudioDestinationGenericImpl::startRecording() {
  oboe::Result result = inputStream->requestStart();
  if (result == oboe::Result::OK) {
    m_isRecording = true;
    return true;
  } else {
    return false;
  }
}

bool AudioDestinationGenericImpl::stopRecording() {
  oboe::Result result = inputStream->requestStop();
  if (result == oboe::Result::OK) {
    m_isRecording = false;
    return true;
  } else {
    return false;
  }
}

void AudioDestinationGenericImpl::render(int numberOfFrames, void *outputBuffer, void *inputBuffer) {
  renderFn(numberOfFrames, outputBuffer, inputBuffer);
}

bool AudioDestinationGenericImpl::isRecording() {
  return m_isRecording;
}

#endif

#ifdef LUMIN

const uint32_t mlBufferSize = 4096;

void processBuffers(AudioDestinationGenericImpl *audioDestination) {
  for (;;) {
    if (audioDestination->outputMlBuffers.size() > 0) {
      MLAudioBuffer &outputMlBuffer = audioDestination->outputMlBuffers.front();
      int totalOutputFrames = outputMlBuffer.size / 2 / sizeof(int16_t);
      int remainingOutputFrames = totalOutputFrames - audioDestination->outputIndex;
      int currentFrames = std::min<int>(remainingOutputFrames, AudioNode::ProcessingSizeInFrames);

      if (audioDestination->isRecording()) {
        if (audioDestination->inputMlBuffers.size() > 0) {
          std::vector<float> &inputBuffer = audioDestination->inputBuffers.front();
          int totalInputFrames = inputBuffer.size();
          int remainingInputFrames = totalInputFrames - audioDestination->inputIndex;

          currentFrames = std::min<int>(currentFrames, remainingInputFrames);
        } else {
          break;
        }
      }

      std::vector<float> outputBuffer(currentFrames * 2);
      std::vector<float> *inputBuffer;
      int inputIndex;
      std::vector<float> inputBufferCache;
      if (audioDestination->isRecording()) {
        inputBuffer = &audioDestination->inputBuffers.front();
        inputIndex = audioDestination->inputIndex;
      } else {
        inputBufferCache.resize(outputBuffer.size());
        inputBuffer = &inputBufferCache;
        inputIndex = 0;
      }
      audioDestination->render(currentFrames, outputBuffer.data(), inputBuffer->data() + inputIndex);

      std::vector<int16_t> outputS16Buffer(outputBuffer.size());
      uint8_t *dstData[] = {
        (uint8_t *)outputS16Buffer.data(),
      };
      const uint8_t *srcData[] = {
        (const uint8_t *)(&outputBuffer[0]),
        (const uint8_t *)(&outputBuffer[currentFrames]),
      };
      swr_convert(audioDestination->output_swr_ctx, dstData, currentFrames, srcData, currentFrames);
      memcpy((int16_t *)outputMlBuffer.ptr + audioDestination->outputIndex * 2, outputS16Buffer.data(), currentFrames * 2 * sizeof(int16_t));

      audioDestination->outputIndex += currentFrames;
      if (audioDestination->outputIndex >= totalOutputFrames) {
        MLResult result = MLAudioReleaseOutputStreamBuffer(audioDestination->outputHandle);
        if (result != MLResult_Ok) {
          std::cerr << "failed to release ml output buffer: " << result << std::endl;
        }

        audioDestination->outputMlBuffers.pop_front();
        audioDestination->outputIndex = 0;
      }

      if (audioDestination->isRecording()) {
        audioDestination->inputIndex += currentFrames;

        int totalInputFrames = inputBuffer->size();
        if (audioDestination->inputIndex >= totalInputFrames) {
          MLResult result = MLAudioReleaseInputStreamBuffer(audioDestination->inputHandle);
          if (result != MLResult_Ok) {
            std::cerr << "failed to release ml input buffer: " << result << std::endl;
          }

          audioDestination->inputMlBuffers.pop_front();
          audioDestination->inputBuffers.pop_front();
          audioDestination->inputIndex = 0;
        }
      }
    } else {
      break;
    }
  }
}

void outputBufferCallback(MLHandle handle, void *callback_context) {
  AudioDestinationGenericImpl *audioDestination = (AudioDestinationGenericImpl *)callback_context;

  {
    std::lock_guard<std::mutex> lock(audioDestination->mutex);

    MLAudioBuffer outputMlBuffer;
    MLResult result = MLAudioGetOutputStreamBuffer(
      audioDestination->outputHandle,
      &outputMlBuffer
    );
    if (result == MLResult_Ok) {
      audioDestination->outputMlBuffers.push_back(outputMlBuffer);

      processBuffers(audioDestination);
    } else {
      std::cerr << "failed to get ml output buffer: " << result << std::endl;
    }
  }
}

void inputBufferCallback(MLHandle handle, void *callback_context) {
  AudioDestinationGenericImpl *audioDestination = (AudioDestinationGenericImpl *)callback_context;

  {
    std::lock_guard<std::mutex> lock(audioDestination->mutex);

    MLAudioBuffer inputMlBuffer;
    MLResult result = MLAudioGetInputStreamBuffer(
      audioDestination->inputHandle,
      &inputMlBuffer
    );
    if (result == MLResult_Ok) {
      int numRawFrames = inputMlBuffer.size / sizeof(int16_t);
      int numConvertedFrames = mlBufferSize;
      std::vector<float> inputBuffer(numConvertedFrames);

      uint8_t *dstData[] = {
        (uint8_t *)inputBuffer.data(),
      };
      const uint8_t *srcData[] = {
        (const uint8_t *)inputMlBuffer.ptr,
      };
      swr_convert(audioDestination->input_swr_ctx, dstData, numConvertedFrames, srcData, numRawFrames);

      audioDestination->inputMlBuffers.push_back(inputMlBuffer);
      audioDestination->inputBuffers.push_back(std::move(inputBuffer));
      // audioDestination->m_isRecording = true;

      processBuffers(audioDestination);
    } else {
      std::cerr << "failed to get ml input buffer: " << result << std::endl;
    }
  }
}

AudioDestinationGenericImpl::AudioDestinationGenericImpl(float sampleRate, std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn) :
  renderFn(renderFn)
{
    {
      outputAudioBufferFormat.bits_per_sample = 16;
      outputAudioBufferFormat.channel_count = 2;
      outputAudioBufferFormat.sample_format = MLAudioSampleFormat_Int;
      outputAudioBufferFormat.samples_per_second = (uint32_t)sampleRate;
      outputAudioBufferFormat.valid_bits_per_sample = 16;

      uint32_t outputBufferSize = mlBufferSize*2*sizeof(int16_t);
      
      MLResult result = MLAudioCreateSoundWithOutputStream(
        &outputAudioBufferFormat,
        // nOutputBufferFramesPerChannel * 2 * sizeof(uint16_t),
        outputBufferSize,
        outputBufferCallback,
        this,
        &outputHandle
      );
      if (result != MLResult_Ok) {
        std::cerr << "failed to create ml output sound: " << result << std::endl;
      }
    }

    {
      inputAudioBufferFormat.bits_per_sample = 16;
      inputAudioBufferFormat.channel_count = 1;
      inputAudioBufferFormat.sample_format = MLAudioSampleFormat_Int;
      inputAudioBufferFormat.samples_per_second = 16000;
      inputAudioBufferFormat.valid_bits_per_sample = 16;

      uint32_t inputBufferSize = (uint32_t)av_rescale_rnd(mlBufferSize*sizeof(int16_t), inputAudioBufferFormat.samples_per_second, outputAudioBufferFormat.samples_per_second, AV_ROUND_UP);
      inputBufferSize += (sizeof(int16_t) - (inputBufferSize % sizeof(int16_t)));

      MLResult result = MLAudioCreateInputFromVoiceComm(
        &inputAudioBufferFormat,
        // nInputBufferFramesPerChannel * sizeof(uint16_t),
        inputBufferSize,
        inputBufferCallback,
        this,
        &inputHandle
      );
      if (result != MLResult_Ok) {
        std::cerr << "failed to create ml microphone input: " << result << std::endl;
      }
    }

    {
      int64_t src_ch_layout = AV_CH_LAYOUT_STEREO, dst_ch_layout = AV_CH_LAYOUT_STEREO;
      enum AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_FLTP, dst_sample_fmt = AV_SAMPLE_FMT_S16;
      int src_rate = outputAudioBufferFormat.samples_per_second, dst_rate = outputAudioBufferFormat.samples_per_second;

      output_swr_ctx = swr_alloc_set_opts(
        nullptr,
        dst_ch_layout,
        dst_sample_fmt,
        dst_rate,
        src_ch_layout,
        src_sample_fmt,
        src_rate,
        0,
        nullptr
      );
      if (!output_swr_ctx) {
        std::cerr << "failed to allocate output resmapler context" << std::endl;
      }

      /* initialize the resampling context */
      if (swr_init(output_swr_ctx) < 0) {
        std::cerr << "failed to initialize output resampler context" << std::endl;
      }
    }

    {
      int64_t src_ch_layout = AV_CH_LAYOUT_MONO, dst_ch_layout = AV_CH_LAYOUT_MONO;
      enum AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_S16, dst_sample_fmt = AV_SAMPLE_FMT_FLTP;
      int src_rate = inputAudioBufferFormat.samples_per_second, dst_rate = outputAudioBufferFormat.samples_per_second;

      input_swr_ctx = swr_alloc_set_opts(
        nullptr,
        dst_ch_layout,
        dst_sample_fmt,
        dst_rate,
        src_ch_layout,
        src_sample_fmt,
        src_rate,
        0,
        nullptr
      );
      if (!input_swr_ctx) {
        std::cerr << "failed to allocate input resmapler context" << std::endl;
      }

      /* initialize the resampling context */
      if (swr_init(input_swr_ctx) < 0) {
        std::cerr << "failed to initialize input resampler context" << std::endl;
      }
    }
}

AudioDestinationGenericImpl::~AudioDestinationGenericImpl() {
  swr_free(&audioDestinationGenericImpl->output_swr_ctx);
  swr_free(&audioDestinationGenericImpl->input_swr_ctx);
    // dac.release(); // XXX
    /* if (dac.isStreamOpen())
        dac.closeStream(); */
}

bool AudioDestinationGenericImpl::start() {
    MLResult result = MLAudioStartSound(outputHandle);

    if (result == MLResult_Ok) {
      return true;
    } else {
      std::cerr << "failed to start ml output sound: " << result << std::endl;
      return false;
    }
}

bool AudioDestinationGenericImpl::stop() {
    MLResult result = MLAudioStopSound(outputHandle);

    if (result == MLResult_Ok) {
      return true;
    } else {
      std::cerr << "failed to stop ml output sound: " << result << std::endl;

      return false;
    }
}

bool AudioDestinationGenericImpl::startRecording() {
    MLResult result = MLAudioStartInput(inputHandle);

    if (result == MLResult_Ok) {
      m_isRecording = true;
      return true;
    } else {
      std::cerr << "failed to start ml input: " << result << std::endl;
      return false;
    }
}

bool AudioDestinationGenericImpl::stopRecording() {
    MLResult result = MLAudioStopInput(inputHandle);

    if (result == MLResult_Ok) {
      m_isRecording = false;
      return true;
    } else {
      std::cerr << "failed to stop ml input: " << result << std::endl;
      return false;
    }
}

void AudioDestinationGenericImpl::render(int numberOfFrames, void *outputBuffer, void *inputBuffer) {
  renderFn(numberOfFrames, outputBuffer, inputBuffer);
}

bool AudioDestinationGenericImpl::isRecording() {
  return m_isRecording;
}

#endif

#if defined(ANDROID) || defined(LUMIN)

void *adgCreate(float sampleRate, std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn) {
  return new AudioDestinationGenericImpl(sampleRate, renderFn);
}

void adgDestroy(void *handle) {
  delete (AudioDestinationGenericImpl *)handle;
}

bool adgStart(void *handle) {
  return ((AudioDestinationGenericImpl *)handle)->start();
}

bool adgStop(void *handle) {
  return ((AudioDestinationGenericImpl *)handle)->stop();
}

bool adgStartRecording(void *handle) {
  return ((AudioDestinationGenericImpl *)handle)->startRecording();
}

bool adgStopRecording(void *handle) {
  return ((AudioDestinationGenericImpl *)handle)->stopRecording();
}

#endif

}

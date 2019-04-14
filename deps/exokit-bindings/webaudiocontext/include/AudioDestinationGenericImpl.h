#ifndef _AUDIO_DESTINATION_GENERIC_IMPL_H_
#define _AUDIO_DESTINATION_GENERIC_IMPL_H_

#include <AudioContext.h>
#include <functional>

#ifdef ANDROID
#include <oboe/Oboe.h>
#endif
#ifdef LUMIN
#include <ml_audio.h>
#endif

namespace webaudio {

#ifdef ANDROID

class OutputCallback;
class InputCallback;

class AudioDestinationGenericImpl {
public:
  AudioDestinationGenericImpl(float sampleRate, std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn);
  ~AudioDestinationGenericImpl();

  bool start();
  bool stop();
  bool startRecording();
  bool stopRecording();
  void render(int numberOfFrames, void *outputBuffer, void *inputBuffer);
  bool isRecording();

  std::mutex mutex;

  oboe::AudioStream *inputStream;
  oboe::AudioStream *outputStream;
  OutputCallback *outputCallback;
  InputCallback *inputCallback;
  std::deque<std::vector<float>> outputBuffers;
  std::deque<std::vector<float>> inputBuffers;
  int outputIndex = 0;
  int inputIndex = 0;
  std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn;
  bool m_isRecording = false;
};

class OutputCallback : public oboe::AudioStreamCallback {
public:
  OutputCallback(AudioDestinationGenericImpl *audioDestination);

  oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames);

  AudioDestinationGenericImpl *audioDestination;
};

class InputCallback : public oboe::AudioStreamCallback {
public:
  InputCallback(AudioDestinationGenericImpl *audioDestination);

  oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames);

  AudioDestinationGenericImpl *audioDestination;
};

#endif

#ifdef LUMIN

class AudioDestinationGenericImpl {
public:
  AudioDestinationGenericImpl(float sampleRate, std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn);
  ~AudioDestinationGenericImpl();

  bool start();
  bool stop();
  bool startRecording();
  bool stopRecording();
  void render(int numberOfFrames, void *outputBuffer, void *inputBuffer);
  bool isRecording();
  
  std::mutex mutex;

  MLHandle outputHandle;
  MLHandle inputHandle;
  MLAudioBufferFormat outputAudioBufferFormat;
  MLAudioBufferFormat inputAudioBufferFormat;
  std::deque<MLAudioBuffer> outputMlBuffers;
  std::deque<MLAudioBuffer> inputMlBuffers;
  std::deque<std::vector<float>> inputBuffers;
  int outputIndex = 0;
  int inputIndex = 0;
  // std::vector<float> outputBuffer;
  // std::vector<float> inputBuffer;
  std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn;
  bool m_isRecording = false;
};

#endif

void *adgCreate(float sampleRate, std::function<void(int numberOfFrames, void *outputBuffer, void *inputBuffer)> renderFn);
void adgDestroy(void *handle);
bool adgStart(void *handle);
bool adgStop(void *handle);
bool adgStartRecording(void *handle);
bool adgStopRecording(void *handle);

}

#endif

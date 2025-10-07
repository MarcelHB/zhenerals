// SPDX-License-Identifier: GPL-2.0

#include <array>

#include "Backend.h"
#include "../Logging.h"

namespace ZH::Audio {

static bool checkALError(const std::string& message) {
  int error = alGetError();
  if (error != AL_NO_ERROR) {
    WARN_ZH("OpenAL", "{}: {:#x} - {}", message, error, alGetString(error));
    return true;
  }

  return false;
}

static void logALCError(const std::string& message, ALCdevice* device) {
  int error = alcGetError(device);
  if (error != AL_NO_ERROR) {
    WARN_ZH("OpenAL/ALC", "{}: {:#x}", message, error);
  }
}

static ALenum getFormat(uint8_t channels, uint8_t bits) {
  switch (channels) {
    case 1:
      if (bits == 8)
        return AL_FORMAT_MONO8;
      else
        return AL_FORMAT_MONO16;

    case 2:
      if (bits == 8)
        return AL_FORMAT_STEREO8;
      else
        return AL_FORMAT_STEREO16;
  }

  return AL_FORMAT_MONO8;
}

static void configureSource(ALuint source) {
  std::array<ALfloat, 3> sourceVel = { 0.0f, 0.0f, 0.0f };
  std::array<ALfloat, 3> sourcePos = { 0.0f, 0.0f, 0.0f };

  alSourcefv(source, AL_VELOCITY, sourceVel.data());
  alSourcefv(source, AL_POSITION, sourcePos.data());
  alSourcef(source, AL_GAIN, 1.0f);
  alSourcei(source, AL_SOURCE_RELATIVE, true);
}

SoundBuffer::SoundBuffer(ALPair bufferPair) : buffers(bufferPair)
{}

SoundBuffer::SoundBuffer(SoundBuffer&& other) : buffers(other.buffers) {
  other.buffers = {0, 0};
}

bool SoundBuffer::isDisposable() {
  if (buffers.first == 0 && buffers.second == 0) {
    return true;
  }

  std::array<ALuint, 2> bufferArray = { buffers.first, buffers.second };
  alDeleteBuffers(buffers.second != 0 ? 2 : 1, bufferArray.data());

  auto success = alGetError() == AL_NO_ERROR;
  if (success) {
    buffers = { 0, 0 };
  }

  return success;
}

ALPair SoundBuffer::getBuffers() const {
  return buffers;
}

SoundBuffer::~SoundBuffer() {
  isDisposable();
}

SoundEmitter::SoundEmitter(ALPair sources) : sources(sources)
{
  configureSource(sources.first);
}

SoundEmitter::SoundEmitter(SoundEmitter&& other) : sources(other.sources) {
  other.sources = {0, 0};
}

SoundEmitter::~SoundEmitter() {
  if (sources.first == 0 && sources.second == 0) {
    return;
  }

  std::array<ALuint, 2> sourcesArray = { sources.first, sources.second };
  auto numSources = sources.second != 0 ? 2 : 1;

  alSourceStopv(numSources, sourcesArray.data());
  alSourcei(sources.first, AL_BUFFER, 0);
  if (numSources == 2) {
    alSourcei(sources.second, AL_BUFFER, 0);
  }

  alDeleteSources(numSources, sourcesArray.data());
}

bool SoundEmitter::enqueue(const SoundBuffer& buffer) {
  auto alBuffer = buffer.getBuffers().first;
  alSourceQueueBuffers(sources.first, 1, &alBuffer);
  if (checkALError("Unable to enqueue buffer")) {
    return false;
  }

  std::array<ALuint, 2> sourcesArray = { sources.first, 0 };
  alSourcePlayv(1, sourcesArray.data());

  return true;
}

bool SoundEmitter::hasFinishedPlaying() const {
  ALint state = 0;
  alGetSourcei(sources.first, AL_SOURCE_STATE, &state);

  return state == AL_STOPPED;
}

Backend::Backend(Backend&& other) : alContext(other.alContext) {
  other.alContext = nullptr;
}

Backend::~Backend() {
  if (alContext != nullptr) {
    alcMakeContextCurrent(nullptr);

    ALCdevice* device = alcGetContextsDevice(alContext);
    alcDestroyContext(alContext);
    alcCloseDevice(device);
    alContext = nullptr;
  }
}

bool Backend::init() {
  auto device = alcOpenDevice(nullptr);
  if (device == nullptr) {
    WARN_ZH("OpenAL", "Failed to acquire device");
    return false;
  }

  alContext = alcCreateContext(device, nullptr);
  if (alContext == nullptr) {
    logALCError("Failed to create context", device);
    alcCloseDevice(device);
    return false;
  }

  if (!alcMakeContextCurrent(alContext)) {
    logALCError("Failed to enable context", device);
    alcDestroyContext(alContext);
    alcCloseDevice(device);
    return false;
  }

  return true;
}

SoundEmitter Backend::createEmitter() {
  std::array<ALuint, 2> sourcesArray = { 0, 0 };
  alGenSources(1, sourcesArray.data());

  if (checkALError("Error creating source")) {
    return {};
  }

  return {ALPair {sourcesArray[0], sourcesArray[1] }};
}

SoundBuffer Backend::loadBuffer(const SoundData& sound) {
  std::array<ALuint, 2> buffers {0, 0};
  alGenBuffers(1, buffers.data());

  if (checkALError("Unable to create sound buffer")) {
    return {};
  }

  auto& format = sound.getFormat();
  alBufferData(
      buffers[0]
    , getFormat(format.numChannels, format.bits)
    , sound.getData().data()
    , sound.getData().size()
    , format.numSamplesPerSec
  );

  if (checkALError("Unable to fill sound buffer")) {
    alDeleteBuffers(1, buffers.data());
    return {};
  }

  return ALPair { buffers[0], 0 };
}

}

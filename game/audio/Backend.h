#ifndef H_GAME_AUDIO_BACKEND
#define H_GAME_AUDIO_BACKEND

#include <al.h>
#include <alc.h>

#include "SoundData.h"

namespace ZH::Audio {

using ALPair = std::pair<ALuint, ALuint>;

class SoundBuffer {
  public:
    SoundBuffer() = default;
    SoundBuffer(ALPair);
    SoundBuffer(const SoundBuffer&) = delete;
    SoundBuffer(SoundBuffer&&);
    ~SoundBuffer();

    bool isDisposable();
    ALPair getBuffers() const;
  private:
    ALPair buffers = {0, 0};
};

class SoundEmitter {
  public:
    SoundEmitter() = default;
    SoundEmitter(ALPair);
    SoundEmitter(const SoundEmitter&) = delete;
    SoundEmitter(SoundEmitter&&);
    ~SoundEmitter();

    bool enqueue(const SoundBuffer&);
  private:
    ALPair sources = {0, 0};
};

class Backend {
  public:
    Backend() = default;
    Backend(const Backend&) = delete;
    Backend(Backend&&);
    ~Backend();

    bool init();
    SoundEmitter createEmitter();
    SoundBuffer loadBuffer(const SoundData&);
  private:
    ALCcontext* alContext = nullptr;
};

}

#endif

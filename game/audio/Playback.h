// SPDX-License-Identifier: GPL-2.0

#ifndef H_AUDIO_PLAYBACK
#define H_AUDIO_PLAYBACK

#include <functional>
#include <list>
#include <map>
#include <queue>

#include "../common.h"
#include "../ResourceLoader.h"
#include "Backend.h"
#include "../inis/SoundEffectsINI.h"

namespace ZH::Audio {

class Playback {
  public:
    Playback(
        Backend& backend
      , ResourceLoader& iniLoader
      , ResourceLoader& audioLoader
      , size_t capacity = 50
    );

    bool load();
    bool playSoundEffect(const std::string&);
  private:
    size_t capacity;

    Backend& backend;
    ResourceLoader& audioLoader;
    ResourceLoader& iniLoader;
    SoundEffectsINI::SoundEffects soundEffects;

    std::list<SoundEmitter> activeEmitters;
    std::map<std::string, Audio::SoundBuffer> soundBuffers;

    using BufferMapIt = decltype(soundBuffers)::iterator;
    std::priority_queue<
        BufferMapIt
      , std::vector<BufferMapIt>
      , std::function<bool(BufferMapIt&, BufferMapIt&)>
    > usage;

    void housekeeping();
    void tryCleanUpCache();
};

}

#endif

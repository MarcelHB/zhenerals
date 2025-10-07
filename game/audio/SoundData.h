// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_AUDIO_SOUND_DATA
#define H_GAME_AUDIO_SOUND_DATA

#include <cstdint>
#include <vector>

#include "../common.h"

namespace ZH::Audio {

struct SoundFormat {
  uint8_t numChannels;
  uint8_t bits;
  uint16_t numSamplesPerSec;
};

class SoundData {
  public:
    SoundData(SoundFormat, std::vector<char>&&);

    const std::vector<char>& getData() const;
    const SoundFormat& getFormat() const;
  private:
    SoundFormat format;
    std::vector<char> data;
};

}

#endif

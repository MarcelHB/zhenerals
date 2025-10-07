// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_AUDIO_FILE
#define H_GAME_AUDIO_FILE

#include <istream>
#include <memory>

#include "../common.h"
#include "../audio/SoundData.h"

namespace ZH {

class AudioFile {
  public:
    AudioFile(std::istream&);

    std::shared_ptr<Audio::SoundData> parseSoundData();
  private:
    std::istream& stream;
};

}

#endif

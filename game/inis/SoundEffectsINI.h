// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_SOUND_EFFECTS_INI
#define H_GAME_SOUND_EFFECTS_INI

#include <unordered_map>
#include <utility>
#include <vector>

#include "../common.h"
#include "INIFile.h"
#include "../common.h"

namespace ZH {

enum class SoundEffectPriority {
    LOWEST
  , LOW
  , NORMAL
  , HIGH
  , CRITICAL
};

enum class SoundEffectType : uint16_t {
    NONE = 0
  , UI = 0x1
  , WORLD = 0x2
  , SHROUDED = 0x4
  , GLOBAL = 0x8
  , VOICE = 0x10
  , PLAYER = 0x20
  , ALLIES = 0x40
  , ENEMIES = 0x80
  , EVERYONE = 0x100
};

enum class SoundEffectControl : uint8_t {
    NONE = 0
  , LOOP = 0x1
  , RANDOM = 0x2
  , ALL = 0x4
  , POSTDELAY = 0x8
  , INTERRUPT = 0x10
};

struct SoundEffect {
  uint8_t volume = 100;
  uint8_t minVolume = 0;
  int8_t volumeShift = 0;
  uint8_t limit = 1;
  uint8_t loopCount = 1;
  uint16_t minRange = 0;
  uint16_t maxRange = 1024;
  uint8_t lowPassCutOff = 0;
  SoundEffectPriority priority = SoundEffectPriority::NORMAL;
  BitField<SoundEffectType> types;
  BitField<SoundEffectControl> controlFlags;
  std::pair<int8_t, int8_t> pitchShift = {0, 0};
  std::pair<uint16_t, uint16_t> delay = {0, 0};
  std::vector<std::string> sounds;
  std::vector<std::string> attackSounds;
  std::vector<std::string> decaySounds;
};

class SoundEffectsINI : public INIFile {
  public:
    using SoundEffects = std::unordered_map<std::string, SoundEffect>;

    SoundEffectsINI(std::istream& instream);
    SoundEffects parse();

  private:
    bool parseSoundEffect(SoundEffects& effects);
    std::optional<SoundEffectPriority> parsePriority();
};

}

#endif

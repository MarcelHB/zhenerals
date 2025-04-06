#include <fstream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "../inis/SoundEffectsINI.h"

namespace ZH {

TEST(SoundEffectsINI, parsing) {
  std::ifstream stream {"tests/resources/SoundEffectsINI/effects.ini", std::ios::binary};
  SoundEffectsINI unit {stream};
  auto effects = unit.parse();

  ASSERT_EQ(2, effects.size());
  auto lookup = effects.find("Toot");
  ASSERT_NE(effects.cend(), lookup);

  auto& effect1 = lookup->second;
  EXPECT_EQ(98, effect1.volume);
  EXPECT_EQ(-10, effect1.volumeShift);
  EXPECT_EQ(27, effect1.minVolume);
  EXPECT_EQ(std::make_pair(12, 34), effect1.pitchShift);
  EXPECT_EQ(std::make_pair(0, 500), effect1.delay);
  EXPECT_EQ(2, effect1.limit);
  EXPECT_EQ(3, effect1.loopCount);
  EXPECT_EQ(SoundEffectPriority::LOW, effect1.priority);
  EXPECT_EQ(10, effect1.minRange);
  EXPECT_EQ(77, effect1.maxRange);
  EXPECT_EQ(51, effect1.lowPassCutOff);

  std::vector<std::string> sounds {
    "toot_a", "toot_b", "toot_c"
  };
  EXPECT_EQ(sounds, effect1.sounds);

  std::vector<std::string> attackSounds {
    "attack1", "attack2"
  };
  EXPECT_EQ(attackSounds, effect1.attackSounds);

  std::vector<std::string> decaySounds {
    "decayX", "decayY"
  };
  EXPECT_EQ(decaySounds, effect1.decaySounds);

  EXPECT_EQ(
      BitField(SoundEffectControl::LOOP) | SoundEffectControl::ALL
    , effect1.controlFlags
  );

  EXPECT_EQ(
      BitField(SoundEffectType::WORLD) | SoundEffectType::EVERYONE | SoundEffectType::SHROUDED
    , effect1.types
  );

  lookup = effects.find("Boop");
  ASSERT_NE(effects.cend(), lookup);

  auto& effect2 = lookup->second;
  EXPECT_EQ(89, effect2.volume);
  EXPECT_EQ(-14, effect2.volumeShift);
  EXPECT_EQ(72, effect2.minVolume);
  EXPECT_EQ(std::make_pair(23, 45), effect2.pitchShift);
  EXPECT_EQ(std::make_pair(2, 502), effect2.delay);
  EXPECT_EQ(1, effect2.limit);
  EXPECT_EQ(2, effect2.loopCount);
  EXPECT_EQ(SoundEffectPriority::CRITICAL, effect2.priority);
  EXPECT_EQ(11, effect2.minRange);
  EXPECT_EQ(78, effect2.maxRange);
  EXPECT_EQ(52, effect2.lowPassCutOff);

  sounds = std::vector<std::string> { "boop" };
  EXPECT_EQ(sounds, effect2.sounds);

  attackSounds = std::vector<std::string> { "boop_2" };
  EXPECT_EQ(attackSounds, effect2.attackSounds);

  decaySounds = std::vector<std::string> { "boop_3" };
  EXPECT_EQ(decaySounds, effect2.decaySounds);

  EXPECT_EQ(
      BitField(SoundEffectControl::RANDOM) | SoundEffectControl::INTERRUPT
    , effect2.controlFlags
  );

  EXPECT_EQ(BitField(SoundEffectType::PLAYER) , effect2.types);
}

}

#include <iostream>
#include <fstream>

#include <gtest/gtest.h>

#include "../audio/Backend.h"
#include "../formats/AudioFile.h"

namespace ZH {

void dumpSound(const Audio::SoundData& soundData);
void playSound(const Audio::SoundData& soundData);

TEST(AudioFile, WAVFile) {
  std::ifstream stream {"tests/resources/AudioFile/rain.wav", std::ios::binary};
  AudioFile unit {stream};

  auto result = unit.parseSoundData();
  ASSERT_TRUE(result);

  auto format = result->getFormat();
  EXPECT_EQ(2, format.numChannels);
  EXPECT_EQ(44100, format.numSamplesPerSec);
  EXPECT_EQ(16, format.bits);
  EXPECT_EQ(178992, result->getData().size());
}

TEST(AudioFile, MP3File) {
  std::ifstream stream {"tests/resources/AudioFile/rain.mp3", std::ios::binary};
  AudioFile unit {stream};

  auto result = unit.parseSoundData();
  ASSERT_TRUE(result);

  auto format = result->getFormat();
  EXPECT_EQ(2, format.numChannels);
  EXPECT_EQ(44100, format.numSamplesPerSec);
  EXPECT_EQ(16, format.bits);
  EXPECT_EQ(179900, result->getData().size());
}

void dumpSound(const Audio::SoundData& result) {
  std::ofstream stream{"out.bin", std::ios::binary};
  stream.write(result.getData().data(), result.getData().size());
}

void playSound(const Audio::SoundData& result) {
  Audio::Backend backend;
  ASSERT_TRUE(backend.init());

  auto buffer = backend.loadBuffer(result);
  auto emitter = backend.createEmitter();
  emitter.enqueue(buffer);

  int32_t a = 0;
  std::cin >> a;
}

}

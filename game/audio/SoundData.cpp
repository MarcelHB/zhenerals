#include "SoundData.h"

namespace ZH::Audio {

SoundData::SoundData(SoundFormat format, std::vector<char>&& data)
  : format(std::move(format))
  , data(std::move(data))
{}

const std::vector<char>& SoundData::getData() const {
  return data;
}

const SoundFormat& SoundData::getFormat() const {
  return format;
}

}

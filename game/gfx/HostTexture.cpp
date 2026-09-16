// SPDX-License-Identifier: GPL-2.0

#include "HostTexture.h"

namespace ZH::GFX {

HostTexture::HostTexture (
    Size size
  , Format format
  , std::vector<unsigned char>&& data
  , uint32_t numMipMaps
)
  : size(size)
  , format(format)
  , data(std::move(data))
  , numMipMaps(numMipMaps)
{}

const std::vector<unsigned char>& HostTexture::getData() const {
  return data;
}

HostTexture::Format HostTexture::getFormat() const {
  return format;
}

uint32_t HostTexture::getNumMipMaps() const {
  return numMipMaps;
}

Size HostTexture::getSize() const {
  return size;
}

}

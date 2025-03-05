#include "HostTexture.h"

namespace ZH::GFX {

HostTexture::HostTexture (Size size, Format format, std::vector<char>&& data)
  : size(size)
  , format(format)
  , data(std::move(data))
{}

const std::vector<char>& HostTexture::getData() const {
  return data;
}

HostTexture::Format HostTexture::getFormat() const {
  return format;
}

Size HostTexture::getSize() const {
  return size;
}

}

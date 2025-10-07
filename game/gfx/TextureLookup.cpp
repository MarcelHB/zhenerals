// SPDX-License-Identifier: GPL-2.0

#include "../common.h"
#include "../Logging.h"
#include "TextureLookup.h"

namespace ZH::GFX {

TextureLookup::TextureLookup(ResourceLoader& iniLoader) : iniLoader(iniLoader) {}

bool TextureLookup::load() {
  auto fs = iniLoader.getFileStream("data\\ini\\mappedimages\\handcreated\\handcreatedmappedimages.ini");
  if (fs) {
    auto stream = fs->getStream();
    MappedImageINI iniFile {stream};

    auto texturesSubset = iniFile.parse();
    textures.merge(texturesSubset);
  }

  auto it = iniLoader.findByPrefix("data\\ini\\mappedimages\\texturesize_512\\");

  for (; it != iniLoader.cend(); ++it) {
    auto fs = iniLoader.getFileStream(it.key());
    if (!fs) {
      continue;
    }

    auto stream = fs->getStream();
    MappedImageINI iniFile {stream};

    auto texturesSubset = iniFile.parse();
    if (texturesSubset.size() == 0) {
      WARN_ZH("TextureLookup", "INI file empty or broken: {}", it.key());
      continue;
    }

    textures.merge(texturesSubset);
  }

  return true;
}

OptionalCRef<INIImage> TextureLookup::getTexture(const std::string& name) {
  auto it = textures.find(name);
  if (it == textures.cend()) {
    return {};
  }

  return std::make_optional(std::cref(it->second));
}

}

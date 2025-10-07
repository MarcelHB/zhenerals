// SPDX-License-Identifier: GPL-2.0

#include "MappedImageINI.h"

namespace ZH {

MappedImageINI::MappedImageINI(std::istream& instream) : INIFile(instream) {
}

MappedImageINI::MappedImages MappedImageINI::parse() {
  MappedImages mappedImages;

  while (!stream.eof()) {
    parseMappedImage(mappedImages);
  }

  return mappedImages;
}

bool MappedImageINI::parseCoords(INIImage& iniImage) {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return false;
  }

  for (uint8_t i = 0; i < 4; ++i) {
    advanceStream();
    token = getTokenInLine();
    if (token.starts_with("Left:")) {
      iniImage.topLeft.x = parseIntegerFromCoord(token);
    } else if (token.starts_with("Top:")) {
      iniImage.topLeft.y = parseIntegerFromCoord(token);
    } else if (token.starts_with("Right:")) {
      iniImage.bottomRight.x = parseIntegerFromCoord(token);
    } else if (token.starts_with("Bottom:")) {
      iniImage.bottomRight.y = parseIntegerFromCoord(token);
    } else {
      return false;
    }
  }

  return true;
}

uint16_t MappedImageINI::parseIntegerFromCoord(const std::string& value) {
  auto pos = value.find(':');
  if (pos == value.npos) {
    return 0;
  }

  auto intToken = value.substr(pos + 1);
  auto intOptional = parseInteger(intToken);
  if (!intOptional) {
    return 0;
  }

  return *intOptional;
}

bool MappedImageINI::parseMappedImage(MappedImages& mappedImages) {
  auto token = consumeComment();

  if (token != "MappedImage") {
    return false;
  }

  advanceStream();
  auto key = getTokenInLine();
  INIImage iniImage;
  token = consumeComment();

  while (token != "End" && !stream.eof()) {
    if (token == "Texture") {
      if (!parseTexture(iniImage)) {
        return false;
      }
    } else if (token == "TextureWidth") {
      auto value = parseShort();
      if (!value) {
        return false;
      }
      iniImage.size.x = *value;
    } else if (token == "TextureHeight") {
      auto value = parseShort();
      if (!value) {
        return false;
      }
      iniImage.size.y = *value;
    } else if (token == "Coords") {
      if (!parseCoords(iniImage)) {
        return false;
      }
    } else if (token == "Status") {
      advanceStream();
      getTokenInLine(); // =
      advanceStream();
      getTokenInLine(); // NONE
    }

    token = consumeComment();
  }

  mappedImages.emplace(std::move(key), std::move(iniImage));

  return true;
}

bool MappedImageINI::parseTexture(INIImage& iniImage) {
  advanceStream();
  auto token = getTokenInLine();

  if (token != "=") {
    return false;
  }

  advanceStream();
  iniImage.texture = getTokenInLine();

  return true;
}

}

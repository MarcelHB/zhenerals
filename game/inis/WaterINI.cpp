#include "WaterINI.h"

namespace ZH {

WaterINI::WaterINI(std::istream& stream) : INIFile(stream) {}

WaterINI::WaterSettings WaterINI::parse() {
  WaterSettings waterSettings;

  while (!stream.eof()) {
    auto token = consumeComment();
    if (token == "WaterSet") {
      parseWaterSet(waterSettings.waterSets);
    } else if (token == "WaterTransparency") {
      parseWaterTransparency(waterSettings.transparency);
    }
  }

  return waterSettings;
}

void WaterINI::parseWaterSet(WaterSets& waterSets) {
  Water water;

  advanceStream();
  auto key = getTokenInLine();
  auto token = consumeComment();

  while (token != "End" && !stream.eof()) {
    if (token == "SkyTexture") {
      auto texture = parseString();
      if (texture.empty()) {
        return;
      }

      water.skyTexture = std::move(texture);
    } else if (token == "WaterTexture") {
      auto texture = parseString();
      if (texture.empty()) {
        return;
      }

      water.waterTexture = std::move(texture);
    } else if (token == "Vertex00Color") {
      water.v1color = parseRGB();
    } else if (token == "Vertex01Color") {
      water.v2color = parseRGB();
    } else if (token == "Vertex10Color") {
      water.v3color = parseRGB();
    } else if (token == "Vertex11Color") {
      water.v4color = parseRGB();
    } else if (token == "DiffuseColor") {
      water.diffuseColor = parseRGBA();
    } else if (token == "TransparentDiffuseColor") {
      water.transparentDiffuseColor = parseRGBA();
    } else if (token == "WaterRepeatCount") {
      water.waterRepeat = parseShort().value_or(1);
    } else if (token == "UScrollPerMS") {
      water.uPerMs = parseFloat().value_or(0.0f);
    } else if (token == "VScrollPerMS") {
      water.vPerMs = parseFloat().value_or(0.0f);
    } else if (token == "SkyTexelsPerUnit") {
      water.skyTexelsPerUnit = parseFloat().value_or(1.0f);
    }

    token = consumeComment();
  }

  Daytime daytime = Daytime::AFTERNOON;
  if (key == "MORNING") {
    daytime = Daytime::MORNING;
  } else if (key == "EVENING") {
    daytime = Daytime::EVENING;
  } else if (key == "NIGHT") {
    daytime = Daytime::NIGHT;
  }

  waterSets.emplace(daytime, std::move(water));
}

Color WaterINI::parseRGB() {
  auto color = parseRGBA();
  color.a = 0xFF;

  return color;
}

Color WaterINI::parseRGBA() {
  Color color;

  auto values = parseAttributes();
  if (values.contains("R")) {
    color.r = std::min(parseInteger(values["R"]).value_or(0), 255u);
  }
  if (values.contains("G")) {
    color.g = std::min(parseInteger(values["G"]).value_or(0), 255u);
  }
  if (values.contains("B")) {
    color.b = std::min(parseInteger(values["B"]).value_or(0), 255u);
  }
  if (values.contains("A")) {
    color.a = std::min(parseInteger(values["A"]).value_or(255), 255u);
  }

  return color;
}

void WaterINI::parseWaterTransparency(WaterTransparency& transparency) {
  advanceStream();
  auto key = getTokenInLine();
  auto token = consumeComment();

  while (token != "End" && !stream.eof()) {
    if (token == "StandingWaterTexture") {
      auto texture = parseString();
      if (texture.empty()) {
        return;
      }

      transparency.standingWaterTexture = std::move(texture);
    } else if (token == "TransparentWaterMinOpacity") {
      transparency.transparentWaterMinOpacity = parseFloat().value_or(1.0f);
    } else if (token == "TransparentWaterDepth") {
      transparency.transparentWaterDepth = parseFloat().value_or(1.0f);
    } else if (token == "StandingWaterColor") {
      transparency.standingWaterColor = parseRGB();
    } else if (token == "AdditiveBlending") {
      transparency.additiveBlending = parseBool();
    } else if (token == "RadarWaterColor") {
      transparency.radarWaterColor = parseRGB();
    }

    token = consumeComment();
  }
}

}

#include "SoundEffectsINI.h"

namespace ZH {

static SoundEffectControl getControlValueByString(const std::string& value) {
  if (value == "loop") { return SoundEffectControl::LOOP; }
  else if (value == "random") { return SoundEffectControl::RANDOM; }
  else if (value == "all") { return SoundEffectControl::ALL; }
  else if (value == "postdelay") { return SoundEffectControl::POSTDELAY; }
  else if (value == "interrupt") { return SoundEffectControl::INTERRUPT; }
  else { return SoundEffectControl::NONE; }
}

static SoundEffectType getTypeValueByString(const std::string& value) {
  if (value == "ui") { return SoundEffectType::UI; }
  else if (value == "world") { return SoundEffectType::WORLD; }
  else if (value == "shrouded") { return SoundEffectType::SHROUDED; }
  else if (value == "global") { return SoundEffectType::GLOBAL; }
  else if (value == "voice") { return SoundEffectType::VOICE; }
  else if (value == "player") { return SoundEffectType::PLAYER; }
  else if (value == "allies") { return SoundEffectType::ALLIES; }
  else if (value == "enemies") { return SoundEffectType::ENEMIES; }
  else if (value == "everyone") { return SoundEffectType::EVERYONE; }
  else { return SoundEffectType::NONE; }
}

SoundEffectsINI::SoundEffectsINI(std::istream& instream) : INIFile(instream) {
}

SoundEffectsINI::SoundEffects SoundEffectsINI::parse() {
  SoundEffects effects;

  while (!stream.eof()) {
    parseSoundEffect(effects);
  }

  return effects;
}

bool SoundEffectsINI::parseSoundEffect(SoundEffects& effects) {
  auto token = consumeComment();

  if (token != "AudioEvent") {
    return false;
  }

  advanceStream();
  auto key = getToken();
  SoundEffect effect;
  token = consumeComment();

  while (token != "End" && !stream.eof()) {
    if (token == "Volume") {
      auto valueOpt = parsePercent();
      if (!valueOpt) {
        return false;
      }
      effect.volume = *valueOpt;
    } else if (token == "VolumeShift") {
      auto valueOpt = parseSignedShort();
      if (!valueOpt) {
        return false;
      }
      effect.volumeShift = *valueOpt;
    } else if (token == "MinVolume") {
      auto valueOpt = parsePercent();
      if (!valueOpt) {
        return false;
      }
      effect.minVolume = *valueOpt;
    } else if (token == "PitchShift") {
      auto valueOpt = parseSignedShortPair();
      if (!valueOpt || valueOpt->first < -100 || valueOpt->second < valueOpt->first) {
        return false;
      }
      effect.pitchShift = *valueOpt;
    } else if (token == "Delay") {
      auto valueOpt = parseShortPair();
      if (!valueOpt || valueOpt->second < valueOpt->first) {
        return false;
      }
      effect.delay = *valueOpt;
    } else if (token == "Limit") {
      auto valueOpt = parseShort();
      if (!valueOpt) {
        return false;
      }
      effect.limit = *valueOpt;
    } else if (token == "LoopCount") {
      auto valueOpt = parseShort();
      if (!valueOpt) {
        return false;
      }
      effect.loopCount = *valueOpt;
    } else if (token == "Priority") {
      auto valueOpt = parsePriority();
      if (!valueOpt) {
        return false;
      }
      effect.priority = *valueOpt;
    } else if (token == "MinRange") {
      auto valueOpt = parseShort();
      if (!valueOpt) {
        return false;
      }
      effect.minRange = *valueOpt;
    } else if (token == "MaxRange") {
      auto valueOpt = parseShort();
      if (!valueOpt) {
        return false;
      }
      effect.maxRange = *valueOpt;
    } else if (token == "LowPassCutOff") {
      auto valueOpt = parsePercent();
      if (!valueOpt) {
        return false;
      }
      effect.lowPassCutOff = *valueOpt;
    } else if (token == "Sounds") {
      effect.sounds = parseStringList();
    } else if (token == "Attack") {
      effect.attackSounds = parseStringList();
    } else if (token == "Decay") {
      effect.decaySounds = parseStringList();
    } else if (token == "Control") {
      effect.controlFlags = parseEnumBitField<SoundEffectControl>(&getControlValueByString);
    } else if (token == "Type") {
      effect.types = parseEnumBitField<SoundEffectType>(&getTypeValueByString);
    }

    token = consumeComment();
  }

  effects.emplace(std::move(key), std::move(effect));

  return true;
}

std::optional<SoundEffectPriority> SoundEffectsINI::parsePriority() {
  advanceStream();
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getToken();

  if (token == "lowest") {
    return {SoundEffectPriority::LOWEST};
  } else if (token == "low") {
    return {SoundEffectPriority::LOW};
  } else if (token == "normal") {
    return {SoundEffectPriority::NORMAL};
  } else if (token == "high") {
    return {SoundEffectPriority::HIGH};
  } else if (token == "critical") {
    return {SoundEffectPriority::CRITICAL};
  } else {
    return {};
  }
}

}

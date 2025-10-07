// SPDX-License-Identifier: GPL-2.0

#include "Playback.h"
#include "../formats/AudioFile.h"
#include "../Logging.h"

namespace ZH::Audio {

// EVAL language
static std::vector<std::string> soundPrefixes {{
    "data\\audio\\sounds\\"
  , "data\\audio\\sounds\\english\\"
  , "data\\audio\\speech\\"
  , "data\\audio\\speech\\english\\"
}};

Playback::Playback(
    Backend& backend
  , ResourceLoader& iniLoader
  , ResourceLoader& audioLoader
  , size_t capacity
) : backend(backend)
  , audioLoader(audioLoader)
  , iniLoader(iniLoader)
  , capacity(capacity)
{
  auto queueOrder = [](BufferMapIt& a, BufferMapIt& b) {
    return a->second.isDisposable() && !b->second.isDisposable();
  };

  usage = decltype(usage) {queueOrder};
}

bool Playback::load() {
  TRACY(ZoneScoped);

  auto iniFile = iniLoader.getFileStream("data\\ini\\soundeffects.ini");
  if (!iniFile) {
    return false;
  }

  auto stream = iniFile->getStream();
  auto soundEffectsINI = SoundEffectsINI(stream);
  soundEffects = soundEffectsINI.parse();

  if (soundEffects.empty()) {
    return false;
  }

  return true;
}

bool Playback::playSoundEffect(const std::string& iniKey) {
  TRACY(ZoneScoped);
  housekeeping();

  auto iniLookup = soundEffects.find(iniKey);
  if (iniLookup == soundEffects.cend()) {
    return false;
  }

  const auto& soundEffect = iniLookup->second;
  auto soundEmitter = backend.createEmitter();

  std::optional<ResourceLoader::MemoryStream> lookup;

  for (auto& keyName : soundEffect.sounds) {
    auto cacheLookup = soundBuffers.find(keyName);
    if (cacheLookup != soundBuffers.cend()) {
      soundEmitter.enqueue(cacheLookup->second);
      activeEmitters.emplace_back(std::move(soundEmitter));
      continue;
    }

    for (auto& prefix : soundPrefixes) {
      std::string path {prefix};
      path.append(keyName);
      path.append(".wav");
      lookup = audioLoader.getFileStream(path, true);
      if (lookup) {
        break;
      }
    }

    if (!lookup) {
      WARN_ZH("Playback", "Did not find sound effect: {}", keyName);
      continue;
    }

    auto stream = lookup->getStream();
    AudioFile audioFile {stream};
    auto soundData = audioFile.parseSoundData();
    if (!soundData) {
      WARN_ZH("Playback", "Failed to load sound effect: {}", keyName);
      continue;
    }

    auto soundBuffer = backend.loadBuffer(*soundData);
    soundEmitter.enqueue(soundBuffer);
    activeEmitters.emplace_back(std::move(soundEmitter));

    if (soundBuffers.size() >= capacity) {
      tryCleanUpCache();
    }

    auto result = soundBuffers.emplace(std::move(keyName), std::move(soundBuffer));
    if (result.second) {
      usage.push(result.first);
    }
  }

  return true;
}

void Playback::housekeeping() {
  for (auto it = activeEmitters.begin(); it != activeEmitters.end();) {
    if (it->hasFinishedPlaying()) {
      it = activeEmitters.erase(it);
    } else {
      ++it;
    }
  }
}

void Playback::tryCleanUpCache() {
  auto& top = usage.top();
  if (top->second.isDisposable()) {
    usage.pop();
    soundBuffers.erase(top);
  }
}

}

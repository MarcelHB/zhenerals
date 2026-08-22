// SPDX-License-Identifier: GPL-2.0

#include <mutex>

#include "common.h"
#include "MurmurHash.h"
#include "ObjectLoader.h"
#include "ThreadPool.h"

namespace ZH {

ObjectLoader::ObjectLoader(ResourceLoader& iniLoader) : iniLoader(iniLoader) {}

bool ObjectLoader::init() {
  TRACY(ZoneScoped);

  // whitelist as long as every INI file needs to be reviewed
  std::vector<std::string> keys = {
      "data\\ini\\object\\americavehicle.ini"
    , "data\\ini\\object\\chinaair.ini"
    , "data\\ini\\object\\chinavehicle.ini"
    , "data\\ini\\object\\civilianbuilding.ini"
    , "data\\ini\\object\\civilianprop.ini"
    , "data\\ini\\object\\civilianunit.ini"
    , "data\\ini\\object\\factionbuilding.ini"
    , "data\\ini\\object\\glainfantry.ini"
    , "data\\ini\\object\\natureprop.ini"
    , "data\\ini\\object\\techbuildings.ini"
  };

  ThreadPool pool {4};
  std::mutex mutex;

  pool.kickAll([&, this](uint16_t j) {
    TRACY(ZoneScoped);
    for (size_t i = 0; i < keys.size(); ++i) {
      if (i % 4 != j) {
        continue;
      }
      std::optional<ResourceLoader::MemoryStream> fs;
      {
        std::unique_lock<std::mutex> lock {mutex};
        fs = iniLoader.getFileStream(keys[i]);
      }
      if (!fs) {
        continue;
      }

      auto stream = fs->getStream();
      ObjectsINI iniFile {stream};

      auto partialIndex = iniFile.parse();

      {
        std::unique_lock<std::mutex> lock {mutex};
        index.merge(partialIndex);
      }
    }
  });
  pool.waitOnTasks();
  pool.join();

  return true;
}

std::shared_ptr<Objects::ObjectBuilder> ObjectLoader::getObject(const std::string& key) const {
  MurmurHash3_32 hasher;
  hasher.feed(key);

  auto lookup = index.find(hasher.getHash());

  if (lookup == index.cend()) {
    return {};
  } else {
    return lookup->second;
  }
}

}

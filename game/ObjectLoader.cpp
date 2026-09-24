// SPDX-License-Identifier: GPL-2.0

#include <mutex>
#include <queue>

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
      "data\\ini\\object\\americacineunit.ini"
    , "data\\ini\\object\\americainfantry.ini"
    , "data\\ini\\object\\americamiscunit.ini"
    , "data\\ini\\object\\americavehicle.ini"
    , "data\\ini\\object\\chinaair.ini"
    , "data\\ini\\object\\chinacineunit.ini"
    , "data\\ini\\object\\chinamiscunit.ini"
    , "data\\ini\\object\\chinavehicle.ini"
    , "data\\ini\\object\\civilianbuilding.ini"
    , "data\\ini\\object\\civilianprop.ini"
    , "data\\ini\\object\\civilianunit.ini"
    , "data\\ini\\object\\factionbuilding.ini"
    , "data\\ini\\object\\factionunit.ini"
    , "data\\ini\\object\\glainfantry.ini"
    , "data\\ini\\object\\gc_chem_glasystem.ini"
    , "data\\ini\\object\\gc_slth_glasystem.ini"
    , "data\\ini\\object\\glaair.ini"
    , "data\\ini\\object\\glamiscunit.ini"
    , "data\\ini\\object\\hulk.ini"
    , "data\\ini\\object\\lasergeneral.ini"
    , "data\\ini\\object\\natureprop.ini"
    , "data\\ini\\object\\natureunit.ini"
    , "data\\ini\\object\\specialpowerobjects.ini"
    , "data\\ini\\object\\system.ini"
    , "data\\ini\\object\\techbuildings.ini"
  };

  ThreadPool pool = ThreadPool::maxAllowed();
  std::mutex mergeMutex;
  std::mutex pullMutex;

  // C++23
  std::queue<std::string> queue;
  for (auto& key : keys) {
    queue.push(key);
  }

  pool.kickAll([&, this](uint16_t) {
    TRACY(ZoneScoped);
    while (true) {
      std::optional<ResourceLoader::MemoryStream> fs;
      {
        std::unique_lock<std::mutex> lock {pullMutex};
        if (queue.empty()) {
          break;
        }

        auto key = queue.front();
        queue.pop();
        fs = iniLoader.getFileStream(key);
      }
      if (!fs) {
        continue;
      }

      auto stream = fs->getStream();
      ObjectsINI iniFile {stream};

      auto partialIndex = iniFile.parse();

      {
        std::unique_lock<std::mutex> lock {mergeMutex};
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

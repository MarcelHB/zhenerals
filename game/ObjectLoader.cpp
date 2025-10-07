// SPDX-License-Identifier: GPL-2.0

#include <omp.h>

#include "common.h"
#include "MurmurHash.h"
#include "ObjectLoader.h"

namespace ZH {

ObjectLoader::ObjectLoader(ResourceLoader& iniLoader) : iniLoader(iniLoader) {}

bool ObjectLoader::init() {
  TRACY(ZoneScoped);

  std::vector<std::string> keys = {
      "data\\ini\\object\\civilianbuilding.ini"
    , "data\\ini\\object\\civilianprop.ini"
    , "data\\ini\\object\\natureprop.ini"
  };

#pragma omp parallel num_threads(4)
  {
    TRACY(ZoneScoped);
#pragma omp for
    for (size_t i = 0; i < keys.size(); ++i) {
      std::optional<ResourceLoader::MemoryStream> fs;
#pragma omp critical
      {
        fs = iniLoader.getFileStream(keys[i]);
      }
      if (!fs) {
        continue;
      }

      auto stream = fs->getStream();
      ObjectsINI iniFile {stream};

      auto partialIndex = iniFile.parse();

#pragma omp critical
      {
        index.merge(partialIndex);
      }
    }
  }

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

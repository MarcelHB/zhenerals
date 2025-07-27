#include <omp.h>

#include "common.h"
#include "MurmurHash.h"
#include "ObjectLoader.h"

namespace ZH {

ObjectLoader::ObjectLoader(ResourceLoader& iniLoader) : iniLoader(iniLoader) {}

bool ObjectLoader::init() {
  TRACY(ZoneScoped);

  auto it = iniLoader.findByPrefix("data\\ini\\object\\natureprop.ini");
  std::vector<std::string> keys;

  for (; it != iniLoader.cend(); ++it) {
    keys.push_back(it.key());
  }

#pragma omp parallel num_threads(4)
  {
    TRACY(ZoneScoped);
#pragma omp for
    for (size_t i = 0; i < keys.size(); ++i) {
      auto fs = iniLoader.getFileStream(keys[i]);
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

// SPDX-License-Identifier: GPL-2.0

#include "fmt/core.h"

#include "../common.h"
#include "../formats/W3DFile.h"
#include "ModelCache.h"

namespace ZH::GFX {

ModelCache::ModelCache(
    ResourceLoader& resourceLoader
) : resourceLoader(resourceLoader)
{}

ModelCache::Models ModelCache::getModels(const std::string& key) {
  TRACY(ZoneScoped);

  auto path = fmt::format("art\\w3d\\{}.w3d", key);
  auto cacheLookup = modelCache.get(path);
  if (cacheLookup) {
    return cacheLookup;
  }

  auto lookup = resourceLoader.getFileStream(path, true);
  if (!lookup) {
    return {};
  }

  auto stream = lookup->getStream();
  auto w3d = W3DFile(stream);

  auto w3dModels = w3d.parse();
  if (w3dModels.empty()) {
    return {};
  }

  Models models = std::make_shared<std::vector<std::shared_ptr<Model>>>();

  for (auto& w3dModel : w3dModels) {
    auto model =
      std::make_shared<Model>(std::move(Model::fromW3D(*w3dModel)));
    models->emplace_back(std::move(model));
  }
  modelCache.put(path, models);

  return models;
}

}

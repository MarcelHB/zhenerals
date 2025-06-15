#include "fmt/core.h"

#include "../common.h"
#include "../formats/W3DFile.h"
#include "ModelCache.h"

namespace ZH::GFX {

ModelCache::ModelCache(
    ResourceLoader& resourceLoader
  , size_t capacity
) : capacity(capacity)
  , resourceLoader(resourceLoader)
{}

std::shared_ptr<Model> ModelCache::getModel(const std::string& key) {
  TRACY(ZoneScoped);

  auto path = fmt::format("art\\w3d\\{}.w3d", key);
  auto cacheLookup = models.find(path);
  if (cacheLookup != models.cend()) {
    return cacheLookup->second;
  }

  auto lookup = resourceLoader.getFileStream(path, true);
  if (!lookup) {
    return {};
  }

  auto stream = lookup->getStream();
  auto w3d = W3DFile(stream);

  auto w3dModel = w3d.parse();
  if (!w3dModel) {
    return {};
  }
  auto model = Model::fromW3D(*w3dModel);

  if (models.size() >= capacity) {
    tryCleanUpCache();
  }

  auto cachedModel = std::make_shared<Model>(std::move(model));

  auto result = models.emplace(std::move(key), cachedModel);
  if (result.second) {
    usage.push(result.first);
  }

  return cachedModel;
}

void ModelCache::tryCleanUpCache() {
  auto& top = usage.top();
  if (top->second.use_count() == 1) {
    usage.pop();
    models.erase(top);
  }
}

}

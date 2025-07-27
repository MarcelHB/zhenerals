#ifndef H_GAME_GFX_MODEL_CACHE
#define H_GAME_GFX_MODEL_CACHE

#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>

#include "../common.h"
#include "Model.h"
#include "../Cache.h"
#include "../ResourceLoader.h"

namespace ZH::GFX {

class ModelCache {
  public:
    ModelCache(ResourceLoader& resourceLoader);

    std::shared_ptr<Model> getModel(const std::string&);
  private:
    ResourceLoader& resourceLoader;
    Cache<Model> modelCache;
};

}

#endif

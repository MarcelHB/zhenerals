// SPDX-License-Identifier: GPL-2.0

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
    using Models = std::shared_ptr<std::vector<std::shared_ptr<Model>>>;
    ModelCache(ResourceLoader& resourceLoader);

    Models getModels(const std::string&);
  private:
    ResourceLoader& resourceLoader;
    Cache<std::vector<std::shared_ptr<Model>>> modelCache;
};

}

#endif

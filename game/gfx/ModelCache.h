#ifndef H_GAME_GFX_MODEL_CACHE
#define H_GAME_GFX_MODEL_CACHE

#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>

#include "Model.h"
#include "../ResourceLoader.h"

namespace ZH::GFX {

class ModelCache {
  public:
    ModelCache(ResourceLoader& resourceLoader, size_t capacity = 128);

    std::shared_ptr<Model> getModel(const std::string&);
  private:
    size_t capacity;
    ResourceLoader& resourceLoader;
    std::unordered_map<std::string, std::shared_ptr<Model>> models;

    using ModelMapIt = decltype(models)::const_iterator;
    std::priority_queue<
        ModelMapIt
      , std::vector<ModelMapIt>
      , std::function<bool(ModelMapIt&, ModelMapIt&)>
    > usage;

    void tryCleanUpCache();
};

}

#endif

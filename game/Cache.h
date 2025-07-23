#ifndef H_GAME_CACHE
#define H_GAME_CACHE

namespace ZH {

template<typename T>
class Cache {
  public:
    Cache(size_t capacity = 128) : capacity(capacity) {}

  private:
    size_t capacity;

    // EVAL hashing
    std::unordered_map<std::string, std::shared_ptr<T>> cacheEntries;

    using CacheIt = decltype(cacheEntries)::const_iterator;
    struct QueueSort {
      bool operator()(const CacheIt& a, const CacheIt& b) {
        return a->second.use_count() < b->second.use_count();
      }
    };

    std::priority_queue<CacheIt, std::vector<CacheIt>, QueueSort> usage;

  public:
    std::shared_ptr<T> get(const std::string& key) {
      auto lookup = cacheEntries.find(key);
      if (lookup == cacheEntries.cend()) {
        return {};
      }

      return lookup->second;
    }

    void put(std::string key, std::shared_ptr<T> ptr) {
      auto result = cacheEntries.emplace(std::move(key), ptr);

      if (result.second) {
        usage.push(result.first);
      }
    }

  private:
    void tryCleanUpCache() {
      if (cacheEntries.size() < capacity) {
        return;
      }

      auto& top = usage.top();
      if (top->second.use_count() == 1) {
        usage.pop();
        cacheEntries.erase(top);
      }
    }
};

}

#endif

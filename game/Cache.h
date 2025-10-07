// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_CACHE
#define H_GAME_CACHE

#include "common.h"
#include "MurmurHash.h"

namespace ZH {

template<typename T>
class Cache {
  public:
    Cache(size_t capacity = 128) : capacity(capacity) {}

  private:
    size_t capacity;

    std::unordered_map<uint32_t, std::shared_ptr<T>> cacheEntries;

    using CacheIt = decltype(cacheEntries)::const_iterator;
    struct QueueSort {
      bool operator()(const CacheIt& a, const CacheIt& b) {
        return a->second.use_count() < b->second.use_count();
      }
    };

    std::priority_queue<CacheIt, std::vector<CacheIt>, QueueSort> usage;

  public:
    std::shared_ptr<T> get(const std::string& key) {
      auto hash = getCacheKey(key);

      auto lookup = cacheEntries.find(hash);
      if (lookup == cacheEntries.cend()) {
        return {};
      }

      return lookup->second;
    }

    void put(std::string key, std::shared_ptr<T> ptr) {
      auto hash = getCacheKey(key);
      auto result = cacheEntries.emplace(hash, ptr);

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

    uint32_t getCacheKey(const std::string& value) {
      MurmurHash3_32 hasher;
      hasher.feed(value);

      return hasher.getHash();
    }
};

}

#endif

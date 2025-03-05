#include "common.h"
#include "Logging.h"
#include "ResourceLoader.h"

namespace ZH {

ResourceLoader::ResourceLoader(
    const std::vector<fs::path>& paths
  , const std::filesystem::path& basePath
) {
  for (auto& path : paths) {
    std::filesystem::path fullPath {basePath};
    fullPath = fullPath / path;
    if (std::filesystem::exists(fullPath)) {
      bigFiles.emplace_back(std::make_pair(fullPath, State::NEW));
    }
  }
}

ResourceLoader::Iterator ResourceLoader::cend() const {
  return {bigFiles.cend()};
}

ResourceLoader::Iterator ResourceLoader::findByPrefix(const std::string& prefix) {
  openBIGFiles();

  return {prefix, bigFiles.cbegin(), bigFiles.cend()};
}

std::optional<ResourceLoader::MemoryStream> ResourceLoader::getFileStream(const std::string& resource, bool silent) {
  TRACY(ZoneScoped);

  auto cacheIt = lookupCache.find(resource);
  if (cacheIt != lookupCache.cend()) {
    auto& it = cacheIt->second.second;
    auto& bigFile = cacheIt->second.first.get();

    MemoryStream stream;
    bigFile.extract(it, stream.getData(it.size()), 0, it.size());

    return {std::move(stream)};
  }

  openBIGFiles();

  for (auto& bigEntry : bigFiles) {
    auto& bigFile = bigEntry.first;

    auto it = bigFile.find(resource);
    if (it == bigFile.cend()) {
      continue;
    }

    lookupCache.emplace(std::make_pair(resource, std::make_pair(std::ref(bigFile), it)));

    MemoryStream stream;
    bigFile.extract(it, stream.getData(it.size()), 0, it.size());

    return {std::move(stream)};
  }

  if (!silent) {
    WARN_ZH("ResourceLoader", "Could not find resource: {}", resource);
  }
  return {};
}

void ResourceLoader::openBIGFiles() {
  for (auto& bigEntry : bigFiles) {
    auto& bigFile = bigEntry.first;
    if (bigEntry.second == State::NEW) {
      if (!bigFile.open()) {
        bigEntry.second = State::FAILED;
        WARN_ZH("ResourceLoader", "Could not open: {}", bigFile.getPath());
        continue;
      }

      bigEntry.second = State::OPEN;
    }
  }
}

char* ResourceLoader::MemoryStream::getData(uint32_t size) {
  buffer.resize(size);
  return buffer.data();
}

std::istringstream ResourceLoader::MemoryStream::getStream() const {
  return std::istringstream {std::string {buffer.data(), buffer.size()}};
}

size_t ResourceLoader::MemoryStream::size() const {
  return buffer.size();
}

ResourceLoader::Iterator::Iterator(
    const std::string& prefix
  , BIGFiles::const_iterator begin
  , BIGFiles::const_iterator end
) : prefix(prefix)
  , fileIterator(begin)
  , fileEndIterator(end)
  , bigIterator(fileIterator->first.cbegin())
{
  operator++();
}

ResourceLoader::Iterator::Iterator(BIGFiles::const_iterator it) : fileIterator(it) {}

ResourceLoader::Iterator& ResourceLoader::Iterator::operator++() {
  while (true) {
    bool found = false;
    if (bigIterator->key().starts_with(prefix)) {
      found = true;
      currentKey = bigIterator->key();
    }

    ++(*bigIterator);

    if (*bigIterator == fileIterator->first.cend()) {
      ++fileIterator;
      if (fileIterator != fileEndIterator) {
        bigIterator = {fileIterator->first.cbegin()};
      } else {
        break;
      }
    }

    if (found) {
      break;
    }
  }

  return *this;
}

bool ResourceLoader::Iterator::operator==(const Iterator& other) const {
  return fileIterator == other.fileIterator;
}

bool ResourceLoader::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

const std::string& ResourceLoader::Iterator::key() const {
  return currentKey;
}

}

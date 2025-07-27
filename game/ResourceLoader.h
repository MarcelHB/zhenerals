#ifndef H_RESOURCE_LOADER
#define H_RESOURCE_LOADER

#include <filesystem>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "formats/BIGFile.h"
#include "MemoryViewStream.h"

namespace ZH {

class ResourceLoader {
  public:
    enum class State {
        NEW
      , OPEN
      , FAILED
    };
    using BIGFiles = std::vector<std::pair<BIGFile, State>>;

    ResourceLoader(const std::vector<fs::path>&, const std::filesystem::path& path);

    class MemoryStream {
      friend ResourceLoader;

      public:
        MemoryViewStream getStream() const;
        size_t size() const;

      private:
        std::vector<char> buffer;

        char* getData(uint32_t);
    };

    class Iterator {
      friend ResourceLoader;
      private:
        Iterator(
            const std::string&
          , BIGFiles::const_iterator begin
          , BIGFiles::const_iterator end
        );
        Iterator(BIGFiles::const_iterator);

      public:
        Iterator& operator++();
        bool operator==(const Iterator&) const;
        bool operator!=(const Iterator&) const;

        const std::string& key() const;
      private:
        BIGFiles::const_iterator fileIterator;
        BIGFiles::const_iterator fileEndIterator;
        std::optional<BIGFile::Iterator> bigIterator;
        std::string prefix;
        std::string currentKey;
    };

    Iterator cend() const;
    Iterator findByPrefix(std::string);

    std::optional<MemoryStream> getFileStream(std::string, bool silent = false);
  private:
    BIGFiles bigFiles;
    std::unordered_map<std::string, std::pair<std::reference_wrapper<BIGFile>, BIGFile::Iterator>> lookupCache;

    void openBIGFiles();
};

}

#endif

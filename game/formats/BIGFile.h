#ifndef H_BIG_FILE
#define H_BIG_FILE

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "../common.h"

namespace ZH {

namespace fs = std::filesystem;

class BIGFile {
  public:
    BIGFile(fs::path path) : path(std::move(path)) {}

    bool open();
  private:
    struct FileEntry {
      FileEntry() = default;
      FileEntry(uint32_t offset, uint32_t size) : offset(offset), size(size) {}

      uint32_t offset = 0;
      uint32_t size = 0;
    };

    fs::path path;
    std::ifstream file;
  public:
    using IndexT = std::unordered_map<std::string, FileEntry>;

    class Iterator {
      friend BIGFile;

      private:
        Iterator(IndexT::const_iterator it) : it(it) {}

      public:
        Iterator& operator++();
        bool operator==(const Iterator&) const;
        bool operator!=(const Iterator&) const;
        const IndexT::key_type& operator*() const;
        const std::string& key() const;
        uint32_t size() const;

      private:
        IndexT::const_iterator it;
    };

    Iterator cbegin() const;
    Iterator cend() const;
    uint32_t extract(const Iterator& it, char *data, uint32_t offset, uint32_t numBytes);
    Iterator find(const std::string&) const;
    const fs::path& getPath() const;

    static void normalizeEntryName(std::string&);
  private:
    IndexT index;
};

}

#endif

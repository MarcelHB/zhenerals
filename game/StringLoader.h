// SPDX-License-Identifier: GPL-2.0

#ifndef H_STRING_LOADER
#define H_STRING_LOADER

#include "common.h"
#include "formats/CSFFile.h"
#include "ResourceLoader.h"

namespace ZH {

class StringLoader {
  public:
    using StringEntry = CSFFile::StringEntry;

    StringLoader(ResourceLoader&);
    OptionalCRef<StringEntry> getString(const std::string&) const;
    bool load();
  private:
    ResourceLoader& resourceLoader;
    CSFFile::StringMap map;
};

}

#endif

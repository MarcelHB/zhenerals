#include "StringLoader.h"

namespace ZH {

StringLoader::StringLoader(ResourceLoader& resourceLoader) : resourceLoader(resourceLoader)
{}

bool StringLoader::load() {
  if (!map.empty()) {
    return true;
  }

  // EVAL language
  auto lookup = resourceLoader.getFileStream("data\\english\\generals.csf");
  if (!lookup) {
    return false;
  }

  auto stream = lookup->getStream();
  CSFFile csfFile{stream};

  map = csfFile.getStrings();
  if (map.empty()) {
    return false;
  }

  return true;
}

OptionalCRef<StringLoader::StringEntry> StringLoader::getString(const std::string& key) const {
  auto it = map.find(key);
  if (it == map.cend()) {
    return {};
  }

  return std::make_optional(std::cref(it->second));
}

}

// SPDX-License-Identifier: GPL-2.0

#include <algorithm>
#include <array>
#include <vector>

#include "../common.h"
#include "BIGFile.h"

namespace ZH {

static std::array<char, 4> MAGIC_BYTES = {'B', 'I', 'G', 'F'};

static uint32_t BEToHost(uint32_t value) {
  return
    ((value & 0xFF) << 24) |
    ((value & 0xFF00) << 8) |
    ((value & 0xFF0000) >> 8) |
    ((value & 0xFF000000) >> 24);
}

bool BIGFile::open() {
  TRACY(ZoneScoped);

  file = std::ifstream {path, std::ios::binary};

  if (!file.is_open()) {
    return false;
  }

  std::array<char, 4> magicBytes;
  file.read(magicBytes.data(), 4);

  if (file.gcount() != 4 || magicBytes != MAGIC_BYTES) {
    return false;
  }

  uint32_t totalSize = 0;
  file.read(reinterpret_cast<char*>(&totalSize), 4);
  if (file.gcount() != 4) {
    return false;
  }

  uint32_t numFiles = 0;
  file.read(reinterpret_cast<char*>(&numFiles), 4);
  if (file.gcount() != 4) {
    return false;
  }

  numFiles = BEToHost(numFiles);
  file.seekg(4, std::ios::cur);

  std::vector<char> fileNameBuffer;
  fileNameBuffer.reserve(1024);

  for (decltype(numFiles) i = 0; i < numFiles; ++i) {
    std::array<uint32_t, 2> stat; // offset + size

    file.read(reinterpret_cast<char*>(stat.data()), 8);
    if (file.gcount() != 8) {
      return false;
    }

    stat[0] = BEToHost(stat[0]);
    stat[1] = BEToHost(stat[1]);

    uint32_t total = 0;
    bool overflow = __builtin_add_overflow(stat[0], stat[1], &total);
    if (overflow || stat[0] > totalSize || total > totalSize) {
      continue;
    }

    fileNameBuffer.clear();
    char input = 0;
    do {
      input = 0;
      file.read(&input, 1);
      fileNameBuffer.push_back(input);
    } while (input != 0);

    if (file.eof()) {
      return false;
    }

    std::string fileName {fileNameBuffer.data()};
    normalizeEntryName(fileName);
    index.emplace(std::make_pair(fileName, FileEntry {stat[0], stat[1]}));
  }

  return true;
}

BIGFile::Iterator& BIGFile::Iterator::operator++() {
  ++it;
  return *this;
}

bool BIGFile::Iterator::operator==(const Iterator& other) const {
  return it == other.it;
}

bool BIGFile::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

const BIGFile::IndexT::key_type& BIGFile::Iterator::operator*() const {
  return it->first;
}

const std::string& BIGFile::Iterator::key() const {
  return it->first;
}

uint32_t BIGFile::Iterator::size() const {
  return it->second.size;
}

BIGFile::Iterator BIGFile::cbegin() const {
  return {index.cbegin()};
}

BIGFile::Iterator BIGFile::cend() const {
  return {index.cend()};
}

BIGFile::Iterator BIGFile::find(const std::string& key) const {
  std::string lookup {key};
  normalizeEntryName(lookup);

  return {index.find(lookup)};
}

uint32_t BIGFile::extract(const Iterator& it, char *data, uint32_t offset, uint32_t numBytes) {
  if (!file.is_open() || it == cend()) {
    return 0;
  }

  file.seekg(it.it->second.offset + offset, std::ios::beg);
  file.read(data, std::min(numBytes, it.it->second.size - offset));

  return file.gcount();
}

const fs::path& BIGFile::getPath() const {
  return path;
}

void BIGFile::normalizeEntryName(std::string& entry) {
  std::transform(entry.begin(), entry.end(), entry.begin(), [](char c) { return std::tolower(c); });
}

}

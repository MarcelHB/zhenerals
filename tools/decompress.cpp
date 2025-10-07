// SPDX-License-Identifier: GPL-2.0

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif

#include "../game/InflatingStream.h"

static int printHelp() {
  std::cout
    << "Options: decompress <compressed-path>"
    << std::endl;
  return 1;
}

static int printError(uint8_t code) {
  switch (code) {
    case 1:
      std::cerr << "File I/O error. File does not exist or is malformed." << std::endl;
      break;
  }

  return 1;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    return printHelp();
  }

#ifdef _WIN32
    setmode(fileno(stdout), O_BINARY);
#endif

  std::filesystem::path inPath {argv[1]};
  std::ifstream inStream {inPath, std::ios::binary};

  ZH::InflatingStream inflatingStream {inStream};
  auto totalSize = inflatingStream.getInflatedSize();

  std::vector<char> buffer;
  buffer.resize(1024);

  auto bytesRead = 0;
  while (bytesRead < totalSize) {
    auto bytesToRead = std::min(1024U, totalSize - bytesRead);
    buffer.resize(bytesToRead);

    auto actualBytesRead = inflatingStream.read(buffer.data(), bytesToRead);
    if (bytesToRead != actualBytesRead) {
      return printError(1);
    }

    bytesRead += bytesToRead;
    std::copy(buffer.cbegin(), buffer.cend(), std::ostreambuf_iterator<char>(std::cout));
  }

  return 0;
}

// SPDX-License-Identifier: GPL-2.0

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif

#include "../game/formats/BIGFile.h"

static int printHelp() {
  std::cout
    << "Options: x <path> <entry>"
    << "| xall <path> <output-path>"
    << "| ls <path>"
    << std::endl;
  return 1;
}

static int printError(uint8_t code) {
  switch (code) {
    case 1:
      std::cerr << "File I/O error. File does not exist or is malformed." << std::endl;
      break;
    case 2:
      std::cerr << "This entry does not exist." << std::endl;
      break;
    case 3:
      std::cerr << "Output directory does not exist or not a directory." << std::endl;
      break;
  }

  return 1;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    return printHelp();
  }

  std::string command {argv[1]};
  // extract
  if (command == "x") {
#ifdef _WIN32
    setmode(fileno(stdout), O_BINARY);
#endif
    if (argc < 4) {
      return printHelp();
    }

    ZH::BIGFile big {std::filesystem::path {argv[2]}};
    if (!big.open()) {
      return printError(1);
    }

    auto it = big.find(argv[3]);
    if (it == big.cend()) {
      return printError(2);
    }

    std::vector<char> buffer;
    buffer.resize(1024);

    auto bytesRead = 0;
    while (bytesRead < it.size()) {
      auto bytesToRead = std::min(1024U, it.size() - bytesRead);
      buffer.resize(bytesToRead);

      auto actualBytesRead = big.extract(it, buffer.data(), bytesRead, bytesToRead);
      if (bytesToRead != actualBytesRead) {
        return printError(1);
      }

      bytesRead += bytesToRead;
      std::copy(buffer.cbegin(), buffer.cend(), std::ostreambuf_iterator<char>(std::cout));
    }

  } else if (command == "xall") {
    if (argc < 4) {
      return printHelp();
    }

    ZH::BIGFile big {std::filesystem::path {argv[2]}};
    if (!big.open()) {
      return printError(1);
    }

    std::filesystem::path destination = {argv[3]};
    if (!std::filesystem::exists(destination) || !std::filesystem::is_directory(destination)) {
      return printError(3);
    }

    for (auto it = big.cbegin(); it != big.cend(); ++it) {
      auto filename = it.key();
      std::replace(filename.begin(), filename.end(), '\\', '_');
      auto path = destination / filename;

      std::ofstream output (path, std::ios::binary);
      std::vector<char> buffer;
      buffer.resize(1024);

      auto bytesRead = 0;
      while (bytesRead < it.size()) {
        auto bytesToRead = std::min(1024U, it.size() - bytesRead);
        buffer.resize(bytesToRead);

        auto actualBytesRead = big.extract(it, buffer.data(), bytesRead, bytesToRead);
        if (bytesToRead != actualBytesRead) {
          return printError(1);
        }

        bytesRead += bytesToRead;
        output.write(buffer.data(), actualBytesRead);
      }
    }
  // list
  } else if (command == "ls") {
    if (argc < 3) {
      return printHelp();
    }

    ZH::BIGFile big {std::filesystem::path {argv[2]}};
    if (!big.open()) {
      return printError(1);
    }

    for (auto it = big.cbegin(); it != big.cend(); ++it) {
      std::cout << *it << std::endl;
    }
  } else {
    return printHelp();
  }

  return 0;
}

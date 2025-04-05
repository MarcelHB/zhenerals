#include "INIFile.h"

namespace ZH {

INIFile::INIFile(std::istream& instream) : stream(instream) {
  readBuffer.reserve(128);
}

void INIFile::advanceStream() {
  do {
    auto peek = stream.peek();
    if (peek == ' ' || peek == '\n' || peek == '\r') {
      stream.get();
    } else {
      break;
    }
  } while (!stream.eof());
}

std::string INIFile::consumeComment() {
  advanceStream();
  auto token = getToken();

  while (!stream.eof() && token == ";") {
    while (!stream.eof()) {
      auto c = stream.get();
      if (c == '\n') {
        break;
      }
    }
    advanceStream();
    token = getToken();
  }

  return token;
}

std::string INIFile::getToken() {
  readBuffer.clear();

  // get first whatever it is
  auto c = stream.get();
  readBuffer.push_back(c);

  do {
    auto peek = stream.peek();
    if (peek == ' ' || peek == '\n' || peek == '\r') {
      break;
    } else {
      auto c = stream.get();
      readBuffer.push_back(c);
    }
  } while (!stream.eof());

  return {readBuffer.cbegin(), readBuffer.cend()};
}

}


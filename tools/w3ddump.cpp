// SPDX-License-Identifier: GPL-2.0

#include <istream>
#include <iostream>
#include <string>

#include "fmt/core.h"

#include "../game/Config.h"
#include "../game/Logger.h"
#include "../game/ResourceLoader.h"

#define read4() \
  stream.read(reinterpret_cast<char*>(&buffer4), 4); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 4) { \
    return {totalBytes, true}; \
  }

#define read2() \
  stream.read(reinterpret_cast<char*>(&buffer2), 2); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 2) { \
    return {totalBytes, true}; \
  }

#define readf() \
  stream.read(reinterpret_cast<char*>(&bufferf), 4); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 4) { \
    return {totalBytes, true}; \
  }

#define readByte() \
  stream.read(reinterpret_cast<char*>(&bufferByte), 1); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 1) { \
    return {totalBytes, true}; \
  }

#define readStr(n) \
  strBuffer.resize((n)); \
  stream.read(strBuffer.data(), (n)); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != (n)) { \
    return {totalBytes, true}; \
  }

#define readVec3() \
  stream.read(reinterpret_cast<char*>(vec3Buf.data()), 12); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 12) { \
    return {totalBytes, true}; \
  }

#define readRGB() \
  stream.read(reinterpret_cast<char*>(rgbBuf.data()), 3); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 3) { \
    return {totalBytes, true}; \
  }

struct Result {
  Result (size_t bytesRead, bool broken) : bytesRead(bytesRead), broken(broken) {}

  size_t bytesRead = 0;
  bool broken = false;
};

template <typename... ARGS>
void dump(uint16_t depth, fmt::format_string<ARGS...> message, ARGS&& ...args) {
  fmt::print(std::cout, "{}", std::string(depth * 2,  ' '));
  fmt::print(std::cout, message, std::forward<ARGS>(args)...);
  fmt::print(std::cout, "\n");
}

void formatNewlines(std::string& string) {
  auto pos = string.find("\n");
  while (pos != std::string::npos) {
    string[pos] = ' ';
    pos = string.find("\n");
  }

  pos = string.find("\r");
  while (pos != std::string::npos) {
    string[pos] = ' ';
    pos = string.find("\r");
  }
}

struct State {
  size_t numPivots = 0;
  size_t numShaderValues = 0;
};

Result parseChunk(std::istream& stream, uint16_t depth, State& state) {
  uint32_t buffer4 = 0;
  uint16_t buffer2 = 0;
  std::string strBuffer;
  float bufferf = 0.0f;
  std::array<float, 3> vec3Buf;
  std::array<uint8_t, 3> rgbBuf;
  uint8_t bufferByte;

  size_t bytesRead = 0;
  size_t totalBytes = 0;

  read4()
  uint32_t chunkType = buffer4;
  read4()

  bool skip = false;
  bool hasSubchunks = (buffer4 & (1 << 31)) > 0;
  uint32_t chunkSize = (buffer4 & 0x7FFFFFFF);

  auto d1 = depth + 1;
  auto d2 = d1 + 1;

  switch (chunkType) {
    case 0x0:
      dump(depth, "Chunk 0x{:x}: Mesh", chunkType);
      break;
    case 0x2:
      dump(depth, "Chunk 0x{:x}: (Vertices)", chunkType);
      skip = true;
      break;
    case 0x3:
      dump(depth, "Chunk 0x{:x}: (Normals)", chunkType);
      skip = true;
      break;
    case 0xE:
      dump(depth, "Chunk 0x{:x}: (Vertex influences)", chunkType);
      skip = true;
      break;
    case 0x1F:
      dump(depth, "Chunk 0x{:x}: Mesh header", chunkType);

      read4()
      dump(d1, "Version: {}", buffer4);
      read4()
      dump(d1, "Flags: 0x{:x}", buffer4);

      readStr(16);
      dump(d1, "Mesh name: {}", strBuffer);
      readStr(16)
      dump(d1, "Container name: {}", strBuffer);

      read4()
      dump(d1, "# triangles: {}", buffer4);
      read4()
      dump(d1, "# vertices: {}", buffer4);
      read4()
      dump(d1, "# shader materials: {}", buffer4);
      read4()
      dump(d1, "# damage stages: {}", buffer4);
      read4()
      dump(d1, "Sort level: {}", buffer4);
      read4()
      dump(d1, "Prelit version: {}", buffer4);
      read4()
      dump(d1, "Future counts: {}", buffer4);
      read4()
      dump(d1, "Vertex channels: 0x{:x}", buffer4);
      read4()
      dump(d1, "Face channels: 0x{:x}", buffer4);

      readVec3()
      dump(d1, "BBox min: {}, {}, {}", vec3Buf[0], vec3Buf[1], vec3Buf[2]);
      readVec3()
      dump(d1, "BBox max: {}, {}, {}", vec3Buf[0], vec3Buf[1], vec3Buf[2]);

      readVec3()
      dump(d1, "Bounding sphere center: {}, {}, {}", vec3Buf[0], vec3Buf[1], vec3Buf[2]);
      readf()
      dump(d1, "Bounding sphere radius: {}", bufferf);
      break;
    case 0x20:
      dump(depth, "Chunk 0x{:x}: (Triangles)", chunkType);
      skip = true;
      break;
    case 0x22:
      dump(depth, "Chunk 0x{:x}: (Shade indices)", chunkType);
      skip = true;
      break;
    case 0x28:
      dump(depth, "Chunk 0x{:x}: Material info", chunkType);
      read4()
      dump(d1, "# material passes: {}", buffer4);
      read4()
      dump(d1, "# vertex materials: {}", buffer4);
      read4()
      dump(d1, "# shader values: {}", buffer4);
      state.numShaderValues = buffer4;
      read4()
      dump(d1, "# textures: {}", buffer4);
      break;
    case 0x29:
      dump(depth, "Chunk 0x{:x}: Shader values", chunkType);

      for (size_t i = 0; i < state.numShaderValues; ++i) {
        dump(d1, "{}:", i);
        for (size_t j = 0; j < 16; ++j) {
          readByte()
          dump(d2, "{}: {}", j, bufferByte);
        }
      }

      break;
    case 0x2A:
      dump(depth, "Chunk 0x{:x}: (Vertex materials)", chunkType);
      skip = true;
      break;
    case 0x2B:
      dump(depth, "Chunk 0x{:x}: (Vertex material)", chunkType);
      skip = true;
      break;
    case 0x2C:
      dump(depth, "Chunk 0x{:x}: Material name", chunkType);
      readStr(chunkSize - 1);
      dump(d1, "Name: {}", strBuffer);

      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      break;
    case 0x2E:
      dump(depth, "Chunk 0x{:x}: Args 0", chunkType);
      readStr(chunkSize - 1);
      formatNewlines(strBuffer);
      dump(d1, "Args: {}", strBuffer);

      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      break;
    case 0x2F:
      dump(depth, "Chunk 0x{:x}: Args 1", chunkType);
      readStr(chunkSize - 1);
      formatNewlines(strBuffer);
      dump(d1, "Args: {}", strBuffer);

      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      break;
    case 0x2D:
      dump(depth, "Chunk 0x{:x}: Material info", chunkType);
      read4()
      dump(d1, "Attributes: 0x{:x}", buffer4);

      readRGB()
      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      dump(d1, "Ambient: {}, {}, {}", rgbBuf[0], rgbBuf[1], rgbBuf[2]);
      readRGB()
      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      dump(d1, "Diffuse: {}, {}, {}", rgbBuf[0], rgbBuf[1], rgbBuf[2]);
      readRGB()
      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      dump(d1, "Specular: {}, {}, {}", rgbBuf[0], rgbBuf[1], rgbBuf[2]);
      readRGB()
      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      dump(d1, "Emissive: {}, {}, {}", rgbBuf[0], rgbBuf[1], rgbBuf[2]);

      readf()
      dump(d1, "Shininess: {}", bufferf);
      readf()
      dump(d1, "Opacity: {}", bufferf);
      readf()
      dump(d1, "Translucency: {}", bufferf);

      break;
    case 0x30:
      dump(depth, "Chunk 0x{:x}: (Textures)", chunkType);
      skip = true;
      break;
    case 0x31:
      dump(depth, "Chunk 0x{:x}: (Texture)", chunkType);
      skip = true;
      break;
    case 0x32:
      dump(depth, "Chunk 0x{:x}: Texture name", chunkType);
      readStr(chunkSize - 1);
      dump(d1, "Name: {}", strBuffer);

      stream.seekg(1, std::ios::cur);
      totalBytes += 1;
      break;
    case 0x38:
      dump(depth, "Chunk 0x{:x}: (Material pass)", chunkType);
      skip = true;
      break;
    case 0x39:
      dump(depth, "Chunk 0x{:x}: (Vertex material IDs)", chunkType);
      skip = true;
      break;
    case 0x3A:
      dump(depth, "Chunk 0x{:x}: (Shader IDs)", chunkType);
      skip = true;
      break;
    case 0x48:
      dump(depth, "Chunk 0x{:x}: (Texture stage)", chunkType);
      skip = true;
      break;
    case 0x49:
      dump(depth, "Chunk 0x{:x}: (Texture IDs)", chunkType);
      skip = true;
      break;
    case 0x4A:
      dump(depth, "Chunk 0x{:x}: (UV coords)", chunkType);
      skip = true;
      break;
    case 0x100:
      dump(depth, "Chunk 0x{:x}: (Hierarchy)", chunkType);
      break;
    case 0x101:
      dump(depth, "Chunk 0x{:x}: Hierarchy header", chunkType);

      stream.seekg(20, std::ios::cur);
      totalBytes += 20;

      read4()
      dump(d1, "# pivots: {}", buffer4);
      state.numPivots = buffer4;

      readVec3()
      dump(d1, "Center: {}, {}, {}", vec3Buf[0], vec3Buf[1], vec3Buf[2]);
      break;
    case 0x102:
      dump(depth, "Chunk 0x{:x}: Pivots", chunkType);
      for (size_t i = 0; i < state.numPivots; ++i) {
        dump(d1, "{}:", i);

        readStr(16);
        dump(d2, "Name: {}", strBuffer);
        read4()
        dump(d2, "Parent: {}", buffer4);

        readVec3()
        dump(d2, "Translation: {}, {}, {}", vec3Buf[0], vec3Buf[1], vec3Buf[2]);

        // Quaternion
        stream.seekg(28, std::ios::cur);
        totalBytes += 28;
      }
      break;
    case 0x103:
      dump(depth, "Chunk 0x{:x}: (Pivots fixup)", chunkType);
      skip = true;
      break;
    case 0x200:
      dump(depth, "Chunk 0x{:x}: (Animations)", chunkType);
      skip = true;
      break;
    case 0x201:
      dump(depth, "Chunk 0x{:x}: Animation header", chunkType);

      read4()
      dump(d1, "Version: {}", buffer4);
      readStr(16);
      dump(d1, "Name: {}", strBuffer);
      readStr(16);
      dump(d1, "Hierarchy name: {}", strBuffer);
      read4()
      dump(d1, "# frames: {}", buffer4);
      read4()
      dump(d1, "frame rate: {}", buffer4);

      break;
    case 0x202:
      dump(depth, "Chunk 0x{:x}: Animation channel", chunkType);

      read2()
      dump(d1, "1st frame: {}", buffer2);
      read2()
      dump(d1, "Last frame: {}", buffer2);
      read2()
      dump(d1, "Vector length: {}", buffer2);
      read2()
      dump(d1, "Flags: 0x{:x}", buffer2);
      read2()
      dump(d1, "Pivot: {}", buffer2);

      stream.seekg(chunkSize - 10, std::ios::cur);
      totalBytes += chunkSize - 10;
      break;
    case 0x700:
      dump(depth, "Chunk 0x{:x}: (HLOD)", chunkType);
      skip = true;
      break;
    case 0x701:
      dump(depth, "Chunk 0x{:x}: (HLOD header)", chunkType);
      skip = true;
      break;
    case 0x702:
      dump(depth, "Chunk 0x{:x}: (HLOD array)", chunkType);
      skip = true;
      break;
    case 0x703:
      dump(depth, "Chunk 0x{:x}: (HLOD item header)", chunkType);
      skip = true;
      break;
    case 0x704:
      dump(depth, "Chunk 0x{:x}: (HLOD item)", chunkType);
      skip = true;
      break;
    case 0x740:
      dump(depth, "Chunk 0x{:x}: (Chunk box)", chunkType);
      skip = true;
      break;
    default:
      skip = true;
      dump(depth, "Chunk 0x{:x}: UNSUPPORTED", chunkType);
      break;
  }

  if (hasSubchunks) {
    bool broken = false;
    while (bytesRead < chunkSize && !stream.eof() && !broken) {
      auto result = parseChunk(stream, depth + 1, state);
      broken = result.broken;
      auto numBytes = result.bytesRead;

      bytesRead += numBytes;
      totalBytes += numBytes;
    }
  } else if (skip) {
     stream.seekg(chunkSize, std::ios::cur);
     totalBytes += chunkSize;
  }

  if (totalBytes != chunkSize + 8) {
    fmt::print(std::cerr, "Error at chunk 0x{:x}: {} (read) vs. {} (spec.)", chunkType, totalBytes, chunkSize + 8);
    fmt::print(std::cerr, "\n");
    return {totalBytes, true};
  }

  return {totalBytes, false};
}

int main(int argc, char **argv) {
  ZH::Logger logger;
  logger.start();

  if (argc < 2) {
    std::cerr << "Please supply a model name." << std::endl;
    return 1;
  }

  ZH::Config config;
  auto modelLoader =
    std::shared_ptr<ZH::ResourceLoader> {
      new ZH::ResourceLoader {{"W3DZH.big", "ZH_Generals/W3D.big"} , config.baseDir}
    };

  auto path = fmt::format("art\\w3d\\{}.w3d", argv[1]);
  auto lookup = modelLoader->getFileStream(path, true);
  if (!lookup) {
    std::cerr << "Model does not exist." << std::endl;
    return 1;
  }

  bool broken = false;
  auto stream = lookup->getStream();

  State state;
  while (!stream.eof() && !broken) {
    auto result = parseChunk(stream, 0, state);
    broken = result.broken;
  }

  return broken ? 1 : 0;
}

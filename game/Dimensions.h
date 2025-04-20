#ifndef H_DIMENSIONS
#define H_DIMENSIONS

#include <array>
#include <cstdint>

#include <glm/glm.hpp>

namespace ZH {

using Point = glm::ivec2;
using Size = glm::uvec2;

template<typename P, typename S>
struct FlatBox {
  P position;
  S size;
};

using IntFlatBox = FlatBox<glm::ivec2, glm::uvec2>;

}

#endif

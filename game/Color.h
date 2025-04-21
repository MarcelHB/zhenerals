#ifndef H_COLOR
#define H_COLOR

#include <cstdint>

#include <glm/glm.hpp>

namespace ZH {

struct Color {
  using value_type = uint8_t;

  Color() = default;

  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    : r(r), g(g), b(b), a(a)
  {}

  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t a = 255;

  operator glm::vec4() const {
    return { r/255.0f, g/255.0f, b/255.0f, a/255.0f };
  }

  static Color White;
};

using ColorRGBf = glm::vec3;

}

#endif

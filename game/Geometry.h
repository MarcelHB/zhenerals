// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_GEOMETRY
#define H_GAME_GEOMETRY

#include <iterator>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "common.h"
#include "Dimensions.h"
#include "ThreadPool.h"

namespace ZH {

struct Sphere {
  Sphere() = default;
  Sphere(glm::vec3 && pos, float r) : position(std::move(pos)), radius(r) {}

  glm::vec3 position {0.0f};
  float radius = 0.0f;
};

template <typename T>
std::vector<uint8_t> getPointsInPolygon(
    Size size
  , const std::vector<T>& polygonVertices
  , const glm::mat4& conversionMatrix
) {
  TRACY(ZoneScoped);

  std::vector<uint8_t> field;
  field.resize(size.x * size.y);

  // borders
  for (size_t i = 0; i < polygonVertices.size(); ++i) {
    auto& pt1 = polygonVertices[i == 0 ? polygonVertices.size() - 1 : i - 1];
    auto& pt2 = polygonVertices[i];

    auto ptC1 = conversionMatrix * glm::vec4 {pt1.x, pt1.y, 1.0f, 1.0f};
    auto ptC2 = conversionMatrix * glm::vec4 {pt2.x, pt2.y, 1.0f, 1.0f};

    auto drawBorder = [&field, size](int32_t a1, int32_t a2, int32_t b1, int32_t b2, bool switchAxis) {
      float m = (b2 - b1) / (1.0f * (a2 - a1));

      auto op = [a1, a2](int32_t x) -> int32_t {
        if (a2 < a1) {
          return x - 1;
        } else {
          return x + 1;
        }
      };

      auto check = [a1, a2](int32_t x) -> bool {
        if (a2 < a1) {
          return x >= (a2 - a1);
        } else {
          return x <= (a2 - a1);
        }
      };

      T pt;
      for (int32_t x = 0; check(x); x = op(x)) {
        pt.x = a1 + x;
        pt.y = b1 + static_cast<T::value_type>(std::round(m * x));

        if (switchAxis) {
          auto t = pt.x;
          pt.x = pt.y;
          pt.y = t;
        }

        if (pt.y >= 0 && pt.y < static_cast<decltype(pt.x)>(size.y)
            && pt.x >= 0 && pt.x < static_cast<decltype(pt.x)>(size.x)) {
          field[pt.y * size.x + pt.x] = 2;
        }
      }
    };

    auto ptC1x = static_cast<int32_t>(ptC1.x);
    auto ptC1y = static_cast<int32_t>(ptC1.y);
    auto ptC2x = static_cast<int32_t>(ptC2.x);
    auto ptC2y = static_cast<int32_t>(ptC2.y);

    auto dy = std::abs(ptC2y - ptC1y);
    auto dx = std::abs(ptC2x - ptC1x);

    if (dy > dx) {
      drawBorder(ptC1y, ptC2y, ptC1x, ptC2x, true);
    } else {
      drawBorder(ptC1x, ptC2x, ptC1y, ptC2y, false);
    }
  }

  // scan line
  ThreadPool pool {2};
  pool.kickAll([&](uint16_t i) {
    TRACY(ZoneScoped);
    for (size_t y = 0; y < size.y; ++y) {
      if (y % 2 != i) {
        continue;
      }

      uint8_t lastValue = 0;
      size_t startX = 0;

      for (size_t x = 0; x < size.x; ++x) {
        auto value = field[y * size.x + x];
        if (lastValue == 2 && value == 0 && startX == 0) {
          startX = x;
        } else if (lastValue == 2 && value == 0 && startX > 0) {
          for (size_t xb = startX; xb < x; ++xb) {
            field[y * size.x + xb] = 1;
          }
          startX = 0;
        }

        lastValue = value;
      }
    }
  });
  pool.waitOnTasks();
  pool.join();

  return field;
}
// returns height (y) of `pos` between these vertices
float interpolateVertexTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec2& pos);

// returns a sphere covering all spheres internally, supposedly minimal?
template<std::input_iterator It> requires std::convertible_to<std::iter_value_t<It>, Sphere>
Sphere getSphereFromSpheres(It start, It end) {
  Sphere sphere;
  // min/max positions for X, Y, Z
  std::array<glm::vec3, 6> extremePositions;

  std::array<glm::vec3, 3> units;
  units[0] = glm::vec3 { 1.0f, 0.0f, 0.0f };
  units[1] = glm::vec3 { 0.0f, 1.0f, 0.0f };
  units[2] = glm::vec3 { 0.0f, 0.0f, 1.0f };

  bool first = true;

  auto current = start;
  while (current != end) {
    auto minX = (current->position - current->radius * units[0]);
    auto maxX = (current->position + current->radius * units[0]);
    auto minY = (current->position - current->radius * units[1]);
    auto maxY = (current->position + current->radius * units[1]);
    auto minZ = (current->position - current->radius * units[2]);
    auto maxZ = (current->position + current->radius * units[2]);

    if (first) {
      extremePositions[0] = minX;
      extremePositions[1] = maxX;
      extremePositions[2] = minY;
      extremePositions[3] = maxX;
      extremePositions[4] = minZ;
      extremePositions[5] = maxZ;

      first = false;
      current++;
      continue;
    }

    if (minX.x < extremePositions[0].x) {
      extremePositions[0] = minX;
    }
    if (maxX.x > extremePositions[1].x) {
      extremePositions[1] = maxX;
    }
    if (minY.y < extremePositions[2].y) {
      extremePositions[2] = minY;
    }
    if (maxY.y > extremePositions[3].y) {
      extremePositions[3] = maxY;
    }
    if (minZ.z < extremePositions[4].z) {
      extremePositions[4] = minZ;
    }
    if (maxZ.z > extremePositions[5].z) {
      extremePositions[5] = maxZ;
    }

    current++;
  }

  sphere.position = {
      extremePositions[0].x + (extremePositions[1].x - extremePositions[0].x) / 2.0f
    , extremePositions[2].y + (extremePositions[3].y - extremePositions[2].y) / 2.0f
    , extremePositions[4].z + (extremePositions[5].z - extremePositions[4].z) / 2.0f
  };
  sphere.radius =
    std::max(
        glm::length(extremePositions[1] - sphere.position)
      , std::max(
            glm::length(extremePositions[3] - sphere.position)
          , glm::length(extremePositions[5] - sphere.position)
        )
    );

  return sphere;
}

}

#endif

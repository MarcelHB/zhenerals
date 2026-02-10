// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_GEOMETRY
#define H_GAME_GEOMETRY

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "common.h"
#include "Dimensions.h"

namespace ZH {

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

        if (pt.y >= 0 && pt.y < size.y && pt.x >= 0 && pt.x < size.x) {
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
#pragma omp parallel num_threads(2)
  {
    TRACY(ZoneScoped);
#pragma omp for
    for (size_t y = 0; y < size.y; ++y) {
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
  }

  return field;
}

}

#endif

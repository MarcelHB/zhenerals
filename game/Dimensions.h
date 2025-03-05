#ifndef H_DIMENSIONS
#define H_DIMENSIONS

#include <array>
#include <cstdint>

namespace ZH {

template <typename T>
struct PointT {
  using underlying_type = T;
  PointT() = default;
  PointT(T x, T y) : x(x), y(y) {}
  PointT(const std::array<T, 2>& list) : x(list[0]), y(list[1]) {}

  T x = 0;
  T y = 0;

  PointT& operator+=(const PointT& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  PointT operator+(const PointT& other) const {
    PointT p(*this);
    p.x += other.x;
    p.y += other.y;
    return p;
  }
};

template <typename T>
struct SizeT {
  using underlying_type = T;
  SizeT() = default;
  SizeT(T w, T h) : w(w), h(h) {}
  SizeT(const std::array<T, 2>& list) : w(list[0]), h(list[1]) {}
  T w = 0;
  T h = 0;
};

using Point = PointT<int32_t>;
using Size = SizeT<uint32_t>;

}

#endif

#ifndef H_COMMON
#define H_COMMON

#define USE_TRACY_MEMORY 1

#if defined(USE_TRACY) && !defined(NO_TRACY)
  #if defined(USE_TRACY_MEMORY)
#include <cstddef>
#include <new>
void* operator new(size_t count);
void operator delete(void* ptr, std::size_t sz) noexcept;
    #define TRACY_ON_DEMAND 1
  #endif

  #define TRACY_ENABLE 1
  #include <tracy/Tracy.hpp>
  #define TRACY(x) x
#else
  #define TRACY(x)
#endif

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ZH {

template <typename T>
using OptionalRef = std::optional<std::reference_wrapper<T>>;
template <typename T>
using OptionalCRef = std::optional<std::reference_wrapper<const T>>;
using OptionalString = std::optional<std::string>;

template <typename T>
std::optional<std::reference_wrapper<const T>> optToCRef(const std::optional<T>& t) {
  if (t) {
    return std::optional<std::reference_wrapper<const T>>(std::cref(*t));
  } else {
    return {};
  }
}

template<template <typename> class T, typename P>
std::optional<std::reference_wrapper<P>> ptrToOptional(T<P>& ptr) {
  if (ptr) {
    return std::make_optional(std::ref(*ptr));
  } else {
    return {};
  }
}

template<template <typename> class T, typename P>
std::optional<std::reference_wrapper<const P>> ptrToCOptional(const T<P>& ptr) {
  if (ptr) {
    return std::make_optional(std::cref(*ptr));
  } else {
    return {};
  }
}

std::vector<char> readFile(const std::filesystem::path& path);

template <typename T>
class BitField {
  public:
    using ValueType = std::underlying_type_t<T>;

    BitField() = default;
    BitField(T t) : value(static_cast<ValueType>(t)) {}
    BitField(ValueType t) : value(t) {}

    operator bool() const {
      return value != 0;
    }

    BitField<T> operator&(T t) const {
      return BitField<T>(value & static_cast<ValueType>(t));
    }

    BitField<T> operator|(T t) const {
      return BitField<T>(value | static_cast<ValueType>(t));
    }

    BitField<T> unset(T t) const {
      return BitField<T>(value & ~static_cast<ValueType>(t));
    }

    BitField<T>& operator|=(T t) {
      value |= static_cast<ValueType>(t);
      return *this;
    }
  private:
    ValueType value = 0;
};

}

#endif

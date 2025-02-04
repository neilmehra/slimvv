#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <utility>

// replace type storage w/ bitmap that stores types using min # of types
// so if we had variant<int, short, long> -> 00, 01, 10
// and we had an int, short, long it would be
// 000110 stored
namespace vv5 {

namespace detail {

template <std::size_t I, class Q, class T, class... Rest>
constexpr std::size_t find_type_in_pack() {
  if (std::same_as<std::decay_t<Q>, std::decay_t<T>>) {
    return I;
  } else {
    static_assert(sizeof...(Rest) > 0);
    return find_type_in_pack<I + 1, Q, Rest...>();
  }
}

template <class T> constexpr std::size_t get_padding(std::uintptr_t addr) {
  std::size_t align = alignof(T);
  std::size_t aligned_addr = (addr + (align - 1)) & ~(align - 1);
  return aligned_addr - addr;
}

// assumes there is enough space allocated at `addr`
template <class T>
constexpr std::size_t copy_to(std::uintptr_t* addr, const void* const p) {
  std::uintptr_t u_addr = reinterpret_cast<std::uintptr_t>(addr);
  std::size_t padding = get_padding<T>(addr);
  const auto& data = *reinterpret_cast<const T* const>(p);
  ::new (addr + padding) T(data);
  return padding;
}

// assumes there is enough space allocated at `addr`
template <class T>
constexpr std::size_t move_to(std::uintptr_t* addr, void* const p) {
  std::uintptr_t u_addr = reinterpret_cast<std::uintptr_t>(addr);
  std::size_t padding = get_padding<T>(addr);
  const auto& data = *reinterpret_cast<const T* const>(p);
  ::new (addr + padding) T(std::move(data));
  return padding;
}

template <std::size_t Size>
constexpr std::size_t read_bits(const std::byte* const addr) {}

} // namespace detail
} // namespace vv5

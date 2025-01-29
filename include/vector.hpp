#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
namespace vv {

namespace v0 {

// memory footprint: 16 bytes for std::vector + std::vector::capacity *
// (max({sizeof(Types)...}) + 8) performance: no heap allocs, O(1) index
template <class... Types> using vector = std::vector<std::variant<Types...>>;
} // namespace v0

namespace v1 {

} // namespace v1

template <class... Types> class vector {
public:
private:
  std::size_t capacity;
  std::size_t size;
  void* data;
  std::size_t* offsets;
  void* types;
};
} // namespace vv

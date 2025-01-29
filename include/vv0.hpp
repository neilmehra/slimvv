#pragma once

#include <variant>
#include <vector>

namespace v0 {

// memory footprint: 16 bytes for std::vector + std::vector::capacity *
// (max({sizeof(Types)...}) + 8 (type index))
// performance: no heap allocs, O(1) index
template <class... Types> using vector = std::vector<std::variant<Types...>>;
} // namespace v0

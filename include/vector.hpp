#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>
#include <variant>
#include <vector>
namespace vv {

namespace v0 {

// memory footprint: 16 bytes for std::vector + std::vector::capacity *
// (max({sizeof(Types)...}) + 8) performance: no heap allocs, O(1) index
template <class... Types> using vector = std::vector<std::variant<Types...>>;
} // namespace v0

namespace v1 {

namespace detail {

template <std::size_t, class...> struct var_index {};

template <std::size_t I, class T1, class... TN>
struct var_index<I, T1, TN...> : public var_index<I - 1, TN...> {};

template <class T1, class... TN> struct var_index<0, T1, TN...> {
  using type = T1;
};

template <std::size_t I, class... TN>
using var_index_t = typename var_index<I, TN...>::type;

template <class, class...>
struct var_find_type
    : std::integral_constant<int, std::numeric_limits<int>::min()> {};

template <class T, class U, class... UN>
struct var_find_type<T, U, UN...>
    : public std::integral_constant<int, 1 + var_find_type<T, UN...>::value> {};

template <class T, class... UN>
struct var_find_type<T, T, UN...> : public std::integral_constant<int, 0> {};

template <class T, class... UN>
constexpr int var_find_type_v = var_find_type<T, UN...>::value;

} // namespace detail

template <class... Types> class variant {
public:
  variant() : type_index(0), data(new detail::var_index_t<0, Types...>()) {}

  variant(const variant& rhs) : type_index(rhs.type_index), data(rhs.get_p()) {}

  variant(variant&& rhs) noexcept : type_index(rhs.type_index) {
    data = rhs.data;
    rhs.data = nullptr;
  }

  variant& operator=(const variant& rhs) {
    if (this != &rhs) {
      type_index = rhs.type_index;
      data = new detail::var_index_t<rhs.type_index, Types...>(rhs.data);
    }
    return *this;
  }

  variant& operator=(variant&& rhs) noexcept {
    if (this != &rhs) {
      type_index = rhs.type_index;
      data = rhs.data;
      rhs.data = nullptr;
    }
    return *this;
  }

  ~variant() noexcept { delete get_p(); }

  template <class U> U& get() const { return *static_cast<U*>(data); }

  template <std::size_t I> auto& get() const { return *get_p(); }

  auto* get_p() const {
    return static_cast<detail::var_index_t<type_index, Types...>*>(data);
  }

  std::size_t index() const noexcept { return type_index; }

private:
  std::size_t type_index;
  void* data;
};

template <class... Types> class vector {
public:
  vector() : capacity(0), size(0) {}

private:
  std::size_t capacity;
  std::size_t size;
  void** data;
};

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

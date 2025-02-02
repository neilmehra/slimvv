#pragma once

#include <cstddef>
#include <utility>

template <class T>
concept bruh = requires(T a) {
  { a + a } -> std::same_as<T>;
};

namespace vv5 {

using dtor_fptr_t = void (*)(std::byte* const);
using cm_fptr_t = void (*)(std::byte* const, const std::byte* const);

template <class U> void destroy_impl(std::byte* const p) {
  reinterpret_cast<U*>(p)->~U();
}

template <class U>
void copy_impl(std::byte* const loc, const std::byte* const p) {
  ::new (loc) U(*reinterpret_cast<const U* const>(p));
}

template <class U>
void move_impl(std::byte* const loc, const std::byte* const p) {
  ::new (loc) U(std::move(*reinterpret_cast<U*>(const_cast<std::byte*>(p))));
}

template <class... Types> class TypeBitmask {
public:
  void reserve(std::size_t bytes) {
    std::byte* new_types = new std::byte[bytes];
    for (std::size_t i = 0; i < bytes; i++) {
      new_types[i] = types[i];
    }
    types = new_types;
    delete[] new_types;
  }

private:
  static constexpr std::size_t N = sizeof...(Types);
  static constexpr std::size_t bnum = __builtin_clz(N);

  static constexpr dtor_fptr_t dtable[N]{destroy_impl<Types>...};
  static constexpr cm_fptr_t ctable[N]{copy_impl<Types>...};
  static constexpr cm_fptr_t mtable[N]{move_impl<Types>...};

  std::byte* types;

  template <std::size_t I, class U, class T, class... TN>
  [[nodiscard]] std::size_t find_type_index() const;
};

template <class... Types>
template <std::size_t I, class U, class T, class... TN>
std::size_t TypeBitmask<Types...>::find_type_index() const {
  if constexpr (std::is_same_v<std::decay_t<U>, std::decay_t<T>>) {
    return I;
  } else {
    return find_type_index<I + 1, U, TN...>;
  }
}

} // namespace vv5

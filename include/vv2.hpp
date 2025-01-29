#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace vv2 {

namespace detail {

using copy_fptr = void* (*)(const void* const);
template <class T> static void* copy_impl(const void* const p) {
  return new T(*static_cast<const T* const>(p));
}

// ok
// at the very least, we know that we can find what type is at index i
// now we need some way of finding what index is type k at
template <std::size_t Cur, class T, class... TN> struct TypeByIndex {
  using type = T;
  using next_type = TypeByIndex<Cur + 1, TN...>;
  static bool flag;

  static void execute(std::size_t index) {
    flag = false;
    if (index == Cur) {
      flag = true;
    }
  }
};

template <std::size_t Cur, class T> struct TypeByIndex<Cur, T> {
  using type = T;
  static bool flag;

  static void execute(std::size_t index) { flag = (index == Cur); }
};

template <class U, class... TN> struct IndexByType {
  static void execute(const char* name) {
    if (name == typeid(U).name()) {

    } else {
      IndexByType<TN...>::execute(name);
    }
  }
};

template <class U> struct IndexByType<U> {
  static void execute(const char* name) {
    if (name == typeid(U).name()) {

    } else {
      assert(false);
    }
  }
};

template <class U, class... Us> struct TypeIndex;

template <class U> struct TypeIndex<U> {};

template <class U, class First, class... Rest>
struct TypeIndex<U, First, Rest...> {
  static constexpr std::size_t value =
      std::is_same_v<U, First> ? 0 : 1 + TypeIndex<U, Rest...>::value;
};

template <class U, class... Us>
static constexpr std::size_t TypeIndexV = TypeIndex<U, Us...>::value;

} // namespace detail

template <class... Types> class vector {
public:
  static constexpr std::size_t N = sizeof...(Types);

  // void handleByIndex(std::size_t index) {
  //   RuntimeType<0, Types...>::execute(index);
  // }

  std::size_t size;
  std::size_t capacity;
  void** data;
  std::size_t* ptr;
  std::size_t* types;
};

template <class... Types> class vector2 {
public:
  static constexpr std::size_t N = sizeof...(Types);
  using tt = std::tuple<std::type_identity<Types>...>;

  int test() {
    tt t{};
    return sizeof(tt);
  }

  std::size_t size;
  std::size_t capacity;
  void** data;
  std::size_t* offsets;
  std::size_t* types;
};

template <class... Types> struct Vec {
  Vec()
      : size(0), capacity(0), data(nullptr), offsets(nullptr), types(nullptr) {}

  ~Vec() {
    delete[] data;
    delete[] offsets;
    delete[] types;
  }

  void reserve(std::size_t new_cap) {
    if (new_cap > capacity) {
      capacity = new_cap;
      void** new_data = new void*[capacity];
      std::size_t* new_offsets = new std::size_t[capacity];
      std::size_t* new_types = new std::size_t[capacity];
      for (int i = 0; i < size; i++) {
        new_data[i] = data[i];
        new_offsets[i] = offsets[i];
        new_types[i] = types[i];
      }
      std::swap(new_data, data);
      std::swap(new_offsets, offsets);
      std::swap(new_types, types);
    }
  }

  template <class U> void push_back(const U& u) {}

  void ensure_size() { reserve(2 * capacity + 1); }

  template <class U> U& operator[](std::size_t index) {
    auto chk = detail::TypeByIndex<0, Types...>{};
    chk.execute(index);
    bool done = chk.flag;
    while (!done) {
      chk = decltype(chk)::next_type;
      done = chk.flag;
    }
  }

  std::size_t size;
  std::size_t capacity;
  void** data;
  std::size_t* offsets;
  std::size_t* types;
};

// ok
// what specifically do we need?
// when we index, we need to know how much to read, and we need to have offsets
// and we need to know what type it is, to convert back?
//
// talk says store an index that corresponds to the type

} // namespace vv2

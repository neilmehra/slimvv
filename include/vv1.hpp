#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

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

  ~variant() { destroy_data(); }

  variant(const variant& rhs) { copy_from(rhs); }

  variant(variant&& rhs) { move_from(rhs); }

  variant& operator=(const variant& rhs) {
    copy_from(rhs);
    return *this;
  }

  variant& operator=(variant&& rhs) noexcept {
    move_from(rhs);
    return *this;
  }

  template <class U> variant(const U& rhs) {
    constexpr int u_index = detail::var_find_type_v<U, Types...>;
    assert(u_index >= 0);
    type_index = static_cast<std::size_t>(u_index);
    data = static_cast<void*>(new U{std::forward<U>(rhs)});
  }

  // if non-const T& is passed to this, then U = T&. Can't allocate a ref, so we
  // need to decay it
  // this gets choosen during template overload resolution if non-const ref to
  // variant<Types...> gets passed in, so need to disable if matches
  template <class U, typename = std::enable_if_t<
                         !std::is_same_v<std::decay_t<U>, variant<Types...>>>>
  variant(U&& rhs) {
    using Udec = std::decay_t<U>;
    constexpr int u_index = detail::var_find_type_v<Udec, Types...>;
    assert(u_index >= 0);
    type_index = static_cast<std::size_t>(u_index);
    data = static_cast<void*>(new Udec(std::move(rhs)));
  }

  template <class U, class... Args>
  variant(std::in_place_type_t<U>, Args&&... args) {
    constexpr int u_index = detail::var_find_type_v<U, Types...>;
    assert(u_index >= 0);
    type_index = static_cast<std::size_t>(u_index);
    data = static_cast<void*>(new U(std::forward<Args>(args)...));
  }

  // Observers
  std::size_t index() const noexcept { return type_index; }

  template <class U> const U& get() const {
    return *static_cast<const U*>(data);
  }

  template <class U> U& get() { return *static_cast<U*>(data); }

  template <std::size_t I> auto& get() const {
    static_assert(I == type_index);
    using T = detail::var_index_t<I, Types...>;
    return *static_cast<T*>(data);
  }

  template <std::size_t I> const auto& get() const {
    static_assert(I == type_index);
    using T = detail::var_index_t<I, Types...>;
    return *static_cast<const T*>(data);
  }

private:
  static constexpr std::size_t N = sizeof...(Types);
  std::size_t type_index;
  void* data;

  using destructor_fptr = void (*)(void*);
  template <class T> static void destroy_impl(void* p) {
    delete static_cast<T*>(p);
  }

  using copy_fptr = void* (*)(const void* const);
  template <class T> static void* copy_impl(const void* const p) {
    return new T(*static_cast<const T* const>(p));
  }

  using move_fptr = void* (*)(const void* const);
  template <class T> static void* move_impl(const void* const p) {
    return new T(std::move(*static_cast<const T* const>(p)));
  }

  static constexpr destructor_fptr dtable[N] = {destroy_impl<Types>...};
  static constexpr copy_fptr ctable[N] = {copy_impl<Types>...};
  static constexpr move_fptr mtable[N] = {move_impl<Types>...};

  void destroy_data() {
    dtable[type_index](data);
    data = nullptr;
  }

  void copy_from(const variant& rhs) {
    if (this != &rhs) {
      if (!rhs.data) {
        data = nullptr;
        return;
      }
      data = ctable[rhs.type_index](rhs.data);
      type_index = rhs.type_index;
    }
  }

  void move_from(variant& rhs) {
    if (this != &rhs) {
      if (!rhs.data) {
        data = nullptr;
        return;
      }
      data = mtable[rhs.type_index](rhs.data);
      type_index = rhs.type_index;
      rhs.data = nullptr;
    }
  }
};

template <class... Types> class vector {
public:
  using value_type = variant<Types...>;

  vector() : capacity_(0), size_(0), data_(nullptr) {}

  vector(const vector& rhs)
      : size_(rhs.size_), capacity_(rhs.capacity_),
        data_(new value_type[capacity_]) {
    for (std::size_t i = 0; i < rhs.size_; i++) {
      data_[i] = rhs.data_[i];
    }
  }

  vector(vector&& rhs)
      : size_(rhs.size_), capacity_(rhs.capacity_), data_(rhs.data_) {
    rhs.data_ = nullptr;
  }

  vector& operator=(const vector& rhs) {
    if (this != &rhs) {
      size_ = rhs.size_;
      reserve(capacity_);
      for (std::size_t i = 0; i < rhs.size_; i++) {
        data_[i] = rhs.data_[i];
      }
      capacity_ = rhs.capacity_;
    }
    return *this;
  }

  vector& operator=(vector&& rhs) {
    if (this != &rhs) {
      size_ = rhs.size_;
      capacity_ = rhs.capacity_;
      data_ = rhs.data_;
      rhs.data_ = nullptr;
    }
    return *this;
  }

  ~vector() noexcept { delete[] data_; }

  template <class U> void push_back(const U& rhs) {
    ensure_size();
    data_[size_++] = value_type(rhs);
  }

  void push_back(const value_type& v) {
    ensure_size();
    data_[size_++] = v;
  }

  template <class U> void push_back(U&& rhs) {
    ensure_size();
    data_[size_++] = value_type(std::move(rhs));
  }

  template <class U> void push_back(value_type&& v) {
    ensure_size();
    data_[size_++] = value_type(std::move(v));
  }

  template <class U, class... Args>
  void emblace_back(std::in_place_type_t<U> in_place, Args&&... args) {
    ensure_size();
    data_[size_++] = value_type(in_place, std::forward<Args>(args)...);
  }

  constexpr void reserve(std::size_t new_cap) {
    if (new_cap > capacity_) {
      value_type* _data = new value_type[new_cap];
      for (std::size_t i = 0; i < capacity_; i++) {
        _data[i] = data_[i];
      }
      std::swap(data_, _data);
      capacity_ = new_cap;
    }
  }

  [[nodiscard]] constexpr value_type& operator[](std::size_t index) {
    return data_[index];
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }

  // [[nodiscard]] constexpr std::size_t size() const noexcept {
  //   return size;
  // }

private:
  std::size_t capacity_;
  std::size_t size_;
  value_type* data_;

  void ensure_size() {
    if (capacity_ == size_)
      reserve(2 * capacity_ + 1);
  }
};

} // namespace v1

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace vv3 {

struct Element {
  std::size_t type_index;
  std::byte* data;
};

constexpr std::size_t get_padding(std::uintptr_t addr, std::size_t align) {
  std::size_t aligned_addr = (addr + (align - 1)) & ~(align - 1);
  return aligned_addr - addr;
}

template <class... Types> class vector {
public:
  vector();

  ~vector();

  vector(const vector& rhs);

  vector(vector&& rhs);

  vector& operator=(const vector& rhs);

  vector& operator=(vector&& rhs);

  void reserve_entries(std::size_t new_entries);

  void reserve_cap(std::size_t new_cap);

  template <class U> void push_back(const U& u);

private:
  // sfinae this to stop the universal rref push_back from competing in overload
  // resolution if U is lvalue
  template <class U, class = std::enable_if_t<!std::is_lvalue_reference_v<U>>>
  using rval_ref = U&&;

public:
  template <class U> void push_back(rval_ref<U> u);

  [[nodiscard]] Element operator[](std::size_t index);

  template <class U> [[nodiscard]] U& get(std::size_t index);

  [[nodiscard]] std::size_t size() const noexcept { return size_; }

private:
  static constexpr std::size_t N = sizeof...(Types);
  using dtor_fptr_t = void (*)(std::byte* const);
  using cm_fptr_t = void (*)(std::byte* const, const std::byte* const);

  template <class U> static void destroy_impl(std::byte* const p) {
    reinterpret_cast<U*>(p)->~U();
  }

  template <class U>
  static void copy_impl(std::byte* const loc, const std::byte* const p) {
    ::new (loc) U(*reinterpret_cast<const U* const>(p));
  }

  template <class U>
  static void move_impl(std::byte* const loc, const std::byte* const p) {
    ::new (loc) U(std::move(*reinterpret_cast<U*>(const_cast<std::byte*>(p))));
  }

  static constexpr dtor_fptr_t dtable[N]{destroy_impl<Types>...};
  static constexpr cm_fptr_t ctable[N]{copy_impl<Types>...};
  static constexpr cm_fptr_t mtable[N]{move_impl<Types>...};

  std::size_t size_;
  std::size_t capacity;
  std::size_t entries;

  std::byte* data;
  std::size_t* offsets;
  std::size_t* type_size;
  std::size_t* type_index;
  std::size_t* type_align;

  template <std::size_t I, class U, class T, class... TN>
  [[nodiscard]] std::size_t find_type_index() const;

  void place_obj(std::size_t index, const std::byte* const p,
                 cm_fptr_t place_func);

  void delete_data();

  void reset();
};

template <class... Types>
vector<Types...>::vector()
    : size_(0), capacity(0), entries(0), data(nullptr), offsets(nullptr),
      type_size(nullptr), type_index(nullptr), type_align(nullptr) {}

template <class... Types> vector<Types...>::~vector() { delete_data(); }

// have to set size_ to 0 first, otherwise reserve_entries would access
// non-owned memory
template <class... Types> vector<Types...>::vector(const vector& rhs) {
  reset();                      // bad practice?
  reserve_entries(rhs.entries); // handles initialization of entries, offsets,
                                // type_size, type_index, type_align
  for (std::size_t i = 0; i < rhs.size_; i++) {
    type_size[i] = rhs.type_size[i];
    type_index[i] = rhs.type_index[i];
    type_align[i] = rhs.type_align[i];
    place_obj(
        i, rhs.data + rhs.offsets[i],
        ctable[type_index[i]]); // capacity will automatically grow to handle
                                // new offsets (if alignment requires it)
  }
}

template <class... Types>
vector<Types...>::vector(vector&& rhs)
    : size_(rhs.size_), capacity(rhs.capacity), entries(rhs.entries),
      data(rhs.data), offsets(rhs.offsets), type_size(rhs.type_size),
      type_index(rhs.type_index), type_align(rhs.type_align) {
  rhs.reset();
}

template <class... Types>
vector<Types...>& vector<Types...>::operator=(const vector& rhs) {
  if (this != &rhs) {
    delete_data();
    reset();

    reserve_entries(rhs.entries); // handles initialization of entries, offsets,
                                  // type_size, type_index, type_align
    for (std::size_t i = 0; i < rhs.size_; i++) {
      type_size[i] = rhs.type_size[i];
      type_index[i] = rhs.type_index[i];
      type_align[i] = rhs.type_align[i];
      place_obj(i, rhs.data + rhs.offsets[i],
                ctable[type_index[i]]); // capacity will automatically grow to
                                        // handle new offsets (if alignment
                                        // requires it)
    }
  }
  return *this;
}

template <class... Types>
vector<Types...>& vector<Types...>::operator=(vector&& rhs) {
  if (this != &rhs) {
    delete_data();
    size_ = rhs.size_;
    capacity = rhs.capacity;
    entries = rhs.entries;
    data = rhs.data;
    offsets = rhs.offsets;
    type_size = rhs.type_size;
    type_index = rhs.type_index;
    type_align = rhs.type_align;
    rhs.reset();
  }
  return *this;
}

template <class... Types>
template <class U>
void vector<Types...>::push_back(const U& u) {
  if (size_ == entries) {
    reserve_entries(2 * entries + 1);
  }

  std::size_t index = find_type_index<0, U, Types...>();

  type_size[size_] = sizeof(U);
  type_index[size_] = index;
  type_align[size_] = alignof(U);

  place_obj(size_, reinterpret_cast<const std::byte* const>(&u),
            ctable[type_index[size_]]);
}

template <class... Types>
template <class U>
void vector<Types...>::push_back(rval_ref<U> u) {
  using Udec = std::decay_t<U>;
  if (size_ == entries) {
    reserve_entries(2 * entries + 1);
  }

  std::size_t index = find_type_index<0, Udec, Types...>();

  type_size[size_] = sizeof(Udec);
  type_index[size_] = index;
  type_align[size_] = alignof(Udec);

  place_obj(size_, reinterpret_cast<const std::byte* const>(&u),
            mtable[type_index[size_]]);
}

template <class... Types>
[[nodiscard]] Element vector<Types...>::operator[](std::size_t index) {
  return {
      type_index[index],
      data + offsets[index],
  };
}

template <class... Types>
template <class T>
[[nodiscard]] T& vector<Types...>::get(std::size_t index) {
  if (type_index[index] != find_type_index<0, T, Types...>()) {
    throw std::bad_cast();
  }
  return *reinterpret_cast<T*>(data + offsets[index]);
}

template <class... Types>
void vector<Types...>::reserve_entries(std::size_t new_entries) {
  if (new_entries > entries) {
    std::size_t* new_offsets = new std::size_t[new_entries];
    std::size_t* new_type_size = new std::size_t[new_entries];
    std::size_t* new_types = new std::size_t[new_entries];
    std::size_t* new_type_align = new std::size_t[new_entries];

    for (std::size_t i = 0; i < size_; i++) {
      new_offsets[i] = offsets[i];
      new_type_size[i] = type_size[i];
      new_types[i] = type_index[i];
      new_type_align[i] = type_align[i];
    }

    delete[] offsets;
    delete[] type_size;
    delete[] type_index;
    delete[] type_align;

    offsets = new_offsets;
    entries = new_entries;
    type_size = new_type_size;
    type_index = new_types;
    type_align = new_type_align;
  }
}

template <class... Types>
void vector<Types...>::reserve_cap(std::size_t new_cap) {
  if (new_cap > capacity) {
    std::cout << "reserving cap from " << capacity << " to " << new_cap
              << std::endl;
    std::byte* new_data = new std::byte[new_cap];
    for (std::size_t i = 0; i < size_; i++) {
      std::size_t old_offset = offsets[i];
      if (i > 0) {
        offsets[i] = offsets[i - 1] + type_size[i - 1];
      } else {
        offsets[i] = 0;
      }
      std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(data + offsets[i]);
      offsets[i] += get_padding(addr, type_align[i]);
      mtable[type_index[i]](new_data + offsets[i], data + old_offset);
    }
    delete[] data;
    data = new_data;
    capacity = new_cap;
  }
}

template <class... Types>
template <std::size_t I, class U, class T, class... TN>
[[nodiscard]] std::size_t vector<Types...>::find_type_index() const {
  if constexpr (std::is_same_v<std::decay_t<U>, std::decay_t<T>>) {
    return I;
  } else {
    return find_type_index<I + 1, U, TN...>();
  }
}

template <class... Types>
void vector<Types...>::place_obj(std::size_t index, const std::byte* const p,
                                 cm_fptr_t place_func) {
  if (index > 0) {
    offsets[index] = offsets[index - 1] + type_size[index - 1];
  } else {
    offsets[index] = 0;
  }

  std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(data + offsets[size_]);
  offsets[size_] += get_padding(addr, type_align[size_]);

  std::size_t new_cap = offsets[size_] + type_size[size_] + 1;
  if (new_cap > capacity) {
    reserve_cap(std::max(new_cap, capacity * 2));
  }
  place_func(data + offsets[index], p);
  size_++;
}

template <class... Types> void vector<Types...>::delete_data() {
  for (std::size_t i = 0; i < size_; i++) {
    dtable[type_index[i]](data + offsets[i]);
  }
  delete[] data;
  delete[] offsets;
  delete[] type_size;
  delete[] type_index;
  delete[] type_align;
}

template <class... Types> void vector<Types...>::reset() {
  size_ = 0;
  capacity = 0;
  entries = 0;
  data = nullptr;
  offsets = nullptr;
  type_size = nullptr;
  type_index = nullptr;
  type_align = nullptr;
}

} // namespace vv3

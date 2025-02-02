#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <typeinfo>

// this isn't meant to work anyway (due to unaligned access), so I won't bother
// making it work beyond the bare minimum. vv3 is this but actually works
namespace vv2 {

template <class... Types> class vector {
public:
  struct Element {
    std::size_t type_index;
    void* data;
  };

  vector()
      : size_(0), capacity(0), entries(0), data(nullptr), offsets(nullptr),
        type_size(nullptr), types(nullptr) {}

  ~vector() {
    delete[] data;
    delete[] offsets;
    delete[] type_size;
    delete[] types;
  }

  template <class U> void push_back(const U& u) {
    if (size_ == entries) {
      reserve_entries(2 * entries + 1);
    }

    std::size_t index = find_type_index<0, U, Types...>();

    type_size[size_] = sizeof(U);
    types[size_] = index;

    if (size_ > 0) {
      offsets[size_] = offsets[size_ - 1] + type_size[size_ - 1];
    } else {
      offsets[size_] = 0;
    }

    reserve_cap(offsets[size_] + type_size[size_] + 1);
    std::memcpy(data + offsets[size_], &u, sizeof(U));
    size_++;
  }

  [[nodiscard]] Element operator[](std::size_t index) const {
    return {
        .type_index{types[index]},
        .data{static_cast<void*>(data + offsets[index])},
    };
  }

  template <class T> [[nodiscard]] T& get(std::size_t index) const {
    if (types[index] != find_type_index<0, T, Types...>()) {
      throw std::bad_cast();
    }
    return *reinterpret_cast<T*>(data + offsets[index]);
  }

  [[nodiscard]] std::size_t size() const noexcept { return size_; }

private:
  std::size_t size_;
  std::size_t capacity;
  std::size_t entries;

  std::byte* data;
  std::size_t* type_size;
  std::size_t* types;
  std::size_t* offsets;

  void reserve_entries(std::size_t new_entries) {
    if (new_entries > entries) {
      std::size_t* new_offsets = new std::size_t[new_entries];
      std::size_t* new_type_size = new std::size_t[new_entries];
      std::size_t* new_types = new std::size_t[new_entries];

      for (int i = 0; i < size_; i++) {
        new_offsets[i] = offsets[i];
        new_type_size[i] = type_size[i];
        new_types[i] = types[i];
      }

      delete[] offsets;
      delete[] type_size;
      delete[] types;

      offsets = new_offsets;
      type_size = new_type_size;
      types = new_types;
      entries = new_entries;
    }
  }

  void reserve_cap(std::size_t new_cap) {
    if (new_cap > capacity) {
      std::byte* new_data = new std::byte[new_cap];
      if (data) {
        std::memcpy(new_data, data, capacity);
        delete[] data;
      }
      data = new_data;
      capacity = new_cap;
    }
  }

  template <std::size_t I, class U, class T, class... TN>
  std::size_t find_type_index() const {
    if constexpr (std::is_same_v<std::decay_t<U>, std::decay_t<T>>) {
      return I;
    } else {
      return find_type_index<I + 1, U, TN...>();
    }
  }
};

} // namespace vv2

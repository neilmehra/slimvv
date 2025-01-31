#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <typeinfo>

namespace vv3 {

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
    for (std::size_t i = 0; i < size_; i++) {
      dtable[types[i]](data + offsets[i]);
    }
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

    std::uintptr_t addr =
        reinterpret_cast<std::uintptr_t>(data + offsets[size_]);
    offsets[size_] += get_padding(addr, alignof(U));
    ;

    reserve_cap(offsets[size_] + type_size[size_] + 1);
    ::new (data + offsets[size_]) U(u);
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
  static constexpr std::size_t N = sizeof...(Types);
  using dtor_fptr_t = void (*)(std::byte* const);
  using cm_fptr_t = void (*)(std::byte* const, const std::byte* const);

  template <class T> static void destroy_impl(std::byte* const p) {
    static_cast<T*>(p)->~T();
  }

  template <class T>
  static std::byte* copy_impl(std::byte* const loc, std::byte* p) {
    ::new (loc) T(*static_cast<const T* const>(p));
  }

  template <class T>
  static std::byte* move_impl(std::byte* const loc, std::byte* p) {
    ::new (loc) T(*static_cast<const T* const>(p));
  }

  static constexpr dtor_fptr_t dtable[N]{destroy_impl<Types>...};
  static constexpr cm_fptr_t ctable[N]{copy_impl<Types>...};
  static constexpr cm_fptr_t mtable[N]{move_impl<Types>...};

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

  std::size_t get_padding(std::uintptr_t addr, std::size_t align) {
    std::size_t aligned_addr = (addr + (align - 1)) & ~(align - 1);
    std::size_t padding = aligned_addr - addr;
  }
};

} // namespace vv3

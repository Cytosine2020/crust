#ifndef CRUST_SLICE_HPP
#define CRUST_SLICE_HPP


#include "utility.hpp"
#include "ops.hpp"


namespace crust {
template<class T>
class Slice : public index::Index<Slice<T>, usize, T> {
private:
  T *inner;
  usize size;

  Slice() noexcept : inner{nullptr}, size{0} {}

  Slice(T *inner, usize size) : inner{inner}, size{size} {}

public:
  static Slice from_ptr(T *inner) {
    return Slice{inner, 1};
  }

  static Slice from_raw_parts(T *inner, usize size) {
    return Slice{inner, size};
  }

  usize len() const { return size; }

  bool is_empty() const { return len() == 0; }

  const T *as_ptr() const { return inner; }

  T *as_ptr() { return inner; }

  const T &index(usize index) const {
    if (index >= len()) { CRUST_PANIC("index out of boundary!"); }
    return as_ptr()[index];
  }

  T &index_mut(usize index) {
    if (index >= len()) { CRUST_PANIC("index_mut out of boundary!"); }
    return as_ptr()[index];
  }
};
}


#endif //CRUST_SLICE_HPP

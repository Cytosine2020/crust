#ifndef _CRUST_INCLUDE_SLICE_HPP
#define _CRUST_INCLUDE_SLICE_HPP


#include "utility.hpp"
#include "ops/mod.hpp"


namespace crust {
template<class T>
class crust_ebco Slice : public index::Index<Slice<T>, usize, T> {
private:
  T *inner;
  usize size;

  Slice() : inner{nullptr}, size{0} {}

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
    if (index >= len()) { crust_panic("index out of boundary!"); }
    return as_ptr()[index];
  }

  T &index_mut(usize index) {
    if (index >= len()) { crust_panic("index_mut out of boundary!"); }
    return as_ptr()[index];
  }
};
}


#endif //_CRUST_INCLUDE_SLICE_HPP

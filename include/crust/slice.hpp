#ifndef CRUST_SLICE_HPP
#define CRUST_SLICE_HPP


#include "crust/ops/mod.hpp"
#include "crust/utility.hpp"


namespace crust {
template <class T>
struct crust_ebco Slice : Impl<Slice<T>, Trait<index::Index, usize, T>> {
private:
  T *inner;
  usize size;

  Slice() : inner{nullptr}, size{0} {}

  Slice(T *inner, usize size) : inner{inner}, size{size} {}

public:
  static Slice from_ptr(T *inner) { return Slice{inner, 1}; }

  static Slice from_raw_parts(T *inner, usize size) {
    return Slice{inner, size};
  }

  usize len() const { return size; }

  bool is_empty() const { return len() == 0; }

  const T *as_ptr() const { return inner; }

  T *as_ptr() { return inner; }
};

template <class T>
CRUST_IMPL_FOR(CRUST_MACRO(index::Index<Slice<T>, usize, T>)) {
  CRUST_IMPL_USE_SELF(Slice<T>);

  const T &index(usize index) const {
    if (index >= self().len()) {
      crust_panic("index out of boundary!");
    }
    return self().as_ptr()[index];
  }

  T &index_mut(usize index) {
    if (index >= self().len()) {
      crust_panic("index_mut out of boundary!");
    }
    return self().as_ptr()[index];
  }
};
} // namespace crust


#endif // CRUST_SLICE_HPP

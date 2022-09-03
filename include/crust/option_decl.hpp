#ifndef CRUST_OPTION_DECL_HPP
#define CRUST_OPTION_DECL_HPP


#include "crust/enum_decl.hpp"
#include "crust/ops/function.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace option {
struct None;

template <class T>
struct Some;

template <class T>
struct Option;
} // namespace option

using option::None;
using option::Option;
using option::Some;

template <>
struct BluePrint<option::None> : TmplType<TupleStruct<>> {};

template <class T>
struct BluePrint<option::Some<T>> : TmplType<TupleStruct<T>> {};

template <class T>
struct BluePrint<Option<T>> : TmplType<Enum<None, Some<T>>> {};

namespace option {
template <class T>
struct crust_ebco Option :
    Enum<None, Some<T>>,
    Derive<
        Option<T>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>,
        Trait<cmp::PartialOrd>,
        Trait<cmp::Ord>> {
  CRUST_ENUM_USE_BASE(Option, Enum<None, Some<T>>)

  constexpr bool is_some() const {
    return this->template is_variant<Some<T>>();
  }

  constexpr bool is_none() const { return this->template is_variant<None>(); }

  constexpr bool contains(const T &other) const {
    return this->template eq_variant<Some<T>>(other);
  }

  constexpr Option<const T *> as_ptr() const {
    return map(ops::bind([](const T &value) { return &value; }));
  }

  crust_cxx14_constexpr Option<T *> as_mut_ptr() {
    return map(ops::bind([](T &value) { return &value; }));
  }

  crust_cxx14_constexpr T unwrap() && {
    return this->template visit<T>(
        [](Some<T> &value) { return move(value.template get<0>()); },
        [](None &) {
          crust_panic("called `Option::unwrap()` on a `None` value");
        });
  }

  crust_cxx14_constexpr T unwrap_or(T &&d) && {
    return this->template visit<T>(
        [](Some<T> &value) { return move(value.template get<0>()); },
        [&](None &) { return d; });
  }

  template <class U, class F>
  constexpr Option<U> map(ops::Fn<F, U(const T &)> f) const;

  template <class U, class F>
  constexpr U map_or(U &&d, ops::Fn<F, U(const T &)> f) const {
    return this->template visit<U>(
        [&](const Some<T> &value) { return f(value.template get<0>()); },
        [&](const None &) { return move<U>(d); });
  }

  template <class U, class D, class F>
  constexpr U map_or_else(ops::Fn<D, U()> d, ops::Fn<F, U(const T &)> f) const {
    return this->template visit<U>(
        [&](const Some<T> &value) { return f(value.template get<0>()); },
        [&](const None &) { return d(); });
  }

  crust_cxx14_constexpr Option<T> take();
};
} // namespace option
} // namespace crust


#endif // CRUST_OPTION_DECL_HPP

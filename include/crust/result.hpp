#ifndef CRUST_RESULT_HPP
#define CRUST_RESULT_HPP


#include "crust/enum.hpp"
#include "crust/option.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace result {
template <class T>
struct Ok;

template <class E>
struct Err;

template <class T, class E>
struct Result;
} // namespace result

using result::Err;
using result::Ok;
using result::Result;

template <class T>
struct BluePrint<Ok<T>> : TmplType<TupleStruct<T>> {};

template <class E>
struct BluePrint<Err<E>> : TmplType<TupleStruct<E>> {};

template <class T, class E>
struct BluePrint<Result<T, E>> : TmplType<Enum<Ok<T>, Err<E>>> {};

namespace result {
template <class T>
CRUST_ENUM_TUPLE_VARIANT(Ok, Ok<T>, T);

template <class E>
CRUST_ENUM_TUPLE_VARIANT(Err, Err<E>, E);

template <class T, class E>
struct crust_ebco Result :
    Enum<Ok<T>, Err<E>>,
    Derive<
        Result<T, E>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>,
        Trait<cmp::PartialOrd>,
        Trait<cmp::Ord>> {
  CRUST_ENUM_USE_BASE(Result, Enum<Ok<T>, Err<E>>);

  constexpr bool is_ok() const { return this->template is_variant<Ok<T>>(); }

  constexpr bool is_err() const { return this->template is_variant<Err<E>>(); }

  constexpr bool contains(const T &other) const {
    return this->template visit<bool>(
        [&](const Ok<T> &value) { return value.template get<0>() == other; },
        [](const Err<E> &) { return false; });
  }

  constexpr bool contains_err(const E &other) const {
    return this->template visit<bool>(
        [](const Ok<T> &) { return false; },
        [&](const Err<E> &value) { return value.template get<0>() == other; });
  }

  crust_cxx14_constexpr Option<T> ok() {
    return this->template move_variant<Option<T>>();
  }

  crust_cxx14_constexpr Option<E> err() {
    return this->template move_variant<Option<E>>();
  }
};
} // namespace result
} // namespace crust


#endif // CRUST_RESULT_HPP

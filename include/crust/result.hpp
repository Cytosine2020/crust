#ifndef CRUST_RESULT_HPP
#define CRUST_RESULT_HPP


#include "crust/enum.hpp"
#include "crust/option.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace result {
template <class T>
struct Ok :
    TupleStruct<T>,
    AutoImpl<
        Ok<T>,
        TupleStruct<T>,
        ZeroSizedType,
        cmp::PartialEq,
        cmp::Eq,
        cmp::PartialOrd,
        cmp::Ord> {};

template <class E>
struct Err :
    TupleStruct<E>,
    AutoImpl<
        Err<E>,
        TupleStruct<E>,
        ZeroSizedType,
        cmp::PartialEq,
        cmp::Eq,
        cmp::PartialOrd,
        cmp::Ord> {};

template <class T, class E>
class Result :
    public Enum<Ok<T>, Err<E>>,
    AutoImpl<Result<T, E>, Enum<Ok<T>, Err<E>>, cmp::PartialEq, cmp::Eq> {
public:
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

using result::Result;
} // namespace crust


#endif // CRUST_RESULT_HPP

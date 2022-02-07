#ifndef _CRUST_INCLUDE_RESULT_HPP
#define _CRUST_INCLUDE_RESULT_HPP


#include "utility.hpp"

#include "enum.hpp"
#include "option.hpp"


namespace crust {
namespace result {
template<class T>
CRUST_ENUM_VARIANT(Ok, T);

template<class E>
CRUST_ENUM_VARIANT(Err, E);

template<class T, class E>
class Result : public Enum<Ok<T>, Err<E>> {
public:
  CRUST_ENUM_USE_BASE(Result, Enum<Ok<T>, Err<E>>);

  constexpr bool is_ok() const { return this->template is_variant<Ok<T>>(); }

  constexpr bool is_err() const { return this->template is_variant<Err<E>>(); }

  constexpr bool contains(const T &other) const {
    return this->template visit<bool>(
        [&](const Ok<T> &value) { return value.template get<0>() == other; },
        [](const Err<E> &) { return false; }
    );
  }

  constexpr bool contains_err(const E &other) const {
    return this->template visit<bool>(
        [](const Ok<T> &) { return false; },
        [&](const Err<E> &value) { return value.template get<0>() == other; }
    );
  }

  crust_cxx14_constexpr Option<T> ok() {
    return this->template move_variant<Option<T>>();
  }

  crust_cxx14_constexpr Option<E> err() {
    return this->template move_variant<Option<E>>();
  }
};
}

using result::Result;
}


#endif //_CRUST_INCLUDE_RESULT_HPP

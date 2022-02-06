#ifndef _CRUST_INCLUDE_OPTION_DECL_HPP
#define _CRUST_INCLUDE_OPTION_DECL_HPP


#include "utility.hpp"
#include "enum_decl.hpp"
#include "ops/function.hpp"


namespace crust {
namespace option {
template<class T>
struct Some;

struct None;

template<class T>
class Option : public Enum<Some<T>, None> {
public:
  CRUST_ENUM_USE_BASE(Option, Enum<Some<T>, None>);

  constexpr bool is_some() const {
    return this->template is_variant<Some<T>>();
  }

  constexpr bool is_none() const {
    return this->template is_variant<None>();
  }

  constexpr bool contains(const T &other) const {
    return this->template eq_variant<Some<T>>(other);
  }

  crust_cxx14_constexpr Option<const T *> as_ptr() const {
    return map(ops::bind([](const T &value) { return &value; }));
  }

  crust_cxx14_constexpr Option<T *> as_mut_ptr() {
    return map(bind([](T &value) { return &value; }));
  }

  crust_cxx14_constexpr T unwrap() {
    return this->template visit<T>(
        [](Some<T> &value) { return move(value.template get<0>()); },
        [](None &) {
          crust_panic("called `Option::unwrap()` on a `None` value");
        }
    );
  }

  crust_cxx14_constexpr T unwrap_or(T &&d) {
    return this->template visit<T>(
        [](Some<T> &value) { return move(value.template get<0>()); },
        [&](None &) { return d; }
    );
  }

  template<class U, class F>
  crust_cxx14_constexpr Option<U> map(ops::Fn<F, U(const T &)> f) const;

  template<class U, class F>
  crust_cxx14_constexpr U map_or(U &&d, ops::Fn<F, U(const T &)> f) const {
    return this->template visit<U>(
        [&](const Some<T> &value) { return f(value.template get<0>()); },
        [&](const None &) { return move<U>(d); }
    );
  }

  template<class U, class D, class F>
  crust_cxx14_constexpr U
  map_or_else(ops::Fn<D, U()> d, ops::Fn<F, U(const T &)> f) const {
    return this->template visit<U>(
        [&](const Some<T> &value) { return f(value.template get<0>()); },
        [&](const None &) { return d(); }
    );
  }

  crust_cxx14_constexpr Option<T> take();
};
}

using option::Option;
using option::Some;
using option::None;
}


#endif //_CRUST_INCLUDE_OPTION_DECL_HPP

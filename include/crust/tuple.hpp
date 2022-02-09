#ifndef CRUST_TUPLE_HPP
#define CRUST_TUPLE_HPP


#include "tuple_decl.hpp"

#include <utility>

#include "crust/cmp.hpp"
#include "crust/option.hpp"


namespace crust {
namespace _impl_tuple {
template <class Field, class... Fields>
constexpr Option<cmp::Ordering>
TupleHolderImpl<false, false, Field, Fields...>::partial_cmp(
    const TupleHolderImpl &other) const {
  return cmp::operator_partial_cmp(this->field, other.field)
      .map(ops::bind([&](cmp::Ordering value) {
        return value.then(
            cmp::operator_partial_cmp(this->remains, other.remains));
      }));
}

template <class Field, class... Fields>
constexpr cmp::Ordering TupleHolderImpl<false, false, Field, Fields...>::cmp(
    const TupleHolderImpl &other) const {
  return cmp::operator_cmp(this->field, other.field)
      .then(cmp::operator_cmp(this->remains, other.remains));
}

template <class Field, class... Fields>
constexpr Option<cmp::Ordering>
TupleHolderImpl<false, true, Field, Fields...>::partial_cmp(
    const TupleHolderImpl &other) const {
  return cmp::operator_partial_cmp(this->field, other.field);
}

template <class Field, class... Fields>
constexpr cmp::Ordering TupleHolderImpl<false, true, Field, Fields...>::cmp(
    const TupleHolderImpl &other) const {
  return cmp::operator_cmp(this->field, other.field);
}

template <class... Fields>
constexpr Option<cmp::Ordering>
TupleHolderImpl<true, true, Fields...>::partial_cmp(
    const TupleHolderImpl &) const {
  return make_some(cmp::make_equal());
}

template <class... Fields>
inline constexpr cmp::Ordering
TupleHolderImpl<true, true, Fields...>::cmp(const TupleHolderImpl &) const {
  return cmp::make_equal();
}
} // namespace _impl_tuple

template <class... Fields>
constexpr Option<cmp::Ordering>
Tuple<Fields...>::partial_cmp(const Tuple &other) const {
  return this->holder.partial_cmp(other.holder);
}

template <class... Fields>
constexpr cmp::Ordering Tuple<Fields...>::cmp(const Tuple &other) const {
  return this->holder.cmp(other.holder);
}

template <class... Fields>
constexpr Tuple<typename RemoveConstOrRefType<Fields>::Result...>
make_tuple(Fields &&...fields) {
  return Tuple<typename RemoveConstOrRefType<Fields>::Result...>{
      forward<Fields>(fields)...};
}

namespace _impl_tuple {
template <class... Fields>
class LetTuple {
private:
  TupleHolder<Fields &...> ref;

public:
  explicit constexpr LetTuple(Fields &...fields) : ref{fields...} {}

  crust_cxx14_constexpr void operator=(Tuple<Fields...> &&tuple) {
    LetTupleHelper<sizeof...(Fields), Fields...>::inner(ref, move(tuple));
  }
};
} // namespace _impl_tuple

template <class... Fields>
crust_cxx14_constexpr
    _impl_tuple::LetTuple<typename RemoveRefType<Fields>::Result...>
    let(Fields &&...fields) {
  return _impl_tuple::LetTuple<typename RemoveRefType<Fields>::Result...>{
      forward<Fields>(fields)...};
}
} // namespace crust

namespace std {

/// c++ std bindings

template <class... Fields>
struct tuple_size<crust::Tuple<Fields...>> :
    integral_constant<crust::usize, sizeof...(Fields)> {};

template <crust::usize index, class... Fields>
struct tuple_element<index, crust::Tuple<Fields...>> {
  using type =
      typename crust::_impl_tuple::TupleGetter<index, Fields...>::Result;
};

template <crust::usize index, class... Fields>
constexpr const typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(const crust::Tuple<Fields...> &object) {
  return object.template get<index>();
}

template <crust::usize index, class... Fields>
constexpr typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(crust::Tuple<Fields...> &object) {
  return object.template get<index>();
}
} // namespace std


#endif // CRUST_TUPLE_HPP

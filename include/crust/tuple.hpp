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
TupleHolderSizedImpl<false, false, Field, Fields...>::_partial_cmp(
    const TupleHolderSizedImpl &other) const {
  return cmp::operator_partial_cmp(field, other.field)
      .map(ops::bind([&](cmp::Ordering value) {
        return value.then(remains._partial_cmp(other.remains)); // FIXME
      }));
}

template <class Field, class... Fields>
constexpr cmp::Ordering
TupleHolderSizedImpl<false, false, Field, Fields...>::_cmp(
    const TupleHolderSizedImpl &other) const {
  return cmp::operator_cmp(field, other.field)
      .then(remains._cmp(other.remains));
}

template <class Field, class... Fields>
constexpr Option<cmp::Ordering>
TupleHolderSizedImpl<false, true, Field, Fields...>::_partial_cmp(
    const TupleHolderSizedImpl &other) const {
  return cmp::operator_partial_cmp(field, other.field);
}

template <class Field, class... Fields>
constexpr cmp::Ordering
TupleHolderSizedImpl<false, true, Field, Fields...>::_cmp(
    const TupleHolderSizedImpl &other) const {
  return cmp::operator_cmp(field, other.field);
}

template <class... Fields>
constexpr Option<cmp::Ordering>
TupleHolderSizedImpl<true, true, Fields...>::_partial_cmp(
    const TupleHolderSizedImpl &) const {
  return make_some(cmp::make_equal());
}

template <class... Fields>
constexpr cmp::Ordering TupleHolderSizedImpl<true, true, Fields...>::_cmp(
    const TupleHolderSizedImpl &) const {
  return cmp::make_equal();
}

inline constexpr Option<cmp::Ordering>
TupleHolder<>::_partial_cmp(const TupleHolder &) const {
  return make_some(cmp::make_equal());
}

inline constexpr cmp::Ordering TupleHolder<>::_cmp(const TupleHolder &) const {
  return cmp::make_equal();
}
} // namespace _impl_tuple

template <class... Fields>
constexpr Tuple<typename RemoveConstOrRefType<Fields>::Result...>
make_tuple(Fields &&...fields) {
  return Tuple<typename RemoveConstOrRefType<Fields>::Result...>{
      forward<Fields>(fields)...};
}

namespace _auto_impl {
template <class Self>
constexpr Option<::crust::cmp::Ordering>
TuplePartialOrdImpl<Self>::partial_cmp(const Self &other) const {
  return self()._partial_cmp(other);
}

template <class Self>
constexpr ::crust::cmp::Ordering
TupleOrdImpl<Self>::cmp(const Self &other) const {
  return self()._cmp(other);
}
} // namespace _auto_impl

// namespace _impl_tuple {
// template <class... Fields>
// class LetTuple {
// private:
//   TupleSizedHolder<Fields &...> ref;

// public:
//   explicit constexpr LetTuple(Fields &...fields) : ref{fields...} {}

//   crust_cxx14_constexpr void operator=(Tuple<Fields...> &&tuple) {
//     LetTupleHelper<sizeof...(Fields), Fields...>::inner(ref, move(tuple));
//   }
// };
// } // namespace _impl_tuple

// template <class... Fields>
// crust_cxx14_constexpr
//     _impl_tuple::LetTuple<typename RemoveRefType<Fields>::Result...>
//     let(Fields &&...fields) {
//   return _impl_tuple::LetTuple<typename RemoveRefType<Fields>::Result...>{
//       forward<Fields>(fields)...};
// }
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

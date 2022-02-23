#ifndef CRUST_TUPLE_HPP
#define CRUST_TUPLE_HPP


#include "tuple_decl.hpp"

#include <utility>

#include "crust/cmp.hpp"
#include "crust/option.hpp"


namespace crust {
namespace _auto_impl {
template <class Self, class... Fields>
constexpr Option<cmp::Ordering> AutoImpl<
    Self,
    TupleStruct<Fields...>,
    cmp::PartialOrd,
    EnableIf<Derive<Fields, cmp::PartialOrd>...>>::partial_cmp(const Self
                                                                   &other)
    const {
  return PartialOrdHelper::partial_cmp(self(), other);
}

template <class Self, class... Fields>
constexpr cmp::Ordering AutoImpl<
    Self,
    TupleStruct<Fields...>,
    cmp::Ord,
    EnableIf<Derive<Fields, cmp::Ord>...>>::cmp(const Self &other) const {
  return TupleLikeOrdHelper<Self, TupleStruct<Fields...>>::cmp(self(), other);
}
} // namespace _auto_impl

template <class... Fields>
constexpr Tuple<typename RemoveConstOrRefType<Fields>::Result...>
make_tuple(Fields &&...fields) {
  return Tuple<typename RemoveConstOrRefType<Fields>::Result...>{
      forward<Fields>(fields)...};
}

namespace _impl_tuple {
template <usize index, class... Fields>
struct LetTupleHelper {
  static crust_cxx14_constexpr void
  inner(TupleSizedHolder<Fields &...> &ref, TupleStruct<Fields...> &&tuple) {
    TupleGetter<index - 1, Fields &...>::inner(ref) =
        move(tuple.template get<index - 1>());
    LetTupleHelper<index - 1, Fields...>::inner(ref, move(tuple));
  }
};

template <class... Fields>
struct LetTupleHelper<0, Fields...> {
  static crust_cxx14_constexpr void
  inner(TupleSizedHolder<Fields &...> &, TupleStruct<Fields...> &&) {}
};

template <class... Fields>
struct LetTuple {
private:
  TupleSizedHolder<Fields &...> ref;

public:
  explicit constexpr LetTuple(Fields &...fields) : ref{fields...} {}

  crust_cxx14_constexpr void operator=(TupleSizedHolder<Fields...> &&tuple) {
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
    integral_constant<
        crust::usize,
        crust::_auto_impl::TupleLikeSize<
            crust::TupleStruct<Fields...>>::result> {};

template <crust::usize index, class... Fields>
struct tuple_element<index, crust::Tuple<Fields...>> {
  using type = typename crust::_auto_impl::
      TupleLikeGetter<crust::TupleStruct<Fields...>, index>::Result;
};

template <crust::usize index, class... Fields>
constexpr const typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(const crust::Tuple<Fields...> &object) {
  return crust::_auto_impl::
      TupleLikeGetter<crust::TupleStruct<Fields...>, index>::get(object);
}

template <crust::usize index, class... Fields>
constexpr typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(crust::Tuple<Fields...> &object) {
  return crust::_auto_impl::
      TupleLikeGetter<crust::TupleStruct<Fields...>, index>::get(object);
}
} // namespace std


#endif // CRUST_TUPLE_HPP

#ifndef CRUST_TUPLE_HPP
#define CRUST_TUPLE_HPP


#include "tuple_decl.hpp"

#include <utility>

#include "crust/cmp.hpp"
#include "crust/option.hpp"


namespace crust {
namespace _impl_derive {
template <class Self, class... Fields>
constexpr Option<cmp::Ordering> Derive<
    Self,
    TupleStruct<Fields...>,
    cmp::PartialOrd,
    EnableIf<Require<Fields, cmp::PartialOrd>...>>::partial_cmp(const Self
                                                                    &other)
    const {
  return PartialOrdHelper::partial_cmp(self(), other);
}

template <class Self, class... Fields>
constexpr cmp::Ordering Derive<
    Self,
    TupleStruct<Fields...>,
    cmp::Ord,
    EnableIf<Require<Fields, cmp::Ord>...>>::cmp(const Self &other) const {
  return TupleLikeOrdHelper<Self, TupleStruct<Fields...>>::cmp(self(), other);
}
} // namespace _impl_derive

template <class... Fields>
constexpr Tuple<typename RemoveConstOrRefType<Fields>::Result...>
tuple(Fields &&...fields) {
  return Tuple<typename RemoveConstOrRefType<Fields>::Result...>{
      forward<Fields>(fields)...};
}

namespace _impl_tuple {
template <class T>
struct LetField {
  T *ptr;

  constexpr LetField(T &ref) : ptr{&ref} {}

  crust_cxx14_constexpr void assign_ref(const T &other) { *ptr = other; }

  crust_cxx14_constexpr void assign_ref_mut(const T &other) { *ptr = other; }

  crust_cxx14_constexpr void assign_move(T &other) { *ptr = move(other); }
};

template <>
struct LetField<_impl_utility::Ignore> {
  constexpr LetField(const _impl_utility::Ignore &) {}

  template <class T>
  crust_cxx14_constexpr void assign_ref(const T &) const {}

  template <class T>
  crust_cxx14_constexpr void assign_ref_mut(const T &) const {}

  template <class T>
  crust_cxx14_constexpr void assign_move(const T &) const {}
};

template <>
struct LetField<_impl_utility::IgnoreRange> {
  constexpr LetField(const _impl_utility::IgnoreRange &) {}

  template <class T>
  crust_cxx14_constexpr void assign_ref(const T &) const {}

  template <class T>
  crust_cxx14_constexpr void assign_ref_mut(const T &) const {}

  template <class T>
  crust_cxx14_constexpr void assign_move(const T &) const {}
};

template <class T>
struct LetField<Ref<T>> {
  Ref<T> *ptr;

  constexpr LetField(Ref<T> &ref) : ptr{&ref} {}

  crust_cxx14_constexpr void assign_ref(const Ref<T> &other) { *ptr = other; }

  crust_cxx14_constexpr void assign_ref_mut(const Ref<T> &other) {
    *ptr = other;
  }

  crust_cxx14_constexpr void assign(Ref<T> &other) { *ptr = move(other); }

  crust_cxx14_constexpr void assign_ref(const T &other) { *ptr = ref(other); }

  crust_cxx14_constexpr void assign_ref_mut(const T &other) {
    *ptr = ref(other);
  }

  crust_cxx14_constexpr void assign_move(T &other) { *ptr = ref(other); }
};

template <class T>
struct LetField<RefMut<T>> {
  RefMut<T> *ptr;

  constexpr LetField(RefMut<T> &ref) : ptr{&ref} {}

  crust_cxx14_constexpr void assign_ref(const RefMut<T> &other) {
    *ptr = other;
  }

  crust_cxx14_constexpr void assign_ref_mut(const RefMut<T> &other) {
    *ptr = other;
  }

  crust_cxx14_constexpr void assign(RefMut<T> &other) { *ptr = move(other); }

  crust_cxx14_constexpr void assign_ref_mut(T &other) { *ptr = ref_mut(other); }

  crust_cxx14_constexpr void assign_move(T &other) { *ptr = ref_mut(other); }
};

template <bool is_range, usize r1_idx, usize r2_idx, class Self, class Base>
struct LetTupleHelperRemain;

template <usize r1_idx, usize r2_idx, class Self, class Base>
struct LetTupleHelper {
  static constexpr usize index1 =
      _impl_derive::TupleLikeSize<Self>::result - r1_idx;
  static constexpr usize index2 =
      _impl_derive::TupleLikeSize<Base>::result - r2_idx;
  using Getter = _impl_derive::TupleLikeGetter<Base, index2>;
  using This = typename _impl_derive::TupleLikeGetter<Self, index1>::Result;
  static constexpr bool is_range =
      IsSame<This, LetField<_impl_utility::IgnoreRange>>::result;
  using Remains =
      typename LetTupleHelperRemain<is_range, r1_idx, r2_idx, Self, Base>::
          Result;

  static crust_cxx14_constexpr void assign_ref(Self &ref, const Base &tuple) {
    ref.template get<index1>().assign_ref(Getter::get(tuple));
    Remains::assign_ref(ref, tuple);
  }

  static crust_cxx14_constexpr void assign_ref_mut(Self &ref, Base &tuple) {
    ref.template get<index1>().assign_ref_mut(Getter::get(tuple));
    Remains::assign_ref_mut(ref, tuple);
  }

  static crust_cxx14_constexpr void assign_move(Self &ref, Base &&tuple) {
    ref.template get<index1>().assign_move(Getter::get(tuple));
    Remains::assign_move(ref, move(tuple));
  }
};

template <class Self, class Base>
struct LetTupleHelper<0, 0, Self, Base> {
  static crust_cxx14_constexpr void assign_ref(Self &, const Base &) {}

  static crust_cxx14_constexpr void assign_ref_mut(Self &, Base &) {}

  static crust_cxx14_constexpr void assign_move(Self &, Base &&) {}
};

template <usize r1_idx, usize r2_idx, class Self, class Base>
struct LetTupleHelperRemain<false, r1_idx, r2_idx, Self, Base> {
  using Result = LetTupleHelper<r1_idx - 1, r2_idx - 1, Self, Base>;
};

template <usize r1_idx, usize r2_idx, class Self, class Base>
struct LetTupleHelperRemain<true, r1_idx, r2_idx, Self, Base> :
    _impl_derive::TupleLikeSize<Self> {
  using Result = LetTupleHelper<r1_idx - 1, r1_idx - 1, Self, Base>;
};

template <class... Fields>
struct LetTuple {
private:
  crust_static_assert(!Any<IsConstOrRefVal<Fields>...>::result);
  crust_static_assert(
      _impl_types::TypesCountType<_impl_utility::IgnoreRange, Fields...>::
          result <= 1);

  Tuple<LetField<Fields>...> ref;

public:
  template <class... Fs>
  explicit constexpr LetTuple(Fs &...fields) :
      ref{LetField<Fields>{fields}...} {}

  template <class... Fs>
  crust_cxx14_constexpr LetTuple &operator=(const TupleStruct<Fs...> &tuple) {
    LetTupleHelper<
        sizeof...(Fields),
        sizeof...(Fs),
        TupleStruct<LetField<Fields>...>,
        TupleStruct<Fs...>>::assign_ref(ref, tuple);
    return *this;
  }

  template <class... Fs>
  crust_cxx14_constexpr LetTuple &operator=(TupleStruct<Fs...> &tuple) {
    LetTupleHelper<
        sizeof...(Fields),
        sizeof...(Fs),
        TupleStruct<LetField<Fields>...>,
        TupleStruct<Fs...>>::assign_ref_mut(ref, tuple);
    return *this;
  }

  template <class... Fs>
  crust_cxx14_constexpr LetTuple &operator=(TupleStruct<Fs...> &&tuple) {
    LetTupleHelper<
        sizeof...(Fields),
        sizeof...(Fs),
        TupleStruct<LetField<Fields>...>,
        TupleStruct<Fs...>>::assign_move(ref, move(tuple));
    return *this;
  }
};
} // namespace _impl_tuple

template <class... Fields>
crust_cxx14_constexpr
    _impl_tuple::LetTuple<typename RemoveConstOrRefType<Fields>::Result...>
    let(Fields &...fields) {
  return _impl_tuple::LetTuple<
      typename RemoveConstOrRefType<Fields>::Result...>{fields...};
}
} // namespace crust

namespace std {

/// c++ std bindings

template <class... Fields>
struct tuple_size<crust::Tuple<Fields...>> :
    integral_constant<
        crust::usize,
        crust::_impl_derive::TupleLikeSize<
            crust::TupleStruct<Fields...>>::result> {};

template <crust::usize index, class... Fields>
struct tuple_element<index, crust::Tuple<Fields...>> {
  using type = typename crust::_impl_derive::
      TupleLikeGetter<crust::TupleStruct<Fields...>, index>::Result;
};

template <crust::usize index, class... Fields>
constexpr const typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(const crust::TupleStruct<Fields...> &object) {
  return crust::_impl_derive::
      TupleLikeGetter<crust::TupleStruct<Fields...>, index>::get(object);
}

template <crust::usize index, class... Fields>
constexpr typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(crust::TupleStruct<Fields...> &object) {
  return crust::_impl_derive::
      TupleLikeGetter<crust::TupleStruct<Fields...>, index>::get(object);
}
} // namespace std


#endif // CRUST_TUPLE_HPP

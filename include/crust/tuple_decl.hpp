#ifndef CRUST_TUPLE_DECL_HPP
#define CRUST_TUPLE_DECL_HPP


#include "crust/clone.hpp"
#include "crust/cmp_decl.hpp"
#include "crust/helper/types.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_tuple {
template <class Ts>
struct AllZeroSizedType;

template <class... Fields>
struct AllZeroSizedType<_impl_types::Types<Fields...>> :
    All<Require<Fields, ZeroSizedType>...> {};

template <bool is_zst, bool remain_is_zst, class Ts>
struct TupleSizedHolderImpl;

template <class Ts>
using TupleSizedHolderIsZST =
    Require<typename _impl_types::TypesIndex<0, Ts>::Result, ZeroSizedType>;

template <class Ts>
using TupleSizedHolderRemainZST =
    AllZeroSizedType<typename _impl_types::TypesAfter<0, Ts>::Result>;

template <class... Fields>
using TupleSizedHolder = TupleSizedHolderImpl<
    TupleSizedHolderIsZST<_impl_types::Types<Fields...>>::result,
    TupleSizedHolderRemainZST<_impl_types::Types<Fields...>>::result,
    _impl_types::Types<Fields...>>;

template <class Field, class... Fields>
struct TupleSizedHolderImpl<
    false,
    false,
    _impl_types::Types<Field, Fields...>> {
  Field field;
  TupleSizedHolder<Fields...> remains;

  constexpr TupleSizedHolderImpl() : field{}, remains{} {}

  template <class T, class... Ts>
  explicit constexpr TupleSizedHolderImpl(T &&field, Ts &&...fields) :
      field{forward<T>(field)}, remains{forward<Ts>(fields)...} {}
};

template <class Field, class... Fields>
struct TupleSizedHolderImpl<false, true, _impl_types::Types<Field, Fields...>> {
  Field field;

  constexpr TupleSizedHolderImpl() : field{} {}

  explicit constexpr TupleSizedHolderImpl(Field &&field, Fields &&...) :
      field{forward<Field>(field)} {}
};

template <class Field, class... Fields>
struct TupleSizedHolderImpl<true, false, _impl_types::Types<Field, Fields...>> {
  TupleSizedHolder<Fields...> remains;

  constexpr TupleSizedHolderImpl() : remains{} {}

  explicit constexpr TupleSizedHolderImpl(Field &&, Fields &&...fields) :
      remains{forward<Fields>(fields)...} {}
};

template <class... Fields>
struct TupleSizedHolderImpl<true, true, _impl_types::Types<Fields...>> {
  constexpr TupleSizedHolderImpl() {}

  explicit constexpr TupleSizedHolderImpl(Fields &&...) {}
};

template <usize index, bool is_zst, class... Fields>
struct TupleGetterImpl;

template <usize index, class... Fields>
struct TupleGetterImpl<index, true, Fields...> :
    _impl_types::ZeroSizedTypeGetter<index, Fields...> {};

template <usize index, class Field, class... Fields>
struct TupleGetterImpl<index, false, Field, Fields...> {
  using Self = TupleSizedHolder<Field, Fields...>;
  using Result = typename _impl_types::
      TypesIndex<index, _impl_types::Types<Field, Fields...>>::Result;

  static constexpr const Result &inner(const Self &self) {
    return TupleGetterImpl<index - 1, false, Fields...>::inner(self.remains);
  }

  static constexpr Result &inner(Self &self) {
    return TupleGetterImpl<index - 1, false, Fields...>::inner(self.remains);
  }
};

template <class Field, class... Fields>
struct TupleGetterImpl<0, false, Field, Fields...> {
  using Self = TupleSizedHolder<Field, Fields...>;
  using Result = Field;

  static constexpr const Result &inner(const Self &self) { return self.field; }

  static constexpr Result &inner(Self &self) { return self.field; }
};

template <usize index, class... Fields>
using TupleGetter = TupleGetterImpl<
    index,
    Require<
        typename _impl_types::TypesIndex<index, _impl_types::Types<Fields...>>::
            Result,
        ZeroSizedType>::result,
    Fields...>;
} // namespace _impl_tuple

template <class... Fields>
struct TupleStruct :
    private _impl_types::ZeroSizedTypeHolder<Fields...>,
    private _impl_tuple::TupleSizedHolder<Fields...> {
private:
  crust_static_assert(All<Not<IsConstOrRefVal<Fields>>...>::result);

  template <usize index>
  using Getter = _impl_tuple::TupleGetter<index, Fields...>;

protected:
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleStruct, _impl_tuple::TupleSizedHolder<Fields...>);

public:
  template <usize index>
  constexpr const typename Getter<index>::Result &get() const {
    return Getter<index>::inner(*this);
  }

  template <usize index>
  crust_cxx14_constexpr typename Getter<index>::Result &get() {
    return Getter<index>::inner(*this);
  }
};

template <>
struct TupleStruct<> {
protected:
  constexpr TupleStruct() {}
};

namespace _impl_derive {
template <class... Fields>
struct TupleLikeSize<TupleStruct<Fields...>> :
    TmplVal<usize, sizeof...(Fields)> {};

template <usize index, class... Fields>
struct TupleLikeGetter<TupleStruct<Fields...>, index> {
  using Result = typename _impl_types::
      TypesIndex<index, _impl_types::Types<Fields...>>::Result;

  static constexpr const Result &get(const TupleStruct<Fields...> &self) {
    return self.template get<index>();
  }

  static constexpr Result &get(TupleStruct<Fields...> &self) {
    return self.template get<index>();
  }
};

template <usize... indexs>
struct IndexSequence {};

template <usize size, usize... indexs>
struct IndexSequenceImpl : IndexSequenceImpl<size - 1, size - 1, indexs...> {};

template <usize... indexs>
struct IndexSequenceImpl<0, indexs...> : TmplType<IndexSequence<indexs...>> {};

template <usize size>
using MakeIndexSequence = typename IndexSequenceImpl<size>::Result;

template <class Self, class Index>
struct TupleLikeCloneHelper;

template <class Self, usize... indexs>
struct TupleLikeCloneHelper<Self, IndexSequence<indexs...>> {
  static constexpr Self clone(const Self &self) {
    return Self{clone::clone(self.template get<indexs>())...};
  }
};

template <class T, template <class, class...> class Trait, class... Args>
struct ImplForTupleStructHelper : TmplVal<bool, false> {};

template <
    class... Fields,
    template <class, class...>
    class Trait,
    class... Args>
struct ImplForTupleStructHelper<TupleStruct<Fields...>, Trait, Args...> :
    All<Require<Fields, Trait, Args...>...> {};

template <class T, template <class, class...> class Trait, class... Args>
using ImplForTupleStruct =
    ImplForTupleStructHelper<typename BluePrint<T>::Result, Trait, Args...>;
} // namespace _impl_derive

template <class S>
CRUST_IMPL_FOR(
    ZeroSizedType<S>, _impl_derive::ImplForTupleStruct<S, ZeroSizedType>){};

template <class S>
CRUST_IMPL_FOR(
    clone::Clone<S>, _impl_derive::ImplForTupleStruct<S, clone::Clone>) {
  CRUST_IMPL_USE_SELF(S);

private:
  using CloneHelper = _impl_derive::TupleLikeCloneHelper<
      Self,
      _impl_derive::MakeIndexSequence<
          _impl_derive::TupleLikeSize<typename BluePrint<S>::Result>::result>>;

public:
  Self clone() const { return CloneHelper::clone(self()); }
};

template <class S>
CRUST_IMPL_FOR(
    cmp::PartialEq<S>, _impl_derive::ImplForTupleStruct<S, cmp::PartialEq>) {
  CRUST_IMPL_USE_SELF(S);

private:
  using PartialEqHelper = _impl_derive::
      TupleLikePartialEqHelper<Self, typename BluePrint<S>::Result>;

public:
  constexpr bool eq(const Self &other) const {
    return PartialEqHelper::eq(self(), other);
  }

  constexpr bool ne(const Self &other) const {
    return PartialEqHelper::ne(self(), other);
  }
};

template <class S>
CRUST_IMPL_FOR(cmp::Eq<S>, _impl_derive::ImplForTupleStruct<S, cmp::Eq>){};

template <class S>
CRUST_IMPL_FOR(
    cmp::PartialOrd<S>, _impl_derive::ImplForTupleStruct<S, cmp::PartialOrd>) {
  CRUST_IMPL_USE_SELF(S);

private:
  using PartialOrdHelper = _impl_derive::
      TupleLikePartialOrdHelper<Self, typename BluePrint<S>::Result>;

public:
  constexpr Option<cmp::Ordering> partial_cmp(const Self &other) const;

  constexpr bool lt(const Self &other) const {
    return PartialOrdHelper::lt(self(), other);
  }

  constexpr bool le(const Self &other) const {
    return PartialOrdHelper::le(self(), other);
  }

  constexpr bool gt(const Self &other) const {
    return PartialOrdHelper::gt(self(), other);
  }

  constexpr bool ge(const Self &other) const {
    return PartialOrdHelper::ge(self(), other);
  }
};

template <class S>
CRUST_IMPL_FOR(cmp::Ord<S>, _impl_derive::ImplForTupleStruct<S, cmp::Ord>) {
  CRUST_IMPL_USE_SELF(S);

private:
  using OrdHelper =
      _impl_derive::TupleLikeOrdHelper<Self, typename BluePrint<S>::Result>;

public:
  constexpr cmp::Ordering cmp(const Self &other) const;
};

template <class... Fields>
struct crust_ebco Tuple :
    TupleStruct<Fields...>,
    Derive<
        Tuple<Fields...>,
        Trait<ZeroSizedType>,
        Trait<clone::Clone>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>,
        Trait<cmp::PartialOrd>,
        Trait<cmp::Ord>> {
  CRUST_USE_BASE_CONSTRUCTORS(Tuple, TupleStruct<Fields...>);
};

template <class... Fields>
struct BluePrint<Tuple<Fields...>> : TmplType<TupleStruct<Fields...>> {};

template <class... Fields, class U>
struct ZeroSizedTypeConvertible<Tuple<Fields...>, U> :
    Any<_impl_types::TypesIncludeVal<U, _impl_types::Types<Fields...>>,
        _impl_types::TypesContainVal<U, _impl_types::Types<Fields...>>> {};

namespace _impl_tuple {
template <class T, class U, bool is_contain>
struct ZeroSizedTypeConvertHelper;

template <class... Fields, class U>
struct ZeroSizedTypeConvertHelper<Tuple<Fields...>, U, true> {
  static constexpr usize index =
      _impl_types::TypesFirstContain<U, _impl_types::Types<Fields...>>::result;
  using Convert = ZeroSizedTypeConvert<
      typename _impl_types::TypesIndex<index, _impl_types::Types<Fields...>>::
          Result,
      U>;

  static constexpr const U &inner(const Tuple<Fields...> &self) {
    return Convert::inner(self.template get<index>());
  }

  static constexpr U &inner(Tuple<Fields...> &self) {
    return Convert::inner(self.template get<index>());
  }
};

template <class... Fields, class U>
struct ZeroSizedTypeConvertHelper<Tuple<Fields...>, U, false> {
  static constexpr usize index =
      _impl_types::TypesFirstIndex<U, _impl_types::Types<Fields...>>::result;

  static constexpr const U &inner(const Tuple<Fields...> &self) {
    return self.template get<index>();
  }

  static constexpr U &inner(Tuple<Fields...> &self) {
    return self.template get<index>();
  }
};
} // namespace _impl_tuple

template <class... Fields, class U>
struct ZeroSizedTypeConvert<Tuple<Fields...>, U> :
    _impl_tuple::ZeroSizedTypeConvertHelper<
        Tuple<Fields...>,
        U,
        _impl_types::TypesContainVal<U, _impl_types::Types<Fields...>>::
            result> {};
} // namespace crust


#endif // CRUST_TUPLE_DECL_HPP

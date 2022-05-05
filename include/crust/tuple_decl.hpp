#ifndef CRUST_TUPLE_DECL_HPP
#define CRUST_TUPLE_DECL_HPP


#include "crust/clone.hpp"
#include "crust/cmp_decl.hpp"
#include "crust/helper/types.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_tuple {
template <bool is_zst, bool remain_is_zst, class... Fields>
struct TupleHolderSizedImpl;

template <class... Fields>
struct crust_ebco TupleSizedHolder;

template <class Field, class... Fields>
struct TupleHolderSizedImpl<false, false, Field, Fields...> {
  Field field;
  TupleSizedHolder<Fields...> remains;

  constexpr TupleHolderSizedImpl() : field{}, remains{} {}

  template <class T, class... Ts>
  explicit constexpr TupleHolderSizedImpl(T &&field, Ts &&...fields) :
      field{forward<T>(field)}, remains{forward<Ts>(fields)...} {}
};

template <class Field, class... Fields>
struct TupleHolderSizedImpl<false, true, Field, Fields...> {
  Field field;

  constexpr TupleHolderSizedImpl() : field{} {}

  explicit constexpr TupleHolderSizedImpl(Field &&field, Fields &&...) :
      field{forward<Field>(field)} {}
};

template <class Field, class... Fields>
struct TupleHolderSizedImpl<true, false, Field, Fields...> {
  TupleSizedHolder<Fields...> remains;

  constexpr TupleHolderSizedImpl() : remains{} {}

  explicit constexpr TupleHolderSizedImpl(Field &&, Fields &&...fields) :
      remains{forward<Fields>(fields)...} {}
};

template <class... Fields>
struct TupleHolderSizedImpl<true, true, Fields...> {
  constexpr TupleHolderSizedImpl() {}

  explicit constexpr TupleHolderSizedImpl(Fields &&...) {}
};

template <class Field, class... Fields>
struct TupleSizedHolder<Field, Fields...> :
    TupleHolderSizedImpl<
        Require<Field, ZeroSizedType>::result,
        All<Require<Fields, ZeroSizedType>...>::result,
        Field,
        Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleSizedHolder,
      TupleHolderSizedImpl<
          Require<Field, ZeroSizedType>::result,
          All<Require<Fields, ZeroSizedType>...>::result,
          Field,
          Fields...>);
};

template <class Field>
struct TupleSizedHolder<Field> :
    TupleHolderSizedImpl<Require<Field, ZeroSizedType>::result, true, Field> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleSizedHolder,
      TupleHolderSizedImpl<Require<Field, ZeroSizedType>::result, true, Field>);
};

template <bool is_all_zst, class... Fields>
struct crust_ebco TupleHolder;

template <class... Fields>
struct TupleHolder<false, Fields...> :
    InheritIf<
        _impl_types::ZeroSizedTypeHolder<Fields...>,
        Any<Require<Fields, ZeroSizedType>...>>,
    TupleSizedHolder<Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(TupleHolder, TupleSizedHolder<Fields...>);
};

template <class... Fields>
struct TupleHolder<true, Fields...> :
    _impl_types::ZeroSizedTypeHolder<Fields...> {
  constexpr TupleHolder() : _impl_types::ZeroSizedTypeHolder<Fields...>{} {}

  constexpr TupleHolder(Fields &&...) :
      _impl_types::ZeroSizedTypeHolder<Fields...>{} {}
};

template <>
struct TupleHolder<true> {};

template <usize index, bool is_zst, class... Fields>
struct TupleGetterImpl;

template <usize index, class... Fields>
using TupleGetter = TupleGetterImpl<
    index,
    Require<
        typename _impl_types::
            TypesIndexToType<index, _impl_types::Types<Fields...>>::Result,
        ZeroSizedType>::result,
    Fields...>;

template <usize index, class... Fields>
struct TupleGetterImpl<index, true, Fields...> :
    _impl_types::ZeroSizedTypeGetter<index, Fields...> {};

template <usize index, class Field, class... Fields>
struct TupleGetterImpl<index, false, Field, Fields...> {
  using Self = TupleSizedHolder<Field, Fields...>;
  using Result = typename _impl_types::
      TypesIndexToType<index, _impl_types::Types<Field, Fields...>>::Result;

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
} // namespace _impl_tuple
// todo: repr(transparent)
template <class... Fields>
struct TupleStruct :
    private _impl_tuple::
        TupleHolder<All<Require<Fields, ZeroSizedType>...>::result, Fields...> {
private:
  crust_static_assert(All<Not<IsConstOrRefVal<Fields>>...>::result);

  template <usize index>
  using Getter = _impl_tuple::TupleGetter<index, Fields...>;

  template <usize index>
  using Result = typename Getter<index>::Result;

protected:
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleStruct,
      _impl_tuple::TupleHolder<
          All<Require<Fields, ::crust::ZeroSizedType>...>::result,
          Fields...>);

public:
  template <usize index>
  constexpr const Result<index> &get() const {
    return Getter<index>::inner(*this);
  }

  template <usize index>
  crust_cxx14_constexpr Result<index> &get() {
    return Getter<index>::inner(*this);
  }
};

namespace _impl_derive {
template <class... Fields>
struct TupleLikeSize<TupleStruct<Fields...>> :
    TmplVal<usize, sizeof...(Fields)> {};

template <usize index, class... Fields>
struct TupleLikeGetter<TupleStruct<Fields...>, index> {
  using Result = typename _impl_types::
      TypesIndexToType<index, _impl_types::Types<Fields...>>::Result;

  static constexpr const Result &get(const TupleStruct<Fields...> &self) {
    return self.template get<index>();
  }

  static constexpr Result &get(TupleStruct<Fields...> &self) {
    return self.template get<index>();
  }
};

template <class Self, class... Fields>
struct Derive<
    Self,
    TupleStruct<Fields...>,
    ZeroSizedType,
    EnableIf<Require<Fields, ZeroSizedType>...>> : ZeroSizedType<Self> {};

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

template <class Self, class... Fields>
struct Derive<
    Self,
    TupleStruct<Fields...>,
    clone::Clone,
    EnableIf<Require<Fields, clone::Clone>...>> : clone::Clone<Self> {
  CRUST_TRAIT_USE_SELF(Derive);

private:
  using CloneHelper =
      TupleLikeCloneHelper<Self, MakeIndexSequence<sizeof...(Fields)>>;

public:
  Self clone() const { return CloneHelper::clone(self()); }
};

template <class Self, class... Fields>
struct Derive<
    Self,
    TupleStruct<Fields...>,
    cmp::PartialEq,
    EnableIf<Require<Fields, cmp::PartialEq>...>> : cmp::PartialEq<Self> {
  CRUST_TRAIT_USE_SELF(Derive);

private:
  using PartialEqHelper =
      _impl_derive::TupleLikePartialEqHelper<Self, TupleStruct<Fields...>>;

public:
  constexpr bool eq(const Self &other) const {
    return PartialEqHelper::eq(self(), other);
  }

  constexpr bool ne(const Self &other) const {
    return PartialEqHelper::ne(self(), other);
  }
};

template <class Self, class... Fields>
struct Derive<
    Self,
    TupleStruct<Fields...>,
    cmp::Eq,
    EnableIf<Require<Fields, cmp::Eq>...>> : cmp::Eq<Self> {};

template <class Self, class... Fields>
struct Derive<
    Self,
    TupleStruct<Fields...>,
    cmp::PartialOrd,
    EnableIf<Require<Fields, cmp::PartialOrd>...>> : cmp::PartialOrd<Self> {
  CRUST_TRAIT_USE_SELF(Derive);

private:
  using PartialOrdHelper =
      _impl_derive::TupleLikePartialOrdHelper<Self, TupleStruct<Fields...>>;

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

template <class Self, class... Fields>
struct Derive<
    Self,
    TupleStruct<Fields...>,
    cmp::Ord,
    EnableIf<Require<Fields, cmp::Ord>...>> : cmp::Ord<Self> {
  CRUST_TRAIT_USE_SELF(Derive);

  constexpr cmp::Ordering cmp(const Self &other) const;
};
} // namespace _impl_derive

template <class... Fields>
struct crust_ebco Tuple :
    TupleStruct<Fields...>,
    Derive<
        Tuple<Fields...>,
        TupleStruct<Fields...>,
        ZeroSizedType,
        clone::Clone,
        cmp::PartialEq,
        cmp::Eq,
        cmp::PartialOrd,
        cmp::Ord> {
  CRUST_USE_BASE_CONSTRUCTORS(Tuple, TupleStruct<Fields...>);
};
} // namespace crust


#endif // CRUST_TUPLE_DECL_HPP

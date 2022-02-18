#ifndef CRUST_TUPLE_DECL_HPP
#define CRUST_TUPLE_DECL_HPP


#include "crust/cmp_decl.hpp"
#include "crust/helper/types.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace option {
template <class T>
class Option;
} // namespace option

using option::Option;

namespace _impl_tuple {
template <bool is_zst, bool remain_is_zst, class... Fields>
struct TupleHolderSizedImpl;

template <class... Fields>
struct crust_ebco TupleSizedHolder;

template <class Field, class... Fields>
struct TupleHolderSizedImpl<false, false, Field, Fields...> {
  Field field;
  TupleSizedHolder<Fields...> remains;

  constexpr TupleHolderSizedImpl() {}

  template <class T, class... Ts>
  explicit constexpr TupleHolderSizedImpl(T &&field, Ts &&...fields) :
      field{forward<T>(field)}, remains{forward<Ts>(fields)...} {}
};

template <class Field, class... Fields>
struct TupleHolderSizedImpl<false, true, Field, Fields...> {
  Field field;

  constexpr TupleHolderSizedImpl() {}

  explicit constexpr TupleHolderSizedImpl(Field &&field, Fields &&...) :
      field{forward<Field>(field)} {}
};

template <class Field, class... Fields>
struct TupleHolderSizedImpl<true, false, Field, Fields...> {
  TupleSizedHolder<Fields...> remains;

  constexpr TupleHolderSizedImpl() {}

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
        Derive<Field, ZeroSizedType>::result,
        All<Derive<Fields, ZeroSizedType>...>::result,
        Field,
        Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleSizedHolder,
      TupleHolderSizedImpl<
          Derive<Field, ZeroSizedType>::result,
          All<Derive<Fields, ZeroSizedType>...>::result,
          Field,
          Fields...>);
};

template <class Field>
struct TupleSizedHolder<Field> :
    TupleHolderSizedImpl<Derive<Field, ZeroSizedType>::result, true, Field> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleSizedHolder,
      TupleHolderSizedImpl<Derive<Field, ZeroSizedType>::result, true, Field>);
};

template <bool is_all_zst, class... Fields>
struct crust_ebco TupleHolder;

template <class... Fields>
struct TupleHolder<false, Fields...> :
    InheritIf<
        _impl_types::ZeroSizedTypeHolder<Fields...>,
        Any<Derive<Fields, ZeroSizedType>...>>,
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
    Derive<
        typename _impl_types::TypesIndexToType<index, Fields...>::Result,
        ZeroSizedType>::result,
    Fields...>;

template <usize index, class... Fields>
struct TupleGetterImpl<index, true, Fields...> :
    _impl_types::ZeroSizedTypeGetter<index, Fields...> {};

template <usize index, class Field, class... Fields>
struct TupleGetterImpl<index, false, Field, Fields...> {
  using Self = TupleSizedHolder<Field, Fields...>;
  using Result =
      typename _impl_types::TypesIndexToType<index, Field, Fields...>::Result;

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

template <class... Fields>
struct TupleStruct :
    private _impl_tuple::
        TupleHolder<All<Derive<Fields, ZeroSizedType>...>::result, Fields...> {
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
          All<Derive<Fields, ::crust::ZeroSizedType>...>::result,
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

namespace _auto_impl {
template <class... Fields>
struct TupleLikeSize<TupleStruct<Fields...>> :
    TmplVal<usize, sizeof...(Fields)> {};

template <usize index, class... Fields>
struct TupleLikeGetter<TupleStruct<Fields...>, index> {
  using Result =
      typename _impl_types::TypesIndexToType<index, Fields...>::Result;

  static constexpr const Result &get(const TupleStruct<Fields...> &self) {
    return self.template get<index>();
  }

  static constexpr Result &get(TupleStruct<Fields...> &self) {
    return self.template get<index>();
  }
};

template <class Self, class... Fields>
struct AutoImpl<
    Self,
    TupleStruct<Fields...>,
    ZeroSizedType,
    EnableIf<Derive<Fields, ZeroSizedType>...>> : ZeroSizedType<Self> {};

template <class Self, class... Fields>
struct AutoImpl<
    Self,
    TupleStruct<Fields...>,
    cmp::PartialEq,
    EnableIf<Derive<Fields, cmp::PartialEq>...>> : cmp::PartialEq<Self> {
  CRUST_TRAIT_USE_SELF(AutoImpl);

private:
  using PartialEqHelper =
      _auto_impl::TupleLikePartialEqHelper<Self, TupleStruct<Fields...>>;

public:
  constexpr bool eq(const Self &other) const {
    return PartialEqHelper::eq(self(), other);
  }

  constexpr bool ne(const Self &other) const {
    return PartialEqHelper::ne(self(), other);
  }
};

template <class Self, class... Fields>
struct AutoImpl<
    Self,
    TupleStruct<Fields...>,
    cmp::Eq,
    EnableIf<Derive<Fields, cmp::Eq>...>> : cmp::Eq<Self> {};

template <class Self, class... Fields>
struct AutoImpl<
    Self,
    TupleStruct<Fields...>,
    cmp::PartialOrd,
    EnableIf<Derive<Fields, cmp::PartialOrd>...>> : cmp::PartialOrd<Self> {
  CRUST_TRAIT_USE_SELF(AutoImpl);

private:
  using PartialOrdHelper =
      _auto_impl::TupleLikePartialOrdHelper<Self, TupleStruct<Fields...>>;

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
struct AutoImpl<
    Self,
    TupleStruct<Fields...>,
    cmp::Ord,
    EnableIf<Derive<Fields, cmp::Ord>...>> : cmp::Ord<Self> {
  CRUST_TRAIT_USE_SELF(AutoImpl);

  constexpr cmp::Ordering cmp(const Self &other) const;
};
} // namespace _auto_impl

template <class... Fields>
struct crust_ebco Tuple :
    TupleStruct<Fields...>,
    AutoImpl<
        Tuple<Fields...>,
        TupleStruct<Fields...>,
        ZeroSizedType,
        cmp::PartialEq,
        cmp::Eq,
        cmp::PartialOrd,
        cmp::Ord> {
  CRUST_USE_BASE_CONSTRUCTORS(Tuple, TupleStruct<Fields...>);
};
} // namespace crust


#endif // CRUST_TUPLE_DECL_HPP

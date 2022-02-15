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
        AllVal<Derive<Fields, ZeroSizedType>...>::result,
        Field,
        Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleSizedHolder,
      TupleHolderSizedImpl<
          Derive<Field, ZeroSizedType>::result,
          AllVal<Derive<Fields, ZeroSizedType>...>::result,
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
    Impl<
        _impl_types::ZeroSizedTypeHolder<Fields...>,
        AnyVal<Derive<Fields, ZeroSizedType>...>>,
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
struct TupleGetter;

template <usize index, class Field, class... Fields>
struct TupleGetterImpl<index, true, Field, Fields...> {
  using Self = _impl_types::ZeroSizedTypeHolder<Field, Fields...>;
  using Result =
      typename _impl_types::TypesIndexToType<index, Field, Fields...>::Result;

  static constexpr const Result &inner(const Self &self) { return self; }

  static constexpr Result &inner(Self &self) { return self; }
};

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

template <usize index, class Field, class... Fields>
struct TupleGetter<index, Field, Fields...> :
    TupleGetterImpl<
        index,
        Derive<
            typename _impl_types::TypesIndexToType<index, Field, Fields...>::
                Result,
            ZeroSizedType>::result,
        Field,
        Fields...> {};
} // namespace _impl_tuple

template <class... Fields>
struct TupleStruct :
    private _impl_tuple::TupleHolder<
        AllVal<Derive<Fields, ZeroSizedType>...>::result,
        Fields...> {
private:
  crust_static_assert(AllVal<NotVal<IsConstOrRefVal<Fields>>...>::result);

  template <usize index>
  using Getter = _impl_tuple::TupleGetter<index, Fields...>;

  template <usize index>
  using Result = typename Getter<index>::Result;

protected:
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleStruct,
      _impl_tuple::TupleHolder<
          AllVal<Derive<Fields, ::crust::ZeroSizedType>...>::result,
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
struct AutoImpl<Self, TupleStruct<Fields...>, ZeroSizedType> :
    Impl<ZeroSizedType<Self>, Derive<Fields, ZeroSizedType>...> {};

template <class Self, class Base>
struct crust_ebco TuplePartialEqImpl : cmp::PartialEq<Self> {
  CRUST_TRAIT_REQUIRE(TuplePartialEqImpl);

  constexpr bool eq(const Self &other) const {
    return TupleLikePartialEqHelper<Self, Base>::eq(self(), other);
  }

  constexpr bool ne(const Self &other) const {
    return TupleLikePartialEqHelper<Self, Base>::ne(self(), other);
  }
};

template <class Self, class... Fields>
struct AutoImpl<Self, TupleStruct<Fields...>, cmp::PartialEq> :
    Impl<
        TuplePartialEqImpl<Self, TupleStruct<Fields...>>,
        Derive<Fields, cmp::PartialEq>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
};

template <class Self, class... Fields>
struct AutoImpl<Self, TupleStruct<Fields...>, cmp::Eq> :
    Impl<cmp::Eq<Self>, Derive<Fields, cmp::Eq>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
};

template <class Self, class Base>
struct crust_ebco TuplePartialOrdImpl : cmp::PartialOrd<Self> {
  CRUST_TRAIT_REQUIRE(TuplePartialOrdImpl);

  constexpr Option<cmp::Ordering> partial_cmp(const Self &other) const;

  constexpr bool lt(const Self &other) const {
    return TupleLikePartialOrdHelper<Self, Base>::lt(self(), other);
  }

  constexpr bool le(const Self &other) const {
    return TupleLikePartialOrdHelper<Self, Base>::le(self(), other);
  }

  constexpr bool gt(const Self &other) const {
    return TupleLikePartialOrdHelper<Self, Base>::gt(self(), other);
  }

  constexpr bool ge(const Self &other) const {
    return TupleLikePartialOrdHelper<Self, Base>::ge(self(), other);
  }
};

template <class Self, class... Fields>
struct AutoImpl<Self, TupleStruct<Fields...>, cmp::PartialOrd> :
    Impl<
        TuplePartialOrdImpl<Self, TupleStruct<Fields...>>,
        Derive<Fields, cmp::PartialOrd>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
};

template <class Self, class Base>
struct crust_ebco TupleOrdImpl : cmp::Ord<Self> {
  CRUST_TRAIT_REQUIRE(TupleOrdImpl);

  constexpr cmp::Ordering cmp(const Self &other) const;
};

template <class Self, class... Fields>
struct AutoImpl<Self, TupleStruct<Fields...>, cmp::Ord> :
    Impl<
        TupleOrdImpl<Self, TupleStruct<Fields...>>,
        Derive<Fields, cmp::Ord>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
};
} // namespace _auto_impl

template <class... Fields>
struct crust_ebco Tuple :
    TupleStruct<Fields...>,
    Impl<ZeroSizedType<Tuple<Fields...>>, Derive<Fields, ZeroSizedType>...>,
    Impl<cmp::PartialEq<Tuple<Fields...>>, Derive<Fields, cmp::PartialEq>...>,
    Impl<cmp::Eq<Tuple<Fields...>>, Derive<Fields, cmp::Eq>...>,
    Impl<cmp::PartialOrd<Tuple<Fields...>>, Derive<Fields, cmp::PartialOrd>...>,
    Impl<cmp::Ord<Tuple<Fields...>>, Derive<Fields, cmp::Ord>...> {
private:
  using PartialEqHelper = _auto_impl::
      TupleLikePartialEqHelper<Tuple<Fields...>, TupleStruct<Fields...>>;
  using PartialOrdHelper = _auto_impl::
      TupleLikePartialOrdHelper<Tuple<Fields...>, TupleStruct<Fields...>>;
  using OrdHelper =
      _auto_impl::TupleLikeOrdHelper<Tuple<Fields...>, TupleStruct<Fields...>>;

public:
  CRUST_USE_BASE_CONSTRUCTORS(Tuple, TupleStruct<Fields...>);

  constexpr bool eq(const Tuple<Fields...> &other) const {
    return PartialEqHelper::eq(*this, other);
  }

  constexpr bool ne(const Tuple<Fields...> &other) const {
    return PartialEqHelper::ne(*this, other);
  }

  constexpr Option<cmp::Ordering>
  partial_cmp(const Tuple<Fields...> &other) const;

  constexpr bool lt(const Tuple<Fields...> &other) const {
    return PartialOrdHelper::lt(*this, other);
  }

  constexpr bool le(const Tuple<Fields...> &other) const {
    return PartialOrdHelper::le(*this, other);
  }

  constexpr bool gt(const Tuple<Fields...> &other) const {
    return PartialOrdHelper::gt(*this, other);
  }

  constexpr bool ge(const Tuple<Fields...> &other) const {
    return PartialOrdHelper::ge(*this, other);
  }

  constexpr cmp::Ordering cmp(const Tuple<Fields...> &other) const;
};

namespace _impl_tuple {
template <usize index, class... Fields>
struct TupleEqHelper {
  static constexpr bool inner(
      TupleSizedHolder<const Fields &...> ref, const TupleStruct<Fields...> &tuple) {
    return TupleEqHelper<index - 1, Fields...>::inner(ref, tuple) &&
        TupleGetter<index - 1, const Fields &...>::inner(ref) ==
        tuple.template get<index - 1>();
  }
};

template <class... Fields>
struct TupleEqHelper<0, Fields...> {
  static crust_cxx14_constexpr bool
  inner(TupleSizedHolder<const Fields &...>, const TupleStruct<Fields...> &) {
    return true;
  }
};

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
} // namespace _impl_tuple
} // namespace crust


#endif // CRUST_TUPLE_DECL_HPP

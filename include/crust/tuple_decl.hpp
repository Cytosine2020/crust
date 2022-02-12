#ifndef CRUST_TUPLE_DECL_HPP
#define CRUST_TUPLE_DECL_HPP


#include "crust/cmp_decl.hpp"
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
struct TupleSizedHolder;

template <class Field, class... Fields>
struct TupleHolderSizedImpl<false, false, Field, Fields...> {
  Field field;
  TupleSizedHolder<Fields...> remains;

  constexpr TupleHolderSizedImpl() : field{}, remains{} {}

  template <class T, class... Ts>
  explicit constexpr TupleHolderSizedImpl(T &&field, Ts &&...fields) :
      field{forward<T>(field)}, remains{forward<Ts>(fields)...} {}

  constexpr bool _eq(const TupleHolderSizedImpl &other) const {
    return field == other.field && remains._eq(other.remains);
  }

  constexpr bool _ne(const TupleHolderSizedImpl &other) const {
    return field != other.field || remains._ne(other.remains);
  }

  constexpr Option<cmp::Ordering>
  _partial_cmp(const TupleHolderSizedImpl &other) const;

  constexpr bool _lt(const TupleHolderSizedImpl &other) const {
    return field != other.field ? field < other.field :
                                  remains._lt(other.remains);
  }

  constexpr bool _le(const TupleHolderSizedImpl &other) const {
    return field != other.field ? field <= other.field :
                                  remains._le(other.remains);
  }

  constexpr bool _gt(const TupleHolderSizedImpl &other) const {
    return field != other.field ? field > other.field :
                                  remains._gt(other.remains);
  }

  constexpr bool _ge(const TupleHolderSizedImpl &other) const {
    return field != other.field ? field >= other.field :
                                  remains._ge(other.remains);
  }

  constexpr cmp::Ordering _cmp(const TupleHolderSizedImpl &other) const;
};

template <class Field, class... Fields>
struct TupleHolderSizedImpl<false, true, Field, Fields...> {
  Field field;

  constexpr TupleHolderSizedImpl() : field{} {}

  explicit constexpr TupleHolderSizedImpl(Field &&field, Fields &&...) :
      field{forward<Field>(field)} {}

  constexpr bool _eq(const TupleHolderSizedImpl &other) const {
    return field == other.field;
  }

  constexpr bool _ne(const TupleHolderSizedImpl &other) const {
    return field != other.field;
  }

  constexpr Option<cmp::Ordering>
  _partial_cmp(const TupleHolderSizedImpl &other) const;

  constexpr bool _lt(const TupleHolderSizedImpl &other) const {
    return field < other.field;
  }

  constexpr bool _le(const TupleHolderSizedImpl &other) const {
    return field <= other.field;
  }

  constexpr bool _gt(const TupleHolderSizedImpl &other) const {
    return field > other.field;
  }

  constexpr bool _ge(const TupleHolderSizedImpl &other) const {
    return field >= other.field;
  }

  constexpr cmp::Ordering _cmp(const TupleHolderSizedImpl &other) const;
};

template <class Field, class... Fields>
struct crust_ebco TupleHolderSizedImpl<true, false, Field, Fields...> {
  TupleSizedHolder<Fields...> remains;

  constexpr TupleHolderSizedImpl() : remains{} {}

  explicit constexpr TupleHolderSizedImpl(Field &&, Fields &&...fields) :
      remains{forward<Fields>(fields)...} {}
};

template <class... Fields>
struct crust_ebco TupleHolderSizedImpl<true, true, Fields...> {
  constexpr TupleHolderSizedImpl() :
      TupleHolderSizedImpl<true, true, Fields...>{} {}

  explicit constexpr TupleHolderSizedImpl(Fields &&...) :
      TupleHolderSizedImpl<true, true, Fields...>{} {}

  constexpr bool _eq(const TupleHolderSizedImpl &) const { return true; }

  constexpr bool _ne(const TupleHolderSizedImpl &) const { return false; }

  constexpr Option<cmp::Ordering>
  _partial_cmp(const TupleHolderSizedImpl &) const;

  constexpr bool _lt(const TupleHolderSizedImpl &) const { return false; }

  constexpr bool _le(const TupleHolderSizedImpl &) const { return true; }

  constexpr bool _gt(const TupleHolderSizedImpl &) const { return false; }

  constexpr bool _ge(const TupleHolderSizedImpl &) const { return true; }

  constexpr cmp::Ordering _cmp(const TupleHolderSizedImpl &) const;
};

template <class Field, class... Fields>
struct crust_ebco TupleSizedHolder<Field, Fields...> :
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
struct crust_ebco TupleSizedHolder<Field> :
    TupleHolderSizedImpl<Derive<Field, ZeroSizedType>::result, true, Field> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleSizedHolder,
      TupleHolderSizedImpl<Derive<Field, ZeroSizedType>::result, true, Field>);
};

template <bool is_zst, class... Fields>
struct ZeroSizedTypeHolderImpl;

template <class... Fields>
struct ZeroSizedTypeHolder;

template <class Field, class... Fields>
struct crust_ebco ZeroSizedTypeHolderImpl<true, Field, Fields...> :
    Field,
    ZeroSizedTypeHolder<Fields...> {};

template <class Field>
struct crust_ebco ZeroSizedTypeHolderImpl<true, Field> : Field {};

template <class Field, class... Fields>
struct crust_ebco ZeroSizedTypeHolderImpl<false, Field, Fields...> :
    ZeroSizedTypeHolder<Fields...> {};

template <class Field>
struct ZeroSizedTypeHolderImpl<false, Field> {};

template <class Field, class... Fields>
struct crust_ebco ZeroSizedTypeHolder<Field, Fields...> :
    ZeroSizedTypeHolderImpl<
        AllVal<
            Derive<Field, ZeroSizedType>,
            NotVal<TypesIncludeVal<Field, Fields...>>>::result,
        Field,
        Fields...> {};

template <bool is_all_zst, class... Fields>
struct TupleHolderImpl;

template <class... Fields>
struct crust_ebco TupleHolderImpl<false, Fields...> :
    ZeroSizedTypeHolder<Fields...>,
    TupleSizedHolder<Fields...> {
  constexpr TupleHolderImpl() :
      ZeroSizedTypeHolder<Fields...>{}, TupleSizedHolder<Fields...>{} {}

  constexpr TupleHolderImpl(Fields &&...fields) :
      ZeroSizedTypeHolder<Fields...>{}, TupleSizedHolder<Fields...>{
                                            forward<Fields>(fields)...} {}
};

template <class... Fields>
struct crust_ebco TupleHolderImpl<true, Fields...> :
    ZeroSizedTypeHolder<Fields...> {
  constexpr TupleHolderImpl() : ZeroSizedTypeHolder<Fields...>{} {}

  constexpr TupleHolderImpl(Fields &&...) : ZeroSizedTypeHolder<Fields...>{} {}
};

template <class... Fields>
struct TupleHolder :
    TupleHolderImpl<
        AllVal<Derive<Fields, ZeroSizedType>...>::result,
        Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleHolder,
      TupleHolderImpl<
          AllVal<Derive<Fields, ZeroSizedType>...>::result,
          Fields...>);
};

template <>
struct crust_ebco TupleHolder<> {
  constexpr bool _eq(const TupleHolder &) const { return true; }

  constexpr bool _ne(const TupleHolder &) const { return false; }

  constexpr Option<cmp::Ordering> _partial_cmp(const TupleHolder &) const;

  constexpr bool _lt(const TupleHolder &) const { return false; }

  constexpr bool _le(const TupleHolder &) const { return true; }

  constexpr bool _gt(const TupleHolder &) const { return false; }

  constexpr bool _ge(const TupleHolder &) const { return true; }

  constexpr cmp::Ordering _cmp(const TupleHolder &) const;
};

template <usize index, bool is_zst, class... Fields>
struct TupleGetterImpl;

template <usize index, class... Fields>
struct TupleGetter;

template <usize index, class Field, class... Fields>
struct TupleGetterImpl<index, false, Field, Fields...> {
  using Self = TupleSizedHolder<Field, Fields...>;
  using Result = typename TupleGetter<index - 1, Fields...>::Result;

  static constexpr const Result &inner(const Self &self) {
    return TupleGetter<index - 1, Fields...>::inner(self.remains);
  }

  static constexpr Result &inner(Self &self) {
    return TupleGetter<index - 1, Fields...>::inner(self.remains);
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
struct TupleGetterImpl<index, true, Field, Fields...> {
  using Self = ZeroSizedTypeHolder<Field, Fields...>;
  using Result = typename TypesIndexToType<index, Field, Fields...>::Result;

  static constexpr const Result &inner(const Self &self) { return self; }

  static constexpr Result &inner(Self &self) { return self; }
};

template <usize index, class Field, class... Fields>
struct TupleGetter<index, Field, Fields...> :
    TupleGetterImpl<
        index,
        Derive<
            typename TypesIndexToType<index, Field, Fields...>::Result,
            ZeroSizedType>::result,
        Field,
        Fields...> {};
} // namespace _impl_tuple

template <class... Fields>
class crust_ebco TupleStruct : public _impl_tuple::TupleHolder<Fields...> {
private:
  crust_static_assert(AllVal<NotVal<IsConstOrRefVal<Fields>>...>::result);

  template <usize index>
  using Getter = _impl_tuple::TupleGetter<index, Fields...>;

  template <usize index>
  using Result = typename Getter<index>::Result;

  static constexpr usize size = sizeof...(Fields);

protected:
  CRUST_USE_BASE_CONSTRUCTORS(TupleStruct, _impl_tuple::TupleHolder<Fields...>);

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
template <class Self, class... Fields>
struct crust_ebco AutoImpl<Self, TupleStruct<Fields...>, ZeroSizedType> :
    Impl<ZeroSizedType<Self>, Derive<Fields, ZeroSizedType>...> {};

template <class Self>
struct crust_ebco TuplePartialEqImpl : ::crust::cmp::PartialEq<Self> {
  CRUST_TRAIT_REQUIRE(TuplePartialEqImpl);

  constexpr bool eq(const Self &other) const { return self()._eq(other); }

  constexpr bool ne(const Self &other) const { return self()._ne(other); }
};

template <class Self, class... Fields>
struct crust_ebco
    AutoImpl<Self, TupleStruct<Fields...>, ::crust::cmp::PartialEq> :
    Impl<TuplePartialEqImpl<Self>, Derive<Fields, ::crust::cmp::PartialEq>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
};

template <class Self, class... Fields>
struct crust_ebco AutoImpl<Self, TupleStruct<Fields...>, ::crust::cmp::Eq> :
    Impl<::crust::cmp::Eq<Self>, Derive<Fields, ::crust::cmp::Eq>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
};

template <class Self>
struct crust_ebco TuplePartialOrdImpl : ::crust::cmp::PartialOrd<Self> {
  CRUST_TRAIT_REQUIRE(TuplePartialOrdImpl);

  constexpr Option<::crust::cmp::Ordering> partial_cmp(const Self &other) const;

  constexpr bool lt(const Self &other) const { return self()._lt(other); }

  constexpr bool le(const Self &other) const { return self()._le(other); }

  constexpr bool gt(const Self &other) const { return self()._gt(other); }

  constexpr bool ge(const Self &other) const { return self()._ge(other); }
};

template <class Self, class... Fields>
struct crust_ebco
    AutoImpl<Self, TupleStruct<Fields...>, ::crust::cmp::PartialOrd> :
    Impl<
        TuplePartialOrdImpl<Self>,
        Derive<Fields, ::crust::cmp::PartialOrd>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
};

template <class Self>
struct crust_ebco TupleOrdImpl : ::crust::cmp::Ord<Self> {
  CRUST_TRAIT_REQUIRE(TupleOrdImpl);

  constexpr ::crust::cmp::Ordering cmp(const Self &other) const;

  // todo: implement other
};

template <class Self, class... Fields>
struct crust_ebco AutoImpl<Self, TupleStruct<Fields...>, ::crust::cmp::Ord> :
    Impl<TupleOrdImpl<Self>, Derive<Fields, ::crust::cmp::Ord>...> {
protected:
  constexpr AutoImpl() {
    crust_static_assert(IsBaseOfTypeVal<TupleStruct<Fields...>, Self>::result);
  }
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

// namespace _impl_tuple {
// template <usize index, class... Fields>
// struct TupleEqHelper {
//   static constexpr bool inner(
//       TupleSizedHolder<const Fields &...> ref, const Tuple<Fields...> &tuple)
//       {
//     return TupleEqHelper<index - 1, Fields...>::inner(ref, tuple) &&
//         TupleGetter<index - 1, const Fields &...>::inner(ref) ==
//         tuple.template get<index - 1>();
//   }
// };

// template <class... Fields>
// struct TupleEqHelper<0, Fields...> {
//   static crust_cxx14_constexpr bool
//   inner(TupleSizedHolder<const Fields &...>, const Tuple<Fields...> &) {
//     return true;
//   }
// };

// template <usize index, class... Fields>
// struct LetTupleHelper {
//   static crust_cxx14_constexpr void
//   inner(TupleSizedHolder<Fields &...> &ref, Tuple<Fields...> &&tuple) {
//     TupleGetter<index - 1, Fields &...>::inner(ref) =
//         move(tuple.template get<index - 1>());
//     LetTupleHelper<index - 1, Fields...>::inner(ref, move(tuple));
//   }
// };

// template <class... Fields>
// struct LetTupleHelper<0, Fields...> {
//   static crust_cxx14_constexpr void
//   inner(TupleSizedHolder<Fields &...> &, Tuple<Fields...> &&) {}
// };
// } // namespace _impl_tuple
} // namespace crust


#endif // CRUST_TUPLE_DECL_HPP

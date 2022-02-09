#ifndef _CRUST_INCLUDE_TUPLE_DECL_HPP
#define _CRUST_INCLUDE_TUPLE_DECL_HPP


#include "cmp_decl.hpp"
#include "utility.hpp"


namespace crust {
namespace option {
template <class T>
class Option;
} // namespace option

using option::Option;

namespace _impl_tuple {
template <bool is_zst, bool remain_is_zst, class... Fields>
struct TupleHolderImpl;

template <class... Fields>
struct TupleHolder;

template <class Field, class... Fields>
struct TupleHolderImpl<false, false, Field, Fields...> {
  Field field;
  TupleHolder<Fields...> remains;

  constexpr TupleHolderImpl() : field{}, remains{} {}

  template <class T, class... Ts>
  explicit constexpr TupleHolderImpl(T &&field, Ts &&...fields) :
      field{forward<T>(field)}, remains{forward<Ts>(fields)...} {}

  constexpr bool eq(const TupleHolderImpl &other) const {
    return field == other.field && remains.eq(other.remains);
  }

  constexpr bool ne(const TupleHolderImpl &other) const {
    return field != other.field || remains.ne(other.remains);
  }

  constexpr Option<cmp::Ordering>
  partial_cmp(const TupleHolderImpl &other) const;

  constexpr bool lt(const TupleHolderImpl &other) const {
    return field != other.field ? field < other.field :
                                  remains.lt(other.remains);
  }

  constexpr bool le(const TupleHolderImpl &other) const {
    return field != other.field ? field <= other.field :
                                  remains.le(other.remains);
  }

  constexpr bool gt(const TupleHolderImpl &other) const {
    return field != other.field ? field > other.field :
                                  remains.gt(other.remains);
  }

  constexpr bool ge(const TupleHolderImpl &other) const {
    return field != other.field ? field >= other.field :
                                  remains.ge(other.remains);
  }

  constexpr cmp::Ordering cmp(const TupleHolderImpl &other) const;
};

template <class Field, class... Fields>
struct TupleHolderImpl<false, true, Field, Fields...> {
  Field field;

  constexpr bool eq(const TupleHolderImpl &other) const {
    return field == other.field;
  }

  constexpr bool ne(const TupleHolderImpl &other) const {
    return field != other.field;
  }

  constexpr Option<cmp::Ordering>
  partial_cmp(const TupleHolderImpl &other) const;

  constexpr bool lt(const TupleHolderImpl &other) const {
    return field < other.field;
  }

  constexpr bool le(const TupleHolderImpl &other) const {
    return field <= other.field;
  }

  constexpr bool gt(const TupleHolderImpl &other) const {
    return field > other.field;
  }

  constexpr bool ge(const TupleHolderImpl &other) const {
    return field >= other.field;
  }

  constexpr cmp::Ordering cmp(const TupleHolderImpl &other) const;
};


template <class Field, class... Fields>
struct crust_ebco TupleHolderImpl<true, false, Field, Fields...> :
    TupleHolder<Fields...> {};

template <class... Fields>
struct TupleHolderImpl<true, true, Fields...> {
  constexpr bool eq(const TupleHolderImpl &) const { return true; }

  constexpr bool ne(const TupleHolderImpl &) const { return false; }

  constexpr Option<cmp::Ordering> partial_cmp(const TupleHolderImpl &) const;

  constexpr bool lt(const TupleHolderImpl &) const { return false; }

  constexpr bool le(const TupleHolderImpl &) const { return true; }

  constexpr bool gt(const TupleHolderImpl &) const { return false; }

  constexpr bool ge(const TupleHolderImpl &) const { return true; }

  constexpr cmp::Ordering cmp(const TupleHolderImpl &) const;
};

template <class Field, class... Fields>
struct crust_ebco TupleHolder<Field, Fields...> :
    TupleHolderImpl<
        IsZeroSizedType<Field>::result,
        AllType<IsZeroSizedType<Fields>...>::result,
        Field,
        Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleHolder,
      TupleHolderImpl<
          IsZeroSizedType<Field>::result,
          AllType<IsZeroSizedType<Fields>...>::result,
          Field,
          Fields...>);
};

template <class Field>
struct crust_ebco TupleHolder<Field> :
    TupleHolderImpl<IsZeroSizedType<Field>::result, true, Field> {
  CRUST_USE_BASE_CONSTRUCTORS(
      TupleHolder,
      TupleHolderImpl<IsZeroSizedType<Field>::result, true, Field>);
};

template <>
struct TupleHolder<> : TupleHolderImpl<true, true> {};

template <usize index, class... Fields>
struct TupleGetter;

template <usize index, class Field, class... Fields>
struct TupleGetter<index, Field, Fields...> {
  using Self = TupleHolder<Field, Fields...>;
  using Result = typename TupleGetter<index - 1, Fields...>::Result;

  static constexpr const Result &inner(const Self &self) {
    return TupleGetter<index - 1, Fields...>::inner(self.remains);
  }

  static constexpr Result &inner(Self &self) {
    return TupleGetter<index - 1, Fields...>::inner(self.remains);
  }
};

template <class Field, class... Fields>
struct TupleGetter<0, Field, Fields...> {
  using Self = TupleHolder<Field, Fields...>;
  using Result = Field;

  static constexpr const Result &inner(const Self &self) { return self.field; }

  static constexpr Result &inner(Self &self) { return self.field; }
};
} // namespace _impl_tuple

template <class... Fields>
class crust_ebco Tuple :
    public Impl<
        cmp::PartialEq<Tuple<Fields...>>,
        Derive<Fields, cmp::PartialEq>...>,
    public Impl<cmp::Eq<Tuple<Fields...>>, Derive<Fields, cmp::Eq>...>,
    public Impl<
        cmp::PartialOrd<Tuple<Fields...>>,
        Derive<Fields, cmp::PartialOrd>...>,
    public Impl<cmp::Ord<Tuple<Fields...>>, Derive<Fields, cmp::Ord>...>,
    public Impl<ZeroSizedType, IsZeroSizedType<Fields>...> {
private:
  crust_static_assert(AllType<NotType<IsConstOrRef<Fields>>...>::result);

  using Holder =
      _impl_tuple::TupleHolder<typename RemoveRef<Fields>::Result...>;

  template <usize index>
  using Getter = _impl_tuple::TupleGetter<index, Fields...>;

  template <usize index>
  using Result = typename Getter<index>::Result;

  static constexpr usize size = sizeof...(Fields);

  Holder holder;

public:
  constexpr Tuple() : holder{} {}

  template <class... Ts>
  explicit constexpr Tuple(Ts &&...fields) : holder{forward<Ts>(fields)...} {}

  template <usize index>
  constexpr const Result<index> &get() const {
    return Getter<index>::inner(holder);
  }

  template <usize index>
  crust_cxx14_constexpr Result<index> &get() {
    return Getter<index>::inner(holder);
  }

  /// impl PartialEq

  constexpr bool eq(const Tuple &other) const {
    return holder.eq(other.holder);
  }

  constexpr bool ne(const Tuple &other) const {
    return holder.ne(other.holder);
  }

  /// impl PartialOrd

  constexpr Option<cmp::Ordering> partial_cmp(const Tuple &other) const;

  constexpr bool lt(const Tuple &other) const {
    return holder.lt(other.holder);
  }

  constexpr bool le(const Tuple &other) const {
    return holder.le(other.holder);
  }

  constexpr bool gt(const Tuple &other) const {
    return holder.gt(other.holder);
  }

  constexpr bool ge(const Tuple &other) const {
    return holder.ge(other.holder);
  }

  /// impl Ord

  constexpr cmp::Ordering cmp(const Tuple &other) const;
};

namespace _impl_tuple {
template <usize index, class... Fields>
struct TupleEqHelper {
  static constexpr bool
  inner(TupleHolder<const Fields &...> ref, const Tuple<Fields...> &tuple) {
    return TupleEqHelper<index - 1, Fields...>::inner(ref, tuple) &&
        TupleGetter<index - 1, const Fields &...>::inner(ref) ==
        tuple.template get<index - 1>();
  }
};

template <class... Fields>
struct TupleEqHelper<0, Fields...> {
  static crust_cxx14_constexpr bool
  inner(TupleHolder<const Fields &...>, const Tuple<Fields...> &) {
    return true;
  }
};


template <usize index, class... Fields>
struct LetTupleHelper {
  static crust_cxx14_constexpr void
  inner(TupleHolder<Fields &...> &ref, Tuple<Fields...> &&tuple) {
    TupleGetter<index - 1, Fields &...>::inner(ref) =
        move(tuple.template get<index - 1>());
    LetTupleHelper<index - 1, Fields...>::inner(ref, move(tuple));
  }
};

template <class... Fields>
struct LetTupleHelper<0, Fields...> {
  static crust_cxx14_constexpr void
  inner(TupleHolder<Fields &...> &, Tuple<Fields...> &&) {}
};
} // namespace _impl_tuple
} // namespace crust


#endif //_CRUST_INCLUDE_TUPLE_DECL_HPP

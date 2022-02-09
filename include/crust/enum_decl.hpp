#ifndef CRUST_ENUM_DECL_HPP
#define CRUST_ENUM_DECL_HPP


#include <limits>
#include <new>

#include "crust/cmp_decl.hpp"
#include "crust/repeat_macro.hpp"
#include "crust/tuple_decl.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace option {
template <class T>
class Option;
} // namespace option

using option::Option;

namespace _impl_enum {
template <class T, class... Fields>
struct EnumInclude;

template <class T, class Field, class... Fields>
struct EnumInclude<T, Field, Fields...> : EnumInclude<T, Fields...> {};

template <class T, class... Fields>
struct EnumInclude<T, T, Fields...> : BoolVal<true> {};

template <class T>
struct EnumInclude<T> : BoolVal<false> {};

template <class... Fields>
struct EnumDuplicate;

template <class Field, class... Fields>
struct EnumDuplicate<Field, Fields...> :
    AnyVal<EnumInclude<Field, Fields...>, EnumDuplicate<Fields...>> {};

template <>
struct EnumDuplicate<> : BoolVal<false> {};

template <usize index, class... Fields>
struct EnumIndexToType;

template <usize index, class Field, class... Fields>
struct EnumIndexToType<index, Field, Fields...> :
    EnumIndexToType<index - 1, Fields...> {};

template <class Field, class... Fields>
struct EnumIndexToType<0, Field, Fields...> : TmplType<Field> {};

template <class T, class... Fields>
struct EnumTypeToIndex;

template <class T, class Field, class... Fields>
struct EnumTypeToIndex<T, Field, Fields...> :
    IncVal<EnumTypeToIndex<T, Fields...>> {};

template <class T, class... Fields>
struct EnumTypeToIndex<T, T, Fields...> : TmplVal<u32, 0> {};

template <bool trivial, class... Fields>
union EnumHolder;

template <class Field, class... Fields>
union EnumHolder<true, Field, Fields...> {
  Field field;
  EnumHolder<true, Fields...> remains;

  constexpr EnumHolder() : remains{} {}

  explicit constexpr EnumHolder(Field &&field) : field{move(field)} {}

  explicit constexpr EnumHolder(const Field &field) : field{field} {}

  template <class T>
  explicit constexpr EnumHolder(T &&field) : remains{forward<T>(field)} {}
};

template <class Field, class... Fields>
union EnumHolder<false, Field, Fields...> {
  Field field;
  EnumHolder<false, Fields...> remains;

  constexpr EnumHolder() : remains{} {}

  explicit constexpr EnumHolder(Field &&field) : field{move(field)} {}

  explicit constexpr EnumHolder(const Field &field) : field{field} {}

  template <class T>
  explicit constexpr EnumHolder(T &&field) : remains{forward<T>(field)} {}

  EnumHolder(const EnumHolder &) {}

  EnumHolder(EnumHolder &&) noexcept {}

  EnumHolder &operator=(const EnumHolder &) { return *this; }

  EnumHolder &operator=(EnumHolder &&) noexcept { return *this; }

  ~EnumHolder() {}
};

template <bool trivial>
union EnumHolder<trivial> {};

template <usize index, bool trivial, class... Fields>
struct EnumGetter;

template <usize index, bool trivial, class Field, class... Fields>
struct EnumGetter<index, trivial, Field, Fields...> {
  using Self = EnumHolder<trivial, Field, Fields...>;
  using Remain = EnumGetter<index - 1, trivial, Fields...>;
  using Result = typename Remain::Result;

  static constexpr const Result &inner(const Self &self) {
    return Remain::inner(self.remains);
  }

  static constexpr Result &inner(Self &self) {
    return Remain::inner(self.remains);
  }
};

template <bool trivial, class Field, class... Fields>
struct EnumGetter<0, trivial, Field, Fields...> {
  using Self = EnumHolder<trivial, Field, Fields...>;
  using Result = Field;

  static constexpr const Result &inner(const Self &self) { return self.field; }

  static constexpr Result &inner(Self &self) { return self.field; }
};

template <usize offset, usize size, bool trivial, class... Fields>
struct EnumVisitor {
  static constexpr usize cut = size / 2;

  using LowerGetter = EnumVisitor<offset, cut, trivial, Fields...>;
  using UpperGetter = EnumVisitor<offset + cut, size - cut, trivial, Fields...>;

  template <class R, class V>
  static constexpr R
  inner(const EnumHolder<trivial, Fields...> &self, V &&impl, usize index) {
    return index < cut ?
        LowerGetter::template inner<R, V>(self, forward<V>(impl), index) :
        UpperGetter::template inner<R, V>(self, forward<V>(impl), index);
  }

  template <class R, class V>
  static constexpr R
  inner(EnumHolder<trivial, Fields...> &self, V &&impl, usize index) {
    return index < cut ?
        LowerGetter::template inner<R, V>(self, forward<V>(impl), index) :
        UpperGetter::template inner<R, V>(self, forward<V>(impl), index);
  }
};

#define _ENUM_VISITOR_BRANCH(index)                                            \
  case offset + index:                                                         \
    return impl(EnumGetter<offset + index, trivial, Fields...>::inner(self))

#define _ENUM_VISITOR_IMPL(len)                                                \
  template <usize offset, bool trivial, class... Fields>                       \
  struct EnumVisitor<offset, len, trivial, Fields...> {                        \
    template <class R, class V>                                                \
    static crust_cxx14_constexpr R                                             \
    inner(const EnumHolder<trivial, Fields...> &self, V &&impl, usize index) { \
      switch (index) {                                                         \
      default:                                                                 \
        crust_unreachable();                                                   \
        CRUST_MACRO_REPEAT(len, _ENUM_VISITOR_BRANCH);                         \
      }                                                                        \
    }                                                                          \
    template <class R, class V>                                                \
    static crust_cxx14_constexpr R                                             \
    inner(EnumHolder<trivial, Fields...> &self, V &&impl, usize index) {       \
      switch (index) {                                                         \
      default:                                                                 \
        crust_unreachable();                                                   \
        CRUST_MACRO_REPEAT(len, _ENUM_VISITOR_BRANCH);                         \
      }                                                                        \
    }                                                                          \
  }

_ENUM_VISITOR_IMPL(16);
_ENUM_VISITOR_IMPL(15);
_ENUM_VISITOR_IMPL(14);
_ENUM_VISITOR_IMPL(13);
_ENUM_VISITOR_IMPL(12);
_ENUM_VISITOR_IMPL(11);
_ENUM_VISITOR_IMPL(10);
_ENUM_VISITOR_IMPL(9);
_ENUM_VISITOR_IMPL(8);
_ENUM_VISITOR_IMPL(7);
_ENUM_VISITOR_IMPL(6);
_ENUM_VISITOR_IMPL(5);
_ENUM_VISITOR_IMPL(4);
_ENUM_VISITOR_IMPL(3);
_ENUM_VISITOR_IMPL(2);
_ENUM_VISITOR_IMPL(1);

#undef _ENUM_VISITOR_IMPL
#undef _ENUM_VISITOR_BRANCH


template <usize offset, usize size, class... Fields>
struct TagVisitor {
  static constexpr usize cut = size / 2;

  using LowerGetter = TagVisitor<offset, cut, Fields...>;
  using UpperGetter = TagVisitor<offset + cut, size - cut, Fields...>;

  template <class R, class V>
  static constexpr R inner(V &&impl, usize index) {
    return index < cut ?
        LowerGetter::template inner<R, V>(forward<V>(impl), index) :
        UpperGetter::template inner<R, V>(forward<V>(impl), index);
  }
};

#define _TAG_VISITOR_BRANCH(index)                                             \
  case offset + index:                                                         \
    return impl(typename EnumIndexToType<offset + index, Fields...>::Result{});

#define _TAG_VISITOR_IMPL(len)                                                 \
  template <usize offset, class... Fields>                                     \
  struct TagVisitor<offset, len, Fields...> {                                  \
    template <class R, class V>                                                \
    static crust_cxx14_constexpr R inner(V &&impl, usize index) {              \
      switch (index) {                                                         \
      default:                                                                 \
        crust_unreachable();                                                   \
        CRUST_MACRO_REPEAT(len, _TAG_VISITOR_BRANCH);                          \
      }                                                                        \
    }                                                                          \
  }

_TAG_VISITOR_IMPL(16);
_TAG_VISITOR_IMPL(15);
_TAG_VISITOR_IMPL(14);
_TAG_VISITOR_IMPL(13);
_TAG_VISITOR_IMPL(12);
_TAG_VISITOR_IMPL(11);
_TAG_VISITOR_IMPL(10);
_TAG_VISITOR_IMPL(9);
_TAG_VISITOR_IMPL(8);
_TAG_VISITOR_IMPL(7);
_TAG_VISITOR_IMPL(6);
_TAG_VISITOR_IMPL(5);
_TAG_VISITOR_IMPL(4);
_TAG_VISITOR_IMPL(3);
_TAG_VISITOR_IMPL(2);
_TAG_VISITOR_IMPL(1);

#undef _TAG_VISITOR_IMPL
#undef _TAG_VISITOR_BRANCH

template <class Self, bool flag>
struct EnumTrivial;

template <class Self>
struct EnumTrivial<Self, true> {
  void drop() {}
};

template <class Self>
struct EnumTrivial<Self, false> {
  constexpr EnumTrivial() {}

  constexpr const Self &self() const {
    return *static_cast<const Self *>(this);
  }

  crust_cxx14_constexpr Self &self() { return *static_cast<Self *>(this); };

  void drop() {
    if (self().index != 0) {
      self().visit(typename Self::Drop{});
    }
  }

  EnumTrivial(const EnumTrivial &other) {
    if (this != &other) {
      other.self().visit(typename Self::Copy{&self().holder});
    }
  }

  EnumTrivial(EnumTrivial &&other) noexcept {
    if (this != &other) {
      other.self().visit(typename Self::Emplace{&self().holder});
    }
  }

  EnumTrivial &operator=(const EnumTrivial &other) {
    if (this != &other) {
      drop();
      other.self().visit(typename Self::Copy{&self().holder});
    }

    return *this;
  }

  EnumTrivial &operator=(EnumTrivial &&other) noexcept {
    if (this != &other) {
      drop();
      other.self().visit(typename Self::Emplace{&self().holder});
    }

    return *this;
  }

  ~EnumTrivial() { drop(); }
};

template <class... Fields>
struct EnumIsTrivial : AllVal<IsTriviallyCopyableVal<Fields>...> {};

template <class... Fields>
struct EnumIsTagOnly : AllVal<IsZeroSizedTypeVal<Fields>...> {};

template <class... Fields>
struct EnumTagUnion :
    EnumTrivial<EnumTagUnion<Fields...>, EnumIsTrivial<Fields...>::result> {
  static constexpr bool trivial = EnumIsTrivial<Fields...>::result;

  friend EnumTrivial<EnumTagUnion, trivial>;

  using Holder =
      EnumHolder<trivial, typename RemoveConstOrRefType<Fields>::Result...>;
  template <class T>
  using IndexGetter = EnumTypeToIndex<T, Fields...>;
  using Getter = EnumVisitor<0, sizeof...(Fields), trivial, Fields...>;

  Holder holder;
  u32 index;

  constexpr EnumTagUnion() : holder{}, index{0} {}

  template <class T>
  explicit constexpr EnumTagUnion(T &&value) :
      holder{forward<T>(value)},
      index{IndexGetter<typename RemoveConstOrRefType<T>::Result>::result + 1} {
  }

  struct Drop {
    template <class T>
    crust_cxx14_constexpr void operator()(T &value) const {
      value.~T();
    }
  };

  struct Copy {
    Holder *holder;

    template <class T>
    void operator()(const T &value) {
      ::new (holder) Holder{value};
    }
  };

  struct Emplace {
    Holder *holder;

    template <class T>
    void operator()(T &value) {
      ::new (holder) Holder{move(value)};
    }
  };

  template <class T>
  EnumTagUnion &operator=(T &&value) {
    this->drop();

    ::new (&holder) Holder{forward<T>(value)};
    index = IndexGetter<typename RemoveConstOrRefType<T>::Result>::result + 1;

    return *this;
  }

  template <class R = void, class V>
  constexpr R visit(V &&visitor) const {
    return crust_debug_assert(index - 1 < sizeof...(Fields)),
           Getter::template inner<R, V>(holder, forward<V>(visitor), index - 1);
  }

  template <class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) {
    return crust_debug_assert(index - 1 < sizeof...(Fields)),
           Getter::template inner<R, V>(holder, forward<V>(visitor), index - 1);
  }

  template <class T, class V>
  constexpr bool visit_variant(V &&visitor) const {
    using Index = IndexGetter<typename RemoveConstOrRefType<T>::Result>;
    return index == Index::result + 1 ?
        visitor(EnumGetter<Index::result, trivial, Fields...>::inner(holder)) :
        visitor();
  }

  template <class T>
  constexpr bool is_variant() const {
    return index ==
        IndexGetter<typename RemoveConstOrRefType<T>::Result>::result + 1;
  }

  template <class T, class... Fs>
  constexpr bool eq_variant(const Fs &...other) const {
    using Index = IndexGetter<typename RemoveConstOrRefType<T>::Result>;
    return index == Index::result + 1 &&
        _impl_tuple::TupleEqHelper<sizeof...(Fs), Fs...>::inner(
               _impl_tuple::TupleHolder<const Fs &...>{other...},
               EnumGetter<Index::result, trivial, Fields...>::inner(holder));
  }

  template <class T>
  crust_cxx14_constexpr Option<T> move_variant();

  template <class T, class... Fs>
  crust_cxx14_constexpr bool let_helper(_impl_tuple::TupleHolder<Fs &...> ref) {
    if (is_variant<T>()) {
      _impl_tuple::LetTupleHelper<sizeof...(Fs), Fs...>::inner(
          ref,
          move(EnumGetter<
               IndexGetter<typename RemoveConstOrRefType<T>::Result>::result,
               trivial,
               Fields...>::inner(holder)));
      return true;
    } else {
      return false;
    }
  }

  /// impl PartialEq

  struct Equal {
    const Holder *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value ==
          EnumGetter<
                 IndexGetter<typename RemoveConstOrRefType<T>::Result>::result,
                 trivial,
                 Fields...>::inner(*other);
    };
  };

  constexpr bool eq(const EnumTagUnion &other) const {
    return index == other.index && visit<bool>(Equal{&other.holder});
  }

  struct NotEqual {
    const Holder *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value !=
          EnumGetter<
                 IndexGetter<typename RemoveConstOrRefType<T>::Result>::result,
                 trivial,
                 Fields...>::inner(*other);
    };
  };

  constexpr bool ne(const EnumTagUnion &other) const {
    return index != other.index || visit<bool>(NotEqual{&other.holder});
  }
};

// todo: allow assigning number
template <class... Fields>
struct EnumTagOnly {
  template <class T>
  using IndexGetter = EnumTypeToIndex<T, Fields...>;
  using Getter = TagVisitor<0, sizeof...(Fields), Fields...>;

  u32 index;

  constexpr EnumTagOnly() : index{0} {}

  template <class T>
  explicit constexpr EnumTagOnly(T &&) :
      index{IndexGetter<typename RemoveConstOrRefType<T>::Result>::result + 1} {
  }

  template <class T>
  EnumTagOnly &operator=(T &&) {
    index = IndexGetter<typename RemoveConstOrRefType<T>::Result>::result + 1;

    return *this;
  }

  template <class R = void, class V>
  constexpr R visit(V &&visitor) const {
    return crust_debug_assert(index - 1 < sizeof...(Fields)),
           Getter::template inner<R, V>(forward<V>(visitor), index - 1);
  }

  template <class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) {
    return crust_debug_assert(index - 1 < sizeof...(Fields)),
           Getter::template inner<R, V>(forward<V>(visitor), index - 1);
  }

  template <class T>
  constexpr bool is_variant() const {
    return index ==
        IndexGetter<typename RemoveConstOrRefType<T>::Result>::result + 1;
  }

  template <class T>
  constexpr bool eq_variant() const {
    return is_variant<T>();
  }

  template <class T>
  crust_cxx14_constexpr Option<T> move_variant();

  template <class T, class... Fs>
  crust_cxx14_constexpr bool let_helper(_impl_tuple::TupleHolder<Fs &...> ref) {
    if (is_variant<T>()) {
      _impl_tuple::LetTupleHelper<sizeof...(Fs), Fs...>(ref, T{});
      return true;
    } else {
      return false;
    }
  }

  constexpr bool eq(const EnumTagOnly &other) const {
    return index == other.index;
  }
};

template <bool is_tag_only, class... Fields>
struct EnumSelect;

template <class... Fields>
struct EnumSelect<false, Fields...> : EnumTagUnion<Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelect, EnumTagUnion<Fields...>);
};

template <class... Fields>
struct EnumSelect<true, Fields...> : EnumTagOnly<Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelect, EnumTagOnly<Fields...>);
};

template <class T, class... Fields>
struct LetEnum;

template <class... Ts>
struct Overloaded;

template <class T, class... Ts>
struct crust_ebco Overloaded<T, Ts...> : T, Overloaded<Ts...> {
  using T::operator();
  using Overloaded<Ts...>::operator();

  explicit constexpr Overloaded(T t, Ts... ts) :
      T{forward<T>(t)}, Overloaded<Ts...>{forward<Ts>(ts)...} {}
};

template <class T>
struct crust_ebco Overloaded<T> : T {
  using T::operator();

  explicit constexpr Overloaded(T t) : T{forward<T>(t)} {}
};

template <class... Ts>
constexpr Overloaded<Ts...> overloaded(Ts &&...ts) {
  return Overloaded<Ts...>{forward<Ts>(ts)...};
}
} // namespace _impl_enum

// todo: implement PartialOrd and Ord
template <class... Fields>
class crust_ebco Enum :
    public Impl<
        cmp::PartialEq<Enum<Fields...>>,
        AllVal<Derive<Fields, cmp::PartialEq>...>>,
    public Impl<cmp::Eq<Tuple<Fields...>>, AllVal<Derive<Fields, cmp::Eq>...>> {
private:
  crust_static_assert(sizeof...(Fields) < std::numeric_limits<u32>::max());
  crust_static_assert(sizeof...(Fields) > 0);
  crust_static_assert(NotVal<_impl_enum::EnumDuplicate<Fields...>>::result);
  crust_static_assert(AllVal<NotVal<IsConstOrRefVal<Fields>>...>::result);

  using Inner = _impl_enum::
      EnumSelect<_impl_enum::EnumIsTagOnly<Fields...>::result, Fields...>;

  Inner inner;

  template <class, class...>
  friend struct _impl_enum::LetEnum;

protected: // todo: optimize for constexpr
  template <class T>
  constexpr bool is_variant() const {
    return inner.template is_variant<T>();
  }

  template <class T, class... Fs>
  constexpr bool eq_variant(const Fs &...other) const {
    return inner.template eq_variant<T>(other...);
  }

  template <class T>
  crust_cxx14_constexpr Option<T> move_variant();

public:
  constexpr Enum() : inner{} {}

  template <
      class T,
      typename EnableIf<
          !IsSameVal<typename RemoveConstOrRefType<T>::Result, Enum>::result>::
          Result * = nullptr>
  constexpr Enum(T &&value) : inner{forward<T>(value)} {}

  template <
      class T,
      typename EnableIf<
          !IsSameVal<typename RemoveConstOrRefType<T>::Result, Enum>::result>::
          Result * = nullptr>
  Enum &operator=(T &&value) {
    inner = Inner{forward<T>(value)};
    return *this;
  }

  template <class R = void, class... Ts>
  constexpr R visit(Ts &&...ts) const {
    return inner.template visit<R>(_impl_enum::overloaded(forward<Ts>(ts)...));
  }

  template <class R = void, class... Ts>
  crust_cxx14_constexpr R visit(Ts &&...ts) {
    return inner.template visit<R>(_impl_enum::overloaded(forward<Ts>(ts)...));
  }

  constexpr bool eq(const Enum &other) const { return inner.eq(other.inner); }

  constexpr bool ne(const Enum &other) const { return inner.ne(other.inner); }
};

#define CRUST_ENUM_USE_BASE(NAME, ...)                                         \
  template <class... Args>                                                     \
  constexpr NAME(Args &&...args) :                                             \
      __VA_ARGS__{::crust::forward<Args>(args)...} {}                          \
  template <class = void>                                                      \
  static void _detect_trait_partial_eq(const NAME &) {                         \
    crust_static_assert(                                                       \
        ::crust::Derive<__VA_ARGS__, ::crust::cmp::PartialEq>::result);        \
  }                                                                            \
  template <class = void>                                                      \
  static void _detect_trait_eq() {                                             \
    crust_static_assert(                                                       \
        ::crust::Derive<__VA_ARGS__, ::crust::cmp::Eq>::result);               \
  }
} // namespace crust


#endif // CRUST_ENUM_DECL_HPP

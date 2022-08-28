#ifndef CRUST_ENUM_DECL_HPP
#define CRUST_ENUM_DECL_HPP


#include <new>

#include "crust/clone.hpp"
#include "crust/cmp_decl.hpp"
#include "crust/helper/repeat_macro.hpp"
#include "crust/helper/types.hpp"
#include "crust/num/mod.hpp"
#include "crust/tuple_decl.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_enum {
#define CRUST_ENUM_VARIANT(NAME)                                               \
  struct crust_ebco NAME :                                                     \
      ::crust::TupleStruct<>,                                                  \
      ::crust::Derive<                                                         \
          NAME,                                                                \
          ::crust::Trait<::crust::ZeroSizedType>,                              \
          ::crust::Trait<::crust::clone::Clone>,                               \
          ::crust::Trait<::crust::cmp::PartialEq>,                             \
          ::crust::Trait<::crust::cmp::Eq>,                                    \
          ::crust::Trait<::crust::cmp::PartialOrd>,                            \
          ::crust::Trait<::crust::cmp::Ord>> {                                 \
    CRUST_USE_BASE_CONSTRUCTORS(NAME, ::crust::TupleStruct<>);                 \
  }

CRUST_TRAIT(DiscriminantVariant) { CRUST_TRAIT_USE_SELF(DiscriminantVariant); };

#define CRUST_DISCRIMINANT_VARIANT(NAME, VALUE)                                \
  struct crust_ebco NAME :                                                     \
      ::crust::TupleStruct<>,                                                  \
      ::crust::_impl_enum::DiscriminantVariant<NAME>,                          \
      ::crust::Derive<                                                         \
          NAME,                                                                \
          ::crust::Trait<::crust::ZeroSizedType>,                              \
          ::crust::Trait<::crust::clone::Clone>,                               \
          ::crust::Trait<::crust::cmp::PartialEq>,                             \
          ::crust::Trait<::crust::cmp::Eq>,                                    \
          ::crust::Trait<::crust::cmp::PartialOrd>,                            \
          ::crust::Trait<::crust::cmp::Ord>> {                                 \
    static constexpr ::crust::isize result = VALUE;                            \
    CRUST_USE_BASE_CONSTRUCTORS(NAME, ::crust::TupleStruct<>);                 \
  }

template <class I, bool is_spec, class T, class... Fields>
struct DiscriminantImpl;

template <class I, class T, class... Fields>
using Discriminant =
    DiscriminantImpl<I, Require<T, DiscriminantVariant>::result, T, Fields...>;

template <class I, class T, class... Fields>
struct DiscriminantImpl<I, false, T, Fields...> :
    IncVal<Discriminant<
        I,
        typename _impl_types::TypesPrevType<T, Fields...>::Result,
        Fields...>> {};

template <class I, class T, class... Fields>
struct DiscriminantImpl<I, false, T, T, Fields...> : TmplVal<I, 0> {};
// FIXME: use the real type
template <class I, class T, class... Fields>
struct DiscriminantImpl<I, true, T, Fields...> :
    TmplVal<I, static_cast<I>(T::result)> {
  crust_static_assert(T::result <= static_cast<isize>(num::Int<I>::MAX));
  crust_static_assert(T::result >= static_cast<isize>(num::Int<I>::MIN));
};

template <class I, isize index, class... Fields>
struct IndexToDiscriminant :
    Discriminant<
        I,
        typename _impl_types::TypesIndex<index, _impl_types::Types<Fields...>>::
            Result,
        Fields...> {};

template <class I, isize index, class... Fields>
struct CheckDiscriminant :
    All<LTVal<
            IndexToDiscriminant<I, index - 1, Fields...>,
            IndexToDiscriminant<I, index, Fields...>>,
        CheckDiscriminant<I, index - 1, Fields...>> {};

template <class I, class... Fields>
struct CheckDiscriminant<I, 0, Fields...> : BoolVal<true> {};

#define CRUST_ENUM_TUPLE_VARIANT(NAME, FULL_NAME, ...)                         \
  struct crust_ebco NAME :                                                     \
      ::crust::TupleStruct<__VA_ARGS__>,                                       \
      ::crust::Derive<                                                         \
          FULL_NAME,                                                           \
          ::crust::Trait<::crust::ZeroSizedType>,                              \
          ::crust::Trait<::crust::clone::Clone>,                               \
          ::crust::Trait<::crust::cmp::PartialEq>,                             \
          ::crust::Trait<::crust::cmp::Eq>,                                    \
          ::crust::Trait<::crust::cmp::PartialOrd>,                            \
          ::crust::Trait<::crust::cmp::Ord>> {                                 \
    CRUST_USE_BASE_CONSTRUCTORS(NAME, ::crust::TupleStruct<__VA_ARGS__>);      \
  }

#define CRUST_ENUM_USE_BASE(NAME, ...)                                         \
  template <class... Args>                                                     \
  constexpr NAME(Args &&...args) :                                             \
      __VA_ARGS__{::crust::forward<Args>(args)...} {}

template <bool trivial, class... Fields>
union EnumHolderImpl;

template <class Field, class... Fields>
union EnumHolderImpl<true, Field, Fields...> {
  Field field;
  EnumHolderImpl<true, Fields...> remains;

  constexpr EnumHolderImpl() : remains{} {}

  explicit constexpr EnumHolderImpl(Field &&field) : field{move(field)} {}

  explicit constexpr EnumHolderImpl(const Field &field) : field{field} {}

  template <class T>
  explicit constexpr EnumHolderImpl(T &&field) : remains{forward<T>(field)} {}
};

template <class Field, class... Fields>
union EnumHolderImpl<false, Field, Fields...> {
  Field field;
  EnumHolderImpl<false, Fields...> remains;

  constexpr EnumHolderImpl() : remains{} {}

  explicit constexpr EnumHolderImpl(Field &&field) : field{move(field)} {}

  explicit constexpr EnumHolderImpl(const Field &field) : field{field} {}

  template <class T>
  explicit constexpr EnumHolderImpl(T &&field) : remains{forward<T>(field)} {}

  EnumHolderImpl(const EnumHolderImpl &) {}

  EnumHolderImpl(EnumHolderImpl &&) noexcept {}

  EnumHolderImpl &operator=(const EnumHolderImpl &) { return *this; }

  EnumHolderImpl &operator=(EnumHolderImpl &&) noexcept { return *this; }

  ~EnumHolderImpl() {}
};

template <bool trivial>
union EnumHolderImpl<trivial> {};

template <class... Fields>
using EnumHolder =
    EnumHolderImpl<All<IsTriviallyCopyable<Fields>...>::result, Fields...>;

template <isize index, class... Fields>
struct EnumGetter;

template <isize index, class Field, class... Fields>
struct EnumGetter<index, Field, Fields...> {
  using Self = EnumHolder<Field, Fields...>;
  using Result = typename _impl_types::
      TypesIndex<index, _impl_types::Types<Field, Fields...>>::Result;

  static constexpr const Result &inner(const Self &self) {
    return EnumGetter<index - 1, Fields...>::inner(self.remains);
  }

  static constexpr Result &inner(Self &self) {
    return EnumGetter<index - 1, Fields...>::inner(self.remains);
  }
};

template <class Field, class... Fields>
struct EnumGetter<0, Field, Fields...> {
  using Self = EnumHolder<Field, Fields...>;
  using Result = Field;

  static constexpr const Result &inner(const Self &self) { return self.field; }

  static constexpr Result &inner(Self &self) { return self.field; }
};

template <class Self, isize offset, isize size, class... Fields>
struct EnumVisitor {
  static constexpr isize cut = size / 2;

  using LowerGetter = EnumVisitor<Self, offset, cut, Fields...>;
  using UpperGetter = EnumVisitor<Self, offset + cut, size - cut, Fields...>;
  static constexpr typename Self::Index discriminant =
      IndexToDiscriminant<typename Self::Index, offset + cut, Fields...>::
          result;

  template <class R, class V>
  static constexpr R inner(const Self &self, V &&impl) {
    return self.get_index() < discriminant ?
        LowerGetter::template inner<R, V>(self, forward<V>(impl)) :
        UpperGetter::template inner<R, V>(self, forward<V>(impl));
  }

  template <class R, class V>
  static constexpr R inner(Self &self, V &&impl) {
    return self.get_index() < discriminant ?
        LowerGetter::template inner<R, V>(self, forward<V>(impl)) :
        UpperGetter::template inner<R, V>(self, forward<V>(impl));
  }
};

#define _ENUM_VISITOR_BRANCH(index)                                            \
  case GetIndex<index>::result:                                                \
    return impl(self.template unsafe_get_variant<GetType<index>>());

#define _ENUM_VISITOR_SWITCH_IMPL(len)                                         \
  template <class Self, isize offset, class... Fields>                         \
  struct EnumVisitor<Self, offset, len, Fields...> {                           \
    template <isize index>                                                     \
    using GetIndex =                                                           \
        IndexToDiscriminant<typename Self::Index, offset + index, Fields...>;  \
    template <isize index>                                                     \
    using GetType = typename _impl_types::                                     \
        TypesIndex<offset + index, _impl_types::Types<Fields...>>::Result;     \
    template <class R, class V>                                                \
    static crust_cxx14_constexpr R inner(const Self &self, V &&impl) {         \
      switch (self.get_index()) {                                              \
        CRUST_MACRO_REPEAT(len, _ENUM_VISITOR_BRANCH);                         \
      default:                                                                 \
        crust_unreachable();                                                   \
      }                                                                        \
    }                                                                          \
    template <class R, class V>                                                \
    static crust_cxx14_constexpr R inner(Self &self, V &&impl) {               \
      switch (self.get_index()) {                                              \
        CRUST_MACRO_REPEAT(len, _ENUM_VISITOR_BRANCH);                         \
      default:                                                                 \
        crust_unreachable();                                                   \
      }                                                                        \
    }                                                                          \
  }

_ENUM_VISITOR_SWITCH_IMPL(16);
_ENUM_VISITOR_SWITCH_IMPL(15);
_ENUM_VISITOR_SWITCH_IMPL(14);
_ENUM_VISITOR_SWITCH_IMPL(13);
_ENUM_VISITOR_SWITCH_IMPL(12);
_ENUM_VISITOR_SWITCH_IMPL(11);
_ENUM_VISITOR_SWITCH_IMPL(10);
_ENUM_VISITOR_SWITCH_IMPL(9);
_ENUM_VISITOR_SWITCH_IMPL(8);
_ENUM_VISITOR_SWITCH_IMPL(7);
_ENUM_VISITOR_SWITCH_IMPL(6);
_ENUM_VISITOR_SWITCH_IMPL(5);
_ENUM_VISITOR_SWITCH_IMPL(4);
_ENUM_VISITOR_SWITCH_IMPL(3);
_ENUM_VISITOR_SWITCH_IMPL(2);
_ENUM_VISITOR_SWITCH_IMPL(1);

#undef _ENUM_VISITOR_SWITCH_IMPL
#undef _ENUM_VISITOR_BRANCH

template <class Self, bool flag>
struct EnumTrivial {};

template <class Self>
struct EnumTrivial<Self, false> {
private:
  constexpr const Self &self() const {
    return *static_cast<const Self *>(this);
  }

  crust_cxx14_constexpr Self &self() { return *static_cast<Self *>(this); }

  struct Drop {
    template <class T>
    crust_cxx14_constexpr void operator()(T &value) const {
      value.~T();
    }
  };

  struct MoveTo {
    Self *other;

    template <class T>
    void operator()(T &value) {
      ::new (&other->template unsafe_get_variant<T>()) T{move(value)};
    }
  };

  struct CopyTo {
    Self *other;

    template <class T>
    void operator()(const T &value) {
      ::new (&other->template unsafe_get_variant<T>()) T{value};
    }
  };

  void drop() {
    if (self().index != 0) {
      self().visit(Drop{});
      self().index = 0;
    }
  }

  void move_from(Self &&other) { other.visit(MoveTo{&self()}); }

  void clone_from(const Self &other) { other.visit(CopyTo{&self()}); }

public:
  constexpr EnumTrivial() {}

  EnumTrivial(const EnumTrivial &other) { clone_from(other.self()); }

  EnumTrivial(EnumTrivial &&other) noexcept { move_from(move(other.self())); }

  EnumTrivial &operator=(const EnumTrivial &other) {
    if (this != &other) {
      drop();
      clone_from(other.self());
    }

    return *this;
  }

  EnumTrivial &operator=(EnumTrivial &&other) noexcept {
    if (this != &other) {
      drop();
      move_from(move(other.self()));
    }

    return *this;
  }

  ~EnumTrivial() { drop(); }
};

template <class Index_, class... Fields>
struct crust_ebco EnumTagUnion :
    EnumTrivial<
        EnumTagUnion<Index_, Fields...>,
        All<IsTriviallyCopyable<Fields>...>::result> {
  using Index = Index_;

  using Getter = EnumVisitor<EnumTagUnion, 0, sizeof...(Fields), Fields...>;

  template <class T>
  using IndexGetter =
      Discriminant<Index, typename RemoveConstOrRefType<T>::Result, Fields...>;

  crust_static_assert(
      All<Not<Require<Fields, DiscriminantVariant>>...>::result);

  struct Equal {
    const EnumTagUnion *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value == other->unsafe_get_variant<T>();
    };
  };

  struct NotEqual {
    const EnumTagUnion *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value != other->unsafe_get_variant<T>();
    };
  };

  struct PartialCmp {
    const EnumTagUnion *other;

    template <class T>
    constexpr Option<cmp::Ordering> operator()(const T &value) const;
  };

  struct LessThan {
    const EnumTagUnion *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value < other->unsafe_get_variant<T>();
    };
  };

  struct LessEqual {
    const EnumTagUnion *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value <= other->unsafe_get_variant<T>();
    };
  };

  struct GreaterThan {
    const EnumTagUnion *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value > other->unsafe_get_variant<T>();
    };
  };

  struct GreaterEqual {
    const EnumTagUnion *other;

    template <class T>
    constexpr bool operator()(const T &value) const {
      return value >= other->unsafe_get_variant<T>();
    };
  };

  struct Cmp {
    const EnumTagUnion *other;

    template <class T>
    constexpr cmp::Ordering operator()(const T &value) const;
  };

  Index index;
  EnumHolder<Fields...> holder;

  constexpr EnumTagUnion() : index{0}, holder{} {}

  template <class T>
  explicit constexpr EnumTagUnion(T &&value) :
      index{IndexGetter<T>::result + 1}, holder{forward<T>(value)} {}

  template <class T>
  EnumTagUnion &operator=(T &&value) {
    ~EnumTagUnion();

    ::new (&holder) EnumHolder<Fields...>{forward<T>(value)};
    set_index(IndexGetter<T>::result);

    return *this;
  }

  constexpr Index get_index() const { return index - 1; }

  crust_cxx14_constexpr void set_index(Index index) { this->index = index + 1; }

  template <class T>
  constexpr const T &unsafe_get_variant() const {
    return EnumGetter<
        _impl_types::TypesFirstIndex<T, _impl_types::Types<Fields...>>::result,
        Fields...>::inner(holder);
  }

  template <class T>
  crust_cxx14_constexpr T &unsafe_get_variant() {
    return EnumGetter<
        _impl_types::TypesFirstIndex<T, _impl_types::Types<Fields...>>::result,
        Fields...>::inner(holder);
  }

  template <class T>
  constexpr bool is_variant() const {
    return get_index() == IndexGetter<T>::result;
  }

  template <class R = void, class V>
  constexpr R visit(V &&visitor) const {
    return Getter::template inner<R, V>(*this, forward<V>(visitor));
  }

  template <class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) {
    return Getter::template inner<R, V>(*this, forward<V>(visitor));
  }

  constexpr bool eq(const EnumTagUnion &other) const {
    return index == other.index && visit<bool>(Equal{&other});
  }

  constexpr bool ne(const EnumTagUnion &other) const {
    return index != other.index || visit<bool>(NotEqual{&other});
  }

  constexpr Option<cmp::Ordering> partial_cmp(const EnumTagUnion &other) const;

  constexpr bool lt(const EnumTagUnion &other) const {
    return index != other.index ? index < other.index :
                                  visit<bool>(LessThan{&other});
  }

  constexpr bool le(const EnumTagUnion &other) const {
    return index != other.index ? index <= other.index :
                                  visit<bool>(LessEqual{&other});
  }

  constexpr bool gt(const EnumTagUnion &other) const {
    return index != other.index ? index > other.index :
                                  visit<bool>(GreaterThan{&other});
  }

  constexpr bool ge(const EnumTagUnion &other) const {
    return index != other.index ? index >= other.index :
                                  visit<bool>(GreaterEqual{&other});
  }

  constexpr cmp::Ordering cmp(const EnumTagUnion &other) const;
};

template <class Index_, class... Fields>
struct crust_ebco EnumTagOnly : _impl_types::ZeroSizedTypeHolder<Fields...> {
  using Index = Index_;

  using Getter = EnumVisitor<EnumTagOnly, 0, sizeof...(Fields), Fields...>;

  template <class T>
  using IndexGetter =
      Discriminant<Index, typename RemoveConstOrRefType<T>::Result, Fields...>;

  crust_static_assert(
      CheckDiscriminant<Index, sizeof...(Fields) - 1, Fields...>::result);

  Index index;

  constexpr EnumTagOnly() : index{0} {}

  template <class T>
  explicit constexpr EnumTagOnly(T &&) : index{IndexGetter<T>::result} {}

  constexpr Index get_index() const { return index; }

  crust_cxx14_constexpr void set_index(Index index) { this->index = index; }

  template <class T>
  constexpr const T &unsafe_get_variant() const {
    return _impl_types::ZeroSizedTypeGetter<
        _impl_types::TypesFirstIndex<T, _impl_types::Types<Fields...>>::result,
        Fields...>::inner(*this);
  }

  template <class T>
  crust_cxx14_constexpr T &unsafe_get_variant() {
    return _impl_types::ZeroSizedTypeGetter<
        _impl_types::TypesFirstIndex<T, _impl_types::Types<Fields...>>::result,
        Fields...>::inner(*this);
  }

  template <class T>
  constexpr bool is_variant() const {
    return get_index() == IndexGetter<T>::result;
  }

  template <class R = void, class V>
  constexpr R visit(V &&visitor) const {
    return Getter::template inner<R, V>(*this, forward<V>(visitor));
  }

  template <class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) {
    return Getter::template inner<R, V>(*this, forward<V>(visitor));
  }

  // FIXME: to make it constexpr, we cannot compare the real type here
  constexpr bool eq(const EnumTagOnly &other) const {
    return index == other.index;
  }

  constexpr bool ne(const EnumTagOnly &other) const {
    return index != other.index;
  }

  constexpr Option<cmp::Ordering> partial_cmp(const EnumTagOnly &other) const;

  constexpr bool lt(const EnumTagOnly &other) const {
    return index < other.index;
  }

  constexpr bool le(const EnumTagOnly &other) const {
    return index <= other.index;
  }

  constexpr bool gt(const EnumTagOnly &other) const {
    return index > other.index;
  }

  constexpr bool ge(const EnumTagOnly &other) const {
    return index >= other.index;
  }

  constexpr cmp::Ordering cmp(const EnumTagOnly &other) const;
};

template <class hint, bool is_tag_only, class... Fields>
struct EnumSelectImpl;

template <class hint, class... Fields>
struct EnumSelectImpl<hint, false, Fields...> : EnumTagUnion<hint, Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelectImpl, EnumTagUnion<hint, Fields...>);
};

template <class hint, class... Fields>
struct EnumSelectImpl<hint, true, Fields...> : EnumTagOnly<hint, Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelectImpl, EnumTagOnly<hint, Fields...>);
};

// TODO: auto detect
template <class... Fields>
struct EnumSelectImpl<void, false, Fields...> : EnumTagUnion<i32, Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelectImpl, EnumTagUnion<i32, Fields...>);
};

template <class... Fields>
struct EnumSelectImpl<void, true, Fields...> : EnumTagOnly<i32, Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelectImpl, EnumTagOnly<i32, Fields...>);
};

template <class hint, class... Fields>
using EnumSelect = EnumSelectImpl<
    hint,
    All<Require<Fields, ZeroSizedType>...>::result,
    Fields...>;

template <class T>
struct LetEnum;

template <class... Ts>
struct crust_ebco Overloaded;

template <class T, class... Ts>
struct Overloaded<T, Ts...> : T, Overloaded<Ts...> {
  using T::operator();
  using Overloaded<Ts...>::operator();

  explicit constexpr Overloaded(T t, Ts... ts) :
      T{forward<T>(t)}, Overloaded<Ts...>{forward<Ts>(ts)...} {}
};

template <class T>
struct Overloaded<T> : T {
  using T::operator();

  explicit constexpr Overloaded(T t) : T{forward<T>(t)} {}
};

template <class... Ts>
constexpr Overloaded<Ts...> overloaded(Ts &&...ts) {
  return Overloaded<Ts...>{forward<Ts>(ts)...};
}

CRUST_TRAIT(EnumAs, class Inner) {
  CRUST_TRAIT_USE_SELF(EnumAs);

  constexpr typename Inner::Index as() const {
    return self().inner.get_index();
  }
};

template <class Inner, class... Fields>
struct Enum :
    InheritIf<
        EnumAs<Enum<Inner, Fields...>, Inner>,
        All<Require<Fields, ZeroSizedType>...>> {
private:
  crust_static_assert(sizeof...(Fields) > 0);
  crust_static_assert(Not<_impl_types::TypesDuplicateVal<
                          _impl_types::Types<Fields...>>>::result);
  crust_static_assert(All<Not<IsConstOrRefVal<Fields>>...>::result);

  template <class>
  friend struct LetEnum;

  template <class, class>
  friend struct EnumAs;

  template <class, class>
  friend struct ::crust::ImplFor;

  Inner inner;

protected:
  constexpr Enum() : inner{} {}

  template <
      class T,
      class = EnableIf<
          Not<IsBaseOfVal<Enum, typename RemoveConstOrRefType<T>::Result>>>>
  constexpr Enum(T &&value) : inner{forward<T>(value)} {}

  template <class T>
  constexpr bool is_variant() const {
    return inner.template is_variant<T>();
  }

  template <class T>
  crust_cxx14_constexpr Option<T> move_variant() &&;

  template <class T>
  crust_cxx14_constexpr bool let_helper(T &ref) && {
    if (is_variant<T>()) {
      ref = move(inner.template unsafe_get_variant<T>());
      return true;
    } else {
      return false;
    }
  }

public:
  template <
      class T,
      class = EnableIf<
          Not<IsBaseOfVal<Enum, typename RemoveConstOrRefType<T>::Result>>>>
  Enum &operator=(T &&value) {
    inner = Inner{forward<T>(value)};
    return *this;
  }

  template <class R = void, class... Fs>
  constexpr R visit(Fs &&...fs) const {
    return inner.template visit<R>(overloaded(forward<Fs>(fs)...));
  }

  template <class R = void, class... Fs>
  crust_cxx14_constexpr R visit(Fs &&...fs) {
    return inner.template visit<R>(overloaded(forward<Fs>(fs)...));
  }

  template <class T, class R = void, class... Fs>
  constexpr R visit_variant(Fs &&...fs) const {
    return is_variant<T>() ?
        overloaded(forward<Fs>(fs)...)(inner.template unsafe_get_variant<T>()) :
        overloaded(forward<Fs>(fs)...)();
  }

  template <class T, class R = void, class... Fs>
  crust_cxx14_constexpr R visit_variant(Fs &&...fs) {
    return is_variant<T>() ?
        overloaded(forward<Fs>(fs)...)(inner.template unsafe_get_variant<T>()) :
        overloaded(forward<Fs>(fs)...)();
  }
};

// TODO: option like enum

// template<class Field>
// struct Enum<void, Field> {
//   // TODO: only one possibility, this is just Field
// };
} // namespace _impl_enum

template <class Inner, class... Fields>
CRUST_IMPL_FOR(
    CRUST_MACRO(_impl_enum::EnumAs<_impl_enum::Enum<Inner, Fields...>, Inner>),
    Require<Fields, ZeroSizedType>...){};

template <class Type>
struct EnumRepr : TmplType<Type> {};

template <class... Fields>
struct crust_ebco Enum :
    _impl_enum::Enum<_impl_enum::EnumSelect<void, Fields...>, Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      Enum,
      _impl_enum::Enum<_impl_enum::EnumSelect<void, Fields...>, Fields...>);
};

template <class Type, class... Fields>
struct crust_ebco Enum<EnumRepr<Type>, Fields...> :
    _impl_enum::Enum<_impl_enum::EnumSelect<Type, Fields...>, Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      Enum,
      _impl_enum::Enum<_impl_enum::EnumSelect<Type, Fields...>, Fields...>);
};

namespace _impl_derive {
template <class T, template <class, class...> class Trait, class... Args>
struct ImplForEnum : TmplVal<bool, false> {};

template <
    class... Fields,
    template <class, class...>
    class Trait,
    class... Args>
struct ImplForEnum<Enum<Fields...>, Trait, Args...> :
    All<Require<Fields, Trait, Args...>...> {};
} // namespace _impl_derive

template <class S>
CRUST_IMPL_FOR(
    clone::Clone<S>,
    _impl_derive::ImplForEnum<typename BluePrint<S>::Result, clone::Clone>) {
  CRUST_IMPL_USE_SELF(S);

private:
  struct Clone {
    template <class T>
    Self operator()(const T &value) {
      return clone::clone(value);
    }
  };

public:
  Self clone() const { return self().template visit<Self>(Clone{}); }
};

template <class S>
CRUST_IMPL_FOR(
    cmp::PartialEq<S>,
    _impl_derive::ImplForEnum<typename BluePrint<S>::Result, cmp::PartialEq>) {
  CRUST_IMPL_USE_SELF(S);

  constexpr bool eq(const Self &other) const {
    return self().inner.eq(other.inner);
  }

  constexpr bool ne(const Self &other) const {
    return self().inner.ne(other.inner);
  }
};

template <class S>
CRUST_IMPL_FOR(
    cmp::Eq<S>,
    _impl_derive::ImplForEnum<typename BluePrint<S>::Result, cmp::Eq>){};

template <class S>
CRUST_IMPL_FOR(
    cmp::PartialOrd<S>,
    _impl_derive::ImplForEnum<typename BluePrint<S>::Result, cmp::PartialOrd>) {
  CRUST_IMPL_USE_SELF(S);

  constexpr Option<cmp::Ordering> partial_cmp(const Self &other) const;

  constexpr bool lt(const Self &other) const {
    return self().inner.lt(other.inner);
  }

  constexpr bool le(const Self &other) const {
    return self().inner.le(other.inner);
  }

  constexpr bool gt(const Self &other) const {
    return self().inner.gt(other.inner);
  }

  constexpr bool ge(const Self &other) const {
    return self().inner.ge(other.inner);
  }
};

template <class S>
CRUST_IMPL_FOR(
    cmp::Ord<S>,
    _impl_derive::ImplForEnum<typename BluePrint<S>::Result, cmp::Ord>) {
  CRUST_IMPL_USE_SELF(S);

  constexpr cmp::Ordering cmp(const Self &other) const;
};
} // namespace crust


#endif // CRUST_ENUM_DECL_HPP

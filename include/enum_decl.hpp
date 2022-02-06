#ifndef _CRUST_INCLUDE_ENUM_DECL_HPP
#define _CRUST_INCLUDE_ENUM_DECL_HPP


#include <new>
#include <limits>

#include "utility.hpp"
#include "cmp_decl.hpp"
#include "tuple_decl.hpp"


namespace crust {
namespace option {
template<class T>
class Option;
}

namespace _impl_enum {
template<class T, class ...Fields>
struct EnumInclude;

template<class T, class Field, class ...Fields>
struct EnumInclude<T, Field, Fields...> {
  static constexpr bool result = EnumInclude<T, Fields...>::result;
};

template<class T, class ...Fields>
struct EnumInclude<T, T, Fields...> {
  static constexpr bool result = true;
};

template<class T>
struct EnumInclude<T> {
  static constexpr bool result = false;
};

template<class ...Fields>
struct EnumDuplicate;

template<class Field, class ...Fields>
struct EnumDuplicate<Field, Fields...> {
  static constexpr bool result =
      EnumInclude<Field, Fields...>::result ||
      EnumDuplicate<Fields...>::result;
};

template<>
struct EnumDuplicate<> {
  static constexpr bool result = false;
};


template<bool trivial, class ...Fields>
union EnumHolder;

template<class Field, class ...Fields>
union EnumHolder<true, Field, Fields...> {
  using Remains = EnumHolder<true, Fields...>;

  Field field;
  Remains remains;

  explicit constexpr EnumHolder(Field &&field) noexcept : field{move(field)} {}

  explicit constexpr EnumHolder(const Field &field) noexcept : field{field} {}

  template<class T>
  explicit constexpr EnumHolder(T &&field) noexcept :
      remains{forward<T>(field)}
  {
    crust_static_assert(
        !IsSame<typename RemoveConstOrRef<T>::Result, Field>::result
    );
  }
};

template<class Field, class ...Fields>
union EnumHolder<false, Field, Fields...> {
  using Remains = EnumHolder<false, Fields...>;

  Field field;
  Remains remains;

  constexpr EnumHolder() noexcept : remains{} {}

  explicit constexpr EnumHolder(Field &&field) noexcept : field{move(field)} {}

  explicit constexpr EnumHolder(const Field &field) noexcept : field{field} {}

  template<class T>
  explicit constexpr EnumHolder(T &&field) noexcept :
      remains{forward<T>(field)}
  {
    crust_static_assert(
        !IsSame<typename RemoveConstOrRef<T>::Result, Field>::result
    );
  }

  EnumHolder(const EnumHolder &) noexcept {}

  EnumHolder(EnumHolder &&) noexcept {}

  EnumHolder &operator=(const EnumHolder &) noexcept { return *this; }

  EnumHolder &operator=(EnumHolder &&) noexcept { return *this; }

  ~EnumHolder() {}
};

template<bool trivial>
union EnumHolder<trivial> {};

template<usize index, class ...Fields>
struct EnumType;

template<usize index, class Field, class ...Fields>
struct EnumType<index, Field, Fields...> {
  using Result = typename EnumType<index - 1, Fields...>::Result;
};

template<class Field, class ...Fields>
struct EnumType<0, Field, Fields...> {
  using Result = Field;
};

template<usize index, bool trivial, class ...Fields>
struct EnumGetter;

template<usize index, bool trivial, class Field, class ...Fields>
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

template<bool trivial, class Field, class ...Fields>
struct EnumGetter<0, trivial, Field, Fields...> {
  using Self = EnumHolder<trivial, Field, Fields...>;
  using Result = Field;

  static constexpr const Result &inner(const Self &self) {
    return self.field;
  }

  static constexpr Result &inner(Self &self) { return self.field; }
};

template<usize offset, usize size, bool trivial, class ...Fields>
struct EnumVisitor {
private:
  crust_static_assert(offset + size <= sizeof...(Fields));

  static constexpr usize cut = size / 2;

  using LowerGetter = EnumVisitor<offset, cut, trivial, Fields...>;
  using UpperGetter =
      EnumVisitor<offset + cut, size - cut, trivial, Fields...>;

public:
  template<class R, class V>
  static crust_cxx14_constexpr R
  inner(const EnumHolder<trivial, Fields...> &self, V &&impl, usize index) {
    return index < cut
         ? LowerGetter::template inner<R, V>(self, forward<V>(impl), index)
         : UpperGetter::template inner<R, V>(self, forward<V>(impl), index);
  }

  template<class R, class V>
  static crust_cxx14_constexpr R
  inner(EnumHolder<trivial, Fields...> &self, V &&impl, usize index) {
    return index < cut
         ? LowerGetter::template inner<R, V>(self, forward<V>(impl), index)
         : UpperGetter::template inner<R, V>(self, forward<V>(impl), index);
  }
};

#define _ENUM_VISITOR_BRANCH(index) \
  case offset + index: \
    return impl(EnumGetter<offset + index, trivial, Fields...>::inner(self))

#define _ENUM_VISITOR_IMPL(len) \
  template<usize offset, bool trivial, class ...Fields> \
  struct EnumVisitor<offset, len, trivial, Fields...> { \
    crust_static_assert(offset + len <= sizeof...(Fields)); \
    template<class R, class V> \
    static crust_cxx14_constexpr R \
    inner(const EnumHolder<trivial, Fields...> &self, V &&impl, usize index) { \
      switch (index) { \
        default: \
          crust_unreachable(); \
        CRUST_MACRO_REPEAT(len, _ENUM_VISITOR_BRANCH); \
      } \
    } \
    template<class R, class V> \
    static crust_cxx14_constexpr R \
    inner(EnumHolder<trivial, Fields...> &self, V &&impl, usize index) { \
      switch (index) { \
        default: \
          crust_unreachable(); \
        CRUST_MACRO_REPEAT(len, _ENUM_VISITOR_BRANCH); \
      } \
    } \
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


template<usize offset, usize size, class ...Fields>
struct TagVisitor {
private:
  crust_static_assert(offset + size <= sizeof...(Fields));

  static constexpr usize cut = size / 2;

  using LowerGetter = TagVisitor<offset, cut, Fields...>;
  using UpperGetter = TagVisitor<offset + cut, size - cut, Fields...>;

public:
  template<class R, class V>
  static crust_cxx14_constexpr R inner(V &&impl, usize index) {
    return index < cut ?
        LowerGetter::template inner<R, V>(forward<V>(impl), index) :
        UpperGetter::template inner<R, V>(forward<V>(impl), index);
  }
};

#define _TAG_VISITOR_BRANCH(index) \
  case offset + index: \
    return impl(typename EnumType<offset + index, Fields...>::Result{});

#define _TAG_VISITOR_IMPL(len) \
  template<usize offset, class ...Fields> \
  struct TagVisitor<offset, len, Fields...> { \
    crust_static_assert(offset + len <= sizeof...(Fields)); \
    template<class R, class V> \
    static crust_cxx14_constexpr R inner(V &&impl, usize index) { \
      switch (index) { \
        default: \
          crust_unreachable(); \
        CRUST_MACRO_REPEAT(len, _TAG_VISITOR_BRANCH); \
      } \
    } \
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


template<class T, class ...Fields>
struct EnumTypeToIndex;

template<class T, class Field, class ...Fields>
struct EnumTypeToIndex<T, Field, Fields...> {
  static constexpr u32 result = EnumTypeToIndex<T, Fields...>::result + 1;
};

template<class T, class ...Fields>
struct EnumTypeToIndex<T, T, Fields...> {
  static constexpr u32 result = 0;
};

template<class Self, bool flag>
struct EnumTrivial;

template<class Self>
struct EnumTrivial<Self, true> {
  void drop() {}
};

template<class Self>
struct EnumTrivial<Self, false> {
  constexpr EnumTrivial() {
    crust_static_assert(std::is_base_of<EnumTrivial, Self>::value);
  }

  constexpr const Self &self() const {
    return *static_cast<const Self *>(this);
  }

  crust_cxx14_constexpr Self &self() {
    return *static_cast<Self *>(this);
  };

  void drop() {
    if (self().index != 0) { self().visit(typename Self::Drop{}); }
  }

  EnumTrivial(const EnumTrivial &other) noexcept {
    if (this != &other) {
      other.self().visit(typename Self::Copy{&self().holder});
    }
  }

  EnumTrivial(EnumTrivial &&other) noexcept {
    if (this != &other) {
      other.self().visit(typename Self::Emplace{&self().holder});
    }
  }

  EnumTrivial &operator=(const EnumTrivial &other) noexcept {
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

template<class ...Fields>
struct EnumIsTrivial :
    public All<std::is_trivially_copyable<Fields>::value...>
{};

template<class ...Fields>
struct EnumIsTagOnly : public All<IsMonoState<Fields>::result...> {};


template<class ...Fields>
struct crust_ebco EnumTagUnion :
    public EnumTrivial<
        EnumTagUnion<Fields...>,
        EnumIsTrivial<Fields...>::result
    >,
    public cmp::PartialEq<EnumTagUnion<Fields...>>,
    public cmp::Eq<EnumTagUnion<Fields...>>
{
  static constexpr bool _trivial = EnumIsTrivial<Fields...>::result;

  friend EnumTrivial<EnumTagUnion, _trivial>;

  using Holder =
      EnumHolder<_trivial, typename RemoveConstOrRef<Fields>::Result...>;
  template<class T> using IndexGetter = EnumTypeToIndex<T, Fields...>;
  using Getter = EnumVisitor<0, sizeof...(Fields), _trivial, Fields...>;

  Holder holder;
  u32 index;

  constexpr EnumTagUnion() noexcept : holder{}, index{0} {}

  template<class T>
  explicit constexpr EnumTagUnion(T &&value) noexcept :
      holder{forward<T>(value)},
      index{IndexGetter<typename RemoveConstOrRef<T>::Result>::result + 1}
  {
    crust_static_assert(
        !IsSame<typename RemoveConstOrRef<T>::Result, EnumTagUnion>::result
    );
  }

  struct Drop {
    template<class T>
    crust_cxx14_constexpr void operator()(T &value) const { value.~T(); }
  };

  struct Copy {
    Holder *holder;

    template<class T>
    void operator()(const T &value) { ::new(holder) Holder{value}; }
  };

  struct Emplace {
    Holder *holder;

    template<class T>
    void operator()(T &value) { ::new(holder) Holder{move(value)}; }
  };

  template<class T>
  EnumTagUnion &operator=(T &&value) noexcept {
    crust_static_assert(
        !IsSame<typename RemoveConstOrRef<T>::Result, EnumTagUnion>::result
    );

    this->drop();

    ::new(&holder) Holder{forward<T>(value)};
    index = IndexGetter<typename RemoveConstOrRef<T>::Result>::result + 1;

    return *this;
  }

  template<class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) const {
    crust_assert(index - 1 < sizeof...(Fields));
    return Getter::template inner<R, V>(
        holder, forward<V>(visitor), index - 1
    );
  }

  template<class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) {
    crust_assert(index - 1 < sizeof...(Fields));
    return Getter::template inner<R, V>(
        holder, forward<V>(visitor), index - 1
    );
  }

  template<class T, class V>
  constexpr bool visit_variant(V &&visitor) const {
    using Index = IndexGetter<typename RemoveConstOrRef<T>::Result>;
    return index == Index::result + 1 ? visitor(
        EnumGetter<Index::result, _trivial, Fields...>::inner(holder)
    ) : visitor();
  }

  template<class T>
  constexpr bool is_variant() const {
    return index ==
        IndexGetter<typename RemoveConstOrRef<T>::Result>::result + 1;
  }

  template<class T, class ...Fs>
  constexpr bool eq_variant(const Fs &...other) const {
    using Index = IndexGetter<typename RemoveConstOrRef<T>::Result>;
    return index == Index::result + 1 &&
        _impl_tuple::TupleEqHelper<sizeof...(Fs), Fs...>::inner(
            _impl_tuple::TupleHolder<const Fs &...>{other...},
            EnumGetter<Index::result, _trivial, Fields...>::inner(holder)
        );
  }

  template<class T>
  crust_cxx14_constexpr Option<T> move_variant();

  template<class T, class ...Fs>
  crust_cxx14_constexpr bool
  let_helper(_impl_tuple::TupleHolder<Fs &...> ref) {
    if (is_variant<T>()) {
      _impl_tuple::LetTupleHelper<sizeof...(Fs), Fs...>::inner(
          ref, move(EnumGetter<
              IndexGetter<typename RemoveConstOrRef<T>::Result>::result,
              _trivial,
              Fields...
          >::inner(holder))
      );
      return true;
    } else {
      return false;
    }
  }

  /// impl PartialEq

  struct Equal {
    const Holder *other;

    template<class T>
    constexpr bool operator()(const T &value) const {
      return value == EnumGetter<
          IndexGetter<typename RemoveConstOrRef<T>::Result>::result,
          _trivial,
          Fields...
      >::inner(*other);
    };
  };

  crust_cxx14_constexpr bool eq(const EnumTagUnion &other) const {
    return this->index == other.index && visit<bool>(Equal{&other.holder});
  }

  struct NotEqual {
    const Holder *other;

    template<class T>
    constexpr bool operator()(const T &value) const {
      return value != EnumGetter<
          IndexGetter<typename RemoveConstOrRef<T>::Result>::result,
          _trivial,
          Fields...
      >::inner(*other);
    };
  };

  crust_cxx14_constexpr bool ne(const EnumTagUnion &other) const {
    return this->index != other.index ||
        visit<bool>(NotEqual{&other.holder});
  }
};


template<class ...Fields>
struct crust_ebco EnumTagOnly :
    public cmp::PartialEq<EnumTagOnly<Fields...>>,
    public cmp::Eq<EnumTagOnly<Fields...>> {
  template<class T> using IndexGetter = EnumTypeToIndex<T, Fields...>;
  using Getter = TagVisitor<0, sizeof...(Fields), Fields...>;

  u32 index;

  constexpr EnumTagOnly() noexcept : index{0} {}

  template<class T>
  explicit constexpr EnumTagOnly(T &&) noexcept :
      index{IndexGetter<typename RemoveConstOrRef<T>::Result>::result + 1}
  {
    crust_static_assert(!IsSame<RemoveConstOrRef<T>, EnumTagOnly>::result);
  }

  template<class T>
  EnumTagOnly &operator=(T &&) noexcept {
    crust_static_assert(!IsSame<RemoveConstOrRef<T>, EnumTagOnly>::result);

    index = IndexGetter<typename RemoveConstOrRef<T>::Result>::result + 1;

    return *this;
  }

  template<class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) const {
    crust_assert(index - 1 < sizeof...(Fields));
    return Getter::template inner<R, V>(forward<V>(visitor), index - 1);
  }

  template<class R = void, class V>
  crust_cxx14_constexpr R visit(V &&visitor) {
    crust_assert(index - 1 < sizeof...(Fields));
    return Getter::template inner<R, V>(forward<V>(visitor), index - 1);
  }

  template<class T>
  constexpr bool is_variant() const {
    return index ==
        IndexGetter<typename RemoveConstOrRef<T>::Result>::result + 1;
  }

  template<class T>
  constexpr bool eq_variant() const { return is_variant<T>(); }

  template<class T>
  crust_cxx14_constexpr Option<T> move_variant();

  template<class T, class ...Fs>
  crust_cxx14_constexpr bool
  let_helper(_impl_tuple::TupleHolder<Fs &...> ref) {
    if (is_variant<T>()) {
      _impl_tuple::LetTupleHelper<sizeof...(Fs), Fs...>(ref, T{});
      return true;
    } else {
      return false;
    }
  }

  constexpr bool eq(const EnumTagOnly &other) const {
    return this->index == other.index;
  }
};

template<bool is_tag_only, class ...Fields>
struct EnumSelect;

template<class ...Fields>
struct EnumSelect<false, Fields...> : public EnumTagUnion<Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelect, EnumTagUnion<Fields...>);
};

template<class ...Fields>
struct EnumSelect<true, Fields...> : public EnumTagOnly<Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(EnumSelect, EnumTagOnly<Fields...>);
};

template<class T, class ...Fields>
struct LetEnum;

template<class... Ts>
struct Overloaded;

template<class T, class... Ts>
struct Overloaded<T, Ts...> : public T, public Overloaded<Ts ...> {
  using T::operator();
  using Overloaded<Ts ...>::operator();

  explicit Overloaded(T t, Ts ...ts) :
      T{forward<T>(t)}, Overloaded<Ts ...>{forward<Ts>(ts)...} {}
};

template<class T>
struct Overloaded<T> : public T {
  using T::operator();

  explicit Overloaded(T t) : T{forward<T>(t)} {}
};

template<class... Ts>
Overloaded<Ts...> overloaded(Ts &&...ts) {
  return Overloaded<Ts...>{forward<Ts>(ts)...};
}
}

// todo: implement PartialOrd and Ord
template<class ...Fields>
class crust_ebco Enum :
    public Impl<
        cmp::PartialEq<Enum<Fields...>>,
        Derive<Fields, cmp::PartialEq>::result...
    >,
    public Impl<cmp::Eq<Enum<Fields...>>, Derive<Fields, cmp::Eq>::result...>
{
private:
  crust_static_assert(sizeof...(Fields) < std::numeric_limits<u32>::max());
  crust_static_assert(sizeof...(Fields) > 0);
  crust_static_assert(!_impl_enum::EnumDuplicate<Fields...>::result);
  crust_static_assert(All<!IsConstOrRef<Fields>::result...>::result);

  using Inner = _impl_enum::EnumSelect<
      _impl_enum::EnumIsTagOnly<Fields...>::result, Fields...
  >;

  Inner inner;

  template<class, class ...>
  friend struct _impl_enum::LetEnum;

protected:
  template<class T>
  constexpr bool is_variant() const { return inner.template is_variant<T>(); }

  template<class T, class ...Fs>
  constexpr bool eq_variant(const Fs &...other) const {
    return inner.template eq_variant<T>(other...);
  }

  template<class T>
  crust_cxx14_constexpr Option<T> move_variant();

public:
  constexpr Enum() : inner{} {}

  template<class T, typename EnableIf<
      !IsSame<typename RemoveConstOrRef<T>::Result, Enum>::result
  >::Result * = nullptr>
  constexpr Enum(T &&value) noexcept :
      inner{forward<T>(value)}
  {}

  template<class T, typename EnableIf<
      !IsSame<typename RemoveConstOrRef<T>::Result, Enum>::result
  >::Result * = nullptr>
  Enum &operator=(T &&value) noexcept {
    inner = Inner{forward<T>(value)};
    return *this;
  }

  template<class R = void, class... Ts>
  crust_cxx14_constexpr R visit(Ts &&...ts) const {
    return inner.template visit<R>(_impl_enum::overloaded(forward<Ts>(ts)...));
  }

  template<class R = void, class... Ts>
  crust_cxx14_constexpr R visit(Ts &&...ts) {
    return inner.template visit<R>(_impl_enum::overloaded(forward<Ts>(ts)...));
  }

  crust_cxx14_constexpr bool eq(const Enum &other) const {
    return inner == other.inner;
  }

  crust_cxx14_constexpr bool ne(const Enum &other) const {
    return inner != other.inner;
  }
};

#define CRUST_ENUM_USE_BASE(NAME, ...) \
  template<class ...Args> \
  constexpr NAME(Args &&...args) : \
      __VA_ARGS__{::crust::forward<Args>(args)...} \
  {} \
  template<class = void> \
  static void _detect_trait_partial_eq(const NAME &) { \
    crust_static_assert( \
        ::crust::Derive<__VA_ARGS__, ::crust::cmp::PartialEq>::result) \
    ; \
  } \
  template<class = void> \
  static void _detect_trait_eq() { \
    crust_static_assert( \
        ::crust::Derive<__VA_ARGS__, ::crust::cmp::Eq>::result \
    ); \
  }

#define CRUST_ENUM_VARIANT(NAME, ...) \
  struct NAME final : public ::crust::Tuple<__VA_ARGS__> { \
    CRUST_USE_BASE_CONSTRUCTORS(NAME, ::crust::Tuple<__VA_ARGS__>) \
    template<class = void> \
    static void _detect_trait_partial_eq(const NAME &) { \
      crust_static_assert(::crust::Derive< \
          ::crust::Tuple<__VA_ARGS__>, ::crust::cmp::PartialEq \
      >::result); \
    } \
    template<class = void> \
    static void _detect_trait_eq() { \
      crust_static_assert(::crust::Derive< \
          ::crust::Tuple<__VA_ARGS__>, ::crust::cmp::Eq \
      >::result); \
    } \
  }
}


#endif //_CRUST_INCLUDE_ENUM_DECL_HPP

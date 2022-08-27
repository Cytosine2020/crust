#ifndef CRUST_UTILITY_HPP
#define CRUST_UTILITY_HPP


#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <type_traits>


namespace crust {
#if __cplusplus > 201103L
#define crust_cxx14_constexpr constexpr
#else
#define crust_cxx14_constexpr
#endif

#if __cplusplus > 201402L
#define crust_cxx17_constexpr constexpr
#else
#define crust_cxx17_constexpr
#endif

#if __cplusplus > 201402L
#define crust_static_assert(...) static_assert(__VA_ARGS__)
#else
#define crust_static_assert(...) static_assert(__VA_ARGS__, #__VA_ARGS__)
#endif

#if defined(_MSC_VER)
#define crust_ebco __declspec(empty_bases)
#else
#define crust_ebco
#endif

#if defined(__GNUC__) || defined(__clang__)
#define crust_no_return __attribute__((noreturn))
#elif defined(_MSC_VER)
#define crust_no_return __declspec(noreturn)
#else
#define crust_no_return
#endif

#if defined(__GNUC__) || defined(__clang__)
#define crust_likely(x) __builtin_expect(!!(x), 1)
#define crust_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define crust_likely(x) x
#define crust_unlikely(x) x
#endif

#if defined(__GNUC__) || defined(__clang__)
#define crust_always_inline inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define crust_always_inline __forceinline
#else
#define crust_always_inline inline
#endif

namespace _impl_utility {
crust_no_return inline void
panic(const char *file, int line, const char *msg) noexcept {
  fprintf(stderr, "Panic at file %s, line %d: %s\n", file, line, msg);
  exit(1);
}

#define crust_panic(msg) _impl_utility::panic(__FILE__, __LINE__, msg)

#define crust_assert(expr)                                                     \
  (crust_likely(expr) ? void(0) : [](const char *f, int l, const char *m) {    \
    fprintf(stderr, "Assert failed at file %s, line %d: %s\n", f, l, m);       \
    exit(-1);                                                                  \
  }(__FILE__, __LINE__, #expr))

#if defined(NODEBUG)
#define crust_debug_assert(expr)
#else
#define crust_debug_assert(expr) crust_assert(expr)
#endif

#if defined(NODEBUG)
#if defined(__GNUC__) || defined(__clang__)
#define crust_unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define crust_unreachable() __assume(false)
#else
#define crust_unreachable()
#endif
#else
#define crust_unreachable() crust_panic("should not reach here!")
#endif
} // namespace _impl_utility

#define CRUST_MACRO(...) __VA_ARGS__

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
#if INTPTR_MAX == INT32_MAX
using isize = i32;
using usize = u32;
#elif INTPTR_MAX == INT64_MAX
using isize = i64;
using usize = u64;
#else
#error "Unsupported bit width."
#endif

template <class Value>
struct TmplType {
  using Result = Value;
};

template <class Type, Type value>
struct TmplVal : TmplType<Type> {
  static constexpr Type result = value;
};

#define crust_tmpl_val(obj)                                                    \
  ::crust::TmplVal<decltype(obj), (obj)> {}

template <bool value>
struct BoolVal : TmplVal<bool, value> {};

template <class Bool>
struct Not : BoolVal<!Bool::result> {};

template <class... Bools>
struct All;

template <class Bool, class... Bools>
struct All<Bool, Bools...> : BoolVal<Bool::result && All<Bools...>::result> {};

template <>
struct All<> : BoolVal<true> {};

template <class... Bools>
struct Any;

template <class Bool, class... Bools>
struct Any<Bool, Bools...> : BoolVal<Bool::result || Any<Bools...>::result> {};

template <>
struct Any<> : BoolVal<false> {};

template <class U, class A>
struct AsVal : TmplVal<U, A::result> {};

template <class T, template <class> class... Tmpls>
struct CompositionType;

template <class T, template <class> class Tmpl, template <class> class... Tmpls>
struct CompositionType<T, Tmpl, Tmpls...> :
    CompositionType<typename Tmpl<T>::Result, Tmpls...> {};

template <class T>
struct CompositionType<T> : TmplType<T> {};

template <class A, class B>
struct IsSame : BoolVal<false> {};

template <class A>
struct IsSame<A, A> : BoolVal<true> {};

template <class T>
struct RemoveRefType : TmplType<T> {};

template <class T>
struct RemoveRefType<T &> : TmplType<T> {};

template <class T>
struct RemoveRefType<T &&> : TmplType<T> {};

template <class T>
struct RemoveConstType : TmplType<T> {};

template <class T>
struct RemoveConstType<T const> : TmplType<T> {};

template <class T>
struct RemoveConstOrRefType :
    CompositionType<T, RemoveRefType, RemoveConstType> {};

template <class T>
struct IsEmptyType : BoolVal<std::is_empty<T>::value> {};

template <class T>
struct IsLValueRefVal : BoolVal<false> {};

template <class T>
struct IsLValueRefVal<T &> : BoolVal<true> {};

template <class T>
struct IsRValueRefVal : BoolVal<false> {};

template <class T>
struct IsRValueRefVal<T &&> : BoolVal<true> {};

template <class T>
struct IsConstVal : BoolVal<false> {};

template <class T>
struct IsConstVal<const T> : BoolVal<true> {};

template <class T>
struct IsRefVal : Any<IsLValueRefVal<T>, IsRValueRefVal<T>> {};

template <class T>
struct IsConstOrRefVal : Any<IsRefVal<T>, IsConstVal<T>> {};

template <class B, class T>
struct IsBaseOfVal :
    All<BoolVal<std::is_base_of<B, T>::value>, Not<IsSame<B, T>>> {};

template <class T>
struct IsTriviallyCopyable : BoolVal<std::is_trivially_copyable<T>::value> {};

template <class T>
constexpr typename RemoveRefType<T>::Result &&move(T &&t) {
  return static_cast<typename RemoveRefType<T>::Result &&>(t);
}

template <class T>
constexpr T &&forward(typename RemoveRefType<T>::Result &t) {
  return static_cast<T &&>(t);
}

template <class T>
constexpr T &&forward(typename RemoveRefType<T>::Result &&t) {
  crust_static_assert(!IsLValueRefVal<T>::result);
  return static_cast<T &&>(t);
}

#define CRUST_USE_BASE_CONSTRUCTORS(NAME, ...)                                 \
  template <class... Args>                                                     \
  explicit constexpr NAME(Args &&...args) :                                    \
      __VA_ARGS__{::crust::forward<Args>(args)...} {}

#define CRUST_TRAIT(TRAIT, ...)                                                \
  template <class Self, ##__VA_ARGS__>                                         \
  struct TRAIT

#define CRUST_TRAIT_USE_SELF(TRAIT, ...)                                       \
private:                                                                       \
  constexpr const Self &self() const {                                         \
    return *static_cast<const Self *>(this);                                   \
  }                                                                            \
  crust_cxx14_constexpr Self &self() { return *static_cast<Self *>(this); }    \
                                                                               \
protected:                                                                     \
  constexpr TRAIT() {                                                          \
    crust_static_assert(::crust::IsBaseOfVal<TRAIT, Self>::result);            \
    crust_static_assert(::crust::All<__VA_ARGS__>::result);                    \
  }                                                                            \
                                                                               \
public:

namespace _impl_utility {
template <class Self, template <class, class...> class T, class... Args>
using Trait = T<Self, Args...>;
} // namespace _impl_utility

template <template <class, class...> class T, class... Args>
struct Trait {};

template <class Trait, class Self, class Enable = void>
struct ImplFor {};

#define CRUST_IMPL_FOR(TRAIT, SELF, ...)                                       \
  struct ImplFor<Trait<TRAIT>, SELF, EnableIf<__VA_ARGS__>> :                  \
      _impl_utility::Trait<SELF, TRAIT>

#define CRUST_IMPL_USE_SELF(...)                                               \
private:                                                                       \
  using Self = __VA_ARGS__;                                                    \
  constexpr const Self &self() const {                                         \
    return *static_cast<const Self *>(this);                                   \
  }                                                                            \
  crust_cxx14_constexpr Self &self() { return *static_cast<Self *>(this); }    \
                                                                               \
protected:                                                                     \
  constexpr ImplFor() {                                                        \
    crust_static_assert(::crust::IsBaseOfVal<ImplFor, Self>::result);          \
  }                                                                            \
                                                                               \
public:

template <class Self, class... Traits>
struct crust_ebco Impl;

template <
    class Self,
    template <class, class...>
    class T,
    class... Args,
    class... Traits>
struct Impl<Self, Trait<T, Args...>, Traits...> :
    ImplFor<Trait<T, Args...>, Self>,
    Impl<Self, Traits...> {};

template <class Self>
struct Impl<Self> {};

namespace _impl_derive {
template <class Self>
struct TupleLikeSize;

template <class Self, usize index>
struct TupleLikeGetter;

template <
    class Self,
    class BluePrint,
    template <class, class...>
    class Trait,
    class Enable = void>
struct crust_ebco Derive {};
} // namespace _impl_derive

template <class Self, class BluePrint, template <class...> class... Traits>
struct crust_ebco Derive;

template <
    class Self,
    class BluePrint,
    template <class, class...>
    class Trait,
    template <class, class...>
    class... Traits>
struct Derive<Self, BluePrint, Trait, Traits...> :
    _impl_derive::Derive<Self, BluePrint, Trait>,
    Derive<Self, BluePrint, Traits...> {};

template <class Self, class BluePrint>
struct Derive<Self, BluePrint> {};

template <class S, class B, class... T>
struct DeriveInfo {
  using Result = DeriveInfo;
  using BluePrint = B;
};

template <class Self>
struct NewDerive;

namespace _impl_derive {
template <class Info>
struct AutoDerive;

template <class Self, class BluePrint, class T, class... Ts>
struct AutoDerive<DeriveInfo<Self, BluePrint, T, Ts...>> :
    ImplFor<T, Self>,
    AutoDerive<DeriveInfo<Self, BluePrint, Ts...>> {};

template <class Self, class BluePrint>
struct AutoDerive<DeriveInfo<Self, BluePrint>> {};
} // namespace _impl_derive

template <class Self>
using AutoDerive = _impl_derive::AutoDerive<typename NewDerive<Self>::Result>;

template <class Struct, template <class, class...> class Trait, class... Args>
struct Require : IsBaseOfVal<Trait<Struct, Args...>, Struct> {};

namespace _impl_utility {
template <bool enable>
struct EnableIf {};

template <>
struct EnableIf<true> : TmplType<void> {};

template <class T, bool Bool, class>
struct InheritIf;

template <class T, class Marker>
struct InheritIf<T, true, Marker> : T {
  CRUST_USE_BASE_CONSTRUCTORS(InheritIf, T);
};

template <class T, class Marker>
struct InheritIf<T, false, Marker> {
  constexpr InheritIf() {}

  constexpr InheritIf(T &&) {}
};
} // namespace _impl_utility

template <class... Enable>
using EnableIf =
    typename _impl_utility::EnableIf<All<Enable...>::result>::Result;

template <class T, class Bools, class Marker = void>
using InheritIf = _impl_utility::InheritIf<T, Bools::result, Marker>;

/// this is used by Tuple, Enum and Slice for zero sized type optimization
/// foreign type can implement ZeroSizedType to be treated as zero sized type.

CRUST_TRAIT(ZeroSizedType) {
  CRUST_TRAIT_USE_SELF(
      ZeroSizedType, IsEmptyType<Self>, IsTriviallyCopyable<Self>);
};

template <class T>
struct RefMut {
private:
  T *ptr;

public:
  constexpr RefMut() : ptr{nullptr} {}

  explicit constexpr RefMut(T &ptr) : ptr{&ptr} {}

  RefMut(const RefMut &other) = default;

  RefMut &operator=(const RefMut &other) = default;

  crust_cxx14_constexpr T &operator*() {
    return crust_debug_assert(ptr != nullptr), *ptr;
  }

  crust_cxx14_constexpr T *operator->() {
    return crust_debug_assert(ptr != nullptr), ptr;
  }
};

template <class T>
struct Ref {
private:
  const T *ptr;

public:
  constexpr Ref() : ptr{nullptr} {}

  explicit constexpr Ref(const T &ptr) : ptr{&ptr} {}

  constexpr Ref(RefMut<T> &&ptr) : ptr{&*ptr} {}

  Ref(const Ref &other) = default;

  Ref &operator=(const Ref &other) = default;

  constexpr const T &operator*() const {
    return crust_debug_assert(ptr != nullptr), *ptr;
  }

  constexpr const T *operator->() const {
    return crust_debug_assert(ptr != nullptr), ptr;
  }
};

template <class T>
constexpr Ref<T> ref(const T &self) {
  return Ref<T>{self};
}


template <class T>
constexpr RefMut<T> ref_mut(T &self) {
  return RefMut<T>{self};
}

namespace _impl_utility {
class Ignore {};

class IgnoreRange {};

class Argument {};
} // namespace _impl_utility

constexpr _impl_utility::Ignore _{};

constexpr _impl_utility::IgnoreRange __{};

constexpr _impl_utility::Argument arg{};
} // namespace crust


#endif // CRUST_UTILITY_HPP

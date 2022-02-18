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

#define crust_static_assert(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

#if defined(_MSC_VER)
#define crust_ebco __declspec(empty_bases)
#else
#define crust_ebco
#endif

#if defined(__GNUC__) || defined(__clang__)
#define no_return __attribute__((noreturn))
#elif defined(_MSC_VER)
#define no_return __declspec(noreturn)
#else
#define no_return
#endif

#if defined(__GNUC__) || defined(__clang__)
#define crust_likely(x) __builtin_expect(!!(x), 1)
#define crust_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define crust_likely(x) x
#define crust_unlikely(x) x
#endif

#if defined(__GNUC__) || defined(__clang__)
#define always_inline inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define always_inline __forceinline
#else
#define always_inline inline
#endif

namespace _impl_utility {
no_return inline void
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

template <class T, template <class> class... Tmpls>
struct CompositionType;

template <class T, template <class> class Tmpl, template <class> class... Tmpls>
struct CompositionType<T, Tmpl, Tmpls...> :
    CompositionType<typename Tmpl<T>::Result, Tmpls...> {};

template <class T>
struct CompositionType<T> : TmplType<T> {};

template <class A, class B>
struct IsSameVal : BoolVal<false> {};

template <class A>
struct IsSameVal<A, A> : BoolVal<true> {};

template <class A, class B>
struct LTVal : BoolVal<(A::result < B::result)> {
  crust_static_assert(
      IsSameVal<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct LEVal : BoolVal<(A::result <= B::result)> {
  crust_static_assert(
      IsSameVal<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct GTVal : BoolVal<(A::result > B::result)> {
  crust_static_assert(
      IsSameVal<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct GEVal : BoolVal<(A::result >= B::result)> {
  crust_static_assert(
      IsSameVal<typename A::Result, typename B::Result>::result);
};

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
struct IsBaseOfVal : BoolVal<std::is_base_of<B, T>::value> {};

template <class T>
struct IsTriviallyCopyable : BoolVal<std::is_trivially_copyable<T>::value> {};

namespace _impl_utility {
template <bool enable>
struct EnableIf {};

template <>
struct EnableIf<true> : TmplType<void> {};

template <class T, bool Bool>
struct InheritIf;

template <class T>
struct InheritIf<T, true> : T {};

template <class T>
struct InheritIf<T, false> {};
} // namespace _impl_utility

template <class... Enable>
using EnableIf =
    typename _impl_utility::EnableIf<All<Enable...>::result>::Result;

template <class T, class... Bools>
using InheritIf = _impl_utility::InheritIf<T, All<Bools...>::result>;

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
protected:                                                                     \
  constexpr TRAIT() {                                                          \
    crust_static_assert(::crust::IsBaseOfVal<TRAIT, Self>::result);            \
    crust_static_assert(::crust::All<__VA_ARGS__>::result);                    \
  }                                                                            \
  constexpr const Self &self() const {                                         \
    return *static_cast<const Self *>(this);                                   \
  }                                                                            \
  crust_cxx14_constexpr Self &self() { return *static_cast<Self *>(this); }    \
                                                                               \
public:

template <template <class, class...> class T, class... Args>
struct Trait {
  template <class Self>
  using Result = T<Self, Args...>;
};

template <class Trait, class Self, class Enable = void>
struct ImplFor {};

#define CRUST_IMPL_FOR(TRAIT, SELF, ...)                                       \
  struct ImplFor<Trait<TRAIT>, SELF, EnableIf<__VA_ARGS__>> :                  \
      Trait<TRAIT>::Result<SELF>

#define CRUST_IMPL_USE_SELF(SELF, ...)                                         \
  using Self = SELF;                                                           \
                                                                               \
protected:                                                                     \
  constexpr ImplFor() {                                                        \
    crust_static_assert(::crust::IsBaseOfVal<ImplFor, Self>::result);          \
    crust_static_assert(::crust::All<__VA_ARGS__>::result);                    \
  }                                                                            \
  constexpr const Self &self() const {                                         \
    return *static_cast<const Self *>(this);                                   \
  }                                                                            \
  crust_cxx14_constexpr Self &self() { return *static_cast<Self *>(this); }    \
                                                                               \
public:

template <class Self, class... Traits>
struct crust_ebco Impl;

template <class Self, class Trait, class... Traits>
struct Impl<Self, Trait, Traits...> :
    ImplFor<Trait, Self>,
    Impl<Self, Traits...> {};

template <class Self>
struct Impl<Self> {};

namespace _auto_impl {
template <class Self>
struct TupleLikeSize;

template <class Self, usize index>
struct TupleLikeGetter;

template <
    class Self,
    class Base,
    template <class...>
    class Trait,
    class Enable = void>
struct crust_ebco AutoImpl {};
} // namespace _auto_impl

template <class Self, class Base, template <class...> class... Traits>
struct crust_ebco AutoImpl;

template <
    class Self,
    class Base,
    template <class, class...>
    class Trait,
    template <class, class...>
    class... Traits>
struct AutoImpl<Self, Base, Trait, Traits...> :
    _auto_impl::AutoImpl<Self, Base, Trait>,
    AutoImpl<Self, Base, Traits...> {};

template <class Self, class Base>
struct AutoImpl<Self, Base> {};

template <
    class Struct,
    template <class Self, class... Args>
    class Trait,
    class... Args>
struct Derive : IsBaseOfVal<Trait<Struct, Args...>, Struct> {};

/// this is used by Tuple, Enum and Slice for zero sized type optimization
/// forign type can implement ZeroSizedType to be treated as zero sized type.

CRUST_TRAIT(ZeroSizedType) {
  CRUST_TRAIT_USE_SELF(
      ZeroSizedType, IsEmptyType<Self>, IsTriviallyCopyable<Self>);
};

struct MonoStateType {};

namespace _auto_impl {
template <class Self>
struct AutoImpl<Self, MonoStateType, ZeroSizedType> : ZeroSizedType<Self> {};
} // namespace _auto_impl
} // namespace crust


#endif // CRUST_UTILITY_HPP

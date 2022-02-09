#ifndef _CRUST_INCLUDE_UTILITY_HPP
#define _CRUST_INCLUDE_UTILITY_HPP


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

#if defined(__GNUC__) || defined(__clang__)
#define crust_unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define crust_unreachable() __assume(false)
#else
#define crust_unreachable()
#endif

#if defined(_MSC_VER)
#define crust_ebco __declspec(empty_bases)
#else
#define crust_ebco
#endif

#if INTPTR_MAX == INT32_MAX
#define CRUST_BIT_WIDTH 32
#elif INTPTR_MAX == INT64_MAX
#define CRUST_BIT_WIDTH 64
#else
#error "Unsupported bit width."
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
inline void warn(const char *file, int line, const char *msg) {
  fprintf(stderr, "Warn at file %s, line %d: %s\n", file, line, msg);
}

#define crust_warn(msg) ::crust::_impl_utility::warn(__FILE__, __LINE__, msg)

no_return inline void
panic(const char *file, int line, const char *msg) noexcept {
  fprintf(stderr, "Abort at file %s, line %d: %s\n", file, line, msg);
  exit(1);
}

#define crust_panic(msg)                                                       \
  ([](const char *f, int l, const char *m) {                                   \
    fprintf(stderr, "Panic at file %s, line %d: %s\n", f, l, m);               \
    exit(-1);                                                                  \
  }(__FILE__, __LINE__, msg))

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
} // namespace _impl_utility

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
#if CRUST_BIT_WIDTH == 32
using isize = i32;
using usize = u32;
#elif CRUST_BIT_WIDTH == 64
using isize = i64;
using usize = u64;
#else
#error "Unsupported bit width."
#endif

template <class Value>
struct TmplArg {
  using Result = Value;
};

template <class Type, Type value>
struct StaticTmplArg {
  static constexpr Type result = value;
};

#define crust_tmpl_arg(obj)                                                    \
  ::crust::StaticTmplArg<decltype(obj), (obj)> {}

template <bool enable>
struct EnableIf {};

template <>
struct EnableIf<true> : TmplArg<void> {};

template <bool value>
struct BoolType : StaticTmplArg<bool, value> {};

using TrueType = BoolType<true>;
using FalseType = BoolType<false>;

template <class Condition>
struct NotType : BoolType<!Condition::result> {};

template <class... Conditions>
struct AllType;

template <class Condition, class... Conditions>
struct AllType<Condition, Conditions...> :
    BoolType<Condition::result && AllType<Conditions...>::result> {};

template <>
struct AllType<> : TrueType {};

template <class... Conditions>
struct AnyType;

template <class Condition, class... Conditions>
struct AnyType<Condition, Conditions...> :
    BoolType<Condition::result || AnyType<Conditions...>::result> {};

template <>
struct AnyType<> : FalseType {};

template <class T, template <class> class... Tmpls>
struct CompositionTmpl;

template <class T, template <class> class Tmpl, template <class> class... Tmpls>
struct CompositionTmpl<T, Tmpl, Tmpls...> :
    CompositionTmpl<typename Tmpl<T>::Result, Tmpls...> {};

template <class T>
struct CompositionTmpl<T> : TmplArg<T> {};

template <class A, class B>
struct IsSame : FalseType {};

template <class A>
struct IsSame<A, A> : TrueType {};

template <class T>
struct RemoveRef : TmplArg<T> {};

template <class T>
struct RemoveRef<T &> : TmplArg<T> {};

template <class T>
struct RemoveRef<T &&> : TmplArg<T> {};

template <class T>
struct RemoveConst : TmplArg<T> {};

template <class T>
struct RemoveConst<T const> : TmplArg<T> {};

template <class T>
struct RemoveConstOrRef : CompositionTmpl<T, RemoveRef, RemoveConst> {};

template <class T>
struct IsLValueRef : FalseType {};

template <class T>
struct IsLValueRef<T &> : TrueType {};

template <class T>
struct IsRValueRef : FalseType {};

template <class T>
struct IsRValueRef<T &&> : TrueType {};

template <class T>
struct IsConst : FalseType {};

template <class T>
struct IsConst<const T> : TrueType {};

template <class T>
struct IsRef : AnyType<IsLValueRef<T>, IsRValueRef<T>> {};

template <class T>
struct IsConstOrRef : AnyType<IsRef<T>, IsConst<T>> {};

template <class B, class T>
struct IsBaseOfType : BoolType<std::is_base_of<B, T>::value> {};

template <class T>
struct IsTriviallyCopyableType :
    BoolType<std::is_trivially_copyable<T>::value> {};

template <class T>
constexpr typename RemoveRef<T>::Result &&move(T &&t) {
  return static_cast<typename RemoveRef<T>::Result &&>(t);
}

template <class T>
constexpr T &&forward(typename RemoveRef<T>::Result &t) {
  return static_cast<T &&>(t);
}

template <class T>
constexpr T &&forward(typename RemoveRef<T>::Result &&t) {
  crust_static_assert(!IsLValueRef<T>::result);
  return static_cast<T &&>(t);
}

namespace _impl_utility {
template <class T, bool condition>
struct Impl;

template <class T>
struct Impl<T, true> : T {};

template <class T>
struct Impl<T, false> {};
} // namespace _impl_utility

template <class T, class... Conditions>
using Impl = _impl_utility::Impl<T, AllType<Conditions...>::result>;

template <
    class Struct,
    template <class Self, class... Args>
    class Trait,
    class... Args>
struct Derive : IsBaseOfType<Trait<Struct, Args...>, Struct> {};

/// this is used by Tuple, Enum and Slice for zero sized type optimization
/// forign type can inherit ZeroSizedType to be treated as mono state.

struct ZeroSizedType {};

template <class T>
struct IsZeroSizedType : IsBaseOfType<ZeroSizedType, T> {};

#define CRUST_TRAIT(TRAIT, ...)                                                \
  template <class Self, ##__VA_ARGS__>                                         \
  struct TRAIT

#define CRUST_TRAIT_REQUIRE(TRAIT, ...)                                        \
protected:                                                                     \
  constexpr TRAIT() {                                                          \
    crust_static_assert(::crust::IsBaseOfType<TRAIT, Self>::result);           \
    crust_static_assert(::crust::AllType<__VA_ARGS__>::result);                \
  }                                                                            \
  constexpr const Self &self() const {                                         \
    return *static_cast<const Self *>(this);                                   \
  }                                                                            \
  crust_cxx14_constexpr Self &self() { return *static_cast<Self *>(this); }    \
                                                                               \
public:

#define CRUST_USE_BASE_CONSTRUCTORS(NAME, ...)                                 \
  template <class... Args>                                                     \
  explicit constexpr NAME(Args &&...args) :                                    \
      __VA_ARGS__{::crust::forward<Args>(args)...} {}

#define CRUST_MACRO_REPEAT_1(FN) FN(0)
#define CRUST_MACRO_REPEAT_2(FN)                                               \
  CRUST_MACRO_REPEAT_1(FN);                                                    \
  FN(1)
#define CRUST_MACRO_REPEAT_3(FN)                                               \
  CRUST_MACRO_REPEAT_2(FN);                                                    \
  FN(2)
#define CRUST_MACRO_REPEAT_4(FN)                                               \
  CRUST_MACRO_REPEAT_3(FN);                                                    \
  FN(3)
#define CRUST_MACRO_REPEAT_5(FN)                                               \
  CRUST_MACRO_REPEAT_4(FN);                                                    \
  FN(4)
#define CRUST_MACRO_REPEAT_6(FN)                                               \
  CRUST_MACRO_REPEAT_5(FN);                                                    \
  FN(5)
#define CRUST_MACRO_REPEAT_7(FN)                                               \
  CRUST_MACRO_REPEAT_6(FN);                                                    \
  FN(6)
#define CRUST_MACRO_REPEAT_8(FN)                                               \
  CRUST_MACRO_REPEAT_7(FN);                                                    \
  FN(7)
#define CRUST_MACRO_REPEAT_9(FN)                                               \
  CRUST_MACRO_REPEAT_8(FN);                                                    \
  FN(8)
#define CRUST_MACRO_REPEAT_10(FN)                                              \
  CRUST_MACRO_REPEAT_9(FN);                                                    \
  FN(9)
#define CRUST_MACRO_REPEAT_11(FN)                                              \
  CRUST_MACRO_REPEAT_10(FN);                                                   \
  FN(10)
#define CRUST_MACRO_REPEAT_12(FN)                                              \
  CRUST_MACRO_REPEAT_11(FN);                                                   \
  FN(11)
#define CRUST_MACRO_REPEAT_13(FN)                                              \
  CRUST_MACRO_REPEAT_12(FN);                                                   \
  FN(12)
#define CRUST_MACRO_REPEAT_14(FN)                                              \
  CRUST_MACRO_REPEAT_13(FN);                                                   \
  FN(13)
#define CRUST_MACRO_REPEAT_15(FN)                                              \
  CRUST_MACRO_REPEAT_14(FN);                                                   \
  FN(14)
#define CRUST_MACRO_REPEAT_16(FN)                                              \
  CRUST_MACRO_REPEAT_15(FN);                                                   \
  FN(15)
#define CRUST_MACRO_REPEAT(N, FN) CRUST_MACRO_REPEAT_##N(FN)
} // namespace crust


#endif //_CRUST_INCLUDE_UTILITY_HPP

#ifndef _CRUST_INCLUDE_UTILITY_HPP
#define _CRUST_INCLUDE_UTILITY_HPP


#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <type_traits>


namespace crust {
#if __cplusplus > 201103L
#define crust_cxx14_constexpr constexpr
#else
#define crust_cxx14_constexpr
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


[[noreturn]] static inline void
_panic(const char *file, int line, const char *msg) {
  fprintf(stderr, "Abort at file %s, line %d: %s\n", file, line, msg);

  abort();
}

#define crust_panic(msg) ::crust::_panic(__FILE__, __LINE__, msg)

static inline crust_cxx14_constexpr void
_assert(const char *file, int line, const char *msg, bool condition) {
  if (!condition) { _panic(file, line, msg); }
}

#define crust_assert(...) \
    ::crust::_assert(__FILE__, __LINE__, #__VA_ARGS__, __VA_ARGS__)

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

template<bool enable>
struct EnableIf {};

template<>
struct EnableIf<true> {
    using Result = void;
};

struct TrueType {
  static constexpr bool result = true;
};

struct FalseType {
  static constexpr bool result = false;
};

template<class Condition>
struct NotType {
  static constexpr bool result = !Condition::result;
};

template<class ...Conditions>
struct AllType;

template<class Condition, class ...Conditions>
struct AllType<Condition, Conditions...> {
  static constexpr bool result = Condition::result &&
      AllType<Conditions...>::result;
};

template<>
struct AllType<> : public TrueType {};

template<class ...Conditions>
struct AnyType;

template<class Condition, class ...Conditions>
struct AnyType<Condition, Conditions...> {
  static constexpr bool result = Condition::result ||
      AnyType<Conditions...>::result;
};

template<>
struct AnyType<> : public FalseType {};

template<class A, class B>
struct IsSame : public FalseType {};

template<class A>
struct IsSame<A, A> : public TrueType {};

template<class T>
struct RemoveRef {
  using Result = T;
};

template<class T>
struct RemoveRef<T &> {
  using Result = T;
};

template<class T>
struct RemoveRef<T &&> {
  using Result = T;
};

template<class T>
struct RemoveConst {
  using Result = T;
};

template<class T>
struct RemoveConst<const T> {
  using Result = T;
};

template<class T>
struct RemoveConstOrRef {
  using Result = typename RemoveRef<typename RemoveConst<T>::Result>::Result;
};

template<class T>
struct IsLValueRef : public FalseType {};

template<class T>
struct IsLValueRef<T &> : public TrueType {};

template<class T>
struct IsRValueRef : public FalseType {};

template<class T>
struct IsRValueRef<T &&> : public TrueType {};

template<class T>
struct IsConst : public FalseType {};

template<class T>
struct IsConst<const T> : public TrueType {};

template<class T>
struct IsRef : public AnyType<IsLValueRef<T>, IsRValueRef<T>> {};

template<class T>
struct IsConstOrRef : public AnyType<IsRef<T>, IsConst<T>> {};

template<class B, class T>
struct IsBaseOfType {
  static constexpr bool result = std::is_base_of<B, T>::value;
};

template<class T>
struct IsTriviallyCopyableType {
  static constexpr bool result = std::is_trivially_copyable<T>::value;
};

template<class T>
constexpr typename RemoveRef<T>::Result &&move(T &&t) {
  return static_cast<typename RemoveRef<T>::Result &&>(t);
}

template<class T>
constexpr T &&forward(typename RemoveRef<T>::Result &t) {
  return static_cast<T &&>(t);
}

template<class T>
constexpr T &&forward(typename RemoveRef<T>::Result &&t) {
  crust_static_assert(!IsLValueRef<T>::result);
  return static_cast<T &&>(t);
}

namespace _impl_impl {
template<class T, bool condition>
struct Impl;

template<class T>
struct crust_ebco Impl<T, true> : public T {};

template<class T>
struct Impl<T, false> {};
}

template<class T, class ...Conditions>
using Impl = _impl_impl::Impl<T, AllType<Conditions...>::result>;

template<
    class Struct,
    template<class Self, class ...Args> class Trait,
    class ...Args
>
struct Derive : public IsBaseOfType<Trait<Struct, Args...>, Struct> {};

/// this is used by Tuple, Enum and Slice for zero sized type optimization
/// forign type can inherit ZeroSizedType to be treated as mono state.

struct ZeroSizedType {};

template<class T>
struct IsZeroSizedType : public IsBaseOfType<ZeroSizedType, T> {};

#define CRUST_TRAIT(TRAIT, ...) \
  template<class Self, ##__VA_ARGS__> \
  struct TRAIT

#define CRUST_TRAIT_REQUIRE(TRAIT, ...) \
  protected: \
    constexpr TRAIT() { \
      crust_static_assert(::crust::IsBaseOfType<TRAIT, Self>::result); \
      crust_static_assert(::crust::AllType<__VA_ARGS__>::result); \
    } \
    constexpr const Self &self() const { \
      return *static_cast<const Self *>(this); \
    } \
    crust_cxx14_constexpr Self &self() { \
      return *static_cast<Self *>(this); \
    } \
  public:

#define CRUST_USE_BASE_CONSTRUCTORS(NAME, ...) \
  template<class ...Args> \
  explicit constexpr NAME(Args &&...args) : \
      __VA_ARGS__{::crust::forward<Args>(args)...} \
  {}

#define CRUST_MACRO_REPEAT_1(FN) FN(0)
#define CRUST_MACRO_REPEAT_2(FN) CRUST_MACRO_REPEAT_1(FN); FN(1)
#define CRUST_MACRO_REPEAT_3(FN) CRUST_MACRO_REPEAT_2(FN); FN(2)
#define CRUST_MACRO_REPEAT_4(FN) CRUST_MACRO_REPEAT_3(FN); FN(3)
#define CRUST_MACRO_REPEAT_5(FN) CRUST_MACRO_REPEAT_4(FN); FN(4)
#define CRUST_MACRO_REPEAT_6(FN) CRUST_MACRO_REPEAT_5(FN); FN(5)
#define CRUST_MACRO_REPEAT_7(FN) CRUST_MACRO_REPEAT_6(FN); FN(6)
#define CRUST_MACRO_REPEAT_8(FN) CRUST_MACRO_REPEAT_7(FN); FN(7)
#define CRUST_MACRO_REPEAT_9(FN) CRUST_MACRO_REPEAT_8(FN); FN(8)
#define CRUST_MACRO_REPEAT_10(FN) CRUST_MACRO_REPEAT_9(FN); FN(9)
#define CRUST_MACRO_REPEAT_11(FN) CRUST_MACRO_REPEAT_10(FN); FN(10)
#define CRUST_MACRO_REPEAT_12(FN) CRUST_MACRO_REPEAT_11(FN); FN(11)
#define CRUST_MACRO_REPEAT_13(FN) CRUST_MACRO_REPEAT_12(FN); FN(12)
#define CRUST_MACRO_REPEAT_14(FN) CRUST_MACRO_REPEAT_13(FN); FN(13)
#define CRUST_MACRO_REPEAT_15(FN) CRUST_MACRO_REPEAT_14(FN); FN(14)
#define CRUST_MACRO_REPEAT_16(FN) CRUST_MACRO_REPEAT_15(FN); FN(15)
#define CRUST_MACRO_REPEAT(N, FN) CRUST_MACRO_REPEAT_##N(FN)

template<class Type, Type obj>
struct TmplArg {};

#define crust_tmpl_arg(obj) ::crust::TmplArg<decltype(obj), (obj)>{}
}


#endif //_CRUST_INCLUDE_UTILITY_HPP

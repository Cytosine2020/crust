#ifndef CRUST_NUM_MOD_HPP
#define CRUST_NUM_MOD_HPP


#include "crust/utility.hpp"


namespace crust {
namespace num {
namespace _impl_num {
template <class Type, Type min, Type max>
struct Int {
  static constexpr Type BITS = static_cast<Type>(8 * sizeof(Type));
  static constexpr Type MIN = min;
  static constexpr Type MAX = max;
};
} // namespace _impl_num

template <class Type>
struct Int;

template <>
struct Int<u8> : _impl_num::Int<u8, 0, 0xFF> {};

template <>
struct Int<u16> : _impl_num::Int<u16, 0, 0xFFFF> {};

template <>
struct Int<u32> : _impl_num::Int<u32, 0, 0xFFFFFFFF> {};

template <>
struct Int<u64> : _impl_num::Int<u64, 0, 0xFFFFFFFFFFFFFFFFULL> {};

template <>
struct Int<i8> : _impl_num::Int<i8, -0x80, 0x7F> {};

template <>
struct Int<i16> : _impl_num::Int<i16, -0x8000, 0x7FFF> {};

template <>
struct Int<i32> : _impl_num::Int<i32, -0x80000000LL, 0x7FFFFFFF> {};

template <>
struct Int<i64> :
    _impl_num::Int<
        i64,
        static_cast<i64>(0x8000000000000000ULL),
        0x7FFFFFFFFFFFFFFFLL> {};
} // namespace num

template <class A, class B>
struct AddVal : TmplVal<typename A::Result, A::result + B::result> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
  crust_static_assert(
      A::result <= num::Int<typename A::Result>::MAX - B::result);
};

template <class T, class... Vs>
struct SumVal;

template <class T, class V, class... Vs>
struct SumVal<T, V, Vs...> : AddVal<V, SumVal<T, Vs...>> {};

template <class T>
struct SumVal<T> : TmplVal<T, 0> {};

template <class A, class B>
struct SubVal : TmplVal<typename A::Result, A::result - B::result> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
  crust_static_assert(
      A::result >= num::Int<typename A::Result>::MIN + B::result);
};

template <class T>
struct IncVal : AddVal<T, TmplVal<typename T::Result, 1>> {};

template <class T>
struct DecVal : SubVal<T, TmplVal<typename T::Result, 1>> {};

template <class A, class B>
struct EQVal : BoolVal<(A::result == B::result)> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct NEVal : BoolVal<(A::result != B::result)> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct LTVal : BoolVal<(A::result < B::result)> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct LEVal : BoolVal<(A::result <= B::result)> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct GTVal : BoolVal<(A::result > B::result)> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
};

template <class A, class B>
struct GEVal : BoolVal<(A::result >= B::result)> {
  crust_static_assert(IsSame<typename A::Result, typename B::Result>::result);
};
} // namespace crust


#endif // CRUST_NUM_MOD_HPP

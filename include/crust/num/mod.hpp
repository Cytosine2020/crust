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

template <class T>
struct IncVal : TmplVal<typename T::Result, T::result + 1> {
  crust_static_assert(T::result != num::Int<typename T::Result>::MAX);
};

template <class T>
struct DecVal : TmplVal<typename T::Result, T::result - 1> {
  crust_static_assert(T::result != num::Int<typename T::Result>::MIN);
};
} // namespace crust


#endif // CRUST_NUM_MOD_HPP

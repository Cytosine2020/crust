#ifndef CRUST_HELPER_TYPES_HPP
#define CRUST_HELPER_TYPES_HPP


#include "crust/num/mod.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_types {
template <class... Fields>
struct Types : TmplVal<usize, sizeof...(Fields)> {};

template <class T, class Ts>
struct TypesPushFront;

template <class T, class... Fields>
struct TypesPushFront<T, Types<Fields...>> : TmplType<Types<T, Fields...>> {};

template <usize index, class Ts>
struct TypesIndex;

template <usize index, class Field, class... Fields>
struct TypesIndex<index, Types<Field, Fields...>> :
    TypesIndex<index - 1, Types<Fields...>> {};

template <class Field, class... Fields>
struct TypesIndex<0, Types<Field, Fields...>> : TmplType<Field> {};

template <usize index, class Ts>
struct TypesAfter;

template <usize index, class Field, class... Fields>
struct TypesAfter<index, Types<Field, Fields...>> :
    TypesAfter<index - 1, Types<Fields...>> {};

template <class Field, class... Fields>
struct TypesAfter<0, Types<Field, Fields...>> : TmplType<Types<Fields...>> {};

template <usize index, class Ts>
struct TypesBefore;

template <usize index, class Field, class... Fields>
struct TypesBefore<index, Types<Field, Fields...>> :
    TypesPushFront<
        Field,
        typename TypesBefore<index - 1, Types<Fields...>>::Result> {};

template <class Field, class... Fields>
struct TypesBefore<0, Types<Field, Fields...>> : TmplType<Types<>> {};

template <class T, class Ts>
struct TypesCountType;

template <class T, class... Fields>
struct TypesCountType<T, Types<Fields...>> :
    SumVal<usize, AsVal<usize, IsSame<T, Fields>>...> {};

template <class T, class Ts>
struct TypesIncludeVal : GTVal<TypesCountType<T, Ts>, TmplVal<usize, 0>> {};

template <class Ts>
struct TypesDuplicateVal;

template <class Field, class... Fields>
struct TypesDuplicateVal<Types<Field, Fields...>> :
    Any<TypesIncludeVal<Field, Types<Fields...>>,
        TypesDuplicateVal<Types<Fields...>>> {};

template <>
struct TypesDuplicateVal<Types<>> : BoolVal<false> {};

template <class T, class Ts>
struct TypesFirstIndex;

template <class T, class Field, class... Fields>
struct TypesFirstIndex<T, Types<Field, Fields...>> :
    IncVal<TypesFirstIndex<T, Types<Fields...>>> {};

template <class T, class... Fields>
struct TypesFirstIndex<T, Types<T, Fields...>> : TmplVal<usize, 0> {};

template <class T, class... Fields>
struct TypesPrevType :
    TypesIndex<
        DecVal<TypesFirstIndex<T, Types<Fields...>>>::result,
        Types<Fields...>> {};

template <class T, class U>
struct TypesContainHelper : BoolVal<false> {};

template <class T, class Ts>
struct TypesContainVal;

template <class T, class... Fields>
struct TypesContainVal<T, Types<Fields...>> :
    Any<TypesContainHelper<T, Fields>...> {};

template <usize index, class Ts>
using ZeroSizedTypeHolderFieldEnable =
    All<Require<typename TypesIndex<index, Ts>::Result, ZeroSizedType>,
        Not<TypesIncludeVal<
            typename TypesIndex<index, Ts>::Result,
            typename TypesBefore<index, Ts>::Result>>,
        Not<TypesContainVal<typename TypesIndex<index, Ts>::Result, Ts>>>;

template <usize index, class Ts>
using ZeroSizedTypeHolderField = InheritIf<
    typename TypesIndex<index, Ts>::Result,
    ZeroSizedTypeHolderFieldEnable<index, Ts>,
    TypesIndex<index, Ts>>;

template <usize remain, class Ts>
struct crust_ebco ZeroSizedTypeHolderImpl :
    ZeroSizedTypeHolderField<Ts::result - remain, Ts>,
    ZeroSizedTypeHolderImpl<remain - 1, Ts> {
  constexpr ZeroSizedTypeHolderImpl() {}
};

template <class Ts>
struct ZeroSizedTypeHolderImpl<0, Ts> {};

template <class... Fields>
using ZeroSizedTypeHolder =
    ZeroSizedTypeHolderImpl<sizeof...(Fields), Types<Fields...>>;

template <usize index, class... Fields>
struct ZeroSizedTypeGetter {
  using Self = ZeroSizedTypeHolder<Fields...>;
  using Result = typename TypesIndex<index, Types<Fields...>>::Result;
  using Field = ZeroSizedTypeHolderField<
      TypesFirstIndex<Result, Types<Fields...>>::result,
      Types<Fields...>>;

  static constexpr const Result &inner(const Self &self) {
    return static_cast<const Field &>(self);
  }

  static constexpr Result &inner(Self &self) {
    return static_cast<Field &>(self);
  }
};
} // namespace _impl_types
} // namespace crust


#endif // CRUST_HELPER_TYPES_HPP

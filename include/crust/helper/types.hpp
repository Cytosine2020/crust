#ifndef CRUST_HELPER_TYPES_HPP
#define CRUST_HELPER_TYPES_HPP


#include "crust/num/mod.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_types {
template <class... Fields>
struct Types {};

template <class T, class Ts>
struct TypesCountType;

template <class T, class Field, class... Fields>
struct TypesCountType<T, Types<Field, Fields...>> :
    AddVal<
        TypesCountType<T, Types<Fields...>>,
        AsVal<usize, IsSame<T, Field>>> {};

template <class T>
struct TypesCountType<T, Types<>> : TmplVal<usize, 0> {};

template <class T, class Ts>
struct TypesIncludeVal : GTVal<TypesCountType<T, Ts>, TmplVal<usize, 0>> {};

template <class T, class Ts>
struct TypesContainVal;

template <class T, class Field, class... Fields>
struct TypesContainVal<T, Types<Field, Fields...>> :
    Any<IsBaseOfVal<T, Field>, TypesContainVal<T, Types<Fields...>>> {};

template <class T>
struct TypesContainVal<T, Types<>> : BoolVal<false> {};

template <class Ts>
struct TypesDuplicateVal;

template <class Field, class... Fields>
struct TypesDuplicateVal<Types<Field, Fields...>> :
    Any<TypesIncludeVal<Field, Types<Fields...>>,
        TypesDuplicateVal<Types<Fields...>>> {};

template <>
struct TypesDuplicateVal<Types<>> : BoolVal<false> {};

template <usize index, class Ts>
struct TypesIndexToType;

template <usize index, class Field, class... Fields>
struct TypesIndexToType<index, Types<Field, Fields...>> :
    TypesIndexToType<index - 1, Types<Fields...>> {};

template <class Field, class... Fields>
struct TypesIndexToType<0, Types<Field, Fields...>> : TmplType<Field> {};

template <class T, class Ts>
struct TypesTypeToIndex;

template <class T, class Field, class... Fields>
struct TypesTypeToIndex<T, Types<Field, Fields...>> :
    IncVal<TypesTypeToIndex<T, Types<Fields...>>> {};

template <class T, class... Fields>
struct TypesTypeToIndex<T, Types<T, Fields...>> : TmplVal<usize, 0> {};

template <class T, class... Fields>
struct TypesPrevType :
    TypesIndexToType<
        DecVal<TypesTypeToIndex<T, Types<Fields...>>>::result,
        Types<Fields...>> {};

template <class Ts, bool is_zst, class... Fields>
struct crust_ebco ZeroSizedTypeHolderImpl;

template <class Ts, class... Fields>
struct crust_ebco ZeroSizedTypeHolderHelper;

template <class Ts, class Field, class... Fields>
struct ZeroSizedTypeHolderImpl<Ts, true, Field, Fields...> :
    Field,
    ZeroSizedTypeHolderHelper<Ts, Fields...> {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&field) :
      Field{forward<Field>(field)} {}

  template <class T>
  constexpr ZeroSizedTypeHolderImpl(T &&t) :
      ZeroSizedTypeHolderHelper<Ts, Fields...>{forward<T>(t)} {}

  constexpr ZeroSizedTypeHolderImpl(Field &&field, Fields &&...fields) :
      Field{forward<Field>(field)}, ZeroSizedTypeHolderHelper<Ts, Fields...>{
                                        forward<Fields>(fields)...} {}
};

template <class Ts, class Field>
struct ZeroSizedTypeHolderImpl<Ts, true, Field> : Field {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&field) :
      Field{forward<Field>(field)} {}
};

template <class Ts, class Field, class... Fields>
struct ZeroSizedTypeHolderImpl<Ts, false, Field, Fields...> :
    ZeroSizedTypeHolderHelper<Ts, Fields...> {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&) {}

  template <class T>
  constexpr ZeroSizedTypeHolderImpl(T &&t) :
      ZeroSizedTypeHolderHelper<Ts, Fields...>{forward<T>(t)} {}

  constexpr ZeroSizedTypeHolderImpl(Field &&, Fields &&...fields) :
      ZeroSizedTypeHolderHelper<Ts, Fields...>{forward<Fields>(fields)...} {}
};

template <class Ts, class Field>
struct ZeroSizedTypeHolderImpl<Ts, false, Field> {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&) {}
};

template <class Ts, class Field, class... Fields>
struct ZeroSizedTypeHolderHelper<Ts, Field, Fields...> :
    ZeroSizedTypeHolderImpl<
        Ts,
        All<Require<Field, ZeroSizedType>,
            Not<TypesIncludeVal<Field, Types<Fields...>>>,
            Not<TypesContainVal<Field, Ts>>>::result,
        Field,
        Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      ZeroSizedTypeHolderHelper,
      ZeroSizedTypeHolderImpl<
          Ts,
          All<Require<Field, ZeroSizedType>,
              Not<TypesIncludeVal<Field, Types<Fields...>>>,
              Not<TypesContainVal<Field, Ts>>>::result,
          Field,
          Fields...>);
};

template <class... Fields>
struct ZeroSizedTypeHolder :
    ZeroSizedTypeHolderHelper<Types<Fields...>, Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      ZeroSizedTypeHolder,
      ZeroSizedTypeHolderHelper<Types<Fields...>, Fields...>);
};

template <usize index, class... Fields>
struct ZeroSizedTypeGetter {
  using Self = ZeroSizedTypeHolder<Fields...>;
  using Result = typename TypesIndexToType<index, Types<Fields...>>::Result;

  static constexpr const Result &inner(const Self &self) { return self; }

  static constexpr Result &inner(Self &self) { return self; }
};
} // namespace _impl_types
} // namespace crust


#endif // CRUST_HELPER_TYPES_HPP

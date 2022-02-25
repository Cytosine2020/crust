#ifndef CRUST_HELPER_TYPES_HPP
#define CRUST_HELPER_TYPES_HPP


#include "crust/num/mod.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_types {
template <class T, class... Fields>
struct TypesCountType;

template <class T, class Field, class... Fields>
struct TypesCountType<T, Field, Fields...> : TypesCountType<T, Fields...> {};

template <class T, class... Fields>
struct TypesCountType<T, T, Fields...> :
    IncVal<TypesCountType<T, Fields...>> {};

template <class T>
struct TypesCountType<T> : TmplVal<usize, 0> {};

template <class T, class... Fields>
struct TypesIncludeVal :
    GTVal<TypesCountType<T, Fields...>, TmplVal<usize, 0>> {};

template <class... Fields>
struct TypesDuplicateVal;

template <class Field, class... Fields>
struct TypesDuplicateVal<Field, Fields...> :
    Any<TypesIncludeVal<Field, Fields...>, TypesDuplicateVal<Fields...>> {};

template <>
struct TypesDuplicateVal<> : BoolVal<false> {};

template <usize index, class... Fields>
struct TypesIndexToType;

template <usize index, class Field, class... Fields>
struct TypesIndexToType<index, Field, Fields...> :
    TypesIndexToType<index - 1, Fields...> {};

template <class Field, class... Fields>
struct TypesIndexToType<0, Field, Fields...> : TmplType<Field> {};

template <class T, class... Fields>
struct TypesTypeToIndex;

template <class T, class Field, class... Fields>
struct TypesTypeToIndex<T, Field, Fields...> :
    IncVal<TypesTypeToIndex<T, Fields...>> {};

template <class T, class... Fields>
struct TypesTypeToIndex<T, T, Fields...> : TmplVal<usize, 0> {};

template <class T, class... Fields>
struct TypesPrevType :
    TypesIndexToType<
        DecVal<TypesTypeToIndex<T, Fields...>>::result,
        Fields...> {};

template <bool is_zst, class... Fields>
struct crust_ebco ZeroSizedTypeHolderImpl;

template <class... Fields>
struct crust_ebco ZeroSizedTypeHolder;

template <class Field, class... Fields>
struct ZeroSizedTypeHolderImpl<true, Field, Fields...> :
    Field,
    ZeroSizedTypeHolder<Fields...> {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&field) :
      Field{forward<Field>(field)} {}

  template <class T>
  constexpr ZeroSizedTypeHolderImpl(T &&t) :
      ZeroSizedTypeHolder<Fields...>{forward<T>(t)} {}

  constexpr ZeroSizedTypeHolderImpl(Field &&field, Fields &&...fields) :
      Field{forward<Field>(field)}, ZeroSizedTypeHolder<Fields...>{
                                        forward<Fields>(fields)...} {}
};

template <class Field>
struct ZeroSizedTypeHolderImpl<true, Field> : Field {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&field) :
      Field{forward<Field>(field)} {}
};

template <class Field, class... Fields>
struct ZeroSizedTypeHolderImpl<false, Field, Fields...> :
    ZeroSizedTypeHolder<Fields...> {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&) {}

  template <class T>
  constexpr ZeroSizedTypeHolderImpl(T &&t) :
      ZeroSizedTypeHolder<Fields...>{forward<T>(t)} {}

  constexpr ZeroSizedTypeHolderImpl(Field &&, Fields &&...fields) :
      ZeroSizedTypeHolder<Fields...>{forward<Fields>(fields)...} {}
};

template <class Field>
struct ZeroSizedTypeHolderImpl<false, Field> {
  constexpr ZeroSizedTypeHolderImpl() {}

  constexpr ZeroSizedTypeHolderImpl(Field &&) {}
};

template <class Field, class... Fields>
struct ZeroSizedTypeHolder<Field, Fields...> :
    ZeroSizedTypeHolderImpl<
        All<Require<Field, ZeroSizedType>,
            Not<TypesIncludeVal<Field, Fields...>>>::result,
        Field,
        Fields...> {
  CRUST_USE_BASE_CONSTRUCTORS(
      ZeroSizedTypeHolder,
      ZeroSizedTypeHolderImpl<
          All<Require<Field, ::crust::ZeroSizedType>,
              Not<TypesIncludeVal<Field, Fields...>>>::result,
          Field,
          Fields...>);
};

template <usize index, class... Fields>
struct ZeroSizedTypeGetter {
  using Self = _impl_types::ZeroSizedTypeHolder<Fields...>;
  using Result =
      typename _impl_types::TypesIndexToType<index, Fields...>::Result;

  static constexpr const Result &inner(const Self &self) { return self; }

  static constexpr Result &inner(Self &self) { return self; }
};
} // namespace _impl_types
} // namespace crust


#endif // CRUST_HELPER_TYPES_HPP

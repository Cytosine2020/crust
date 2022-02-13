#ifndef CRUST_HELPER_TYPES_HPP
#define CRUST_HELPER_TYPES_HPP


#include "crust/utility.hpp"


namespace crust {
namespace _impl_types {
template <class T, class... Fields>
struct TypesIncludeVal;

template <class T, class Field, class... Fields>
struct TypesIncludeVal<T, Field, Fields...> : TypesIncludeVal<T, Fields...> {};

template <class T, class... Fields>
struct TypesIncludeVal<T, T, Fields...> : BoolVal<true> {};

template <class T>
struct TypesIncludeVal<T> : BoolVal<false> {};

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
struct TypesTypeToIndex<T, T, Fields...> : TmplVal<u32, 0> {};

template <bool is_zst, class... Fields>
struct ZeroSizedTypeHolderImpl;

template <class... Fields>
struct ZeroSizedTypeHolder;

template <class Field, class... Fields>
struct crust_ebco ZeroSizedTypeHolderImpl<true, Field, Fields...> :
    Field,
    ZeroSizedTypeHolder<Fields...> {};

template <class Field>
struct crust_ebco ZeroSizedTypeHolderImpl<true, Field> : Field {};

template <class Field, class... Fields>
struct crust_ebco ZeroSizedTypeHolderImpl<false, Field, Fields...> :
    ZeroSizedTypeHolder<Fields...> {};

template <class Field>
struct ZeroSizedTypeHolderImpl<false, Field> {};

template <class Field, class... Fields>
struct crust_ebco ZeroSizedTypeHolder<Field, Fields...> :
    ZeroSizedTypeHolderImpl<
        AllVal<
            Derive<Field, ZeroSizedType>,
            NotVal<_impl_types::TypesIncludeVal<Field, Fields...>>>::result,
        Field,
        Fields...> {};
} // namespace _impl_types
} // namespace crust


#endif // CRUST_HELPER_TYPES_HPP

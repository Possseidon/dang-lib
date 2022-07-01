#pragma once

#include <type_traits>

#include "dang-utils/global.h"
#include "dang-utils/utils.h"

namespace dang::utils {

// --- NullType

struct NullType {};

// --- is_null_type

template <typename T>
struct is_null_type : std::is_same<T, NullType> {};

template <typename T>
inline constexpr auto is_null_type_v = is_null_type<T>::value;

template <typename T>
concept NullTyped = is_null_type_v<T>;

// --- TypeListExhaustion

/// @brief The type list got exhausted from a drop, take or slice.
struct TypeListExhaustion {};

// --- is_type_list_exhaustion

template <typename T>
struct is_type_list_exhaustion : std::is_same<T, TypeListExhaustion> {};

template <typename T>
inline constexpr auto is_type_list_exhaustion_v = is_type_list_exhaustion<T>::value;

template <typename T>
concept ExhaustedTypeList = is_type_list_exhaustion_v<T>;

// --- TypeList

template <typename... TTypes>
struct TypeList;

// --- make_type_list

template <typename... TTypes>
struct make_type_list : std::type_identity<TypeList<TTypes...>> {};

template <typename... TTypes>
using make_type_list_t = TypeList<TTypes...>;

// --- is_type_list

template <typename T>
struct is_type_list : std::false_type {};

template <typename T>
inline constexpr auto is_type_list_v = is_type_list<T>::value;

template <typename... TTypes>
struct is_type_list<TypeList<TTypes...>> : std::true_type {};

template <typename T>
concept AnyTypeList = is_type_list_v<T>;

// --- is_empty_type_list

template <typename T>
struct is_empty_type_list : std::is_same<T, TypeList<>> {};

template <typename T>
inline constexpr auto is_empty_type_list_v = is_empty_type_list<T>::value;

template <typename T>
concept EmptyTypeList = is_empty_type_list_v<T>;

// --- type_list_size

template <AnyTypeList TTypeList>
struct type_list_size;

template <AnyTypeList TTypeList>
inline constexpr auto type_list_size_v = type_list_size<TTypeList>::value;

template <typename... TTypes>
struct type_list_size<TypeList<TTypes...>> : constant<sizeof...(TTypes)> {};

// --- type_list_contains

template <AnyTypeList TTypeList, typename TCheckedType>
struct type_list_contains;

template <AnyTypeList TTypeList, typename TCheckedType>
inline constexpr auto type_list_contains_v = type_list_contains<TTypeList, TCheckedType>::value;

template <typename... TTypes, typename TCheckedType>
struct type_list_contains<TypeList<TTypes...>, TCheckedType>
    : std::disjunction<std::is_same<TCheckedType, TTypes>...> {};

// --- type_list_at

template <AnyTypeList TTypeList, std::size_t v_index>
struct type_list_at;

template <AnyTypeList TTypeList, std::size_t v_index>
using type_list_at_t = typename type_list_at<TTypeList, v_index>::type;

template <std::size_t v_index>
struct type_list_at<TypeList<>, v_index> : std::type_identity<NullType> {};

template <typename TType, typename... TOtherTypes>
struct type_list_at<TypeList<TType, TOtherTypes...>, 0> : std::type_identity<TType> {};

template <typename TType, typename... TOtherTypes, std::size_t v_index>
struct type_list_at<TypeList<TType, TOtherTypes...>, v_index> : type_list_at<TypeList<TOtherTypes...>, v_index - 1> {};

// --- type_list_first

template <AnyTypeList TTypeList>
using type_list_first = type_list_at<TTypeList, 0>;

template <AnyTypeList TTypeList>
using type_list_first_t = typename type_list_first<TTypeList>::type;

// --- type_list_last

template <AnyTypeList TTypeList>
using type_list_last = type_list_at<TTypeList, TTypeList::size - 1>;

template <AnyTypeList TTypeList>
using type_list_last_t = typename type_list_last<TTypeList>::type;

// --- type_list_append

template <AnyTypeList TTypeList, typename... TTypes>
struct type_list_append;

template <AnyTypeList TTypeList, typename... TTypes>
using type_list_append_t = typename type_list_append<TTypeList, TTypes...>::type;

template <typename... TFirstTypes, typename... TSecondTypes>
struct type_list_append<TypeList<TFirstTypes...>, TSecondTypes...>
    : std::type_identity<TypeList<TFirstTypes..., TSecondTypes...>> {};

// --- type_list_prepend

template <AnyTypeList TTypeList, typename... TTypes>
struct type_list_prepend;

template <AnyTypeList TTypeList, typename... TTypes>
using type_list_prepend_t = typename type_list_prepend<TTypeList, TTypes...>::type;

template <typename... TFirstTypes, typename... TSecondTypes>
struct type_list_prepend<TypeList<TFirstTypes...>, TSecondTypes...>
    : std::type_identity<TypeList<TSecondTypes..., TFirstTypes...>> {};

// --- type_list_join

template <AnyTypeList... TTypeLists>
struct type_list_join;

template <AnyTypeList... TTypeLists>
using type_list_join_t = typename type_list_join<TTypeLists...>::type;

template <>
struct type_list_join<> : std::type_identity<TypeList<>> {};

template <AnyTypeList TTypeList>
struct type_list_join<TTypeList> : std::type_identity<TTypeList> {};

template <AnyTypeList TFirstTypeList, typename... TSecondTypes, AnyTypeList... TOtherTypeLists>
struct type_list_join<TFirstTypeList, TypeList<TSecondTypes...>, TOtherTypeLists...>
    : type_list_join<type_list_append_t<TFirstTypeList, TSecondTypes...>, TOtherTypeLists...> {};

// --- type_list_drop

template <AnyTypeList TTypeList, std::size_t v_count>
struct type_list_drop;

template <AnyTypeList TTypeList, std::size_t v_count>
using type_list_drop_t = typename type_list_drop<TTypeList, v_count>::type;

template <typename... TTypes>
struct type_list_drop<TypeList<TTypes...>, 0> : std::type_identity<TypeList<TTypes...>> {};

// clang-format off
template <std::size_t v_count>
requires(v_count > 0)
struct type_list_drop<TypeList<>, v_count> : std::type_identity<TypeListExhaustion> {};

template <typename TFirstType, typename... TOtherTypes, std::size_t v_count>
requires(v_count > 0)
struct type_list_drop<TypeList<TFirstType, TOtherTypes...>, v_count>
    : type_list_drop<TypeList<TOtherTypes...>, v_count - 1> {};
// clang-format on

// --- type_list_take

namespace detail {

template <AnyTypeList TTypeList, std::size_t v_count, typename... TTakenTypes>
struct type_list_take_helper;

template <typename... TTypes, typename... TTakenTypes>
struct type_list_take_helper<TypeList<TTypes...>, 0, TTakenTypes...> : std::type_identity<TypeList<TTakenTypes...>> {};

// clang-format off
template <std::size_t v_count, typename... TTakenTypes>
requires(v_count > 0)
struct type_list_take_helper<TypeList<>, v_count, TTakenTypes...> : std::type_identity<TypeListExhaustion> {};

template <typename TFirstType, typename... TOtherTypes, std::size_t v_count, typename... TTakenTypes>
requires(v_count > 0)
struct type_list_take_helper<TypeList<TFirstType, TOtherTypes...>, v_count, TTakenTypes...>
    : type_list_take_helper<TypeList<TOtherTypes...>, v_count - 1, TTakenTypes..., TFirstType> {};
// clang-format on

} // namespace detail

template <AnyTypeList TTypeList, std::size_t v_count>
struct type_list_take : detail::type_list_take_helper<TTypeList, v_count> {};

template <AnyTypeList TTypeList, std::size_t v_count>
using type_list_take_t = typename type_list_take<TTypeList, v_count>::type;

// --- type_list_slice

template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
struct type_list_slice;

template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
using type_list_slice_t = typename type_list_slice<TTypeList, v_begin, v_end>::type;

// clang-format off
template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
requires(v_begin <= v_end && v_end > TTypeList::size)
struct type_list_slice<TTypeList, v_begin, v_end> : std::type_identity<TypeListExhaustion> {};

template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
requires(v_begin <= v_end && v_end <= TTypeList::size)
struct type_list_slice<TTypeList, v_begin, v_end>
    : type_list_take<type_list_drop_t<TTypeList, v_begin>, v_end - v_begin> {};
// clang-format on

// --- type_list_erase

template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
struct type_list_erase;

// clang-format off
template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
requires(v_begin <= v_end && v_end > TTypeList::size)
struct type_list_erase<TTypeList, v_begin, v_end> : std::type_identity<TypeListExhaustion> {};

template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
requires(v_begin <= v_end && v_end <= TTypeList::size)
struct type_list_erase<TTypeList, v_begin, v_end>
    : type_list_join<type_list_take_t<TTypeList, v_begin>, type_list_drop_t<TTypeList, v_end>> {};
// clang-format on

template <AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
using type_list_erase_t = typename type_list_erase<TTypeList, v_begin, v_end>::type;

// --- type_list_insert

template <AnyTypeList TTypeList, std::size_t v_index, typename... TInsertedTypes>
struct type_list_insert
    : type_list_join<type_list_take_t<TTypeList, v_index>,
                     TypeList<TInsertedTypes...>,
                     type_list_drop_t<TTypeList, v_index>> {};

template <AnyTypeList TTypeList, std::size_t v_index, typename... TInsertedTypes>
using type_list_insert_t = typename type_list_insert<TTypeList, v_index, TInsertedTypes...>::type;

// --- type_list_filter

namespace detail {

template <AnyTypeList TTypeList,
          template <typename...>
          typename TFilter,
          AnyTypeList TParameterListBefore,
          AnyTypeList TParameterListAfter,
          bool v_invert,
          typename... TFilteredTypes>
struct type_list_filter_helper;

template <template <typename...> typename TFilter,
          AnyTypeList TParameterListBefore,
          AnyTypeList TParameterListAfter,
          bool v_invert,
          typename... TFilteredTypes>
struct type_list_filter_helper<TypeList<>,
                               TFilter,
                               TParameterListBefore,
                               TParameterListAfter,
                               v_invert,
                               TFilteredTypes...> : std::type_identity<TypeList<TFilteredTypes...>> {};

template <typename TFirstType,
          typename... TOtherTypes,
          template <typename...>
          typename TFilter,
          typename... TParametersBefore,
          typename... TParametersAfter,
          bool v_invert,
          typename... TFilteredTypes>
struct type_list_filter_helper<TypeList<TFirstType, TOtherTypes...>,
                               TFilter,
                               TypeList<TParametersBefore...>,
                               TypeList<TParametersAfter...>,
                               v_invert,
                               TFilteredTypes...>
    : std::conditional_t<TFilter<TParametersBefore..., TFirstType, TParametersAfter...>::value != v_invert,
                         type_list_filter_helper<TypeList<TOtherTypes...>,
                                                 TFilter,
                                                 TypeList<TParametersBefore...>,
                                                 TypeList<TParametersAfter...>,
                                                 v_invert,
                                                 TFilteredTypes...,
                                                 TFirstType>,
                         type_list_filter_helper<TypeList<TOtherTypes...>,
                                                 TFilter,
                                                 TypeList<TParametersBefore...>,
                                                 TypeList<TParametersAfter...>,
                                                 v_invert,
                                                 TFilteredTypes...>> {};

} // namespace detail

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_filter
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<>, TypeList<TParameters...>, false> {};

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_filter_t = typename type_list_filter<TTypeList, TFilter, TParameters...>::type;

// ---

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_filter_parameters_first
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<TParameters...>, TypeList<>, false> {};

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_filter_parameters_first_t =
    typename type_list_filter_parameters_first<TTypeList, TFilter, TParameters...>::type;

// ---

template <AnyTypeList TTypeList,
          template <typename...>
          typename TFilter,
          AnyTypeList TParameterListBefore,
          AnyTypeList TParameterListAfter>
struct type_list_filter_unpack_parameters
    : detail::type_list_filter_helper<TTypeList, TFilter, TParameterListBefore, TParameterListAfter, false> {};

template <AnyTypeList TTypeList,
          template <typename...>
          typename TFilter,
          AnyTypeList TParameterListBefore,
          AnyTypeList TParameterListAfter>
using type_list_filter_unpack_parameters_t =
    typename type_list_filter_unpack_parameters<TTypeList, TFilter, TParameterListBefore, TParameterListAfter>::type;

// --- type_list_erase_if

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_erase_if
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<>, TypeList<TParameters...>, true> {};

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_erase_if_t = typename type_list_erase_if<TTypeList, TFilter, TParameters...>::type;

// ---

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_erase_if_parameters_first
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<TParameters...>, TypeList<>, true> {};

template <AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_erase_if_parameters_first_t =
    typename type_list_erase_if_parameters_first<TTypeList, TFilter, TParameters...>::type;

// ---

template <AnyTypeList TTypeList,
          template <typename...>
          typename TFilter,
          AnyTypeList TParameterListBefore,
          AnyTypeList TParameterListAfter>
struct type_list_erase_if_unpack_parameters
    : detail::type_list_filter_helper<TTypeList, TFilter, TParameterListBefore, TParameterListAfter, true> {};

template <AnyTypeList TTypeList,
          template <typename...>
          typename TFilter,
          AnyTypeList TParameterListBefore,
          AnyTypeList TParameterListAfter>
using type_list_erase_if_unpack_parameters_t =
    typename type_list_erase_if_unpack_parameters<TTypeList, TFilter, TParameterListBefore, TParameterListAfter>::type;

// --- type_list_apply

template <AnyTypeList TTypeList, template <typename...> typename TApplyTarget>
struct type_list_apply;

template <AnyTypeList TTypeList, template <typename...> typename TApplyTarget>
using type_list_apply_t = typename type_list_apply<TTypeList, TApplyTarget>::type;

template <typename... TTypes, template <typename...> typename TApplyTarget>
struct type_list_apply<TypeList<TTypes...>, TApplyTarget> : std::type_identity<TApplyTarget<TTypes...>> {};

// --- type_list_transform

template <AnyTypeList TTypeList, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform;

template <AnyTypeList TTypeList, template <typename...> typename TTransform, typename... TParameters>
using type_list_transform_t = typename type_list_transform<TTypeList, TTransform, TParameters...>::type;

template <typename... TTypes, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform<TypeList<TTypes...>, TTransform, TParameters...>
    : std::type_identity<TypeList<typename TTransform<TTypes, TParameters...>::type...>> {};

// ---

template <AnyTypeList TTypeList, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform_parameters_first;

template <AnyTypeList TTypeList, template <typename...> typename TTransform, typename... TParameters>
using type_list_transform_parameters_first_t =
    typename type_list_transform_parameters_first<TTypeList, TTransform, TParameters...>::type;

template <typename... TTypes, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform_parameters_first<TypeList<TTypes...>, TTransform, TParameters...>
    : std::type_identity<TypeList<typename TTransform<TParameters..., TTypes>::type...>> {};

// ---

template <AnyTypeList TTypeList,
          template <typename...>
          typename TTransform,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct type_list_transform_unpack_parameters;

template <AnyTypeList TTypeList,
          template <typename...>
          typename TTransform,
          typename TParameterListBefore,
          typename TParameterListAfter>
using type_list_transform_unpack_parameters_t =
    typename type_list_transform_unpack_parameters<TTypeList, TTransform, TParameterListBefore, TParameterListAfter>::
        type;

template <typename... TTypes,
          template <typename...>
          typename TTransform,
          typename... TParametersBefore,
          typename... TParametersAfter>
struct type_list_transform_unpack_parameters<TypeList<TTypes...>,
                                             TTransform,
                                             TypeList<TParametersBefore...>,
                                             TypeList<TParametersAfter...>>
    : std::type_identity<TypeList<typename TTransform<TParametersBefore..., TTypes, TParametersAfter...>::type...>> {};

// --- TypeList Implementation

template <typename... TTypes>
struct TypeList {
    static constexpr auto empty = is_empty_type_list_v<TypeList>;
    static constexpr auto size = type_list_size_v<TypeList>;

    template <typename TCheckedValue>
    static constexpr auto contains = type_list_contains_v<TypeList, TCheckedValue>;

    template <std::size_t v_index>
    using at = type_list_at_t<TypeList, v_index>;

    using first = type_list_first_t<TypeList>;
    using last = type_list_last_t<TypeList>;

    template <typename... TAppendedTypes>
    using append = type_list_append_t<TypeList, TAppendedTypes...>;

    template <typename... TPrependedTypes>
    using prepend = type_list_prepend_t<TypeList, TPrependedTypes...>;

    template <typename... TOtherTypeLists>
    using join = type_list_join_t<TypeList, TOtherTypeLists...>;

    template <std::size_t v_count>
    using drop = type_list_drop_t<TypeList, v_count>;

    template <std::size_t v_count>
    using take = type_list_take_t<TypeList, v_count>;

    template <std::size_t v_begin, std::size_t v_end>
    using slice = type_list_slice_t<TypeList, v_begin, v_end>;

    template <std::size_t v_begin, std::size_t v_end>
    using erase = type_list_erase_t<TypeList, v_begin, v_end>;

    template <std::size_t v_index, typename... TInsertTypes>
    using insert = type_list_insert_t<TypeList, v_index, TInsertTypes...>;

    template <template <typename...> typename TFilter, typename... TParameters>
    using filter = type_list_filter_t<TypeList, TFilter, TParameters...>;

    template <template <typename...> typename TFilter, typename... TParameters>
    using filter_parameters_first = type_list_filter_parameters_first_t<TypeList, TFilter, TParameters...>;

    template <template <typename...> typename TFilter, typename TParameterListBefore, typename TParameterListAfter>
    using filter_unpack_parameters =
        type_list_filter_unpack_parameters_t<TypeList, TFilter, TParameterListBefore, TParameterListAfter>;

    template <template <typename...> typename TFilter, typename... TParameters>
    using erase_if = type_list_erase_if_t<TypeList, TFilter, TParameters...>;

    template <template <typename...> typename TFilter, typename... TParameters>
    using erase_if_parameters_first = type_list_erase_if_parameters_first_t<TypeList, TFilter, TParameters...>;

    template <template <typename...> typename TFilter, typename TParameterListBefore, typename TParameterListAfter>
    using erase_if_unpack_parameters =
        type_list_erase_if_unpack_parameters_t<TypeList, TFilter, TParameterListBefore, TParameterListAfter>;

    template <template <typename...> typename TApplyTarget>
    using apply = type_list_apply_t<TypeList, TApplyTarget>;

    template <template <typename...> typename TTransform, typename... TParameters>
    using transform = type_list_transform_t<TypeList, TTransform, TParameters...>;

    template <template <typename...> typename TTransform, typename... TParameters>
    using transform_parameters_first = type_list_transform_parameters_first_t<TypeList, TTransform, TParameters...>;

    template <template <typename...> typename TTransform, typename TParameterListBefore, typename TParameterListAfter>
    using transform_unpack_parameters =
        type_list_transform_unpack_parameters_t<TypeList, TTransform, TParameterListBefore, TParameterListAfter>;
};

} // namespace dang::utils

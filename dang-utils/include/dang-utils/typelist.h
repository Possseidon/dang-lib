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

// --- TypeListExhaustion

/// @brief The type list got exhausted from a drop, take or slice.
struct TypeListExhaustion {};

// --- is_type_list_exhaustion

template <typename T>
struct is_type_list_exhaustion : std::is_same<T, TypeListExhaustion> {};

template <typename T>
inline constexpr auto is_type_list_exhaustion_v = is_type_list_exhaustion<T>::value;

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

// --- is_empty_type_list

template <typename T>
struct is_empty_type_list : std::is_same<T, TypeList<>> {};

template <typename T>
inline constexpr auto is_empty_type_list_v = is_empty_type_list<T>::value;

// --- type_list_size

template <typename TTypeList>
struct type_list_size;

template <typename TTypeList>
inline constexpr auto type_list_size_v = type_list_size<TTypeList>::value;

template <typename... TTypes>
struct type_list_size<TypeList<TTypes...>> : constant<sizeof...(TTypes)> {};

// --- type_list_contains

template <typename TTypeList, typename TCheckedType>
struct type_list_contains;

template <typename TTypeList, typename TCheckedType>
inline constexpr auto type_list_contains_v = type_list_contains<TTypeList, TCheckedType>::value;

template <typename... TTypes, typename TCheckedType>
struct type_list_contains<TypeList<TTypes...>, TCheckedType>
    : std::disjunction<std::is_same<TCheckedType, TTypes>...> {};

// --- type_list_at

template <typename TTypeList, std::size_t v_index>
struct type_list_at;

template <typename TTypeList, std::size_t v_index>
using type_list_at_t = typename type_list_at<TTypeList, v_index>::type;

template <std::size_t v_index>
struct type_list_at<TypeList<>, v_index> : std::type_identity<NullType> {};

template <typename TType, typename... TOtherTypes>
struct type_list_at<TypeList<TType, TOtherTypes...>, 0> : std::type_identity<TType> {};

template <typename TType, typename... TOtherTypes, std::size_t v_index>
struct type_list_at<TypeList<TType, TOtherTypes...>, v_index> : type_list_at<TypeList<TOtherTypes...>, v_index - 1> {};

// --- type_list_first

template <typename TTypeList>
using type_list_first = type_list_at<TTypeList, 0>;

template <typename TTypeList>
using type_list_first_t = typename type_list_first<TTypeList>::type;

// --- type_list_last

template <typename TTypeList>
using type_list_last = type_list_at<TTypeList, TTypeList::size - 1>;

template <typename TTypeList>
using type_list_last_t = typename type_list_last<TTypeList>::type;

// --- type_list_append

template <typename TTypeList, typename... TTypes>
struct type_list_append;

template <typename TTypeList, typename... TTypes>
using type_list_append_t = typename type_list_append<TTypeList, TTypes...>::type;

template <typename... TFirstTypes, typename... TSecondTypes>
struct type_list_append<TypeList<TFirstTypes...>, TSecondTypes...>
    : std::type_identity<TypeList<TFirstTypes..., TSecondTypes...>> {};

// --- type_list_prepend

template <typename TTypeList, typename... TTypes>
struct type_list_prepend;

template <typename TTypeList, typename... TTypes>
using type_list_prepend_t = typename type_list_prepend<TTypeList, TTypes...>::type;

template <typename... TFirstTypes, typename... TSecondTypes>
struct type_list_prepend<TypeList<TFirstTypes...>, TSecondTypes...>
    : std::type_identity<TypeList<TSecondTypes..., TFirstTypes...>> {};

// --- type_list_join

template <typename... TTypeLists>
struct type_list_join;

template <typename... TTypeLists>
using type_list_join_t = typename type_list_join<TTypeLists...>::type;

template <>
struct type_list_join<> : std::type_identity<TypeList<>> {};

template <typename TTypeList>
struct type_list_join<TTypeList> : std::type_identity<TTypeList> {};

template <typename TFirstTypeList, typename... TSecondTypes, typename... TOtherTypeLists>
struct type_list_join<TFirstTypeList, TypeList<TSecondTypes...>, TOtherTypeLists...>
    : type_list_join<type_list_append_t<TFirstTypeList, TSecondTypes...>, TOtherTypeLists...> {};

// --- type_list_drop

template <typename TTypeList, std::size_t v_count, typename = void>
struct type_list_drop;

template <typename TTypeList, std::size_t v_count>
using type_list_drop_t = typename type_list_drop<TTypeList, v_count>::type;

template <typename... TTypes>
struct type_list_drop<TypeList<TTypes...>, 0> : std::type_identity<TypeList<TTypes...>> {};

template <std::size_t v_count>
struct type_list_drop<TypeList<>, v_count, std::enable_if_t<(v_count > 0)>> : std::type_identity<TypeListExhaustion> {};

template <typename TFirstType, typename... TOtherTypes, std::size_t v_count>
struct type_list_drop<TypeList<TFirstType, TOtherTypes...>, v_count, std::enable_if_t<(v_count > 0)>>
    : type_list_drop<TypeList<TOtherTypes...>, v_count - 1> {};

// --- type_list_take

namespace detail {

template <typename TTypeList, std::size_t v_count, bool v_empty = v_count == 0, typename... TTakenTypes>
struct type_list_take_helper;

template <typename... TTypes, typename... TTakenTypes>
struct type_list_take_helper<TypeList<TTypes...>, 0, true, TTakenTypes...>
    : std::type_identity<TypeList<TTakenTypes...>> {};

template <std::size_t v_count, typename... TTakenTypes>
struct type_list_take_helper<TypeList<>, v_count, false, TTakenTypes...> : std::type_identity<TypeListExhaustion> {};

template <typename TFirstType, typename... TOtherTypes, std::size_t v_count, typename... TTakenTypes>
struct type_list_take_helper<TypeList<TFirstType, TOtherTypes...>, v_count, false, TTakenTypes...>
    : type_list_take_helper<TypeList<TOtherTypes...>, v_count - 1, (v_count == 1), TTakenTypes..., TFirstType> {};

} // namespace detail

template <typename TTypeList, std::size_t v_count>
struct type_list_take : detail::type_list_take_helper<TTypeList, v_count> {};

template <typename TTypeList, std::size_t v_count>
using type_list_take_t = typename type_list_take<TTypeList, v_count>::type;

// --- type_list_slice

template <typename TTypeList, std::size_t v_begin, std::size_t v_end, typename = void>
struct type_list_slice;

template <typename TTypeList, std::size_t v_begin, std::size_t v_end>
using type_list_slice_t = typename type_list_slice<TTypeList, v_begin, v_end>::type;

template <typename TTypeList, std::size_t v_begin, std::size_t v_end>
struct type_list_slice<TTypeList, v_begin, v_end, std::enable_if_t<(v_begin <= v_end && v_end > TTypeList::size)>>
    : std::type_identity<TypeListExhaustion> {};

template <typename TTypeList, std::size_t v_begin, std::size_t v_end>
struct type_list_slice<TTypeList, v_begin, v_end, std::enable_if_t<(v_begin <= v_end && v_end <= TTypeList::size)>>
    : type_list_take<type_list_drop_t<TTypeList, v_begin>, v_end - v_begin> {};

// --- type_list_erase

template <typename TTypeList, std::size_t v_begin, std::size_t v_end, typename = void>
struct type_list_erase;

template <typename TTypeList, std::size_t v_begin, std::size_t v_end>
struct type_list_erase<TTypeList, v_begin, v_end, std::enable_if_t<(v_begin <= v_end && v_end > TTypeList::size)>>
    : std::type_identity<TypeListExhaustion> {};

template <typename TTypeList, std::size_t v_begin, std::size_t v_end>
struct type_list_erase<TTypeList, v_begin, v_end, std::enable_if_t<(v_begin <= v_end && v_end <= TTypeList::size)>>
    : type_list_join<type_list_take_t<TTypeList, v_begin>, type_list_drop_t<TTypeList, v_end>> {};

template <typename TTypeList, std::size_t v_begin, std::size_t v_end>
using type_list_erase_t = typename type_list_erase<TTypeList, v_begin, v_end>::type;

// --- type_list_insert

template <typename TTypeList, std::size_t v_index, typename... TInsertedTypes>
struct type_list_insert
    : type_list_join<type_list_take_t<TTypeList, v_index>,
                     TypeList<TInsertedTypes...>,
                     type_list_drop_t<TTypeList, v_index>> {};

template <typename TTypeList, std::size_t v_index, typename... TInsertedTypes>
using type_list_insert_t = typename type_list_insert<TTypeList, v_index, TInsertedTypes...>::type;

// --- type_list_filter

namespace detail {

template <typename TTypeList,
          template <typename...>
          typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter,
          bool v_invert,
          typename... TFilteredTypes>
struct type_list_filter_helper;

template <template <typename...> typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter,
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

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_filter
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<>, TypeList<TParameters...>, false> {};

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_filter_t = typename type_list_filter<TTypeList, TFilter, TParameters...>::type;

// ---

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_filter_parameters_first
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<TParameters...>, TypeList<>, false> {};

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_filter_parameters_first_t =
    typename type_list_filter_parameters_first<TTypeList, TFilter, TParameters...>::type;

// ---

template <typename TTypeList,
          template <typename...>
          typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct type_list_filter_unpack_parameters
    : detail::type_list_filter_helper<TTypeList, TFilter, TParameterListBefore, TParameterListAfter, false> {};

template <typename TTypeList,
          template <typename...>
          typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter>
using type_list_filter_unpack_parameters_t =
    typename type_list_filter_unpack_parameters<TTypeList, TFilter, TParameterListBefore, TParameterListAfter>::type;

// --- type_list_erase_if

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_erase_if
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<>, TypeList<TParameters...>, true> {};

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_erase_if_t = typename type_list_erase_if<TTypeList, TFilter, TParameters...>::type;

// ---

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct type_list_erase_if_parameters_first
    : detail::type_list_filter_helper<TTypeList, TFilter, TypeList<TParameters...>, TypeList<>, true> {};

template <typename TTypeList, template <typename...> typename TFilter, typename... TParameters>
using type_list_erase_if_parameters_first_t =
    typename type_list_erase_if_parameters_first<TTypeList, TFilter, TParameters...>::type;

// ---

template <typename TTypeList,
          template <typename...>
          typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct type_list_erase_if_unpack_parameters
    : detail::type_list_filter_helper<TTypeList, TFilter, TParameterListBefore, TParameterListAfter, true> {};

template <typename TTypeList,
          template <typename...>
          typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter>
using type_list_erase_if_unpack_parameters_t =
    typename type_list_erase_if_unpack_parameters<TTypeList, TFilter, TParameterListBefore, TParameterListAfter>::type;

// --- type_list_apply

template <typename TTypeList, template <typename...> typename TApplyTarget>
struct type_list_apply;

template <typename TTypeList, template <typename...> typename TApplyTarget>
using type_list_apply_t = typename type_list_apply<TTypeList, TApplyTarget>::type;

template <typename... TTypes, template <typename...> typename TApplyTarget>
struct type_list_apply<TypeList<TTypes...>, TApplyTarget> : std::type_identity<TApplyTarget<TTypes...>> {};

// --- type_list_transform

template <typename TTypeList, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform;

template <typename TTypeList, template <typename...> typename TTransform, typename... TParameters>
using type_list_transform_t = typename type_list_transform<TTypeList, TTransform, TParameters...>::type;

template <typename... TTypes, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform<TypeList<TTypes...>, TTransform, TParameters...>
    : std::type_identity<TypeList<typename TTransform<TTypes, TParameters...>::type...>> {};

// ---

template <typename TTypeList, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform_parameters_first;

template <typename TTypeList, template <typename...> typename TTransform, typename... TParameters>
using type_list_transform_parameters_first_t =
    typename type_list_transform_parameters_first<TTypeList, TTransform, TParameters...>::type;

template <typename... TTypes, template <typename...> typename TTransform, typename... TParameters>
struct type_list_transform_parameters_first<TypeList<TTypes...>, TTransform, TParameters...>
    : std::type_identity<TypeList<typename TTransform<TParameters..., TTypes>::type...>> {};

// ---

template <typename TTypeList,
          template <typename...>
          typename TTransform,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct type_list_transform_unpack_parameters;

template <typename TTypeList,
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

// --- type_list_instantiate

template <typename TTypeList, template <typename...> typename TInstantiated, typename... TParameters>
struct type_list_instantiate;

template <typename TTypeList, template <typename...> typename TInstantiated, typename... TParameters>
using type_list_instantiate_t = typename type_list_instantiate<TTypeList, TInstantiated, TParameters...>::type;

template <typename... TTypes, template <typename...> typename TInstantiated, typename... TParameters>
struct type_list_instantiate<TypeList<TTypes...>, TInstantiated, TParameters...>
    : std::type_identity<TypeList<TInstantiated<TTypes, TParameters...>...>> {};

// ---

template <typename TTypeList, template <typename...> typename TInstantiated, typename... TParameters>
struct type_list_instantiate_parameters_first;

template <typename TTypeList, template <typename...> typename TInstantiated, typename... TParameters>
using type_list_instantiate_parameters_first_t =
    typename type_list_instantiate_parameters_first<TTypeList, TInstantiated, TParameters...>::type;

template <typename... TTypes, template <typename...> typename TInstantiated, typename... TParameters>
struct type_list_instantiate_parameters_first<TypeList<TTypes...>, TInstantiated, TParameters...>
    : std::type_identity<TypeList<TInstantiated<TParameters..., TTypes>...>> {};

// ---

template <typename TTypeList,
          template <typename...>
          typename TInstantiated,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct type_list_instantiate_unpack_parameters;

template <typename TTypeList,
          template <typename...>
          typename TInstantiated,
          typename TParameterListBefore,
          typename TParameterListAfter>
using type_list_instantiate_unpack_parameters_t =
    typename type_list_instantiate_unpack_parameters<TTypeList,
                                                     TInstantiated,
                                                     TParameterListBefore,
                                                     TParameterListAfter>::type;

template <typename... TTypes,
          template <typename...>
          typename TInstantiated,
          typename... TParametersBefore,
          typename... TParametersAfter>
struct type_list_instantiate_unpack_parameters<TypeList<TTypes...>,
                                               TInstantiated,
                                               TypeList<TParametersBefore...>,
                                               TypeList<TParametersAfter...>>
    : std::type_identity<TypeList<TInstantiated<TParametersBefore..., TTypes, TParametersAfter...>...>> {};

// --- TypeList Implementation

template <typename... TTypes>
struct TypeList {
    static constexpr bool empty = is_empty_type_list_v<TypeList>;
    static constexpr std::size_t size = type_list_size_v<TypeList>;

    template <typename TCheckedValue>
    static constexpr bool contains = type_list_contains_v<TypeList, TCheckedValue>;

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

    template <template <typename...> typename TInstantiated, typename... TParameters>
    using instantiate = type_list_instantiate_t<TypeList, TInstantiated, TParameters...>;

    template <template <typename...> typename TInstantiated, typename... TParameters>
    using instantiate_parameters_first =
        type_list_instantiate_parameters_first_t<TypeList, TInstantiated, TParameters...>;

    template <template <typename...> typename TInstantiated,
              typename TParameterListBefore,
              typename TParameterListAfter>
    using instantiate_unpack_parameters =
        type_list_instantiate_unpack_parameters_t<TypeList, TInstantiated, TParameterListBefore, TParameterListAfter>;
};

} // namespace dang::utils

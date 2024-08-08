#include <type_traits>

#include "dang-utils/global.h"
#include "dang-utils/typelist.h"

#include "catch2/catch_test_macros.hpp"

namespace dutils = dang::utils;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};
struct F {};

template <typename T>
static constexpr bool testIsNullType()
{
    return dutils::is_null_type<T>::value && //
           dutils::is_null_type_v<T> &&      //
           dutils::NullTyped<T>;
}

TEST_CASE("NullTypes can be checked for.", "[typelist]")
{
    STATIC_CHECK(testIsNullType<dutils::NullType>());

    STATIC_CHECK_FALSE(testIsNullType<dutils::TypeListExhaustion>());
    STATIC_CHECK_FALSE(testIsNullType<dutils::TypeList<>>());
    STATIC_CHECK_FALSE(testIsNullType<int>());
}

template <typename T>
static constexpr bool testIsTypeListExhaustion()
{
    return dutils::is_type_list_exhaustion<T>::value && //
           dutils::is_type_list_exhaustion_v<T> &&      //
           dutils::ExhaustedTypeList<T>;
}

TEST_CASE("TypeListExhaustions can be checked for.", "[typelist]")
{
    STATIC_CHECK(testIsTypeListExhaustion<dutils::TypeListExhaustion>());

    STATIC_CHECK_FALSE(testIsTypeListExhaustion<dutils::NullType>());
    STATIC_CHECK_FALSE(testIsTypeListExhaustion<dutils::TypeList<>>());
    STATIC_CHECK_FALSE(testIsTypeListExhaustion<int>());
}

template <typename... TTypes>
static constexpr bool testMakeTypeList()
{
    return std::is_same_v<typename dutils::make_type_list<TTypes...>::type, dutils::TypeList<TTypes...>> &&
           std::is_same_v<dutils::make_type_list_t<TTypes...>, dutils::TypeList<TTypes...>>;
}

TEST_CASE("TypeLists can be created using a type trait.", "[typelist]")
{
    STATIC_CHECK(testMakeTypeList<>());
    STATIC_CHECK(testMakeTypeList<A>());
    STATIC_CHECK(testMakeTypeList<A, B>());
    STATIC_CHECK(testMakeTypeList<A, B, C>());
}

template <typename T>
static constexpr bool testIsTypeList()
{
    return dutils::is_type_list<T>::value && //
           dutils::is_type_list_v<T> &&      //
           dutils::AnyTypeList<T>;
}

TEST_CASE("TypeLists can be checked for.", "[typelist]")
{
    STATIC_CHECK(testIsTypeList<dutils::TypeList<>>());
    STATIC_CHECK(testIsTypeList<dutils::TypeList<A>>());
    STATIC_CHECK(testIsTypeList<dutils::TypeList<A, B>>());
    STATIC_CHECK(testIsTypeList<dutils::TypeList<A, B, C>>());
    STATIC_CHECK(testIsTypeList<dutils::TypeList<A, B, C>>());

    STATIC_CHECK_FALSE(testIsTypeList<dutils::NullType>());
    STATIC_CHECK_FALSE(testIsTypeList<dutils::TypeListExhaustion>());
    STATIC_CHECK_FALSE(testIsTypeList<int>());
}

template <typename T>
static constexpr bool testIsEmptyTypeList()
{
    return dutils::is_empty_type_list<T>::value && //
           dutils::is_empty_type_list_v<T> &&      //
           dutils::EmptyTypeList<T>;
}

template <dutils::AnyTypeList TTypeList>
static constexpr bool testIsTypeListEmpty()
{
    return testIsEmptyTypeList<TTypeList>() && TTypeList::empty;
}

TEST_CASE("TypeLists can be checked for emptiness.", "[typelist]")
{
    STATIC_CHECK(testIsTypeListEmpty<dutils::TypeList<>>());

    STATIC_CHECK_FALSE(testIsTypeListEmpty<dutils::TypeList<A>>());
    STATIC_CHECK_FALSE(testIsTypeListEmpty<dutils::TypeList<A, B>>());
    STATIC_CHECK_FALSE(testIsTypeListEmpty<dutils::TypeList<A, B, C>>());

    STATIC_CHECK_FALSE(testIsEmptyTypeList<dutils::NullType>());
    STATIC_CHECK_FALSE(testIsEmptyTypeList<dutils::TypeListExhaustion>());
    STATIC_CHECK_FALSE(testIsEmptyTypeList<int>());
}

template <typename... TTypes>
struct TestTypeListSize {
    static constexpr bool is(std::size_t v_expected_size)
    {
        return dutils::type_list_size<dutils::TypeList<TTypes...>>::value == v_expected_size && //
               dutils::type_list_size_v<dutils::TypeList<TTypes...>> == v_expected_size &&      //
               dutils::TypeList<TTypes...>::size == v_expected_size;
    }
};

TEST_CASE("TypeLists can have their size queried.", "[typelist]")
{
    STATIC_CHECK(TestTypeListSize<>::is(0));
    STATIC_CHECK(TestTypeListSize<A>::is(1));
    STATIC_CHECK(TestTypeListSize<A, B>::is(2));
    STATIC_CHECK(TestTypeListSize<A, B, C>::is(3));
}

template <dutils::AnyTypeList TTypeList, typename TContainedType>
static constexpr bool testTypeListContains()
{
    return dutils::type_list_contains<TTypeList, TContainedType>::value && //
           dutils::type_list_contains_v<TTypeList, TContainedType> &&      //
           TTypeList::template contains<TContainedType>;
}

TEST_CASE("TypeLists can be checked for specific contained types.", "[typelist]")
{
    STATIC_CHECK_FALSE(testTypeListContains<dutils::TypeList<>, A>());

    STATIC_CHECK(testTypeListContains<dutils::TypeList<A>, A>());
    STATIC_CHECK_FALSE(testTypeListContains<dutils::TypeList<A>, B>());

    STATIC_CHECK(testTypeListContains<dutils::TypeList<A, B>, A>());
    STATIC_CHECK(testTypeListContains<dutils::TypeList<A, B>, B>());
    STATIC_CHECK_FALSE(testTypeListContains<dutils::TypeList<A, B>, C>());
}

template <dutils::AnyTypeList TTypeList, std::size_t v_index>
struct TestTypeListAt {
    template <typename TExpectedResult>
    static constexpr bool is()
    {
        return std::is_same_v<typename dutils::type_list_at<TTypeList, v_index>::type, TExpectedResult> &&
               std::is_same_v<dutils::type_list_at_t<TTypeList, v_index>, TExpectedResult> &&
               std::is_same_v<typename TTypeList::template at<v_index>, TExpectedResult>;
    }
};

TEST_CASE("TypeLists can be indexed.", "[typelist]")
{
    STATIC_CHECK(TestTypeListAt<dutils::TypeList<>, 0>::is<dutils::NullType>());

    STATIC_CHECK(TestTypeListAt<dutils::TypeList<A>, 0>::is<A>());
    STATIC_CHECK(TestTypeListAt<dutils::TypeList<A>, 1>::is<dutils::NullType>());

    STATIC_CHECK(TestTypeListAt<dutils::TypeList<A, B>, 0>::is<A>());
    STATIC_CHECK(TestTypeListAt<dutils::TypeList<A, B>, 1>::is<B>());
    STATIC_CHECK(TestTypeListAt<dutils::TypeList<A, B>, 2>::is<dutils::NullType>());
}

template <typename... TTypes>
struct TestTypeListFirst {
    template <typename TExpectedResult>
    static constexpr bool is()
    {
        return std::is_same_v<typename dutils::type_list_first<dutils::TypeList<TTypes...>>::type, TExpectedResult> &&
               std::is_same_v<dutils::type_list_first_t<dutils::TypeList<TTypes...>>, TExpectedResult> &&
               std::is_same_v<typename dutils::TypeList<TTypes...>::first, TExpectedResult>;
    }
};

TEST_CASE("TypeLists can query their first contained type.", "[typelist]")
{
    STATIC_CHECK(TestTypeListFirst<>::is<dutils::NullType>());
    STATIC_CHECK(TestTypeListFirst<A>::is<A>());
    STATIC_CHECK(TestTypeListFirst<A, B>::is<A>());
    STATIC_CHECK(TestTypeListFirst<A, B, C>::is<A>());
}

template <typename... TTypes>
struct TestTypeListLast {
    template <typename TExpectedResult>
    static constexpr bool is()
    {
        return std::is_same_v<typename dutils::type_list_last<dutils::TypeList<TTypes...>>::type, TExpectedResult> &&
               std::is_same_v<dutils::type_list_last_t<dutils::TypeList<TTypes...>>, TExpectedResult> &&
               std::is_same_v<typename dutils::TypeList<TTypes...>::last, TExpectedResult>;
    }
};

TEST_CASE("TypeLists can query their last contained type.", "[typelist]")
{
    STATIC_CHECK(TestTypeListLast<>::is<dutils::NullType>());
    STATIC_CHECK(TestTypeListLast<A>::is<A>());
    STATIC_CHECK(TestTypeListLast<A, B>::is<B>());
    STATIC_CHECK(TestTypeListLast<A, B, C>::is<C>());
}

template <dutils::AnyTypeList TTypeList, typename... TAppendedTypes>
struct TestTypeListAppend {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_append<TTypeList, TAppendedTypes...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_append_t<TTypeList, TAppendedTypes...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template append<TAppendedTypes...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

TEST_CASE("TypeLists can be appended with additional types.", "[typelist]")
{
    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<>>::isTypeList<>());
    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<>, A>::isTypeList<A>());
    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<>, A, B>::isTypeList<A, B>());

    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<A>>::isTypeList<A>());
    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<A>, B>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<A>, B, C>::isTypeList<A, B, C>());

    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<A, B>>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<A, B>, C>::isTypeList<A, B, C>());
    STATIC_CHECK(TestTypeListAppend<dutils::TypeList<A, B>, C, D>::isTypeList<A, B, C, D>());
}

template <dutils::AnyTypeList TTypeList, typename... TPrependedTypes>
struct TestTypeListPrepend {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_prepend<TTypeList, TPrependedTypes...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_prepend_t<TTypeList, TPrependedTypes...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template prepend<TPrependedTypes...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

TEST_CASE("TypeLists can be prepended with additional types.", "[typelist]")
{
    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<>>::isTypeList<>());
    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<>, A>::isTypeList<A>());
    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<>, A, B>::isTypeList<A, B>());

    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<A>>::isTypeList<A>());
    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<A>, B>::isTypeList<B, A>());
    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<A>, B, C>::isTypeList<B, C, A>());

    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<A, B>>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<A, B>, C>::isTypeList<C, A, B>());
    STATIC_CHECK(TestTypeListPrepend<dutils::TypeList<A, B>, C, D>::isTypeList<C, D, A, B>());
}

template <dutils::AnyTypeList TFirstTypeList, dutils::AnyTypeList... TOtherTypeLists>
struct TestTypeListJoin {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_join<TFirstTypeList, TOtherTypeLists...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_join_t<TFirstTypeList, TOtherTypeLists...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TFirstTypeList::template join<TOtherTypeLists...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

TEST_CASE("TypeLists can be joined.", "[typelist]")
{
    STATIC_CHECK(std::is_same_v<typename dutils::type_list_join<>::type, dutils::TypeList<>>);
    STATIC_CHECK(std::is_same_v<dutils::type_list_join_t<>, dutils::TypeList<>>);

    STATIC_CHECK(TestTypeListJoin<dutils::TypeList<>>::isTypeList<>());
    STATIC_CHECK(TestTypeListJoin<dutils::TypeList<>, dutils::TypeList<>>::isTypeList<>());
    STATIC_CHECK(TestTypeListJoin<dutils::TypeList<>, dutils::TypeList<>, dutils::TypeList<>>::isTypeList<>());

    STATIC_CHECK(TestTypeListJoin<dutils::TypeList<A>>::isTypeList<A>());
    STATIC_CHECK(TestTypeListJoin<dutils::TypeList<A>, dutils::TypeList<B>>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListJoin<dutils::TypeList<A>, //
                                  dutils::TypeList<B>,
                                  dutils::TypeList<C>>::isTypeList<A, B, C>());

    STATIC_CHECK(TestTypeListJoin<dutils::TypeList<A, B>, //
                                  dutils::TypeList<>,
                                  dutils::TypeList<C>>::isTypeList<A, B, C>());
}

template <dutils::AnyTypeList TTypeList, std::size_t v_count>
struct TestTypeListDrop {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_drop<TTypeList, v_count>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_drop_t<TTypeList, v_count>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template drop<v_count>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }

    static constexpr bool exhausts()
    {
        return dutils::ExhaustedTypeList<typename dutils::type_list_drop<TTypeList, v_count>::type> &&
               dutils::ExhaustedTypeList<dutils::type_list_drop_t<TTypeList, v_count>> &&
               dutils::ExhaustedTypeList<typename TTypeList::template drop<v_count>>;
    }
};

TEST_CASE("TypeLists can have a given number of types dropped.", "[typelist]")
{
    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<>, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<>, 1>::exhausts());

    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<A>, 0>::isTypeList<A>());
    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<A>, 1>::isTypeList<>());
    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<A>, 2>::exhausts());

    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<A, B>, 0>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<A, B>, 1>::isTypeList<B>());
    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<A, B>, 2>::isTypeList<>());
    STATIC_CHECK(TestTypeListDrop<dutils::TypeList<A, B>, 3>::exhausts());
}

template <dutils::AnyTypeList TTypeList, std::size_t v_count>
struct TestTypeListTake {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_take<TTypeList, v_count>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_take_t<TTypeList, v_count>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template take<v_count>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }

    static constexpr bool exhausts()
    {
        return dutils::ExhaustedTypeList<typename dutils::type_list_take<TTypeList, v_count>::type> &&
               dutils::ExhaustedTypeList<dutils::type_list_take_t<TTypeList, v_count>> &&
               dutils::ExhaustedTypeList<typename TTypeList::template take<v_count>>;
    }
};

TEST_CASE("TypeLists can have a given number of types taken.", "[typelist]")
{
    STATIC_CHECK(TestTypeListTake<dutils::TypeList<>, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListTake<dutils::TypeList<>, 1>::exhausts());

    STATIC_CHECK(TestTypeListTake<dutils::TypeList<A>, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListTake<dutils::TypeList<A>, 1>::isTypeList<A>());
    STATIC_CHECK(TestTypeListTake<dutils::TypeList<A>, 2>::exhausts());

    STATIC_CHECK(TestTypeListTake<dutils::TypeList<A, B>, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListTake<dutils::TypeList<A, B>, 1>::isTypeList<A>());
    STATIC_CHECK(TestTypeListTake<dutils::TypeList<A, B>, 2>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListTake<dutils::TypeList<A, B>, 3>::exhausts());
}

template <dutils::AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
struct TestTypeListSlice {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v<typename dutils::type_list_slice<TTypeList, v_begin, v_end>::type,
                              dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v<dutils::type_list_slice_t<TTypeList, v_begin, v_end>,
                              dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v<typename TTypeList::template slice<v_begin, v_end>,
                              dutils::TypeList<TExpectedResultTypes...>>;
    }

    static constexpr bool exhausts()
    {
        return dutils::ExhaustedTypeList<typename dutils::type_list_slice<TTypeList, v_begin, v_end>::type> &&
               dutils::ExhaustedTypeList<dutils::type_list_slice_t<TTypeList, v_begin, v_end>> &&
               dutils::ExhaustedTypeList<typename TTypeList::template slice<v_begin, v_end>>;
    }
};

TEST_CASE("TypeLists can be sliced arbitrarily.", "[typelist]")
{
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<>, 0, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<>, 0, 1>::exhausts());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<>, 1, 1>::exhausts());

    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A>, 0, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A>, 0, 1>::isTypeList<A>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A>, 0, 2>::exhausts());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A>, 1, 1>::isTypeList<>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A>, 1, 2>::exhausts());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A>, 2, 2>::exhausts());

    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 0, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 0, 1>::isTypeList<A>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 0, 2>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 0, 3>::exhausts());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 1, 1>::isTypeList<>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 1, 2>::isTypeList<B>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 1, 3>::exhausts());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 2, 2>::isTypeList<>());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 2, 3>::exhausts());
    STATIC_CHECK(TestTypeListSlice<dutils::TypeList<A, B>, 3, 3>::exhausts());
}

template <dutils::AnyTypeList TTypeList, std::size_t v_begin, std::size_t v_end>
struct TestTypeListErase {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v<typename dutils::type_list_erase<TTypeList, v_begin, v_end>::type,
                              dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v<dutils::type_list_erase_t<TTypeList, v_begin, v_end>,
                              dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v<typename TTypeList::template erase<v_begin, v_end>,
                              dutils::TypeList<TExpectedResultTypes...>>;
    }

    static constexpr bool exhausts()
    {
        return dutils::ExhaustedTypeList<typename dutils::type_list_erase<TTypeList, v_begin, v_end>::type> &&
               dutils::ExhaustedTypeList<dutils::type_list_erase_t<TTypeList, v_begin, v_end>> &&
               dutils::ExhaustedTypeList<typename TTypeList::template erase<v_begin, v_end>>;
    }
};

TEST_CASE("TypeLists can have slices erased.", "[typelist]")
{
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<>, 0, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<>, 0, 1>::exhausts());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<>, 1, 1>::exhausts());

    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A>, 0, 0>::isTypeList<A>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A>, 0, 1>::isTypeList<>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A>, 0, 2>::exhausts());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A>, 1, 1>::isTypeList<A>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A>, 1, 2>::exhausts());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A>, 2, 2>::exhausts());

    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 0, 0>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 0, 1>::isTypeList<B>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 0, 2>::isTypeList<>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 0, 3>::exhausts());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 1, 1>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 1, 2>::isTypeList<A>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 1, 3>::exhausts());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 2, 2>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 2, 3>::exhausts());
    STATIC_CHECK(TestTypeListErase<dutils::TypeList<A, B>, 3, 3>::exhausts());
}

template <dutils::AnyTypeList TTypeList, std::size_t v_index, typename... TInsertedTypes>
struct TestTypeListInsert {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_insert<TTypeList, v_index, TInsertedTypes...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_insert_t<TTypeList, v_index, TInsertedTypes...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template insert<v_index, TInsertedTypes...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

TEST_CASE("TypeLists can have new types inserted at an arbitrary position.", "[typelist]")
{
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<>, 0>::isTypeList<>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<>, 0, A>::isTypeList<A>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<>, 0, A, B>::isTypeList<A, B>());

    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A>, 0>::isTypeList<A>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A>, 0, B>::isTypeList<B, A>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A>, 0, B, C>::isTypeList<B, C, A>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A>, 1>::isTypeList<A>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A>, 1, B>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A>, 1, B, C>::isTypeList<A, B, C>());

    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 0>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 0, C>::isTypeList<C, A, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 0, C, D>::isTypeList<C, D, A, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 1>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 1, C>::isTypeList<A, C, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 1, C, D>::isTypeList<A, C, D, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 2>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 2, C>::isTypeList<A, B, C>());
    STATIC_CHECK(TestTypeListInsert<dutils::TypeList<A, B>, 2, C, D>::isTypeList<A, B, C, D>());
}

struct Base {};
struct Derived : Base {};

template <dutils::AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct TestTypeListFilter {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_filter<TTypeList, TFilter, TParameters...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_filter_t<TTypeList, TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template filter<TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <dutils::AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct TestTypeListFilterParametersFirst {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_filter_parameters_first<TTypeList, TFilter, TParameters...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_filter_parameters_first_t<TTypeList, TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template filter_parameters_first<TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <dutils::AnyTypeList TTypeList,
          template <typename...>
          typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct TestTypeListFilterUnpackParameters {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_filter_unpack_parameters<TTypeList,
                                                                       TFilter,
                                                                       TParameterListBefore,
                                                                       TParameterListAfter>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_filter_unpack_parameters_t<TTypeList,
                                                                TFilter,
                                                                TParameterListBefore,
                                                                TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::
                       template filter_unpack_parameters<TFilter, TParameterListBefore, TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

TEST_CASE("TypeLists can be filtered on an optionally type-parametrized predicate.", "[typelist]")
{
    STATIC_CHECK(TestTypeListFilter< //
                 dutils::TypeList<A, Base, B>,
                 std::is_base_of,
                 Derived>::isTypeList<Base>());
    STATIC_CHECK(TestTypeListFilterParametersFirst< //
                 dutils::TypeList<A, Derived, B>,
                 std::is_base_of,
                 Base>::isTypeList<Derived>());

    STATIC_CHECK(TestTypeListFilterUnpackParameters< //
                 dutils::TypeList<A, Base, B>,
                 std::is_base_of,
                 dutils::TypeList<>,
                 dutils::TypeList<Derived>>::isTypeList<Base>());
    STATIC_CHECK(TestTypeListFilterUnpackParameters< //
                 dutils::TypeList<A, Derived, B>,
                 std::is_base_of,
                 dutils::TypeList<Base>,
                 dutils::TypeList<>>::isTypeList<Derived>());
}

template <dutils::AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct TestTypeListEraseIf {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_erase_if<TTypeList, TFilter, TParameters...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_erase_if_t<TTypeList, TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template erase_if<TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <dutils::AnyTypeList TTypeList, template <typename...> typename TFilter, typename... TParameters>
struct TestTypeListEraseIfParametersFirst {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_erase_if_parameters_first<TTypeList, TFilter, TParameters...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_erase_if_parameters_first_t<TTypeList, TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template erase_if_parameters_first<TFilter, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <dutils::AnyTypeList TTypeList,
          template <typename...>
          typename TFilter,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct TestTypeListEraseIfUnpackParameters {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_erase_if_unpack_parameters<TTypeList,
                                                                         TFilter,
                                                                         TParameterListBefore,
                                                                         TParameterListAfter>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_erase_if_unpack_parameters_t<TTypeList,
                                                                  TFilter,
                                                                  TParameterListBefore,
                                                                  TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::
                       template erase_if_unpack_parameters<TFilter, TParameterListBefore, TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

TEST_CASE("TypeLists can be filtered on a negated optionally type-parametrized predicate.", "[typelist]")
{
    STATIC_CHECK(TestTypeListEraseIf< //
                 dutils::TypeList<A, Base, B>,
                 std::is_base_of,
                 Derived>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListEraseIfParametersFirst< //
                 dutils::TypeList<A, Derived, B>,
                 std::is_base_of,
                 Base>::isTypeList<A, B>());

    STATIC_CHECK(TestTypeListEraseIfUnpackParameters< //
                 dutils::TypeList<A, Base, B>,
                 std::is_base_of,
                 dutils::TypeList<>,
                 dutils::TypeList<Derived>>::isTypeList<A, B>());
    STATIC_CHECK(TestTypeListEraseIfUnpackParameters< //
                 dutils::TypeList<A, B>,
                 std::is_base_of,
                 dutils::TypeList<Base>,
                 dutils::TypeList<>>::isTypeList<A, B>());
}

template <typename...>
struct ApplyTarget;

TEST_CASE("TypeLists can apply their types on other templated types.", "[typelist]")
{
    STATIC_CHECK(std::is_same_v< //
                 typename dutils::type_list_apply<dutils::TypeList<>, ApplyTarget>::type,
                 ApplyTarget<>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::type_list_apply_t<dutils::TypeList<>, ApplyTarget>,
                 ApplyTarget<>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::TypeList<>::apply<ApplyTarget>,
                 ApplyTarget<>>);

    STATIC_CHECK(std::is_same_v< //
                 typename dutils::type_list_apply<dutils::TypeList<A>, ApplyTarget>::type,
                 ApplyTarget<A>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::type_list_apply_t<dutils::TypeList<A>, ApplyTarget>,
                 ApplyTarget<A>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::TypeList<A>::apply<ApplyTarget>,
                 ApplyTarget<A>>);

    STATIC_CHECK(std::is_same_v< //
                 typename dutils::type_list_apply<dutils::TypeList<A, B>, ApplyTarget>::type,
                 ApplyTarget<A, B>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::type_list_apply_t<dutils::TypeList<A, B>, ApplyTarget>,
                 ApplyTarget<A, B>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::TypeList<A, B>::apply<ApplyTarget>,
                 ApplyTarget<A, B>>);

    STATIC_CHECK(std::is_same_v< //
                 typename dutils::type_list_apply<dutils::TypeList<A, B, C>, ApplyTarget>::type,
                 ApplyTarget<A, B, C>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::type_list_apply_t<dutils::TypeList<A, B, C>, ApplyTarget>,
                 ApplyTarget<A, B, C>>);
    STATIC_CHECK(std::is_same_v< //
                 dutils::TypeList<A, B, C>::apply<ApplyTarget>,
                 ApplyTarget<A, B, C>>);
}

template <dutils::AnyTypeList TTypeList, template <typename...> typename TTransform, typename... TParameters>
struct TestTypeListTransform {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_transform<TTypeList, TTransform, TParameters...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_transform_t<TTypeList, TTransform, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template transform<TTransform, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <dutils::AnyTypeList TTypeList, template <typename...> typename TTransform, typename... TParameters>
struct TestTypeListTransformParametersFirst {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_transform_parameters_first<TTypeList, TTransform, TParameters...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_transform_parameters_first_t<TTypeList, TTransform, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template transform_parameters_first<TTransform, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <dutils::AnyTypeList TTypeList,
          template <typename...>
          typename TTransform,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct TestTypeListTransformUnpackParameters {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_transform_unpack_parameters<TTypeList,
                                                                          TTransform,
                                                                          TParameterListBefore,
                                                                          TParameterListAfter>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_transform_unpack_parameters_t<TTypeList,
                                                                   TTransform,
                                                                   TParameterListBefore,
                                                                   TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::
                       template transform_unpack_parameters<TTransform, TParameterListBefore, TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

TEST_CASE("TypeLists can apply a transformation on each type.", "[typelist]")
{
    STATIC_CHECK(TestTypeListTransform< //
                 dutils::TypeList<>,
                 std::add_pointer>::isTypeList<>());
    STATIC_CHECK(TestTypeListTransform< //
                 dutils::TypeList<A>,
                 std::add_pointer>::isTypeList<A*>());
    STATIC_CHECK(TestTypeListTransform< //
                 dutils::TypeList<A, B>,
                 std::add_pointer>::isTypeList<A*, B*>());
    STATIC_CHECK(TestTypeListTransform< //
                 dutils::TypeList<A, B, C>,
                 std::add_pointer>::isTypeList<A*, B*, C*>());

    STATIC_CHECK(TestTypeListTransform< //
                 dutils::TypeList<A, B>,
                 dutils::make_type_list,
                 C,
                 D>::isTypeList<dutils::TypeList<A, C, D>, dutils::TypeList<B, C, D>>());

    STATIC_CHECK(TestTypeListTransformParametersFirst< //
                 dutils::TypeList<A, B>,
                 dutils::make_type_list,
                 C,
                 D>::isTypeList<dutils::TypeList<C, D, A>, dutils::TypeList<C, D, B>>());

    STATIC_CHECK(TestTypeListTransformUnpackParameters< //
                 dutils::TypeList<A, B>,
                 dutils::make_type_list,
                 dutils::TypeList<C, D>,
                 dutils::TypeList<E, F>>::isTypeList< //
                 dutils::TypeList<C, D, A, E, F>,
                 dutils::TypeList<C, D, B, E, F>>());
}

template <typename TTypeList, template <typename...> typename TInstantiated, typename... TParameters>
struct TestTypeListInstantiate {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_instantiate<TTypeList, TInstantiated, TParameters...>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_instantiate_t<TTypeList, TInstantiated, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template instantiate<TInstantiated, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <typename TTypeList, template <typename...> typename TInstantiated, typename... TParameters>
struct TestTypeListInstantiateParametersFirst {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_instantiate_parameters_first<TTypeList, TInstantiated, TParameters...>::
                       type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_instantiate_parameters_first_t<TTypeList, TInstantiated, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::template instantiate_parameters_first<TInstantiated, TParameters...>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <typename TTypeList,
          template <typename...>
          typename TInstantiated,
          typename TParameterListBefore,
          typename TParameterListAfter>
struct TestTypeListInstantiateUnpackParameters {
    template <typename... TExpectedResultTypes>
    static constexpr bool isTypeList()
    {
        return std::is_same_v< //
                   typename dutils::type_list_instantiate_unpack_parameters<TTypeList,
                                                                            TInstantiated,
                                                                            TParameterListBefore,
                                                                            TParameterListAfter>::type,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   dutils::type_list_instantiate_unpack_parameters_t<TTypeList,
                                                                     TInstantiated,
                                                                     TParameterListBefore,
                                                                     TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>> &&
               std::is_same_v< //
                   typename TTypeList::
                       template instantiate_unpack_parameters<TInstantiated, TParameterListBefore, TParameterListAfter>,
                   dutils::TypeList<TExpectedResultTypes...>>;
    }
};

template <typename...>
struct InstantiateTarget;

TEST_CASE("TypeLists can instantiate a template with each type.", "[typelist]")
{
    STATIC_CHECK(TestTypeListInstantiate< //
                 dutils::TypeList<>,
                 InstantiateTarget>::isTypeList<>());
    STATIC_CHECK(TestTypeListInstantiate< //
                 dutils::TypeList<A>,
                 InstantiateTarget>::isTypeList<InstantiateTarget<A>>());
    STATIC_CHECK(TestTypeListInstantiate< //
                 dutils::TypeList<A, B>,
                 InstantiateTarget>::isTypeList<InstantiateTarget<A>, InstantiateTarget<B>>());
    STATIC_CHECK(TestTypeListInstantiate< //
                 dutils::TypeList<A, B, C>,
                 InstantiateTarget>::isTypeList<InstantiateTarget<A>, InstantiateTarget<B>, InstantiateTarget<C>>());

    STATIC_CHECK(TestTypeListInstantiate< //
                 dutils::TypeList<A, B>,
                 InstantiateTarget,
                 C,
                 D>::isTypeList<InstantiateTarget<A, C, D>, InstantiateTarget<B, C, D>>());

    STATIC_CHECK(TestTypeListInstantiateParametersFirst< //
                 dutils::TypeList<A, B>,
                 InstantiateTarget,
                 C,
                 D>::isTypeList<InstantiateTarget<C, D, A>, InstantiateTarget<C, D, B>>());

    STATIC_CHECK(TestTypeListInstantiateUnpackParameters< //
                 dutils::TypeList<A, B>,
                 InstantiateTarget,
                 dutils::TypeList<C, D>,
                 dutils::TypeList<E, F>>::isTypeList< //
                 InstantiateTarget<C, D, A, E, F>,
                 InstantiateTarget<C, D, B, E, F>>());
}

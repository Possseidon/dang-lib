#include <concepts>
#include <ranges>
#include <tuple>
#include <type_traits>

#include "dang-utils/tree.h"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

namespace dutils = dang::utils;

template <typename... Ts>
using maybe_const = std::tuple<Ts..., const Ts...>;

using AllTreeTypes = std::tuple<dutils::Tree<int>,
                                dutils::TreeOptionalData<int>,
                                dutils::TreeLeafData<int>,
                                dutils::TreeOptionalLeafData<int>>;

TEST_CASE("Trees expose all template parameters.", "[tree]")
{
    SECTION("Trees default to not having leaf or optional data.")
    {
        STATIC_CHECK(std::is_same_v<dutils::Tree<int>::Data, int>);
        STATIC_CHECK(!dutils::Tree<int>::leaf_data);
        STATIC_CHECK(!dutils::Tree<int>::optional_data);

        STATIC_CHECK(std::is_same_v<dutils::Tree<int, true>::Data, int>);
        STATIC_CHECK(dutils::Tree<int, true>::leaf_data);
        STATIC_CHECK(!dutils::Tree<int, true>::optional_data);

        STATIC_CHECK(std::is_same_v<dutils::Tree<int, false, true>::Data, int>);
        STATIC_CHECK(!dutils::Tree<int, false, true>::leaf_data);
        STATIC_CHECK(dutils::Tree<int, false, true>::optional_data);

        STATIC_CHECK(std::is_same_v<dutils::Tree<int, true, true>::Data, int>);
        STATIC_CHECK(dutils::Tree<int, true, true>::leaf_data);
        STATIC_CHECK(dutils::Tree<int, true, true>::optional_data);
    }
    SECTION("Custom aliases exist for more convenient usage.")
    {
        STATIC_CHECK(std::is_same_v<dutils::TreeLeafData<int>, dutils::Tree<int, true, false>>);
        STATIC_CHECK(std::is_same_v<dutils::TreeLeafData<int, false>, dutils::Tree<int, true, false>>);
        STATIC_CHECK(std::is_same_v<dutils::TreeLeafData<int, true>, dutils::Tree<int, true, true>>);

        STATIC_CHECK(std::is_same_v<dutils::TreeOptionalData<int>, dutils::Tree<int, false, true>>);
        STATIC_CHECK(std::is_same_v<dutils::TreeOptionalData<int, false>, dutils::Tree<int, false, true>>);
        STATIC_CHECK(std::is_same_v<dutils::TreeOptionalData<int, true>, dutils::Tree<int, true, true>>);

        STATIC_CHECK(std::is_same_v<dutils::TreeOptionalLeafData<int>, dutils::Tree<int, true, true>>);
    }
}

TEST_CASE("Trees expose the corresponding node type.", "[tree]")
{
    STATIC_CHECK(std::is_same_v<typename dutils::Tree<int, false, false>::Node, dutils::TreeNode<int, false, false>>);
    STATIC_CHECK(std::is_same_v<typename dutils::Tree<int, false, true>::Node, dutils::TreeNode<int, false, true>>);
    STATIC_CHECK(std::is_same_v<typename dutils::Tree<int, true, false>::Node, dutils::TreeNode<int, true, false>>);
    STATIC_CHECK(std::is_same_v<typename dutils::Tree<int, true, true>::Node, dutils::TreeNode<int, true, true>>);
}

TEMPLATE_LIST_TEST_CASE("Trees can be default constructed to null.", "[tree]", AllTreeTypes)
{
    CHECK(!TestType());
    CHECK(!TestType{});
}

TEMPLATE_LIST_TEST_CASE("Trees can be initialized from values and initializer lists.", "[tree]", AllTreeTypes)
{
    // Constructing a tree with an empty initializer list is ambiguous.
    // dutils::Tree<int>({});

    SECTION("Constructing a tree with a value.")
    {
        auto tree = TestType(42);
        CHECK(tree);
        CHECKED_IF(tree->hasData()) { CHECK(tree->data() == 42); }
        CHECK(tree->supportsChildren() != TestType::leaf_data);
    }
    SECTION("Constructing a tree that supports children.")
    {
        auto tree = TestType{{}};
        CHECK(tree);
        CHECK(tree->hasData() == !TestType::leaf_data);
        CHECKED_IF(tree->supportsChildren()) { CHECK(tree->children().empty()); }
    }
}

TEMPLATE_LIST_TEST_CASE("Trees can be initialized using an existing tree node.", "[tree]", AllTreeTypes) {}

TEMPLATE_LIST_TEST_CASE("Trees can be copied and moved.", "[tree]", AllTreeTypes) {}

TEMPLATE_LIST_TEST_CASE("Trees can be checked for null.", "[tree]", AllTreeTypes) {}

TEMPLATE_LIST_TEST_CASE("Trees provide access to the owned tree node.", "[tree]", AllTreeTypes) {}

using AllTreeNodeTypes = std::tuple<dutils::TreeNode<int>,
                                    dutils::TreeNodeOptionalData<int>,
                                    dutils::TreeNodeLeafData<int>,
                                    dutils::TreeNodeOptionalLeafData<int>>;

using AllTreeNodeTypesMaybeConst = maybe_const<dutils::TreeNode<int>,
                                               dutils::TreeNodeOptionalData<int>,
                                               dutils::TreeNodeLeafData<int>,
                                               dutils::TreeNodeOptionalLeafData<int>>;

TEMPLATE_LIST_TEST_CASE("Tree nodes can be default constructed.", "[tree]", AllTreeNodeTypesMaybeConst)
{
    TestType node;

    SECTION("Tree nodes default to having no data iff they have optional data.")
    {
        CHECK(!node.hasData() == TestType::optional_data);
        CHECK(!node == TestType::optional_data);
        CHECK((node.dataOrNull() == nullptr) == TestType::optional_data);
        // data and operator* return references which cannot be null
    }
    SECTION("Tree nodes default to not supporting children iff data is allowed only on leaf nodes.")
    {
        CHECK(!node.supportsChildren() == TestType::leaf_data);
    }
    SECTION("Tree nodes default to being a root node, i.e. not having a parent.")
    {
        CHECK(node.parent() == nullptr);
        CHECK(node.isRoot());
        CHECK(!node.isChild());
        CHECK(&node.root() == &node);
    }
}

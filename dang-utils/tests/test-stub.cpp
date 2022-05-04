#include <tuple>
#include <vector>

#include "dang-utils/catch2-stub-matcher.h"
#include "dang-utils/stub.h"

#include "catch2/catch.hpp"

namespace dutils = dang::utils;

TEST_CASE("Stubs can track their invocations.", "[stub]")
{
    auto stub = dutils::Stub<void(int, int)>();

    SECTION("A newly created stub does not have any invocations.") { CHECK(stub.invocations().empty()); }
    SECTION("Once a stub is called, it will track all invocations.")
    {
        stub(1, 2);
        CHECK(stub.invocations() == std::vector{std::tuple{1, 2}});
        stub(3, 4);
        CHECK(stub.invocations() == std::vector{std::tuple{1, 2}, {3, 4}});

        SECTION("The list of invocations can be cleared again.")
        {
            stub.clear();
            CHECK(stub.invocations().empty());
        }
    }
}

TEST_CASE("Stubs can wrap arbitrary implementations.", "[stub]")
{
    int x = 0;
    auto set_x = [&](int new_x) { x = new_x; };

    auto stub = dutils::Stub<void(int)>(set_x);

    stub(42);

    CHECK(x == 42);
}

TEST_CASE("Stubs can return values from their implementation.", "[stub]")
{
    SECTION("The stub returns whatever its implementation returns.")
    {
        auto stub = dutils::Stub<int()>([] { return 42; });
        CHECK(stub() == 42);
    }
    SECTION("A default constructed stub generates an implementation, that returns a default constructed value.")
    {
        auto stub = dutils::Stub<int()>();
        CHECK(stub() == 0);
    }
    SECTION("Just providing a value generates am implementation, that just returns this value.")
    {
        auto stub = dutils::Stub<int()>(42);
        CHECK(stub() == 42);
    }
}

TEST_CASE("Stubs and their parameters have names.", "[stub]")
{
    auto stub = dutils::Stub<void(int, int)>();

    SECTION("By default a stub is named \"stub\" and all parameter names are empty.")
    {
        CHECK(stub.info().name == "stub");
        CHECK(stub.info().parameters[0] == "");
        CHECK(stub.info().parameters[1] == "");
    }
    SECTION("Names for stubs and their parameters can be changed.")
    {
        stub.setInfo({"my_stub", {"a", "b"}});

        CHECK(stub.info().name == "my_stub");
        CHECK(stub.info().parameters[0] == "a");
        CHECK(stub.info().parameters[1] == "b");
    }
}

using dutils::Matchers::Called;
using dutils::Matchers::CalledWith;
using dutils::Matchers::ignored;
using dutils::Matchers::invocation;

TEST_CASE("Stubs can be assessed thoroughly using Catch2 matchers.", "[stub]")
{
    SECTION("The \"Called\" matcher can be used to count the invocations.")
    {
        auto stub = dutils::Stub<void()>();

        CHECK_THAT(stub, !Called(stub));
        CHECK_THAT(stub, Called(stub, 0));
        CHECK_THAT(stub, !Called(stub, 1));

        stub();

        CHECK_THAT(stub, Called(stub));
        CHECK_THAT(stub, !Called(stub, 0));
        CHECK_THAT(stub, Called(stub, 1));
        CHECK_THAT(stub, !Called(stub, 2));
    }
    SECTION("The \"CalledWith\" matcher can be used to check the arguments.")
    {
        SECTION("The simple form of \"CalledWith\" expects any invocation.")
        {
            auto stub = dutils::Stub<void(int)>();

            CHECK_THAT(stub, !CalledWith(stub, 42));

            stub(42);

            CHECK_THAT(stub, CalledWith(stub, 42));

            stub(256);

            CHECK_THAT(stub, CalledWith(stub, 42));
            CHECK_THAT(stub, CalledWith(stub, 256));
        }
        SECTION("Specific invocations can be checked by index.")
        {
            auto stub = dutils::Stub<void(int)>();

            CHECK_THAT(stub, !CalledWith(stub, invocation(0), 1));
            CHECK_THAT(stub, !CalledWith(stub, invocation(1), 2));

            stub(1);

            CHECK_THAT(stub, CalledWith(stub, invocation(0), 1));
            CHECK_THAT(stub, !CalledWith(stub, invocation(0), 2));
            CHECK_THAT(stub, !CalledWith(stub, invocation(1), 2));

            stub(2);

            CHECK_THAT(stub, CalledWith(stub, invocation(0), 1));
            CHECK_THAT(stub, !CalledWith(stub, invocation(0), 3));
            CHECK_THAT(stub, CalledWith(stub, invocation(1), 2));
            CHECK_THAT(stub, !CalledWith(stub, invocation(1), 4));
        }
        SECTION("Single parameters can be ignored.")
        {
            auto stub = dutils::Stub<void(int, int)>();

            stub(1, 2);

            CHECK_THAT(stub, CalledWith(stub, ignored, ignored));
            CHECK_THAT(stub, CalledWith(stub, 1, ignored));
            CHECK_THAT(stub, CalledWith(stub, ignored, 2));
            CHECK_THAT(stub, CalledWith(stub, ignored, ignored));
            CHECK_THAT(stub, !CalledWith(stub, 3, ignored));
            CHECK_THAT(stub, !CalledWith(stub, ignored, 4));
        }
        SECTION("Parameters can be checked for reference identity by passing it as a pointer.")
        {
            auto stub = dutils::Stub<void(const int&)>();

            int param = 1;
            int other_param = 1;

            stub(param);

            CHECK_THAT(stub, CalledWith(stub, &param));
            CHECK_THAT(stub, !CalledWith(stub, &other_param));

            SECTION("Even though the parameter is a reference, it can still be compared by value.")
            {
                CHECK_THAT(stub, CalledWith(stub, param));
                CHECK_THAT(stub, CalledWith(stub, other_param));
            }
        }
    }
}

#include <cstddef>
#include <memory>

#include "dang-lua/Allocator.h"

#include "catch2/catch.hpp"

namespace dlua = dang::lua;

void* dummyAlloc(void*, void*, std::size_t, std::size_t) { return nullptr; };

TEST_CASE("Lua allocator wraps allocation function and optional userdata.")
{
    SECTION("It can be implicitly converted from an allocation function.")
    {
        constexpr dlua::Allocator allocator = dummyAlloc;
        STATIC_REQUIRE(allocator.function == dummyAlloc);
        STATIC_REQUIRE(allocator.userdata == nullptr);
    }
    SECTION("It can be constructed from allocator and userdata.")
    {
        auto userdata = std::make_unique<int>();
        auto allocator = dlua::Allocator(dummyAlloc, userdata.get());
        CHECK(allocator.function == dummyAlloc);
        CHECK(allocator.userdata == userdata.get());
    }
}

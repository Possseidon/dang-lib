#include "dang-lua/convert/Base.h"

#include "catch.hpp"

namespace dlua = dang::lua;

struct Inconvertible {};

TEST_CASE("Inconvertible types can neither be pushed nor checked.", "[lua][convert][inconvertible][check][push]")
{
    using Convert = dlua::Convert<Inconvertible>;
    STATIC_REQUIRE_FALSE(Convert::can_check);
    STATIC_REQUIRE_FALSE(Convert::can_push);
}

#include "dang-math/vector.h"

#include "catch2/catch.hpp"

namespace dmath = dang::math;

TEST_CASE("Vectors support component-wise operations.")
{
    const dmath::vec3 a(6, 4, 6);
    const dmath::vec3 b(1, 2, 3);

    CAPTURE(a, b);

    CHECK(a + b == dmath::vec3(7, 6, 9));
    CHECK(a - b == dmath::vec3(5, 2, 3));
    CHECK(a * b == dmath::vec3(6, 8, 18));
    CHECK(a / b == dmath::vec3(6, 2, 2));
}

TEST_CASE("Vectors support component-wise compound assignment operations.")
{
    dmath::vec3 a(6, 4, 6);
    const dmath::vec3 b(1, 2, 3);

    CAPTURE(a, b);

    dmath::vec3* a_ptr = nullptr;

    SECTION("a += b")
    {
        a_ptr = &(a += b);
        CHECK(a == dmath::vec3{7, 6, 9});
    }
    SECTION("a -= b")
    {
        a_ptr = &(a -= b);
        CHECK(a == dmath::vec3{5, 2, 3});
    }
    SECTION("a *= b")
    {
        a_ptr = &(a *= b);
        CHECK(a == dmath::vec3{6, 8, 18});
    }
    SECTION("a /= b")
    {
        a_ptr = &(a /= b);
        CHECK(a == dmath::vec3{6, 2, 2});
    }

    INFO("Compound assignment returns reference to a.");
    CHECK(&a == a_ptr);
}

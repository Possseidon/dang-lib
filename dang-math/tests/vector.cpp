#include "dang-math/vector.h"

#include "catch2/catch.hpp"

namespace dmath = dang::math;

TEST_CASE("Vectors default to being zero initialized.", "[vector][initialization]")
{
    CHECK(dmath::vec1() == dmath::vec1(0));
    CHECK(dmath::vec2() == dmath::vec2(0, 0));
    CHECK(dmath::vec3() == dmath::vec3(0, 0, 0));
    CHECK(dmath::vec4() == dmath::vec4(0, 0, 0, 0));
}

TEST_CASE("Vectors can be initialized with a single value, which is used for all components.",
          "[vector][initialization]")
{
    CHECK(dmath::vec1(42) == dmath::vec1(42));
    CHECK(dmath::vec2(42) == dmath::vec2(42, 42));
    CHECK(dmath::vec3(42) == dmath::vec3(42, 42, 42));
    CHECK(dmath::vec4(42) == dmath::vec4(42, 42, 42, 42));
}

TEST_CASE("Vectors can be read using swizzles.", "[vector][access][swizzle]")
{
    const dmath::vec4 a(1, 2, 3, 4);

    CAPTURE(a);

    SECTION("Using singular swizzles.")
    {
        CHECK(a.x() == 1);
        CHECK(a.y() == 2);
        CHECK(a.z() == 3);
        CHECK(a.w() == 4);
    }
    SECTION("Using combined swizzles.")
    {
        CHECK(a.xy() == dmath::vec2(1, 2));
        CHECK(a.xyz() == dmath::vec3(1, 2, 3));
        CHECK(a.xyzw() == a);

        CHECK(a.yx() == dmath::vec2(2, 1));
        CHECK(a.zxy() == dmath::vec3(3, 1, 2));
        CHECK(a.wyzx() == dmath::vec4(4, 2, 3, 1));
    }
}

TEST_CASE("Vectors can be assigned using swizzles.", "[vector][access][swizzle]")
{
    dmath::vec4 a;

    SECTION("Using singular swizzles with direct assignment.")
    {
        a.x() = 1;
        CHECK(a == dmath::vec4(1, 0, 0, 0));
        a.y() = 2;
        CHECK(a == dmath::vec4(1, 2, 0, 0));
        a.z() = 3;
        CHECK(a == dmath::vec4(1, 2, 3, 0));
        a.w() = 4;
        CHECK(a == dmath::vec4(1, 2, 3, 4));
    }
    SECTION("Using combined swizzles with set_<swizzle>.")
    {
        a.set_xy({1, 2});
        CHECK(a == dmath::vec4(1, 2, 0, 0));
        a.set_zx({3, 4});
        CHECK(a == dmath::vec4(4, 2, 3, 0));
        a.set_xyzw({5, 6, 7, 8});
        CHECK(a == dmath::vec4(5, 6, 7, 8));
        a.set_wxzy({1, 2, 3, 4});
        CHECK(a == dmath::vec4(2, 4, 3, 1));
    }
}

TEST_CASE("Vectors support component-wise operations.", "[vector][operators]")
{
    const dmath::vec3 a(6, 4, 6);
    const dmath::vec3 b(1, 2, 3);

    CAPTURE(a, b);

    CHECK(a + b == dmath::vec3(7, 6, 9));
    CHECK(a - b == dmath::vec3(5, 2, 3));
    CHECK(a * b == dmath::vec3(6, 8, 18));
    CHECK(a / b == dmath::vec3(6, 2, 2));
}

TEST_CASE("Vectors support component-wise compound assignment operations.", "[vector][operators]")
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

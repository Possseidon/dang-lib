#include "dang-math/enums.h"
#include "dang-math/utils.h"
#include "dang-math/vector.h"

#include "catch2/catch.hpp"

namespace dmath = dang::math;

using namespace Catch::literals;

TEST_CASE("Vectors default to being zero initialized.", "[vector][initialization]")
{
    CHECK(dmath::vec1() == dmath::vec1(0));
    CHECK(dmath::vec2() == dmath::vec2(0, 0));
    CHECK(dmath::vec3() == dmath::vec3(0, 0, 0));
    CHECK(dmath::vec4() == dmath::vec4(0, 0, 0, 0));
}

TEST_CASE("Vectors can be initialized with a single value.", "[vector][initialization]")
{
    CHECK(dmath::vec1(42) == dmath::vec1(42));
    CHECK(dmath::vec2(42) == dmath::vec2(42, 42));
    CHECK(dmath::vec3(42) == dmath::vec3(42, 42, 42));
    CHECK(dmath::vec4(42) == dmath::vec4(42, 42, 42, 42));
}

TEST_CASE("4-dimensional vectors can be initialized from 3-dimensional vectors.", "[vector][initialization]")
{
    CHECK(dmath::vec4({1, 2, 3}, 4) == dmath::vec4(1, 2, 3, 4));
}

TEST_CASE("Vectors have various conversions.", "[vector][conversion]")
{
    SECTION("Explicit conversion between vectors of same size but different types.")
    {
        dmath::ivec3 ivec(1, 2, 3);
        CHECK(dmath::vec3(ivec) == dmath::vec3(1, 2, 3));
    }
    SECTION("Explicit conversion from single-value vectors to their respective value type.")
    {
        dmath::vec1 value(42);
        CHECK(float(value) == 42);
    }
}

TEST_CASE("Vectors can be read using operator[].", "[vector][access]")
{
    const dmath::vec3 a(1, 2, 3);

    SECTION("Using regular indexing.")
    {
        CHECK(a[0] == 1);
        CHECK(a[1] == 2);
        CHECK(a[2] == 3);
    }
    SECTION("Using the Axis enum.")
    {
        CHECK(a[dmath::Axis3::X] == 1);
        CHECK(a[dmath::Axis3::Y] == 2);
        CHECK(a[dmath::Axis3::Z] == 3);
    }
}

TEST_CASE("Vectors can be assigned using operator[].", "[vector][access]")
{
    dmath::vec3 a;

    SECTION("Using regular indexing.")
    {
        a[0] = 1;
        CHECK(a == dmath::vec3(1, 0, 0));
        a[1] = 2;
        CHECK(a == dmath::vec3(1, 2, 0));
        a[2] = 3;
        CHECK(a == dmath::vec3(1, 2, 3));
    }
    SECTION("Using the Axis enum.")
    {
        a[dmath::Axis3::X] = 1;
        CHECK(a == dmath::vec3(1, 0, 0));
        a[dmath::Axis3::Y] = 2;
        CHECK(a == dmath::vec3(1, 2, 0));
        a[dmath::Axis3::Z] = 3;
        CHECK(a == dmath::vec3(1, 2, 3));
    }
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

TEST_CASE("Vectors support reduce operations.", "[vector][operations]")
{
    const dmath::vec3 a(1, 3, 5);

    CAPTURE(a);

    CHECK(a.sum() == 9);
    CHECK(a.product() == 15);
}

TEST_CASE("Vectors support geometric operations.", "[vector][operations]")
{
    const dmath::vec3 a(1, 3, 5);
    const dmath::vec3 b(2, 5, 8);

    CAPTURE(a, b);

    CHECK(a.dot(b) == 57);
    CHECK(a.sqrdot() == a.dot(a));
    CHECK(a.vectorTo(b) == dmath::vec3(1, 2, 3));
    CHECK(a.reflect({0, 1, 0}) == dmath::vec3(1, -3, 5));

    CHECK(a.length() == 5.91608_a);

    auto a_normalized = a.normalize();
    CAPTURE(a_normalized);
    CHECK(a_normalized.x() == Approx(a.x() / a.length()));
    CHECK(a_normalized.y() == Approx(a.y() / a.length()));
    CHECK(a_normalized.z() == Approx(a.z() / a.length()));

    CHECK(a.distanceTo(b) == Approx(a.vectorTo(b).length()));
}

TEST_CASE("Vectors support angle operations.", "[vector][operations]")
{
    const dmath::vec3 a(1, 0, 0);
    const dmath::vec3 b(1, 1, 0);
    const float angle_deg = 45.0f;
    const float angle_rad = dmath::radians(angle_deg);

    CAPTURE(a, b, angle_deg, angle_rad);

    CHECK(a.cosAngleTo(b) == Approx(std::cos(angle_rad)));
    CHECK(a.radiansTo(b) == Approx(angle_rad));
    CHECK(a.degreesTo(b) == Approx(angle_deg));
}

TEST_CASE("Vectors support unary component-wise operations.", "[vector][operations]")
{
    CHECK(dmath::vec3(-1, -2, -3).abs() == dmath::vec3(1, 2, 3));
    CHECK(dmath::vec3(1.1, 2.5, 3.9).floor() == dmath::vec3(1, 2, 3));
    CHECK(dmath::vec3(1.1, 2.5, 3.9).ceil() == dmath::vec3(2, 3, 4));

    const dmath::vec3 deg1(180, 360, 720);
    const dmath::vec3 rad1 = deg1.radians();

    CAPTURE(deg1, rad1);

    CHECK(rad1.x() == Approx(dmath::radians(deg1.x())));
    CHECK(rad1.y() == Approx(dmath::radians(deg1.y())));
    CHECK(rad1.z() == Approx(dmath::radians(deg1.z())));

    const float pi = dmath::pi_v<float>;
    const dmath::vec3 rad2(pi / 2, pi, pi * 2);
    const dmath::vec3 deg2 = rad2.degrees();

    CAPTURE(deg2, rad2);

    CHECK(deg2.x() == Approx(dmath::degrees(rad2.x())));
    CHECK(deg2.y() == Approx(dmath::degrees(rad2.y())));
    CHECK(deg2.z() == Approx(dmath::degrees(rad2.z())));
}

TEST_CASE("Vectors support binary component-wise operations.", "[vector][operations]")
{
    CHECK(dmath::vec3(1, 2, 3).min({3, 2, 1}) == dmath::vec3(1, 2, 1));
    CHECK(dmath::vec3(1, 2, 3).max({3, 2, 1}) == dmath::vec3(3, 2, 3));
}

TEST_CASE("Vectors support trinary component-wise operations.", "[vector][operations]")
{
    CHECK(dmath::vec3(1, 2, 3).clamp({2, 1, 1}, {3, 3, 2}) == dmath::vec3(2, 2, 2));
}

TEST_CASE("Vectors support component-wise comparison.", "[vector][operations]")
{
    const dmath::vec3 a(1, 2, 3);
    const dmath::vec3 b(3, 2, 1);

    CHECK(a.equal(b) == dmath::bvec3(false, true, false));
    CHECK(a.notEqual(b) == dmath::bvec3(true, false, true));
    CHECK(a.lessThan(b) == dmath::bvec3(true, false, false));
    CHECK(a.lessThanEqual(b) == dmath::bvec3(true, true, false));
    CHECK(a.greaterThan(b) == dmath::bvec3(false, false, true));
    CHECK(a.greaterThanEqual(b) == dmath::bvec3(false, true, true));
}

TEST_CASE("Vectors support full equality comparison.", "[vector][operations]")
{
    CHECK(dmath::vec3(1, 2, 3) == dmath::vec3(1, 2, 3));
    CHECK(dmath::vec3(1, 2, 3) != dmath::vec3(1, 2, 2));
}

TEST_CASE("Vectors support boolean reduce operations.", "[vector][operations]")
{
    const dmath::bvec3 all(true, true, true);
    const dmath::bvec3 one(false, true, false);
    const dmath::bvec3 none(false, false, false);

    CAPTURE(all, one, none);

    CHECK(all.all());
    CHECK(!one.all());
    CHECK(!none.all());

    CHECK(all.any());
    CHECK(one.any());
    CHECK(!none.any());

    CHECK(!all.none());
    CHECK(!one.none());
    CHECK(none.none());
}

TEST_CASE("Vectors support boolean unary component-wise operations.", "[vector][operations]")
{
    const dmath::bvec3 all(true, true, true);
    const dmath::bvec3 one(false, true, false);
    const dmath::bvec3 none(false, false, false);

    CAPTURE(all, one, none);

    CHECK(all == all);
    CHECK(all != one);
    CHECK(all != none);
    CHECK(one != all);
    CHECK(one == one);
    CHECK(one != none);
    CHECK(none != all);
    CHECK(none != one);
    CHECK(none == none);
}

TEST_CASE("Vectors support 2-dimensional operations.", "[vector][operations]")
{
    const dmath::vec2 right(1, 0);
    const dmath::vec2 diag(1, 1);
    const dmath::vec2 up(0, 1);

    CAPTURE(right, diag, up);

    CHECK(dmath::vec2(1, 0).slope() == 0);
    CHECK(dmath::vec2(1, 1).slope() == 1);
    CHECK(dmath::vec2(0, 1).slope() == std::nullopt);

    CHECK(dmath::vec2::fromSlope(0.0f).slope() == 0);
    CHECK(dmath::vec2::fromSlope(1.0f).slope() == 1);
    CHECK(dmath::vec2::fromSlope(std::nullopt).slope() == std::nullopt);

    const dmath::vec2 diag_norm = diag.normalize();
    const dmath::vec2 deg_0 = dmath::vec2::fromDegrees(0);
    const dmath::vec2 deg_45 = dmath::vec2::fromDegrees(45);
    const dmath::vec2 deg_90 = dmath::vec2::fromDegrees(90);

    CAPTURE(diag_norm, deg_0, deg_45, deg_90);

    CHECK(deg_0.x() == Approx(right.x()));
    CHECK(deg_0.y() == Approx(right.y()).margin(1e-7));
    CHECK(deg_45.x() == Approx(diag_norm.x()));
    CHECK(deg_45.y() == Approx(diag_norm.y()));
    CHECK(deg_90.x() == Approx(up.x()).margin(1e-7));
    CHECK(deg_90.y() == Approx(up.y()));

    const dmath::vec2 rad_0 = dmath::vec2::fromRadians(0);
    const dmath::vec2 rad_pi_4 = dmath::vec2::fromRadians(dmath::pi_v<float> / 4);
    const dmath::vec2 rad_pi_2 = dmath::vec2::fromRadians(dmath::pi_v<float> / 2);

    CAPTURE(rad_0, rad_pi_4, rad_pi_2);
    CHECK(rad_0.x() == Approx(deg_0.x()));
    CHECK(rad_0.y() == Approx(deg_0.y()).margin(1e-7));
    CHECK(rad_pi_4.x() == Approx(deg_45.x()));
    CHECK(rad_pi_4.y() == Approx(deg_45.y()));
    CHECK(rad_pi_2.x() == Approx(deg_90.x()).margin(1e-7));
    CHECK(rad_pi_2.y() == Approx(deg_90.y()));

    CHECK(dmath::vec2(1, 2).cross() == dmath::vec2(-2, 1));
    CHECK(dmath::vec2(1, 2).cross(dmath::vec2(2, 3)) == -1);
}

TEST_CASE("Vectors support 3-dimensional operations.", "[vector][operations]")
{
    CHECK(dmath::vec3(1, 0, 0).cross(dmath::vec3(0, 1, 0)) == dmath::vec3(0, 0, 1));
}

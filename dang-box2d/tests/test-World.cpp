#include <cstddef>
#include <filesystem>
#include <optional>
#include <utility>
#include <vector>

#include "dang-box2d/box2d.h"
#include "dang-utils/catch2-stub-matcher.h"

#include "shared/World.h"

namespace b2 = dang::box2d;
namespace dutils = dang::utils;

namespace fs = std::filesystem;

using Catch::UnorderedEquals;
using dutils::Matchers::CalledWith;

TEST_CASE("Box2D worlds can be created.")
{
    SECTION("Defaulting to zero gravity.")
    {
        auto world = World();
        CHECK(world.getGravity() == b2::vec2{});
    }
    SECTION("With a given gravity.")
    {
        auto gravity = b2::vec2{0.0f, -10.0f};
        auto world = World(gravity);
        CHECK(world.getGravity() == gravity);
    }
}

TEST_CASE("Box2D worlds can have a contact filter.")
{
    auto world = World();

    // Spawn two dynamic stacked circles that will cause a contact on the first time step.
    auto circle_shape = b2::CircleShape();
    circle_shape.radius = 1.0f;

    auto body1 = world.createBody(b2::BodyType::Dynamic);
    auto fixture1 = body1.createFixture(circle_shape);
    fixture1.user_data = "1";

    auto body2 = world.createBody(b2::BodyType::Dynamic);
    auto fixture2 = body2.createFixture(circle_shape);
    fixture2.user_data = "2";

    // Register the contact filter.
    auto filter_stub = dutils::Stub<bool(World::Fixture&, World::Fixture&)>();
    world.setContactFilter(filter_stub);

    stepWorld(world);

    CHECK_THAT(filter_stub, CalledWith(filter_stub, &fixture1, &fixture2));
}

TEST_CASE("Box2D worlds can have a debug draw callback.")
{
    World().setDebugDraw(nullptr);
    SUCCEED();
}

TEST_CASE("Box2D worlds can create bodies.")
{
    auto world = World();

    World::Body body;

    SECTION("Using default values.") { body = world.createBody(); }
    SECTION("Only specifying the body type.") { body = world.createBody(b2::BodyType::Static); }
    SECTION("Using a full body definition.") { body = world.createBody(b2::BodyDef()); }

    CHECKED_IF(body) { CHECK(body.getType() == b2::BodyType::Static); }
}

TEST_CASE("Box2D worlds can create joints.")
{
    auto world = World();

    auto body1 = world.createBody();
    body1.user_data = "1";
    auto body2 = world.createBody();
    body2.user_data = "2";

    auto joint_def = World::RevoluteJointDef();
    joint_def.body_a = &body1;
    joint_def.body_b = &body2;
    auto joint = world.createJoint(joint_def);

    CHECKED_IF(joint)
    {
        CHECK(joint.getBodyA() == body1);
        CHECK(joint.getBodyB() == body2);
    }
}

TEST_CASE("Box2D worlds can step the simulation.")
{
    auto world = World({0.0f, -10.0f});

    auto circle_shape = b2::CircleShape();
    circle_shape.radius = 1.0f;

    auto body = world.createBody(b2::BodyType::Dynamic);
    auto fixture = body.createFixture(circle_shape);

    // A ball is dropped from the world origin using gravity.
    CHECK(body.getPosition() == b2::vec2());
    stepWorld(world);
    auto [x, y] = body.getPosition();
    CHECK(x == 0.0f);
    CHECK(y < 0.0f);
}

TEST_CASE("Box2D worlds can clear all forces.")
{
    auto world = World();

    auto body = world.createBody(b2::BodyType::Dynamic);

    auto force = b2::Force();
    force.force = {1.0f, 0.0f};
    force.point = {0.0f, 0.0f};
    body.apply(force);

    // The previously applied force should be cleared and the body should stay still.
    world.clearForces();
    stepWorld(world);
    CHECK(body.getPosition() == b2::vec2());
}

TEST_CASE("Box2D worlds can query fixtures.")
{
    auto world = World();

    auto circle_shape = b2::CircleShape();
    circle_shape.radius = 1.0f;

    auto body = world.createBody();

    auto fixture1 = body.createFixture(circle_shape);
    fixture1.user_data = "1";

    auto fixture2 = body.createFixture(circle_shape);
    fixture2.user_data = "2";

    SECTION("Using an AABB.")
    {
        auto aabb = b2::AABB();
        aabb.lowerBound.Set(-1.0f, -1.0f);
        aabb.upperBound.Set(1.0f, 1.0f);

        auto query_callback = dutils::Stub<bool(World::Fixture&)>(true);
        query_callback.setInfo({"query_callback", {"fixture"}});
        world.queryAABB(query_callback, aabb);

        CHECK_THAT(query_callback, CalledWith(query_callback, &fixture1));
        CHECK_THAT(query_callback, CalledWith(query_callback, &fixture2));
    }
    SECTION("Using a ray cast.")
    {
        auto ray_cast_callback = dutils::Stub<float(World::RayCastData)>(World::RayCastData::next);
        ray_cast_callback.setInfo({"ray_cast_callback", {"ray_cast_data"}});
        world.rayCast(ray_cast_callback, b2::vec2{-2.0f, 0.0f}, b2::vec2{0.0f, 0.0f});

        CHECK_THAT(ray_cast_callback,
                   CalledWith(ray_cast_callback, World::RayCastData{&fixture1, {-1.0f, 0.0f}, {-1.0f, 0.0f}, 0.5f}));
        CHECK_THAT(ray_cast_callback,
                   CalledWith(ray_cast_callback, World::RayCastData{&fixture2, {-1.0f, 0.0f}, {-1.0f, 0.0f}, 0.5f}));
    }
}

TEST_CASE("Box2D can iterate over all bodies, joints and contacts.")
{
    auto world = World();

    SECTION("Iterating over bodies.")
    {
        auto body1 = world.createBody();
        body1.user_data = "1";
        auto body2 = world.createBody();
        body2.user_data = "2";
        auto body3 = world.createBody();
        body3.user_data = "3";

        auto actual_bodies = std::vector(world.bodies().begin(), world.bodies().end());
        auto expected_bodies = std::vector{&body1, &body2, &body3};

        CHECK_THAT(actual_bodies, UnorderedEquals(expected_bodies));
    }
    SECTION("Iterating over joints.")
    {
        auto body1 = world.createBody();
        auto body2 = world.createBody();

        auto joint_def = World::RevoluteJointDef();
        joint_def.body_a = &body1;
        joint_def.body_b = &body2;
        auto joint1 = world.createJoint(joint_def);
        joint1.user_data = "1";
        auto joint2 = world.createJoint(joint_def);
        joint2.user_data = "2";
        auto joint3 = world.createJoint(joint_def);
        joint3.user_data = "3";

        auto actual_joints = std::vector(world.joints().begin(), world.joints().end());
        auto expected_joints = std::vector<World::Joint*>{&joint1, &joint2, &joint3};

        CHECK_THAT(actual_joints, UnorderedEquals(expected_joints));
    }
    SECTION("Iterating over contacts.")
    {
        auto circle_shape = b2::CircleShape();
        circle_shape.radius = 1.0f;

        auto body1 = world.createBody(b2::BodyType::Dynamic);
        auto fixture1 = body1.createFixture(circle_shape);
        auto body2 = world.createBody(b2::BodyType::Dynamic);
        auto fixture2 = body2.createFixture(circle_shape);

        stepWorld(world);
        auto contacts = std::vector(world.contacts().begin(), world.contacts().end());
        CHECK_FALSE(contacts.empty());
    }
}

TEST_CASE("Box2D worlds can query and set various simulation properties.")
{
    auto world = World();

    CHECK(world.getAllowSleeping());
    world.setAllowSleeping(false);
    CHECK_FALSE(world.getAllowSleeping());

    CHECK(world.getWarmStarting());
    world.setWarmStarting(false);
    CHECK_FALSE(world.getWarmStarting());

    CHECK(world.getContinuousPhysics());
    world.setContinuousPhysics(false);
    CHECK_FALSE(world.getContinuousPhysics());

    CHECK_FALSE(world.getSubStepping());
    world.setSubStepping(true);
    CHECK(world.getSubStepping());

    CHECK(world.getAutoClearForces());
    world.setAutoClearForces(false);
    CHECK_FALSE(world.getAutoClearForces());
}

TEST_CASE("Box2D worlds can query the total number of proxies, bodies, joints and contacts.")
{
    auto world = World();

    SECTION("Query the number of proxies.")
    {
        [[maybe_unused]] auto proxy_count = world.getProxyCount();
        SUCCEED();
    }
    SECTION("The total number of bodies.")
    {
        CHECK(world.getBodyCount() == 0);

        auto body1 = world.createBody();
        auto body2 = world.createBody();
        auto body3 = world.createBody();

        CHECK(world.getBodyCount() == 3);
    }
    SECTION("The total number of joints.")
    {
        CHECK(world.getJointCount() == 0);

        auto body1 = world.createBody();
        auto body2 = world.createBody();

        auto joint_def = World::RevoluteJointDef();
        joint_def.body_a = &body1;
        joint_def.body_b = &body2;
        auto joint1 = world.createJoint(joint_def);
        auto joint2 = world.createJoint(joint_def);
        auto joint3 = world.createJoint(joint_def);

        CHECK(world.getJointCount() == 3);
    }
    SECTION("The total number of contacts.")
    {
        CHECK(world.getContactCount() == 0);

        auto circle_shape = b2::CircleShape();
        circle_shape.radius = 1.0f;

        auto body1 = world.createBody(b2::BodyType::Dynamic);
        auto fixture1 = body1.createFixture(circle_shape);
        auto body2 = world.createBody(b2::BodyType::Dynamic);
        auto fixture2 = body2.createFixture(circle_shape);

        stepWorld(world);
        CHECK(world.getContactCount() == 1);
    }
}

TEST_CASE("Box2D worlds can query dynamic tree properties.")
{
    auto world = World();
    [[maybe_unused]] auto tree_height = world.getTreeHeight();
    [[maybe_unused]] auto tree_balance = world.getTreeBalance();
    [[maybe_unused]] auto tree_quality = world.getTreeQuality();
    SUCCEED();
}

TEST_CASE("Box2D worlds can query and modify the gravity.")
{
    auto world = World();

    world.setGravity({0.0f, -10.0f});
    CHECK(world.getGravity() == b2::vec2{0.0f, -10.0f});
}

TEST_CASE("Box2D worlds can check if the simulation is currently being stepped.")
{
    auto world = World();

    auto circle_shape = b2::CircleShape();
    circle_shape.radius = 1.0f;

    auto body1 = world.createBody(b2::BodyType::Dynamic);
    auto fixture1 = body1.createFixture(circle_shape);
    auto body2 = world.createBody(b2::BodyType::Dynamic);
    auto fixture2 = body2.createFixture(circle_shape);

    std::optional<bool> locked_during_contact;
    world.on_begin_contact.append([&] { locked_during_contact = world.isLocked(); });

    CHECK_FALSE(world.isLocked());
    stepWorld(world);
    CHECK_FALSE(world.isLocked());

    CHECK(locked_during_contact.value());
}

TEST_CASE("Box2D worlds can shift their point of origin.")
{
    auto world = World();

    auto body = world.createBody();

    CHECK(body.getPosition() == b2::vec2{0.0f, 0.0f});

    world.shiftOrigin({1.0f, 2.0f});

    CHECK(body.getPosition() == b2::vec2{-1.0f, -2.0f});
}

TEST_CASE("Box2D worlds provide access to their contact manager.")
{
    auto world = World();
    [[maybe_unused]] const auto& contact_manager = world.getContactManager();
    SUCCEED();
}

TEST_CASE("Box2D worlds provide profiling information.")
{
    auto world = World();
    [[maybe_unused]] const auto& profile = world.getProfile();
    SUCCEED();
}

TEST_CASE("Box2D worlds can dump their contents to a file.")
{
    World().dump();
    CHECK(fs::remove("box2d_dump.inl"));
}

TEST_CASE("Box2D worlds have contact events.")
{
    auto world = World({0.0f, -10.0f});

    auto circle_shape = b2::CircleShape();
    circle_shape.radius = 1.0f;

    auto body1 = world.createBody(b2::BodyType::Static);
    auto fixture1 = body1.createFixture(circle_shape);
    fixture1.user_data = "1";
    auto body2 = world.createBody(b2::BodyType::Dynamic);
    auto fixture2 = body2.createFixture(circle_shape);
    fixture2.user_data = "2";

    std::optional<World::Contact> begin_contact;
    world.on_begin_contact.append([&](World::Contact contact) { begin_contact = contact; });

    std::optional<World::Contact> end_contact;
    world.on_end_contact.append([&](World::Contact contact) { end_contact = contact; });

    std::optional<World::Contact> pre_solve;
    world.on_pre_solve.append([&](World::Contact contact, const b2::Manifold*) { pre_solve = contact; });

    std::optional<World::Contact> post_solve;
    world.on_post_solve.append([&](World::Contact contact, const b2::ContactImpulse*) { post_solve = contact; });

    stepWorld(world);

    auto expected_fixtures = std::vector<World::Fixture*>{&fixture1, &fixture2};

    CHECKED_IF(begin_contact)
    {
        auto actual_fixtures = std::vector{&begin_contact->getFixtureA(), &begin_contact->getFixtureB()};
        CHECK_THAT(actual_fixtures, UnorderedEquals(expected_fixtures));
    }
    CHECKED_IF(pre_solve)
    {
        auto actual_fixtures = std::vector{&pre_solve->getFixtureA(), &pre_solve->getFixtureB()};
        CHECK_THAT(actual_fixtures, UnorderedEquals(expected_fixtures));
    }
    CHECKED_IF(post_solve)
    {
        auto actual_fixtures = std::vector{&post_solve->getFixtureA(), &post_solve->getFixtureB()};
        CHECK_THAT(actual_fixtures, UnorderedEquals(expected_fixtures));
    }

    // Wait for the ball to fall and the contact to end.
    for (std::size_t iterations = 0; iterations < 10 && !end_contact; iterations++)
        stepWorld(world);

    CHECKED_IF(end_contact)
    {
        auto actual_fixtures = std::vector{&end_contact->getFixtureA(), &end_contact->getFixtureB()};
        CHECK_THAT(actual_fixtures, UnorderedEquals(expected_fixtures));
    }
}

TEST_CASE("World, bodies and fixtures can be destroyed in any order.")
{
    SECTION("World <- Body <- Fixture")
    {
        World world;
        World::Body body;
        World::Fixture fixture;

        body = world.createBody();
        fixture = body.createFixture(b2::CircleShape());
    }
    SECTION("World <- Fixture <- Body")
    {
        World world;
        World::Fixture fixture;
        World::Body body;

        body = world.createBody();
        fixture = body.createFixture(b2::CircleShape());
    }
    SECTION("Body <- World <- Fixture")
    {
        World::Body body;
        World world;
        World::Fixture fixture;

        body = world.createBody();
        fixture = body.createFixture(b2::CircleShape());
    }
    SECTION("Body <- Fixture <- World")
    {
        World::Body body;
        World::Fixture fixture;
        World world;

        body = world.createBody();
        fixture = body.createFixture(b2::CircleShape());
    }
    SECTION("Fixture <- World <- Body")
    {
        World::Fixture fixture;
        World world;
        World::Body body;

        body = world.createBody();
        fixture = body.createFixture(b2::CircleShape());
    }
    SECTION("Fixture <- Body <- World")
    {
        World::Fixture fixture;
        World::Body body;
        World world;

        body = world.createBody();
        fixture = body.createFixture(b2::CircleShape());
    }

    SUCCEED();
}

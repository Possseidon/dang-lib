#pragma once

#include <string>

#include "dang-box2d/box2d.h"

#include "catch2/catch_tostring.hpp"

struct Data {
    using Fixture = std::string;
    using Body = std::string;
    using Joint = std::string;
};

using World = dang::box2d::World<Data>;

namespace Catch {

template <>
struct StringMaker<World::Fixture> {
    static std::string convert(const World::Fixture& fixture)
    {
        return fixture ? "Fixture(" + fixture.user_data + ")" : "Fixture(null)";
    }
};

template <>
struct StringMaker<World::Fixture*> {
    static std::string convert(const World::Fixture* fixture)
    {
        return fixture ? "Fixture*(" + fixture->user_data + ")" : "Fixture*(null)";
    }
};

template <>
struct StringMaker<const World::Fixture*> : StringMaker<World::Fixture*> {};

template <>
struct StringMaker<World::Body> {
    static std::string convert(const World::Body& body) { return body ? "Body(" + body.user_data + ")" : "Body(null)"; }
};

template <>
struct StringMaker<World::Body*> {
    static std::string convert(const World::Body* body)
    {
        return body ? "Body*(" + body->user_data + ")" : "Body*(null)";
    }
};

template <>
struct StringMaker<const World::Body*> : StringMaker<World::Body*> {};

template <>
struct StringMaker<World::Joint> {
    static std::string convert(const World::Joint& joint)
    {
        return joint ? "Joint(" + joint.user_data + ")" : "Joint(null)";
    }
};

template <>
struct StringMaker<World::Joint*> {
    static std::string convert(const World::Joint* joint)
    {
        return joint ? "Joint*(" + joint->user_data + ")" : "Joint*(null)";
    }
};

template <>
struct StringMaker<const World::Joint*> : StringMaker<World::Joint*> {};

template <>
struct StringMaker<World::RayCastData> {
    static std::string convert(const World::RayCastData& data)
    {
        using namespace std::literals;
        return "RayCastData("s + StringMaker<World::Fixture*>::convert(data.fixture) + " "s + data.point.format() +
               " normal: " + data.normal.format() + " fraction: " + std::to_string(data.fraction) + ")"s;
    }
};

} // namespace Catch

void stepWorld(World& world)
{
    auto time_step = 1.0f / 60.0f;
    auto velocity_iterations = 6;
    auto position_iterations = 2;
    world.step(time_step, velocity_iterations, position_iterations);
}

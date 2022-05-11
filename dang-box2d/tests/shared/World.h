#pragma once

#include <string>

#include "dang-box2d/box2d.h"

#include "catch2/catch.hpp"

struct Data {
    using Fixture = const char;
    using Body = const char;
    using Joint = const char;
};

using World = dang::box2d::World<Data>;

namespace Catch {

template <>
struct StringMaker<World::Fixture> {
    static std::string convert(World::Fixture fixture)
    {
        using namespace std::literals;
        if (fixture)
            return "Fixture("s + (fixture.getUserData() ? fixture.getUserData() : ""s) + ")"s;
        return "Fixture(null)"s;
    }
};

template <>
struct StringMaker<World::BodyRef> {
    static std::string convert(World::BodyRef body)
    {
        using namespace std::literals;
        return body ? "Body("s + (body.getUserData() ? body.getUserData() : ""s) + ")"s : "Body(null)";
    }
};

template <>
struct StringMaker<World::JointRef> {
    static std::string convert(World::JointRef joint)
    {
        using namespace std::literals;
        return joint ? "Joint("s + (joint.getUserData() ? joint.getUserData() : ""s) + ")"s : "Joint(null)";
    }
};

template <>
struct StringMaker<World::RayCastData> {
    static std::string convert(const World::RayCastData& data)
    {
        using namespace std::literals;
        return "RayCastData("s + StringMaker<World::Fixture>::convert(data.fixture) + " "s + data.point.format() +
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

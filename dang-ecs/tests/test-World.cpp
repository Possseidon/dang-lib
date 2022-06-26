#include <iostream>

#include "dang-ecs/World.h"

#include "catch2/catch_test_macros.hpp"

namespace decs = dang::ecs;

struct Health {
    int current;
    int max;
};

struct Position {
    int pos;
};

struct Caption {
    std::string text;
};

struct Alive {};
struct Dead {};

using MyComponents = decs::Components<Position, Caption, Health, Alive, Dead>;
using MyWorld = decs::World<MyComponents>;

decs::Entity createPlayer(MyWorld& world, int pos) { return world.spawn(Position{pos}, Health{10, 10}, Alive{}); }

#include <bitset>

TEST_CASE()
{
    boost::dynamic_bitset<> a(2);
    boost::dynamic_bitset<> b(2);
    a.set(0);
    b.set(1);

    CHECK(a < b);
    return;

    decs::EntitiesBitset entities;
    for (std::size_t i = 0; i < 30; i += 3)
        entities.insert(decs::Entity{i});

    decs::EntitiesBitset other_entities;
    for (std::size_t i = 0; i < 30; i += 2)
        other_entities.insert(decs::Entity{i});

    decs::Entities magic;
    for (std::size_t i = 0; i < 30; i += 5)
        magic.insert(decs::Entity{i});

    CAPTURE(entities < magic);
    CAPTURE(entities.size());
    CAPTURE(other_entities.size());
    FAIL();
}

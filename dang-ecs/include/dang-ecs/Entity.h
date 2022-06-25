#pragma once

#include <cstddef>
#include <limits>

#include "dang-ecs/global.h"

namespace dang::ecs {

struct Entity {
    using ID = std::size_t;
    static constexpr ID invalid_id = std::numeric_limits<ID>::max();

    ID id = invalid_id;

    constexpr explicit operator bool() { return id != invalid_id; }

    constexpr friend auto operator<=>(const Entity&, const Entity&) = default;
};

static constexpr Entity invalid_entity;

} // namespace dang::ecs

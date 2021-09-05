#include <algorithm>
#include <cstddef>

#include "dang-math/consts.h"
#include "dang-math/enums.h"
#include "dang-math/marchingcubes.h"
#include "dang-math/vector.h"

#include "dang-utils/enum.h"

#include "catch2/catch.hpp"

namespace dmath = dang::math;
namespace dutils = dang::utils;

using Catch::Generators::range;

TEMPLATE_TEST_CASE("MarchingCubes generates a valid configuration of plane informations.",
                   "[marchingcubes]",
                   dmath::MarchingCubes<>,
                   dmath::MarchingCubes<true>)
{
    TestType mc;

    auto corners_bits = GENERATE(range(std::size_t{0}, dmath::Corners3::allValues().toBits<std::size_t>() + 1));
    auto corners = dmath::Corners3::fromBits(corners_bits);
    auto plane_infos = mc[corners];

    SECTION("Plane information can be inspected directly.")
    {
        for (const auto& plane_info : plane_infos) {
            for (const auto& point : plane_info.points) {
                if (point.corner == dmath::Corner3::None) {
                    UNSCOPED_INFO("Only the specialized version should contain center values.");
                    CHECK(mc.with_center);

                    UNSCOPED_INFO("All center values should be at least 0.1666 from 0.0.");
                    CHECK(point.position.minValue() > 0.1666666f);

                    UNSCOPED_INFO("All center values should be at least 0.1666 from 1.0.");
                    CHECK(point.position.maxValue() < 0.8333334f);

                    UNSCOPED_INFO("Center points should not move with varying offsets.");
                    CHECK(point.direction == 0.0f);
                }
                else {
                    UNSCOPED_INFO("The point's corner should be part of the initial corner set.");
                    CHECK(corners.contains(point.corner));

                    auto corner_position = dmath::vec3(dmath::corner_vector_3[point.corner]);
                    UNSCOPED_INFO("The point's position should that of its assigned corner.");
                    CHECK(point.position == corner_position);

                    auto opposite_position = dmath::ivec3(point.position + point.direction);
                    auto is_opposite_corner = [&](auto corner) {
                        return dmath::corner_vector_3[corner] == opposite_position;
                    };
                    auto opposite_corner = *std::find_if(begin(dutils::enumerate<dmath::Corner3>),
                                                         end(dutils::enumerate<dmath::Corner3>),
                                                         is_opposite_corner);
                    UNSCOPED_INFO("The point's opposite corner should not be part of the initial corner set.");
                    CHECK_FALSE(corners.contains(opposite_corner));
                }
            }
        }
    }
    SECTION("Plane information can generate planes with varying offsets.")
    {
        auto offset = GENERATE(0.0f, 0.25f, 0.5f);
        for (const auto& plane_info : plane_infos) {
            auto plane = plane_info.makePlane(offset);
            const auto& points = plane_info.points;
            // These should be approximates, but the chosen offsets seem to work anyway.
            // This should also only be a problem, when a center point exists.
            CHECK(plane[{0, 0}] == points[0].position + points[0].direction * offset);
            CHECK(plane[{1, 0}] == points[1].position + points[1].direction * offset);
            CHECK(plane[{0, 1}] == points[2].position + points[2].direction * offset);
        }
    }
}

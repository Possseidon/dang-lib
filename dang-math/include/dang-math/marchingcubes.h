#pragma once

#include "dang-math/enums.h"
#include "dang-math/geometry.h"
#include "dang-math/vector.h"

#include "dang-utils/enum.h"

#include "global.h"

namespace dang::math {

template <bool v_with_center = false>
class MarchingCubes {
public:
    static constexpr auto with_center = v_with_center;

    template <typename T, std::size_t v_max_size>
    struct LimitedVector {
    public:
        using iterator = T*;
        using const_iterator = const T*;

        LimitedVector() = default;

        template <typename... TArgs>
        constexpr LimitedVector(const TArgs&... items)
            : items_{items...}
            , size_(sizeof...(TArgs))
        {}

        constexpr auto size() const { return size_; }
        constexpr auto max_size() const { return v_max_size; }
        constexpr auto empty() const { return size_ == 0; }

        constexpr iterator begin() { return &items_[0]; }
        constexpr iterator end() { return begin() + size_; }

        constexpr const_iterator begin() const { return &items_[0]; }
        constexpr const_iterator end() const { return begin() + size_; }

        constexpr const_iterator cbegin() const { return begin(); }
        constexpr const_iterator cend() const { return end(); }

        constexpr auto& operator[](std::size_t index) { return *(begin() + index); }
        constexpr const auto& operator[](std::size_t index) const { return *(begin() + index); }

        constexpr auto& front() { return *begin(); }
        constexpr auto& back() { return *(end() - 1); }

        constexpr const auto& front() const { return *begin(); }
        constexpr const auto& back() const { return *(end() - 1); }

        constexpr void push_back(const T& item) { items_[size_++] = item; }
        constexpr void pop_back() { size_--; }

        template <typename... TArgs>
        constexpr T& emplace_back(const TArgs&... args)
        {
            push_back(T(args...));
            return back();
        }

        constexpr auto erase(const_iterator pos) { return erase(pos, pos + 1); }
        constexpr auto erase(const_iterator first, const_iterator last)
        {
            auto mut_first = begin() + (first - cbegin());
            auto dst = mut_first;
            for (auto iter = last; iter != cend(); iter++, dst++)
                *dst = *iter;
            size_ -= last - first;
            return mut_first;
        }
        constexpr auto clear() { size_ = 0; }

    private:
        T items_[v_max_size];
        std::size_t size_ = 0;
    };

    struct PlanePoint {
        PlanePoint() = default;

        constexpr PlanePoint(vec3 position, vec3 direction = {}, Corner3 corner = Corner3::None)
            : position(position)
            , direction(direction)
            , corner(corner)
        {}

        constexpr PlanePoint(Corner3 corner, vec3 direction)
            : PlanePoint(vec3{corner_vector_3[corner]}, direction, corner)
        {}

        constexpr PlanePoint(Corner3 corner, Axis3 axis)
        {
            ivec3 pos = corner_vector_3[corner];
            ivec3 dir;
            dir[axis] = 1 - pos[axis] * 2;
            *this = PlanePoint{vec3{pos}, vec3{dir}, corner};
        }

        static constexpr PlanePoint inverted(Corner3 corner, Axis3 axis)
        {
            PlanePoint result{corner, axis};
            result.corner = static_cast<Corner3>(static_cast<int>(result.corner) ^ (1 << static_cast<int>(axis)));
            return result;
        }

        constexpr friend bool operator==(const PlanePoint& lhs, const PlanePoint& rhs)
        {
            return std::tie(lhs.position, lhs.direction) == std::tie(rhs.position, rhs.direction);
        }

        constexpr friend bool operator!=(const PlanePoint& lhs, const PlanePoint& rhs) { return !(lhs == rhs); }

        vec3 position;
        vec3 direction;
        Corner3 corner = Corner3::None;
    };

    struct PlaneInfo {
        std::array<PlanePoint, 3> points;

        constexpr auto makePlane(float offset = 0.5) const
        {
            return Plane3(points[0].position + offset * points[0].direction,
                          {{points[0].position.vectorTo(points[1].position) +
                                offset * (points[1].direction - points[0].direction),
                            points[0].position.vectorTo(points[2].position) +
                                offset * (points[2].direction - points[0].direction)}});
        }
    };

    using Planes = LimitedVector<PlaneInfo, with_center ? 12 : 5>;
    using Lookup = std::array<Planes, 256>;

    constexpr const auto& operator[](Corners3 corners) const { return lookup_[corners.toBits<std::size_t>()]; }

private:
    struct Line {
        PlanePoint start;
        PlanePoint stop;

        Line() = default;

        constexpr Line(bool flip, bool invert, PlanePoint start, PlanePoint stop)
            : start(flip ? stop : start)
            , stop(flip ? start : stop)
        {
            if (invert) {
                this->start.position += this->start.direction;
                this->start.direction = -this->start.direction;
                this->stop.position += this->stop.direction;
                this->stop.direction = -this->stop.direction;
            }
        }
    };

    using Lines = LimitedVector<Line, 12>;
    using Loop = LimitedVector<PlanePoint, 7>;
    using Loops = LimitedVector<Loop, 4>;

    static constexpr auto generateLines(Corners3 corners)
    {
        auto count_windings = [](Corner3 corner) { return corner_vector_3[corner].sum() % 2 == 0; };

        auto corners_connected = [](Corner3 a, Corner3 b) {
            return corner_vector_3[a].vectorTo(corner_vector_3[b]).sum() == 1;
        };

        Lines lines;
        for (auto dir : dutils::enumerate<Facing3>) {
            auto masked_corners = corners & facing_corners_3[dir];
            auto right = static_cast<Axis3>((static_cast<int>(facing_axis_3[dir]) + 2) % 3);
            auto up = static_cast<Axis3>((static_cast<int>(facing_axis_3[dir]) + 1) % 3);

            switch (masked_corners.size()) {
            case 1: {
                auto corner = masked_corners.front();
                count_windings(corner);
                lines.emplace_back(count_windings(corner), false, PlanePoint(corner, right), PlanePoint(corner, up));
            } break;
            case 2: {
                auto corner1 = masked_corners.front();
                auto corner2 = masked_corners.back();
                if (corners_connected(corner1, corner2)) {
                    auto normal = facing_vector_3[dir].cross(corner_vector_3[corner2] - corner_vector_3[corner1]);
                    auto flipped = facing_vector_3[dir]
                                       .cross(corner_vector_3[corner2] + corner_vector_3[corner1] - 1)
                                       .lessThanEqual(0)
                                       .all();
                    if (flipped)
                        normal = -normal;
                    lines.emplace_back(
                        flipped, false, PlanePoint(corner1, vec3{normal}), PlanePoint(corner2, vec3{normal}));
                }
                else {
                    lines.emplace_back(
                        count_windings(corner1), false, PlanePoint(corner1, right), PlanePoint(corner1, up));
                    lines.emplace_back(
                        count_windings(corner2), false, PlanePoint(corner2, right), PlanePoint(corner2, up));
                }
            } break;
            case 3: {
                auto corner = (facing_corners_3[dir] - corners).front();
                lines.emplace_back(!count_windings(corner),
                                   true,
                                   PlanePoint::inverted(corner, right),
                                   PlanePoint::inverted(corner, up));
            } break;
            }
        }
        return lines;
    }

    static constexpr auto generateLoops(Corners3 corners)
    {
        Lines lines = generateLines(corners);
        Loops loops;
        while (!lines.empty()) {
            const auto& line = lines.back();
            Loop& loop = loops.emplace_back(Loop{line.start, line.stop});
            lines.pop_back();
            bool done = false;
            do {
                for (auto line_iter = lines.begin(); line_iter != lines.end(); line_iter++) {
                    auto other_line = *line_iter;
                    if (other_line.start == loop.back()) {
                        done = other_line.stop == loop.front();
                        if (!done)
                            loop.push_back(other_line.stop);
                        lines.erase(line_iter);
                        break;
                    }
                }
            } while (!done);
        }
        return loops;
    }

    static constexpr auto generatePlanes(Corners3 corners)
    {
        Planes result;
        for (const auto& loop : generateLoops(corners)) {
            if constexpr (with_center) {
                vec3 center;
                for (const auto& point : loop)
                    center += point.position + point.direction / 2;
                center /= static_cast<float>(loop.size());

                for (std::size_t i = 0; i < loop.size() - 1; i++)
                    result.push_back(PlaneInfo{center, loop[i], loop[i + 1]});
                result.push_back(PlaneInfo{center, loop.back(), loop.front()});
            }
            else {
                for (std::size_t i = 1; i < loop.size() - 1; i++)
                    result.push_back(PlaneInfo{loop[0], loop[i], loop[i + 1]});
            }
        }
        return result;
    }

    static constexpr auto generateLookup()
    {
        Lookup result;
        for (std::size_t i = 0; i < result.size(); i++)
            result[i] = generatePlanes(Corners3::fromBits(i));
        return result;
    }

    Lookup lookup_ = generateLookup();
};

} // namespace dang::math

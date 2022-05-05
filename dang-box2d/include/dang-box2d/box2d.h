#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "dang-math/vector.h"
#include "dang-utils/event.h"
#include "dang-utils/utils.h"

#include "box2d/box2d.h"

// TODO: C++20 put requires clauses on non-const wrapper functions.

namespace dang::box2d {

// --- Math

namespace dmath = dang::math;
namespace dutils = dang::utils;

using dmath::vec2;
using dmath::vec3;

inline vec2 cast(b2Vec2 vec) { return {vec.x, vec.y}; }
inline b2Vec2 cast(vec2 vec) { return {vec.x(), vec.y()}; }

inline vec3 cast(b2Vec3 vec) { return {vec.x, vec.y, vec.z}; }
inline b2Vec3 cast(vec3 vec) { return {vec.x(), vec.y(), vec.z()}; }

inline std::vector<b2Vec2> cast(const std::vector<vec2>& vertices)
{
    std::vector<b2Vec2> vertices_data;
    vertices_data.reserve(vertices.size());
    std::transform(
        vertices.begin(), vertices.end(), std::back_inserter(vertices_data), [](const vec2& vec) { return cast(vec); });
    return vertices_data;
}

// --- Enums

enum class ShapeType {
    Unknown = -1,
    Circle,
    Edge,
    Polygon,
    Chain,

    COUNT
};

enum class JointType {
    Unknown,
    Revolute,
    Prismatic,
    Distance,
    Pulley,
    Mouse,
    Gear,
    Wheel,
    Weld,
    Friction,
    Rope, // removed
    Motor,

    COUNT
};

enum class BodyType {
    Static,
    Kinematic,
    Dynamic,

    COUNT
};

namespace detail {

template <typename TShape>
struct shape_type;

template <typename TShape>
inline constexpr auto shape_type_v = shape_type<TShape>::value;

template <>
struct shape_type<b2Shape> : dutils::constant<ShapeType::Unknown> {};

template <>
struct shape_type<b2CircleShape> : dutils::constant<ShapeType::Circle> {};

template <>
struct shape_type<b2EdgeShape> : dutils::constant<ShapeType::Edge> {};

template <>
struct shape_type<b2PolygonShape> : dutils::constant<ShapeType::Polygon> {};

template <>
struct shape_type<b2ChainShape> : dutils::constant<ShapeType::Chain> {};

template <ShapeType v_shape_type>
struct shape_b2type;

template <ShapeType v_shape_type>
using shape_b2type_t = typename shape_b2type<v_shape_type>::type;

template <>
struct shape_b2type<ShapeType::Unknown> {
    using type = b2Shape;
};

template <>
struct shape_b2type<ShapeType::Circle> {
    using type = b2CircleShape;
};

template <>
struct shape_b2type<ShapeType::Edge> {
    using type = b2EdgeShape;
};

template <>
struct shape_b2type<ShapeType::Polygon> {
    using type = b2PolygonShape;
};

template <>
struct shape_b2type<ShapeType::Chain> {
    using type = b2ChainShape;
};

template <typename TJoint>
struct joint_type;

template <typename TJoint>
inline constexpr auto joint_type_v = joint_type<TJoint>::value;

template <>
struct joint_type<b2Joint> : dutils::constant<JointType::Unknown> {};

template <>
struct joint_type<b2RevoluteJoint> : dutils::constant<JointType::Revolute> {};

template <>
struct joint_type<b2PrismaticJoint> : dutils::constant<JointType::Prismatic> {};

template <>
struct joint_type<b2DistanceJoint> : dutils::constant<JointType::Distance> {};

template <>
struct joint_type<b2PulleyJoint> : dutils::constant<JointType::Pulley> {};

template <>
struct joint_type<b2MouseJoint> : dutils::constant<JointType::Mouse> {};

template <>
struct joint_type<b2GearJoint> : dutils::constant<JointType::Gear> {};

template <>
struct joint_type<b2WheelJoint> : dutils::constant<JointType::Wheel> {};

template <>
struct joint_type<b2WeldJoint> : dutils::constant<JointType::Weld> {};

template <>
struct joint_type<b2FrictionJoint> : dutils::constant<JointType::Friction> {};

template <>
struct joint_type<b2MotorJoint> : dutils::constant<JointType::Motor> {};

template <typename TJoint>
struct joint_def_result_type;

template <typename TJoint>
using joint_def_result_type_t = typename joint_def_result_type<TJoint>::type;

template <>
struct joint_def_result_type<b2RevoluteJointDef> {
    using type = b2RevoluteJoint;
};

template <>
struct joint_def_result_type<b2PrismaticJointDef> {
    using type = b2PrismaticJoint;
};

template <>
struct joint_def_result_type<b2DistanceJointDef> {
    using type = b2DistanceJoint;
};

template <>
struct joint_def_result_type<b2PulleyJointDef> {
    using type = b2PulleyJoint;
};

template <>
struct joint_def_result_type<b2MouseJointDef> {
    using type = b2MouseJoint;
};

template <>
struct joint_def_result_type<b2GearJointDef> {
    using type = b2GearJoint;
};

template <>
struct joint_def_result_type<b2WheelJointDef> {
    using type = b2WheelJoint;
};

template <>
struct joint_def_result_type<b2WeldJointDef> {
    using type = b2WeldJoint;
};

template <>
struct joint_def_result_type<b2FrictionJointDef> {
    using type = b2FrictionJoint;
};

template <>
struct joint_def_result_type<b2MotorJointDef> {
    using type = b2MotorJoint;
};

} // namespace detail

constexpr auto cast(b2Shape::Type type) { return static_cast<ShapeType>(type); }
constexpr auto cast(ShapeType type) { return static_cast<b2Shape::Type>(type); }

constexpr auto cast(b2JointType type) { return static_cast<JointType>(type); }
constexpr auto cast(JointType type) { return static_cast<b2JointType>(type); }

constexpr auto cast(b2BodyType type) { return static_cast<BodyType>(type); }
constexpr auto cast(BodyType type) { return static_cast<b2BodyType>(type); }

// --- Iterator

template <typename T>
class ForwardIterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::forward_iterator_tag;

    ForwardIterator(T current = {})
        : current_(current)
    {}

    ForwardIterator& operator++()
    {
        current_ = current_.getNext();
        return *this;
    }

    ForwardIterator operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    bool operator==(ForwardIterator other) const { return current_ == other.current_; }
    bool operator!=(ForwardIterator other) const { return !(*this == other); }

    T& operator*() { return current_; }
    const T& operator*() const { return current_; }
    T* operator->() { return &current_; }
    const T* operator->() const { return &current_; }

private:
    T current_;
};

template <typename T>
class ForwardIterable {
public:
    ForwardIterable(T first)
        : first_(first)
    {}

    ForwardIterator<T> begin() const { return {first_}; }
    ForwardIterator<T> end() const { return {}; }

private:
    T first_;
};

template <typename T>
class BidirectionalIterator : public ForwardIterator<T> {
public:
    BidirectionalIterator& operator--()
    {
        **this = (*this)->getPrev();
        return *this;
    }

    BidirectionalIterator operator--(int)
    {
        auto temp = *this;
        --*this;
        return temp;
    }
};

template <typename T>
class BidirectionalIterable {
public:
    BidirectionalIterable(T first)
        : first_(first)
    {}

    BidirectionalIterator<T> begin() const { return {first_}; }
    BidirectionalIterator<T> end() const { return {}; }

private:
    T first_;
};

namespace detail {

template <typename TVector>
class VectorIteratorHelper;

template <typename TVector>
class VectorReference {
public:
    vec2 value() const { return cast(*vec_); }
    operator vec2() const { return value(); }

    VectorReference& operator=(vec2 vec)
    {
        *vec_ = cast(vec);
        return *this;
    }

private:
    template <typename>
    friend class VectorIteratorHelper;

    template <typename>
    friend class VectorsHelper;

    VectorReference(TVector* vec = nullptr)
        : vec_(vec)
    {}

    TVector* vec_;
};

template <typename TVector>
class VectorsHelper;

template <typename TVector>
class VectorIteratorHelperBase;

template <>
class VectorIteratorHelperBase<b2Vec2> {
public:
    using reference = VectorReference<b2Vec2>;
};

template <>
class VectorIteratorHelperBase<const b2Vec2> {
public:
    using reference = vec2;
};

template <typename TVector>
class VectorIteratorHelper : public VectorIteratorHelperBase<TVector> {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = vec2;
    using reference = typename VectorIteratorHelperBase<TVector>::reference;
    using pointer = reference*;
    using iterator_category = std::random_access_iterator_tag;

    VectorIteratorHelper() = default;

    VectorIteratorHelper& operator++()
    {
        current_.vec_++;
        return *this;
    }

    VectorIteratorHelper operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    VectorIteratorHelper& operator--()
    {
        current_.vec_--;
        return *this;
    }

    VectorIteratorHelper operator--(int)
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    VectorIteratorHelper& operator+=(difference_type diff)
    {
        current_.vec_ += diff;
        return *this;
    }

    VectorIteratorHelper& operator-=(difference_type diff)
    {
        current_.vec_ -= diff;
        return *this;
    }

    VectorIteratorHelper operator+(difference_type diff) const { return current_.vec_ + diff; }
    friend VectorIteratorHelper operator+(difference_type diff, VectorIteratorHelper iter) { return iter + diff; }
    VectorIteratorHelper operator-(difference_type diff) const { return current_.vec_ - diff; }

    difference_type operator-(VectorIteratorHelper other) const { return current_.vec_ - other.current_.vec_; }

    reference operator[](difference_type diff) const { return current_.vec_ + diff; }

    bool operator==(VectorIteratorHelper other) const { return current_.vec_ == other.current_.vec_; }
    bool operator!=(VectorIteratorHelper other) const { return !(*this == other); }
    bool operator<(VectorIteratorHelper other) const { return current_.vec_ < other.current_.vec_; }
    bool operator<=(VectorIteratorHelper other) const { return !(*this > other); }
    bool operator>(VectorIteratorHelper other) const { return other < *this; }
    bool operator>=(VectorIteratorHelper other) const { return !(*this < other); }

    reference operator*() const { return current_; }
    pointer operator->() const { return &current_; }

private:
    template <typename>
    friend class VectorsHelper;

    VectorIteratorHelper(TVector* current)
        : current_(current)
    {}

    VectorReference<TVector> current_;
};

template <typename TVector>
class VectorsHelper {
private:
    static constexpr bool is_const = std::is_const_v<TVector>;

public:
    using value_type = vec2;
    using reference = VectorReference<TVector>;
    using const_reference = value_type;
    using iterator = VectorIteratorHelper<TVector>;
    using const_iterator = VectorIteratorHelper<const TVector>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    VectorsHelper() = default;

    auto begin() const { return std::conditional_t<is_const, const_iterator, iterator>{vectors_}; }
    auto end() const { return std::conditional_t<is_const, const_iterator, iterator>{vectors_ + count_}; }

    const_iterator const cbegin() const { return vectors_; }
    const_iterator const cend() const { return vectors_ + count_; }

    bool operator==(const VectorsHelper& other) const { return std::equal(begin(), end(), other.begin(), other.end()); }

    bool operator!=(const VectorsHelper& other) const { return !(*this == other); }

    bool operator<(const VectorsHelper& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    bool operator<=(const VectorsHelper& other) const { return !(*this > other); }
    bool operator>(const VectorsHelper& other) const { return other < *this; }
    bool operator>=(const VectorsHelper& other) const { return !(*this < other); }

    std::size_t size() const { return static_cast<std::size_t>(count_); }
    std::size_t max_size() const { return size(); }
    bool empty() const { return size() == 0; }

    auto operator[](size_type index) const
    {
        if constexpr (is_const)
            return cast(vectors_[index]);
        else
            return reference{vectors_ + index};
    }

    auto front() const
    {
        if constexpr (is_const)
            return cast(vectors_[0]);
        else
            return reference{vectors_};
    }

    auto back() const
    {
        if constexpr (is_const)
            return cast(vectors_[count_ - 1]);
        else
            return reference{vectors_ + count_ - 1};
    }

private:
    template <typename, ShapeType>
    friend class ShapeRefWrapper;

    VectorsHelper(TVector* vectors, int32 count)
        : vectors_(vectors)
        , count_(count)
    {}

    TVector* vectors_ = nullptr;
    int32 count_ = 0;
};

} // namespace detail

using Vectors = detail::VectorsHelper<b2Vec2>;
using ConstVectors = detail::VectorsHelper<const b2Vec2>;

// --- Forward Declarations

namespace detail {

template <typename TUserData, typename TJoint, JointType v_type = joint_type_v<std::remove_const_t<TJoint>>>
class JointWrapper;

template <typename TUserData, typename TJointEdge>
class JointEdgeWrapper;

template <typename TShape, ShapeType v_type = shape_type_v<std::remove_const_t<TShape>>>
class ShapeRefWrapper;

template <typename TUserData, typename TFixture, typename TShape>
class FixtureWrapper;

template <typename TUserData, typename TBody>
class BodyWrapper;

template <typename TUserData, typename TWorld>
class WorldRefWrapper;

template <typename TUserData, typename TContact>
class ContactWrapper;

template <typename TUserData>
using Joint = detail::JointWrapper<TUserData, b2Joint>;
template <typename TUserData>
using ConstJoint = detail::JointWrapper<TUserData, const b2Joint>;
template <typename TUserData, typename TConstAs>
using JointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2Joint, TConstAs>>;

template <typename TUserData>
using RevoluteJoint = detail::JointWrapper<TUserData, b2RevoluteJoint>;
template <typename TUserData>
using ConstRevoluteJoint = detail::JointWrapper<TUserData, const b2RevoluteJoint>;
template <typename TUserData, typename TConstAs>
using RevoluteJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2RevoluteJoint, TConstAs>>;

template <typename TUserData>
using PrismaticJoint = detail::JointWrapper<TUserData, b2PrismaticJoint>;
template <typename TUserData>
using ConstPrismaticJoint = detail::JointWrapper<TUserData, const b2PrismaticJoint>;
template <typename TUserData, typename TConstAs>
using PrismaticJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2PrismaticJoint, TConstAs>>;

template <typename TUserData>
using DistanceJoint = detail::JointWrapper<TUserData, b2DistanceJoint>;
template <typename TUserData>
using ConstDistanceJoint = detail::JointWrapper<TUserData, const b2DistanceJoint>;
template <typename TUserData, typename TConstAs>
using DistanceJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2DistanceJoint, TConstAs>>;

template <typename TUserData>
using PulleyJoint = detail::JointWrapper<TUserData, b2PulleyJoint>;
template <typename TUserData>
using ConstPulleyJoint = detail::JointWrapper<TUserData, const b2PulleyJoint>;
template <typename TUserData, typename TConstAs>
using PulleyJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2PulleyJoint, TConstAs>>;

template <typename TUserData>
using MouseJoint = detail::JointWrapper<TUserData, b2MouseJoint>;
template <typename TUserData>
using ConstMouseJoint = detail::JointWrapper<TUserData, const b2MouseJoint>;
template <typename TUserData, typename TConstAs>
using MouseJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2MouseJoint, TConstAs>>;

template <typename TUserData>
using GearJoint = detail::JointWrapper<TUserData, b2GearJoint>;
template <typename TUserData>
using ConstGearJoint = detail::JointWrapper<TUserData, const b2GearJoint>;
template <typename TUserData, typename TConstAs>
using GearJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2GearJoint, TConstAs>>;

template <typename TUserData>
using WheelJoint = detail::JointWrapper<TUserData, b2WheelJoint>;
template <typename TUserData>
using ConstWheelJoint = detail::JointWrapper<TUserData, const b2WheelJoint>;
template <typename TUserData, typename TConstAs>
using WheelJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2WheelJoint, TConstAs>>;

template <typename TUserData>
using WeldJoint = detail::JointWrapper<TUserData, b2WeldJoint>;
template <typename TUserData>
using ConstWeldJoint = detail::JointWrapper<TUserData, const b2WeldJoint>;
template <typename TUserData, typename TConstAs>
using WeldJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2WeldJoint, TConstAs>>;

template <typename TUserData>
using FrictionJoint = detail::JointWrapper<TUserData, b2FrictionJoint>;
template <typename TUserData>
using ConstFrictionJoint = detail::JointWrapper<TUserData, const b2FrictionJoint>;
template <typename TUserData, typename TConstAs>
using FrictionJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2FrictionJoint, TConstAs>>;

template <typename TUserData>
using MotorJoint = detail::JointWrapper<TUserData, b2MotorJoint>;
template <typename TUserData>
using ConstMotorJoint = detail::JointWrapper<TUserData, const b2MotorJoint>;
template <typename TUserData, typename TConstAs>
using MotorJointConstAs = detail::JointWrapper<TUserData, dutils::copy_const_t<b2MotorJoint, TConstAs>>;

template <typename TUserData>
using JointEdge = detail::JointEdgeWrapper<TUserData, b2JointEdge>;
template <typename TUserData>
using ConstJointEdge = detail::JointEdgeWrapper<TUserData, const b2JointEdge>;
template <typename TUserData, typename TConstAs>
using JointEdgeConstAs = detail::JointEdgeWrapper<TUserData, dutils::copy_const_t<b2JointEdge, TConstAs>>;

using ShapeRef = detail::ShapeRefWrapper<b2Shape>;
using ConstShapeRef = detail::ShapeRefWrapper<const b2Shape>;
template <typename TConstAs>
using ShapeRefConstAs = detail::ShapeRefWrapper<dutils::copy_const_t<b2Shape, TConstAs>>;

using CircleShapeRef = detail::ShapeRefWrapper<b2CircleShape>;
using ConstCircleShapeRef = detail::ShapeRefWrapper<const b2CircleShape>;
template <typename TConstAs>
using CircleShapeRefConstAs = detail::ShapeRefWrapper<dutils::copy_const_t<b2CircleShape, TConstAs>>;

using EdgeShapeRef = detail::ShapeRefWrapper<b2EdgeShape>;
using ConstEdgeShapeRef = detail::ShapeRefWrapper<const b2EdgeShape>;
template <typename TConstAs>
using EdgeShapeRefConstAs = detail::ShapeRefWrapper<dutils::copy_const_t<b2EdgeShape, TConstAs>>;

using PolygonShapeRef = detail::ShapeRefWrapper<b2PolygonShape>;
using ConstPolygonShapeRef = detail::ShapeRefWrapper<const b2PolygonShape>;
template <typename TConstAs>
using PolygonShapeRefConstAs = detail::ShapeRefWrapper<dutils::copy_const_t<b2PolygonShape, TConstAs>>;

using ChainShapeRef = detail::ShapeRefWrapper<b2ChainShape>;
using ConstChainShapeRef = detail::ShapeRefWrapper<const b2ChainShape>;
template <typename TConstAs>
using ChainShapeRefConstAs = detail::ShapeRefWrapper<dutils::copy_const_t<b2ChainShape, TConstAs>>;

template <typename TUserData>
using Fixture = detail::FixtureWrapper<TUserData, b2Fixture, b2Shape>;
template <typename TUserData>
using ConstFixture = detail::FixtureWrapper<TUserData, const b2Fixture, b2Shape>;
template <typename TUserData, typename TConstAs>
using FixtureConstAs = detail::FixtureWrapper<TUserData, dutils::copy_const_t<b2Fixture, TConstAs>, b2Shape>;

template <typename TUserData>
using CircleFixture = detail::FixtureWrapper<TUserData, b2Fixture, b2CircleShape>;
template <typename TUserData>
using ConstCircleFixture = detail::FixtureWrapper<TUserData, const b2Fixture, b2CircleShape>;
template <typename TUserData, typename TConstAs>
using CircleFixtureConstAs =
    detail::FixtureWrapper<TUserData, dutils::copy_const_t<b2Fixture, TConstAs>, b2CircleShape>;

template <typename TUserData>
using EdgeFixture = detail::FixtureWrapper<TUserData, b2Fixture, b2EdgeShape>;
template <typename TUserData>
using ConstEdgeFixture = detail::FixtureWrapper<TUserData, const b2Fixture, b2EdgeShape>;
template <typename TUserData, typename TConstAs>
using EdgeFixtureConstAs = detail::FixtureWrapper<TUserData, dutils::copy_const_t<b2Fixture, TConstAs>, b2EdgeShape>;

template <typename TUserData>
using PolygonFixture = detail::FixtureWrapper<TUserData, b2Fixture, b2PolygonShape>;
template <typename TUserData>
using ConstPolygonFixture = detail::FixtureWrapper<TUserData, const b2Fixture, b2PolygonShape>;
template <typename TUserData, typename TConstAs>
using PolygonFixtureConstAs =
    detail::FixtureWrapper<TUserData, dutils::copy_const_t<b2Fixture, TConstAs>, b2PolygonShape>;

template <typename TUserData>
using ChainFixture = detail::FixtureWrapper<TUserData, b2Fixture, b2ChainShape>;
template <typename TUserData>
using ConstChainFixture = detail::FixtureWrapper<TUserData, const b2Fixture, b2ChainShape>;
template <typename TUserData, typename TConstAs>
using ChainFixtureConstAs = detail::FixtureWrapper<TUserData, dutils::copy_const_t<b2Fixture, TConstAs>, b2ChainShape>;

template <typename TUserData>
using Body = detail::BodyWrapper<TUserData, b2Body>;
template <typename TUserData>
using ConstBody = detail::BodyWrapper<TUserData, const b2Body>;
template <typename TUserData, typename TConstAs>
using BodyConstAs = detail::BodyWrapper<TUserData, dutils::copy_const_t<b2Body, TConstAs>>;

template <typename TUserData>
using WorldRef = detail::WorldRefWrapper<TUserData, b2World>;
template <typename TUserData>
using ConstWorldRef = detail::WorldRefWrapper<TUserData, const b2World>;
template <typename TUserData, typename TConstAs>
using WorldRefConstAs = detail::WorldRefWrapper<TUserData, dutils::copy_const_t<b2World, TConstAs>>;

template <typename TUserData>
using Contact = detail::ContactWrapper<TUserData, b2Contact>;
template <typename TUserData>
using ConstContact = detail::ContactWrapper<TUserData, const b2Contact>;
template <typename TUserData, typename TConstAs>
using ContactConstAs = detail::ContactWrapper<TUserData, dutils::copy_const_t<b2Contact, TConstAs>>;

} // namespace detail

struct DefaultUserData {
    using Fixture = void;
    using Body = void;
    using Joint = void;
};

template <typename TUserData = DefaultUserData>
class World;

// --- TODO

using AABB = b2AABB;
using ContactImpulse = b2ContactImpulse;
using ContactManager = b2ContactManager;
using Draw = b2Draw;
using Filter = b2Filter;
using Manifold = b2Manifold;
using MassData = b2MassData;
using Profile = b2Profile;
using RayCastInput = b2RayCastInput;
using RayCastOutput = b2RayCastOutput;
using Transform = b2Transform;
using WorldManifold = b2WorldManifold;

// --- HandleWrapper

namespace detail {

template <typename THandle>
class HandleWrapper {
public:
    HandleWrapper(std::nullptr_t = {}) {}

    HandleWrapper(THandle* handle)
        : handle_(handle)
    {}

    THandle* handle() const { return handle_; }

    explicit operator bool() const { return handle_ != nullptr; }
    operator HandleWrapper<const THandle>() const { return {handle_}; }

    friend bool operator==(HandleWrapper lhs, HandleWrapper rhs) { return lhs.handle_ == rhs.handle_; }
    friend bool operator!=(HandleWrapper lhs, HandleWrapper rhs) { return !(lhs == rhs); }

private:
    THandle* handle_ = nullptr;
};

// --- JointEdge

template <typename TUserData, typename TJointEdge>
class JointEdgeWrapper : public HandleWrapper<TJointEdge> {
public:
    using HandleWrapper<TJointEdge>::HandleWrapper;

    BodyConstAs<TUserData, TJointEdge> other() const { return this->handle()->other; }
    JointConstAs<TUserData, TJointEdge> joint() const { return this->handle()->joint; }
    JointEdgeWrapper getPrev() const { return this->handle()->prev; }
    JointEdgeWrapper getNext() const { return this->handle()->next; }
};

} // namespace detail

// --- Shape

struct CircleShape {
    float radius = 0.0f;
    vec2 position;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2CircleShape;

    void build(Data& shape) const
    {
        shape.m_radius = radius;
        shape.m_p = cast(position);
    }
};

struct OneSidedEdgeShape {
    vec2 from_vertex;
    vec2 to_vertex;
    vec2 prev_vertex;
    vec2 next_vertex;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2EdgeShape;

    void build(Data& shape) const
    {
        shape.SetOneSided(cast(prev_vertex), cast(from_vertex), cast(to_vertex), cast(next_vertex));
    }
};

struct TwoSidedEdgeShape {
    vec2 from_vertex;
    vec2 to_vertex;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2EdgeShape;

    void build(Data& shape) const { shape.SetTwoSided(cast(from_vertex), cast(to_vertex)); }
};

class PolygonShape {
public:
    explicit PolygonShape(std::initializer_list<vec2> vertices)
        : vertex_count_(0)
    {
        for (const vec2& vertex : vertices)
            vertices_[vertex_count_++] = cast(vertex);
    }

    template <typename TVertices>
    explicit PolygonShape(const TVertices& vertices)
        : vertex_count_(0)
    {
        for (const vec2& vertex : vertices)
            vertices_[vertex_count_++] = cast(vertex);
    }

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2PolygonShape;

    void build(Data& shape) const { shape.Set(vertices_.data(), vertex_count_); }

    std::array<b2Vec2, b2_maxPolygonVertices> vertices_;
    int32 vertex_count_;
};

struct BoxShape {
    vec2 size;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2PolygonShape;

    void build(Data& shape) const { shape.SetAsBox(size.x(), size.y()); }
};

struct OrientedBoxShape {
    vec2 size;
    vec2 center;
    float angle = 0.0f;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2PolygonShape;

    void build(Data& shape) const { shape.SetAsBox(size.x(), size.y(), cast(center), angle); }
};

struct LoopShape {
    std::vector<vec2> vertices;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2ChainShape;

    void build(Data& shape) const
    {
        assert(vertices.size() <= std::size_t{std::numeric_limits<int32>::max()});

        auto vertices_data = cast(vertices);
        shape.CreateLoop(data(vertices_data), static_cast<int32>(vertices.size()));
    }
};

struct ChainShape {
    std::vector<vec2> vertices;
    vec2 prev_vertex;
    vec2 next_vertex;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    using Data = b2ChainShape;

    void build(Data& shape) const
    {
        assert(vertices.size() <= std::size_t{std::numeric_limits<int32>::max()});

        auto vertices_data = cast(vertices);
        shape.CreateChain(
            data(vertices_data), static_cast<int32>(vertices.size()), cast(prev_vertex), cast(next_vertex));
    }
};

using Shape = std::variant<CircleShape,
                           OneSidedEdgeShape,
                           TwoSidedEdgeShape,
                           PolygonShape,
                           BoxShape,
                           OrientedBoxShape,
                           LoopShape,
                           ChainShape>;

using EdgeShape = std::variant<OneSidedEdgeShape, TwoSidedEdgeShape>;

// --- ShapeRef

namespace detail {

template <typename TShape>
class ShapeRefWrapper<TShape, ShapeType::Unknown> : public HandleWrapper<TShape> {
private:
    static constexpr auto can_devirtualize = !std::is_same_v<std::remove_const_t<TShape>, b2Shape>;

public:
    using HandleWrapper<TShape>::HandleWrapper;

    operator ShapeRefConstAs<TShape>() const { return this->handle(); }

    constexpr ShapeType getType() const
    {
        if constexpr (can_devirtualize)
            return shape_type_v<std::remove_const_t<TShape>>;
        else
            return cast(this->handle()->m_type);
    }

    float getRadius() const { return this->handle()->m_radius; }
    void setRadius(float radius) const { this->handle()->m_radius = radius; }

    int32 getChildCount() const
    {
        if constexpr (can_devirtualize)
            return this->handle()->TShape::GetChildCount();
        else
            return this->handle()->GetChildCount();
    }

    bool testPoint(const Transform& xf, vec2 p) const
    {
        if constexpr (can_devirtualize)
            return this->handle()->TShape::TestPoint(xf, cast(p));
        else
            return this->handle()->TestPoint(xf, cast(p));
    }

    std::optional<RayCastOutput> rayCast(const RayCastInput& input, const Transform& transform, int32 child_index) const
    {
        RayCastOutput result;
        if constexpr (can_devirtualize) {
            if (!this->handle()->TShape::RayCast(&result, input, transform, child_index))
                return std::nullopt;
        }
        else {
            if (!this->handle()->RayCast(&result, input, transform, child_index))
                return std::nullopt;
        }
        return result;
    }

    AABB computeAABB(const Transform& xf, int32 child_index) const
    {
        AABB result;
        if constexpr (can_devirtualize)
            return this->handle()->TShape::ComputeAABB(&result, xf, child_index);
        else
            return this->handle()->ComputeAABB(&result, xf, child_index);
        return result;
    }

    MassData computeMass(float density) const
    {
        MassData result;
        if constexpr (can_devirtualize)
            this->handle()->TShape::ComputeMass(&result, density);
        else
            this->handle()->ComputeMass(&result, density);
        return result;
    }
};

template <typename TShape>
class ShapeRefWrapper<TShape, ShapeType::Circle> : public ShapeRefWrapper<TShape, ShapeType::Unknown> {
public:
    using ShapeRefWrapper<TShape, ShapeType::Unknown>::ShapeRefWrapper;

    vec2 getPosition() const { return cast(this->handle()->m_p); }
    void setPosition(vec2 position) const { this->handle()->m_p = cast(position); }
};

template <typename TShape>
class ShapeRefWrapper<TShape, ShapeType::Edge> : public ShapeRefWrapper<TShape, ShapeType::Unknown> {
public:
    using ShapeRefWrapper<TShape, ShapeType::Unknown>::ShapeRefWrapper;

    vec2 getFromVertex() const { return cast(this->handle()->m_vertex1); }
    void setFromVertex(vec2 vertex) const { this->handle()->m_vertex1 = cast(vertex); }
    vec2 getToVertex() const { return cast(this->handle()->m_vertex2); }
    void setToVertex(vec2 vertex) const { this->handle()->m_vertex2 = cast(vertex); }
    vec2 getPrevVertex() const { return cast(this->handle()->m_vertex0); }
    void setPrevVertex(vec2 vertex) const { this->handle()->m_vertex0 = cast(vertex); }
    vec2 getNextVertex() const { return cast(this->handle()->m_vertex3); }
    void setNextVertex(vec2 vertex) const { this->handle()->m_vertex3 = cast(vertex); }

    bool isOneSided() const { return this->handle()->m_oneSided; }
    void setOneSided(bool one_sided) const { this->handle()->m_oneSided = one_sided; }

    EdgeShape toEdgeShape() const
    {
        if (isOneSided())
            return OneSidedEdgeShape{getFromVertex(), getToVertex(), getPrevVertex(), getNextVertex()};
        else
            return TwoSidedEdgeShape{getFromVertex(), getToVertex()};
    }

    void set(const EdgeShape& edge_shape) const
    {
        std::visit(dutils::Overloaded{[&](const OneSidedEdgeShape& one_sided_edge_shape) {
                                          setOneSided(true);
                                          setFromVertex(one_sided_edge_shape.from_vertex);
                                          setToVertex(one_sided_edge_shape.to_vertex);
                                          setPrevVertex(one_sided_edge_shape.prev_vertex);
                                          setNextVertex(one_sided_edge_shape.next_vertex);
                                      },
                                      [&](const TwoSidedEdgeShape& two_sided_edge_shape) {
                                          setOneSided(false);
                                          setFromVertex(two_sided_edge_shape.from_vertex);
                                          setToVertex(two_sided_edge_shape.to_vertex);
                                      }},
                   edge_shape);
    }
};

template <typename TShape>
class ShapeRefWrapper<TShape, ShapeType::Polygon> : public ShapeRefWrapper<TShape, ShapeType::Unknown> {
public:
    using ShapeRefWrapper<TShape, ShapeType::Unknown>::ShapeRefWrapper;

    bool validate() const { return this->handle()->Validate(); }

    vec2 centroid() const { return this->handle()->m_centroid; }
    ConstVectors vertices() const { return {this->handle()->m_vertices, this->handle()->m_count}; }
    ConstVectors normals() const { return {this->handle()->m_normals, this->handle()->m_count}; }

    PolygonShape toPolygonShape() { return {vertices()}; }
};

template <typename TShape>
class ShapeRefWrapper<TShape, ShapeType::Chain> : public ShapeRefWrapper<TShape, ShapeType::Unknown> {
public:
    using ShapeRefWrapper<TShape, ShapeType::Unknown>::ShapeRefWrapper;

    EdgeShape getChildEdge(int32 index) const
    {
        b2EdgeShape edge_shape_data;
        this->handle()->GetChildEdge(&edge_shape_data, index);
        return EdgeShapeRef{&edge_shape_data}.toEdgeShape();
    }

    Vectors vertices() const { return {this->handle()->m_vertices, this->handle()->m_count}; }

    vec2 getPrevVertex() const { return cast(this->handle()->m_prevVertex); }
    vec2 getNextVertex() const { return cast(this->handle()->m_nextVertex); }
};

// --- Fixture

template <typename TUserData>
struct FixtureDef {
    typename TUserData::Fixture* user_data = nullptr;
    float friction = 0.2f;
    float restitution = 0.0f;
    float restitution_threshold = 1.0f * b2_lengthUnitsPerMeter;
    float density = 0.0f;
    bool is_sensor = false;
    Filter filter;

private:
    template <typename, typename>
    friend class detail::BodyWrapper;

    b2FixtureDef build(const b2Shape* shape) const
    {
        b2FixtureDef result;
        result.shape = shape;
        // TODO: C++20 use std::bit_cast
        std::memcpy(&result.userData.pointer, &user_data, sizeof user_data);
        result.friction = friction;
        result.restitution = restitution;
        result.restitutionThreshold = restitution_threshold;
        result.density = density;
        result.isSensor = is_sensor;
        result.filter = filter;
        return result;
    }
};

struct ForceFixture {};

template <typename TUserData, typename TFixture, typename TShape>
class FixtureWrapper : public HandleWrapper<TFixture> {
private:
    template <typename, typename, typename>
    friend class FixtureWrapper;

    template <typename, typename>
    friend class BodyWrapper;

    FixtureWrapper(ForceFixture, TFixture* handle)
        : HandleWrapper<TFixture>(handle)
    {}

public:
    FixtureWrapper(std::nullptr_t = {})
        : HandleWrapper<TFixture>(nullptr)
    {}

    FixtureWrapper(TFixture* handle)
        : HandleWrapper<TFixture>(handle)
    {
        static_assert(std::is_same_v<std::remove_const_t<TShape>, b2Shape>,
                      "only fixtures of an unknown shape can be initialized from a b2Fixture*");
    }

    detail::ShapeRefWrapper<dutils::copy_const_t<TShape, TFixture>> getShape() const
    {
        return static_cast<dutils::copy_const_t<TShape, TFixture>*>(this->handle()->GetShape());
    }

    template <ShapeType v_shape_type>
    FixtureWrapper<TUserData, TFixture, shape_b2type_t<v_shape_type>> shaped() const
    {
        static_assert(std::is_same_v<TShape, b2Shape>, "fixture shape type already statically known");
        if constexpr (v_shape_type != ShapeType::Unknown) {
            if (v_shape_type != getShape().getType())
                return nullptr;
            return {ForceFixture{}, this->handle()};
        }
        else {
            return this->handle();
        }
    }

    constexpr operator FixtureWrapper<TUserData, TFixture, b2Shape>() const { return this->handle(); }

    void setSensor(bool sensor) const { this->handle()->SetSensor(sensor); }
    bool isSensor() const { return this->handle()->IsSensor(); }

    void setFilterData(const Filter& filter) const { this->handle()->SetFilterData(filter); }
    const Filter& getFilterData() const { return this->handle()->GetFilterData(); }
    void refilter() const { this->handle()->Refilter(); }

    BodyConstAs<TUserData, TFixture> getBody() const { return this->handle()->GetBody(); }

    FixtureConstAs<TUserData, TFixture> getNext() const { return this->handle()->GetNext(); }

    void setUserData(typename TUserData::Fixture* user_data) const
    {
        // TODO: C++20 use std::bit_cast
        std::memcpy(&this->handle()->GetUserData().pointer, &user_data, sizeof user_data);
    }

    typename TUserData::Fixture* getUserData() const
    {
        // TODO: C++20 use std::bit_cast
        typename TUserData::Fixture* result;
        std::memcpy(&result, &this->handle()->GetUserData().pointer, sizeof result);
        return result;
    }

    bool testPoint(vec2 p) const { return this->handle()->TestPoint(cast(p)); }
    std::optional<RayCastOutput> rayCast(const RayCastInput& input, int32 childIndex) const
    {
        RayCastOutput output;
        if (!this->handle()->RayCast(&output, input, childIndex))
            return std::nullopt;
        return output;
    }

    MassData getMassData() const { return this->handle()->GetMassData(); }
    void setDensity(float density) const { this->handle()->SetDensity(density); }
    float getDensity() const { return this->handle()->GetDensity(); }

    float getFriction() const { return this->handle()->GetFriction(); }
    void setFriction(float friction) const { this->handle()->SetFriction(friction); }
    float getRestitution() const { return this->handle()->GetRestitution(); }
    void setRestitution(float restitution) const { this->handle()->SetRestitution(restitution); }
    float getRestitutionThreshold() const { return this->handle()->GetRestitutionThreshold(); }
    void setRestitutionThreshold(float threshold) const { this->handle()->SetRestitutionThreshold(threshold); }

    const AABB& getAABB(int32 child_index) const { return this->handle()->GetAABB(child_index); }

    void dump(int32 body_index) const { this->handle()->Dump(body_index); }
};

} // namespace detail

// --- Applicable

struct Force {
    vec2 force;
    vec2 point;
};

struct ForceToCenter {
    vec2 force;
};

struct Torque {
    float torque;
};

struct LinearImpulse {
    vec2 impulse;
    vec2 point;
};

struct LinearImpulseToCenter {
    vec2 impulse;
};

struct AngularImpulse {
    float impulse;
};

using Applicable = std::variant<Force, ForceToCenter, Torque, LinearImpulse, LinearImpulseToCenter, AngularImpulse>;

namespace detail {

struct Applier {
    b2Body* body;
    bool wake;

    void operator()(const Force& applicable) const
    {
        body->ApplyForce(cast(applicable.force), cast(applicable.point), wake);
    }

    void operator()(const ForceToCenter& applicable) const { body->ApplyForceToCenter(cast(applicable.force), wake); }

    void operator()(const Torque& applicable) const { body->ApplyTorque(applicable.torque, wake); }

    void operator()(const LinearImpulse& applicable) const
    {
        body->ApplyLinearImpulse(cast(applicable.impulse), cast(applicable.point), wake);
    }

    void operator()(const LinearImpulseToCenter& applicable) const
    {
        body->ApplyLinearImpulseToCenter(cast(applicable.impulse), wake);
    }

    void operator()(const AngularImpulse& applicable) const { body->ApplyAngularImpulse(applicable.impulse, wake); }
};

// --- Body

template <typename TUserData>
struct BodyDef {
    BodyType type = BodyType::Static;
    vec2 position;
    float angle = 0.0f;
    vec2 linear_velocity;
    float angular_velocity = 0.0f;
    float linear_damping = 0.0f;
    float angular_damping = 0.0f;
    bool allow_sleep = true;
    bool awake = true;
    bool fixed_rotation = false;
    bool bullet = false;
    bool enabled = true;
    typename TUserData::Body* user_data = nullptr;
    float gravity_scale = 1.0f;

private:
    template <typename, typename>
    friend class detail::WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2BodyDef build() const
    {
        b2BodyDef result;
        result.type = cast(type);
        result.position = cast(position);
        result.angle = angle;
        result.linearVelocity = cast(linear_velocity);
        result.angularVelocity = angular_velocity;
        result.linearDamping = linear_damping;
        result.angularDamping = angular_damping;
        result.allowSleep = allow_sleep;
        result.awake = awake;
        result.fixedRotation = fixed_rotation;
        result.bullet = bullet;
        result.enabled = enabled;
        // TODO: C++20 use bit_cast
        std::memcpy(&result.userData.pointer, &user_data, sizeof user_data);
        result.gravityScale = gravity_scale;
        return result;
    }
};

template <typename TUserData, typename TBody>
class BodyWrapper : public HandleWrapper<TBody> {
public:
    using HandleWrapper<TBody>::HandleWrapper;

    template <typename TShape>
    auto createFixture(const FixtureDef<TUserData>& fixture, const TShape& shape) const
    {
        typename TShape::Data shape_data;
        shape.build(shape_data);
        auto def = fixture.build(&shape_data);
        return FixtureWrapper<TUserData, b2Fixture, typename TShape::Data>(ForceFixture{},
                                                                           this->handle()->CreateFixture(&def));
    }

    Fixture<TUserData> createFixture(const FixtureDef<TUserData>& fixture, const Shape& shape) const
    {
        return std::visit(
            [&](const auto& concrete_shape) -> Fixture<TUserData> {
                return this->createFixture(fixture, concrete_shape);
            },
            shape);
    }

    template <typename TShape>
    auto createFixture(const TShape& shape, float density = 1.0f) const
    {
        typename TShape::Data shape_data;
        shape.build(shape_data);
        return FixtureWrapper<TUserData, b2Fixture, typename TShape::Data>(
            ForceFixture{}, this->handle()->CreateFixture(&shape_data, density));
    }

    Fixture<TUserData> createFixture(const Shape& shape, float density = 1.0f) const
    {
        return std::visit(
            [&](const auto& concrete_shape) -> Fixture<TUserData> {
                return this->createFixture(concrete_shape, density);
            },
            shape);
    }

    template <typename TShape>
    void destroyFixture(FixtureWrapper<TUserData, b2Fixture, TShape>&& fixture) const
    {
        this->handle()->DestroyFixture(fixture.handle());
        fixture = nullptr;
    }

    void setTransform(vec2 position, float angle) const { this->handle()->SetTransform(cast(position), angle); }
    const Transform& getTransform() const { return this->handle()->GetTransform(); }
    vec2 getPosition() const { return cast(this->handle()->GetPosition()); }
    float getAngle() const { return this->handle()->GetAngle(); }
    vec2 getWorldCenter() const { return cast(this->handle()->GetWorldCenter()); }
    vec2 getLocalCenter() const { return cast(this->handle()->GetLocalCenter()); }

    void setLinearVelocity(vec2 velocity) const { this->handle()->SetLinearVelocity(cast(velocity)); }
    vec2 getLinearVelocity() const { return cast(this->handle()->GetLinearVelocity()); }
    void setAngularVelocity(float omega) const { this->handle()->SetAngularVelocity(omega); }
    float getAngularVelocity() const { return this->handle()->GetAngularVelocity(); }

    template <typename TApplicable>
    void apply(const TApplicable& applicable, bool wake = true) const
    {
        Applier{this->handle(), wake}(applicable);
    }

    void apply(const Applicable& applicable, bool wake = true) const
    {
        std::visit(Applier{this->handle(), wake}, applicable);
    }

    float getMass() const { return this->handle()->GetMass(); }
    float getInertia() const { return this->handle()->GetInertia(); }

    MassData getMassData() const
    {
        MassData result;
        this->handle()->GetMassData(&result);
        return result;
    }

    void setMassData(const MassData& mass_data) const { this->handle()->SetMassData(&mass_data); }

    void resetMassData() const { this->handle()->ResetMassData(); }

    vec2 getWorldPoint(vec2 local_point) const { return cast(this->handle()->GetWorldPoint(cast(local_point))); }
    vec2 getWorldVector(vec2 local_vector) const { return cast(this->handle()->GetWorldVector(cast(local_vector))); }
    vec2 getLocalPoint(vec2 world_point) const { return cast(this->handle()->GetLocalPoint(cast(world_point))); }
    vec2 getLocalVector(vec2 world_vector) const { return cast(this->handle()->GetLocalVector(cast(world_vector))); }

    vec2 getLinearVelocityFromWorldPoint(vec2 world_point) const
    {
        return cast(this->handle()->GetLinearVelocityFromWorldPoint(cast(world_point)));
    }

    vec2 getLinearVelocityFromLocalPoint(vec2 local_point) const
    {
        return cast(this->handle()->GetLinearVelocityFromLocalPoint(cast(local_point)));
    }

    float getLinearDamping() const { return this->handle()->GetLinearDamping(); }
    void setLinearDamping(float linear_damping) const { this->handle()->SetLinearDamping(linear_damping); }
    float getAngularDamping() const { return this->handle()->GetAngularDamping(); }
    void setAngularDamping(float angular_damping) const { this->handle()->SetAngularDamping(angular_damping); }

    float getGravityScale() const { return this->handle()->GetGravityScale(); }
    void setGravityScale(float scale) const { this->handle()->SetGravityScale(scale); }

    void setType(BodyType type) const { this->handle()->SetType(cast(type)); }
    BodyType getType() const { return cast(this->handle()->GetType()); }

    void setBullet(bool flag) const { this->handle()->SetBullet(flag); }
    bool isBullet() const { return this->handle()->IsBullet(); }

    void setSleepingAllowed(bool flag) const { this->handle()->SetSleepingAllowed(flag); }
    bool isSleepingAllowed() const { return this->handle()->IsSleepingAllowed(); }

    void setAwake(bool flag) const { this->handle()->SetAwake(flag); }
    bool isAwake() const { return this->handle()->IsAwake(); }

    void setEnabled(bool flag) const { this->handle()->SetEnabled(flag); }
    bool isEnabled() const { return this->handle()->IsEnabled(); }

    void setFixedRotation(bool flag) const { this->handle()->SetFixedRotation(flag); }
    bool isFixedRotation() const { return this->handle()->IsFixedRotation(); }

    ForwardIterable<FixtureConstAs<TUserData, TBody>> fixtures() const { return {this->handle()->GetFixtureList()}; }
    BidirectionalIterable<JointEdgeConstAs<TUserData, TBody>> joints() const
    {
        return {this->handle()->GetJointList()};
    }
    ForwardIterable<ContactConstAs<TUserData, TBody>> contacts() const { return {this->handle()->GetContactList()}; }

    BodyConstAs<TUserData, TBody> getNext() const { return this->handle()->GetNext(); }

    void setUserData(typename TUserData::Body* user_data) const
    {
        // TODO: C++20 use std::bit_cast
        std::memcpy(&this->handle()->GetUserData().pointer, &user_data, sizeof user_data);
    }

    typename TUserData::Body* getUserData() const
    {
        // TODO: C++20 use std::bit_cast
        typename TUserData::Body* result;
        std::memcpy(&result, &this->handle()->GetUserData().pointer, sizeof result);
        return result;
    }

    WorldRefConstAs<TUserData, TBody> getWorld() const { this->handle()->GetWorld(); }

    void dump() const { this->handle()->Dump(); }
};

// --- Joint

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Unknown> : public HandleWrapper<TJoint> {
private:
    static constexpr auto can_devirtualize = !std::is_same_v<std::remove_const_t<TJoint>, b2Joint>;

public:
    using HandleWrapper<TJoint>::HandleWrapper;

    operator JointConstAs<TUserData, TJoint>() const { return this->handle(); }

    constexpr JointType getType() const
    {
        if constexpr (can_devirtualize)
            return joint_type_v<std::remove_const_t<TJoint>>;
        else
            return cast(this->handle()->GetType());
    }

    BodyConstAs<TUserData, TJoint> getBodyA() const { return this->handle()->GetBodyA(); }
    BodyConstAs<TUserData, TJoint> getBodyB() const { return this->handle()->GetBodyB(); }

    vec2 getAnchorA() const
    {
        if constexpr (can_devirtualize)
            return this->handle()->TJoint::GetAnchorA();
        else
            return this->handle()->GetAnchorA();
    }

    vec2 getAnchorB() const
    {
        if constexpr (can_devirtualize)
            return this->handle()->TJoint::GetAnchorB();
        else
            return this->handle()->GetAnchorB();
    }

    vec2 getReactionForce(float inv_dt) const
    {
        if constexpr (can_devirtualize)
            return this->handle()->TJoint::GetReactionForce(inv_dt);
        else
            return this->handle()->GetReactionForce(inv_dt);
    }

    float getReactionTorque(float inv_dt) const
    {
        if constexpr (can_devirtualize)
            return this->handle()->TJoint::GetReactionTorque(inv_dt);
        else
            return this->handle()->GetReactionTorque(inv_dt);
    }

    JointConstAs<TUserData, TJoint> getNext() const { return this->handle()->GetNext(); }

    void setUserData(typename TUserData::Joint* user_data) const
    {
        // TODO: C++20 use std::bit_cast
        std::memcpy(&this->handle()->GetUserData().pointer, &user_data, sizeof user_data);
    }

    typename TUserData::Joint* getUserData() const
    {
        // TODO: C++20 use std::bit_cast
        typename TUserData::Joint* result;
        std::memcpy(&result, &this->handle()->GetUserData().pointer, sizeof result);
        return result;
    }

    bool isEnabled() const { return this->handle()->IsEnabled(); }

    bool getCollideConnected() const { return this->handle()->GetCollideConnected(); }

    void dump() const
    {
        if constexpr (can_devirtualize)
            this->handle()->TJoint::Dump();
        else
            this->handle()->Dump();
    }

    void shiftOrigin(vec2 new_origin) const
    {
        if constexpr (can_devirtualize)
            this->handle()->TJoint::ShiftOrigin(cast(new_origin));
        else
            this->handle()->ShiftOrigin(cast(new_origin));
    }

    void draw(Draw* draw) const
    {
        if constexpr (can_devirtualize)
            this->handle()->TJoint::Draw(draw);
        else
            this->handle()->Draw(draw);
    }
};

// If overridden methods were marked as final, reimplementing them here would avoid virtual calls entirely.

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Revolute>
    : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }
    float getReferenceAngle() const { return this->handle()->GetReferenceAngle(); }

    float getJointAngle() const { return this->handle()->GetJointAngle(); }
    float getJointSpeed() const { return this->handle()->GetJointSpeed(); }

    bool isLimitEnabled() const { return this->handle()->IsLimitEnabled(); }
    void enableLimit(bool flag) const { this->handle()->EnableLimit(flag); }

    float getLowerLimit() const { return this->handle()->GetLowerLimit(); }
    float getUpperLimit() const { return this->handle()->GetUpperLimit(); }
    void setLimits(float lower, float upper) const { this->handle()->SetLimits(lower, upper); }

    bool isMotorEnabled() const { return this->handle()->IsMotorEnabled(); }
    void enableMotor(bool flag) const { this->handle()->EnableMotor(flag); }
    void setMotorSpeed(float speed) const { this->handle()->SetMotorSpeed(speed); }
    float getMotorSpeed() const { return this->handle()->GetMotorSpeed(); }
    void setMaxMotorTorque(float torque) const { this->handle()->SetMaxMotorTorque(torque); }
    float getMaxMotorTorque() const { return this->handle()->GetMaxMotorTorque(); }
    float getMotorTorque(float inv_dt) const { return this->handle()->GetMotorTorque(inv_dt); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Prismatic>
    : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }
    vec2 getLocalAxisA() const { return cast(this->handle()->GetLocalAxisA()); }
    float getReferenceAngle() const { return this->handle()->GetReferenceAngle(); }

    float getJointTranslation() const { return this->handle()->GetJointTranslation(); }
    float getJointSpeed() const { return this->handle()->GetJointSpeed(); }

    bool isLimitEnabled() const { return this->handle()->IsLimitEnabled(); }
    void enableLimit(bool flag) const { this->handle()->EnableLimit(flag); }

    float getLowerLimit() const { return this->handle()->GetLowerLimit(); }
    float getUpperLimit() const { return this->handle()->GetUpperLimit(); }
    void setLimits(float lower, float upper) const { this->handle()->SetLimits(lower, upper); }

    bool isMotorEnabled() const { return this->handle()->IsMotorEnabled(); }
    void enableMotor(bool flag) const { this->handle()->EnableMotor(flag); }
    void setMotorSpeed(float speed) const { this->handle()->SetMotorSpeed(speed); }
    float getMotorSpeed() const { return this->handle()->GetMotorSpeed(); }
    void setMaxMotorForce(float force) const { this->handle()->SetMaxMotorForce(force); }
    float getMaxMotorForce() const { return this->handle()->GetMaxMotorForce(); }
    float getMotorForce(float inv_dt) const { return this->handle()->GetMotorForce(inv_dt); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Distance>
    : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }

    float getLength() const { return this->handle()->GetLength(); }
    float setLength(float length) const { return this->handle()->SetLength(length); }

    float getMinLength() const { return this->handle()->GetMinLength(); }
    float setMinLength(float min_length) const { return this->handle()->SetMinLength(min_length); }
    float getMaxLength() const { return this->handle()->GetMaxLength(); }
    float setMaxLength(float max_length) const { return this->handle()->SetMaxLength(max_length); }

    float getCurrentLength() const { return this->handle()->GetCurrentLength(); }

    void setStiffness(float stiffness) const { this->handle()->SetStiffness(stiffness); }
    float getStiffness() const { return this->handle()->GetStiffness(); }

    void setDamping(float damping) const { this->handle()->SetDamping(damping); }
    float getDamping() const { return this->handle()->GetDamping(); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Pulley> : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    vec2 GetGroundAnchorA() const { return cast(this->handle()->GetGroundAnchorA()); }
    vec2 GetGroundAnchorB() const { return cast(this->handle()->GetGroundAnchorB()); }

    float getLengthA() const { return this->handle()->GetLengthA(); }
    float getLengthB() const { return this->handle()->GetLengthB(); }

    float getRatio() const { return this->handle()->GetRatio(); }

    float getCurrentLengthA() const { return this->handle()->GetCurrentLengthA(); }
    float getCurrentLengthB() const { return this->handle()->GetCurrentLengthB(); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Mouse> : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    void setTarget(vec2 target) const { this->handle()->SetTarget(cast(target)); }
    vec2 getTarget() const { return cast(this->handle()->GetTarget()); }

    void setMaxForce(float force) const { this->handle()->SetMaxForce(force); }
    float getMaxForce() const { return this->handle()->GetMaxForce(); }

    void setStiffness(float stiffness) const { this->handle()->SetStiffness(stiffness); }
    float getStiffness() const { return this->handle()->GetStiffness(); }

    void setDamping(float damping) const { this->handle()->SetDamping(damping); }
    float getDamping() const { return this->handle()->GetDamping(); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Gear> : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    JointConstAs<TUserData, TJoint> GetJoint1() const { return this->handle()->GetJoint1(); }
    JointConstAs<TUserData, TJoint> GetJoint2() const { return this->handle()->GetJoint2(); }

    void setRatio(float ratio) const { this->handle()->SetRatio(ratio); }
    float getRatio() const { return this->handle()->GetRatio(); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Wheel> : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }
    vec2 getLocalAxisA() const { return cast(this->handle()->GetLocalAxisA()); }

    float getJointTranslation() const { return this->handle()->GetJointTranslation(); }
    float getJointLinearSpeed() const { return this->handle()->GetJointLinearSpeed(); }
    float getJointAngle() const { return this->handle()->GetJointAngle(); }
    float getJointAngularSpeed() const { return this->handle()->GetJointAngularSpeed(); }

    bool isLimitEnabled() const { return this->handle()->IsLimitEnabled(); }
    void enableLimit(bool flag) const { this->handle()->EnableLimit(flag); }
    float getLowerLimit() const { return this->handle()->GetLowerLimit(); }
    float getUpperLimit() const { return this->handle()->GetUpperLimit(); }
    void setLimits(float lower, float upper) const { this->handle()->SetLimits(lower, upper); }

    bool isMotorEnabled() const { return this->handle()->IsMotorEnabled(); }
    void enableMotor(bool flag) const { this->handle()->EnableMotor(flag); }
    void setMotorSpeed(float speed) const { this->handle()->SetMotorSpeed(speed); }
    float getMotorSpeed() const { return this->handle()->GetMotorSpeed(); }
    void setMaxMotorTorque(float torque) const { this->handle()->SetMaxMotorTorque(torque); }
    float getMaxMotorTorque() const { return this->handle()->GetMaxMotorTorque(); }
    float getMotorTorque(float inv_dt) const { return this->handle()->GetMotorTorque(inv_dt); }

    void setStiffness(float stiffness) const { this->handle()->SetStiffness(stiffness); }
    float getStiffness() const { return this->handle()->GetStiffness(); }

    void setDamping(float damping) const { this->handle()->SetDamping(damping); }
    float getDamping() const { return this->handle()->GetDamping(); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Weld> : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }
    float getReferenceAngle() const { return this->handle()->GetReferenceAngle(); }

    void setStiffness(float hz) const { this->handle()->SetStiffness(hz); }
    float getStiffness() const { return this->handle()->GetStiffness(); }

    void setDamping(float damping) const { this->handle()->SetDamping(damping); }
    float getDamping() const { return this->handle()->GetDamping(); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Friction>
    : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }

    void setMaxForce(float force) const { this->handle()->SetMaxForce(force); }
    float getMaxForce() const { return this->handle()->GetMaxForce(); }

    void setMaxTorque(float torque) const { this->handle()->SetMaxTorque(torque); }
    float getMaxTorque() const { return this->handle()->GetMaxTorque(); }
};

template <typename TUserData, typename TJoint>
class JointWrapper<TUserData, TJoint, JointType::Motor> : public JointWrapper<TUserData, TJoint, JointType::Unknown> {
public:
    using JointWrapper<TUserData, TJoint, JointType::Unknown>::JointWrapper;

    void setLinearOffset(vec2 linearOffset) const { this->handle()->SetLinearOffset(cast(linearOffset)); }
    vec2 getLinearOffset() const { return cast(this->handle()->GetLinearOffset()); }
    void setAngularOffset(float angularOffset) const { this->handle()->SetAngularOffset(angularOffset); }
    float getAngularOffset() const { return this->handle()->GetAngularOffset(); }

    void setMaxForce(float force) const { this->handle()->SetMaxForce(force); }
    float getMaxForce() const { return this->handle()->GetMaxForce(); }
    void setMaxTorque(float torque) const { this->handle()->SetMaxTorque(torque); }
    float getMaxTorque() const { return this->handle()->GetMaxTorque(); }

    void setCorrectionFactor(float factor) const { this->handle()->SetCorrectionFactor(factor); }
    float getCorrectionFactor() const { return this->handle()->GetCorrectionFactor(); }
};

template <typename TUserData>
struct JointDefBase {
    typename TUserData::Joint* user_data;
    Body<TUserData> body_a;
    Body<TUserData> body_b;

protected:
    void build(b2JointDef& def) const
    {
        // TODO: C++20 use std::bit_cast
        std::memcpy(&def.userData.pointer, &user_data, sizeof user_data);
        def.bodyA = body_a.handle();
        def.bodyB = body_b.handle();
    }
};

template <typename TUserData>
struct JointDefNoCollideDefault : JointDefBase<TUserData> {
    bool collide_connected = false;

protected:
    void build(b2JointDef& def) const
    {
        JointDefBase<TUserData>::build(def);
        def.collideConnected = collide_connected;
    }
};

template <typename TUserData>
struct RevoluteJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 local_anchor_a;
    vec2 local_anchor_b;
    float reference_angle = 0.0f;
    bool enable_limit = false;
    float lower_angle = 0.0f;
    float upper_angle = 0.0f;
    bool enable_motor = false;
    float motor_speed = 0.0f;
    float max_motor_torque = 0.0f;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2RevoluteJointDef build() const
    {
        b2RevoluteJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.referenceAngle = reference_angle;
        def.enableLimit = enable_limit;
        def.lowerAngle = lower_angle;
        def.upperAngle = upper_angle;
        def.enableMotor = enable_motor;
        def.motorSpeed = motor_speed;
        def.maxMotorTorque = max_motor_torque;
        return def;
    }
};

template <typename TUserData>
struct PrismaticJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 local_anchor_a;
    vec2 local_anchor_b;
    vec2 local_axis_a = {1.0f, 0.0f};
    float reference_angle = 0.0f;
    bool enable_limit = false;
    float lower_translation = 0.0f;
    float upper_translation = 0.0f;
    bool enable_motor = false;
    float max_motor_force = 0.0f;
    float motor_speed = 0.0f;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2PrismaticJointDef build() const
    {
        b2PrismaticJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.localAxisA = cast(local_axis_a);
        def.referenceAngle = reference_angle;
        def.enableLimit = enable_limit;
        def.lowerTranslation = lower_translation;
        def.upperTranslation = upper_translation;
        def.enableMotor = enable_motor;
        def.maxMotorForce = max_motor_force;
        def.motorSpeed = motor_speed;
        return def;
    }
};

template <typename TUserData>
struct DistanceJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 local_anchor_a;
    vec2 local_anchor_b;
    float length = 1.0f;
    float min_length = 0.0f;
    float max_length = std::numeric_limits<float>::max();
    float stiffness = 0.0f;
    float damping = 0.0f;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2DistanceJointDef build() const
    {
        b2DistanceJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.length = length;
        def.minLength = min_length;
        def.maxLength = max_length;
        def.stiffness = stiffness;
        def.damping = damping;
        return def;
    }
};

template <typename TUserData>
struct PulleyJointDef : JointDefBase<TUserData> {
    bool collide_connected = true;
    vec2 ground_anchor_a = {-1.0f, 1.0f};
    vec2 ground_anchor_b = {1.0f, 1.0f};
    vec2 local_anchor_a = {-1.0f, 0.0f};
    vec2 local_anchor_b = {1.0f, 0.0f};
    float length_a = 0.0f;
    float length_b = 0.0f;
    float ratio = 1.0f;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2PulleyJointDef build() const
    {
        b2PulleyJointDef def;
        JointDefBase<TUserData>::build(def);
        def.collideConnected = collide_connected;
        def.groundAnchorA = cast(ground_anchor_a);
        def.groundAnchorB = cast(ground_anchor_b);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.lengthA = length_a;
        def.lengthB = length_b;
        def.ratio = ratio;
        return def;
    }
};

template <typename TUserData>
struct MouseJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 target;
    float max_force = 0.0f;
    float stiffness = 0.0f;
    float damping = 0.0f;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2MouseJointDef build() const
    {
        b2MouseJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.target = cast(target);
        def.maxForce = max_force;
        def.stiffness = stiffness;
        def.damping = damping;
        return def;
    }
};

template <typename TUserData>
struct GearJointDef : JointDefNoCollideDefault<TUserData> {
    Joint<TUserData> joint1;
    Joint<TUserData> joint2;
    float ratio;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2GearJointDef build() const
    {
        b2GearJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.joint1 = joint1;
        def.joint2 = joint2;
        def.ratio = ratio;
        return def;
    }
};

template <typename TUserData>
struct WheelJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 local_anchor_a;
    vec2 local_anchor_b;
    vec2 local_axis_a;
    bool enable_limit;
    float lower_translation;
    float upper_translation;
    bool enable_motor;
    float max_motor_torque;
    float motor_speed;
    float stiffness;
    float damping;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2WheelJointDef build() const
    {
        b2WheelJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.localAxisA = cast(local_axis_a);
        def.enableLimit = enable_limit;
        def.lowerTranslation = lower_translation;
        def.upperTranslation = upper_translation;
        def.enableMotor = enable_motor;
        def.maxMotorTorque = max_motor_torque;
        def.motorSpeed = motor_speed;
        def.stiffness = stiffness;
        def.damping = damping;
        return def;
    }
};

template <typename TUserData>
struct WeldJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 local_anchor_a;
    vec2 local_anchor_b;
    float reference_angle;
    float stiffness;
    float damping;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2WeldJointDef build() const
    {
        b2WeldJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.referenceAngle = reference_angle;
        def.stiffness = stiffness;
        def.damping = damping;
        return def;
    }
};

template <typename TUserData>
struct FrictionJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 local_anchor_a;
    vec2 local_anchor_b;
    float max_force;
    float max_torque;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2FrictionJointDef build() const
    {
        b2FrictionJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.maxForce = max_force;
        def.maxTorque = max_torque;
        return def;
    }
};

template <typename TUserData>
struct MotorJointDef : JointDefNoCollideDefault<TUserData> {
    vec2 linear_offset;
    float angular_offset;
    float max_force;
    float max_torque;
    float correction_factor;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2MotorJointDef build() const
    {
        b2MotorJointDef def;
        JointDefNoCollideDefault<TUserData>::build(def);
        def.linearOffset = cast(linear_offset);
        def.angularOffset = angular_offset;
        def.maxForce = max_force;
        def.maxTorque = max_torque;
        def.correctionFactor = correction_factor;
        return def;
    }
};

template <typename TUserData>
using JointDef = std::variant<RevoluteJointDef<TUserData>,
                              PrismaticJointDef<TUserData>,
                              DistanceJointDef<TUserData>,
                              PulleyJointDef<TUserData>,
                              MouseJointDef<TUserData>,
                              GearJointDef<TUserData>,
                              WheelJointDef<TUserData>,
                              WeldJointDef<TUserData>,
                              FrictionJointDef<TUserData>,
                              MotorJointDef<TUserData>>;

// --- Contact

template <typename TUserData, typename TContact>
class ContactWrapper : public HandleWrapper<TContact> {
public:
    using HandleWrapper<TContact>::HandleWrapper;

    dutils::copy_const_t<Manifold, TContact> getManifold() const { return this->handle()->GetManifold(); }

    WorldManifold getWorldManifold() const
    {
        WorldManifold result;
        this->handle()->GetWorldManifold(&result);
        return result;
    }

    bool isTouching() const { return this->handle()->IsTouching(); }

    void setEnabled(bool flag) const { this->handle()->SetEnabled(flag); }
    bool isEnabled() const { return this->handle()->IsEnabled(); }

    ContactConstAs<TUserData, TContact> getNext() const { return this->handle()->GetNext(); }

    FixtureConstAs<TUserData, TContact> getFixtureA() const { return this->handle()->GetFixtureA(); }
    int32 getChildIndexA() const { return this->handle()->GetChildIndexA(); }
    FixtureConstAs<TUserData, TContact> getFixtureB() const { return this->handle()->GetFixtureB(); }
    int32 getChildIndexB() const { return this->handle()->GetChildIndexB(); }

    void setFriction(float friction) const { this->handle()->SetFriction(friction); }
    float getFriction() const { return this->handle()->GetFriction(); }
    void resetFriction() const { this->handle()->ResetFriction(); }

    void setRestitution(float restitution) const { this->handle()->SetRestitution(restitution); }
    float getRestitution() const { return this->handle()->GetRestitution(); }
    void resetRestitution() const { this->handle()->ResetRestitution(); }

    void setRestitutionThreshold(float threshold) const { this->handle()->SetRestitutionThreshold(threshold); }
    float getRestitutionThreshold() const { return this->handle()->GetRestitutionThreshold(); }
    void resetRestitutionThreshold() const { this->handle()->ResetRestitutionThreshold(); }

    void setTangentSpeed(float speed) const { this->handle()->SetTangentSpeed(speed); }
    float getTangentSpeed() const { return this->handle()->GetTangentSpeed(); }

    void evaluate(Manifold& manifold, const Transform& xf_a, const Transform& xf_b) const
    {
        return this->handle()->Evaluate(&manifold, xf_a, xf_b);
    }
};

} // namespace detail

// --- WorldRef

namespace detail {

template <typename TUserData>
using QueryCallback = std::function<bool(Fixture<TUserData>)>;

template <typename TUserData>
struct RayCastData {
    Fixture<TUserData> fixture;
    vec2 point;
    vec2 normal;
    float fraction;

    bool operator==(const RayCastData& other) const
    {
        return std::tie(fixture, point, normal, fraction) ==
               std::tie(other.fixture, other.point, other.normal, other.fraction);
    }

    bool operator!=(const RayCastData& other) const { return !(*this == other); }

    static constexpr float filter = -1.0f;
    static constexpr float terminate = 0.0f;
    static constexpr float clip(float value) { return value; }
    static constexpr float next = 1.0f;
};

template <typename TUserData>
using RayCastCallback = std::function<float(const RayCastData<TUserData>&)>;

template <typename TUserData>
class QueryCallbackWrapper : public b2QueryCallback {
public:
    QueryCallbackWrapper(QueryCallback<TUserData> callback)
        : callback_(std::move(callback))
    {}

    bool ReportFixture(b2Fixture* fixture) override { return callback_(fixture); }

private:
    QueryCallback<TUserData> callback_;
};

template <typename TUserData>
class RayCastCallbackWrapper : public b2RayCastCallback {
public:
    RayCastCallbackWrapper(RayCastCallback<TUserData> callback)
        : callback_(std::move(callback))
    {}

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
    {
        return callback_({fixture, cast(point), cast(normal), fraction});
    }

private:
    RayCastCallback<TUserData> callback_;
};

template <typename TUserData, typename TWorld>
class WorldRefWrapper : public HandleWrapper<TWorld> {
public:
    using HandleWrapper<TWorld>::HandleWrapper;

    void setDebugDraw(Draw* debug_draw) { this->handle()->SetDebugDraw(debug_draw); }
    void debugDraw() const { this->handle()->DebugDraw(); }

    Body<TUserData> createBody(const BodyDef<TUserData>& body) const
    {
        auto def = body.build();
        return this->handle()->CreateBody(&def);
    }

    Body<TUserData> createBody(BodyType body_type = BodyType::Static) const
    {
        auto def = BodyDef<TUserData>{};
        def.type = body_type;
        return createBody(def);
    }

    void destroyBody(Body<TUserData>&& body) const
    {
        this->handle()->DestroyBody(body.handle());
        body = nullptr;
    }

    template <typename TJointDef>
    auto createJoint(const TJointDef& joint) const
    {
        auto def = joint.build();
        using ResultType = detail::joint_def_result_type_t<decltype(def)>;
        return detail::JointWrapper<TUserData, ResultType>(static_cast<ResultType*>(this->handle()->CreateJoint(&def)));
    }

    Joint<TUserData> createJoint(const JointDef<TUserData>& joint) const
    {
        return std::visit(
            [&](const auto& concrete_joint) -> Joint<TUserData> { return this->createJoint(concrete_joint); }, joint);
    }

    template <typename TJoint>
    void destroyJoint(JointWrapper<TUserData, TJoint>&& joint) const
    {
        this->handle()->DestroyJoint(joint.handle());
        joint = nullptr;
    }

    void step(float time_step, int32 velocity_iterations, int32 position_iterations) const
    {
        this->handle()->Step(time_step, velocity_iterations, position_iterations);
    }

    void clearForces() const { this->handle()->ClearForces(); }

    void queryAABB(QueryCallback<TUserData> callback, AABB aabb) const
    {
        QueryCallbackWrapper<TUserData> wrapper{std::move(callback)};
        this->handle()->QueryAABB(&wrapper, aabb);
    }

    void rayCast(RayCastCallback<TUserData> callback, vec2 point1, vec2 point2) const
    {
        RayCastCallbackWrapper<TUserData> wrapper{std::move(callback)};
        this->handle()->RayCast(&wrapper, cast(point1), cast(point2));
    }

    ForwardIterable<BodyConstAs<TUserData, TWorld>> bodies() const { return {this->handle()->GetBodyList()}; }
    ForwardIterable<JointConstAs<TUserData, TWorld>> joints() const { return {this->handle()->GetJointList()}; }
    ForwardIterable<ContactConstAs<TUserData, TWorld>> contacts() const { return {this->handle()->GetContactList()}; }

    void setAllowSleeping(bool flag) const { this->handle()->SetAllowSleeping(flag); }
    bool getAllowSleeping() const { return this->handle()->GetAllowSleeping(); }

    void setWarmStarting(bool flag) const { this->handle()->SetWarmStarting(flag); }
    bool getWarmStarting() const { return this->handle()->GetWarmStarting(); }

    void setContinuousPhysics(bool flag) const { this->handle()->SetContinuousPhysics(flag); }
    bool getContinuousPhysics() const { return this->handle()->GetContinuousPhysics(); }

    void setSubStepping(bool flag) const { this->handle()->SetSubStepping(flag); }
    bool getSubStepping() const { return this->handle()->GetSubStepping(); }

    void setAutoClearForces(bool flag) const { this->handle()->SetAutoClearForces(flag); }
    bool getAutoClearForces() const { return this->handle()->GetAutoClearForces(); }

    int32 getProxyCount() const { return this->handle()->GetProxyCount(); }
    int32 getBodyCount() const { return this->handle()->GetBodyCount(); }
    int32 getJointCount() const { return this->handle()->GetJointCount(); }
    int32 getContactCount() const { return this->handle()->GetContactCount(); }

    int32 getTreeHeight() const { return this->handle()->GetTreeHeight(); }
    int32 getTreeBalance() const { return this->handle()->GetTreeBalance(); }
    float getTreeQuality() const { return this->handle()->GetTreeQuality(); }

    void setGravity(vec2 gravity) const { this->handle()->SetGravity(cast(gravity)); }
    vec2 getGravity() const { return cast(this->handle()->GetGravity()); }

    bool isLocked() const { return this->handle()->IsLocked(); }

    void shiftOrigin(vec2 new_origin) const { this->handle()->ShiftOrigin(cast(new_origin)); }

    const ContactManager& getContactManager() const { return this->handle()->GetContactManager(); }

    const Profile& getProfile() const { return this->handle()->GetProfile(); }

    void dump() const { this->handle()->Dump(); }
};

} // namespace detail

// --- World

template <typename TUserData>
class World {
public:
    using UserData = TUserData;

    using FixtureDef = detail::FixtureDef<UserData>;

    using BodyDef = detail::BodyDef<UserData>;

    using RevoluteJointDef = detail::RevoluteJointDef<UserData>;
    using PrismaticJointDef = detail::PrismaticJointDef<UserData>;
    using DistanceJointDef = detail::DistanceJointDef<UserData>;
    using PulleyJointDef = detail::PulleyJointDef<UserData>;
    using MouseJointDef = detail::MouseJointDef<UserData>;
    using GearJointDef = detail::GearJointDef<UserData>;
    using WheelJointDef = detail::WheelJointDef<UserData>;
    using WeldJointDef = detail::WeldJointDef<UserData>;
    using FrictionJointDef = detail::FrictionJointDef<UserData>;
    using MotorJointDef = detail::MotorJointDef<UserData>;

    using JointDef = detail::JointDef<UserData>;

    using Fixture = detail::Fixture<UserData>;
    using ConstFixture = detail::ConstFixture<UserData>;
    template <typename TConstAs>
    using FixtureConstAs = detail::FixtureConstAs<UserData, TConstAs>;

    using CircleFixture = detail::CircleFixture<UserData>;
    using ConstCircleFixture = detail::ConstCircleFixture<UserData>;
    template <typename TConstAs>
    using CircleFixtureConstAs = detail::CircleFixtureConstAs<UserData, TConstAs>;

    using EdgeFixture = detail::EdgeFixture<UserData>;
    using ConstEdgeFixture = detail::ConstEdgeFixture<UserData>;
    template <typename TConstAs>
    using EdgeFixtureConstAs = detail::EdgeFixtureConstAs<UserData, TConstAs>;

    using PolygonFixture = detail::PolygonFixture<UserData>;
    using ConstPolygonFixture = detail::ConstPolygonFixture<UserData>;
    template <typename TConstAs>
    using PolygonFixtureConstAs = detail::PolygonFixtureConstAs<UserData, TConstAs>;

    using ChainFixture = detail::ChainFixture<UserData>;
    using ConstChainFixture = detail::ConstChainFixture<UserData>;
    template <typename TConstAs>
    using ChainFixtureConstAs = detail::ChainFixtureConstAs<UserData, TConstAs>;

    using Body = detail::Body<UserData>;
    using ConstBody = detail::ConstBody<UserData>;
    template <typename TConstAs>
    using BodyConstAs = detail::BodyConstAs<UserData, TConstAs>;

    using Joint = detail::Joint<UserData>;
    using ConstJoint = detail::ConstJoint<UserData>;
    template <typename TConstAs>
    using JointConstAs = detail::JointConstAs<UserData, TConstAs>;

    using RevoluteJoint = detail::RevoluteJoint<UserData>;
    using ConstRevoluteJoint = detail::ConstRevoluteJoint<UserData>;
    template <typename TConstAs>
    using RevoluteJointConstAs = detail::RevoluteJointConstAs<UserData, TConstAs>;

    using PrismaticJoint = detail::PrismaticJoint<UserData>;
    using ConstPrismaticJoint = detail::ConstPrismaticJoint<UserData>;
    template <typename TConstAs>
    using PrismaticJointConstAs = detail::PrismaticJointConstAs<UserData, TConstAs>;

    using DistanceJoint = detail::DistanceJoint<UserData>;
    using ConstDistanceJoint = detail::ConstDistanceJoint<UserData>;
    template <typename TConstAs>
    using DistanceJointConstAs = detail::DistanceJointConstAs<UserData, TConstAs>;

    using PulleyJoint = detail::PulleyJoint<UserData>;
    using ConstPulleyJoint = detail::ConstPulleyJoint<UserData>;
    template <typename TConstAs>
    using PulleyJointConstAs = detail::PulleyJointConstAs<UserData, TConstAs>;

    using MouseJoint = detail::MouseJoint<UserData>;
    using ConstMouseJoint = detail::ConstMouseJoint<UserData>;
    template <typename TConstAs>
    using MouseJointConstAs = detail::MouseJointConstAs<UserData, TConstAs>;

    using GearJoint = detail::GearJoint<UserData>;
    using ConstGearJoint = detail::ConstGearJoint<UserData>;
    template <typename TConstAs>
    using GearJointConstAs = detail::GearJointConstAs<UserData, TConstAs>;

    using WheelJoint = detail::WheelJoint<UserData>;
    using ConstWheelJoint = detail::ConstWheelJoint<UserData>;
    template <typename TConstAs>
    using WheelJointConstAs = detail::WheelJointConstAs<UserData, TConstAs>;

    using WeldJoint = detail::WeldJoint<UserData>;
    using ConstWeldJoint = detail::ConstWeldJoint<UserData>;
    template <typename TConstAs>
    using WeldJointConstAs = detail::WeldJointConstAs<UserData, TConstAs>;

    using FrictionJoint = detail::FrictionJoint<UserData>;
    using ConstFrictionJoint = detail::ConstFrictionJoint<UserData>;
    template <typename TConstAs>
    using FrictionJointConstAs = detail::FrictionJointConstAs<UserData, TConstAs>;

    using MotorJoint = detail::MotorJoint<UserData>;
    using ConstMotorJoint = detail::ConstMotorJoint<UserData>;
    template <typename TConstAs>
    using MotorJointConstAs = detail::MotorJointConstAs<UserData, TConstAs>;

    using JointEdge = detail::JointEdge<UserData>;
    using ConstJointEdge = detail::ConstJointEdge<UserData>;
    template <typename TConstAs>
    using JointEdgeConstAs = detail::JointEdgeConstAs<UserData, TConstAs>;

    using Contact = detail::Contact<UserData>;
    using ConstContact = detail::ConstContact<UserData>;
    template <typename TConstAs>
    using ContactConstAs = detail::ContactConstAs<UserData, TConstAs>;

    using WorldRef = detail::WorldRef<UserData>;
    using ConstWorldRef = detail::ConstWorldRef<UserData>;
    template <typename TConstAs>
    using WorldRefConstAs = detail::WorldRefConstAs<UserData, TConstAs>;

    using QueryCallback = detail::QueryCallback<UserData>;

    using RayCastData = detail::RayCastData<UserData>;
    using RayCastCallback = detail::RayCastCallback<UserData>;

    using ContactFilter = std::function<bool(Fixture, Fixture)>;

    explicit World(vec2 gravity = {})
        : world_(cast(gravity))
    {
        world_.SetDestructionListener(&destruction_listener_);
        world_.SetContactListener(&contact_listener_);
    }

    void setContactFilter(ContactFilter should_collide)
    {
        world_.SetContactFilter(should_collide ? &contact_filter_wrapper_ : nullptr);
        contact_filter_wrapper_ = {std::move(should_collide)};
    }

    void setDebugDraw(Draw* debug_draw) { world_.SetDebugDraw(debug_draw); }
    void debugDraw() { world_.DebugDraw(); }

    Body createBody(const BodyDef& body) { return WorldRef{&world_}.createBody(body); }
    Body createBody(BodyType body_type = BodyType::Static) { return WorldRef{&world_}.createBody(body_type); }
    void destroyBody(Body&& body) { WorldRef{&world_}.destroyBody(std::move(body)); }

    template <typename TJointDef>
    auto createJoint(const TJointDef& joint)
    {
        return WorldRef{&world_}.createJoint(joint);
    }

    Joint createJoint(const JointDef& joint) { return WorldRef{&world_}.createJoint(joint); }

    template <typename TJoint>
    void destroyJoint(detail::JointWrapper<UserData, TJoint>&& joint)
    {
        WorldRef{&world_}.destroyJoint(std::move(joint));
    }

    void step(float time_step, int32 velocity_iterations, int32 position_iterations)
    {
        world_.Step(time_step, velocity_iterations, position_iterations);
    }

    void clearForces() { world_.ClearForces(); }

    void queryAABB(QueryCallback callback, AABB aabb) const
    {
        ConstWorldRef{&world_}.queryAABB(std::move(callback), aabb);
    }

    void rayCast(RayCastCallback callback, vec2 point1, vec2 point2) const
    {
        ConstWorldRef{&world_}.rayCast(std::move(callback), point1, point2);
    }

    ForwardIterable<Body> bodies() { return {world_.GetBodyList()}; }
    ForwardIterable<ConstBody> bodies() const { return {world_.GetBodyList()}; }

    ForwardIterable<Joint> joints() { return {world_.GetJointList()}; }
    ForwardIterable<ConstJoint> joints() const { return {world_.GetJointList()}; }

    ForwardIterable<Contact> contacts() { return {world_.GetContactList()}; }
    ForwardIterable<ConstContact> contacts() const { return {world_.GetContactList()}; }

    void setAllowSleeping(bool flag) { world_.SetAllowSleeping(flag); }
    bool getAllowSleeping() const { return world_.GetAllowSleeping(); }

    void setWarmStarting(bool flag) { world_.SetWarmStarting(flag); }
    bool getWarmStarting() const { return world_.GetWarmStarting(); }

    void setContinuousPhysics(bool flag) { world_.SetContinuousPhysics(flag); }
    bool getContinuousPhysics() const { return world_.GetContinuousPhysics(); }

    void setSubStepping(bool flag) { world_.SetSubStepping(flag); }
    bool getSubStepping() const { return world_.GetSubStepping(); }

    void setAutoClearForces(bool flag) { world_.SetAutoClearForces(flag); }
    bool getAutoClearForces() const { return world_.GetAutoClearForces(); }

    int32 getProxyCount() const { return world_.GetProxyCount(); }
    int32 getBodyCount() const { return world_.GetBodyCount(); }
    int32 getJointCount() const { return world_.GetJointCount(); }
    int32 getContactCount() const { return world_.GetContactCount(); }

    int32 getTreeHeight() const { return world_.GetTreeHeight(); }
    int32 getTreeBalance() const { return world_.GetTreeBalance(); }
    float getTreeQuality() const { return world_.GetTreeQuality(); }

    void setGravity(vec2 gravity) { world_.SetGravity(cast(gravity)); }
    vec2 getGravity() const { return cast(world_.GetGravity()); }

    bool isLocked() const { return world_.IsLocked(); }

    void shiftOrigin(vec2 new_origin) { world_.ShiftOrigin(cast(new_origin)); }

    const ContactManager& getContactManager() const { return world_.GetContactManager(); }

    const Profile& getProfile() const { return world_.GetProfile(); }

    void dump() { world_.Dump(); }

    dutils::Event<Fixture> on_destroy_fixture;
    dutils::Event<Joint> on_destroy_joint;

    dutils::Event<Contact> on_begin_contact;
    dutils::Event<Contact> on_end_contact;
    dutils::Event<Contact, const Manifold*> on_pre_solve;
    dutils::Event<Contact, const ContactImpulse*> on_post_solve;

private:
    class DestructionListener : public b2DestructionListener {
    public:
        explicit DestructionListener(World* world)
            : world_(world)
        {}

        void SayGoodbye(b2Joint* joint) override { world_->on_destroy_joint(joint); }
        void SayGoodbye(b2Fixture* fixture) override { world_->on_destroy_fixture(fixture); }

    private:
        World* world_;
    };

    class ContactListener : public b2ContactListener {
    public:
        explicit ContactListener(World* world)
            : world_(world)
        {}

        void BeginContact(b2Contact* contact) override { world_->on_begin_contact(contact); }

        void EndContact(b2Contact* contact) override { world_->on_end_contact(contact); }

        void PreSolve(b2Contact* contact, const b2Manifold* old_manifold) override
        {
            world_->on_pre_solve(contact, old_manifold);
        }

        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override
        {
            world_->on_post_solve(contact, impulse);
        }

    private:
        World* world_;
    };

    class ContactFilterWrapper : public b2ContactFilter {
    public:
        ContactFilterWrapper(ContactFilter callback = {})
            : callback_(std::move(callback))
        {}

        bool ShouldCollide(b2Fixture* fixture_a, b2Fixture* fixture_b) override
        {
            return callback_(fixture_a, fixture_b);
        }

    private:
        ContactFilter callback_;
    };

    b2World world_;
    DestructionListener destruction_listener_{this};
    ContactListener contact_listener_{this};
    ContactFilterWrapper contact_filter_wrapper_;
};

} // namespace dang::box2d

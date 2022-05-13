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

template <JointType v_type>
struct joint_b2type;

template <JointType v_type>
using joint_b2type_t = typename joint_b2type<v_type>::type;

template <>
struct joint_b2type<JointType::Unknown> {
    using type = b2Joint;
};

template <>
struct joint_b2type<JointType::Revolute> {
    using type = b2RevoluteJoint;
};

template <>
struct joint_b2type<JointType::Prismatic> {
    using type = b2PrismaticJoint;
};

template <>
struct joint_b2type<JointType::Distance> {
    using type = b2DistanceJoint;
};

template <>
struct joint_b2type<JointType::Pulley> {
    using type = b2PulleyJoint;
};

template <>
struct joint_b2type<JointType::Mouse> {
    using type = b2MouseJoint;
};

template <>
struct joint_b2type<JointType::Gear> {
    using type = b2GearJoint;
};

template <>
struct joint_b2type<JointType::Wheel> {
    using type = b2WheelJoint;
};

template <>
struct joint_b2type<JointType::Weld> {
    using type = b2WeldJoint;
};

template <>
struct joint_b2type<JointType::Friction> {
    using type = b2FrictionJoint;
};

template <>
struct joint_b2type<JointType::Motor> {
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
        // TODO: Remove this once all types are updated
        if constexpr (std::is_pointer_v<decltype(current_)>)
            current_ = current_->getNext();
        else
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

template <typename THandle>
class HandleWrapper;

template <typename TUserData, typename THandle>
class OwnedHandle;

template <typename TUserTypes, JointType v_type>
class JointWrapper;

template <typename TUserTypes, typename TJointEdge>
class JointEdgeWrapper;

template <typename TShape, ShapeType v_type = shape_type_v<std::remove_const_t<TShape>>>
class ShapeRefWrapper;

template <typename TUserTypes, typename TShape>
class FixtureWrapper;

template <typename TUserTypes>
class BodyWrapper;

template <typename TUserTypes, typename TWorld>
class WorldRefWrapper;

template <typename TUserTypes, typename TContact>
class ContactWrapper;

template <typename TUserTypes>
using Joint = JointWrapper<TUserTypes, JointType::Unknown>;
template <typename TUserTypes>
using RevoluteJoint = JointWrapper<TUserTypes, JointType::Revolute>;
template <typename TUserTypes>
using PrismaticJoint = JointWrapper<TUserTypes, JointType::Prismatic>;
template <typename TUserTypes>
using DistanceJoint = JointWrapper<TUserTypes, JointType::Distance>;
template <typename TUserTypes>
using PulleyJoint = JointWrapper<TUserTypes, JointType::Pulley>;
template <typename TUserTypes>
using MouseJoint = JointWrapper<TUserTypes, JointType::Mouse>;
template <typename TUserTypes>
using GearJoint = JointWrapper<TUserTypes, JointType::Gear>;
template <typename TUserTypes>
using WheelJoint = JointWrapper<TUserTypes, JointType::Wheel>;
template <typename TUserTypes>
using WeldJoint = JointWrapper<TUserTypes, JointType::Weld>;
template <typename TUserTypes>
using FrictionJoint = JointWrapper<TUserTypes, JointType::Friction>;
template <typename TUserTypes>
using MotorJoint = JointWrapper<TUserTypes, JointType::Motor>;

template <typename TUserTypes>
using JointEdge = JointEdgeWrapper<TUserTypes, b2JointEdge>;
template <typename TUserTypes>
using ConstJointEdge = JointEdgeWrapper<TUserTypes, const b2JointEdge>;
template <typename TUserTypes, typename TConstAs>
using JointEdgeConstAs = JointEdgeWrapper<TUserTypes, dutils::copy_const_t<b2JointEdge, TConstAs>>;

using ShapeRef = ShapeRefWrapper<b2Shape>;
using ConstShapeRef = ShapeRefWrapper<const b2Shape>;
template <typename TConstAs>
using ShapeRefConstAs = ShapeRefWrapper<dutils::copy_const_t<b2Shape, TConstAs>>;

using CircleShapeRef = ShapeRefWrapper<b2CircleShape>;
using ConstCircleShapeRef = ShapeRefWrapper<const b2CircleShape>;
template <typename TConstAs>
using CircleShapeRefConstAs = ShapeRefWrapper<dutils::copy_const_t<b2CircleShape, TConstAs>>;

using EdgeShapeRef = ShapeRefWrapper<b2EdgeShape>;
using ConstEdgeShapeRef = ShapeRefWrapper<const b2EdgeShape>;
template <typename TConstAs>
using EdgeShapeRefConstAs = ShapeRefWrapper<dutils::copy_const_t<b2EdgeShape, TConstAs>>;

using PolygonShapeRef = ShapeRefWrapper<b2PolygonShape>;
using ConstPolygonShapeRef = ShapeRefWrapper<const b2PolygonShape>;
template <typename TConstAs>
using PolygonShapeRefConstAs = ShapeRefWrapper<dutils::copy_const_t<b2PolygonShape, TConstAs>>;

using ChainShapeRef = ShapeRefWrapper<b2ChainShape>;
using ConstChainShapeRef = ShapeRefWrapper<const b2ChainShape>;
template <typename TConstAs>
using ChainShapeRefConstAs = ShapeRefWrapper<dutils::copy_const_t<b2ChainShape, TConstAs>>;

template <typename TUserTypes>
using Fixture = FixtureWrapper<TUserTypes, b2Shape>;
template <typename TUserTypes>
using CircleFixture = FixtureWrapper<TUserTypes, b2CircleShape>;
template <typename TUserTypes>
using EdgeFixture = FixtureWrapper<TUserTypes, b2EdgeShape>;
template <typename TUserTypes>
using PolygonFixture = FixtureWrapper<TUserTypes, b2PolygonShape>;
template <typename TUserTypes>
using ChainFixture = FixtureWrapper<TUserTypes, b2ChainShape>;

template <typename TUserTypes>
using Body = BodyWrapper<TUserTypes>;

template <typename TUserTypes>
using WorldRef = WorldRefWrapper<TUserTypes, b2World>;
template <typename TUserTypes>
using ConstWorldRef = WorldRefWrapper<TUserTypes, const b2World>;
template <typename TUserTypes, typename TConstAs>
using WorldRefConstAs = WorldRefWrapper<TUserTypes, dutils::copy_const_t<b2World, TConstAs>>;

template <typename TUserTypes>
using Contact = ContactWrapper<TUserTypes, b2Contact>;
template <typename TUserTypes>
using ConstContact = ContactWrapper<TUserTypes, const b2Contact>;
template <typename TUserTypes, typename TConstAs>
using ContactConstAs = ContactWrapper<TUserTypes, dutils::copy_const_t<b2Contact, TConstAs>>;

} // namespace detail

struct DefaultUserData {
    using Fixture = void;
    using Body = void;
    using Joint = void;
};

template <typename TUserTypes = DefaultUserData>
class World;

// --- Lifetime

namespace detail {

void destroy(b2Body* body) { body->GetWorld()->DestroyBody(body); }
void destroy(b2Fixture* fixture) { fixture->GetBody()->DestroyFixture(fixture); }
void destroy(b2Joint* joint) { joint->GetBodyA()->GetWorld()->DestroyJoint(joint); }

// --- Owners

// TODO: C++20 use std::bit_cast

template <typename TUserTypes>
Body<TUserTypes>& getOwner(b2Body* body)
{
    Body<TUserTypes>* result;
    std::memcpy(&result, &body->GetUserData().pointer, sizeof result);
    return *result;
}

template <typename TUserData>
void setOwner(b2Body* body, OwnedHandle<TUserData, b2Body>& owner)
{
    auto ptr = &owner;
    std::memcpy(&body->GetUserData().pointer, &ptr, sizeof ptr);
}

template <typename TUserTypes>
Fixture<TUserTypes>& getOwner(b2Fixture* fixture)
{
    Fixture<TUserTypes>* result;
    std::memcpy(&result, &fixture->GetUserData().pointer, sizeof result);
    return *result;
}

template <typename TUserTypes>
void setOwner(b2Fixture* fixture, OwnedHandle<TUserTypes, b2Fixture>& owner)
{
    auto ptr = &owner;
    std::memcpy(&fixture->GetUserData().pointer, &ptr, sizeof ptr);
}

// using ::value instead of _v because of SFINAE
template <typename TUserTypes, typename TJoint>
JointWrapper<TUserTypes, joint_type<TJoint>::value>& getOwner(TJoint* joint)
{
    JointWrapper<TUserTypes, joint_type_v<TJoint>>* result;
    std::memcpy(&result, &joint->GetUserData().pointer, sizeof result);
    return *result;
}

template <typename TUserTypes, typename TJoint>
void setOwner(TJoint* joint, OwnedHandle<TUserTypes, TJoint>& owner)
{
    auto ptr = &owner;
    std::memcpy(&joint->GetUserData().pointer, &ptr, sizeof ptr);
}

template <typename TUserTypes, typename TObject>
auto getOptionalOwner(TObject object)
{
    return object ? &getOwner<TUserTypes>(object) : nullptr;
}

// --- UserData

template <typename TUserData>
struct WithUserData {
    TUserData user_data = {};
};

template <>
struct WithUserData<void> {};

} // namespace detail

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

template <typename TUserData, typename THandle>
class OwnedHandle : public WithUserData<TUserData> {
public:
    OwnedHandle() = default;

    OwnedHandle(const OwnedHandle&) = delete;

    OwnedHandle(OwnedHandle&& other)
        : WithUserData<TUserData>(std::move(other))
        , handle_(std::exchange(other.handle_, nullptr))
    {
        if (handle_)
            setOwner(handle_, *this);
    }

    OwnedHandle& operator=(const OwnedHandle&) = delete;

    OwnedHandle& operator=(OwnedHandle&& other)
    {
        if (handle_)
            detail::destroy(handle_);
        WithUserData<TUserData>::operator=(std::move(other));
        handle_ = std::exchange(other.handle_, nullptr);
        if (handle_)
            setOwner(handle_, *this);
        return *this;
    }

    ~OwnedHandle()
    {
        if (handle_)
            detail::destroy(handle_);
    }

    void destroy() &&
    {
        if (handle_) {
            detail::destroy(handle_);
            handle_ = nullptr;
        }
    }

    // TODO: swap

    explicit operator bool() const { return handle_ != nullptr; }

    friend bool operator==(const OwnedHandle& lhs, const OwnedHandle& rhs) { return lhs.handle_ == rhs.handle_; }
    friend bool operator!=(const OwnedHandle& lhs, const OwnedHandle& rhs) { return !(lhs == rhs); }

private:
    template <typename>
    friend class BodyWrapper;

    template <typename>
    friend struct JointDefBase;

    template <typename, JointType>
    friend class JointWrapper;

    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    void forceHandle(THandle* handle) { handle_ = handle; }

    THandle* handle_ = nullptr;
};

} // namespace detail

// --- Shape

struct CircleShape {
    float radius = 0.0f;
    vec2 position;

private:
    template <typename>
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
    template <typename>
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
    template <typename>
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
    template <typename>
    friend class detail::BodyWrapper;

    using Data = b2PolygonShape;

    void build(Data& shape) const { shape.Set(vertices_.data(), vertex_count_); }

    std::array<b2Vec2, b2_maxPolygonVertices> vertices_;
    int32 vertex_count_;
};

struct BoxShape {
    vec2 size;

private:
    template <typename>
    friend class detail::BodyWrapper;

    using Data = b2PolygonShape;

    void build(Data& shape) const { shape.SetAsBox(size.x(), size.y()); }
};

struct OrientedBoxShape {
    vec2 size;
    vec2 center;
    float angle = 0.0f;

private:
    template <typename>
    friend class detail::BodyWrapper;

    using Data = b2PolygonShape;

    void build(Data& shape) const { shape.SetAsBox(size.x(), size.y(), cast(center), angle); }
};

struct LoopShape {
    std::vector<vec2> vertices;

private:
    template <typename>
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
    template <typename>
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

    template <ShapeType v_shape_type>
    ShapeRefWrapper<shape_b2type_t<v_shape_type>> as() const
    {
        if constexpr (v_shape_type != ShapeType::Unknown) {
            if (getType() != v_shape_type)
                return nullptr;
        }
        return this->handle();
    }

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
        std::visit(dutils::Overloaded{
                       [&](const OneSidedEdgeShape& one_sided_edge_shape) {
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
                       },
                   },
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

} // namespace detail

// --- Fixture

struct FixtureDef {
    float friction = 0.2f;
    float restitution = 0.0f;
    float restitution_threshold = 1.0f * b2_lengthUnitsPerMeter;
    float density = 0.0f;
    bool is_sensor = false;
    Filter filter;

private:
    template <typename>
    friend class detail::BodyWrapper;

    template <typename TUserTypes>
    b2FixtureDef build(detail::Fixture<TUserTypes>* owner, const b2Shape* shape) const
    {
        b2FixtureDef result;
        result.shape = shape;
        // TODO: C++20 use std::bit_cast
        std::memcpy(&result.userData.pointer, &owner, sizeof owner);
        result.friction = friction;
        result.restitution = restitution;
        result.restitutionThreshold = restitution_threshold;
        result.density = density;
        result.isSensor = is_sensor;
        result.filter = filter;
        return result;
    }
};

namespace detail {

template <typename TUserTypes, typename TShape>
class FixtureWrapper : public FixtureWrapper<TUserTypes, b2Shape> {
public:
    using FixtureWrapper<TUserTypes, b2Shape>::FixtureWrapper;

    ShapeRefWrapper<TShape> getShape() { return static_cast<TShape*>(this->handle_->GetShape()); }
    ShapeRefWrapper<const TShape> getShape() const { return static_cast<const TShape*>(this->handle_->GetShape()); }
};

template <typename TUserTypes>
class FixtureWrapper<TUserTypes, b2Shape> : public OwnedHandle<typename TUserTypes::Fixture, b2Fixture> {
public:
    using OwnedHandle<typename TUserTypes::Fixture, b2Fixture>::OwnedHandle;

    ShapeRef getShape() { return this->handle_->GetShape(); }
    ConstShapeRef getShape() const { return this->handle_->GetShape(); }

    void setSensor(bool sensor) { this->handle_->SetSensor(sensor); }
    bool isSensor() const { return this->handle_->IsSensor(); }

    void setFilterData(const Filter& filter) { this->handle_->SetFilterData(filter); }
    const Filter& getFilterData() const { return this->handle_->GetFilterData(); }
    void refilter() { this->handle_->Refilter(); }

    Body<TUserTypes>& getBody() { return getOwner<TUserTypes>(this->handle_->GetBody()); }
    const Body<TUserTypes>& getBody() const { return getOwner<TUserTypes>(this->handle_->GetBody()); }

    Fixture<TUserTypes>* getNext() { return getOptionalOwner(this->handle_->GetNext()); }
    const Fixture<TUserTypes>* getNext() const { return getOptionalOwner(this->handle_->GetNext()); }

    bool testPoint(vec2 p) const { return this->handle_->TestPoint(cast(p)); }
    std::optional<RayCastOutput> rayCast(const RayCastInput& input, int32 childIndex) const
    {
        RayCastOutput output;
        if (!this->handle_->RayCast(&output, input, childIndex))
            return std::nullopt;
        return output;
    }

    MassData getMassData() const { return this->handle_->GetMassData(); }
    void setDensity(float density) { this->handle_->SetDensity(density); }
    float getDensity() const { return this->handle_->GetDensity(); }

    float getFriction() const { return this->handle_->GetFriction(); }
    void setFriction(float friction) { this->handle_->SetFriction(friction); }
    float getRestitution() const { return this->handle_->GetRestitution(); }
    void setRestitution(float restitution) { this->handle_->SetRestitution(restitution); }
    float getRestitutionThreshold() const { return this->handle_->GetRestitutionThreshold(); }
    void setRestitutionThreshold(float threshold) { this->handle_->SetRestitutionThreshold(threshold); }

    const AABB& getAABB(int32 child_index) const { return this->handle_->GetAABB(child_index); }

    void dump(int32 body_index) { this->handle_->Dump(body_index); }
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

} // namespace detail

// --- Body

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
    float gravity_scale = 1.0f;

private:
    template <typename, typename>
    friend class detail::WorldRefWrapper;

    template <typename>
    friend class World;

    template <typename TUserTypes>
    b2BodyDef build(detail::Body<TUserTypes>* owner) const
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
        std::memcpy(&result.userData.pointer, &owner, sizeof owner);
        result.gravityScale = gravity_scale;
        return result;
    }
};

namespace detail {

template <typename TUserTypes>
class BodyWrapper : public OwnedHandle<typename TUserTypes::Body, b2Body> {
public:
    using OwnedHandle<typename TUserTypes::Body, b2Body>::OwnedHandle;

    template <typename TShape>
    [[nodiscard]] auto createFixture(const FixtureDef& fixture, const TShape& shape)
    {
        typename TShape::Data shape_data;
        shape.build(shape_data);
        FixtureWrapper<TUserTypes, typename TShape::Data> result;
        auto def = fixture.build(&result, &shape_data);
        result.forceHandle(this->handle_->CreateFixture(&def));
        return result;
    }

    [[nodiscard]] Fixture<TUserTypes> createFixture(const FixtureDef& fixture, const Shape& shape)
    {
        auto shape_data = std::visit(
            [](auto concrete_shape) {
                typename decltype(concrete_shape)::Data shape_data;
                concrete_shape.build(shape_data);
                return shape_data;
            },
            shape);
        Fixture<TUserTypes> result;
        auto def = fixture.build(&result, &shape_data);
        result.forceHandle(this->handle_->CreateFixture(&def));
        return result;
    }

    template <typename TShape>
    [[nodiscard]] auto createFixture(const TShape& shape, float density = 1.0f)
    {
        FixtureDef def;
        def.density = density;
        return createFixture(def, shape);
    }

    [[nodiscard]] Fixture<TUserTypes> createFixture(const Shape& shape, float density = 1.0f)
    {
        FixtureDef<TUserTypes> def;
        def.density = density;
        return createFixture(def, shape);
    }

    void setTransform(vec2 position, float angle) { this->handle_->SetTransform(cast(position), angle); }
    const Transform& getTransform() const { return this->handle_->GetTransform(); }
    vec2 getPosition() const { return cast(this->handle_->GetPosition()); }
    float getAngle() const { return this->handle_->GetAngle(); }
    vec2 getWorldCenter() const { return cast(this->handle_->GetWorldCenter()); }
    vec2 getLocalCenter() const { return cast(this->handle_->GetLocalCenter()); }

    void setLinearVelocity(vec2 velocity) { this->handle_->SetLinearVelocity(cast(velocity)); }
    vec2 getLinearVelocity() const { return cast(this->handle_->GetLinearVelocity()); }
    void setAngularVelocity(float omega) { this->handle_->SetAngularVelocity(omega); }
    float getAngularVelocity() const { return this->handle_->GetAngularVelocity(); }

    template <typename TApplicable>
    void apply(const TApplicable& applicable, bool wake = true)
    {
        Applier{this->handle_, wake}(applicable);
    }

    void apply(const Applicable& applicable, bool wake = true) { std::visit(Applier{this->handle_, wake}, applicable); }

    float getMass() const { return this->handle_->GetMass(); }
    float getInertia() const { return this->handle_->GetInertia(); }

    MassData getMassData() const
    {
        MassData result;
        this->handle_->GetMassData(&result);
        return result;
    }

    void setMassData(const MassData& mass_data) { this->handle_->SetMassData(&mass_data); }

    void resetMassData() { this->handle_->ResetMassData(); }

    vec2 getWorldPoint(vec2 local_point) const { return cast(this->handle_->GetWorldPoint(cast(local_point))); }
    vec2 getWorldVector(vec2 local_vector) const { return cast(this->handle_->GetWorldVector(cast(local_vector))); }
    vec2 getLocalPoint(vec2 world_point) const { return cast(this->handle_->GetLocalPoint(cast(world_point))); }
    vec2 getLocalVector(vec2 world_vector) const { return cast(this->handle_->GetLocalVector(cast(world_vector))); }

    vec2 getLinearVelocityFromWorldPoint(vec2 world_point) const
    {
        return cast(this->handle_->GetLinearVelocityFromWorldPoint(cast(world_point)));
    }

    vec2 getLinearVelocityFromLocalPoint(vec2 local_point) const
    {
        return cast(this->handle_->GetLinearVelocityFromLocalPoint(cast(local_point)));
    }

    float getLinearDamping() const { return this->handle_->GetLinearDamping(); }
    void setLinearDamping(float linear_damping) { this->handle_->SetLinearDamping(linear_damping); }
    float getAngularDamping() const { return this->handle_->GetAngularDamping(); }
    void setAngularDamping(float angular_damping) { this->handle_->SetAngularDamping(angular_damping); }

    float getGravityScale() const { return this->handle_->GetGravityScale(); }
    void setGravityScale(float scale) { this->handle_->SetGravityScale(scale); }

    void setType(BodyType type) { this->handle_->SetType(cast(type)); }
    BodyType getType() const { return cast(this->handle_->GetType()); }

    void setBullet(bool flag) { this->handle_->SetBullet(flag); }
    bool isBullet() const { return this->handle_->IsBullet(); }

    void setSleepingAllowed(bool flag) { this->handle_->SetSleepingAllowed(flag); }
    bool isSleepingAllowed() const { return this->handle_->IsSleepingAllowed(); }

    void setAwake(bool flag) { this->handle_->SetAwake(flag); }
    bool isAwake() const { return this->handle_->IsAwake(); }

    void setEnabled(bool flag) { this->handle_->SetEnabled(flag); }
    bool isEnabled() const { return this->handle_->IsEnabled(); }

    void setFixedRotation(bool flag) { this->handle_->SetFixedRotation(flag); }
    bool isFixedRotation() const { return this->handle_->IsFixedRotation(); }

    ForwardIterable<Fixture<TUserTypes>*> fixtures() { return {this->handle_->GetFixtureList()}; }
    ForwardIterable<const Fixture<TUserTypes>*> fixtures() const { return {this->handle_->GetFixtureList()}; }
    BidirectionalIterable<JointEdge<TUserTypes>> joints() { return {this->handle_->GetJointList()}; }
    BidirectionalIterable<ConstJointEdge<TUserTypes>> joints() const { return {this->handle_->GetJointList()}; }
    ForwardIterable<Contact<TUserTypes>> contacts() { return {this->handle_->GetContactList()}; }
    ForwardIterable<ConstContact<TUserTypes>> contacts() const { return {this->handle_->GetContactList()}; }

    Body<TUserTypes>* getNext() { return getOptionalOwner<TUserTypes>(this->handle_->GetNext()); }
    const Body<TUserTypes>* getNext() const { return getOptionalOwner<TUserTypes>(this->handle_->GetNext()); }

    WorldRef<TUserTypes> getWorld() { this->handle_->GetWorld(); }
    ConstWorldRef<TUserTypes> getWorld() const { this->handle_->GetWorld(); }

    void dump() { this->handle_->Dump(); }
};

// --- Joint

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Unknown> : public OwnedHandle<typename TUserTypes::Joint, b2Joint> {
public:
    using OwnedHandle<typename TUserTypes::Joint, b2Joint>::OwnedHandle;

    JointType getType() const { return cast(this->handle_->GetType()); }

    Body<TUserTypes>& getBodyA() { return getOwner<TUserTypes>(this->handle_->GetBodyA()); }
    const Body<TUserTypes>& getBodyA() const { return getOwner<TUserTypes>(this->handle_->GetBodyA()); }
    Body<TUserTypes>& getBodyB() { return getOwner<TUserTypes>(this->handle_->GetBodyB()); }
    const Body<TUserTypes>& getBodyB() const { return getOwner<TUserTypes>(this->handle_->GetBodyB()); }

    vec2 getAnchorA() const { return cast(this->handle_->GetAnchorA()); }
    vec2 getAnchorB() const { return cast(this->handle_->GetAnchorB()); }

    vec2 getReactionForce(float inv_dt) const { return cast(this->handle_->GetReactionForce(inv_dt)); }
    float getReactionTorque(float inv_dt) const { return this->handle_->GetReactionTorque(inv_dt); }

    Joint<TUserTypes>* getNext() { return getOptionalOwner<TUserTypes>(this->handle_->GetNext()); }
    const Joint<TUserTypes>* getNext() const { return getOptionalOwner<TUserTypes>(this->handle_->GetNext()); }

    bool isEnabled() const { return this->handle_->IsEnabled(); }

    bool getCollideConnected() const { return this->handle_->GetCollideConnected(); }

    void dump() { this->handle_->Dump(); }

    void shiftOrigin(vec2 new_origin) { this->handle_->ShiftOrigin(cast(new_origin)); }

    void draw(Draw* draw) { this->handle_->Draw(draw); }

protected:
    // Disallow copy to base class. All instances must be of the correct type.

    JointWrapper() = default;
    JointWrapper(const JointWrapper&) = delete;
    JointWrapper(JointWrapper&&) = default;
    JointWrapper& operator=(const JointWrapper&) = delete;
    JointWrapper& operator=(JointWrapper&&) = default;
};

template <typename TUserTypes, JointType v_joint_type>
class JointWrapperTyped : public JointWrapper<TUserTypes, JointType::Unknown> {
public:
    vec2 getAnchorA() const { return cast(this->handle()->Joint::GetAnchorA()); }
    vec2 getAnchorB() const { return cast(this->handle()->Joint::GetAnchorB()); }

    vec2 getReactionForce(float inv_dt) const { return cast(this->handle()->Joint::GetReactionForce(inv_dt)); }
    float getReactionTorque(float inv_dt) const { return this->handle()->Joint::GetReactionTorque(inv_dt); }

    void dump() { this->handle()->Joint::Dump(); }

    void shiftOrigin(vec2 new_origin) { this->handle()->Joint::ShiftOrigin(cast(new_origin)); }

    void draw(Draw* draw) { this->handle()->Joint::Draw(draw); }

private:
    using Joint = joint_b2type_t<v_joint_type>;

    template <typename, JointType>
    friend class JointWrapper;

    auto handle() { return static_cast<Joint>(this->handle_); }
};

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Revolute> : public JointWrapperTyped<TUserTypes, JointType::Revolute> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Revolute>::JointWrapperTyped;

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

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Prismatic> : public JointWrapperTyped<TUserTypes, JointType::Prismatic> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Prismatic>::JointWrapperTyped;

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

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Distance> : public JointWrapperTyped<TUserTypes, JointType::Distance> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Distance>::JointWrapperTyped;

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

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Pulley> : public JointWrapperTyped<TUserTypes, JointType::Pulley> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Pulley>::JointWrapperTyped;

    vec2 GetGroundAnchorA() const { return cast(this->handle()->GetGroundAnchorA()); }
    vec2 GetGroundAnchorB() const { return cast(this->handle()->GetGroundAnchorB()); }

    float getLengthA() const { return this->handle()->GetLengthA(); }
    float getLengthB() const { return this->handle()->GetLengthB(); }

    float getRatio() const { return this->handle()->GetRatio(); }

    float getCurrentLengthA() const { return this->handle()->GetCurrentLengthA(); }
    float getCurrentLengthB() const { return this->handle()->GetCurrentLengthB(); }
};

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Mouse> : public JointWrapperTyped<TUserTypes, JointType::Mouse> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Mouse>::JointWrapperTyped;

    void setTarget(vec2 target) const { this->handle()->SetTarget(cast(target)); }
    vec2 getTarget() const { return cast(this->handle()->GetTarget()); }

    void setMaxForce(float force) const { this->handle()->SetMaxForce(force); }
    float getMaxForce() const { return this->handle()->GetMaxForce(); }

    void setStiffness(float stiffness) const { this->handle()->SetStiffness(stiffness); }
    float getStiffness() const { return this->handle()->GetStiffness(); }

    void setDamping(float damping) const { this->handle()->SetDamping(damping); }
    float getDamping() const { return this->handle()->GetDamping(); }
};

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Gear> : public JointWrapperTyped<TUserTypes, JointType::Gear> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Gear>::JointWrapperTyped;

    Joint<TUserTypes>& GetJoint1() { return getOwner<TUserTypes>(this->handle()->GetJoint1()); }
    const Joint<TUserTypes>& GetJoint1() const { return getOwner<TUserTypes>(this->handle()->GetJoint1()); }
    Joint<TUserTypes>& GetJoint2() { return getOwner<TUserTypes>(this->handle()->GetJoint2()); }
    const Joint<TUserTypes>& GetJoint2() const { return getOwner<TUserTypes>(this->handle()->GetJoint2()); }

    void setRatio(float ratio) const { this->handle()->SetRatio(ratio); }
    float getRatio() const { return this->handle()->GetRatio(); }
};

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Wheel> : public JointWrapperTyped<TUserTypes, JointType::Wheel> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Wheel>::JointWrapperTyped;

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

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Weld> : public JointWrapperTyped<TUserTypes, JointType::Weld> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Weld>::JointWrapperTyped;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }
    float getReferenceAngle() const { return this->handle()->GetReferenceAngle(); }

    void setStiffness(float hz) const { this->handle()->SetStiffness(hz); }
    float getStiffness() const { return this->handle()->GetStiffness(); }

    void setDamping(float damping) const { this->handle()->SetDamping(damping); }
    float getDamping() const { return this->handle()->GetDamping(); }
};

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Friction> : public JointWrapperTyped<TUserTypes, JointType::Friction> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Friction>::JointWrapperTyped;

    vec2 getLocalAnchorA() const { return cast(this->handle()->GetLocalAnchorA()); }
    vec2 getLocalAnchorB() const { return cast(this->handle()->GetLocalAnchorB()); }

    void setMaxForce(float force) const { this->handle()->SetMaxForce(force); }
    float getMaxForce() const { return this->handle()->GetMaxForce(); }

    void setMaxTorque(float torque) const { this->handle()->SetMaxTorque(torque); }
    float getMaxTorque() const { return this->handle()->GetMaxTorque(); }
};

template <typename TUserTypes>
class JointWrapper<TUserTypes, JointType::Motor> : public JointWrapperTyped<TUserTypes, JointType::Motor> {
public:
    using JointWrapperTyped<TUserTypes, JointType::Motor>::JointWrapperTyped;

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

template <typename TUserTypes>
struct JointDefBase {
    const Body<TUserTypes>* body_a = nullptr;
    const Body<TUserTypes>* body_b = nullptr;

protected:
    void build(b2JointDef& def, Joint<TUserTypes>* owner) const
    {
        // TODO: C++20 use std::bit_cast
        std::memcpy(&def.userData.pointer, &owner, sizeof owner);
        def.bodyA = body_a->handle_;
        def.bodyB = body_b->handle_;
    }
};

template <typename TUserTypes>
struct JointDefNoCollideDefault : JointDefBase<TUserTypes> {
    bool collide_connected = false;

protected:
    void build(b2JointDef& def, Joint<TUserTypes>* owner) const
    {
        JointDefBase<TUserTypes>::build(def, owner);
        def.collideConnected = collide_connected;
    }
};

template <typename TUserTypes>
struct RevoluteJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Revolute;

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

    b2RevoluteJointDef build(Joint<TUserTypes>* owner) const
    {
        b2RevoluteJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
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

template <typename TUserTypes>
struct PrismaticJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Prismatic;

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

    b2PrismaticJointDef build(PrismaticJoint<TUserTypes>* owner) const
    {
        b2PrismaticJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
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

template <typename TUserTypes>
struct DistanceJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Distance;

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

    b2DistanceJointDef build(DistanceJoint<TUserTypes>* owner) const
    {
        b2DistanceJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
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

template <typename TUserTypes>
struct PulleyJointDef : JointDefBase<TUserTypes> {
    static constexpr JointType type = JointType::Pulley;

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

    b2PulleyJointDef build(PulleyJoint<TUserTypes>* owner) const
    {
        b2PulleyJointDef def;
        JointDefBase<TUserTypes>::build(def, owner);
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

template <typename TUserTypes>
struct MouseJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Mouse;

    vec2 target;
    float max_force = 0.0f;
    float stiffness = 0.0f;
    float damping = 0.0f;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2MouseJointDef build(MouseJoint<TUserTypes>* owner) const
    {
        b2MouseJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
        def.target = cast(target);
        def.maxForce = max_force;
        def.stiffness = stiffness;
        def.damping = damping;
        return def;
    }
};

template <typename TUserTypes>
struct GearJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Gear;

    const Joint<TUserTypes>* joint1;
    const Joint<TUserTypes>* joint2;
    float ratio;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2GearJointDef build(GearJoint<TUserTypes>* owner) const
    {
        b2GearJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
        def.joint1 = joint1;
        def.joint2 = joint2;
        def.ratio = ratio;
        return def;
    }
};

template <typename TUserTypes>
struct WheelJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Wheel;

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

    b2WheelJointDef build(WheelJoint<TUserTypes>* owner) const
    {
        b2WheelJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
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

template <typename TUserTypes>
struct WeldJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Weld;

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

    b2WeldJointDef build(WeldJoint<TUserTypes>* owner) const
    {
        b2WeldJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.referenceAngle = reference_angle;
        def.stiffness = stiffness;
        def.damping = damping;
        return def;
    }
};

template <typename TUserTypes>
struct FrictionJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Friction;

    vec2 local_anchor_a;
    vec2 local_anchor_b;
    float max_force;
    float max_torque;

private:
    template <typename, typename>
    friend class WorldRefWrapper;

    template <typename>
    friend class dang::box2d::World;

    b2FrictionJointDef build(FrictionJoint<TUserTypes>* owner) const
    {
        b2FrictionJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
        def.localAnchorA = cast(local_anchor_a);
        def.localAnchorB = cast(local_anchor_b);
        def.maxForce = max_force;
        def.maxTorque = max_torque;
        return def;
    }
};

template <typename TUserTypes>
struct MotorJointDef : JointDefNoCollideDefault<TUserTypes> {
    static constexpr JointType type = JointType::Motor;

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

    b2MotorJointDef build(MotorJoint<TUserTypes>* owner) const
    {
        b2MotorJointDef def;
        JointDefNoCollideDefault<TUserTypes>::build(def, owner);
        def.linearOffset = cast(linear_offset);
        def.angularOffset = angular_offset;
        def.maxForce = max_force;
        def.maxTorque = max_torque;
        def.correctionFactor = correction_factor;
        return def;
    }
};

template <typename TUserTypes>
using JointDef = std::variant<RevoluteJointDef<TUserTypes>,
                              PrismaticJointDef<TUserTypes>,
                              DistanceJointDef<TUserTypes>,
                              PulleyJointDef<TUserTypes>,
                              MouseJointDef<TUserTypes>,
                              GearJointDef<TUserTypes>,
                              WheelJointDef<TUserTypes>,
                              WeldJointDef<TUserTypes>,
                              FrictionJointDef<TUserTypes>,
                              MotorJointDef<TUserTypes>>;

// --- JointEdge

template <typename TUserTypes, typename TJointEdge>
class JointEdgeWrapper : public HandleWrapper<TJointEdge> {
public:
    using HandleWrapper<TJointEdge>::HandleWrapper;

    dutils::copy_const_t<Body<TUserTypes>, TJointEdge>& other() const
    {
        return getOwner<TUserTypes>(this->handle()->other);
    }

    dutils::copy_const_t<Joint<TUserTypes>, TJointEdge>& joint() const
    {
        return getOwner<TUserTypes>(this->handle()->joint);
    }

    JointEdgeWrapper getPrev() const { return this->handle()->prev; }
    JointEdgeWrapper getNext() const { return this->handle()->next; }
};

// --- Contact

template <typename TUserTypes, typename TContact>
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

    ContactConstAs<TUserTypes, TContact> getNext() const { return this->handle()->GetNext(); }

    dutils::copy_const_t<Fixture<TUserTypes>, TContact>& getFixtureA() const
    {
        return getOwner<TUserTypes>(this->handle()->GetFixtureA());
    }

    int32 getChildIndexA() const { return this->handle()->GetChildIndexA(); }

    dutils::copy_const_t<Fixture<TUserTypes>, TContact>& getFixtureB() const
    {
        return getOwner<TUserTypes>(this->handle()->GetFixtureB());
    }

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

// --- World

namespace detail {

template <typename TUserTypes>
using QueryCallback = std::function<bool(Fixture<TUserTypes>&)>;

template <typename TUserTypes>
struct RayCastData {
    Fixture<TUserTypes>* fixture;
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

template <typename TUserTypes>
using RayCastCallback = std::function<float(const RayCastData<TUserTypes>&)>;

template <typename TUserTypes>
class QueryCallbackWrapper : public b2QueryCallback {
public:
    QueryCallbackWrapper(QueryCallback<TUserTypes> callback)
        : callback_(std::move(callback))
    {}

    bool ReportFixture(b2Fixture* fixture) override { return callback_(getOwner<TUserTypes>(fixture)); }

private:
    QueryCallback<TUserTypes> callback_;
};

template <typename TUserTypes>
class RayCastCallbackWrapper : public b2RayCastCallback {
public:
    RayCastCallbackWrapper(RayCastCallback<TUserTypes> callback)
        : callback_(std::move(callback))
    {}

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
    {
        return callback_({&getOwner<TUserTypes>(fixture), cast(point), cast(normal), fraction});
    }

private:
    RayCastCallback<TUserTypes> callback_;
};

template <typename TUserTypes, typename TWorld>
class WorldRefWrapper : public HandleWrapper<TWorld> {
public:
    using HandleWrapper<TWorld>::HandleWrapper;

    void setDebugDraw(Draw* debug_draw) { this->handle()->SetDebugDraw(debug_draw); }
    void debugDraw() const { this->handle()->DebugDraw(); }

    [[nodiscard]] Body<TUserTypes> createBody(const BodyDef& body) const
    {
        Body<TUserTypes> result;
        auto def = body.build(&result);
        result.forceHandle(this->handle()->CreateBody(&def));
        return result;
    }

    [[nodiscard]] Body<TUserTypes> createBody(BodyType body_type = BodyType::Static) const
    {
        BodyDef def;
        def.type = body_type;
        return createBody(def);
    }

    template <typename TJointDef>
    [[nodiscard]] auto createJoint(const TJointDef& joint) const
    {
        JointWrapper<TUserTypes, TJointDef::type> result;
        auto def = joint.build(&result);
        result.forceHandle(this->handle()->CreateJoint(&def));
        return result;
    }

    void step(float time_step, int32 velocity_iterations, int32 position_iterations) const
    {
        this->handle()->Step(time_step, velocity_iterations, position_iterations);
    }

    void clearForces() const { this->handle()->ClearForces(); }

    void queryAABB(QueryCallback<TUserTypes> callback, AABB aabb) const
    {
        QueryCallbackWrapper<TUserTypes> wrapper{std::move(callback)};
        this->handle()->QueryAABB(&wrapper, aabb);
    }

    void rayCast(RayCastCallback<TUserTypes> callback, vec2 point1, vec2 point2) const
    {
        RayCastCallbackWrapper<TUserTypes> wrapper{std::move(callback)};
        this->handle()->RayCast(&wrapper, cast(point1), cast(point2));
    }

    ForwardIterable<dutils::copy_const_t<Body<TUserTypes>, TWorld>*> bodies() const
    {
        return this->handle()->GetBodyList();
    }

    ForwardIterable<dutils::copy_const_t<Joint<TUserTypes>, TWorld>*> joints() const
    {
        return this->handle()->GetJointList();
    }

    ForwardIterable<ContactConstAs<TUserTypes, TWorld>> contacts() const { return {this->handle()->GetContactList()}; }

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

template <typename TUserTypes>
class World {
public:
    using UserTypes = TUserTypes;

    using RevoluteJointDef = detail::RevoluteJointDef<UserTypes>;
    using PrismaticJointDef = detail::PrismaticJointDef<UserTypes>;
    using DistanceJointDef = detail::DistanceJointDef<UserTypes>;
    using PulleyJointDef = detail::PulleyJointDef<UserTypes>;
    using MouseJointDef = detail::MouseJointDef<UserTypes>;
    using GearJointDef = detail::GearJointDef<UserTypes>;
    using WheelJointDef = detail::WheelJointDef<UserTypes>;
    using WeldJointDef = detail::WeldJointDef<UserTypes>;
    using FrictionJointDef = detail::FrictionJointDef<UserTypes>;
    using MotorJointDef = detail::MotorJointDef<UserTypes>;

    using JointDef = detail::JointDef<UserTypes>;

    using Fixture = detail::Fixture<UserTypes>;
    using CircleFixture = detail::CircleFixture<UserTypes>;
    using EdgeFixture = detail::EdgeFixture<UserTypes>;
    using PolygonFixture = detail::PolygonFixture<UserTypes>;
    using ChainFixture = detail::ChainFixture<UserTypes>;

    using Body = detail::Body<UserTypes>;

    using Joint = detail::Joint<UserTypes>;
    using RevoluteJoint = detail::RevoluteJoint<UserTypes>;
    using PrismaticJoint = detail::PrismaticJoint<UserTypes>;
    using DistanceJoint = detail::DistanceJoint<UserTypes>;
    using PulleyJoint = detail::PulleyJoint<UserTypes>;
    using MouseJoint = detail::MouseJoint<UserTypes>;
    using GearJoint = detail::GearJoint<UserTypes>;
    using WheelJoint = detail::WheelJoint<UserTypes>;
    using WeldJoint = detail::WeldJoint<UserTypes>;
    using FrictionJoint = detail::FrictionJoint<UserTypes>;
    using MotorJoint = detail::MotorJoint<UserTypes>;

    using JointEdge = detail::JointEdge<UserTypes>;
    using ConstJointEdge = detail::ConstJointEdge<UserTypes>;
    template <typename TConstAs>
    using JointEdgeConstAs = detail::JointEdgeConstAs<UserTypes, TConstAs>;

    using Contact = detail::Contact<UserTypes>;
    using ConstContact = detail::ConstContact<UserTypes>;
    template <typename TConstAs>
    using ContactConstAs = detail::ContactConstAs<UserTypes, TConstAs>;

    using WorldRef = detail::WorldRef<UserTypes>;
    using ConstWorldRef = detail::ConstWorldRef<UserTypes>;
    template <typename TConstAs>
    using WorldRefConstAs = detail::WorldRefConstAs<UserTypes, TConstAs>;

    using QueryCallback = detail::QueryCallback<UserTypes>;

    using RayCastData = detail::RayCastData<UserTypes>;
    using RayCastCallback = detail::RayCastCallback<UserTypes>;

    using ContactFilter = std::function<bool(Fixture&, Fixture&)>;

    explicit World(vec2 gravity = {})
        : world_(cast(gravity))
    {
        world_.SetDestructionListener(&destruction_listener_);
        world_.SetContactListener(&contact_listener_);
    }

    // b2World is neither copyable nor movable.
    // Use std::optional or std::unique_ptr for movability.
    World(const World&) = delete;
    World(World&&) = delete;
    World& operator=(const World&) = delete;
    World& operator=(World&&) = delete;

    ~World()
    {
        // Set all owners of bodies, fixtures and joints to null.
        // The manual loops are necessary, as setting the handles to null breaks the corresponding iterator.

        for (auto body = world_.GetBodyList(); body; body = body->GetNext()) {
            for (auto fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
                detail::getOwner<UserTypes>(fixture).forceHandle(nullptr);
            detail::getOwner<UserTypes>(body).forceHandle(nullptr);
        }

        for (auto joint = world_.GetJointList(); joint; joint = joint->GetNext())
            detail::getOwner<UserTypes>(joint).forceHandle(nullptr);
    }

    void setContactFilter(ContactFilter should_collide)
    {
        world_.SetContactFilter(should_collide ? &contact_filter_wrapper_ : nullptr);
        contact_filter_wrapper_ = {std::move(should_collide)};
    }

    void setDebugDraw(Draw* debug_draw) { world_.SetDebugDraw(debug_draw); }
    void debugDraw() { world_.DebugDraw(); }

    [[nodiscard]] Body createBody(const BodyDef& body) { return WorldRef{&world_}.createBody(body); }

    [[nodiscard]] Body createBody(BodyType body_type = BodyType::Static)
    {
        return WorldRef{&world_}.createBody(body_type);
    }

    template <typename TJointDef>
    [[nodiscard]] auto createJoint(const TJointDef& joint)
    {
        return WorldRef{&world_}.createJoint(joint);
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

    ForwardIterable<Body*> bodies() { return detail::getOptionalOwner<UserTypes>(world_.GetBodyList()); }
    ForwardIterable<const Body*> bodies() const { return detail::getOptionalOwner<UserTypes>(world_.GetBodyList()); }

    ForwardIterable<Joint*> joints() { return detail::getOptionalOwner<UserTypes>(world_.GetJointList()); }
    ForwardIterable<const Joint*> joints() const { return detail::getOptionalOwner<UserTypes>(world_.GetJointList()); }

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

    dutils::Event<Contact> on_begin_contact;
    dutils::Event<Contact> on_end_contact;
    dutils::Event<Contact, const Manifold*> on_pre_solve;
    dutils::Event<Contact, const ContactImpulse*> on_post_solve;

private:
    class DestructionListener : public b2DestructionListener {
    public:
        void SayGoodbye(b2Joint* joint) override { detail::getOwner<UserTypes>(joint).forceHandle(nullptr); }
        void SayGoodbye(b2Fixture* fixture) override { detail::getOwner<UserTypes>(fixture).forceHandle(nullptr); }
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
            return callback_(detail::getOwner<UserTypes>(fixture_a), detail::getOwner<UserTypes>(fixture_b));
        }

    private:
        ContactFilter callback_;
    };

    b2World world_;
    DestructionListener destruction_listener_;
    ContactListener contact_listener_{this};
    ContactFilterWrapper contact_filter_wrapper_;
};

} // namespace dang::box2d

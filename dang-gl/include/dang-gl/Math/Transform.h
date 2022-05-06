#pragma once

#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/global.h"
#include "dang-utils/event.h"

namespace dang::gl {

/// @brief Thrown, when setting a transform parent introduced a cycle.
class TransformCycleError : public std::runtime_error {
    using runtime_error::runtime_error;
};

class Transform;

using UniqueTransform = std::unique_ptr<Transform>;
using SharedTransform = std::shared_ptr<Transform>;
using WeakTransform = std::weak_ptr<Transform>;

/// @brief Represents a transformation, made up of a quaternion and an optional parent.
/// @remark This class can be used directly, however parenting only works with SharedTransform.
class Transform {
public:
    using Event = dutils::Event<Transform>;

    /// @brief Creates a new pointer-based transform.
    static UniqueTransform create();

    /// @brief The own transformation, without any parent transform.
    const dquat& ownTransform() const;
    /// @brief Sets the own transform to the given quaternion, triggering the on_change event.
    void setOwnTransform(const dquat& transform);

    /// @brief The full transformation, including all parent transformations.
    const dquat& fullTransform();

    /// @brief The optional parent of this transformation.
    SharedTransform parent() const;
    /// @brief Checks, if the chain of parents contains the given transform.
    bool parentChainContains(const Transform& transform) const;
    /// @brief UNSAFE! Forces the parent of this transform to the given transform, without checking for potential
    /// cycles.
    /// @remark A cycle will cause an immediate stack overflow, from recursively calling parent change events.
    void forceParent(const SharedTransform& parent);
    /// @brief Tries to set the parent of this transform to the given transform and returns false if it would introduce
    /// a cycle.
    bool trySetParent(const SharedTransform& parent);
    /// @brief Tries to set the parent of this transform to the given transform and throws a TransformCycleError if it
    /// would introduce a cycle.
    void setParent(const SharedTransform& parent);
    /// @brief Removes the current parent, which is the same as setting the parent to nullptr.
    void resetParent();

    /// @brief Triggered, when the full transformation changes, because either the own transformation or that of any
    /// parent changed.
    Event on_change;
    /// @brief Triggered, when the parent of this transform changed.
    Event on_parent_change;

private:
    dquat own_transform_;
    std::optional<dquat> full_transform_;
    SharedTransform parent_;
    Event::Subscription parent_change_;
};

} // namespace dang::gl

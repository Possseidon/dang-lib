#pragma once

#include "dang-utils/event.h"

#include "Types.h"

namespace dang::gl
{

/// <summary>Thrown, when setting a transform parent introduced a cycle.</summary>
class TransformCycleError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Transform;

using UniqueTransform = std::unique_ptr<Transform>;
using SharedTransform = std::shared_ptr<Transform>;
using WeakTransform = std::weak_ptr<Transform>;

/// <summary>Represents a transformation, made up of a quaternion and an optional parent.</summary>
/// <remarks>This class can be used directly, however parenting only works with SharedTransform.</remarks>
class Transform {
public:
    using Event = dutils::Event<Transform>;

    /// <summary>Creates a new pointer-based transform.</summary>
    static UniqueTransform create();

    /// <summary>The own transformation, without any parent transform.</summary>
    const dquat& ownTransform() const;
    /// <summary>Sets the own transform to the given quaternion, triggering the onChange event.</summary>
    void setOwnTransform(const dquat& transform);

    /// <summary>The full transformation, including all parent transformations.</summary>
    const dquat& fullTransform();

    /// <summary>The optional parent of this transformation.</summary>
    SharedTransform parent() const;
    /// <summary>Checks, if the chain of parents contains the given transform.</summary>
    bool parentChainContains(const Transform& transform) const;
    /// <summary>UNSAFE! Forces the parent of this transform to the given transform, without checking for potential cycles.</summary>
    /// <remarks>A cycle will cause the full transform calculation to recurse indefinitely and likely cause a stack overflow.</remarks>
    void forceParent(const SharedTransform& parent);
    /// <summary>Tries to set the parent of this transform to the given transform and returns false if it would introduce a cycle.</summary>
    bool trySetParent(const SharedTransform& parent);
    /// <summary>Tries to set the parent of this transform to the given transform and throws a TransformCycleError if it would introduce a cycle.</summary>
    void setParent(const SharedTransform& parent);

    /// <summary>Triggered, when the full transformation changes, because either the own transformation or that of any parent changed.</summary>
    Event onChange;
    /// <summary>Triggered, when the parent of this transform changed.</summary>
    Event onParentChange;

private:
    dquat own_transform_;
    std::optional<dquat> full_transform_;
    SharedTransform parent_;
    std::optional<Event::Subscription> parent_change_;
};

}

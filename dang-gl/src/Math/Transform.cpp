#include "pch.h"

#include "Math/Transform.h"

namespace dang::gl {

UniqueTransform Transform::create() { return std::make_unique<Transform>(); }

const dquat& Transform::ownTransform() const { return own_transform_; }

void Transform::setOwnTransform(const dquat& transform)
{
    own_transform_ = transform;
    full_transform_.reset();
    onChange(*this);
}

const dquat& Transform::fullTransform()
{
    if (!full_transform_) {
        if (parent_)
            full_transform_ = own_transform_ * parent_->fullTransform();
        else
            full_transform_ = own_transform_;
    }
    return *full_transform_;
}

SharedTransform Transform::parent() const { return parent_; }

bool Transform::parentChainContains(const Transform& transform) const
{
    if (this == &transform)
        return true;
    auto current = parent();
    while (current != nullptr) {
        if (&*current == &transform)
            return true;
        current = current->parent();
    }
    return false;
}

void Transform::forceParent(const SharedTransform& parent)
{
    parent_ = parent;
    if (parent) {
        auto parent_change = [&] {
            full_transform_.reset();
            onChange(*this);
        };
        parent_change_ = parent->onChange.subscribe(parent_change);
    }
    else {
        parent_change_.remove();
    }
    full_transform_.reset();
    onParentChange(*this);
    onChange(*this);
}

bool Transform::trySetParent(const SharedTransform& parent)
{
    if (parent == parent_)
        return true;

    if (parent && parent->parentChainContains(*this))
        return false;

    forceParent(parent);

    return true;
}

void Transform::setParent(const SharedTransform& parent)
{
    if (!trySetParent(parent))
        throw TransformCycleError("Cannot set transform parent, as it would introduce a cycle.");
}

void Transform::resetParent()
{
    if (parent_)
        forceParent(nullptr);
}

} // namespace dang::gl

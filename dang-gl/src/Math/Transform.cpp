#include "dang-gl/Math/Transform.h"

namespace dang::gl {

Transform::Transform(const dquat& own_transform)
    : own_transform_(own_transform)
{}

const dquat& Transform::ownTransform() const { return own_transform_; }

void Transform::setOwnTransform(const dquat& transform)
{
    own_transform_ = transform;
    full_transform_.reset();
    on_change(*this);
}

const dquat& Transform::fullTransform() const
{
    if (!full_transform_)
        full_transform_ = parent_ ? own_transform_ * parent_->fullTransform() : own_transform_;
    return *full_transform_;
}

const SharedTransform& Transform::parent() const { return parent_; }

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

void Transform::forceParent(SharedTransform parent)
{
    parent_ = std::move(parent);
    if (parent_) {
        auto parent_change = [&] {
            full_transform_.reset();
            on_change(*this);
        };
        parent_change_ = parent_->on_change.subscribe(parent_change);
    }
    else {
        parent_change_.remove();
    }
    full_transform_.reset();
    on_parent_change(*this);
    on_change(*this);
}

bool Transform::trySetParent(SharedTransform parent)
{
    if (parent == parent_)
        return true;

    if (parent && parent->parentChainContains(*this))
        return false;

    forceParent(std::move(parent));

    return true;
}

void Transform::setParent(SharedTransform parent)
{
    if (!trySetParent(std::move(parent)))
        throw TransformCycleError("Cannot set transform parent, as it would introduce a cycle.");
}

void Transform::resetParent()
{
    if (parent_)
        forceParent(nullptr);
}

} // namespace dang::gl

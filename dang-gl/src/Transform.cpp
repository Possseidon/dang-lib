#include "pch.h"
#include "Transform.h"

namespace dang::gl
{

UniqueTransform Transform::create()
{
    return std::make_unique<Transform>();
}

const dmath::dquat& Transform::ownTransform() const
{
    return own_transform_;
}

void Transform::setOwnTransform(const dmath::dquat& transform)
{
    own_transform_ = transform;
    full_transform_.reset();
    onChange(*this);
}

const dmath::dquat& Transform::fullTransform()
{
    if (!full_transform_) {
        if (parent_)
            full_transform_ = own_transform_ * parent_->fullTransform();
        else
            full_transform_ = own_transform_;
    }
    return *full_transform_;
}

std::shared_ptr<Transform> Transform::parent() const
{
    return parent_;
}

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
    parent_change_ = std::nullopt;
    parent_ = parent;
    if (parent) {
        parent_change_.emplace(parent->onChange.subscribe(
            [&] {
                full_transform_.reset();
                onChange(*this);
            }));
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

}

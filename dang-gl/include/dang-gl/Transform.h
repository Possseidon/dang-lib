#pragma once

#include "dang-math/quaternion.h"
#include "dang-math/vector.h"
#include "dang-utils/event.h"

namespace dang::gl
{

class TransformCycleError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Transform {
public:
    using Event = dutils::Event<Transform>;

    std::shared_ptr<Transform> parent() const;
    bool parentChainContains(const Transform& transform) const;
    bool trySetParent(std::shared_ptr<Transform> parent);
    void setParent(std::shared_ptr<Transform> parent);

    const dmath::dquat& ownTransform() const;
    void setOwnTransform(const dmath::dquat& transform);

    const dmath::dquat& fullTransform();

    Event onChange;
    Event onParentChange;

private:
    std::shared_ptr<Transform> parent_;
    std::optional<Event::Subscription> parent_change_;
    dmath::dquat own_transform_;
    std::optional<dmath::dquat> full_transform_;
};

}

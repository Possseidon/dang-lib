#pragma once

#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/global.h"

namespace dang::gl {

class Context;

/// @brief The base for the context classes for the different GL-Object types.
class ObjectContextBase {
public:
    /// @brief Initializes the object context with the given window context.
    explicit ObjectContextBase(Context& context);
    /// @brief Virtual destructor, as the window stores them in an array.
    virtual ~ObjectContextBase() = default;

    /// @brief Returns the associated window.
    Context& context() const;

private:
    Context& context_;
};

/// @brief Can be used as base class, when no multiple binding targets are required for the given object type.
template <ObjectType v_type>
class ObjectContextBindable : public ObjectContextBase {
public:
    using Handle = ObjectHandle<v_type>;
    using Wrapper = ObjectWrapper<v_type>;

    using ObjectContextBase::ObjectContextBase;

    /// @brief Binds the GL-Object with the given handle, unless it is already bound.
    void bind(Handle handle)
    {
        if (bound_object_ == handle)
            return;
        Wrapper::bind(handle);
        bound_object_ = handle;
    }

    /// @brief Resets the bound GL-Object, if the given handle is currently bound.
    void reset(Handle handle)
    {
        if (bound_object_ != handle)
            return;
        Wrapper::bind({});
        bound_object_ = {};
    }

private:
    Handle bound_object_;
};

/// @brief The different context classes, which should be specialized for the various types.
template <ObjectType v_type>
class ObjectContext : public ObjectContextBase {
    using ObjectContextBase::ObjectContextBase;
};

} // namespace dang::gl

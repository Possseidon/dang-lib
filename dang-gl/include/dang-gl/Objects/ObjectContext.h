#pragma once

#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"

namespace dang::gl {

class Context;

/// <summary>The base for the context classes for the different GL-Object types.</summary>
class ObjectContextBase {
public:
    /// <summary>Initializes the object context with the given window context.</summary>
    explicit ObjectContextBase(Context& context);
    /// <summary>Virtual destructor, as the window stores them in an array.</summary>
    virtual ~ObjectContextBase() = default;

    /// <summary>Returns the associated window.</summary>
    Context& context() const;

private:
    Context& context_;
};

/// <summary>Can be used as base class, when no multiple binding targets are required for the given object type.</summary>
template <ObjectType Type>
class ObjectContextBindable : public ObjectContextBase {
public:
    using Handle = ObjectHandle<Type>;
    using Wrapper = ObjectWrapper<Type>;

    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the GL-Object with the given handle, unless it is already bound.</summary>
    void bind(Handle handle)
    {
        if (bound_object_ == handle)
            return;
        Wrapper::bind(handle);
        bound_object_ = handle;
    }

    /// <summary>Resets the bound GL-Object, if the given handle is currently bound.</summary>
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

/// <summary>The different context classes, which should be specialized for the various types.</summary>
template <ObjectType Type>
class ObjectContext : public ObjectContextBase {
    using ObjectContextBase::ObjectContextBase;
};

} // namespace dang::gl

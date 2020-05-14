#pragma once

#include "ObjectType.h"

namespace dang::gl
{

class Window;

/// <summary>The base for the context classes for the different GL-Object types.</summary>
class ObjectContextBase {
public:
    /// <summary>Initializes the object context with the given window context.</summary>
    explicit ObjectContextBase(Window& window);
    /// <summary>Virtual destructor, as the window stores them in an array.</summary>
    virtual ~ObjectContextBase() = default;

    /// <summary>Returns the associated window.</summary>
    Window& window() const;

private:
    Window& window_;
};

/// <summary>Can be used as base class, when no multiple binding targets are required for the given object type.</summary>
template <ObjectType Type>
class ObjectContextBindable : public ObjectContextBase {
public:
    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the GL-Object with the given handle, unless it is already bound.</summary>
    void bind(GLuint handle)
    {
        if (bound_object_ == handle)
            return;
        ObjectWrapper<Type>::bind(handle);
        bound_object_ = handle;
    }

    /// <summary>Resets the bound GL-Object, if the given handle is currently bound.</summary>
    void reset(GLuint handle)
    {
        if (bound_object_ != handle)
            return;
        ObjectWrapper<Type>::bind(0);
        bound_object_ = 0;
    }

private:
    GLuint bound_object_ = 0;
};

/// <summary>The different context classes, which should be specialized for the various types.</summary>
template <ObjectType Type>
class ObjectContext : public ObjectContextBase {
    using ObjectContextBase::ObjectContextBase;
};

}

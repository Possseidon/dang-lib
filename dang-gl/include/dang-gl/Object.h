#pragma once

#include "GLFW.h"
#include "ObjectBase.h"
#include "ObjectContext.h"
#include "ObjectType.h"
#include "Window.h"

namespace dang::gl
{

/// <summary>Serves as a base class for all GL-Objects of the template specified type.</summary>
template <ObjectType Type>
class Object : public ObjectBase {
public:
    /// <summary>Creates a new GL-Object of the correct type and associates the active window to it.</summary>
    Object()
        : ObjectBase(ObjectWrapper<Type>::create(), GLFW::Instance.activeWindow())
    {
    }

    /// <summary>Destroys the GL-Object.</summary>
    ~Object()
    {
        ObjectWrapper<Type>::destroy(this->handle());
    }

    /// <summary>Returns the context for this object type.</summary>
    auto& context() const
    {
        return this->window().objectContext<Type>();
    }

    /// <summary>Sets a label for the object, which is used in by OpenGL generated debug messages.</summary>
    /// <remarks>Passing an empty string effectively resets the name, so that it shows the default instead of an empty string.</remarks>
    void setLabel(std::string label)
    {
        label_ = std::move(label);
        if (label_.empty())
            glObjectLabel(toGLConstant(Type), handle(), 0, nullptr);
        else
            glObjectLabel(toGLConstant(Type), handle(), static_cast<GLsizei>(label_.length()), label_.c_str());
    }

    /// <summary>Returns the label used in OpenGL generated debug messages.</summary>
    /// <remarks>Defaults to an empty string.</remarks>
    const std::string& label() const
    {
        return label_;
    }

private:
    std::string label_;
};

/// <summary>A base class for GL-Objects, which can be bound without a target.</summary>
template <ObjectType Type>
class ObjectBindable : public Object<Type> {
public:
    using Object<Type>::Object;

    /// <summary>Resets the bound object in the context if the object is still bound.</summary>
    ~ObjectBindable()
    {
        this->context().reset(this->handle());
    }

    /// <summary>Binds the object.</summary>
    void bind() const
    {
        this->context().bind(this->handle());
    }
};

}

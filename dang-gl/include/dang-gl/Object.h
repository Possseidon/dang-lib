#pragma once

#include "GLFW.h"
#include "ObjectBase.h"
#include "ObjectBinding.h"
#include "ObjectContext.h"
#include "Window.h"

namespace dang::gl
{

/// <summary>A static interface for GL-Objects.</summary>
struct ObjectInfo {
    // Inherit and implement the following:
    // static GLuint create();
    // static void destroy(GLuint handle);
    // static void bind(GLuint handle);

    // A context per ObjectType (e.g. shared by all texture types):
    using Context = ObjectContext;
    // static constexpr ObjectType ObjectType = ObjectType::TODO;

    // A binding for each binding point (e.g. different per buffer type):
    using Binding = ObjectBinding<>;
    // static constexpr BindingPoint BindingPoint = BindingPoint::TODO;
};

/// <summary>Serves as a base class for all GL-Objects and provides creation, destruction and binding facilities using the supplied TInfo struct.</summary>
template <typename TInfo>
class Object : public ObjectBase {
public:
    /// <summary>Initializes the GL-Object using the create function of the TInfo struct and simply uses the active window for the GL-Context.</summary>
    template <typename... TArgs>
    Object(TArgs&&... args)
        : ObjectBase(TInfo::create(), GLFW::Instance.activeWindow(), std::forward<TArgs>(args)...)
    {
    }

    /// <summary>Move-constructs an object, updating the binding if the moved-from object is currently bound.</summary>
    Object(Object&& other) noexcept
        : ObjectBase(std::move(other))
    {
        binding().move<TInfo>(other, *this);
    }

    /// <summary>Destroys the GL-Object using the destroy function of the TInfo struct, unless the object is in a moved-from state.</summary>
    ~Object()
    {
        if (this->handle() != 0)
            TInfo::destroy(this->handle());
    }

    /// <summary>Returns the context for this object type.</summary>
    auto& context() const
    {
        return this->window().objectContext<TInfo>();
    }

    /// <summary>Returns the binding of the correct context for this type of object.</summary>
    auto& binding() const
    {
        return this->window().objectBinding<TInfo>();
    }

    /// <summary>Binds the GL-Object using the bind function of the TInfo struct through the binding of the correct context.</summary>
    void bind() const
    {
        binding().bind<TInfo>(*this);
    }
};

}

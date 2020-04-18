#pragma once

#include "dang-utils/enum.h"

#include "Binding.h"
#include "GLFW.h"
#include "ObjectBase.h"
#include "Window.h"

namespace dang::gl
{

/// <summary>A static interface for GL-Objects.</summary>
struct ObjectInfo {
    // Inherit and implement the following:
    // static GLuint create();
    // static void destroy(GLuint handle);
    // static void bind(GLuint handle);

    // static constexpr BindingPoint BindingPoint = BindingPoint::TODO;

    // Specify a custom binding class if necessary:
    // Note: Must be default-constructible
    using Binding = Binding;
};

/// <summary>Serves as a base class for all GL-Objects and provides creation, destruction and binding facilities using the supplied TInfo struct.</summary>
template <class TInfo>
class Object : public ObjectBase {
public:
    /// <summary>Initializes the GL-Object using the create function of the TInfo struct and simply uses the active window for the GL-Context.</summary>
    Object()
        : ObjectBase(TInfo::create(), GLFW::Instance.activeWindow())
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
        if (handle() != 0)
            TInfo::destroy(handle());
    }

    /// <summary>Returns the binding of the correct context for this type of object.</summary>
    typename TInfo::Binding& binding() const
    {
        return window().binding<TInfo>();
    }

    /// <summary>Binds the GL-Object using the bind function of the TInfo struct through the binding of the correct context.</summary>
    void bind() const
    {
        binding().bind<TInfo>(*this);
    }
};

}

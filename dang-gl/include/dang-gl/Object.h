#pragma once

#include "dang-utils/enum.h"

#include "Binding.h"
#include "ObjectBase.h"
#include "Window.h"
#include "GLFW.h"

namespace dang::gl
{

class GLFW;

struct ObjectInfo {
    // Inherite and implement the following:
    // static GLuint create();
    // static void destroy(GLuint handle);
    // static void bind(GLuint handle);

    // static constexpr ObjectType Type = ObjectType::TODO;

    // Specify a custom binding class if necessary:
    // Note: Must be default-constructible
    using Binding = Binding;
};

template <class TInfo>
class Object : public ObjectBase {
public:
    Object();
    ~Object();

    typename TInfo::Binding& binding() const;
    void bind();
};

template<class TInfo>
inline Object<TInfo>::Object()
    : ObjectBase(TInfo::create(), GLFW::Instance.activeWindow())
{
}

template<class TInfo>
inline Object<TInfo>::~Object()
{
    if (handle() != 0)
        TInfo::destroy(handle());
}

template<class TInfo>
inline typename TInfo::Binding& Object<TInfo>::binding() const
{
    return window().binding<TInfo>();
}

template<class TInfo>
inline void Object<TInfo>::bind()
{
    binding().bind<TInfo>(this);
}

}

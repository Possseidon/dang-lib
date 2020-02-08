#pragma once

#include <string>

#include "dang-utils/enum.h"

namespace dang::gl
{

enum class ObjectType {
    Texture,
    VertexArray,
    Buffer,
    Shader,
    Program,
    Query,
    ProgramPipeline,
    Sampler,
    DisplayList,
    Framebuffer,
    Renderbuffer,
    TransformFeedback,
    COUNT
};

constexpr dutils::EnumArray<ObjectType, GLenum> ObjectTypesGL
{
   GL_TEXTURE,
   GL_VERTEX_ARRAY,
   GL_BUFFER,
   GL_SHADER,
   GL_PROGRAM,
   GL_QUERY,
   GL_PROGRAM_PIPELINE,
   GL_SAMPLER,
   GL_DISPLAY_LIST,
   GL_FRAMEBUFFER,
   GL_RENDERBUFFER,
   GL_TRANSFORM_FEEDBACK
};

class ObjectBase;

class Binding {
public:
    template <class TInfo>
    void bind(ObjectBase* object);

private:
    ObjectBase* bound_object_ = nullptr;
};

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

class GLFW {
public:
    static GLFW Instance;

    void setContext(GLFWwindow* window);

    template <class TInfo>
    Binding& binding();

private:
    GLFW();
    ~GLFW();

    static std::string formatError(int error_code, const char* description);
    static void errorCallback(int error_code, const char* description);

    bool glad_initialized_ = false;
    GLFWwindow* active_window_ = nullptr;
    dutils::EnumArray<ObjectType, std::unique_ptr<Binding>> bindings_;
};

class ObjectBase {
public:
    GLuint handle();

protected:
    Binding& binding_;

    ObjectBase(GLuint handle, Binding& binding);

private:
    GLuint handle_;
};

template <class TInfo>
class Object : public ObjectBase {
public:
    Object();
    ~Object();

    void bind();
};

template<class TInfo>
inline Object<TInfo>::Object()
    : ObjectBase(TInfo::create(), GLFW::Instance.binding<TInfo>())
{
}

template<class TInfo>
inline Object<TInfo>::~Object()
{
    TInfo::destroy(handle());
}

template<class TInfo>
inline void Object<TInfo>::bind()
{
    binding_.bind<TInfo>(this);
}

template<class TInfo>
inline Binding& GLFW::binding()
{
    if (const auto& binding = bindings_[TInfo::Type])
        return *binding;
    return *(bindings_[TInfo::Type] = std::make_unique<TInfo::Binding>());
}

template<class TInfo>
inline void Binding::bind(ObjectBase* object)
{
    if (bound_object_ == object)
        return;
    TInfo::bind(object->handle());
    bound_object_ = object;
}

}

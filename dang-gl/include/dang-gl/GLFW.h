#pragma once

#include "dang-math/vector.h"

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

class Window;

class GLFW {
public:
    static GLFW Instance;

    bool hasActiveWindow();
    Window& activeWindow();
    void setActiveWindow(Window* window);

private:
    GLFW();
    ~GLFW();

    static std::string formatError(int error_code, const char* description);
    static void errorCallback(int error_code, const char* description);

    bool glad_initialized_ = false;
    Window* active_window_ = nullptr;
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

class ObjectBase {
public:
    GLuint handle();
    Window& window();

protected:
    ObjectBase(GLuint handle, Window& window);

private:
    GLuint handle_;
    Window& window_;
};

class WindowInfo {
public:
    WindowInfo();

    dmath::ivec2 size() const;
    void setSize(dmath::ivec2 size);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    std::string title() const;
    void setTitle(std::string title);

    GLFWwindow* createWindow() const;

private:
    dmath::ivec2 size_;
    std::string title_;
};

class Window {
public:
    Window(GLFWwindow* handle);
    Window(const WindowInfo& info = WindowInfo());
    ~Window();

    GLFWwindow* handle();

    template <class TInfo>
    Binding& binding();

    bool shouldClose();

    void update();
    void render();

    void step();
    void run();

private:
    GLFWwindow* handle_;
    dutils::EnumArray<ObjectType, std::unique_ptr<Binding>> bindings_;
};

template <class TInfo>
class Object : public ObjectBase {
public:
    Object();
    ~Object();

    typename TInfo::Binding& binding();
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
    TInfo::destroy(handle());
}

template<class TInfo>
inline typename TInfo::Binding& Object<TInfo>::binding()
{
    return window().binding<TInfo>();
}

template<class TInfo>
inline void Object<TInfo>::bind()
{
    binding().bind<TInfo>(this);
}

template<class TInfo>
inline void Binding::bind(ObjectBase* object)
{
    if (bound_object_ == object)
        return;
    TInfo::bind(object->handle());
    bound_object_ = object;
}

template<class TInfo>
inline Binding& Window::binding()
{
    if (const auto& binding = bindings_[TInfo::Type])
        return *binding;
    return *(bindings_[TInfo::Type] = std::make_unique<TInfo::Binding>());
}

}

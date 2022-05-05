#pragma once

#include "dang-gl/Context/State.h"
#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"
#include "dang-utils/enum.h"
#include "dang-utils/event.h"

namespace dang::gl {

class Context;

enum class DebugSource {
    API = GL_DEBUG_SOURCE_API,
    WindowSystem = GL_DEBUG_SOURCE_WINDOW_SYSTEM,
    ShaderCompiler = GL_DEBUG_SOURCE_SHADER_COMPILER,
    ThirdParty = GL_DEBUG_SOURCE_THIRD_PARTY,
    Application = GL_DEBUG_SOURCE_APPLICATION,
    Other = GL_DEBUG_SOURCE_OTHER
};

enum class DebugType {
    Error = GL_DEBUG_TYPE_ERROR,
    DeprecatedBehaviour = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
    UndefinedBehaviour = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
    Portability = GL_DEBUG_TYPE_PORTABILITY,
    Performance = GL_DEBUG_TYPE_PERFORMANCE,
    Other = GL_DEBUG_TYPE_OTHER,
    Marker = GL_DEBUG_TYPE_MARKER,
    PushGroup = GL_DEBUG_TYPE_PUSH_GROUP,
    PopGroup = GL_DEBUG_TYPE_POP_GROUP
};

enum class DebugSeverity {
    Notification = GL_DEBUG_SEVERITY_NOTIFICATION,
    Low = GL_DEBUG_SEVERITY_LOW,
    Medium = GL_DEBUG_SEVERITY_MEDIUM,
    High = GL_DEBUG_SEVERITY_HIGH
};

struct DebugMessageInfo {
    Context& context;
    DebugSource source;
    DebugType type;
    GLuint id;
    DebugSeverity severity;
    std::string message;
};

using DebugMessageEvent = dutils::Event<DebugMessageInfo>;

class Context {
private:
    static Context* current_;

public:
    using Event = dutils::Event<Context>;

    Context(svec2 size);
    ~Context();

    friend void setContext(Context* context);
    friend Context& context();

    State& state() { return state_; }

    const State& state() const { return state_; }

    State* operator->() { return &state_; }

    const State* operator->() const { return &state_; }

    template <ObjectType v_type>
    auto& contextFor()
    {
        return static_cast<ObjectContext<v_type>&>(*object_contexts_[v_type]);
    }

    template <ObjectType v_type>
    auto& contextFor() const
    {
        return static_cast<const ObjectContext<v_type>&>(*object_contexts_[v_type]);
    }

    svec2 size() const { return size_; }

    float aspect() const { return static_cast<float>(size_.x()) / size_.y(); }

    void resize(svec2 size)
    {
        if (size_ == size)
            return;
        size_ = size;
        onResize(*this);
    }

    Event onResize;

    /// @brief Triggered, if OpenGL debug output is enabled in the state.
    /// @remark Enabling synchronous debug output is very useful for debugging.
    DebugMessageEvent onGLDebugMessage;

private:
    /// @brief Initializes the contexts for the different GL-Object types.
    template <ObjectType... v_types>
    void createContexts(dutils::EnumSequence<ObjectType, v_types...>);

    static void APIENTRY debugMessageCallback(GLenum source,
                                              GLenum type,
                                              GLuint id,
                                              GLenum severity,
                                              GLsizei length,
                                              const GLchar* message,
                                              const void* user_param);

    State state_;
    dutils::EnumArray<ObjectType, std::unique_ptr<ObjectContextBase>> object_contexts_;
    svec2 size_;
};

void setContext(Context* context);
Context& context();

} // namespace dang::gl

#include "dang-gl/Context/Context.h"

// Context specializations:
#include "dang-gl/Objects/BufferContext.h"
#include "dang-gl/Objects/FramebufferContext.h"
#include "dang-gl/Objects/ProgramContext.h"
#include "dang-gl/Objects/RenderbufferContext.h"
#include "dang-gl/Objects/TextureContext.h"
#include "dang-gl/Objects/VertexArrayContext.h"

namespace dang::gl {

Context* Context::current_ = nullptr;

Context::Context(svec2 size)
    : state_(size)
    , size_(size)
{
    createContexts(dutils::makeEnumSequence<ObjectType>());
    glDebugMessageCallback(debugMessageCallback, this);
}

Context::~Context() { glDebugMessageCallback(nullptr, nullptr); }

template <ObjectType... v_types>
void Context::createContexts(dutils::EnumSequence<ObjectType, v_types...>)
{
    ((object_contexts_[v_types] = std::make_unique<ObjectContext<v_types>>(*this)), ...);
}

void Context::debugMessageCallback(GLenum source,
                                   GLenum type,
                                   GLuint id,
                                   GLenum severity,
                                   GLsizei length,
                                   const GLchar* message,
                                   const void* user_param)
{
    Context& context = *static_cast<Context*>(const_cast<void*>(user_param));
    context.on_gl_debug_message({context,
                                 static_cast<DebugSource>(source),
                                 static_cast<DebugType>(type),
                                 id,
                                 static_cast<DebugSeverity>(severity),
                                 std::string(message, message + length)});
}

void setContext(Context* context) { Context::current_ = context; }

Context& context()
{
    assert(Context::current_);
    return *Context::current_;
}

} // namespace dang::gl

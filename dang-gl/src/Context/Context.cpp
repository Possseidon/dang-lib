#include "Context/Context.h"

// Context specializations:
#include "Objects/BufferContext.h"
#include "Objects/FramebufferContext.h"
#include "Objects/ProgramContext.h"
#include "Objects/RenderbufferContext.h"
#include "Objects/TextureContext.h"
#include "Objects/VertexArrayContext.h"

namespace dang::gl {

Context::Context(svec2 size)
    : state_(size)
    , size_(size)
{
    createContexts(dutils::makeEnumSequence<ObjectType>());
    glDebugMessageCallback(debugMessageCallback, this);
}

template <ObjectType... Types>
void Context::createContexts(dutils::EnumSequence<ObjectType, Types...>)
{
    ((object_contexts_[Types] = std::make_unique<ObjectContext<Types>>(*this)), ...);
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
    context.onGLDebugMessage({context,
                              static_cast<DebugSource>(source),
                              static_cast<DebugType>(type),
                              id,
                              static_cast<DebugSeverity>(severity),
                              std::string(message, message + length)});
}

} // namespace dang::gl

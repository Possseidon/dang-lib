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
}

template <ObjectType... Types>
void Context::createContexts(dutils::EnumSequence<ObjectType, Types...>)
{
    ((object_contexts_[Types] = std::make_unique<ObjectContext<Types>>(*this)), ...);
}

} // namespace dang::gl

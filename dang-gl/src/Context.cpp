#include "pch.h"
#include "Context.h"

// Context specializations:
#include "BufferContext.h"
#include "FramebufferContext.h"
#include "ProgramContext.h"
#include "RenderbufferContext.h"
#include "TextureContext.h"
#include "VertexArrayContext.h"

namespace dang::gl
{

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

}

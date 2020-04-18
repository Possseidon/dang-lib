#include "pch.h"
#include "Renderable.h"

namespace dang::gl
{

bool Renderable::isVisible() const
{
    return true;
}

SharedTransform Renderable::transform() const
{
    return nullptr;
}

}

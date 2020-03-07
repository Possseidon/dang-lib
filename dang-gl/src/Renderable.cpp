#include "pch.h"
#include "Renderable.h"

namespace dang::gl
{

bool Renderable::isVisible() const
{
    return true;
}

std::shared_ptr<Transform> Renderable::transform() const
{
    return nullptr;
}

}

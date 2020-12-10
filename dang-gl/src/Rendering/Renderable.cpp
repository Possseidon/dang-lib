#include "pch.h"

#include "Rendering/Renderable.h"

namespace dang::gl {

bool Renderable::isVisible() const { return true; }

SharedTransform Renderable::transform() const { return nullptr; }

} // namespace dang::gl

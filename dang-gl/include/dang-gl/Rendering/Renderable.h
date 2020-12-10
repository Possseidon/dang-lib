#pragma once

#include "dang-gl/Math/Transform.h"
#include "dang-gl/global.h"

namespace dang::gl {

class Program;

class Renderable;

using UniqueRenderable = std::unique_ptr<Renderable>;
using SharedRenderable = std::shared_ptr<Renderable>;
using WeakRenderable = std::weak_ptr<Renderable>;

/// @brief An abstract base class for any renderable object with an optional transformation.
class Renderable {
public:
    virtual ~Renderable() = default;

    /// @brief Whether this object should be rendered or skipped.
    virtual bool isVisible() const;
    /// @brief An optional transformation, describing where to render the object.
    virtual SharedTransform transform() const;
    /// @brief Returns the GL-Program, which is used in the draw method, so that uniforms can be upated.
    virtual Program& program() const = 0;
    /// @brief Draws the object.
    virtual void draw() const = 0;
};

} // namespace dang::gl

#pragma once

#include "Program.h"
#include "Transform.h"

namespace dang::gl
{

class Renderable;

using UniqueRenderable = std::unique_ptr<Renderable>;
using SharedRenderable = std::shared_ptr<Renderable>;
using WeakRenderable = std::weak_ptr<Renderable>;

/// <summary>An abstract base class for any renderable object with an optional transformation.</summary>
class Renderable {
public:
    virtual ~Renderable() = default;

    /// <summary>Whether this object should be rendered or skipped.</summary>
    virtual bool isVisible() const;
    /// <summary>An optional transformation, describing where to render the object.</summary>
    virtual SharedTransform transform() const;
    /// <summary>Returns the GL-Program, which is used in the draw method, so that uniforms can be upated.</summary>
    virtual Program& program() const = 0;
    /// <summary>Draws the object.</summary>
    virtual void draw() const = 0;
};

}

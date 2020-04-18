#pragma once

#include "dang-gl/Transform.h"

namespace dang::gl
{

/// <summary>An abstract base class for any renderable object with an optional transformation.</summary>
class Renderable {
public:
    virtual ~Renderable() = default;

    /// <summary>Wether this object should be rendered or skipped.</summary>
    virtual bool isVisible() const;
    /// <summary>An optional transformation, describing where to render the object.</summary>
    virtual SharedTransform transform() const;
    /// <summary>Draws the object.</summary>
    virtual void draw() const = 0;
};

}

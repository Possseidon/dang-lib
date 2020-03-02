#pragma once

namespace dang::gl
{

class Transform;

class IRenderable {
public:
    virtual bool isVisible() const;
    virtual const Transform& transform() const;
    virtual void draw() const = 0;
};

}

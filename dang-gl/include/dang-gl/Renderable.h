#pragma once

namespace dang::gl
{

class Transform;

class Renderable {
public:
    virtual ~Renderable() = default;
    virtual bool isVisible() const;
    virtual const std::shared_ptr<Transform> transform() const;
    virtual void draw() const = 0;
};

}

#pragma once

#include "dang-math/bounds.h"
#include "dang-math/matrix.h"

#include "Transform.h"

namespace dang::gl
{

class IRenderable;

class ViewMatrixProvider {
public:
    const dmath::mat4& viewMatrix();

protected:
    void resetViewMatrix();
    virtual dmath::mat4 calculateViewMatrix() = 0;

private:
    std::optional<dmath::mat4> view_matrix_;
};

class PerspectiveViewMatrix : public ViewMatrixProvider {
public:
    float aspect() const;
    void setAspect(float aspect);

    float fieldOfView() const;
    void setFieldOfView(float fov) const;

    dmath::bounds1 clip() const;
    void setClip(dmath::bounds1 clip);
    float nearClip() const;
    void setNearClip(float near_clip);
    float farClip() const;
    void setFarClip(float far_clip);

protected:
    dmath::mat4 calculateViewMatrix() override;

private:
    float aspect_ = 1.0f;
    float field_of_view_ = 90.0f;
    dmath::bounds1 clip_ = { 0.1f, 100.0f };
};

class OrthoViewMatrix : public ViewMatrixProvider {
public:

protected:
    dmath::mat4 calculateViewMatrix() override;

};

class Camera {
public:
    Camera(std::unique_ptr<ViewMatrixProvider> view_matrix_provider);

private:
    std::unique_ptr<ViewMatrixProvider> view_matrix_provider_;
    std::shared_ptr<Transform> transform_;
    std::vector<std::weak_ptr<IRenderable>> renderables_;
};

}

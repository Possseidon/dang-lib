#include "pch.h"
#include "Camera.h"

#include "Renderable.h"

namespace dang::gl
{

const dgl::mat4& ProjectionProvider::matrix()
{
    if (!matrix_)
        matrix_ = calculateMatrix();
    return *matrix_;
}

void ProjectionProvider::resetMatrix()
{
    matrix_ = std::nullopt;
}

PerspectiveProjection::PerspectiveProjection(float aspect, float field_of_view, dmath::bounds1 clip)
    : aspect_(aspect)
    , field_of_view_(field_of_view)
    , clip_(clip)
{
}

float PerspectiveProjection::aspect() const
{
    return aspect_;
}

void PerspectiveProjection::setAspect(float aspect)
{
    if (aspect_ == aspect)
        return;
    aspect_ = aspect;
    resetMatrix();
}

float PerspectiveProjection::fieldOfView() const
{
    return field_of_view_;
}

void PerspectiveProjection::setFieldOfView(float field_of_view)
{
    if (field_of_view_ == field_of_view)
        return;
    field_of_view_ = field_of_view;
    resetMatrix();
}

dmath::bounds1 PerspectiveProjection::clip() const
{
    return clip_;
}

void PerspectiveProjection::setClip(dmath::bounds1 clip)
{
    if (clip_ == clip)
        return;
    clip_ = clip;
    resetMatrix();
}

float PerspectiveProjection::nearClip() const
{
    return clip_.low;
}

void PerspectiveProjection::setNearClip(float near_clip)
{
    if (clip_.low.x() == near_clip)
        return;
    clip_.low = near_clip;
    resetMatrix();
}

float PerspectiveProjection::farClip() const
{
    return clip_.high;
}

void PerspectiveProjection::setFarClip(float far_clip)
{
    if (clip_.high.x() == far_clip)
        return;
    clip_.high = far_clip;
    resetMatrix();
}

Camera::Camera(std::unique_ptr<ProjectionProvider> view_matrix_provider)
    : projection_provider_(std::move(view_matrix_provider))
    , transform_(std::make_shared<Transform>())
{
}

void Camera::render()
{
    auto new_end = std::remove_if(
        renderables_.begin(), renderables_.end(),
        std::mem_fn(&std::weak_ptr<Renderable>::expired));
    renderables_.erase(new_end, renderables_.end());

    if (projection_uniform_)
        projection_uniform_->force(projection_provider_->matrix());

    if (auto& uniform = transform_uniforms_[CameraTransformType::View])
        uniform->force(transform_->fullTransform().toMatrix2x4());

    for (const auto& weak_renderable : renderables_) {
        auto renderable = weak_renderable.lock();
        if (!renderable || !renderable->isVisible())
            continue;

        if (auto& uniform = transform_uniforms_[CameraTransformType::Model]) {
            if (auto transform = renderable->transform())
                uniform->force(transform->fullTransform().toMatrix2x4());
            else
                uniform->force(dquat().toMatrix2x4());
        }

        if (auto& uniform = transform_uniforms_[CameraTransformType::ModelView]) {
            if (auto transform = renderable->transform())
                uniform->force((transform_->fullTransform() * transform->fullTransform()).toMatrix2x4());
            else
                uniform->force(transform_->fullTransform().toMatrix2x4());
        }

        renderable->draw();
    }
}

}

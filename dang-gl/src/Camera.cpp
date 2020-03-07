#include "pch.h"
#include "Camera.h"

#include "Renderable.h"

namespace dang::gl
{

ProjectionProvider::ProjectionProvider(float aspect)
    : aspect_(aspect)
{
}

ProjectionProvider::ProjectionProvider(Window& window)
    : aspect_(window.aspect())
    , window_resize_({ window.onFramebufferResize, [&] { setAspect(window.aspect()); } })
{
}

float ProjectionProvider::aspect() const
{
    return aspect_;
}

void ProjectionProvider::setAspect(float aspect)
{
    if (aspect_ == aspect)
        return;
    aspect_ = aspect;
    invalidateMatrix();
}

const dgl::mat4& ProjectionProvider::matrix()
{
    if (!matrix_)
        matrix_ = calculateMatrix();
    return *matrix_;
}

void ProjectionProvider::invalidateMatrix()
{
    matrix_ = std::nullopt;
}

PerspectiveProjection::PerspectiveProjection(float aspect, float field_of_view, dmath::bounds1 clip)
    : ProjectionProvider(aspect)
    , field_of_view_(field_of_view)
    , clip_(clip)
{
}

PerspectiveProjection::PerspectiveProjection(Window& window, float field_of_view, dmath::bounds1 clip)
    : ProjectionProvider(window)
    , field_of_view_(field_of_view)
    , clip_(clip)
{
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
    invalidateMatrix();
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
    invalidateMatrix();
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
    invalidateMatrix();
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
    invalidateMatrix();
}

dgl::mat4 PerspectiveProjection::calculateMatrix()
{
    dgl::mat4 result;
    float a = dmath::degToRad(field_of_view_) / 2;
    float f = std::cos(a) / std::sin(a);
    result(0, 0) = f / aspect();
    result(1, 1) = f;
    result(2, 2) = (nearClip() + farClip()) / (nearClip() - farClip());
    result(2, 3) = -1;
    result(3, 2) = (2 * nearClip() * farClip()) / (nearClip() - farClip());
    return result;
}

OrthoProjection::OrthoProjection(float aspect, dgl::bounds3 clip)
    : ProjectionProvider(aspect)
    , clip_(clip)
{
}

OrthoProjection::OrthoProjection(Window& window, dgl::bounds3 clip)
    : ProjectionProvider(window)
    , clip_(clip)
{
}

const dmath::bounds3& OrthoProjection::clip() const
{
    return clip_;
}

void OrthoProjection::setClip(const dmath::bounds3& clip)
{
    if (clip_ == clip)
        return;
    clip_ = clip;
    invalidateMatrix();
}

dgl::mat4 OrthoProjection::calculateMatrix()
{
    return dgl::mat4();
}

Camera::Camera(std::shared_ptr<ProjectionProvider> view_matrix_provider)
    : projection_provider_(std::move(view_matrix_provider))
    , transform_(std::make_shared<Transform>())
{
}

Camera Camera::perspective(float aspect, float field_of_view, dmath::bounds1 clip)
{
    return Camera(std::make_shared<PerspectiveProjection>(aspect, field_of_view, clip));
}

Camera Camera::perspective(Window& window, float field_of_view, dmath::bounds1 clip)
{
    return Camera(std::make_shared<PerspectiveProjection>(window, field_of_view, clip));
}

Camera Camera::ortho(float aspect, dgl::bounds3 clip)
{
    return Camera(std::make_shared<OrthoProjection>(aspect, clip));
}

Camera Camera::ortho(Window& window, dgl::bounds3 clip)
{
    return Camera(std::make_shared<OrthoProjection>(window, clip));
}

const std::shared_ptr<ProjectionProvider>& Camera::projectionProvider() const
{
    return projection_provider_;
}

const std::shared_ptr<Transform>& Camera::transform() const
{
    return transform_;
}

void Camera::setProjectionUniform(ShaderUniform<dgl::mat4>& uniform)
{
    projection_uniform_ = &uniform;
}

void Camera::resetProjectionUniform()
{
    projection_uniform_ = nullptr;
}

void Camera::setTransformUniform(CameraTransformType type, ShaderUniform<dgl::mat2x4>& uniform)
{
    transform_uniforms_[type] = &uniform;
}

void Camera::resetTransformUniform(CameraTransformType type)
{
    transform_uniforms_[type] = nullptr;
}

void Camera::addRenderable(std::weak_ptr<Renderable> renderable)
{
    renderables_.push_back(std::move(renderable));
}

void Camera::removeRenderable(const std::weak_ptr<Renderable>& renderable)
{
    auto pos = std::find_if(
        renderables_.begin(), renderables_.end(),
        [&](std::weak_ptr<Renderable>& current)
        {
            return !current.owner_before(renderable) && !renderable.owner_before(current);
        });

    if (pos != renderables_.end())
        renderables_.erase(pos);
}

void Camera::clearRenderables()
{
    renderables_.clear();
}

void Camera::render()
{
    removeExpiredRenderables();

    if (projection_uniform_)
        projection_uniform_->force(projection_provider_->matrix());

    const auto& view_transform = transform_->fullTransform().inverse();

    if (auto& uniform = transform_uniforms_[CameraTransformType::View])
        uniform->force(view_transform.toMatrix2x4());

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
                uniform->force((transform->fullTransform() * view_transform).toMatrix2x4());
            else
                uniform->force(view_transform.toMatrix2x4());
        }

        renderable->draw();
    }
}

void Camera::removeExpiredRenderables()
{
    renderables_.erase(
        std::remove_if(
            renderables_.begin(), renderables_.end(),
            std::mem_fn(&std::weak_ptr<Renderable>::expired)),
        renderables_.end());
}

}

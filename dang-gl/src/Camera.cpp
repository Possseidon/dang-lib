#include "pch.h"
#include "Camera.h"

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

const mat4& ProjectionProvider::matrix()
{
    if (!matrix_)
        matrix_ = calculateMatrix();
    return *matrix_;
}

void ProjectionProvider::invalidateMatrix()
{
    matrix_ = std::nullopt;
}

PerspectiveProjection::PerspectiveProjection(float aspect, float field_of_view, bounds1 clip)
    : ProjectionProvider(aspect)
    , field_of_view_(field_of_view)
    , clip_(clip)
{
}

PerspectiveProjection::PerspectiveProjection(Window& window, float field_of_view, bounds1 clip)
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

bounds1 PerspectiveProjection::clip() const
{
    return clip_;
}

void PerspectiveProjection::setClip(bounds1 clip)
{
    if (clip_ == clip)
        return;
    clip_ = clip;
    invalidateMatrix();
}

float PerspectiveProjection::nearClip() const
{
    return clip_.lowValue();
}

void PerspectiveProjection::setNearClip(float near_clip)
{
    if (clip_.lowValue() == near_clip)
        return;
    clip_.low = near_clip;
    invalidateMatrix();
}

float PerspectiveProjection::farClip() const
{
    return clip_.highValue();
}

void PerspectiveProjection::setFarClip(float far_clip)
{
    if (clip_.highValue() == far_clip)
        return;
    clip_.high = far_clip;
    invalidateMatrix();
}

mat4 PerspectiveProjection::calculateMatrix()
{
    mat4 result;
    float a = dmath::degToRad(field_of_view_) / 2;
    float f = std::cos(a) / std::sin(a);
    result(0, 0) = f / aspect();
    result(1, 1) = f;
    result(2, 2) = (nearClip() + farClip()) / (nearClip() - farClip());
    result(2, 3) = -1;
    result(3, 2) = (2 * nearClip() * farClip()) / (nearClip() - farClip());
    return result;
}

OrthoProjection::OrthoProjection(float aspect, bounds3 clip)
    : ProjectionProvider(aspect)
    , clip_(clip)
{
}

OrthoProjection::OrthoProjection(Window& window, bounds3 clip)
    : ProjectionProvider(window)
    , clip_(clip)
{
}

const bounds3& OrthoProjection::clip() const
{
    return clip_;
}

void OrthoProjection::setClip(const bounds3& clip)
{
    if (clip_ == clip)
        return;
    clip_ = clip;
    invalidateMatrix();
}

mat4 OrthoProjection::calculateMatrix()
{
    auto size = clip_.size();
    auto offset = -(clip_.high + clip_.low) / size;
    mat4 result;
    result(0, 0) = 2.0f / size.x() / aspect();
    result(1, 1) = 2.0f / size.y();
    result(2, 2) = -2.0f / size.z();
    result(3, 0) = offset.x();
    result(3, 1) = offset.y();
    result(3, 2) = offset.z();
    result(3, 3) = 1.0f;
    return result;
}

CameraUniforms::CameraUniforms(Program& program, const CameraUniformNames& names)
    : program_(program)
    , projection_uniform_(program.uniform<mat4>(names.ProjectionMatrix))
    , transform_uniforms_{
        program.uniform<mat2x4>(names.ModelTransform),
        program.uniform<mat2x4>(names.ViewTransform),
        program.uniform<mat2x4>(names.ModelViewTransform) }
{
}

Program& CameraUniforms::program() const
{
    return program_;
}

void CameraUniforms::updateProjectionMatrix(const mat4& projection_matrix) const
{
    ShaderUniform<mat4>& uniform = projection_uniform_;
    if (uniform.exists())
        uniform.force(projection_matrix);
}

void CameraUniforms::updateTransform(CameraTransformType type, const dquat& transform) const
{
    ShaderUniform<mat2x4>& uniform = transform_uniforms_[type];
    if (uniform.exists())
        uniform.force(transform.toMatrix2x4());
}

Camera::Camera(SharedProjectionProvider view_matrix_provider)
    : projection_provider_(std::move(view_matrix_provider))
{
}

Camera Camera::perspective(float aspect, float field_of_view, bounds1 clip)
{
    return Camera(std::make_shared<PerspectiveProjection>(aspect, field_of_view, clip));
}

Camera Camera::perspective(Window& window, float field_of_view, bounds1 clip)
{
    return Camera(std::make_shared<PerspectiveProjection>(window, field_of_view, clip));
}

Camera Camera::ortho(float aspect, bounds3 clip)
{
    return Camera(std::make_shared<OrthoProjection>(aspect, clip));
}

Camera Camera::ortho(Window& window, bounds3 clip)
{
    return Camera(std::make_shared<OrthoProjection>(window, clip));
}

const SharedProjectionProvider& Camera::projectionProvider() const
{
    return projection_provider_;
}

const SharedTransform& Camera::transform() const
{
    return transform_;
}

void Camera::setCustomUniforms(Program& program, const CameraUniformNames& names)
{
    auto program_matches = [&](const CameraUniforms& uniforms) {
        return &uniforms.program() != &program;
    };

    auto uniforms = std::find_if(uniforms_.begin(), uniforms_.end(), program_matches);
    if (uniforms == uniforms_.end())
        uniforms_.emplace_back(program, names);
    else
        *uniforms = CameraUniforms(program, names);
}

void Camera::addRenderable(SharedRenderable renderable)
{
    renderables_.push_back(std::move(renderable));
}

void Camera::removeRenderable(const SharedRenderable& renderable)
{
    auto pos = std::find(renderables_.begin(), renderables_.end(), renderable);
    if (pos != renderables_.end())
        renderables_.erase(pos);
}

void Camera::clearRenderables()
{
    renderables_.clear();
}

void Camera::render()
{
    auto force_all = [](const auto& uniforms, const auto& value) {
        for (auto& uniform : uniforms)
            uniform->force(value);
    };

    const auto& view_transform = transform_->fullTransform().inverseFast();

    for (const auto& uniforms : uniforms_) {
        uniforms.updateProjectionMatrix(projection_provider_->matrix());
        uniforms.updateTransform(CameraTransformType::View, view_transform);
    }

    for (const auto& renderable : renderables_) {
        if (!renderable->isVisible())
            continue;

        auto program_matches = [&](const CameraUniforms& uniforms) {
            return &uniforms.program() == &renderable->program();
        };

        auto uniforms = std::find_if(uniforms_.begin(), uniforms_.end(), program_matches);
        if (uniforms == uniforms_.end()) {
            uniforms_.emplace_back(renderable->program());
            uniforms = std::prev(uniforms_.end());
        }

        if (const auto& model_transform = renderable->transform()) {
            uniforms->updateTransform(CameraTransformType::Model, model_transform->fullTransform());
            uniforms->updateTransform(CameraTransformType::ModelView, view_transform * model_transform->fullTransform());
        }
        else {
            uniforms->updateTransform(CameraTransformType::Model, dquat());
            uniforms->updateTransform(CameraTransformType::ModelView, view_transform);
        }

        renderable->draw();
    }
}

}

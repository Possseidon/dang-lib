#pragma once

#include "dang-gl/Context/Context.h"
#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/Math/Transform.h"
#include "dang-gl/Objects/Program.h"
#include "dang-gl/global.h"

#include "dang-utils/enum.h"

namespace dang::gl {

/// <summary>The different cached transform (quaternion, not matrix) types of a camera, namely model, view and a combined model-view.</summary>
/// <remarks>The projection uses a matrix and is therefore handled separately.</remarks>
enum class CameraTransformType { Model, View, ModelView, COUNT };

} // namespace dang::gl

namespace dang::utils {

template <>
struct EnumCount<dang::gl::CameraTransformType> : DefaultEnumCount<dang::gl::CameraTransformType> {};

} // namespace dang::utils

namespace dang::gl {

class ProjectionProvider;

using UniqueProjectionProvider = std::unique_ptr<ProjectionProvider>;
using SharedProjectionProvider = std::shared_ptr<ProjectionProvider>;
using WeakProjectionProvider = std::weak_ptr<ProjectionProvider>;

/// <summary>A base class, providing a projection matrix for a camera.</summary>
class ProjectionProvider {
public:
    /// <summary>Initializes the projection provider with the given aspect.</summary>
    ProjectionProvider(float aspect);
    /// <summary>Automatically updates the aspect to match the given context.</summary>
    ProjectionProvider(Context& context);
    virtual ~ProjectionProvider();

    /// <summary>Returns the current aspect ratio (width/height).</summary>
    float aspect() const;
    /// <summary>Sets the aspect ratio (width/height) to the given value.</summary>
    /// <remarks>This value will only be overwritten on the next context resize, if a context was provided.</remarks>
    void setAspect(float aspect);

    /// <summary>Returns the projection matrix, which is lazily evaulated.</summary>
    const mat4& matrix();

protected:
    /// <summary>Can be used by sub-classes do invalidate the current matrix.</summary>
    void invalidateMatrix();
    /// <summary>Calculates the projection matrix.</summary>
    virtual mat4 calculateMatrix() = 0;

private:
    float aspect_;
    Context::Event::Subscription context_resize_;
    std::optional<mat4> matrix_;
};

/// <summary>A perspective projection provider with field of view and near/far clipping.</summary>
class PerspectiveProjection : public ProjectionProvider {
public:
    static constexpr float DefaultFieldOfView = 90.0f;
    static constexpr bounds1 DefaultClip = {0.1f, 100.0f};

    /// <summary>Initializes the perspective projection with the given field of view and near/far clip.</summary>
    PerspectiveProjection(float aspect, float field_of_view = DefaultFieldOfView, bounds1 clip = DefaultClip);
    /// <summary>Initializes the perspective projection with the given field of view and near/far clip.</summary>
    PerspectiveProjection(Context& context, float field_of_view = DefaultFieldOfView, bounds1 clip = DefaultClip);

    /// <summary>Returns the current field of view.</summary>
    float fieldOfView() const;
    /// <summary>Sets the field of view.</summary>
    void setFieldOfView(float field_of_view);

    /// <summary>Returns the near and far clip as low and high as bounds.</summary>
    bounds1 clip() const;
    /// <summary>Sets the near and far clip as low and high of the given bounds.</summary>
    void setClip(bounds1 clip);
    /// <summary>Returns the current near clip.</summary>
    float nearClip() const;
    /// <summary>Sets the near clip.</summary>
    void setNearClip(float near_clip);
    /// <summary>Returns the current far clip.</summary>
    float farClip() const;
    /// <summary>Sets the far clip.</summary>
    void setFarClip(float far_clip);

protected:
    mat4 calculateMatrix() override;

private:
    float field_of_view_;
    bounds1 clip_;
};

/// <summary>An orthogonal projection provider with simple 3D clipping bounds, defaulting to [-1, 1] on all axes, while the actual clipping planes also have the aspect applied.</summary>
class OrthoProjection : public ProjectionProvider {
public:
    static constexpr bounds3 DefaultClip = {-1.0f, 1.0f};

    /// <summary>Initializes the orthogonal projection with the given clipping bounds.</summary>
    OrthoProjection(float aspect, bounds3 clip = DefaultClip);
    /// <summary>Initializes the orthogonal projection with the given clipping bounds.</summary>
    OrthoProjection(Context& context, bounds3 clip = DefaultClip);

    /// <summary>Returns the current clipping bounds.</summary>
    const bounds3& clip() const;
    /// <summary>Sets the clipping bounds.</summary>
    void setClip(const bounds3& clip);

protected:
    mat4 calculateMatrix() override;

private:
    bounds3 clip_;
};

/// <summary>A simple struct for all the different uniform names, which a camera can write to.</summary>
struct CameraUniformNames {
    std::string ProjectionMatrix;
    std::string ModelTransform;
    std::string ViewTransform;
    std::string ModelViewTransform;
};

/// <summary>The default names for all camera related uniforms.</summary>
// TODO: C++20 use named initializers { .name = value }
inline const CameraUniformNames DefaultCameraUniformNames = {
    "projection_matrix", "model_transform", "view_transform", "modelview_transform"};

/// <summary>Contains references to camera related uniforms of a single GL-Program.</summary>
class CameraUniforms {
public:
    /// <summary>Queries all relevant uniforms using the given uniform names.</summary>
    CameraUniforms(Program& program, const CameraUniformNames& names = DefaultCameraUniformNames);

    /// <summary>Returns the associated GL-Program for the collection of uniforms.</summary>
    Program& program() const;

    /// <summary>Updates the content of the uniform for the projection matrix.</summary>
    void updateProjectionMatrix(const mat4& projection_matrix) const;
    /// <summary>Updates the content of the uniform for the given transform type.</summary>
    void updateTransform(CameraTransformType type, const dquat& transform) const;

private:
    std::reference_wrapper<Program> program_;
    std::reference_wrapper<ShaderUniform<mat4>> projection_uniform_;
    dutils::EnumArray<CameraTransformType, std::reference_wrapper<ShaderUniform<mat2x4>>> transform_uniforms_;
};

/// <summary>A camera, which is capable of drawing renderables.</summary>
class Camera {
public:
    /// <summary>Creates a new camera with the given projection provider.</summary>
    explicit Camera(SharedProjectionProvider projection_provider);

    /// <summary>Creates a new perspective camera with the given parameters.</summary>
    static Camera perspective(float aspect,
                              float field_of_view = PerspectiveProjection::DefaultFieldOfView,
                              bounds1 clip = PerspectiveProjection::DefaultClip);
    /// <summary>Creates a new perspective camera with the given parameters.</summary>
    static Camera perspective(Context& context,
                              float field_of_view = PerspectiveProjection::DefaultFieldOfView,
                              bounds1 clip = PerspectiveProjection::DefaultClip);

    /// <summary>Creates a new orthogonal camera with the given parameters.</summary>
    static Camera ortho(float aspect, bounds3 clip = OrthoProjection::DefaultClip);
    /// <summary>Creates a new orthogonal camera with the given parameters.</summary>
    static Camera ortho(Context& context, bounds3 clip = OrthoProjection::DefaultClip);

    /// <summary>Returns the projection provider of the camera.</summary>
    const SharedProjectionProvider& projectionProvider() const;
    /// <summary>Returns the transform of the camera itself.</summary>
    const SharedTransform& transform() const;

    /// <summary>Allows the given program to use custom uniform names instead of the default ones.</summary>
    void setCustomUniforms(Program& program, const CameraUniformNames& names);

    /// <summary>Draws the given range of renderables, automatically updating the previously supplied uniforms.</summary>
    template <typename TRenderableIter>
    void render(TRenderableIter first, TRenderableIter last) const;
    /// <summary>Draws the given collection of renderables, automatically updating the previously supplied uniforms.</summary>
    template <typename TRenderables>
    void render(const TRenderables& renderables) const;

private:
    SharedProjectionProvider projection_provider_;
    SharedTransform transform_ = Transform::create();
    mutable std::vector<CameraUniforms> uniforms_;
};

template <typename TRenderableIter>
inline void Camera::render(TRenderableIter first, TRenderableIter last) const
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

    for (auto iter = first; iter != last; ++iter) {
        const auto& renderable = *iter;

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
            uniforms->updateTransform(CameraTransformType::ModelView,
                                      view_transform * model_transform->fullTransform());
        }
        else {
            uniforms->updateTransform(CameraTransformType::Model, dquat());
            uniforms->updateTransform(CameraTransformType::ModelView, view_transform);
        }

        renderable->draw();
    }
}

template <typename TRenderables>
void Camera::render(const TRenderables& renderables) const
{
    using std::begin;
    using std::end;
    render(begin(renderables), end(renderables));
}

} // namespace dang::gl

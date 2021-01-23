#pragma once

#include "dang-gl/Context/Context.h"
#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/Math/Transform.h"
#include "dang-gl/Objects/Program.h"
#include "dang-gl/global.h"

#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief The different cached transform (quaternion, not matrix) types of a camera, namely model, view and a combined
/// model-view.
/// @remark The projection uses a matrix and is therefore handled separately.
enum class CameraTransformType { Model, View, ModelView, COUNT };

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::CameraTransformType> : default_enum_count<dang::gl::CameraTransformType> {};

} // namespace dang::utils

namespace dang::gl {

class ProjectionProvider;

using UniqueProjectionProvider = std::unique_ptr<ProjectionProvider>;
using SharedProjectionProvider = std::shared_ptr<ProjectionProvider>;
using WeakProjectionProvider = std::weak_ptr<ProjectionProvider>;

/// @brief A base class, providing a projection matrix for a camera.
class ProjectionProvider {
public:
    /// @brief Initializes the projection provider with the given aspect.
    ProjectionProvider(float aspect);
    /// @brief Automatically updates the aspect to match the given context.
    ProjectionProvider(Context& context);
    virtual ~ProjectionProvider();

    /// @brief Returns the current aspect ratio (width/height).
    float aspect() const;
    /// @brief Sets the aspect ratio (width/height) to the given value.
    /// @remark This value will only be overwritten on the next context resize, if a context was provided.
    void setAspect(float aspect);

    /// @brief Returns the projection matrix, which is lazily evaulated.
    const mat4& matrix();

protected:
    /// @brief Can be used by sub-classes do invalidate the current matrix.
    void invalidateMatrix();
    /// @brief Calculates the projection matrix.
    virtual mat4 calculateMatrix() = 0;

private:
    float aspect_;
    Context::Event::Subscription context_resize_;
    std::optional<mat4> matrix_;
};

/// @brief A perspective projection provider with field of view and near/far clipping.
class PerspectiveProjection : public ProjectionProvider {
public:
    static constexpr float default_field_of_view = 90.0f;
    static constexpr bounds1 default_clip = {0.1f, 100.0f};

    /// @brief Initializes the perspective projection with the given field of view and near/far clip.
    PerspectiveProjection(float aspect, float field_of_view = default_field_of_view, bounds1 clip = default_clip);
    /// @brief Initializes the perspective projection with the given field of view and near/far clip.
    PerspectiveProjection(Context& context, float field_of_view = default_field_of_view, bounds1 clip = default_clip);

    /// @brief Returns the current field of view.
    float fieldOfView() const;
    /// @brief Sets the field of view.
    void setFieldOfView(float field_of_view);

    /// @brief Returns the near and far clip as low and high as bounds.
    bounds1 clip() const;
    /// @brief Sets the near and far clip as low and high of the given bounds.
    void setClip(bounds1 clip);
    /// @brief Returns the current near clip.
    float nearClip() const;
    /// @brief Sets the near clip.
    void setNearClip(float near_clip);
    /// @brief Returns the current far clip.
    float farClip() const;
    /// @brief Sets the far clip.
    void setFarClip(float far_clip);

protected:
    mat4 calculateMatrix() override;

private:
    float field_of_view_;
    bounds1 clip_;
};

/// @brief An orthogonal projection provider with simple 3D clipping bounds, defaulting to [-1, 1] on all axes, while
/// the actual clipping planes also have the aspect applied.
class OrthoProjection : public ProjectionProvider {
public:
    static constexpr bounds3 default_clip = {-1.0f, 1.0f};

    /// @brief Initializes the orthogonal projection with the given clipping bounds.
    OrthoProjection(float aspect, bounds3 clip = default_clip);
    /// @brief Initializes the orthogonal projection with the given clipping bounds.
    OrthoProjection(Context& context, bounds3 clip = default_clip);

    /// @brief Returns the current clipping bounds.
    const bounds3& clip() const;
    /// @brief Sets the clipping bounds.
    void setClip(const bounds3& clip);

protected:
    mat4 calculateMatrix() override;

private:
    bounds3 clip_;
};

/// @brief A simple struct for all the different uniform names, which a camera can write to.
struct CameraUniformNames {
    std::string projection_matrix;
    std::string model_transform;
    std::string view_transform;
    std::string model_view_transform;
};

/// @brief The default names for all camera related uniforms.
// TODO: C++20 use named initializers { .name = value }
inline const CameraUniformNames default_camera_uniform_names = {
    "projection_matrix", "model_transform", "view_transform", "modelview_transform"};

/// @brief Contains references to camera related uniforms of a single GL-Program.
class CameraUniforms {
public:
    /// @brief Queries all relevant uniforms using the given uniform names.
    CameraUniforms(Program& program, const CameraUniformNames& names = default_camera_uniform_names);

    /// @brief Returns the associated GL-Program for the collection of uniforms.
    Program& program() const;

    /// @brief Updates the content of the uniform for the projection matrix.
    void updateProjectionMatrix(const mat4& projection_matrix) const;
    /// @brief Updates the content of the uniform for the given transform type.
    void updateTransform(CameraTransformType type, const dquat& transform) const;

private:
    std::reference_wrapper<Program> program_;
    std::reference_wrapper<ShaderUniform<mat4>> projection_uniform_;
    dutils::EnumArray<CameraTransformType, std::reference_wrapper<ShaderUniform<mat2x4>>> transform_uniforms_;
};

/// @brief A camera, which is capable of drawing renderables.
class Camera {
public:
    /// @brief Creates a new camera with the given projection provider.
    explicit Camera(SharedProjectionProvider projection_provider);

    /// @brief Creates a new perspective camera with the given parameters.
    static Camera perspective(float aspect,
                              float field_of_view = PerspectiveProjection::default_field_of_view,
                              bounds1 clip = PerspectiveProjection::default_clip);
    /// @brief Creates a new perspective camera with the given parameters.
    static Camera perspective(Context& context,
                              float field_of_view = PerspectiveProjection::default_field_of_view,
                              bounds1 clip = PerspectiveProjection::default_clip);

    /// @brief Creates a new orthogonal camera with the given parameters.
    static Camera ortho(float aspect, bounds3 clip = OrthoProjection::default_clip);
    /// @brief Creates a new orthogonal camera with the given parameters.
    static Camera ortho(Context& context, bounds3 clip = OrthoProjection::default_clip);

    /// @brief Returns the projection provider of the camera.
    const SharedProjectionProvider& projectionProvider() const;
    /// @brief Returns the transform of the camera itself.
    const SharedTransform& transform() const;

    /// @brief Allows the given program to use custom uniform names instead of the default ones.
    void setCustomUniforms(Program& program, const CameraUniformNames& names);

    /// @brief Draws the given range of renderables, automatically updating the previously supplied uniforms.
    template <typename TRenderableIter>
    void render(TRenderableIter first, TRenderableIter last) const;
    /// @brief Draws the given collection of renderables, automatically updating the previously supplied uniforms.
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

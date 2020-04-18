#pragma once

#include "dang-math/bounds.h"
#include "dang-math/matrix.h"

#include "Program.h"
#include "Renderable.h"
#include "Transform.h"
#include "Window.h"

namespace dang::gl
{

class ProjectionProvider;

using UniqueProjectionProvider = std::unique_ptr<ProjectionProvider>;
using SharedProjectionProvider = std::shared_ptr<ProjectionProvider>;
using WeakProjectionProvider = std::weak_ptr<ProjectionProvider>;

/// <summary>A base class, providing a projection matrix for a camera.</summary>
class ProjectionProvider {
public:
    /// <summary>Initializes the projection provider with the given aspect.</summary>
    ProjectionProvider(float aspect);
    /// <summary>Automatically updates the aspect to match the given window.</summary>
    ProjectionProvider(Window& window);

    /// <summary>Returns the current aspect ratio (widht/height).</summary>
    float aspect() const;
    /// <summary>Sets the aspect ratio (width/height) to the given value.</summary>
    /// <remarks>This value will only be overwritten on the next window resize, if a window was provided.</remarks>
    void setAspect(float aspect);

    /// <summary>Returns the projection matrix, which is lazily evaulated.</summary>
    const dgl::mat4& matrix();

protected:
    /// <summary>Can be used by sub-classes do invalidate the current matrix.</summary>
    void invalidateMatrix();
    /// <summary>Calculates the projection matrix.</summary>
    virtual dgl::mat4 calculateMatrix() = 0;

private:
    float aspect_;
    std::optional<Window::Event::Subscription> window_resize_;
    std::optional<dgl::mat4> matrix_;
};

/// <summary>A perspective projection provider with field of view and near/far clipping.</summary>
class PerspectiveProjection : public ProjectionProvider {
public:
    static constexpr float DefaultFieldOfView = 90.0f;
    static constexpr dgl::bounds1 DefaultClip = { 0.1f, 100.0f };

    /// <summary>Initializes the perspective projection with the given field of view and near/far clip.</summary>
    PerspectiveProjection(float aspect, float field_of_view = DefaultFieldOfView, dmath::bounds1 clip = DefaultClip);
    /// <summary>Initializes the perspective projection with the given field of view and near/far clip.</summary>
    PerspectiveProjection(Window& window, float field_of_view = DefaultFieldOfView, dmath::bounds1 clip = DefaultClip);

    /// <summary>Returns the current field of view.</summary>
    float fieldOfView() const;
    /// <summary>Sets the field of view.</summary>
    void setFieldOfView(float field_of_view);

    /// <summary>Returns the near and far clip as low and high as bounds.</summary>
    dmath::bounds1 clip() const;
    /// <summary>Sets the near and far clip as low and high of the given bounds.</summary>
    void setClip(dmath::bounds1 clip);
    /// <summary>Returns the current near clip.</summary>
    float nearClip() const;
    /// <summary>Sets the near clip.</summary>
    void setNearClip(float near_clip);
    /// <summary>Returns the current far clip.</summary>
    float farClip() const;
    /// <summary>Sets the far clip.</summary>
    void setFarClip(float far_clip);

protected:
    dgl::mat4 calculateMatrix() override;

private:
    float field_of_view_;
    dgl::bounds1 clip_;
};

/// <summary>An orthogonal projection provider with simple 3D clipping bounds, defaulting to [-1, 1] on all axes.</summary>
class OrthoProjection : public ProjectionProvider {
public:
    static constexpr dgl::bounds3 DefaultClip = { -1.0f, 1.0f };

    /// <summary>Initializes the orthogonal projection with the given clipping bounds.</summary>
    OrthoProjection(float aspect, dgl::bounds3 clip = DefaultClip);
    /// <summary>Initializes the orthogonal projection with the given clipping bounds.</summary>
    OrthoProjection(Window& window, dgl::bounds3 clip = DefaultClip);

    /// <summary>Returns the current clipping bounds.</summary>
    const dmath::bounds3& clip() const;
    /// <summary>Sets the clipping bounds.</summary>
    void setClip(const dmath::bounds3& clip);

protected:
    dgl::mat4 calculateMatrix() override;

private:
    dgl::bounds3 clip_;
};

/// <summary>The different cached transform (quaternion, not matrix) types of a camera, namely model, view and a combined model-view.</summary>
/// <remarks>The projection uses a matrix and is therefore handled separately.</remarks>
enum class CameraTransformType {
    Model,
    View,
    ModelView,
    COUNT
};

/// <summary>A camera, which is capable of drawing objects, deriving the Renderable base class.</summary>
class Camera {
public:
    /// <summary>Creates a new camera with the given projection provider.</summary>
    explicit Camera(SharedProjectionProvider projection_provider);
    /// <summary>Creates a new perspective camera with the given parameters.</summary>
    static Camera perspective(float aspect, float field_of_view = PerspectiveProjection::DefaultFieldOfView, dmath::bounds1 clip = PerspectiveProjection::DefaultClip);
    /// <summary>Creates a new perspective camera with the given parameters.</summary>
    static Camera perspective(Window& window, float field_of_view = PerspectiveProjection::DefaultFieldOfView, dmath::bounds1 clip = PerspectiveProjection::DefaultClip);
    /// <summary>Creates a new orthogonal camera with the given parameters.</summary>
    static Camera ortho(float aspect, dgl::bounds3 clip = OrthoProjection::DefaultClip);
    /// <summary>Creates a new orthogonal camera with the given parameters.</summary>
    static Camera ortho(Window& window, dgl::bounds3 clip = OrthoProjection::DefaultClip);

    /// <summary>Returns the projection provider of the camera.</summary>
    const SharedProjectionProvider& projectionProvider() const;
    /// <summary>Returns the transform of the camera itself.</summary>
    const SharedTransform& transform() const;

    /// <summary>Sets the shader uniform to send the projection matrix to.</summary>
    void setProjectionUniform(ShaderUniform<dgl::mat4>& uniform);
    /// <summary>Resets the shader uniform to send the projection matrix to.</summary>
    void resetProjectionUniform();

    /// <summary>Sets the shader uniform for a specified transform to send the transformation quaternion to.</summary>
    void setTransformUniform(CameraTransformType type, ShaderUniform<dgl::mat2x4>& uniform);
    /// <summary>Resets the shader uniform for a specified transform to send the transformation quaternion to.</summary>
    void resetTransformUniform(CameraTransformType type);

    /// <summary>Adds a new object to the list of renderables.</summary>
    void addRenderable(SharedRenderable renderable);
    /// <summary>Removes an existing object from the list of renderables.</summary>
    void removeRenderable(const SharedRenderable& renderable);
    /// <summary>Removes all renderables.</summary>
    void clearRenderables();

    /// <summary>Draws all renderables, automatically updating the previously supplied uniforms.</summary>
    void render();

private:
    SharedProjectionProvider projection_provider_;
    SharedTransform transform_ = Transform::create();
    std::vector<SharedRenderable> renderables_;
    ShaderUniform<dgl::mat4>* projection_uniform_ = nullptr;
    dutils::EnumArray<CameraTransformType, ShaderUniform<dgl::mat2x4>*> transform_uniforms_{};
};

}

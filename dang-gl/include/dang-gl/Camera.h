#pragma once

#include "dang-math/bounds.h"
#include "dang-math/matrix.h"

#include "Program.h"
#include "Transform.h"

namespace dang::gl
{

class Renderable;

class ProjectionProvider {
public:
    const dgl::mat4& matrix();

protected:
    void resetMatrix();
    virtual dgl::mat4 calculateMatrix() = 0;

private:
    std::optional<dgl::mat4> matrix_;
};

class PerspectiveProjection : public ProjectionProvider {
public:
    PerspectiveProjection() = default;
    PerspectiveProjection(float aspect, float field_of_view, dmath::bounds1 clip);

    float aspect() const;
    void setAspect(float aspect);

    float fieldOfView() const;
    void setFieldOfView(float field_of_view);

    dmath::bounds1 clip() const;
    void setClip(dmath::bounds1 clip);
    float nearClip() const;
    void setNearClip(float near_clip);
    float farClip() const;
    void setFarClip(float far_clip);

protected:
    dgl::mat4 calculateMatrix() override;

private:
    float aspect_ = 1.0f;
    float field_of_view_ = 90.0f;
    dgl::bounds1 clip_ = { 0.1f, 100.0f };
};

class OrthoProjection : public ProjectionProvider {
public:
    const dmath::bounds3& clip() const;
    void setClip(const dmath::bounds3& clip);

protected:
    dgl::mat4 calculateMatrix() override;

private:
    dgl::bounds3 clip_ = { -1.0f, 1.0f };
};

enum class CameraTransformType {
    Model,
    View,
    ModelView,
    COUNT
};

class Camera {
public:
    Camera(std::unique_ptr<ProjectionProvider> view_matrix_provider);

    void setProjectionUniform(ShaderUniform<dgl::mat4>& uniform);
    void resetProjectionUniform();

    void setTransformUniform(CameraTransformType type, ShaderUniform<dgl::mat2x4>& uniform);
    void resetTransformUniform(CameraTransformType type);

    void render();

private:
    std::unique_ptr<ProjectionProvider> projection_provider_;
    std::shared_ptr<Transform> transform_;
    std::vector<std::weak_ptr<Renderable>> renderables_;
    ShaderUniform<dgl::mat4>* projection_uniform_ = nullptr;
    dutils::EnumArray<CameraTransformType, ShaderUniform<dgl::mat2x4>*> transform_uniforms_{};
};

}

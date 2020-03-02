#include "pch.h"
#include "Camera.h"

namespace dang::gl
{

const dmath::mat4& ViewMatrixProvider::viewMatrix()
{
    if (!view_matrix_)
        view_matrix_ = calculateViewMatrix();
    return *view_matrix_;
}

void ViewMatrixProvider::resetViewMatrix()
{
    view_matrix_ = std::nullopt;
}

Camera::Camera(std::unique_ptr<ViewMatrixProvider> view_matrix_provider)
    : view_matrix_provider_(std::move(view_matrix_provider))
    , transform_(std::make_shared<Transform>())
{
}

}

#pragma once

#include "dang-gl/Context/Context.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Serves as a base class for all GL-Objects of the template specified type.
template <ObjectType v_type>
class Object {
public:
    using Handle = ObjectHandle<v_type>;
    using Wrapper = ObjectWrapper<v_type>;

    /// @brief Destroys the GL-Object.
    ~Object() { destroy(); }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    void destroy()
    {
        if (!*this)
            return;
        Wrapper::destroy(handle_);
        handle_ = {};
        context_ = nullptr;
    }

    /// @brief For valid objects, returns the associated GL-Context in form of a window.
    Context& context() const noexcept { return *context_; }

    /// @brief Returns the context for this object type.
    auto& objectContext() const { return context_->contextFor<v_type>(); }

    /// @brief Returns the handle of the GL-Object or InvalidHandle for default constructed objects.
    Handle handle() const noexcept { return handle_; }

    /// @brief Whether the object is valid.
    explicit operator bool() const noexcept { return bool{handle_}; }

    void swap(Object& other) noexcept
    {
        using std::swap;
        swap(context_, other.context_);
        swap(handle_, other.handle_);
        swap(label_, other.label_);
    }

    friend void swap(Object& lhs, Object& rhs) noexcept { lhs.swap(rhs); }

    /// @brief Sets an optional label for the object, which is used in by OpenGL generated debug messages.
    void setLabel(std::optional<std::string> label)
    {
        label_ = std::move(label);
        if (label_)
            glObjectLabel(
                toGLConstant(v_type), handle_.unwrap(), static_cast<GLsizei>(label_->length()), label_->c_str());
        else
            glObjectLabel(toGLConstant(v_type), handle_.unwrap(), 0, nullptr);
    }

    /// @brief Returns the label used in OpenGL generated debug messages.
    const std::optional<std::string>& label() const { return label_; }

protected:
    Object()
        : context_(&dang::gl::context())
        , handle_(Wrapper::create())
    {
        assert(context_);
    }

    Object(Object&& other) noexcept
        : context_(std::move(other.context_))
        , handle_(std::exchange(other.handle_, {}))
        , label_(std::move(other.label_))
    {}

    Object& operator=(Object&& other) noexcept
    {
        if (this == &other)
            return *this;
        destroy();
        context_ = std::move(other.context_);
        handle_ = std::exchange(other.handle_, {});
        label_ = std::move(other.label_);
        return *this;
    }

private:
    Context* context_ = nullptr;
    Handle handle_;
    std::optional<std::string> label_;
};

/// @brief A base class for GL-Objects, which can be bound without a target.
template <ObjectType v_type>
class ObjectBindable : public Object<v_type> {
public:
    /// @brief Resets the bound object in the context if the object is still bound.
    ~ObjectBindable()
    {
        if (*this)
            this->objectContext().reset(this->handle());
    }

    ObjectBindable(const ObjectBindable&) = delete;
    ObjectBindable& operator=(const ObjectBindable&) = delete;

    /// @brief Binds the object.
    void bind() const { this->objectContext().bind(this->handle()); }

protected:
    ObjectBindable() = default;

    ObjectBindable(ObjectBindable&&) = default;
    ObjectBindable& operator=(ObjectBindable&&) = default;
};

} // namespace dang::gl

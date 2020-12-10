#pragma once

#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Wraps OpenGL object creation, destruction and binding with a consistent interface.
template <ObjectType Type>
struct ObjectWrapper {
    using Handle = ObjectHandle<Type>;

    /// @brief Creates a new OpenGL object and returns its handle.
    static Handle create()
    {
        static_assert(detail::canExecute(detail::glGenObjects<Type>) ||
                          detail::canExecute(detail::glCreateObject<Type>),
                      "No function to create this GL-Object type.");

        if constexpr (detail::canExecute(detail::glGenObjects<Type>)) {
            GLuint raw_handle{};
            detail::glGenObjects<Type>(1, &raw_handle);
            return Handle{raw_handle};
        }
        else if constexpr (detail::canExecute(detail::glCreateObject<Type>)) {
            return Handle{detail::glCreateObject<Type>()};
        }
    }

    /// @brief Destroys an OpenGL object with the given handle.
    static void destroy(Handle handle)
    {
        static_assert(detail::canExecute(detail::glDeleteObjects<Type>) ||
                          detail::canExecute(detail::glDeleteObject<Type>),
                      "No function to destroy this GL-Object type.");

        if constexpr (detail::canExecute(detail::glDeleteObjects<Type>)) {
            GLuint raw_handle = handle.unwrap();
            detail::glDeleteObjects<Type>(1, &raw_handle);
        }
        else if constexpr (detail::canExecute(detail::glDeleteObject<Type>)) {
            return detail::glDeleteObject<Type>(handle.unwrap());
        }
    }

    /// @brief Binds the given OpenGL object to the given binding target.
    template <typename Target = ObjectTarget<Type>>
    static void bind(Target target, Handle handle)
    {
        detail::glBindObject<Type>(toGLConstant(target), handle.unwrap());
    }

    /// @brief Binds the given OpenGL object.
    static void bind(Handle handle) { detail::glBindObject<Type>(handle.unwrap()); }
};

} // namespace dang::gl

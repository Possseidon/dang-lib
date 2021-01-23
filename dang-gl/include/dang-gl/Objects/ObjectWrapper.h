#pragma once

#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Wraps OpenGL object creation, destruction and binding with a consistent interface.
template <ObjectType v_type>
struct ObjectWrapper {
    using Handle = ObjectHandle<v_type>;

    /// @brief Creates a new OpenGL object and returns its handle.
    static Handle create()
    {
        static_assert(detail::canExecute(detail::glGenObjects<v_type>) ||
                          detail::canExecute(detail::glCreateObject<v_type>),
                      "No function to create this GL-Object type.");

        if constexpr (detail::canExecute(detail::glGenObjects<v_type>)) {
            GLuint raw_handle{};
            detail::glGenObjects<v_type>(1, &raw_handle);
            return Handle{raw_handle};
        }
        else if constexpr (detail::canExecute(detail::glCreateObject<v_type>)) {
            return Handle{detail::glCreateObject<v_type>()};
        }
    }

    /// @brief Destroys an OpenGL object with the given handle.
    static void destroy(Handle handle)
    {
        static_assert(detail::canExecute(detail::glDeleteObjects<v_type>) ||
                          detail::canExecute(detail::glDeleteObject<v_type>),
                      "No function to destroy this GL-Object type.");

        if constexpr (detail::canExecute(detail::glDeleteObjects<v_type>)) {
            GLuint raw_handle = handle.unwrap();
            detail::glDeleteObjects<v_type>(1, &raw_handle);
        }
        else if constexpr (detail::canExecute(detail::glDeleteObject<v_type>)) {
            return detail::glDeleteObject<v_type>(handle.unwrap());
        }
    }

    /// @brief Binds the given OpenGL object to the given binding target.
    template <typename TTarget = ObjectTarget<v_type>>
    static void bind(TTarget target, Handle handle)
    {
        detail::glBindObject<v_type>(toGLConstant(target), handle.unwrap());
    }

    /// @brief Binds the given OpenGL object.
    static void bind(Handle handle) { detail::glBindObject<v_type>(handle.unwrap()); }
};

} // namespace dang::gl

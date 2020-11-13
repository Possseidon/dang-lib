#pragma once

#include "ObjectHandle.h"
#include "ObjectType.h"

namespace dang::gl {

/// <summary>Wraps OpenGL object creation, destruction and binding with a consistent interface.</summary>
template <ObjectType Type>
struct ObjectWrapper {
    using Handle = ObjectHandle<Type>;

    /// <summary>Creates a new OpenGL object and returns its handle.</summary>
    static Handle create()
    {
        if constexpr (detail::canExecute(detail::glGenObjects<Type>)) {
            GLuint raw_handle{};
            detail::glGenObjects<Type>(1, &raw_handle);
            return Handle{raw_handle};
        }
        else if constexpr (detail::canExecute(detail::glCreateObject<Type>)) {
            return Handle{detail::glCreateObject<Type>()};
        }
        else {
            static_assert(false, "No function to create this GL-Object type.");
        }
    }

    /// <summary>Destroys an OpenGL object with the given handle.</summary>
    static void destroy(Handle handle)
    {
        if constexpr (detail::canExecute(detail::glDeleteObjects<Type>)) {
            GLuint raw_handle = handle.unwrap();
            detail::glDeleteObjects<Type>(1, &raw_handle);
        }
        else if constexpr (detail::canExecute(detail::glDeleteObject<Type>)) {
            return detail::glDeleteObject<Type>(handle.unwrap());
        }
        else {
            static_assert(false, "No function to destroy this GL-Object type.");
        }
    }

    /// <summary>Binds the given OpenGL object to the given binding target.</summary>
    template <typename Target = ObjectTarget<Type>>
    static void bind(Target target, Handle handle)
    {
        detail::glBindObject<Type>(toGLConstant(target), handle.unwrap());
    }

    /// <summary>Binds the given OpenGL object.</summary>
    static void bind(Handle handle) { detail::glBindObject<Type>(handle.unwrap()); }
};

} // namespace dang::gl

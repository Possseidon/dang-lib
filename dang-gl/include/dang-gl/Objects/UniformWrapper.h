#pragma once

#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"

#include "dang-math/matrix.h"
#include "dang-math/vector.h"

namespace dang::gl {

namespace detail {

template <std::size_t Dim, typename T>
inline constexpr auto glUniform = nullptr;

template <>
inline constexpr auto& glUniform<1, GLfloat> = glUniform1f;
template <>
inline constexpr auto& glUniform<2, GLfloat> = glUniform2f;
template <>
inline constexpr auto& glUniform<3, GLfloat> = glUniform3f;
template <>
inline constexpr auto& glUniform<4, GLfloat> = glUniform4f;

template <>
inline constexpr auto& glUniform<1, GLdouble> = glUniform1d;
template <>
inline constexpr auto& glUniform<2, GLdouble> = glUniform2d;
template <>
inline constexpr auto& glUniform<3, GLdouble> = glUniform3d;
template <>
inline constexpr auto& glUniform<4, GLdouble> = glUniform4d;

template <>
inline constexpr auto& glUniform<1, GLint> = glUniform1i;
template <>
inline constexpr auto& glUniform<2, GLint> = glUniform2i;
template <>
inline constexpr auto& glUniform<3, GLint> = glUniform3i;
template <>
inline constexpr auto& glUniform<4, GLint> = glUniform4i;

template <>
inline constexpr auto& glUniform<1, GLuint> = glUniform1ui;
template <>
inline constexpr auto& glUniform<2, GLuint> = glUniform2ui;
template <>
inline constexpr auto& glUniform<3, GLuint> = glUniform3ui;
template <>
inline constexpr auto& glUniform<4, GLuint> = glUniform4ui;

template <std::size_t Dim, typename T>
inline constexpr auto glUniformv = nullptr;

template <>
inline constexpr auto& glUniformv<1, GLfloat> = glUniform1fv;
template <>
inline constexpr auto& glUniformv<2, GLfloat> = glUniform2fv;
template <>
inline constexpr auto& glUniformv<3, GLfloat> = glUniform3fv;
template <>
inline constexpr auto& glUniformv<4, GLfloat> = glUniform4fv;

template <>
inline constexpr auto& glUniformv<1, GLdouble> = glUniform1dv;
template <>
inline constexpr auto& glUniformv<2, GLdouble> = glUniform2dv;
template <>
inline constexpr auto& glUniformv<3, GLdouble> = glUniform3dv;
template <>
inline constexpr auto& glUniformv<4, GLdouble> = glUniform4dv;

template <>
inline constexpr auto& glUniformv<1, GLint> = glUniform1iv;
template <>
inline constexpr auto& glUniformv<2, GLint> = glUniform2iv;
template <>
inline constexpr auto& glUniformv<3, GLint> = glUniform3iv;
template <>
inline constexpr auto& glUniformv<4, GLint> = glUniform4iv;

template <>
inline constexpr auto& glUniformv<1, GLuint> = glUniform1uiv;
template <>
inline constexpr auto& glUniformv<2, GLuint> = glUniform2uiv;
template <>
inline constexpr auto& glUniformv<3, GLuint> = glUniform3uiv;
template <>
inline constexpr auto& glUniformv<4, GLuint> = glUniform4uiv;

template <typename T>
inline constexpr auto glGetUniformv = nullptr;

template <>
inline constexpr auto& glGetUniformv<GLfloat> = glGetUniformfv;
template <>
inline constexpr auto& glGetUniformv<GLdouble> = glGetUniformdv;
template <>
inline constexpr auto& glGetUniformv<GLint> = glGetUniformiv;
template <>
inline constexpr auto& glGetUniformv<GLuint> = glGetUniformuiv;

template <std::size_t Cols, std::size_t Rows, typename T>
inline constexpr auto glUniformMatrixv = nullptr;

template <>
inline constexpr auto& glUniformMatrixv<2, 2, GLfloat> = glUniformMatrix2fv;
template <>
inline constexpr auto& glUniformMatrixv<2, 3, GLfloat> = glUniformMatrix2x3fv;
template <>
inline constexpr auto& glUniformMatrixv<2, 4, GLfloat> = glUniformMatrix2x4fv;
template <>
inline constexpr auto& glUniformMatrixv<3, 2, GLfloat> = glUniformMatrix3x2fv;
template <>
inline constexpr auto& glUniformMatrixv<3, 3, GLfloat> = glUniformMatrix3fv;
template <>
inline constexpr auto& glUniformMatrixv<3, 4, GLfloat> = glUniformMatrix3x4fv;
template <>
inline constexpr auto& glUniformMatrixv<4, 2, GLfloat> = glUniformMatrix4x2fv;
template <>
inline constexpr auto& glUniformMatrixv<4, 3, GLfloat> = glUniformMatrix4x3fv;
template <>
inline constexpr auto& glUniformMatrixv<4, 4, GLfloat> = glUniformMatrix4fv;

template <>
inline constexpr auto& glUniformMatrixv<2, 2, GLdouble> = glUniformMatrix2dv;
template <>
inline constexpr auto& glUniformMatrixv<2, 3, GLdouble> = glUniformMatrix2x3dv;
template <>
inline constexpr auto& glUniformMatrixv<2, 4, GLdouble> = glUniformMatrix2x4dv;
template <>
inline constexpr auto& glUniformMatrixv<3, 2, GLdouble> = glUniformMatrix3x2dv;
template <>
inline constexpr auto& glUniformMatrixv<3, 3, GLdouble> = glUniformMatrix3dv;
template <>
inline constexpr auto& glUniformMatrixv<3, 4, GLdouble> = glUniformMatrix3x4dv;
template <>
inline constexpr auto& glUniformMatrixv<4, 2, GLdouble> = glUniformMatrix4x2dv;
template <>
inline constexpr auto& glUniformMatrixv<4, 3, GLdouble> = glUniformMatrix4x3dv;
template <>
inline constexpr auto& glUniformMatrixv<4, 4, GLdouble> = glUniformMatrix4dv;

} // namespace detail

/// <summary>Wraps shader uniform access with a consistent interface.</summary>
template <typename T>
struct UniformWrapper {
    static T get(ObjectHandle<ObjectType::Program> program, GLint location)
    {
        T value{};
        detail::glGetUniformv<T>(program.unwrap(), location, &value);
        return value;
    }

    static void set(GLint location, T value) { detail::glUniform<1, T>(location, value); }
};

/// <summary>Specializes uniform access for GLboolean, using GLint.</summary>
template <>
struct UniformWrapper<GLboolean> {
    static GLboolean get(ObjectHandle<ObjectType::Program> program, GLint location)
    {
        GLint value{};
        glGetUniformiv(program.unwrap(), location, &value);
        return value != 0;
    }

    static void set(GLint location, GLboolean value) { glUniform1i(location, static_cast<GLint>(value)); }
};

/// <summary>Specializes uniform access for vectors of any supported type and size.</summary>
template <typename T, std::size_t Dim>
struct UniformWrapper<dmath::Vector<T, Dim>> {
    static dmath::Vector<T, Dim> get(ObjectHandle<ObjectType::Program> program, GLint location)
    {
        dmath::Vector<T, Dim> value;
        detail::glGetUniformv<T>(program.unwrap(), location, &value[0]);
        return value;
    }

    static void set(GLint location, const dmath::Vector<T, Dim>& value)
    {
        detail::glUniformv<Dim, T>(location, 1, &value[0]);
    }
};

/// <summary>Specializes uniform access for vectors of GLboolean and any supported size.</summary>
template <std::size_t Dim>
struct UniformWrapper<dmath::Vector<GLboolean, Dim>> {
    static dmath::Vector<GLboolean, Dim> get(ObjectHandle<ObjectType::Program> program, GLint location)
    {
        ivec<Dim> value;
        glGetUniformiv(program.unwrap(), location, &value[0]);
        return static_cast<bvec<Dim>>(value);
    }

    static void set(GLint location, const dmath::Vector<GLboolean, Dim>& value)
    {
        ivec<Dim> bvalue(value);
        detail::glUniformv<Dim, GLint>(location, 1, &bvalue[0]);
    }
};

/// <summary>Specializes uniform access for matrices of any supported type and dimensions.</summary>
template <typename T, std::size_t Cols, std::size_t Rows>
struct UniformWrapper<dmath::Matrix<T, Cols, Rows>> {
    static dmath::Matrix<T, Cols, Rows> get(ObjectHandle<ObjectType::Program> program, GLint location)
    {
        dmath::Matrix<T, Cols, Rows> value;
        detail::glGetUniformv<T>(program.unwrap(), location, &value(0, 0));
        return value;
    }

    static void set(GLint location, const dmath::Matrix<T, Cols, Rows>& value)
    {
        detail::glUniformMatrixv<Cols, Rows, T>(location, 1, GL_FALSE, &value(0, 0));
    }
};

} // namespace dang::gl

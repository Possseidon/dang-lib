#pragma once

#include "dang-math/matrix.h"
#include "dang-math/vector.h"

#include "DataType.h"
#include "Types.h"

namespace dang::gl
{

template <typename T>
struct UniformWrapper {};

template <>
struct UniformWrapper<GLfloat> {
    static GLfloat get(GLuint program, GLint location)
    {
        GLfloat value;
        glGetUniformfv(program, location, &value);
        return value;
    }

    static void set(GLint location, GLfloat value)
    {
        glUniform1f(location, value);
    }
};

template <>
struct UniformWrapper<GLdouble> {
    static GLdouble get(GLuint program, GLint location)
    {
        GLdouble value;
        glGetUniformdv(program, location, &value);
        return value;
    }

    static void set(GLint location, GLdouble value)
    {
        glUniform1d(location, value);
    }
};

template <>
struct UniformWrapper<GLint> {
    static GLint get(GLuint program, GLint location)
    {
        GLint value;
        glGetUniformiv(program, location, &value);
        return value;
    }

    static void set(GLint location, GLint value)
    {
        glUniform1i(location, value);
    }
};

template <>
struct UniformWrapper<GLuint> {
    static GLuint get(GLuint program, GLint location)
    {
        GLuint value;
        glGetUniformuiv(program, location, &value);
        return value;
    }

    static void set(GLint location, GLuint value)
    {
        glUniform1ui(location, value);
    }
};

template <>
struct UniformWrapper<GLboolean> {
    static GLboolean get(GLuint program, GLint location)
    {
        GLint value;
        glGetUniformiv(program, location, &value);
        return value != 0;
    }

    static void set(GLint location, GLboolean value)
    {
        glUniform1i(location, static_cast<GLint>(value));
    }
};

template <typename T, std::size_t Dim>
struct UniformWrapper<dmath::Vector<T, Dim>> {
    static dmath::Vector<T, Dim> get(GLuint program, GLint location)
    {
        if constexpr (std::is_same_v<T, GLboolean>) {
            dgl::ivec<Dim> value;
            glGetUniformiv(program, location, &value[0]);
            return static_cast<dgl::bvec<Dim>>(value);
        }
        else {
            dmath::Vector<T, Dim> value;
            if constexpr (std::is_same_v<T, GLfloat>)
                glGetUniformfv(program, location, &value[0]);
            else if constexpr (std::is_same_v<T, GLdouble>)
                glGetUniformdv(program, location, &value[0]);
            else if constexpr (std::is_same_v<T, GLint>)
                glGetUniformiv(program, location, &value[0]);
            else if constexpr (std::is_same_v<T, GLuint>)
                glGetUniformuiv(program, location, &value[0]);
            return value;
        }
    }

    static void set(GLint location, const dmath::Vector<T, Dim>& value)
    {
        if constexpr (std::is_same_v<T, GLboolean>) {
            dgl::ivec<Dim> bvalue(value);
            glUniform1iv(location, 1, &bvalue[0]);
        }
        else if constexpr (std::is_same_v<T, GLfloat>)
            glUniform1fv(location, 1, &value[0]);
        else if constexpr (std::is_same_v<T, GLdouble>)
            glUniform1dv(location, 1, &value[0]);
        else if constexpr (std::is_same_v<T, GLint>)
            glUniform1iv(location, 1, &value[0]);
        else if constexpr (std::is_same_v<T, GLuint>)
            glUniform1uiv(location, 1, &value[0]);
    }
};

template <typename T, std::size_t Cols, std::size_t Rows>
struct UniformWrapper<dmath::Matrix<T, Cols, Rows>> {
    static dmath::Matrix<T, Cols, Rows> get(GLuint program, GLint location)
    {
        dmath::Matrix<T, Cols, Rows> value;
        if constexpr (std::is_same_v<T, GLfloat>)
            glGetUniformfv(program, location, &value(0, 0));
        else if constexpr (std::is_same_v<T, GLdouble>)
            glGetUniformdv(program, location, &value(0, 0));
        else if constexpr (std::is_same_v<T, GLint>)
            glGetUniformiv(program, location, &value(0, 0));
        else if constexpr (std::is_same_v<T, GLuint>)
            glGetUniformuiv(program, location, &value(0, 0));
        return value;
    }

    static void set(GLint location, const dmath::Matrix<T, Cols, Rows>& value)
    {
        if constexpr (std::is_same_v<T, GLfloat>) {
            if constexpr (Cols == 2) {
                if constexpr (Rows == 2)
                    glUniformMatrix2fv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 3)
                    glUniformMatrix2x3fv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 4)
                    glUniformMatrix2x4fv(location, 1, GL_FALSE, &value(0, 0));
            }
            else if constexpr (Cols == 3) {
                if constexpr (Rows == 2)
                    glUniformMatrix3x2fv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 3)
                    glUniformMatrix3fv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 4)
                    glUniformMatrix3x4fv(location, 1, GL_FALSE, &value(0, 0));
            }
            else if constexpr (Cols == 4) {
                if constexpr (Rows == 2)
                    glUniformMatrix4x2fv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 3)
                    glUniformMatrix4x3fv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 4)
                    glUniformMatrix4fv(location, 1, GL_FALSE, &value(0, 0));
            }
        }
        else if constexpr (std::is_same_v<T, GLdouble>) {
            if constexpr (Cols == 2) {
                if constexpr (Rows == 2)
                    glUniformMatrix2dv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 3)
                    glUniformMatrix2x3dv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 4)
                    glUniformMatrix2x4dv(location, 1, GL_FALSE, &value(0, 0));
            }
            else if constexpr (Cols == 3) {
                if constexpr (Rows == 2)
                    glUniformMatrix3x2dv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 3)
                    glUniformMatrix3dv(location, 1, GL_FALSE, &value(0, 0));
                else if constexpr (Rows == 4)
                    glUniformMatrix3x4dv(location, 1, GL_FALSE, &value(0, 0));
            }
            else if constexpr (Cols == 4) {
                if constexpr (Rows == 2)
                    glUniformMatrix4x2dv(location, 1, &value(0, 0));
                else if constexpr (Rows == 3)
                    glUniformMatrix4x3dv(location, 1, &value(0, 0));
                else if constexpr (Rows == 4)
                    glUniformMatrix4dv(location, 1, &value(0, 0));
            }
        }
    }
};

}

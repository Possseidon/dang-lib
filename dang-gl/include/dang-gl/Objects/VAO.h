#pragma once

#include "dang-gl/General/GLConstants.h"
#include "dang-gl/Objects/Object.h"
#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/Program.h"
#include "dang-gl/Objects/VBO.h"
#include "dang-gl/Objects/VertexArrayContext.h"
#include "dang-gl/global.h"
#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief A list of all supported modes on how to draw vertex data.
enum class BeginMode {
    Points,
    Lines,
    LineLoop,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan,
    LinesAdjacency,
    LineStripAdjacency,
    TrianglesAdjacency,
    TriangleStripAdjacency,
    Patches,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::BeginMode> : default_enum_count<dang::gl::BeginMode> {};

} // namespace dang::utils

namespace dang::gl {

/// @brief Maps the different begin modes to their GL-Constants.
template <>
inline constexpr dutils::EnumArray<BeginMode, GLenum> gl_constants<BeginMode> = {GL_POINTS,
                                                                                 GL_LINES,
                                                                                 GL_LINE_LOOP,
                                                                                 GL_LINE_STRIP,
                                                                                 GL_TRIANGLES,
                                                                                 GL_TRIANGLE_STRIP,
                                                                                 GL_TRIANGLE_FAN,
                                                                                 GL_LINES_ADJACENCY,
                                                                                 GL_LINE_STRIP_ADJACENCY,
                                                                                 GL_TRIANGLES_ADJACENCY,
                                                                                 GL_TRIANGLE_STRIP_ADJACENCY,
                                                                                 GL_PATCHES};

/// @brief A base class for all vertex array objects, which is not templated yet.
class VAOBase : public ObjectBindable<ObjectType::VertexArray> {
public:
    ~VAOBase() = default;

    VAOBase(const VAOBase&) = delete;
    VAOBase& operator=(const VAOBase&) = delete;

    /// @brief The GL-Program associated with the VAO.
    Program& program() const;

    /// @brief Returns the current render mode, which is used in draw calls.
    BeginMode mode() const;
    /// @brief Although not always sensible, allows to modify the render mode after construction.
    /// @remark Different render modes require very different data layouts, often making it impossible to use the same
    /// data with different modes.
    void setMode(BeginMode mode);

protected:
    /// @brief Initializes the VAO base with the given GL-Program and optional render mode, which defaults to the most
    /// commonly used "triangles" mode.
    VAOBase(Program& program, BeginMode mode = BeginMode::Triangles);

    VAOBase(EmptyObject)
        : ObjectBindable<ObjectType::VertexArray>(empty_object)
    {}

    VAOBase(VAOBase&&) = default;
    VAOBase& operator=(VAOBase&&) = default;

private:
    Program* program_;
    BeginMode mode_;
};

/// @brief A vertex array object, combining a GL-Program with a VBO and optional additional VBOs for instancing.
template <typename TData, typename... TInstanceData>
class VAO : public VAOBase {
public:
    /// @brief Creates a new VAO and binds it to the given GL-Program, VBO and potential additional VBOs for instancing.
    /// @remark Various debug assertions check, that the GL-Program and VBOs match.
    VAO(Program& program,
        VBO<TData>& data_vbo,
        VBO<TInstanceData>&... instance_vbo,
        BeginMode mode = BeginMode::Triangles)
        : VAOBase(program, mode)
        , data_vbo_(&data_vbo)
        , instance_vbos_(&instance_vbo...)
    {
        assert(program.instancedAttributeOrder().size() == sizeof...(TInstanceData));
        enableAttributes(std::index_sequence_for<TInstanceData...>());
    }

    VAO(EmptyObject)
        : VAOBase(empty_object)
    {}

    ~VAO() = default;

    VAO(const VAO&) = delete;
    VAO(VAO&&) = default;
    VAO& operator=(const VAO&) = delete;
    VAO& operator=(VAO&&) = default;

    /// @brief Returns the instance count, which should match for all instance VBOs, checked by a debug assertion.
    GLsizei instanceCount() const
    {
        return instanceCountHelper(std::make_index_sequence<sizeof...(TInstanceData) - 1>());
    }

    /// @brief Draws the full content of the VBO, potentially using instanced rendering, if at least one instance VBO
    /// was specified.
    void draw() const
    {
        bind();
        program().bind();
        if constexpr (sizeof...(TInstanceData) == 0)
            glDrawArrays(toGLConstant(mode()), 0, data_vbo_->count());
        else
            glDrawArraysInstanced(toGLConstant(mode()), 0, data_vbo_->count(), instanceCount());
    }

private:
    /// @brief Returns the instance count of the VBO with the given index.
    template <std::size_t v_vbo_index>
    GLsizei instanceCountOf() const
    {
        return std::get<v_vbo_index>(instance_vbos_)->count() *
               program().instancedAttributeOrder()[v_vbo_index].divisor;
    }

    /// @brief Helper function for instance counting, which takes an index list of one less than the actual instance VBO
    /// count.
    template <std::size_t... v_indices>
    GLsizei instanceCountHelper(std::index_sequence<v_indices...>) const
    {
        if constexpr (sizeof...(TInstanceData) > 1)
            assert(((instanceCountOf<v_indices>() == instanceCountOf<v_indices + 1>()) && ...));
        return instanceCountOf<0>();
    }

    /// @brief Enables all attributes for both data and specified instance VBOs.
    template <std::size_t... v_indices>
    void enableAttributes(std::index_sequence<v_indices...>)
    {
        bind();
        enableAttributes(*data_vbo_, program().attributeOrder());
        (enableAttributes(*std::get<v_indices>(instance_vbos_), program().instancedAttributeOrder()[v_indices]), ...);
    }

    /// @brief Enables attributes for the given VBO with the given attribute order.
    template <typename T>
    void enableAttributes(const VBO<T>& vbo, const AttributeOrder& attribute_order)
    {
        assert(attribute_order.stride == sizeof(T));

        vbo.bind();
        for (const ShaderAttribute& attribute : attribute_order.attributes) {
            const auto component_count = getDataTypeComponentCount(attribute.type());
            const auto base_type = getBaseDataType(attribute.type());
            const auto component_size = component_count * getDataTypeSize(base_type);

            const auto index = [&attribute](GLuint offset) -> GLuint { return attribute.location() + offset; };
            const GLint size = component_count;
            const GLenum type = static_cast<GLenum>(base_type);
            constexpr GLboolean normalized = GL_FALSE;
            const GLsizei stride = attribute_order.stride;
            const auto pointer = [&attribute, component_size](GLuint offset) {
                const auto result = static_cast<std::uintptr_t>(offset) * component_size + attribute.offset();
                return reinterpret_cast<const void*>(result);
            };

            // matrices take up one location per column
            // arrays take up one location per index
            GLuint location_count = getDataTypeColumnCount(attribute.type()) * attribute.count();

            for (GLuint offset = 0; offset < location_count; offset++) {
                glEnableVertexAttribArray(index(offset));
                glVertexAttribDivisor(index(offset), attribute_order.divisor);
            }

            switch (base_type) {
            case DataType::Float:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribPointer(index(offset), size, type, normalized, stride, pointer(offset));
                break;

            case DataType::Double:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribLPointer(index(offset), size, type, stride, pointer(offset));
                break;

            case DataType::Int:
            case DataType::UInt:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribIPointer(index(offset), size, type, stride, pointer(offset));
                break;

            default:
                throw std::runtime_error("Invalid base GL-DataType.");
            }
        }
    }

    VBO<TData>* data_vbo_;
    std::tuple<VBO<TInstanceData>*...> instance_vbos_;
};

} // namespace dang::gl

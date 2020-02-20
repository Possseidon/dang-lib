#pragma once

#include "Types.h"

namespace dang::gl
{

enum class DataType {
    None = 0,

    Float = GL_FLOAT,
    Vec2 = GL_FLOAT_VEC2,
    Vec3 = GL_FLOAT_VEC3,
    Vec4 = GL_FLOAT_VEC4,

    Double = GL_DOUBLE,
    DVec2 = GL_DOUBLE_VEC2,
    DVec3 = GL_DOUBLE_VEC3,
    DVec4 = GL_DOUBLE_VEC4,

    Int = GL_INT,
    IVec2 = GL_INT_VEC2,
    IVec3 = GL_INT_VEC3,
    IVec4 = GL_INT_VEC4,

    UInt = GL_UNSIGNED_INT,
    UVec2 = GL_UNSIGNED_INT_VEC2,
    UVec3 = GL_UNSIGNED_INT_VEC3,
    UVec4 = GL_UNSIGNED_INT_VEC4,

    Bool = GL_BOOL,
    BVec2 = GL_BOOL_VEC2,
    BVec3 = GL_BOOL_VEC3,
    BVec4 = GL_BOOL_VEC4,

    Mat2 = GL_FLOAT_MAT2,
    Mat3 = GL_FLOAT_MAT3,
    Mat4 = GL_FLOAT_MAT4,
    Mat2x3 = GL_FLOAT_MAT2x3,
    Mat2x4 = GL_FLOAT_MAT2x4,
    Mat3x2 = GL_FLOAT_MAT3x2,
    Mat3x4 = GL_FLOAT_MAT3x4,
    Mat4x2 = GL_FLOAT_MAT4x2,
    Mat4x3 = GL_FLOAT_MAT4x3,

    DMat2 = GL_DOUBLE_MAT2,
    DMat3 = GL_DOUBLE_MAT3,
    DMat4 = GL_DOUBLE_MAT4,
    DMat2x3 = GL_DOUBLE_MAT2x3,
    DMat2x4 = GL_DOUBLE_MAT2x4,
    DMat3x2 = GL_DOUBLE_MAT3x2,
    DMat3x4 = GL_DOUBLE_MAT3x4,
    DMat4x2 = GL_DOUBLE_MAT4x2,
    DMat4x3 = GL_DOUBLE_MAT4x3,

    Sampler1D = GL_SAMPLER_1D,
    Sampler2D = GL_SAMPLER_2D,
    Sampler3D = GL_SAMPLER_3D,
    SamplerCube = GL_SAMPLER_CUBE,
    Sampler1DShadow = GL_SAMPLER_1D_SHADOW,
    Sampler2DShadow = GL_SAMPLER_2D_SHADOW,
    Sampler1DArray = GL_SAMPLER_1D_ARRAY,
    Sampler2DArray = GL_SAMPLER_2D_ARRAY,
    Sampler1DArrayShadow = GL_SAMPLER_1D_ARRAY_SHADOW,
    Sampler2DArrayShadow = GL_SAMPLER_2D_ARRAY_SHADOW,
    Sampler2DMS = GL_SAMPLER_2D_MULTISAMPLE,
    Sampler2DMSArray = GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
    SamplerCubeShadow = GL_SAMPLER_CUBE_SHADOW,
    SamplerBuffer = GL_SAMPLER_BUFFER,
    Sampler2DRect = GL_SAMPLER_2D_RECT,
    Sampler2DRectShadow = GL_SAMPLER_2D_RECT_SHADOW,

    ISampler1D = GL_INT_SAMPLER_1D,
    ISampler2D = GL_INT_SAMPLER_2D,
    ISampler3D = GL_INT_SAMPLER_3D,
    ISamplerCube = GL_INT_SAMPLER_CUBE,
    ISampler1DArray = GL_INT_SAMPLER_1D_ARRAY,
    ISampler2DArray = GL_INT_SAMPLER_2D_ARRAY,
    ISampler2DMS = GL_INT_SAMPLER_2D_MULTISAMPLE,
    ISampler2DMSArray = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    ISamplerBuffer = GL_INT_SAMPLER_BUFFER,
    ISampler2DRect = GL_INT_SAMPLER_2D_RECT,

    USampler1D = GL_UNSIGNED_INT_SAMPLER_1D,
    USampler2D = GL_UNSIGNED_INT_SAMPLER_2D,
    USampler3D = GL_UNSIGNED_INT_SAMPLER_3D,
    USamplerCube = GL_UNSIGNED_INT_SAMPLER_CUBE,
    USampler1DArray = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,
    USampler2DArray = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
    USampler2DMS = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
    USampler2DMSArray = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    USamplerBuffer = GL_UNSIGNED_INT_SAMPLER_BUFFER,
    USampler2DRect = GL_UNSIGNED_INT_SAMPLER_2D_RECT,

    Image1D = GL_IMAGE_1D,
    Image2D = GL_IMAGE_2D,
    Image3D = GL_IMAGE_3D,
    Image2DRect = GL_IMAGE_2D_RECT,
    ImageCube = GL_IMAGE_CUBE,
    ImageBuffer = GL_IMAGE_BUFFER,
    Image1DArray = GL_IMAGE_1D_ARRAY,
    Image2DArray = GL_IMAGE_2D_ARRAY,
    Image2DMS = GL_IMAGE_2D_MULTISAMPLE,
    Image2DMSArray = GL_IMAGE_2D_MULTISAMPLE_ARRAY,

    IImage1D = GL_INT_IMAGE_1D,
    IImage2D = GL_INT_IMAGE_2D,
    IImage3D = GL_INT_IMAGE_3D,
    IImage2DRect = GL_INT_IMAGE_2D_RECT,
    IImageCube = GL_INT_IMAGE_CUBE,
    IImageBuffer = GL_INT_IMAGE_BUFFER,
    IImage1DArray = GL_INT_IMAGE_1D_ARRAY,
    IImage2DArray = GL_INT_IMAGE_2D_ARRAY,
    IImage2DMS = GL_INT_IMAGE_2D_MULTISAMPLE,
    IImage2DMSArray = GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

    UImage1D = GL_UNSIGNED_INT_IMAGE_1D,
    UImage2D = GL_UNSIGNED_INT_IMAGE_2D,
    UImage3D = GL_UNSIGNED_INT_IMAGE_3D,
    UImage2DRect = GL_UNSIGNED_INT_IMAGE_2D_RECT,
    UImageCube = GL_UNSIGNED_INT_IMAGE_CUBE,
    UImageBuffer = GL_UNSIGNED_INT_IMAGE_BUFFER,
    UImage1DArray = GL_UNSIGNED_INT_IMAGE_1D_ARRAY,
    UImage2DArray = GL_UNSIGNED_INT_IMAGE_2D_ARRAY,
    UImage2DMS = GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,
    UImage2DMSArray = GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

    AtomicUInt = GL_UNSIGNED_INT_ATOMIC_COUNTER
};

constexpr DataType getBaseDataType(DataType type)
{
    switch (type) {
    case DataType::None:
        return DataType::None;

    case DataType::Float:
    case DataType::Vec2:
    case DataType::Vec3:
    case DataType::Vec4:
        return DataType::Float;

    case DataType::Double:
    case DataType::DVec2:
    case DataType::DVec3:
    case DataType::DVec4:
        return DataType::Double;

    case DataType::Int:
    case DataType::IVec2:
    case DataType::IVec3:
    case DataType::IVec4:
        return DataType::Int;

    case DataType::UInt:
    case DataType::UVec2:
    case DataType::UVec3:
    case DataType::UVec4:
        return DataType::UInt;

    case DataType::Bool:
    case DataType::BVec2:
    case DataType::BVec3:
    case DataType::BVec4:
        return DataType::Bool;

    case DataType::Mat2:
    case DataType::Mat3:
    case DataType::Mat4:
    case DataType::Mat2x3:
    case DataType::Mat2x4:
    case DataType::Mat3x2:
    case DataType::Mat3x4:
    case DataType::Mat4x2:
    case DataType::Mat4x3:
        return DataType::Float;

    case DataType::DMat2:
    case DataType::DMat3:
    case DataType::DMat4:
    case DataType::DMat2x3:
    case DataType::DMat2x4:
    case DataType::DMat3x2:
    case DataType::DMat3x4:
    case DataType::DMat4x2:
    case DataType::DMat4x3:
        return DataType::Double;

    case DataType::Sampler1D:
    case DataType::Sampler2D:
    case DataType::Sampler3D:
    case DataType::SamplerCube:
    case DataType::Sampler1DShadow:
    case DataType::Sampler2DShadow:
    case DataType::Sampler1DArray:
    case DataType::Sampler2DArray:
    case DataType::Sampler1DArrayShadow:
    case DataType::Sampler2DArrayShadow:
    case DataType::Sampler2DMS:
    case DataType::Sampler2DMSArray:
    case DataType::SamplerCubeShadow:
    case DataType::SamplerBuffer:
    case DataType::Sampler2DRect:
    case DataType::Sampler2DRectShadow:
    case DataType::ISampler1D:
    case DataType::ISampler2D:
    case DataType::ISampler3D:
    case DataType::ISamplerCube:
    case DataType::ISampler1DArray:
    case DataType::ISampler2DArray:
    case DataType::ISampler2DMS:
    case DataType::ISampler2DMSArray:
    case DataType::ISamplerBuffer:
    case DataType::ISampler2DRect:
    case DataType::USampler1D:
    case DataType::USampler2D:
    case DataType::USampler3D:
    case DataType::USamplerCube:
    case DataType::USampler1DArray:
    case DataType::USampler2DArray:
    case DataType::USampler2DMS:
    case DataType::USampler2DMSArray:
    case DataType::USamplerBuffer:
    case DataType::USampler2DRect:
    case DataType::Image1D:
    case DataType::Image2D:
    case DataType::Image3D:
    case DataType::Image2DRect:
    case DataType::ImageCube:
    case DataType::ImageBuffer:
    case DataType::Image1DArray:
    case DataType::Image2DArray:
    case DataType::Image2DMS:
    case DataType::Image2DMSArray:
    case DataType::IImage1D:
    case DataType::IImage2D:
    case DataType::IImage3D:
    case DataType::IImage2DRect:
    case DataType::IImageCube:
    case DataType::IImageBuffer:
    case DataType::IImage1DArray:
    case DataType::IImage2DArray:
    case DataType::IImage2DMS:
    case DataType::IImage2DMSArray:
    case DataType::UImage1D:
    case DataType::UImage2D:
    case DataType::UImage3D:
    case DataType::UImage2DRect:
    case DataType::UImageCube:
    case DataType::UImageBuffer:
    case DataType::UImage1DArray:
    case DataType::UImage2DArray:
    case DataType::UImage2DMS:
    case DataType::UImage2DMSArray:
        return DataType::Int;

    case DataType::AtomicUInt:
        return DataType::UInt;
    }

    throw std::runtime_error("Unknown base GL-DataType.");
}

constexpr GLsizei getDataTypeSize(DataType type)
{
    switch (type) {
    case DataType::Float:
        return sizeof(GLfloat);
    case DataType::Vec2:
        return sizeof(dgl::vec2);
    case DataType::Vec3:
        return sizeof(dgl::vec3);
    case DataType::Vec4:
        return sizeof(dgl::vec4);

    case DataType::Double:
        return sizeof(GLdouble);
    case DataType::DVec2:
        return sizeof(dgl::dvec2);
    case DataType::DVec3:
        return sizeof(dgl::dvec3);
    case DataType::DVec4:
        return sizeof(dgl::dvec4);

    case DataType::Int:
        return sizeof(GLint);
    case DataType::IVec2:
        return sizeof(dgl::ivec2);
    case DataType::IVec3:
        return sizeof(dgl::ivec3);
    case DataType::IVec4:
        return sizeof(dgl::ivec4);

    case DataType::UInt:
        return sizeof(GLuint);
    case DataType::UVec2:
        return sizeof(dgl::uvec2);
    case DataType::UVec3:
        return sizeof(dgl::uvec3);
    case DataType::UVec4:
        return sizeof(dgl::uvec4);

    case DataType::Bool:
        return sizeof(GLboolean);
    case DataType::BVec2:
        return sizeof(dgl::bvec2);
    case DataType::BVec3:
        return sizeof(dgl::bvec3);
    case DataType::BVec4:
        return sizeof(dgl::bvec4);

    case DataType::Mat2:
        return sizeof(dgl::mat2);
    case DataType::Mat3:
        return sizeof(dgl::mat3);
    case DataType::Mat4:
        return sizeof(dgl::mat4);
    case DataType::Mat2x3:
        return sizeof(dgl::mat2x3);
    case DataType::Mat2x4:
        return sizeof(dgl::mat2x4);
    case DataType::Mat3x2:
        return sizeof(dgl::mat3x2);
    case DataType::Mat3x4:
        return sizeof(dgl::mat3x4);
    case DataType::Mat4x2:
        return sizeof(dgl::mat4x2);
    case DataType::Mat4x3:
        return sizeof(dgl::mat4x3);

    case DataType::DMat2:
        return sizeof(dgl::dmat2);
    case DataType::DMat3:
        return sizeof(dgl::dmat3);
    case DataType::DMat4:
        return sizeof(dgl::dmat4);
    case DataType::DMat2x3:
        return sizeof(dgl::dmat2x3);
    case DataType::DMat2x4:
        return sizeof(dgl::dmat2x4);
    case DataType::DMat3x2:
        return sizeof(dgl::dmat3x2);
    case DataType::DMat3x4:
        return sizeof(dgl::dmat3x4);
    case DataType::DMat4x2:
        return sizeof(dgl::dmat4x2);
    case DataType::DMat4x3:
        return sizeof(dgl::dmat4x3);

    case DataType::Sampler1D:
    case DataType::Sampler2D:
    case DataType::Sampler3D:
    case DataType::SamplerCube:
    case DataType::Sampler1DShadow:
    case DataType::Sampler2DShadow:
    case DataType::Sampler1DArray:
    case DataType::Sampler2DArray:
    case DataType::Sampler1DArrayShadow:
    case DataType::Sampler2DArrayShadow:
    case DataType::Sampler2DMS:
    case DataType::Sampler2DMSArray:
    case DataType::SamplerCubeShadow:
    case DataType::SamplerBuffer:
    case DataType::Sampler2DRect:
    case DataType::Sampler2DRectShadow:
    case DataType::ISampler1D:
    case DataType::ISampler2D:
    case DataType::ISampler3D:
    case DataType::ISamplerCube:
    case DataType::ISampler1DArray:
    case DataType::ISampler2DArray:
    case DataType::ISampler2DMS:
    case DataType::ISampler2DMSArray:
    case DataType::ISamplerBuffer:
    case DataType::ISampler2DRect:
    case DataType::USampler1D:
    case DataType::USampler2D:
    case DataType::USampler3D:
    case DataType::USamplerCube:
    case DataType::USampler1DArray:
    case DataType::USampler2DArray:
    case DataType::USampler2DMS:
    case DataType::USampler2DMSArray:
    case DataType::USamplerBuffer:
    case DataType::USampler2DRect:
    case DataType::Image1D:
    case DataType::Image2D:
    case DataType::Image3D:
    case DataType::Image2DRect:
    case DataType::ImageCube:
    case DataType::ImageBuffer:
    case DataType::Image1DArray:
    case DataType::Image2DArray:
    case DataType::Image2DMS:
    case DataType::Image2DMSArray:
    case DataType::IImage1D:
    case DataType::IImage2D:
    case DataType::IImage3D:
    case DataType::IImage2DRect:
    case DataType::IImageCube:
    case DataType::IImageBuffer:
    case DataType::IImage1DArray:
    case DataType::IImage2DArray:
    case DataType::IImage2DMS:
    case DataType::IImage2DMSArray:
    case DataType::UImage1D:
    case DataType::UImage2D:
    case DataType::UImage3D:
    case DataType::UImage2DRect:
    case DataType::UImageCube:
    case DataType::UImageBuffer:
    case DataType::UImage1DArray:
    case DataType::UImage2DArray:
    case DataType::UImage2DMS:
    case DataType::UImage2DMSArray:
        return sizeof(GLint);

    case DataType::AtomicUInt:
        return sizeof(GLuint);
    }

    throw std::runtime_error("Unknown GL-DataType.");
}

constexpr GLsizei getDataTypeComponentCount(DataType type)
{
    switch (type) {
    case dang::gl::DataType::Vec2:
    case dang::gl::DataType::DVec2:
    case dang::gl::DataType::IVec2:
    case dang::gl::DataType::UVec2:
    case dang::gl::DataType::BVec2:
    case dang::gl::DataType::Mat2:
    case dang::gl::DataType::Mat3x2:
    case dang::gl::DataType::Mat4x2:
    case dang::gl::DataType::DMat2:
    case dang::gl::DataType::DMat3x2:
    case dang::gl::DataType::DMat4x2:
        return 2;

    case dang::gl::DataType::Vec3:
    case dang::gl::DataType::DVec3:
    case dang::gl::DataType::IVec3:
    case dang::gl::DataType::UVec3:
    case dang::gl::DataType::BVec3:
    case dang::gl::DataType::Mat3:
    case dang::gl::DataType::Mat2x3:
    case dang::gl::DataType::Mat4x3:
    case dang::gl::DataType::DMat3:
    case dang::gl::DataType::DMat2x3:
    case dang::gl::DataType::DMat4x3:
        return 3;

    case dang::gl::DataType::Vec4:
    case dang::gl::DataType::DVec4:
    case dang::gl::DataType::IVec4:
    case dang::gl::DataType::UVec4:
    case dang::gl::DataType::BVec4:
    case dang::gl::DataType::Mat4:
    case dang::gl::DataType::Mat2x4:
    case dang::gl::DataType::Mat3x4:
    case dang::gl::DataType::DMat4:
    case dang::gl::DataType::DMat2x4:
    case dang::gl::DataType::DMat3x4:
        return 4;

    default:
        return 1;
    }
}

constexpr GLsizei getDataTypeColumnCount(DataType type)
{
    switch (type) {
    case DataType::Mat2:
    case DataType::Mat2x3:
    case DataType::Mat2x4:
    case DataType::DMat2:
    case DataType::DMat2x3:
    case DataType::DMat2x4:
        return 2;

    case DataType::Mat3x2:
    case DataType::Mat3:
    case DataType::Mat3x4:
    case DataType::DMat3x2:
    case DataType::DMat3:
    case DataType::DMat3x4:
        return 3;

    case DataType::Mat4x2:
    case DataType::Mat4x3:
    case DataType::Mat4:
    case DataType::DMat4x2:
    case DataType::DMat4x3:
    case DataType::DMat4:
        return 4;

    default:
        return 1;
    }
}

}

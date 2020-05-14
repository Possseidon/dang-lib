#pragma once

#include "Buffer.h"
#include "GLConstants.h"

namespace dang::gl
{

/// <summary>Usage hints for how a VBO is going to be used.</summary>
/// <remarks>DynamicDraw is usually the best choice.</remarks>
enum class BufferUsageHint {
    StreamDraw,
    StreamRead,
    StreamCopy,
    StaticDraw,
    StaticRead,
    StaticCopy,
    DynamicDraw,
    DynamicRead,
    DynamicCopy,

    COUNT
};

/// <summary>Maps the various buffer usage hints to their GL-Constants.</summary>
template <>
constexpr dutils::EnumArray<BufferUsageHint, GLenum> GLConstants<BufferUsageHint> = {
    GL_STREAM_DRAW,
    GL_STREAM_READ,
    GL_STREAM_COPY,
    GL_STATIC_DRAW,
    GL_STATIC_READ,
    GL_STATIC_COPY,
    GL_DYNAMIC_DRAW,
    GL_DYNAMIC_READ,
    GL_DYNAMIC_COPY
};

/// <summary>Thrown, when a VBO is locked (e.g. it is mapped) and cannot be rebound.</summary>
class VBOBindError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <typename T>
class VBO;

/// <summary>Provides a random access container interface to a mapped VBO.</summary>
template <typename T>
class VBOMapping {
public:

    /// <summary>An iterator, allowing random access to mapped VBO data.</summary>
    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() = default;
        explicit iterator(pointer position) : position_(position) {}

        reference operator*()
        {
            return *position_;
        }

        pointer operator->()
        {
            return position_;
        }

        friend bool operator==(iterator lhs, iterator rhs)
        {
            return lhs.position_ == rhs.position_;
        }

        friend bool operator!=(iterator lhs, iterator rhs)
        {
            return lhs.position_ != rhs.position_;
        }

        friend bool operator<(iterator lhs, iterator rhs)
        {
            return lhs.position_ < rhs.position_;
        }

        friend bool operator<=(iterator lhs, iterator rhs)
        {
            return lhs.position_ <= rhs.position_;
        }

        friend bool operator>(iterator lhs, iterator rhs)
        {
            return lhs.position_ > rhs.position_;
        }

        friend bool operator>=(iterator lhs, iterator rhs)
        {
            return lhs.position_ >= rhs.position_;
        }

        iterator& operator++()
        {
            position_++;
            return *this;
        }

        iterator operator++(int)
        {
            auto old = *this;
            position_++;
            return old;
        }

        iterator& operator--()
        {
            position_--;
            return *this;
        }

        iterator operator--(int)
        {
            auto old = *this;
            position_--;
            return old;
        }

        iterator& operator+=(std::ptrdiff_t offset)
        {
            position_ += offset;
            return *this;
        }

        iterator operator+(std::ptrdiff_t offset) const
        {
            return *this += offset;
        }

        iterator& operator-=(std::ptrdiff_t offset)
        {
            position_ -= offset;
            return *this;
        }

        iterator operator-(std::ptrdiff_t offset) const
        {
            return *this -= offset;
        }

        reference operator[](std::ptrdiff_t offset) const
        {
            return position_[offset];
        }

    private:
        T* position_ = nullptr;
    };

    /// <summary>Maps and locks the given VBO to stay bound, as only one VBO can be mapped at any given time.</summary>
    VBOMapping(VBO<T>& vbo)
        : vbo_(vbo)
    {
        data_ = static_cast<T*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    }

    /// <summary>Unmaps and unlocks the VBO again.</summary>
    ~VBOMapping()
    {
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    VBOMapping(const VBOMapping&) = delete;
    VBOMapping(VBOMapping&&) = delete;
    VBOMapping& operator=(const VBOMapping&) = delete;
    VBOMapping& operator=(VBOMapping&&) = delete;

    /// <summary>Returns the element count of the VBO.</summary>
    std::size_t size() const
    {
        return vbo_.count();
    }

    /// <summary>Returns the element count of the VBO.</summary>
    std::size_t max_size() const
    {
        return vbo_.count();
    }

    /// <summary>Returns an iterator to the first element of the mapped data.</summary>
    iterator begin() noexcept
    {
        return iterator(data_);
    }

    /// <summary>Returns an iterator to one after the last element of the mapped data.</summary>
    iterator end() noexcept
    {
        return iterator(data_ + size());
    }

private:
    VBO<T>& vbo_;
    // TODO: VBOLock<T> lock_{ vbo_ };
    T* data_;
};

/// <summary>A vertex buffer object for a given data struct.</summary>
template <typename T>
class VBO : public BufferBase<BufferTarget::ArrayBuffer> {
public:
    static_assert(std::is_standard_layout_v<T>, "VBO-Data must be a standard-layout type");

    /// <summary>Returns the element count of the buffer.</summary>
    GLsizei count() const
    {
        return count_;
    }

    /// <summary>Creates new data from the given element count and data pointer.</summary>
    void generate(GLsizei count, const T* data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        bind();
        count_ = count;
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), data, toGLConstant(usage));
    }

    /// <summary>Creates new uninitialized data for a given number of elements.</summary>
    void generate(GLsizei count, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(count, nullptr, usage);
    }

    /// <summary>Creates new uninitialized data for a given number of elements.</summary>
    template <std::size_t Count>
    void generate(BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Count, usage);
    }

    /// <summary>Creates new data from the given initializer list.</summary>
    void generate(std::initializer_list<T> data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(data.size()), data.begin(), usage);
    }

    /// <summary>Creates new data from the given std::vector.</summary>
    void generate(const std::vector<T>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(data.size()), data.data(), usage);
    }

    /// <summary>Creates new data from the given std::vector iterator.</summary>
    void generate(typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(count), &*begin, usage);
    }

    /// <summary>Creates new data from the given C-Style array.</summary>
    template <GLsizei Size>
    void generate(const T(&data)[Size], BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Size, data, usage);
    }

    /// <summary>Creates new data from the given std::array.</summary>
    template <GLsizei Size>
    void generate(const std::array<T, Size>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Size, data.data(), usage);
    }

    /// <summary>Creates new data from the given std::array iterator.</summary>
    template <GLsizei Size>
    void generate(typename std::array<T, Size>::const_iterator begin, typename std::array<T, Size>::const_iterator end, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(count), &*begin, usage);
    }

    /// <summary>Modifies the existing buffer at the given range with the given data pointer.</summary>
    void modify(GLsizei offset, GLsizei count, const T* data)
    {
        bind();
        glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(T), count * sizeof(T), data);
    }

    /// <summary>Modifies the existing buffer at the given position with the given initializer list.</summary>
    void modify(GLsizei offset, std::initializer_list<T> data)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(data.size()), data.begin());
    }

    /// <summary>Modifies the existing buffer at the given position with the given std::vector.</summary>
    void modify(GLsizei offset, const std::vector<T>& data)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(data.size()), data.data());
    }

    /// <summary>Modifies the existing buffer at the given position with the given std::vector iterators.</summary>
    void modify(GLsizei offset, typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(count), &*begin);
    }

    /// <summary>Modifies the existing buffer at the given position with the given C-Style array.</summary>
    template <GLsizei Size>
    void modify(GLsizei offset, const T(&data)[Size])
    {
        modify(offset, Size, data);
    }

    /// <summary>Modifies the existing buffer at the given position with the given std::array.</summary>
    template <GLsizei Size>
    void modify(GLsizei offset, const std::array<T, Size>& data)
    {
        modify(offset, Size, data.data());
    }

    /// <summary>Modifies the existing buffer at the given position with the given std::array iterators.</summary>
    template <GLsizei Size>
    void modify(GLsizei offset, typename std::array<T, Size>::const_iterator begin, typename std::array<T, Size>::const_iterator end)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(count), &*begin);
    }

    /// <summary>Maps the buffer and returns a container-like wrapper to the mapping.</summary>
    VBOMapping<T> map()
    {
        return VBOMapping<T>(*this);
    }

private:
    GLsizei count_ = 0;
};

}

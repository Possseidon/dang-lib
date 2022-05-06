#pragma once

#include "dang-gl/General/GLConstants.h"
#include "dang-gl/Objects/Buffer.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"
#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief Usage hints for how a VBO is going to be used.
/// @remark DynamicDraw is usually the best choice.
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

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::BufferUsageHint> : default_enum_count<dang::gl::BufferUsageHint> {};

} // namespace dang::utils

namespace dang::gl {

/// @brief Maps the various buffer usage hints to their GL-Constants.
template <>
inline constexpr dutils::EnumArray<BufferUsageHint, GLenum> gl_constants<BufferUsageHint> = {
    GL_STREAM_DRAW,
    GL_STREAM_READ,
    GL_STREAM_COPY,
    GL_STATIC_DRAW,
    GL_STATIC_READ,
    GL_STATIC_COPY,
    GL_DYNAMIC_DRAW,
    GL_DYNAMIC_READ,
    GL_DYNAMIC_COPY,
};

/// @brief Thrown, when a VBO is locked (e.g. it is mapped) and cannot be rebound.
class VBOBindError : public std::runtime_error {
    using runtime_error::runtime_error;
};

template <typename T>
class VBO;

/// @brief Provides a random access container interface to a mapped VBO.
template <typename T>
class VBOMapping {
public:
    /// @brief An iterator, allowing random access to mapped VBO data.
    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() = default;
        explicit iterator(pointer position)
            : position_(position)
        {}

        iterator& operator++()
        {
            position_++;
            return *this;
        }

        iterator operator++(int)
        {
            auto temp = *this;
            ++*this;
            return temp;
        }

        iterator& operator--()
        {
            position_--;
            return *this;
        }

        iterator operator--(int)
        {
            auto temp = *this;
            --*this;
            return temp;
        }

        iterator& operator+=(difference_type diff)
        {
            position_ += diff;
            return *this;
        }

        iterator& operator-=(difference_type diff)
        {
            position_ -= diff;
            return *this;
        }

        iterator operator+(difference_type diff) const { return position_ + diff; }
        friend iterator operator+(difference_type diff, iterator iter) { return iter + diff; }
        iterator operator-(difference_type diff) const { return position_ - diff; }

        difference_type operator-(iterator other) const { return position_ - other.position_; }

        reference operator[](difference_type diff) const { return position_ + diff; }

        bool operator==(iterator other) const { return position_ == other.position_; }
        bool operator!=(iterator other) const { return !(*this == other); }
        bool operator<(iterator other) const { return position_ < other.position_; }
        bool operator<=(iterator other) const { return !(*this > other); }
        bool operator>(iterator other) const { return other < *this; }
        bool operator>=(iterator other) const { return !(*this < other); }

        reference operator*() const { return *position_; }
        pointer operator->() const { return position_; }

    private:
        T* position_ = nullptr;
    };

    /// @brief Maps and locks the given VBO to stay bound, as only one VBO can be mapped at any given time.
    VBOMapping(VBO<T>& vbo)
        : size_(vbo.count())
        , data_(empty() ? nullptr : static_cast<T*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE)))
    {}

    VBOMapping(const VBOMapping&) = delete;

    VBOMapping(VBOMapping&& other)
        : size_(other.size_)
        , data_(std::exchange(other.data_, nullptr))
    {}

    VBOMapping& operator=(const VBOMapping&) = delete;

    VBOMapping& operator=(VBOMapping&& other)
    {
        unmap();
        size_ = other.size_;
        data_ = std::exchange(other.data_, nullptr);
        return *this;
    }

    ~VBOMapping() { unmap(); }

    /// @brief Unmaps and unlocks the VBO again.
    void unmap()
    {
        if (data_) {
            glUnmapBuffer(GL_ARRAY_BUFFER);
            data_ = nullptr;
        }
    }

    /// @brief Returns the element count of the VBO.
    std::size_t size() const { return size_; }
    /// @brief Returns the element count of the VBO.
    std::size_t max_size() const { return size(); }
    /// @brief Whether the VBO is empty.
    bool empty() const { return size() == 0; }

    /// @brief Returns an iterator to the first element of the mapped data.
    iterator begin() noexcept { return iterator(data_); }
    /// @brief Returns an iterator to one after the last element of the mapped data.
    iterator end() noexcept { return iterator(data_ + size()); }

private:
    std::size_t size_;
    T* data_;
    // TODO: VBOLock<T> lock_{ vbo_ };
};

/// @brief A vertex buffer object for a given data struct.
template <typename T>
class VBO : public BufferBase<BufferTarget::ArrayBuffer> {
public:
    static_assert(std::is_standard_layout_v<T>, "VBO-Data must be a standard-layout type");

    VBO() = default;

    VBO(EmptyObject)
        : BufferBase<BufferTarget::ArrayBuffer>(empty_object)
    {}

    ~VBO() = default;

    VBO(const VBO&) = delete;
    VBO(VBO&&) = default;
    VBO& operator=(const VBO&) = delete;
    VBO& operator=(VBO&&) = default;

    /// @brief Returns the element count of the buffer.
    GLsizei count() const { return count_; }

    /// @brief Creates new data from the given element count and data pointer.
    void generate(GLsizei count, const T* data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        bind();
        count_ = count;
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), data, toGLConstant(usage));
    }

    /// @brief Creates new uninitialized data for a given number of elements.
    void generate(GLsizei count, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(count, nullptr, usage);
    }

    /// @brief Creates new uninitialized data for a given number of elements.
    template <std::size_t v_count>
    void generate(BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(v_count, usage);
    }

    /// @brief Creates new data from the given initializer list.
    void generate(std::initializer_list<T> data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(data.size()), data.begin(), usage);
    }

    /// @brief Creates new data from the given std::vector.
    void generate(const std::vector<T>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(data.size()), data.data(), usage);
    }

    /// @brief Creates new data from the given std::vector iterator.
    void generate(typename std::vector<T>::const_iterator begin,
                  typename std::vector<T>::const_iterator end,
                  BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(count), &*begin, usage);
    }

    /// @brief Creates new data from the given C-Style array.
    template <GLsizei v_size>
    void generate(const T (&data)[v_size], BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(v_size, data, usage);
    }

    /// @brief Creates new data from the given std::array.
    template <GLsizei v_size>
    void generate(const std::array<T, v_size>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(v_size, data.data(), usage);
    }

    /// @brief Creates new data from the given std::array iterator.
    template <GLsizei v_size>
    void generate(typename std::array<T, v_size>::const_iterator begin,
                  typename std::array<T, v_size>::const_iterator end,
                  BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(count), &*begin, usage);
    }

    /// @brief Modifies the existing buffer at the given range with the given data pointer.
    void modify(GLsizei offset, GLsizei count, const T* data)
    {
        bind();
        glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(T), count * sizeof(T), data);
    }

    /// @brief Modifies the existing buffer at the given position with the given initializer list.
    void modify(GLsizei offset, std::initializer_list<T> data)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(data.size()), data.begin());
    }

    /// @brief Modifies the existing buffer at the given position with the given std::vector.
    void modify(GLsizei offset, const std::vector<T>& data)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(data.size()), data.data());
    }

    /// @brief Modifies the existing buffer at the given position with the given std::vector iterators.
    void modify(GLsizei offset,
                typename std::vector<T>::const_iterator begin,
                typename std::vector<T>::const_iterator end)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(count), &*begin);
    }

    /// @brief Modifies the existing buffer at the given position with the given C-Style array.
    template <GLsizei v_size>
    void modify(GLsizei offset, const T (&data)[v_size])
    {
        modify(offset, v_size, data);
    }

    /// @brief Modifies the existing buffer at the given position with the given std::array.
    template <GLsizei v_size>
    void modify(GLsizei offset, const std::array<T, v_size>& data)
    {
        modify(offset, v_size, data.data());
    }

    /// @brief Modifies the existing buffer at the given position with the given std::array iterators.
    template <GLsizei v_size>
    void modify(GLsizei offset,
                typename std::array<T, v_size>::const_iterator begin,
                typename std::array<T, v_size>::const_iterator end)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(count), &*begin);
    }

    /// @brief Maps the buffer and returns a container-like wrapper to the mapping.
    VBOMapping<T> map() { return VBOMapping<T>(*this); }

    /// @brief Updates the entire buffer with the given elements and to_data function.
    template <typename TElements, typename TToData>
    void update(const TElements& elements, TToData&& to_data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        using std::begin, std::end;
        auto size = std::size(elements);
        assert(size <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(size), usage);
        std::transform(begin(elements), end(elements), map().begin(), std::forward<TToData>(to_data));
    }

private:
    GLsizei count_ = 0;
};

} // namespace dang::gl

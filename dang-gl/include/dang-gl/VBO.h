#pragma once

#include "Object.h"

namespace dang::gl
{

enum class BufferUsageHint : GLenum {
    StreamDraw = GL_STREAM_DRAW,
    StreamRead = GL_STREAM_READ,
    StreamCopy = GL_STREAM_COPY,
    StaticDraw = GL_STATIC_DRAW,
    StaticRead = GL_STATIC_READ,
    StaticCopy = GL_STATIC_COPY,
    DynamicDraw = GL_DYNAMIC_DRAW,
    DynamicRead = GL_DYNAMIC_READ,
    DynamicCopy = GL_DYNAMIC_COPY
};

class VBOBindError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class VBOBinding : public Binding {
public:
    template <class TInfo>
    void bind(const ObjectBase& object)
    {
        if (lock_count_ > 0)
            throw VBOBindError("The current VBO is locked and cannot be rebound.");
        Binding::bind<TInfo>(object);
    }

    void lock()
    {
        lock_count_++;
    }

    void unlock()
    {
        assert(lock_count_ > 0);
        lock_count_--;
    }

private:
    int lock_count_ = 0;
};

struct VBOInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr BindingPoint BindingPoint = BindingPoint::ArrayBuffer;

    using Binding = VBOBinding;
};

template <typename T>
class VBO;

template <typename T>
class VBOMapping {
public:

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

    VBOMapping(VBO<T>& vbo)
        : vbo_(vbo)
    {
        vbo_.bind();
        vbo_.binding().lock();
        data_ = static_cast<T*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    }

    ~VBOMapping()
    {
        vbo_.binding().unlock();
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    VBOMapping(const VBOMapping&) = delete;
    VBOMapping(VBOMapping&&) = delete;
    VBOMapping& operator=(const VBOMapping&) = delete;
    VBOMapping& operator=(VBOMapping&&) = delete;

    std::size_t size() const
    {
        return vbo_.count();
    }

    std::size_t max_size() const
    {
        return vbo_.count();
    }

    iterator begin() noexcept
    {
        return iterator(data_);
    }

    iterator end() noexcept
    {
        return iterator(data_ + size());
    }

private:
    VBO<T>& vbo_;
    T* data_;
};

template <typename T>
class VBO : public Object<VBOInfo> {
public:
    static_assert(std::is_standard_layout_v<T>, "VBO-Data must be a standard-layout type");

    GLsizei count() const
    {
        return count_;
    }

    void generate(GLsizei count, const T* data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        bind();
        count_ = count;
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), data, static_cast<GLenum>(usage));
    }

    void generate(GLsizei count, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(count, nullptr, usage);
    }

    template <std::size_t Size>
    void generate(BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Size, usage);
    }

    void generate(std::initializer_list<T> data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(data.size()), data.begin(), usage);
    }

    void generate(const std::vector<T>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(data.size()), data.data(), usage);
    }

    void generate(typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(count), &*begin, usage);
    }

    template <GLsizei Size>
    void generate(const T(&data)[Size], BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Size, data, usage);
    }

    template <GLsizei Size>
    void generate(const std::array<T, Size>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Size, data.data(), usage);
    }

    template <GLsizei Size>
    void generate(typename std::array<T, Size>::const_iterator begin, typename std::array<T, Size>::const_iterator end, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        generate(static_cast<GLsizei>(count), &*begin, usage);
    }

    void modify(GLsizei offset, GLsizei count, const T* data)
    {
        bind();
        glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(T), count * sizeof(T), data);
    }

    void modify(GLsizei offset, std::initializer_list<T> data)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(data.size()), data.begin());
    }

    void modify(GLsizei offset, const std::vector<T>& data)
    {
        assert(data.size() <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(data.size()), data.data());
    }

    void modify(GLsizei offset, typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(count), &*begin);
    }

    template <GLsizei Size>
    void modify(GLsizei offset, const T(&data)[Size])
    {
        modify(offset, Size, data);
    }

    template <GLsizei Size>
    void modify(GLsizei offset, const std::array<T, Size>& data)
    {
        modify(offset, Size, data.data());
    }

    template <GLsizei Size>
    void modify(GLsizei offset, typename std::array<T, Size>::const_iterator begin, typename std::array<T, Size>::const_iterator end)
    {
        const auto count = std::distance(begin, end);
        assert(count >= 0 && count <= static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()));
        modify(offset, static_cast<GLsizei>(count), &*begin);
    }

    VBOMapping<T> map()
    {
        return VBOMapping<T>(*this);
    }

private:
    GLsizei count_ = 0;
};

}

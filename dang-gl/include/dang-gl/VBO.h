#pragma once

#include "Object.h"

#include <vector>
#include <array>
#include <initializer_list>

#include "dang-utils/NonCopyable.h"

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
    void bind(ObjectBase* object)
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
class VBOMapping : dutils::NonCopyable {
public:

    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        inline iterator() = default;
        inline explicit iterator(pointer position) : position_(position) {}

        inline reference operator*()
        {
            return *position_;
        }

        inline pointer operator->()
        {
            return position_;
        }

        inline friend bool operator==(iterator lhs, iterator rhs)
        {
            return lhs.position_ == rhs.position_;
        }

        inline friend bool operator!=(iterator lhs, iterator rhs)
        {
            return lhs.position_ != rhs.position_;
        }

        inline friend bool operator<(iterator lhs, iterator rhs)
        {
            return lhs.position_ < rhs.position_;
        }

        inline friend bool operator<=(iterator lhs, iterator rhs)
        {
            return lhs.position_ <= rhs.position_;
        }

        inline friend bool operator>(iterator lhs, iterator rhs)
        {
            return lhs.position_ > rhs.position_;
        }

        inline friend bool operator>=(iterator lhs, iterator rhs)
        {
            return lhs.position_ >= rhs.position_;
        }

        inline iterator& operator++()
        {
            position_++;
            return *this;
        }

        inline iterator operator++(int)
        {
            auto old = *this;
            position_++;
            return old;
        }

        inline iterator& operator--()
        {
            position_--;
            return *this;
        }

        inline iterator operator--(int)
        {
            auto old = *this;
            position_--;
            return old;
        }

        inline iterator& operator+=(std::ptrdiff_t offset)
        {
            position_ += offset;
            return *this;
        }

        inline iterator operator+(std::ptrdiff_t offset) const
        {
            return *this += offset;
        }

        inline iterator& operator-=(std::ptrdiff_t offset)
        {
            position_ -= offset;
            return *this;
        }

        inline iterator operator-(std::ptrdiff_t offset) const
        {
            return *this -= offset;
        }

        inline reference operator[](std::ptrdiff_t offset) const
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

    std::size_t count() const
    {
        return count_;
    }

    void generate(std::size_t count, const T* data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        bind();
        count_ = count;
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), data, static_cast<GLenum>(usage));
    }

    void generate(std::size_t count, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
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
        generate(data.size(), data.begin(), usage);
    }

    void generate(const std::vector<T>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(data.size(), data.data(), usage);
    }

    void generate(typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(std::distance(begin, end), &*begin, usage);
    }

    template <std::size_t Size>
    void generate(const T(&data)[Size], BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Size, data, usage);
    }

    template <std::size_t Size>
    void generate(const std::array<T, Size>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(Size, data.data(), usage);
    }

    template <std::size_t Size>
    void generate(typename std::array<T, Size>::const_iterator begin, typename std::array<T, Size>::const_iterator end, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        generate(std::distance(begin, end), &*begin, usage);
    }

    void modify(std::size_t offset, std::size_t count, const T* data)
    {
        bind();
        glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(T), count * sizeof(T), data);
    }

    void modify(std::size_t offset, std::initializer_list<T> data)
    {
        modify(offset, data.size(), data.begin());
    }

    void modify(std::size_t offset, const std::vector<T>& data)
    {
        modify(offset, data.size(), data.data());
    }

    void modify(std::size_t offset, typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end)
    {
        modify(offset, std::distance(begin, end), &*begin);
    }

    template <std::size_t Size>
    void modify(std::size_t offset, const T(&data)[Size])
    {
        modify(offset, Size, data);
    }

    template <std::size_t Size>
    void modify(std::size_t offset, const std::array<T, Size>& data)
    {
        modify(offset, Size, data.data());
    }

    template <std::size_t Size>
    void modify(std::size_t offset, typename std::array<T, Size>::const_iterator begin, typename std::array<T, Size>::const_iterator end)
    {
        modify(offset, std::distance(begin, end), &*begin);
    }

    VBOMapping<T> map()
    {
        return VBOMapping<T>(*this);
    }

private:
    std::size_t count_ = 0;
};

}

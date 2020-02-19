#pragma once

#include "Object.h"

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

enum class BufferAccess : GLenum {
    ReadOnly = GL_READ_ONLY,
    WriteOnly = GL_WRITE_ONLY,
    ReadWrite = GL_READ_WRITE
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
        lock_count_--;
        assert(lock_count_ >= 0);
    }

private:
    int lock_count_ = 0;
};

struct VBOInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr ObjectType Type = ObjectType::Buffer;

    using Binding = VBOBinding;
};

class VBOBase : public Object<VBOInfo> {

};

template <typename T>
class VBO;

template <typename T, BufferAccess Access>
class VBOMapping : dutils::NonCopyable {
public:

    class const_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;
        explicit const_iterator(pointer position) : position_(position) {}

        friend bool operator==(const const_iterator& lhs, const const_iterator& rhs)
        {
            return lhs.position_ == rhs.position_;
        }

        friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        T* position_ = nullptr;
    };

    class iterator : public const_iterator {
    public:
        using pointer = T*;
        using reference = T&;

        iterator() = default;
        explicit iterator(pointer position) : const_iterator(position) {}
    };

    VBOMapping(VBO<T>& vbo)
        : vbo_(vbo)
    {
        vbo_.bind();
        vbo_.binding().lock();
        data_ = reinterpret_cast<T*>(glMapBuffer(GL_ARRAY_BUFFER, static_cast<GLenum>(Access)));
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
        static_assert(Access != BufferAccess::ReadOnly);
        return iterator(data_);
    }

    iterator end() noexcept
    {
        static_assert(Access != BufferAccess::ReadOnly);
        return iterator(data_ + size());
    }

    iterator begin() const noexcept
    {
        return const_iterator(data_);
    }

    iterator end() const noexcept
    {
        return const_iterator(data_ + size());
    }

    iterator cbegin() const noexcept
    {
        return const_iterator(data_);
    }

    iterator cend() const noexcept
    {
        return const_iterator(data_ + size());
    }

private:
    VBO<T>& vbo_;
    T* data_;
};

template <typename T>
class VBO : public VBOBase {
    static_assert(std::is_standard_layout_v<T>, "VBO-Data must be a standard-layout type");

public:
    std::size_t count() const
    {
        return count_;
    }

    void generate(std::size_t count, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        bind();
        count_ = count;
        glBufferData(GL_ARRAY_BUFFER, count_ * sizeof(T), nullptr, static_cast<GLenum>(usage));
    }

    void generate(const std::vector<T>& data, BufferUsageHint usage = BufferUsageHint::DynamicDraw)
    {
        bind();
        count_ = data.size();
        glBufferData(GL_ARRAY_BUFFER, count_ * sizeof(T), data.data(), usage);
    }

    VBOMapping<T, BufferAccess::ReadOnly> mapRead()
    {
        return VBOMapping<T, BufferAccess::ReadOnly>(*this);
    }

    VBOMapping<T, BufferAccess::WriteOnly> mapWrite()
    {
        return VBOMapping<T, BufferAccess::WriteOnly>(*this);
    }

    VBOMapping<T, BufferAccess::ReadWrite> map()
    {
        return VBOMapping<T, BufferAccess::ReadWrite>(*this);
    }

private:
    std::size_t count_ = 0;
};

}

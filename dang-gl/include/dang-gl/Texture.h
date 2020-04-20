#pragma once

#include "Object.h"
#include "ObjectBinding.h"

namespace dang::gl
{

/*

The concept of glActiveTexture and glBindTexture

Quote Khronos.org:
  "Binding textures for use in OpenGL is a little weird."

- There are a set number of texture slots, whose count can be queried using GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
- glActiveTexture sets the current slot to use, using GL_TEXTUREi
- glBindTexture binds a texture to that currently active slot
- Even though different texture types can be bound at the same time...
  -> It is NOT possible to bind different types of textures to the same active texture slot
  -> The spec explicitly disallows this
- The active slot also identifies the texture in a shader sampler

*/

/// <summary>An error related to textures.</summary>
class TextureError : public std::runtime_error {
    using runtime_error::runtime_error;
};

class TextureBase;

// This current implementation is easy to use, but only allows a texture to be bound a single slot
// Possibly consider modification, to allow a texture to be bound for multiple slots, as the spec does technically allow this
// -> This greatly complicates everything and might not be worth the cost (both run-time and possibly ease-of-use)

/// <summary>The context for all different texture types, as binding closely related all different types.</summary>
class TextureContext : public ObjectContext {
public:
    using ObjectContext::ObjectContext;

    /// <summary>Returns the currently active texture unit, in other words the active GL_TEXTUREi.</summary>
    GLenum activeTexture();
    /// <summary>Sets the currently active texture unit to the given GL_TEXTUREi.</summary>
    void setActiveTexture(GLenum active_texture);

    /// <summary>Binds the texture to the first free slot or throws a TextureError, if all slots are occupied.</summary>
    void bind(const TextureBase& texture);
    /// <summary>If the texture is currently bound to a slot, makes that slot free for another texture to use.</summary>
    void release(const TextureBase& texture);
    /// <summary>Used for texture move semantics, to update the references of bound textures.</summary>
    void move(const TextureBase& from, const TextureBase& to);

private:
    GLenum active_texture_;
    std::vector<const TextureBase*> active_textures_{ window().state().max_combined_texture_image_units };
    std::vector<const TextureBase*>::iterator first_free_slot_ = active_textures_.begin();
};

/// <summary>As texture types are closely related, this simply relays to the global texture context.</summary>
template <BindingPoint TextureBindingPoint>
class TextureBinding : public ObjectBindingBase {
public:
    /// <summary>Initializes the binding with the given context.</summary>
    TextureBinding(TextureContext& context)
        : context_(context)
    {
    }

    /// <summary>Returns the associated context.</summary>
    TextureContext& context()
    {
        return context_;
    }

    /// <summary>Relays to the context, effectively binding the texture to the first free slot.</summary>
    template <typename TInfo>
    void bind(const TextureBase& object)
    {
        context_.bind(object);
    }

    /// <summary>Relays to the context, effectively freeing up the slot, that the texture is currently occupying.</summary>
    void release(const TextureBase& object)
    {
        context_.release(object);
    }

    /// <summary>Relays to the context, updating a potentially bound texture.</summary>
    template <typename TInfo>
    void move(const TextureBase& from, const TextureBase& to)
    {
        context_.move(from, to);
    }

private:
    TextureContext& context_;
};

/// <summary>Info struct to create and destroy textures.</summary>
template <BindingPoint TextureBindingPoint>
struct TextureInfo : public ObjectInfo {
    static GLuint create()
    {
        GLuint handle;
        glGenTextures(1, &handle);
        return handle;
    }

    static void destroy(GLuint handle)
    {
        glDeleteTextures(1, &handle);
    }

    using Context = TextureContext;
    static constexpr ObjectType ObjectType = ObjectType::Texture;

    using Binding = TextureBinding<TextureBindingPoint>;
    static constexpr BindingPoint BindingPoint = TextureBindingPoint;
};

class TextureBase : public ObjectBase {
public:
    friend TextureContext;

    std::optional<GLenum> activeTextureSlot();

protected:
    TextureBase(GLuint handle, Window& window, BindingPoint binding_point)
        : ObjectBase(handle, window)
        , binding_point_(binding_point)
    {
    }

private:
    BindingPoint binding_point_;
    mutable std::optional<GLenum> active_texture_slot_;
};

/// <summary>A texture of the template specified type.</summary>
template <BindingPoint TextureBindingPoint>
class Texture : public Object<TextureInfo<TextureBindingPoint>, TextureBase> {
public:
    /// <summary>Creates a new texture of the specified type.</summary>
    Texture()
        : Object<TextureInfo<TextureBindingPoint>, TextureBase>(TextureBindingPoint)
    {
    }

    /// <summary>If the texture is currently bound to a slot, makes that slot free for another texture to use.</summary>
    void release()
    {
        this->binding().release(*this);
    }
};

using Texture1D = Texture<BindingPoint::Texture1D>;
using Texture1DArray = Texture<BindingPoint::Texture1DArray>;
using Texture2D = Texture<BindingPoint::Texture2D>;
using Texture2DArray = Texture<BindingPoint::Texture2DArray>;
using Texture2DMultisample = Texture<BindingPoint::Texture2DMultisample>;
using Texture2DMultisampleArray = Texture<BindingPoint::Texture2DMultisampleArray>;
using Texture3D = Texture<BindingPoint::Texture3D>;
using TextureCubeMap = Texture<BindingPoint::TextureCubeMap>;
using TextureRectangle = Texture<BindingPoint::TextureRectangle>;

}

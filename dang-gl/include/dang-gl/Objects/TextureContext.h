#pragma once

#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief An error related to textures.
class TextureError : public std::runtime_error {
    using runtime_error::runtime_error;
};

// This current implementation is easy to use, but only allows a texture to be bound to a single slot
// Possibly consider modification, to allow a texture to be bound for multiple slots, as the spec does technically allows this
// -> This greatly complicates everything and might not be worth the cost (both run-time and possibly ease-of-use)

/// @brief Specializes the context class for texture objects.
template <>
class ObjectContext<ObjectType::Texture> : public ObjectContextBase {
public:
    using Handle = ObjectHandle<ObjectType::Texture>;
    using Wrapper = ObjectWrapper<ObjectType::Texture>;

    using ObjectContextBase::ObjectContextBase;

    /// @brief Returns the currently active texture slot.
    std::size_t activeSlot();
    /// @brief Sets the currently active texture slot.
    void setActiveSlot(std::size_t slot);

    /// @brief Binds the texture to the first free slot and returns it or throws a TextureError, if all slots are occupied.
    std::size_t bind(TextureTarget target, Handle handle, std::optional<std::size_t> active_slot);
    /// @brief If the texture is currently bound to a slot, makes that slot free for another texture to use.
    void release(TextureTarget target, std::optional<std::size_t> active_slot);

private:
    std::size_t active_slot_;
    // avoid accidental list initialization
    std::vector<Handle> active_textures_ = std::vector<Handle>(context()->max_combined_texture_image_units);
    std::vector<Handle>::iterator first_free_slot_ = active_textures_.begin();
};

inline std::size_t ObjectContext<ObjectType::Texture>::activeSlot() { return active_slot_; }

inline void ObjectContext<ObjectType::Texture>::setActiveSlot(std::size_t active_slot)
{
    if (active_slot_ == active_slot)
        return;
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + active_slot));
    active_slot_ = active_slot;
}

inline std::size_t ObjectContext<ObjectType::Texture>::bind(TextureTarget target,
                                                            Handle handle,
                                                            std::optional<std::size_t> active_slot)
{
    if (active_slot) {
        setActiveSlot(*active_slot);
        return *active_slot;
    }
    if (first_free_slot_ == active_textures_.end())
        throw TextureError("Cannot bind texture, as all slots are in use.");
    std::size_t slot = static_cast<std::size_t>(std::distance(active_textures_.begin(), first_free_slot_));
    setActiveSlot(slot);
    Wrapper::bind(target, handle);
    *first_free_slot_ = handle;
    first_free_slot_ = std::find(std::next(first_free_slot_), active_textures_.end(), Handle{});
    return slot;
}

inline void ObjectContext<ObjectType::Texture>::release(TextureTarget target, std::optional<std::size_t> active_slot)
{
    if (!active_slot)
        return;
    setActiveSlot(*active_slot);
    Wrapper::bind(target, {});
    auto texture_to_free = std::next(active_textures_.begin(), static_cast<std::size_t>(*active_slot));
    *texture_to_free = {};
    if (texture_to_free < first_free_slot_)
        first_free_slot_ = texture_to_free;
}

} // namespace dang::gl

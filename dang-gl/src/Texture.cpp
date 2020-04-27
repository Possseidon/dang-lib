#include "pch.h"
#include "Texture.h"

namespace dang::gl
{

GLint TextureContext::activeSlot()
{
    return active_slot_;
}

void TextureContext::setActiveSlot(GLint active_slot)
{
    if (active_slot_ == active_slot)
        return;
    glActiveTexture(GL_TEXTURE0 + active_slot);
    active_slot_ = active_slot;
}

GLint TextureContext::bind(const TextureBase& texture)
{
    if (texture.active_slot_) {
        setActiveSlot(*texture.active_slot_);
        return *texture.active_slot_;
    }
    if (first_free_slot_ == active_textures_.end())
        throw TextureError("Cannot bind texture, as all slots are in use.");
    GLint slot = static_cast<GLint>(std::distance(active_textures_.begin(), first_free_slot_));
    setActiveSlot(slot);
    glBindTexture(toGLConstant(texture.binding_point_), texture.handle());
    *first_free_slot_ = &texture;
    texture.active_slot_ = slot;
    first_free_slot_ = std::find(std::next(first_free_slot_), active_textures_.end(), nullptr);
    return slot;
}

void TextureContext::release(const TextureBase& texture)
{
    if (!texture.active_slot_)
        return;
    auto texture_to_free = std::next(active_textures_.begin(), static_cast<std::size_t>(*texture.active_slot_) - GL_TEXTURE0);
    *texture_to_free = nullptr;
    texture.active_slot_ = std::nullopt;
    if (texture_to_free < first_free_slot_)
        first_free_slot_ = texture_to_free;
}

void TextureContext::move(const TextureBase& from, const TextureBase& to)
{
    if (!from.active_slot_)
        return;
    auto texture_to_move = std::next(active_textures_.begin(), static_cast<std::size_t>(*from.active_slot_) - GL_TEXTURE0);
    *texture_to_move = &to;
}

}

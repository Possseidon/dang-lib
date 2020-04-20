#include "pch.h"
#include "Texture.h"

namespace dang::gl
{

GLenum TextureContext::activeTexture()
{
    return active_texture_;
}

void TextureContext::setActiveTexture(GLenum active_texture)
{
    if (active_texture_ == active_texture)
        return;
    glActiveTexture(active_texture);
    active_texture_ = active_texture;
}

void TextureContext::bind(const TextureBase& texture)
{
    if (texture.active_texture_slot_)
        return;
    if (first_free_slot_ == active_textures_.end())
        throw TextureError("Cannot bind texture, as all slots are in use.");
    GLenum slot = GL_TEXTURE0 + static_cast<GLenum>(std::distance(active_textures_.begin(), first_free_slot_));
    setActiveTexture(slot);
    glBindTexture(BindingPointTargets[texture.binding_point_], texture.handle());
    *first_free_slot_ = &texture;
    texture.active_texture_slot_ = slot;
    first_free_slot_ = std::find(std::next(first_free_slot_), active_textures_.end(), nullptr);
}

void TextureContext::release(const TextureBase& texture)
{
    if (!texture.active_texture_slot_)
        return;
    auto texture_to_free = std::next(active_textures_.begin(), *texture.active_texture_slot_ - GL_TEXTURE0);
    *texture_to_free = nullptr;
    texture.active_texture_slot_ = std::nullopt;
    if (texture_to_free < first_free_slot_)
        first_free_slot_ = texture_to_free;
}

void TextureContext::move(const TextureBase& from, const TextureBase& to)
{
    if (!from.active_texture_slot_)
        return;
    auto texture_to_move = std::next(active_textures_.begin(), *from.active_texture_slot_ - GL_TEXTURE0);
    *texture_to_move = &to;
}

std::optional<GLenum> TextureBase::activeTextureSlot()
{
    return active_texture_slot_;
}

}

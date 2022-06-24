#pragma once

#include "dang-gl/global.h"
#include "dang-math/vector.h"
#include "dang-utils/utils.h"

namespace dang::gl {

/// @brief Calculates the number of required mipmap levels for the given texture size.
constexpr std::size_t maxMipmapLevels(std::size_t size) { return dutils::ilog2ceil(size) + 1; }

/// @brief Returns the next mipmap level size.
/// @remarks Rounds up, which is unusual for mipmaps, but necessary when used in a texture atlas.
constexpr dmath::svec2 nextMipmapSize(const dmath::svec2& size) { return (size.maxValue() - 1) / 2 + 1; }

/// @brief Combines several mipmap levels of the same image.
template <typename TBorderedImageData>
class MipmapLevels {
public:
    using BorderedImageData = TBorderedImageData;

    /// @brief Only stores the given image without generating any additional mipmaps.
    MipmapLevels(BorderedImageData full_image)
        : mipmap_levels_{std::move(full_image)}
    {}

    /// @brief Stores the given bordered image and all mipmap levels using the provided mipmapper.
    template <typename TMipmapper>
    MipmapLevels(BorderedImageData full_image, TMipmapper mipmapper)
        : mipmap_levels_(generateMipmapLevels(std::move(full_image), mipmapper))
    {}

    /// @brief The full image with the highest resolution.
    auto& fullImage() { return mipmap_levels_.front(); }
    /// @brief The full image with the highest resolution.
    const auto& fullImage() const { return mipmap_levels_.front(); }

    /// @brief The total number of mipmap levels, including the original, full size image.
    auto count() const { return mipmap_levels_.size(); }

    /// @brief A specific mipmap level with the given index, where 0 gives the original, full size image.
    auto& operator[](std::size_t index) { return mipmap_levels_[index]; }
    /// @brief A specific mipmap level with the given index, where 0 gives the original, full size image.
    const auto& operator[](std::size_t index) const { return mipmap_levels_[index]; }

    auto begin() { return mipmap_levels_.begin(); }
    auto begin() const { return mipmap_levels_.begin(); }
    auto end() { return mipmap_levels_.end(); }
    auto end() const { return mipmap_levels_.end(); }

private:
    /// @brief Uses the given mipmapper to generate a vector of all mipmap levels for the given image.
    template <typename TMipmapper>
    static auto generateMipmapLevels(BorderedImageData&& full_image, TMipmapper mipmapper)
    {
        auto count = maxMipmapLevels(full_image.size().maxValue());
        std::vector<BorderedImageData> mipmap_levels{std::move(full_image)};
        for (std::size_t index = 1; index < count; index++) {
            const auto& prev = mipmap_levels.back();
            auto mipmapped = mipmapper(prev);
            ensureSizeHalved(prev.size(), mipmapped.size());
            mipmap_levels.push_back(std::move(mipmapped));
        }
        return mipmap_levels;
    }

    /// @brief Throws an exception if "halved" isn't the correct next mipmap level size for "original".
    static void ensureSizeHalved(dmath::svec2 original, dmath::svec2 halved)
    {
        if (halved != nextMipmapSize(original))
            throw std::invalid_argument("mipmapper did not properly half the image size");
    }

    std::vector<BorderedImageData> mipmap_levels_;
};

} // namespace dang::gl

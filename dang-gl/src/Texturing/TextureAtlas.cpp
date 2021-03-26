#include "dang-gl/Texturing/TextureAtlas.h"

namespace dang::gl {

TextureAtlas::TilePlacement::TilePlacement(std::size_t index, svec2 position, GLsizei layer)
    : index(index)
    , position(position.x(), position.y(), layer)
{}

TextureAtlas::TileData::TileData(std::string&& name, Image2D&& image, TileBorderGeneration border)
    : name(std::move(name))
    , image(std::move(image))
    , border(border)
{}

TextureAtlas::TileData::~TileData()
{
    for (auto& handle : handles)
        handle->data_ = nullptr;
    // handle->reset() also removes the entry from "handles"
    // just set data_ to nullptr instead manually
}

TextureAtlas::Layer::Layer(const svec2& tile_size_log2, std::size_t max_texture_size)
    : tile_size_log2_(tile_size_log2)
    , max_tiles_(calculateMaxTiles(max_texture_size))
{}

svec2 TextureAtlas::Layer::tileSizeLog2() const { return tile_size_log2_; }

svec2 TextureAtlas::Layer::tileSize() const
{
    return {GLsizei{1} << tile_size_log2_.x(), GLsizei{1} << tile_size_log2_.y()};
}

GLsizei TextureAtlas::Layer::requiredGridSizeLog2() const
{
    if (tiles_.empty())
        return 0;
    auto diff_log2 = std::abs(tile_size_log2_.x() - tile_size_log2_.y());
    auto square_tiles = ((tiles_.size() - 1) >> diff_log2) + 1;
    return (dutils::ilog2ceil(square_tiles) + 1) >> 1;
}

GLsizei TextureAtlas::Layer::requiredTextureSize() const { return tileSize().maxValue() << requiredGridSizeLog2(); }

bool TextureAtlas::Layer::full() const { return first_free_tile_ == max_tiles_; }

void TextureAtlas::Layer::addTile(TileData& tile, GLsizei layer)
{
    assert(!full());
    tile.placement = TilePlacement(first_free_tile_, tileSize() * indexToPosition(first_free_tile_), layer);

    // find next free tile
    if (first_free_tile_ < tiles_.size()) {
        auto first = begin(tiles_) + first_free_tile_ + 1;
        auto iter = std::find(first, end(tiles_), &tile);
        first_free_tile_ = static_cast<std::size_t>(std::distance(begin(tiles_), iter));
    }
    else {
        tiles_.push_back(&tile);
        first_free_tile_++;
    }
}

void TextureAtlas::Layer::removeTile(TileData& tile)
{
    auto index = tile.placement.index;
    first_free_tile_ = std::min(first_free_tile_, index);
    tiles_[index] = nullptr;
}

void TextureAtlas::Layer::drawTile(TileData& tile, Texture2DArray& texture) const
{
    const auto& image = tile.image;
    const auto& position = tile.placement.position;

    auto width = image.size().x();
    auto height = image.size().y();

    auto s_width = static_cast<GLsizei>(width);
    auto s_height = static_cast<GLsizei>(height);

    switch (tile.border) {
    case TileBorderGeneration::Positive: {
        // left top -> right bottom
        texture.modify(image[dmath::sbounds2({0, 0}, {1, 1})], position + svec3{s_width, s_height, 0}, 0);
        // left -> right
        texture.modify(image[dmath::sbounds2({0, 0}, {1, height})], position + svec3{s_width, 0, 0}, 0);
        // top -> bottom
        texture.modify(image[dmath::sbounds2({0, 0}, {width, 1})], position + svec3{0, s_height, 0}, 0);

        [[fallthrough]];
    }
    case TileBorderGeneration::None: {
        // full image
        texture.modify(image, position, 0);
        break;
    }
    case TileBorderGeneration::All: {
        // full image (offset by 1)
        texture.modify(image, position + svec3{1, 1, 0}, 0);

        // left top -> right bottom
        texture.modify(image[dmath::sbounds2({0, 0}, {1, 1})], position + svec3{s_width + 1, s_height + 1, 0}, 0);
        // right top -> left bottom
        texture.modify(image[dmath::sbounds2({width - 1, 0}, {width, 1})], position + svec3{0, s_height + 1, 0}, 0);
        // left bottom -> right top
        texture.modify(image[dmath::sbounds2({0, height - 1}, {1, height})], position + svec3{s_width + 1, 0, 0}, 0);
        // right bottom -> left top
        texture.modify(image[dmath::sbounds2({width - 1, height - 1}, {width, height})], position + svec3{0, 0, 0}, 0);

        // left -> right
        texture.modify(image[dmath::sbounds2({0, 0}, {1, height})], position + svec3{s_width + 1, 1, 0}, 0);
        // right -> left
        texture.modify(image[dmath::sbounds2({width - 1, 0}, {width, height})], position + svec3{0, 1, 0}, 0);
        // top -> bottom
        texture.modify(image[dmath::sbounds2({0, 0}, {width, 1})], position + svec3{1, s_height + 1, 0}, 0);
        // bottom -> top
        texture.modify(image[dmath::sbounds2({0, height - 1}, {width, height})], position + svec3{1, 0, 0}, 0);

        break;
    }
    default:
        assert(false);
    }
    tile.placement.written = true;
}

std::size_t TextureAtlas::Layer::calculateMaxTiles(std::size_t max_texture_size) const
{
    assert(tileSize().maxValue() <= max_texture_size);
    auto x_tiles = std::size_t{1} << dutils::ilog2(max_texture_size >> tile_size_log2_.x());
    auto y_tiles = std::size_t{1} << dutils::ilog2(max_texture_size >> tile_size_log2_.y());
    return x_tiles * y_tiles;
}

void TextureAtlas::Layer::drawTiles(Texture2DArray& texture) const
{
    for (auto tile : tiles_) {
        if (tile == nullptr)
            continue;
        if (tile->placement.written)
            continue;
        drawTile(*tile, texture);
    }
}

svec2 TextureAtlas::Layer::indexToPosition(std::size_t index)
{
    auto size_diff_log2 = std::abs(tile_size_log2_.x() - tile_size_log2_.y());
    auto flip = tile_size_log2_.x() < tile_size_log2_.y();
    auto x = static_cast<GLsizei>(dutils::removeOddBits(index >> size_diff_log2));
    auto y = static_cast<GLsizei>(dutils::removeOddBits(index >> (size_diff_log2 + 1)));
    y <<= size_diff_log2;
    y |= index & ~(~std::size_t{0} << size_diff_log2);
    return flip ? svec2(y, x) : svec2(x, y);
}

std::size_t TextureAtlas::Layer::positionToIndex(svec2 position)
{
    auto size_diff_log2 = std::abs(tile_size_log2_.x() - tile_size_log2_.y());
    auto flip = tile_size_log2_.x() < tile_size_log2_.y();
    position = flip ? position.yx() : position;
    auto result = position.y() & ~(~std::size_t{0} << size_diff_log2);
    position.y() >>= size_diff_log2;
    result |= dutils::interleaveZeros(static_cast<std::size_t>(position.x())) << size_diff_log2;
    result |= dutils::interleaveZeros(static_cast<std::size_t>(position.y())) << (size_diff_log2 + 1);
    return result;
}

TextureAtlas::TileHandle::~TileHandle() noexcept { reset(); }

TextureAtlas::TileHandle::TileHandle(const TileHandle& tile_handle)
    : TileHandle(tile_handle.data_)
{}

TextureAtlas::TileHandle::TileHandle(TileHandle&& tile_handle) noexcept
    : data_(tile_handle.data_)
{
    if (!tile_handle)
        return;
    // Just replace the existing entry.
    *tile_handle.find() = this;
    tile_handle.data_ = nullptr;
}

TextureAtlas::TileHandle& TextureAtlas::TileHandle::operator=(const TileHandle& tile_handle)
{
    reset();
    if (!tile_handle)
        return *this;
    data_ = tile_handle.data_;
    data_->handles.push_back(this);
    return *this;
}

TextureAtlas::TileHandle& TextureAtlas::TileHandle::operator=(TileHandle&& tile_handle) noexcept
{
    reset();
    if (!tile_handle)
        return *this;
    data_ = tile_handle.data_;
    // Just replace the existing entry.
    *tile_handle.find() = this;
    tile_handle.data_ = nullptr;
    return *this;
}

void TextureAtlas::TileHandle::reset() noexcept
{
    if (!*this)
        return;
    auto iter = find();
    assert(iter != end(data_->handles));
    data_->handles.erase(iter);
    data_ = nullptr;
}

TextureAtlas::TileHandle::operator bool() const noexcept { return data_ != nullptr; }

const std::string& TextureAtlas::TileHandle::name() const noexcept { return data_->name; }

TextureAtlas::TileData::TileHandles::iterator TextureAtlas::TileHandle::find() const
{
    return std::find(begin(data_->handles), end(data_->handles), this);
}

TextureAtlas::TileHandle::TileHandle(const TileData* data)
    : data_(data)
{
    if (data)
        data->handles.push_back(this);
}

TextureAtlas::TileBorderGeneration TextureAtlas::guessTileBorderGeneration(GLsizei size) const
{
    auto usize = static_cast<std::make_unsigned_t<GLsizei>>(size);
    if (dutils::popcount(usize) == 1)
        return TileBorderGeneration::None;
    if (dutils::popcount(usize + 1) == 1)
        return TileBorderGeneration::Positive;
    if (dutils::popcount(usize + 2) == 1)
        return TileBorderGeneration::All;
    return default_border_;
}

TextureAtlas::TileBorderGeneration TextureAtlas::guessTileBorderGeneration(svec2 size) const
{
    return guessTileBorderGeneration(size.maxValue());
}

GLsizei TextureAtlas::sizeWithBorder(GLsizei size, TileBorderGeneration border)
{
    switch (border) {
    case TileBorderGeneration::None:
        return size;
    case TileBorderGeneration::Positive:
        return size + 1;
    case TileBorderGeneration::All:
        return size + 2;
    default:
        assert(false);
        return 0;
    }
}

svec2 TextureAtlas::sizeWithBorder(svec2 size, TileBorderGeneration border)
{
    return {sizeWithBorder(size.x(), border), sizeWithBorder(size.y(), border)};
}

TextureAtlas::TileBorderGeneration TextureAtlas::defaultBorderGeneration() const { return default_border_; }

void TextureAtlas::setDefaultBorderGeneration(TileBorderGeneration border) { default_border_ = border; }

bool TextureAtlas::add(std::string name, Image2D image, std::optional<TileBorderGeneration> border)
{
    auto [iter, ok] = emplaceTile(std::move(name), std::move(image), border);
    return ok;
}

TextureAtlas::TileHandle TextureAtlas::addWithHandle(std::string name,
                                                     Image2D image,
                                                     std::optional<TileBorderGeneration> border)
{
    auto [tile, ok] = emplaceTile(std::move(name), std::move(image), border);
    return ok ? TileHandle(tile) : TileHandle();
}

bool TextureAtlas::exists(const std::string& name) const
{
    auto iter = tiles_.find(name);
    return iter != tiles_.end();
}

TextureAtlas::TileHandle TextureAtlas::operator[](const std::string& name) const
{
    auto iter = tiles_.find(name);
    if (iter != tiles_.end())
        return TileHandle(&iter->second);
    return TileHandle();
}

bool TextureAtlas::remove(const std::string& name)
{
    auto iter = tiles_.find(name);
    if (iter == tiles_.end())
        return false;
    tiles_.erase(iter);
    return true;

    // TODO: If the corresponding layer is empty after this, it can be removed.
    //       This causes all tiles in successive layers to decrease their layer and therefore get invalidated.
}

void TextureAtlas::generateTexture()
{
    ensureTextureSize();
    for (auto& layer : layers_)
        layer.drawTiles(texture_);
}

void TextureAtlas::ensureTextureSize()
{
    auto required_size = maxLayerSize();
    auto layers = static_cast<GLsizei>(layers_.size());
    // Texture width and height are always the same; only check width.
    if (required_size == texture_.size().x() && layers == texture_.size().z())
        return;
    texture_ = Texture2DArray({required_size, required_size, layers}, 1);
    for (auto& [name, tile] : tiles_)
        tile.placement.written = false;
}

GLsizei TextureAtlas::maxLayerSize() const
{
    GLsizei result = 0;
    for (auto& layer : layers_)
        result = std::max(result, layer.requiredTextureSize());
    return result;
}

std::pair<std::size_t, TextureAtlas::Layer*> TextureAtlas::layerForTile(const TileData& tile)
{
    auto size_with_border = sizeWithBorder(static_cast<svec2>(tile.image.size()), tile.border);
    auto unsigned_width = static_cast<std::make_unsigned_t<GLsizei>>(size_with_border.x());
    auto unsigned_height = static_cast<std::make_unsigned_t<GLsizei>>(size_with_border.y());
    auto tile_size_log2 = svec2(static_cast<GLsizei>(dutils::ilog2ceil(unsigned_width)),
                                static_cast<GLsizei>(dutils::ilog2ceil(unsigned_height)));
    auto layer_iter = std::find_if(begin(layers_), end(layers_), [&](const Layer& layer) {
        return !layer.full() && layer.tileSizeLog2() == tile_size_log2;
    });
    auto layer_index = std::distance(begin(layers_), layer_iter);
    if (layer_iter != end(layers_))
        return {layer_index, &*layer_iter};
    return {layer_index, &layers_.emplace_back(tile_size_log2, max_texture_size_)};
}

TextureAtlas::EmplaceResult TextureAtlas::emplaceTile(std::string&& name,
                                                      Image2D&& image,
                                                      std::optional<TileBorderGeneration> border)
{
    assert(image.size().lessThanEqual(std::numeric_limits<GLsizei>::max()).all());
    // Explicit copy to have two strings to move from.
    std::string key = name;
    auto [iter, ok] =
        tiles_.try_emplace(std::move(key),
                           std::move(name),
                           std::move(image),
                           border ? *border : guessTileBorderGeneration(static_cast<svec2>(image.size())));
    auto& tile = iter->second;
    if (ok) {
        auto [index, layer] = layerForTile(tile);
        layer->addTile(tile, index);
    }
    return {&tile, ok};
}

bool operator==(const TextureAtlas::TileHandle& lhs, const TextureAtlas::TileHandle& rhs) noexcept
{
    return lhs.data_ == rhs.data_;
}

bool operator!=(const TextureAtlas::TileHandle& lhs, const TextureAtlas::TileHandle& rhs) noexcept
{
    return !(lhs == rhs);
}

} // namespace dang::gl

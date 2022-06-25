#pragma once

#include <vector>

#include "dang-ecs/Entity.h"
#include "dang-ecs/global.h"

#include "boost/dynamic_bitset.hpp"

namespace dang::ecs {

// TODO: Different strategies:
// - Fully dense (current)
//   - For dense components of any size
// - Using a map/unordered_map
//   - For very sparse components of any size
// - Indirect using an index vector indexing into a dense data vector
//    - For somewhat sparse components that are very big
//
// Dense -> Almost every entity has this component
// Sparse -> Only a few entities have this component

template <typename T>
concept TagComponent = std::is_empty_v<T>;

namespace detail {

/// @brief Contains a mapping from all entities to the given component.
template <typename TComponent>
class Table {
public:
    using Component = TComponent;

    /// @brief (Re-)Initializes a component for an entity.
    Component& attach(Component component, Entity entity)
    {
        if (entity.id < entities_.size()) {
            entities_[entity.id] = true;
            return components_[entity.id] = std::move(component);
        }
        entities_.reserve(entity.id + 1);
        components_.reserve(entity.id + 1);
        entities_.resize(entity.id);
        components_.resize(entity.id);
        entities_.push_back(true);
        return components_.emplace_back(std::move(component));
    }

    /// @brief Removes a component from an entity and returns its data by move.
    /// @remark Can safely be called if no component exists.
    Component detach(Entity entity)
    {
        if (!get(entity))
            return Component{};
        entities_[entity.id] = false;
        auto data = std::move(components_[entity.id]);
        if (entity.id == entities_.size() - 1) {
            auto new_size = entity.id;
            while (new_size > 0 && !entities_[--new_size])
                ;
            entities_.resize(new_size);
            components_.resize(new_size);
            // TODO: Call shrink_to_fit on both vectors if capacity is vastly too big.
        }
        return data;
    }

    bool has(Entity entity) const { return entity.id < entities_.size() && entities_[entity.id]; }

    Component* get(Entity entity) { return getHelper(*this, entity); }
    const Component* get(Entity entity) const { return getHelper(*this, entity); }

private:
    template <typename TSelf>
    static auto* getHelper(TSelf& self, Entity entity)
    {
        return self.has(entity) ? &self.components_[entity.id] : nullptr;
    }

    boost::dynamic_bitset<> entities_;
    std::vector<Component> components_;
};

/// @brief Contains a mapping from all entities to the given component.
template <TagComponent TComponent>
class Table<TComponent> {
public:
    using Component = TComponent;

    /// @brief (Re-)Initializes a component for an entity.
    void attach(Component component, Entity entity)
    {
        if (entity.id < entities_.size()) {
            entities_[entity.id] = true;
            return;
        }
        entities_.reserve(entity.id + 1);
        entities_.resize(entity.id);
        entities_.push_back(true);
    }

    /// @brief Removes a component from an entity and returns its data by move.
    /// @remark Can safely be called if no component exists.
    void detach(Entity entity)
    {
        if (!has(entity))
            return;
        entities_[entity.id] = false;
        if (entity.id == entities_.size() - 1) {
            auto new_size = entity.id;
            while (new_size > 0 && !entities_[--new_size])
                ;
            entities_.resize(new_size);
            // TODO: Call shrink_to_fit if capacity is vastly too big.
        }
    }

    bool has(Entity entity) const { return entity.id < entities_.size() && entities_[entity.id]; }

private:
    boost::dynamic_bitset<> entities_;
};

} // namespace detail

} // namespace dang::ecs

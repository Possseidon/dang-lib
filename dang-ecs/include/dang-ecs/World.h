#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "dang-ecs/Entities.h"
#include "dang-ecs/Entity.h"
#include "dang-ecs/Table.h"
#include "dang-ecs/global.h"

#include "boost/dynamic_bitset.hpp"

namespace dang::ecs {

namespace detail {

template <typename T>
struct ImplicitReference {
    operator T&() const { return *ptr; }

    T* ptr;
};

} // namespace detail

template <typename... TComponents>
struct Components {};

/// @brief Models an entity component system.
/// @tparam TComponents All supported components.
template <typename TComponents>
class World;

template <typename... TComponents>
class World<Components<TComponents...>> {
public:
    template <typename... TInitialComponents>
    Entity spawn(TInitialComponents... initial_components)
    {
        auto entity = Entity{next_free_entity_id_++};
        if (entity.id < entities_.size())
            entities_[entity.id] = true;
        else
            entities_.push_back(true);

        while (next_free_entity_id_ < entities_.size() && entities_[next_free_entity_id_])
            next_free_entity_id_++;

        (attach(std::move(initial_components), entity), ...);

        return entity;
    }

    void kill(Entity entity)
    {
        (detach<TComponents>(entity), ...);
        entities_[entity.id] = false;
        next_free_entity_id_ = std::min(next_free_entity_id_, entity.id);
        if (entity.id == entities_.size() - 1) {
            auto new_size = entity.id;
            while (new_size > 0 && !entities_[--new_size])
                ;
            entities_.resize(new_size);
            // TODO: Call shrink_to_fit on entities if capacity is vastly too big.
        }
        // TODO: Notify entity death to update references.
    }

    /// @remark May invalidate references to other components of the same type.
    template <typename TComponent>
    decltype(auto) attach(TComponent component, Entity entity)
    {
        return std::get<detail::Table<std::remove_reference_t<TComponent>>>(component_tables_)
            .attach(std::move(component), entity);
    }

    /// @remark May invalidate references to other components of the same type.
    template <typename TComponent>
    auto detach(Entity entity)
    {
        return std::get<detail::Table<TComponent>>(component_tables_).detach(entity);
    }

    template <typename TComponent>
    bool has(Entity entity) const requires(!TagComponent<TComponent>)
    {
        return get<TComponent>(entity);
    }

    template <typename TComponent>
    auto get(Entity entity)
    {
        return std::get<detail::Table<TComponent>>(component_tables_).get(entity);
    }

    template <typename TComponent>
    auto get(Entity entity) const
    {
        return std::get<detail::Table<TComponent>>(component_tables_).get(entity);
    }

    bool set(TagComponent auto component, Entity entity, bool enabled = true)
    {
        if (enabled)
            attach(component, entity);
        else
            clear(component, entity);
        return enabled;
    }

    void clear(TagComponent auto component, Entity entity) { detach<decltype(component)>(entity); }

    bool toggle(TagComponent auto component, Entity entity) { return set(component, entity, !is(component, entity)); }

    bool is(TagComponent auto component, Entity entity) const { return get<decltype(component)>(entity); }

    Entities filter(auto... filters) {}

    void apply(auto... system_chain)
    {
        if constexpr (sizeof...(system_chain) == 0)
            return;

        for (auto entity_id = entities_.find_first(); entity_id != entities_.npos;
             entity_id = entities_.find_next(entity_id))
            (void)(SystemApplier<decltype(std::function(system_chain))>()(system_chain, *this, entity_id) && ...);
    }

private:
    template <typename TFunc>
    struct SystemApplier;

    template <typename TRet, typename... TArgs>
    struct SystemApplier<std::function<TRet(TArgs...)>> {
        template <typename TSystem>
        bool operator()(TSystem system, World& world, Entity::ID entity_id) const
        {
            // This flag isn't very pretty, but actually doing it pretty is way more contrived and even less readable.
            bool has_all_components = true;

            auto args = std::tuple([&]() -> decltype(auto) {
                if constexpr (std::is_same_v<TArgs, Entity>) {
                    return Entity{entity_id};
                }
                else if constexpr (std::is_same_v<TArgs, World&> || std::is_same_v<TArgs, const World&>) {
                    return world;
                }
                else if constexpr (TagComponent<TArgs>) {
                    if (!world.template get<std::remove_cvref_t<TArgs>>(Entity{entity_id}))
                        has_all_components = false;
                    return TArgs{};
                }
                else {
                    auto component = detail::ImplicitReference<std::remove_cvref_t<TArgs>>{
                        world.template get<std::remove_cvref_t<TArgs>>(Entity{entity_id})};
                    if (!component.ptr)
                        has_all_components = false;
                    return component;
                }
            }()...);

            if (!has_all_components)
                return false;

            if constexpr (std::convertible_to<TRet, bool>) {
                return std::apply(system, args);
            }
            else {
                std::apply(system, args);
                return true;
            }
        }
    };

    Entity::ID next_free_entity_id_ = 0;
    boost::dynamic_bitset<> entities_;
    std::tuple<detail::Table<TComponents>...> component_tables_;
};

} // namespace dang::ecs

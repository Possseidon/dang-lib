#pragma once

#include <compare>
#include <numeric>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

#include "dang-ecs/Entity.h"
#include "dang-ecs/global.h"
#include "dang-utils/utils.h"

#include "boost/dynamic_bitset.hpp"

namespace dang::ecs {

namespace detail {

using EntitiesBitStorage = boost::dynamic_bitset<std::size_t>;
static_assert(EntitiesBitStorage::npos == Entity::invalid_id);

template <typename CRTP>
class EntitiesHelper {
public:
    EntitiesHelper() = default;

    /// @brief Mainly meant for testing.
    EntitiesHelper(std::initializer_list<Entity> entities)
    {
        for (auto entity : entities)
            crtp().insert(entity);
    }

    EntitiesHelper(auto&& entities) { crtp() |= std::forward<decltype(entities)>(entities); }

    CRTP& operator=(auto&& entities)
    {
        crtp().clear();
        return crtp() |= std::forward<decltype(entities)>(entities);
    }

    // --- Element Access

    // Vector

    constexpr Entity back() const { return Entity{crtp().maxID().value_or(Entity::invalid_id)}; }

    // Bitset

    constexpr bool test(Entity entity) const { return crtp().contains(entity); }
    constexpr std::size_t count() const { return crtp().size(); }

    // --- Capacity

    // Vector

    constexpr bool empty() const { return crtp().size() == 0; }

    // --- Modifiers

    // Bitset

    constexpr CRTP& set(Entity entity, bool value = true)
    {
        if (value)
            crtp().insert(entity);
        else
            crtp().erase(entity);
        return crtp();
    }

    constexpr CRTP& reset(Entity entity) { return set(entity, false); }

    constexpr CRTP& flip(Entity entity) { return set(entity, crtp().test(entity)); }

    // --- Other

    constexpr std::size_t requiredHeapBytes() const
    {
        return CRTP::requiredHeapBytesFor(crtp().size(), crtp().maxID());
    }

    friend auto operator<=>(const EntitiesHelper&, const EntitiesHelper&) = default;

    friend constexpr void swap(CRTP& lhs, CRTP& rhs) { lhs.swap(rhs); }

    CRTP operator&(auto&& other) && { return std::move(crtp() &= std::forward<decltype(other)>(other)); }
    CRTP operator&(auto&& other) const& { return CRTP(crtp()) &= std::forward<decltype(other)>(other); }
    CRTP operator|(auto&& other) && { return std::move(crtp() |= std::forward<decltype(other)>(other)); }
    CRTP operator|(auto&& other) const& { return CRTP(crtp()) |= std::forward<decltype(other)>(other); }
    CRTP operator^(auto&& other) && { return std::move(crtp() ^= std::forward<decltype(other)>(other)); }
    CRTP operator^(auto&& other) const& { return CRTP(crtp()) ^= std::forward<decltype(other)>(other); }
    CRTP operator-(auto&& other) && { return std::move(crtp() -= std::forward<decltype(other)>(other)); }
    CRTP operator-(auto&& other) const& { return CRTP(crtp()) -= std::forward<decltype(other)>(other); }

private:
    constexpr CRTP& crtp() { return static_cast<CRTP&>(*this); }
    constexpr const CRTP& crtp() const { return static_cast<const CRTP&>(*this); }
};

class EntitiesBitsetIterator {
public:
    struct Begin {};
    struct End {};

    using difference_type = std::ptrdiff_t;
    using value_type = Entity;
    using reference = const value_type&;
    using pointer = const value_type*;
    using iterator_category = std::forward_iterator_tag;

    EntitiesBitsetIterator() = default;

    constexpr EntitiesBitsetIterator(const EntitiesBitStorage& ids, Begin)
        : ids_(&ids)
        , current_{ids.find_first()}
    {}

    constexpr EntitiesBitsetIterator(const EntitiesBitStorage& ids, End)
        : ids_(&ids)
        , current_{Entity::invalid_id}
    {}

    constexpr EntitiesBitsetIterator& operator++()
    {
        current_.id = ids_->find_next(current_.id);
        return *this;
    }

    constexpr EntitiesBitsetIterator operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    constexpr bool operator==(EntitiesBitsetIterator other) const { return current_ == other.current_; }
    constexpr bool operator!=(EntitiesBitsetIterator other) const { return !(*this == other); }

    constexpr reference operator*() const { return current_; }
    constexpr pointer operator->() const { return &current_; }

private:
    const EntitiesBitStorage* ids_;
    Entity current_;
};

} // namespace detail

class EntitiesBitset : public detail::EntitiesHelper<EntitiesBitset> {
public:
    using EntitiesHelper::EntitiesHelper;
    using EntitiesHelper::operator=;

    // --- Element Access

    // Vector

    constexpr Entity front() const { return Entity{ids_.find_first()}; }
    // back() from helper

    constexpr bool contains(Entity entity) const { return entity.id < ids_.size() && ids_[entity.id]; }

    // Bitset

    // test() from helper
    // all()/any()/none() don't really make a lot of sense
    // count() from helper

    // --- Iterators

    constexpr detail::EntitiesBitsetIterator begin() const { return {ids_, detail::EntitiesBitsetIterator::Begin()}; }
    constexpr detail::EntitiesBitsetIterator cbegin() const { return begin(); }
    constexpr detail::EntitiesBitsetIterator end() const { return {ids_, detail::EntitiesBitsetIterator::End()}; }
    constexpr detail::EntitiesBitsetIterator cend() const { return end(); }

    // --- Capacity

    // Vector

    // empty() from helper
    constexpr std::size_t size() const { return ids_.count(); }
    constexpr std::size_t max_size() const { return ids_.max_size(); }
    constexpr void reserve(std::size_t new_capacity) { ids_.reserve(new_capacity); }
    constexpr std::size_t capacity() const { return ids_.capacity(); }
    constexpr void shrink_to_fit() { return ids_.shrink_to_fit(); }

    constexpr std::size_t currentHeapBytes() const { return capacity() / dutils::char_bit; }

    // Bitset

    // size() already means the same as count()

    // --- Modifiers

    // Vector

    constexpr void clear() { ids_.clear(); }

    constexpr bool insert(Entity entity)
    {
        if (entity.id < ids_.size()) {
            auto old = ids_.test(entity.id);
            ids_[entity.id] = true;
            return !old;
        }
        ids_.reserve(entity.id + 1);
        ids_.resize(entity.id);
        ids_.push_back(true);
        return true;
    }

    constexpr bool erase(Entity entity)
    {
        if (!contains(entity))
            return false;
        ids_[entity.id] = false;
        if (entity.id == ids_.size() - 1) {
            auto new_size = entity.id;
            while (new_size > 0 && !ids_[--new_size])
                ;
            ids_.resize(new_size);
        }
        return true;
    }

    constexpr void swap(EntitiesBitset& other) { ids_.swap(other.ids_); }

    // Bitset

    // TODO: A lot of these operations need unnecessary copies/resizing in certain cases.
    //       I might just have to write my own dynamic_bitset to avoid that.

    EntitiesBitset& operator&=(auto&& other)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(other)>, EntitiesBitset>) {
            if (empty())
                return *this;

            if (other.empty()) {
                clear();
                return *this;
            }

            if (other.ids_.size() > ids_.size()) {
                if constexpr (std::is_lvalue_reference_v<decltype(other)>) {
                    auto other_ids = other.ids_;
                    other_ids.resize(ids_.size());
                    ids_ &= other_ids;
                }
                else {
                    other.ids_.resize(ids_.size());
                    ids_ &= other.ids_;
                }
                return *this;
            }

            if (other.ids_.size() < ids_.size())
                ids_.resize(other.ids_.size());

            ids_ &= other.ids_;
        }
        else {
            for (auto entity : *this) {
                if (!other.test(entity))
                    reset(entity);
            }
        }
        return *this;
    }

    EntitiesBitset& operator|=(auto&& other)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(other)>, EntitiesBitset>) {
            if (other.empty())
                return *this;

            if (empty()) {
                ids_ = std::forward<decltype(other)>(other).ids_;
                return *this;
            }

            if (other.ids_.size() < ids_.size()) {
                if constexpr (std::is_lvalue_reference_v<decltype(other)>) {
                    auto other_ids = other.ids_;
                    other_ids.resize(ids_.size());
                    ids_ |= other_ids;
                }
                else {
                    other.ids_.resize(ids_.size());
                    ids_ |= other.ids_;
                }
                return *this;
            }

            if (other.ids_.size() > ids_.size())
                ids_.resize(other.ids_.size());

            ids_ |= other.ids_;
        }
        else {
            for (auto entity : other)
                set(entity);
        }
        return *this;
    }

    EntitiesBitset& operator^=(auto&& other)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(other)>, EntitiesBitset>) {
            if (other.empty())
                return *this;

            if (empty()) {
                ids_ = std::forward<decltype(other)>(other).ids_;
                return *this;
            }

            if (other.ids_.size() < ids_.size()) {
                if constexpr (std::is_lvalue_reference_v<decltype(other)>) {
                    auto other_ids = other.ids_;
                    other_ids.resize(ids_.size());
                    ids_ ^= other_ids;
                }
                else {
                    other.ids_.resize(ids_.size());
                    ids_ ^= other.ids_;
                }
                return *this;
            }

            if (other.ids_.size() > ids_.size())
                ids_.resize(other.ids_.size());

            ids_ ^= other.ids_;
        }
        else {
            for (auto entity : other)
                flip(entity);
        }
        return *this;
    }

    EntitiesBitset& operator-=(auto&& other)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(other)>, EntitiesBitset>) {
            if (empty() || other.empty())
                return *this;

            if (other.ids_.size() != ids_.size()) {
                if constexpr (std::is_lvalue_reference_v<decltype(other)>) {
                    auto other_ids = other.ids_;
                    other_ids.resize(ids_.size());
                    ids_ -= other_ids;
                }
                else {
                    other.ids_.resize(ids_.size());
                    ids_ -= other.ids_;
                }
                return *this;
            }

            ids_ -= other.ids_;
        }
        else {
            for (auto entity : other)
                reset(entity);
        }
        return *this;
    }

    // set() from helper
    // reset() from helper
    // flip() from helper

    // --- Other

    friend std::strong_ordering operator<=>(const EntitiesBitset& lhs, const EntitiesBitset& rhs)
    {
        if (lhs.ids_ == rhs.ids_)
            return std::strong_ordering::equal;
        return oplessthan(lhs.ids_, rhs.ids_) ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    constexpr std::optional<Entity::ID> maxID() const
    {
        return empty() ? std::nullopt : std::optional(ids_.size() - 1);
    }

    static constexpr std::size_t requiredHeapBytesFor([[maybe_unused]] std::size_t entity_count,
                                                      std::optional<Entity::ID> max_entity_id)
    {
        constexpr auto block_size = sizeof(detail::EntitiesBitStorage::block_type);
        constexpr auto bits_per_block = detail::EntitiesBitStorage::bits_per_block;
        return max_entity_id ? (*max_entity_id + bits_per_block - 1) / bits_per_block * block_size : 0;
    }

    static constexpr bool hasConstantLookup() { return true; }

    // operator&() from helper
    // operator|() from helper
    // operator^() from helper
    // operator-() from helper

private:
    EntitiesBitset(detail::EntitiesBitStorage ids)
        : ids_(std::move(ids))
    {}

    // Assumed to always have size shrunk to the last set bit.
    // This makes it trivial to find the last entity id.
    detail::EntitiesBitStorage ids_;
};

class EntitiesSortedVector : public detail::EntitiesHelper<EntitiesSortedVector> {
public:
    using EntitiesHelper::EntitiesHelper;
    using EntitiesHelper::operator=;

    // --- Element Access

    // Vector

    constexpr Entity front() const { return entities_.empty() ? Entity{Entity::invalid_id} : entities_.front(); }
    // back() from helper

    constexpr bool contains(Entity entity) const { return std::ranges::binary_search(entities_, entity); }

    // Bitset

    // test() from helper
    // all()/any()/none() don't really make a lot of sense
    // count() from helper

    // --- Iterators

    constexpr auto begin() const { return entities_.begin(); }
    constexpr auto cbegin() const { return entities_.cbegin(); }

    constexpr auto end() const { return entities_.end(); }
    constexpr auto cend() const { return entities_.cend(); }

    constexpr auto rbegin() const { return entities_.rbegin(); }
    constexpr auto crbegin() const { return entities_.crbegin(); }

    constexpr auto rend() const { return entities_.rend(); }
    constexpr auto crend() const { return entities_.crend(); }

    // --- Capacity

    // Vector

    // empty() from helper
    constexpr std::size_t size() const { return entities_.size(); }
    constexpr std::size_t max_size() const { return entities_.max_size(); }
    constexpr void reserve(std::size_t new_capacity) { entities_.reserve(new_capacity); }
    constexpr std::size_t capacity() const { return entities_.capacity(); }
    constexpr void shrink_to_fit() { return entities_.shrink_to_fit(); }

    constexpr std::size_t currentHeapBytes() const { return capacity() * sizeof(Entity); }

    // Bitset

    // size() already means the same as count()

    // --- Modifiers

    // Vector

    constexpr void clear() { entities_.clear(); }

    constexpr bool insert(Entity entity)
    {
        auto lower_bound_iter = std::ranges::lower_bound(entities_, entity);
        if (lower_bound_iter != entities_.end() && *lower_bound_iter == entity)
            return false;
        entities_.insert(lower_bound_iter, entity);
        return true;
    }

    constexpr bool erase(Entity entity)
    {
        auto lower_bound_iter = std::ranges::lower_bound(entities_, entity);
        if (lower_bound_iter == entities_.end() || *lower_bound_iter != entity)
            return false;
        entities_.erase(lower_bound_iter);
        return true;
    }

    constexpr void swap(EntitiesSortedVector& other) { entities_.swap(other.entities_); }

    // Bitset

    EntitiesSortedVector& operator&=(const auto& other)
    {
        if (other.hasConstantLookup()) {
            std::erase_if(entities_, [&](Entity entity) { return !other.test(entity); });
        }
        else {
            std::vector<Entity> result;

            std::size_t min_result_capacity = 0;
            std::size_t max_result_capacity = std::min(size(), other.size());
            result.reserve(std::midpoint(min_result_capacity, max_result_capacity));

            std::ranges::set_intersection(entities_, other, std::back_inserter(result));
            entities_ = std::move(result);
        }
        return *this;
    }

    EntitiesSortedVector& operator|=(const auto& other)
    {
        std::vector<Entity> result;

        auto other_size = other.size();
        std::size_t min_result_capacity = std::max(size(), other_size);
        std::size_t max_result_capacity = size() + other_size;
        result.reserve(std::midpoint(min_result_capacity, max_result_capacity));

        std::ranges::set_union(entities_, other, std::back_inserter(result));
        entities_ = std::move(result);
        return *this;
    }

    EntitiesSortedVector& operator^=(const auto& other)
    {
        std::vector<Entity> result;

        std::size_t min_result_capacity = 0;
        std::size_t max_result_capacity = size() + other.size();
        result.reserve(std::midpoint(min_result_capacity, max_result_capacity));

        std::ranges::set_symmetric_difference(entities_, other, std::back_inserter(result));
        entities_ = std::move(result);
        return *this;
    }

    constexpr EntitiesSortedVector& operator-=(const auto& other)
    {
        if constexpr (other.hasConstantLookup()) {
            std::erase_if(entities_, [&](Entity entity) { return other.test(entity); });
        }
        else {
            std::vector<Entity> result;

            std::size_t min_result_capacity = 0;
            std::size_t max_result_capacity = size();
            result.reserve(std::midpoint(min_result_capacity, max_result_capacity));

            std::ranges::set_difference(entities_, other, std::back_inserter(result));
            entities_ = std::move(result);
        }
        return *this;
    }

    // set() from helper
    // reset() from helper
    // flip() from helper

    // --- Other

    friend auto operator<=>(const EntitiesSortedVector&, const EntitiesSortedVector&) = default;

    constexpr std::optional<Entity::ID> maxID() const
    {
        return empty() ? std::nullopt : std::optional(entities_.back().id);
    }

    static constexpr std::size_t requiredHeapBytesFor(std::size_t entity_count,
                                                      [[maybe_unused]] std::optional<Entity::ID> max_entity_id)
    {
        return entity_count * sizeof(Entity);
    }

    static constexpr bool hasConstantLookup() { return false; }

    // operator&() from helper
    // operator|() from helper
    // operator^() from helper
    // operator-() from helper

private:
    EntitiesSortedVector(std::vector<Entity> entities)
        : entities_(entities)
    {}

    std::vector<Entity> entities_;
};

template <typename... TImplementations>
class EntitiesVariant : public detail::EntitiesHelper<EntitiesVariant<TImplementations...>> {
public:
    using detail::EntitiesHelper<EntitiesVariant>::EntitiesHelper;
    using detail::EntitiesHelper<EntitiesVariant>::operator=;

    constexpr decltype(auto) visit(auto visitor) & { return std::visit(visitor, implementation_); }
    constexpr decltype(auto) visit(auto visitor) const& { return std::visit(visitor, implementation_); }
    constexpr decltype(auto) visit(auto visitor) && { return std::visit(visitor, std::move(implementation_)); }

    // --- Element Access

    // Vector

    constexpr Entity front() const
    {
        return visit([](const auto& implementation) { return implementation.front(); });
    }
    // back() from helper

    constexpr bool contains(Entity entity) const
    {
        return visit([&](const auto& implementation) { return implementation.contains(entity); });
    }

    // Bitset

    // test() from helper
    // all()/any()/none() don't really make a lot of sense
    // count() from helper

    // --- Iterators

    // Use visit to iterate over begin()/end().
    // Otherwise they would need to return variants which probably makes iteration quite slow.

    // --- Capacity

    // Vector

    // empty() from helper

    constexpr std::size_t size() const
    {
        return visit([](const auto& implementation) { return implementation.size(); });
    }

    constexpr std::size_t max_size() const
    {
        return visit([](const auto& implementation) { return implementation.max_size(); });
    }

    constexpr void reserve(std::size_t new_capacity)
    {
        visit([&](auto& implementation) { implementation.reserve(new_capacity); });
    }

    constexpr std::size_t capacity() const
    {
        return visit([](const auto& implementation) { return implementation.capacity(); });
    }

    constexpr void shrink_to_fit()
    {
        visit([](auto& implementation) { implementation.shrink_to_fit(); });
    }

    constexpr std::size_t currentHeapBytes() const
    {
        return visit([](const auto& implementation) { return implementation.curentHeapBytes(); });
    }

    // Bitset

    // size() already means the same as count()

    // --- Modifiers

    // Vector

    constexpr void clear()
    {
        visit([](auto& implementation) { implementation.clear(); });
    }

    constexpr bool insert(Entity entity)
    {
        return visit([&](auto& implementation) { return implementation.insert(entity); });
    }

    constexpr bool erase(Entity entity)
    {
        return visit([&](auto& implementation) { return implementation.erase(entity); });
    }

    constexpr void swap(EntitiesVariant& other) { implementation_.swap(other.implementation_); }

    // Bitset

    // TODO: does forward work here?

    constexpr EntitiesVariant& operator&=(auto&& other)
    {
        visit([&](auto& implementation) { implementation &= std::forward<decltype(other)>(other); });
        return *this;
    }

    constexpr EntitiesVariant& operator|=(auto&& other)
    {
        visit([&](auto& implementation) { implementation |= std::forward<decltype(other)>(other); });
        return *this;
    }

    constexpr EntitiesVariant& operator^=(auto&& other)
    {
        visit([&](auto& implementation) { implementation ^= std::forward<decltype(other)>(other); });
        return *this;
    }

    constexpr EntitiesVariant& operator-=(auto&& other)
    {
        visit([&](auto& implementation) { implementation -= std::forward<decltype(other)>(other); });
        return *this;
    }

    // set() from helper
    // reset() from helper
    // flip() from helper

    // --- Other

    friend constexpr std::strong_ordering operator<=>(const EntitiesVariant& lhs, const EntitiesVariant& rhs)
    {
        return std::visit(dutils::Overloaded{
                              []<typename TImplementation>(const TImplementation& lhs_implementation,
                                                           const TImplementation& rhs_implementation) {
                                  return lhs_implementation <=> rhs_implementation;
                              },
                              [](const auto& lhs_implementation, const auto& rhs_implementation) {
                                  return std::lexicographical_compare_three_way(lhs_implementation.begin(),
                                                                                lhs_implementation.end(),
                                                                                rhs_implementation.begin(),
                                                                                rhs_implementation.end());
                              },
                          },
                          lhs.implementation_,
                          rhs.implementation_);
    }

    friend constexpr std::strong_ordering operator<=>(const EntitiesVariant& lhs, const auto& rhs_implementation)
    {
        return lhs.visit(dutils::Overloaded{
            [&](decltype(rhs_implementation) lhs_implementation) { return lhs_implementation <=> rhs_implementation; },
            [&](const auto& lhs_implementation) {
                return std::lexicographical_compare_three_way(lhs_implementation.begin(),
                                                              lhs_implementation.end(),
                                                              rhs_implementation.begin(),
                                                              rhs_implementation.end());
            },
        });
    }

    friend constexpr std::strong_ordering operator<=>(const auto& lhs_implementation, const EntitiesVariant& rhs)
    {
        return rhs.visit(dutils::Overloaded{
            [&](decltype(lhs_implementation) rhs_implementation) { return lhs_implementation <=> rhs_implementation; },
            [&](const auto& rhs_implementation) {
                return std::lexicographical_compare_three_way(lhs_implementation.begin(),
                                                              lhs_implementation.end(),
                                                              rhs_implementation.begin(),
                                                              rhs_implementation.end());
            },
        });
    }

    constexpr std::optional<Entity::ID> maxID() const
    {
        return visit([](const auto& implementation) { return implementation.maxID(); });
    }

    constexpr std::size_t requiredHeapBytesFor([[maybe_unused]] std::size_t entity_count,
                                               std::optional<Entity::ID> max_entity_id) const
    {
        return visit([&](const auto& implementation) {
            return implementation.requiredHeapBytesFor(entity_count, max_entity_id);
        });
    }

    constexpr bool hasConstantLookup() const
    {
        return visit([](const auto& implementation) { return implementation.hasConstantLookup(); });
    }

    // operator&() from helper
    // operator|() from helper
    // operator^() from helper
    // operator-() from helper

    // --- Implementation Switching

    template <typename TImplementation>
    TImplementation usingImplementation() const&
    {
        return visit([](const auto& implementation) { return TImplementation(implementation); });
    }

    template <typename TImplementation>
    TImplementation usingImplementation() &&
    {
        return std::move(*this).visit([](auto&& implementation) { return TImplementation(std::move(implementation)); });
    }

private:
    std::variant<TImplementations...> implementation_;
};

using Entities = EntitiesVariant<EntitiesBitset, EntitiesSortedVector>;

} // namespace dang::ecs

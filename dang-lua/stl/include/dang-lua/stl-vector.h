#pragma once

#include "dang-lua/State.h"

#include "dang-utils/utils.h"

namespace dang::lua {

template <typename T, typename TAllocator>
struct ClassInfo<std::vector<T, TAllocator>> : DefaultClassInfo {
    using vector = std::vector<T, TAllocator>;

    static const std::string& getClassName()
    {
        static const std::string class_name = "std::vector<" + std::string(Convert<T>::getPushTypename()) + ">";
        return class_name;
    }

    static const char* className() { return getClassName().c_str(); }

    static auto table()
    {
        constexpr auto assign = +[](vector& vec, const vector& other) { vec = other; };
        constexpr auto insert = +[](State& lua, vector& vec, std::size_t index, const T& value) {
            checkIndex(lua, 2, index, vec.size() + 1);
            vec.insert(vec.begin() + (index - 1), value);
        };
        constexpr auto erase = +[](State& lua, vector& vec, std::size_t first, std::optional<std::size_t> last_opt) {
            checkIndex(lua, 2, first, vec.size());
            auto last = last_opt.value_or(first);
            checkIndex(lua, 3, last, vec.size());
            vec.erase(vec.begin() + (first - 1), vec.begin() + last);
        };
        constexpr auto push_back = +[](vector& vec, const T& value) { vec.push_back(value); };
        constexpr auto resize = +[](vector& vec, std::size_t size, std::optional<T> value) {
            if (value)
                return vec.resize(size, *value);
            return vec.resize(size);
        };

        return std::vector{reg<assign>("assign"),
                           // Element access
                           reg<front>("getFront"),
                           reg<setFront>("setFront"),
                           reg<back>("getBack"),
                           reg<setBack>("setBack"),
                           // Capacity
                           reg<&vector::empty>("isEmpty"),
                           reg<&vector::size>("getSize"),
                           reg<&vector::max_size>("getMaxSize"),
                           reg<&vector::reserve>("reserve"),
                           reg<&vector::capacity>("getCapacity"),
                           reg<&vector::shrink_to_fit>("shrinkToFit"),
                           // Modifiers
                           reg<&vector::clear>("clear"),
                           reg<insert>("insert"),
                           reg<erase>("erase"),
                           reg<push_back>("pushBack"),
                           reg<&vector::pop_back>("popBack"),
                           reg<resize>("resize"),
                           reg<&vector::swap>("swap")};
    }

    static auto metatable()
    {
        constexpr auto index = +[](const vector& vec, std::variant<std::size_t, const char*> index) {
            return std::visit(dutils::Overloaded{[&](std::size_t index) -> std::optional<T> {
                                                     if (index < 1 || index > vec.size())
                                                         return std::nullopt;
                                                     return vec[index - 1];
                                                 },
                                                 [](...) { return std::optional<T>(); }},
                              index);
        };
        constexpr auto newindex = +[](State& lua, vector& vec, std::size_t index, const T& value) {
            checkIndex(lua, 2, index, vec.size());
            vec[index - 1] = value;
        };

        std::vector result{// Element access
                           reg<index>("__index"),
                           reg<newindex>("__newindex"),
                           // Capacity
                           reg<&vector::size>("__len"),
                           // Lua specific
                           reg<indextable_pairs>("__pairs")};

        if constexpr (dutils::is_equal_to_comparable_v<T>) {
            constexpr auto eq = +[](const vector& lhs, const vector& rhs) { return lhs == rhs; };
            result.push_back(reg<eq>("__eq"));
        }
        if constexpr (dutils::is_less_comparable_v<T>) {
            constexpr auto lt = +[](const vector& lhs, const vector& rhs) { return lhs < rhs; };
            result.push_back(reg<lt>("__lt"));
        }
        if constexpr (dutils::is_less_equal_comparable_v<T>) {
            constexpr auto le = +[](const vector& lhs, const vector& rhs) { return lhs <= rhs; };
            result.push_back(reg<le>("__le"));
        }

        return result;
    }

    static auto properties()
    {
        constexpr auto resize = +[](vector& vec, std::size_t size) { return vec.resize(size); };

        return std::array{Property{"front", wrap<front>, wrap<setFront>},
                          Property{"back", wrap<back>, wrap<setBack>},
                          Property{"empty", wrap<&vector::empty>},
                          Property{"size", wrap<&vector::size>, wrap<resize>},
                          Property{"maxSize", wrap<&vector::max_size>},
                          Property{"capacity", wrap<&vector::capacity>}};
    }

private:
    static constexpr auto front = +[](const vector& vec) { return vec.empty() ? std::optional<T>() : vec.front(); };
    static constexpr auto setFront = +[](State& lua, vector& vec, const T& value) {
        if (vec.empty())
            lua.argError(1, "vector is empty");
        vec.front() = value;
    };
    static constexpr auto back = +[](const vector& vec) { return vec.empty() ? std::optional<T>() : vec.back(); };
    static constexpr auto setBack = +[](State& lua, vector& vec, const T& value) {
        if (vec.empty())
            lua.argError(1, "vector is empty");
        vec.back() = value;
    };

    static void checkIndex(State& lua, int arg, int index, int size)
    {
        if (index < 1 || index > size)
            lua.argError(
                arg, ("index " + std::to_string(index) + " out of range [1, " + std::to_string(size) + "]").c_str());
    }
};

} // namespace dang::lua

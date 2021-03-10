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
        constexpr auto front = +[](const vector& vec) { return vec.empty() ? std::optional<T>() : vec.front(); };
        constexpr auto back = +[](const vector& vec) { return vec.empty() ? std::optional<T>() : vec.back(); };
        constexpr auto insert = +[](State& lua, vector& vec, std::size_t index, const T& value) {
            if (index < 1 || index > vec.size() + 1)
                lua.argError(2, "index out of range");
            vec.insert(vec.begin() + (index - 1), value);
        };
        constexpr auto erase = +[](State& lua, vector& vec, std::size_t first, std::optional<std::size_t> last_opt) {
            if (first < 1 || first > vec.size())
                lua.argError(2, "index out of range");
            auto last = last_opt.value_or(first);
            if (last < 1 || last > vec.size())
                lua.argError(3, "index out of range");
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
                           reg<front>("front"),
                           reg<back>("back"),
                           // Capacity
                           reg<&vector::empty>("empty"),
                           reg<&vector::max_size>("maxSize"),
                           reg<&vector::reserve>("reserve"),
                           reg<&vector::capacity>("capacity"),
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
            if (index < 1 || index > vec.size())
                lua.argError(2, "index out of range");
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
};

} // namespace dang::lua

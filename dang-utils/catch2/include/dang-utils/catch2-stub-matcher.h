#pragma once

#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "dang-utils/global.h"
#include "dang-utils/stub.h"
#include "dang-utils/utils.h"

#include "catch2/catch.hpp"

namespace dang::utils::Matchers {

namespace detail {

std::string formatNumeralAdverb(std::size_t count)
{
    using namespace std::literals;
    if (count == 1)
        return "once"s;
    if (count == 2)
        return "twice"s;
    if (count == 3)
        return "thrice"s;
    return std::to_string(count) + " times"s;
}

template <typename TFirst, typename... TRest>
std::string formatArgs(const TFirst& first, const TRest&... rest)
{
    auto result = Catch::StringMaker<TFirst>::convert(first);
    if constexpr (sizeof...(TRest) == 0) {
        return result;
    }
    else {
        return result + ", " + formatArgs(rest...);
    }
}

template <typename... TArgs>
std::string formatTuple(std::tuple<TArgs...> args)
{
    return std::apply(formatArgs<TArgs...>, args);
}

template <typename T>
using CalledWithArg = std::variant<std::monostate, T, T*>;

struct Invocation {
    std::optional<std::size_t> index;
};

} // namespace detail

template <typename TRet, typename... TArgs>
class Called : public Catch::MatcherBase<dang::utils::Stub<TRet(TArgs...)>> {
public:
    Called(const dang::utils::Stub<TRet(TArgs...)>&) {}

    Called(const dang::utils::Stub<TRet(TArgs...)>&, std::size_t count)
        : count_(count)
    {}

    bool match(const dang::utils::Stub<TRet(TArgs...)>& stub) const override
    {
        return count_ ? stub.invocations().size() == *count_ : !stub.invocations().empty();
    }

    std::string describe() const override
    {
        using namespace std::literals;

        if (!count_)
            return "expected to be called"s;
        if (*count_ == 0)
            return "not expected to be called"s;
        return "expected to be called "s + detail::formatNumeralAdverb(*count_);
    }

private:
    std::optional<std::size_t> count_;
};

constexpr std::monostate ignored;
detail::Invocation invocation(std::size_t index) { return {index}; }

template <typename TRet, typename... TArgs>
class CalledWith : public Catch::MatcherBase<dang::utils::Stub<TRet(TArgs...)>> {
public:
    template <typename T>
    using RemoveCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

    CalledWith(const dang::utils::Stub<TRet(TArgs...)>&, const detail::CalledWithArg<RemoveCVRef<TArgs>>&... args)
        : args_(args...)
    {}

    CalledWith(const dang::utils::Stub<TRet(TArgs...)>&,
               detail::Invocation invocation,
               const detail::CalledWithArg<RemoveCVRef<TArgs>>&... args)
        : invocation_(invocation)
        , args_(args...)
    {}

    bool match(const dang::utils::Stub<TRet(TArgs...)>& stub) const override
    {
        if (invocation_.index)
            return calledWith(*invocation_.index, stub, std::index_sequence_for<TArgs...>());
        return calledWith(stub);
    }

    std::string describe() const override
    {
        using namespace std::literals;

        std::string additional = " with:\n\t"s + detail::formatTuple(args_) + '\n';

        if (!invocation_.index)
            return "expected to be called"s + additional;
        return "expected to be called on invocation #"s + std::to_string(*invocation_.index + 1) + additional;
    }

private:
    template <std::size_t v_index>
    struct Checker {
        using ArgType = std::tuple_element_t<v_index, std::tuple<TArgs...>>;
        using CleanArgType = RemoveCVRef<ArgType>;

        detail::Invocation& maybe_invocation;
        const std::string& name;
        const CleanArgType& invocation_arg;

        void info(const std::string& msg) const
        {
            if (maybe_invocation.index) {
                UNSCOPED_INFO("invocation #" << (*maybe_invocation.index + 1));
                maybe_invocation.index = {};
            }
            if (name.empty())
                UNSCOPED_INFO("  arg #" << (v_index + 1) << ":\t" << msg);
            else
                UNSCOPED_INFO("  " << name << ":\t" << msg);
        }

        template <typename T>
        bool compare(const T& lhs, const T& rhs) const
        {
            using namespace std::literals;

            bool result = lhs == rhs;
            if (!result) {
                Catch::StringMaker<T> string_maker;
                info(string_maker.convert(lhs) + " != "s + string_maker.convert(rhs));
            }
            return result;
        }

        bool operator()(std::monostate) const { return true; }

        bool operator()([[maybe_unused]] const CleanArgType& arg) const
        {
            using namespace std::literals;

            if constexpr (dang::utils::is_equal_to_comparable_v<CleanArgType>) {
                return compare(invocation_arg, arg);
            }
            else {
                // Fail the check by returning false here, but add a warning.
                if constexpr (std::is_reference_v<ArgType>)
                    info("/!\\ Type not comparable, ignore it or use pointer equality!"s);
                else
                    info("/!\\ Type not comparable, ignore it!"s);
                FAIL_CHECK();
                return false;
            }
        }

        bool operator()([[maybe_unused]] const CleanArgType* ptr) const
        {
            using namespace std::literals;

            if constexpr (std::is_reference_v<ArgType>)
                return compare(&invocation_arg, ptr);
            else {
                // Fail the check by returning false here, but add a warning.
                info("/!\\ Parameter passed by value, do not use pointer equality!"s);
                FAIL_CHECK();
                return false;
            }
        }
    };

    template <std::size_t... v_indices>
    bool calledWith(std::size_t invocation_index,
                    const dang::utils::Stub<TRet(TArgs...)>& stub,
                    std::index_sequence<v_indices...>) const
    {
        // Checker keeps track if invocation has already been printed by clearing this.
        auto checker_invocation = detail::Invocation{invocation_index};
        // Use & to avoid short circuiting and show all mismatches.
        return invocation_index < stub.invocations().size() &&
               (std::visit(
                    Checker<v_indices>{
                        checker_invocation,
                        stub.info().parameters[v_indices],
                        std::get<v_indices>(stub.invocations()[invocation_index]),
                    },
                    std::get<v_indices>(args_)) &
                ...);
    }

    bool calledWith(const dang::utils::Stub<TRet(TArgs...)>& stub) const
    {
        for (std::size_t i = 0; i < stub.invocations().size(); i++) {
            if (calledWith(i, stub, std::index_sequence_for<TArgs...>()))
                return true;
        }
        return false;
    }

    detail::Invocation invocation_;
    std::tuple<detail::CalledWithArg<RemoveCVRef<TArgs>>...> args_;
};

// TODO: Catch3 new matchers aren't virtual anymore and don't need these deduction guides. (I think)

template <typename TRet, typename... TArgs>
Called(const dang::utils::Stub<TRet(TArgs...)>&, ...) -> Called<TRet, TArgs...>;

template <typename TRet, typename... TArgs>
CalledWith(const dang::utils::Stub<TRet(TArgs...)>&, ...) -> CalledWith<TRet, TArgs...>;

} // namespace dang::utils::Matchers

namespace Catch {

template <typename TRet, typename... TArgs>
struct StringMaker<dang::utils::Stub<TRet(TArgs...)>> {
    static std::string convert(const dang::utils::Stub<TRet(TArgs...)>& stub)
    {
        using namespace std::literals;
        using dang::utils::Matchers::detail::formatNumeralAdverb;
        using dang::utils::Matchers::detail::formatTuple;

        const auto& invocations = stub.invocations();
        const auto& info = stub.info();

        auto format_parameter = [](const std::string& parameter) { return parameter.empty() ? "?"s : parameter; };

        std::stringstream ss;

        ss << info.name << '(';
        if (!info.parameters.empty())
            ss << format_parameter(info.parameters.front());
        for (std::size_t i = 1; i < info.parameters.size(); i++)
            ss << ", "s << format_parameter(info.parameters[i]);
        ss << ')';

        if (invocations.empty()) {
            ss << "\nnever called\n"s;
        }
        else {
            ss << "\ncalled "s << formatNumeralAdverb(invocations.size());
            if constexpr (sizeof...(TArgs) > 0) {
                ss << " with:\n"s;
                std::size_t invocation_index = 1;
                for (const auto& invocation : invocations)
                    ss << '#' << invocation_index++ << ":\t" << formatTuple(invocation) << '\n';
            }
            else {
                ss << '\n';
            }
        }

        return ss.str();
    }
};

template <typename T>
struct StringMaker<dang::utils::Matchers::detail::CalledWithArg<T>> {
    static std::string convert(const dang::utils::Matchers::detail::CalledWithArg<T>& arg)
    {
        using namespace std::literals;

        return std::visit(dang::utils::Overloaded{
                              [](std::monostate) { return "_"s; },
                              [](const T& value) { return StringMaker<T>::convert(value); },
                              [](const T* ptr) { return "&"s + StringMaker<T>::convert(*ptr); },
                          },
                          arg);
    }
};

} // namespace Catch

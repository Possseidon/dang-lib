#pragma once

#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "dang-utils/global.h"
#include "dang-utils/stub.h"
#include "dang-utils/utils.h"

#include "catch2/catch_message.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/catch_tostring.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_templated.hpp"

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

struct Invocation {
    std::optional<std::size_t> index;
};

} // namespace detail

class Called : public Catch::Matchers::MatcherGenericBase {
public:
    Called() = default;

    Called(std::size_t count)
        : count_(count)
    {}

    template <typename TRet, typename... TArgs>
    bool match(const Stub<TRet(TArgs...)>& stub) const
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

struct Ignored {};
constexpr Ignored ignored;
detail::Invocation invocation(std::size_t index) { return {index}; }

template <typename... TArgs>
class CalledWith : public Catch::Matchers::MatcherGenericBase {
public:
    template <typename T>
    using RemoveCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

    CalledWith(const TArgs&... args)
        : args_(args...)
    {}

    CalledWith(detail::Invocation invocation, const TArgs&... args)
        : invocation_(invocation)
        , args_(args...)
    {}

    template <typename TRet, typename... TStubArgs>
    bool match(const Stub<TRet(TStubArgs...)>& stub) const
    {
        static_assert(sizeof...(TStubArgs) == sizeof...(TArgs), "Number of parameters must match the stub.");

        if (invocation_.index)
            return calledWith(*invocation_.index, stub, std::index_sequence_for<TStubArgs...>());
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
    template <std::size_t v_index, typename TStubArg>
    struct Checker {
        using ArgType = std::tuple_element_t<v_index, std::tuple<TArgs...>>;

        detail::Invocation& maybe_invocation;
        const std::string& name;
        const TStubArg& invocation_arg;

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

        bool operator()([[maybe_unused]] const ArgType& arg) const
        {
            using namespace std::literals;

            if constexpr (std::is_same_v<ArgType, Ignored>) {
                return true;
            }
            else {
                // Since references cannot be null, "nullptr" always means value comparison.
                if constexpr (!std::is_null_pointer_v<std::remove_reference_t<ArgType>> &&
                              (std::is_convertible_v<std::remove_reference_t<ArgType>,
                                                     std::remove_reference_t<TStubArg>*> ||
                               std::is_convertible_v<std::remove_reference_t<ArgType>,
                                                     const std::remove_reference_t<TStubArg>*> ||
                               std::is_convertible_v<std::remove_reference_t<ArgType>,
                                                     std::remove_const_t<std::remove_reference_t<TStubArg>>*>)) {
                    if constexpr (std::is_reference_v<TStubArg>) {
                        bool result = &invocation_arg == arg;
                        if (!result)
                            info(Catch::StringMaker<TStubArg>().convert(invocation_arg) + "["s +
                                 Catch::StringMaker<decltype(&invocation_arg)>().convert(&invocation_arg) + "] != "s +
                                 (arg ? Catch::StringMaker<decltype(*arg)>().convert(*arg) + "["s +
                                            Catch::StringMaker<ArgType>().convert(arg) + "]"s
                                      : "null /!\\ references cannot be null"s));
                        return result;
                    }
                    else {
                        static_assert(invalid_type<ArgType, TStubArg>,
                                      "Parameter passed by value, do not use pointer equality.");
                    }
                }
                else if constexpr (is_equal_to_comparable_v<ArgType, TStubArg>) {
                    bool result = invocation_arg == arg;
                    if (!result)
                        info(Catch::StringMaker<TStubArg>().convert(invocation_arg) + " != "s +
                             Catch::StringMaker<ArgType>().convert(arg));
                    return result;
                }
                else if constexpr (std::is_reference_v<TStubArg>) {
                    static_assert(invalid_type<ArgType, TStubArg>,
                                  "Type not comparable, ignore it or use pointer equality.");
                }
                else {
                    static_assert(invalid_type<ArgType, TStubArg>, "Type not comparable, ignore it.");
                }
            }
        }
    };

    template <typename TRet, typename... TStubArgs, std::size_t... v_indices>
    bool calledWith(std::size_t invocation_index,
                    const Stub<TRet(TStubArgs...)>& stub,
                    std::index_sequence<v_indices...>) const
    {
        // Checker keeps track if invocation has already been printed by clearing this.
        auto checker_invocation = detail::Invocation{invocation_index};
        // Use & to avoid short circuiting and show all mismatches.
        return invocation_index < stub.invocations().size() &&
               (Checker<v_indices, std::tuple_element_t<v_indices, std::tuple<TStubArgs...>>>{
                    checker_invocation,
                    stub.info().parameters[v_indices],
                    std::get<v_indices>(stub.invocations()[invocation_index]),
                }(std::get<v_indices>(args_)) &
                ...);
    }

    template <typename TRet, typename... TStubArgs>
    bool calledWith(const Stub<TRet(TStubArgs...)>& stub) const
    {
        for (std::size_t i = 0; i < stub.invocations().size(); i++) {
            if (calledWith(i, stub, std::index_sequence_for<TArgs...>()))
                return true;
        }
        return false;
    }

    detail::Invocation invocation_;
    std::tuple<TArgs...> args_;
};

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

template <>
struct StringMaker<dang::utils::Matchers::Ignored> {
    static std::string convert(dang::utils::Matchers::Ignored)
    {
        using namespace std::literals;
        return "_"s;
    }
};

} // namespace Catch

#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "dang-utils/utils.h"

namespace dang::utils {

template <typename TSignature>
class Stub;

template <typename... TArgs, typename TRet>
class Stub<TRet(TArgs...)> {
public:
    using Signature = TRet(TArgs...);
    using ParameterNames = std::array<std::string, sizeof...(TArgs)>;
    using Invocations = std::vector<std::tuple<TArgs...>>;

    struct Info {
        std::string name = "stub";
        ParameterNames parameters;
    };

    Stub() = default;

    template <typename T, typename = std::enable_if_t<std::is_same_v<remove_cvref_t<T>, TRet>>>
    Stub(T&& ret)
        : data_(std::make_shared<Data>([ret = std::move(ret)](...) { return ret; }))
    {}

    Stub(std::function<Signature> implementation)
        : data_(std::make_shared<Data>(std::move(implementation)))
    {}

    void setInfo(Info info) { data_->info = std::move(info); }
    const Info& info() const { return data_->info; }

    TRet operator()(TArgs... args)
    {
        data_->invocations.emplace_back(args...);
        return data_->implementation(args...);
    }

    const auto& invocations() const { return data_->invocations; }

    void clear() { data_->invocations.clear(); }

private:
    struct Data {
        Data() = default;
        Data(std::function<Signature> implementation)
            : implementation(std::move(implementation))
        {}

        Info info;
        Invocations invocations;
        std::function<Signature> implementation = [](...) { return TRet(); };
    };

    std::shared_ptr<Data> data_ = std::make_shared<Data>();
};

} // namespace dang::utils

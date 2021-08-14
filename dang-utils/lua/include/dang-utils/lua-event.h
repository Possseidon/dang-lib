#pragma once

#include "dang-lua/State.h"

#include "dang-utils/event.h"

namespace dang::lua {

template <typename... TArgs>
struct ClassInfo<dang::utils::Event<TArgs...>> : DefaultClassInfo {
private:
    static const std::string& getClassName()
    {
        using namespace std::literals;
        static const std::string class_name = "Event<>"s;
        return class_name;
    }

    template <typename TFirst, typename... TRest>
    static const std::string& getClassName()
    {
        using namespace std::literals;
        static const std::string class_name = "Event<"s + std::string(Convert<TFirst>::getPushTypename()) +
                                              ((", "s + std::string(Convert<TRest>::getPushTypename())) + ... + ""s) +
                                              ">"s;
        return class_name;
    }

public:
    using Event = dang::utils::Event<TArgs...>;

    static const char* className()
    {
        if constexpr (sizeof...(TArgs) == 0)
            return getClassName().c_str();
        else
            return getClassName<TArgs...>().c_str();
    }

    static auto table()
    {
        static constexpr auto makeHandler = +[](lua_State* state, Arg function) {
            return [state, ref = std::move(function).ref()](const TArgs&... args) {
                // TODO: This might need a forced call to lua_checkstack...
                State lua(state);
                lua.ensurePushable(1 + combinedPushCount(args...));
                lua.push(ref).call(args...);
            };
        };

        constexpr auto append = +[](lua_State* state, Event& event, Arg function) {
            event.append(makeHandler(state, std::move(function)));
        };
        constexpr auto prepend = +[](lua_State* state, Event& event, Arg function) {
            event.prepend(makeHandler(state, std::move(function)));
        };
        constexpr auto subscribe = +[](lua_State* state, Event& event, Arg function) {
            return event.subscribe(makeHandler(state, std::move(function)));
        };

        return std::array{reg<(&Event::operator bool)>("hasHandler"),
                          reg<append>("append"),
                          reg<prepend>("prepend"),
                          reg<subscribe>("subscribe")};
    }

    static auto metatable()
    {
        constexpr auto call = +[](const Event& event, TArgs... args) { event(std::move(args)...); };
        return std::array{reg<call>("__call")};
    }
};

template <typename... TArgs>
struct ClassInfo<dang::utils::EventSubscription<TArgs...>> : DefaultClassInfo {
private:
    static const std::string& getClassName()
    {
        using namespace std::literals;
        static const std::string class_name = "EventSubscription<>"s;
        return class_name;
    }

    template <typename TFirst, typename... TRest>
    static const std::string& getClassName()
    {
        using namespace std::literals;
        static const std::string class_name = "EventSubscription<"s + std::string(Convert<TFirst>::getPushTypename()) +
                                              ((", "s + std::string(Convert<TRest>::getPushTypename())) + ... + ""s) +
                                              ">"s;
        return class_name;
    }

public:
    using Subscription = dang::utils::EventSubscription<TArgs...>;

    static const char* className()
    {
        if constexpr (sizeof...(TArgs) == 0)
            return getClassName().c_str();
        else
            return getClassName<TArgs...>().c_str();
    }

    static auto table()
    {
        return std::array{reg<(&Subscription::operator bool)>("valid"), reg<&Subscription::remove>("remove")};
    }

    static auto metatable() { return std::array{reg<&Subscription::remove>("__close")}; }
};

} // namespace dang::lua

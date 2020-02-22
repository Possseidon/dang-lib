#pragma once

#include "utils.h"

#include <functional>
#include <vector>

namespace dang::utils
{

namespace detail
{

template <int Index>
struct Placeholder {};

template <int Index, int... Indices, typename... TArgs>
auto bind_indices(TArgs&&... args)
{
    if constexpr (Index == 0)
        return std::bind(args..., Placeholder<Indices>()...);
    else
        return bind_indices<Index - 1, Index, Indices...>(std::forward<TArgs>(args)...);
}

template <int Index, typename... TArgs>
auto bind_n(TArgs&&... args)
{
    return bind_indices<Index>(std::forward<TArgs>(args)...);
}

}

/// <summary>
/// <para>Represents an event which can be subscribed to.</para>
/// <para>To create an event, simply declare a public member variable.</para>
/// <para>To subscribe to an event, declare a member variable of type Event::Subscrption.</para>
/// </summary>
template <typename... TArgs>
class Event {
public:
    using Handler = std::function<void(const TArgs &...)>;

    /// <summary>
    /// <para>Allows subscribing to events using either a lambda or a pointer to a member function.</para>
    /// <para>The subscription is automatically removed from the event, once the subscription object itself is destroyed.</para>
    /// </summary>
    class Subscription {
    private:
        /// <summary>Hidden/different to allow for a working parameter deduction of handler</summary>
        Subscription(std::unique_ptr<Handler> handler, Event& event)
            : event_(event)
            , handler_(*event.handlers_.emplace_back(std::move(handler)).get())
        {
        }

    public:
        /// <summary>Subscribes to an event using std::function.</summary>
        template <typename THandler>
        Subscription(Event& event, THandler handler)
            : Subscription(makeHandler(handler), event)
        {
        }

        /// <summary>Subscribes to an event using a member function.</summary>
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, T& instance, void (T::* method)(TOtherArgs...))
            : Subscription(makeHandler(instance, method), event)
        {
        }

        /// <summary>Subscribes to an event using a member function.</summary>
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, T* instance, void (T::* method)(TOtherArgs...))
            : Subscription(makeHandler(*instance, method), event)
        {
        }

        /// <summary>Automatically unsubscribes the handler from the event.</summary>
        ~Subscription()
        {
            auto& handlers = event_.handlers_;
            auto end = handlers.begin();
            auto pos = handlers.end();
            do {
                pos--;
                if (pos->get() == &handler_) {
                    handlers.erase(pos);
                    return;
                }
            } while (pos != end);
            assert(false);
        }

        Subscription(const Subscription&) = delete;
        Subscription(Subscription&&) = delete;
        Subscription& operator=(const Subscription&) = delete;
        Subscription& operator=(Subscription&&) = delete;

    private:
        Event& event_;
        Handler& handler_;
    };

    Event() = default;

    Event(const Event&) = delete;
    Event(Event&&) = delete;
    Event& operator=(const Event&) = delete;
    Event& operator=(Event&&) = delete;

    /// <summary>Utility to create subscriptions.</summary>
    template <typename THandler>
    [[nodiscard]] Subscription subscribe(THandler handler)
    {
        return Subscription(*this, handler);
    }

    /// <summary>Utility to create subscriptions.</summary>
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(T& instance, void (T::* method)(TOtherArgs...))
    {
        return Subscription(*this, instance, method);
    }

    /// <summary>Utility to create subscriptions.</summary>
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(T* instance, void (T::* method)(TOtherArgs...))
    {
        return Subscription(*this, instance, method);
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename THandler>
    void append(THandler handler)
    {
        handlers_.emplace_back(makeHandler(handler));
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void append(T& instance, void (T::* method)(TOtherArgs...))
    {
        handlers_.emplace_back(makeHandler(instance, method));
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void append(T* instance, void (T::* method)(TOtherArgs...))
    {
        handlers_.emplace_back(makeHandler(*instance, method));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename THandler>
    void prepend(THandler handler)
    {
        handlers_.emplace(handlers_.begin(), makeHandler(handler));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void prepend(T& instance, void (T::* method)(TOtherArgs...))
    {
        handlers_.emplace(handlers_.begin(), makeHandler(instance, method));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void prepend(T* instance, void (T::* method)(TOtherArgs...))
    {
        handlers_.emplace(handlers_.begin(), makeHandler(*instance, method));
    }

    /// <summary>Returns true, if the event has at least one handler.</summary>
    explicit operator bool()
    {
        return !handlers_.empty();
    }

    /// <summary>Triggers the event with the given parameters, notifying all subscribers.</summary>
    void operator()(const TArgs&... args) const
    {
        for (const auto& handler : handlers_)
            (*handler)(args...);
    }

private:
    template <typename... TOtherArgs>
    static std::unique_ptr<Handler> makeHandler(std::function<void(TOtherArgs...)> handler)
    {
        return  std::make_unique<Handler>(detail::bind_n<sizeof...(TOtherArgs)>(handler));
    }

    template <typename THandler>
    static std::unique_ptr<Handler> makeHandler(THandler handler)
    {
        return makeHandler(std::function(handler));
    }

    template <typename T, typename... TOtherArgs>
    static std::unique_ptr<Handler> makeHandler(T& instance, void (T::* method)(TOtherArgs...))
    {
        return std::make_unique<Handler>(detail::bind_n<sizeof...(TOtherArgs)>(method, &instance));
    }

    std::vector<std::unique_ptr<Handler>> handlers_;
};

}

namespace std
{

template <int Index>
class is_placeholder<dutils::detail::Placeholder<Index>> : public integral_constant<int, Index> {};

}

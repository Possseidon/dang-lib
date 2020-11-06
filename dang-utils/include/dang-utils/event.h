#pragma once

#include "utils.h"

#include <functional>
#include <list>
#include <memory>

namespace dang::utils
{

namespace detail
{

// TODO: C++20 use std::bind_front

template <int Index>
struct Placeholder {};

template <int Index, int... Indices, typename... TArgs>
auto bind_indices(TArgs&&... args)
{
    if constexpr (Index == 0)
        return std::bind(std::forward<TArgs>(args)..., Placeholder<Indices>()...);
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
/// <para>Represents an event, for which handlers can be registered:</para>
/// <para> - By simply appending/prepending a handler, which cannot be undone.</para>
/// <para> - By subscribing, which is automatically undone, when the subscription goes out of scope.</para>
/// <para>To create an event, simply declare a public member variable, e.g. onWindowResize.</para>
/// <para>To subscribe to an event, declare a member variable of type Event::Subscrption.</para>
/// <para>Events are movable without having to worry about subscriptions.</para>
/// <para>Copying however, will simply do nothing, as this would be hard to "get right" in regards to subscriptions.</para>
/// </summary>
template <typename... TArgs>
class Event {
private:
    /// <summary>How the event stores its handlers internally.</summary>
    using Handler = std::function<void(const TArgs&...)>;

public:
    /// <summary>
    /// <para>Allows subscribing to events using either a lambda or a pointer to a member function.</para>
    /// <para>The subscription is automatically removed from the event, once the subscription object itself is destroyed.</para>
    /// </summary>
    class Subscription {
    private:
        /// <summary>Hidden/different to allow for a working parameter deduction of handler.</summary>
        Subscription(Handler&& handler, Event& event)
            : handlers_(&event.handlers())
            , handler_(handlers_->insert(handlers_->end(), std::move(handler)))
        {
        }

    public:
        /// <summary>Subscriptions can be empty.</summary>
        Subscription() = default;

        /// <summary>Subscribes to an event using any invocable.</summary>
        template <typename THandler>
        Subscription(Event& event, THandler&& handler)
            : Subscription(makeHandler(std::forward<THandler>(handler)), event)
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
        Subscription(Event& event, const T& instance, void (T::* method)(TOtherArgs...) const)
            : Subscription(makeHandler(instance, method), event)
        {
        }

        /// <summary>Subscribes to an event using a member function.</summary>
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, T* instance, void (T::* method)(TOtherArgs...))
            : Subscription(makeHandler(*instance, method), event)
        {
        }

        /// <summary>Subscribes to an event using a member function.</summary>
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, const T* instance, void (T::* method)(TOtherArgs...) const)
            : Subscription(makeHandler(*instance, method), event)
        {
        }

        /// <summary>Automatically unsubscribes the handler from the event.</summary>
        ~Subscription()
        {
            if (handlers_)
                handlers_->erase(handler_);
        }

        Subscription(const Subscription&) = delete;

        Subscription(Subscription&& other) noexcept
            : Subscription()
        {
            swap(other);
        }

        Subscription& operator=(const Subscription&) = delete;

        Subscription& operator=(Subscription&& other) noexcept
        {
            swap(other);
            return *this;
        }

        void swap(Subscription& other) noexcept
        {
            using std::swap;
            swap(handlers_, other.handlers_);
            swap(handler_, other.handler_);
        }

        friend void swap(Subscription& lhs, Subscription& rhs)
        {
            lhs.swap(rhs);
        }

        /// <summary>Whether the subscription is currently subscribed to an event.</summary>
        explicit operator bool() const
        {
            return handlers_;
        }

        /// <summary>Removes an existing subscription prematurely, if there is one.</summary>
        void remove()
        {
            *this = {};
        }

    private:
        std::list<Handler>* handlers_ = nullptr;
        typename std::list<Handler>::iterator handler_;
    };

    Event() = default;

    Event(const Event&)
        : Event()
    {
        // do nothing on copy
    }

    Event(Event&&) = default;

    Event& operator=(const Event&)
    {
        // do nothing on copy
        return *this;
    }

    Event& operator=(Event&&) = default;

    /// <summary>Utility to create subscriptions.</summary>
    template <typename THandler>
    [[nodiscard]] Subscription subscribe(THandler&& handler)
    {
        return Subscription(*this, std::forward<THandler>(handler));
    }

    /// <summary>Utility to create subscriptions.</summary>
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(T& instance, void (T::* method)(TOtherArgs...))
    {
        return Subscription(*this, instance, method);
    }

    /// <summary>Utility to create subscriptions.</summary>
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(const T& instance, void (T::* method)(TOtherArgs...) const)
    {
        return Subscription(*this, instance, method);
    }

    /// <summary>Utility to create subscriptions.</summary>
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(T* instance, void (T::* method)(TOtherArgs...))
    {
        return Subscription(*this, instance, method);
    }

    /// <summary>Utility to create subscriptions.</summary>
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(const T* instance, void (T::* method)(TOtherArgs...) const)
    {
        return Subscription(*this, instance, method);
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename THandler>
    void append(THandler&& handler)
    {
        handlers().emplace_back(makeHandler(std::forward<THandler>(handler)));
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void append(T& instance, void (T::* method)(TOtherArgs...))
    {
        handlers().emplace_back(makeHandler(instance, method));
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void append(const T& instance, void (T::* method)(TOtherArgs...) const)
    {
        handlers().emplace_back(makeHandler(instance, method));
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void append(T* instance, void (T::* method)(TOtherArgs...))
    {
        handlers().emplace_back(makeHandler(*instance, method));
    }

    /// <summary>Appends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void append(const T* instance, void (T::* method)(TOtherArgs...) const)
    {
        handlers().emplace_back(makeHandler(*instance, method));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename THandler>
    void prepend(THandler&& handler)
    {
        handlers().emplace_front(makeHandler(std::forward<THandler>(handler)));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void prepend(T& instance, void (T::* method)(TOtherArgs...))
    {
        handlers().emplace_front(makeHandler(instance, method));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void prepend(const T& instance, void (T::* method)(TOtherArgs...) const)
    {
        handlers().emplace_front(makeHandler(instance, method));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void prepend(T* instance, void (T::* method)(TOtherArgs...))
    {
        handlers().emplace_front(makeHandler(*instance, method));
    }

    /// <summary>Prepends an event handler which cannot be removed.</summary>
    template <typename T, typename... TOtherArgs>
    void prepend(const T* instance, void (T::* method)(TOtherArgs...) const)
    {
        handlers().emplace_front(makeHandler(*instance, method));
    }

    /// <summary>Returns true, if the event has at least one handler.</summary>
    explicit operator bool() const
    {
        return handlers_ && !handlers_->empty();
    }

    /// <summary>Triggers the event with the given parameters, notifying all subscribers.</summary>
    void operator()(const TArgs&... args) const
    {
        if (!handlers_)
            return;
        for (auto& handler : *handlers_)
            handler(args...);
    }

private:
    /// <summary>Returns a reference to the handler list, creating it, if it didn't exist yet.</summary>
    std::list<Handler>& handlers()
    {
        return handlers_ ? *handlers_ : *(handlers_ = std::make_unique<std::list<Handler>>());
    }

    /// <summary>Allows for handlers to have less arguments than the event.</summary>
    template <typename... TOtherArgs>
    static Handler makeHandler(std::function<void(TOtherArgs...)>&& handler)
    {
        return detail::bind_n<sizeof...(TOtherArgs)>(std::move(handler));
    }

    /// <summary>Allows for handlers to have less arguments than the event.</summary>
    template <typename THandler>
    static Handler makeHandler(THandler&& handler)
    {
        return makeHandler(std::function(std::forward<THandler>(handler)));
    }

    /// <summary>Allows for handlers to have less arguments than the event.</summary>
    template <typename T, typename... TOtherArgs>
    static Handler makeHandler(T& instance, void (T::* method)(TOtherArgs...))
    {
        return detail::bind_n<sizeof...(TOtherArgs)>(method, &instance);
    }

    /// <summary>Allows for handlers to have less arguments than the event.</summary>
    template <typename T, typename... TOtherArgs>
    static Handler makeHandler(const T& instance, void (T::* method)(TOtherArgs...) const)
    {
        return detail::bind_n<sizeof...(TOtherArgs)>(method, &instance);
    }

    // Wrapping the list in a unique_ptr has the advantages of:
    // - Only taking up 1 pointer when no handlers are required, which can be quite common
    // - Allows events to be movable without having to worry about subscriptions
    std::unique_ptr<std::list<Handler>> handlers_;
};

}

namespace std
{

template <int Index>
class is_placeholder<dutils::detail::Placeholder<Index>> : public integral_constant<int, Index> {};

}

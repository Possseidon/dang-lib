#pragma once

#include <functional>
#include <list>
#include <memory>

#include "dang-utils/global.h"

namespace dang::utils {

namespace detail {

// TODO: C++20 use std::bind_front

template <int v_index>
struct Placeholder {};

template <int v_index, int... v_indices, typename... TArgs>
auto bind_indices(TArgs&&... args)
{
    if constexpr (v_index == 0)
        return std::bind(std::forward<TArgs>(args)..., Placeholder<v_indices>()...);
    else
        return bind_indices<v_index - 1, v_index, v_indices...>(std::forward<TArgs>(args)...);
}

template <int v_index, typename... TArgs>
auto bind_n(TArgs&&... args)
{
    return bind_indices<v_index>(std::forward<TArgs>(args)...);
}

} // namespace detail

/// @brief Represents an event, for which handlers can be registered.
/// @remark
/// Handlers can be added by:
/// - By simply appending/prepending a handler, which cannot be undone.
/// - By subscribing, which is automatically undone, when the subscription goes out of scope.
/// @remark To create an event, simply declare a public member variable, e.g. on_window_resize.
/// @remark To subscribe to an event, declare a member variable of type Event::Subscription.
/// @remark Events are movable without having to worry about subscriptions.
/// @remark Copying however, will simply do nothing, as this would be hard to "get right" in regards to subscriptions.
template <typename... TArgs>
class Event {
private:
    /// @brief How the event stores its handlers internally.
    using Handler = std::function<void(const TArgs&...)>;

public:
    /// @brief Allows subscribing to events using either a lambda or a pointer to a member function.
    /// @remark The subscription is automatically removed from the event, once the subscription object itself is
    /// destroyed.
    class Subscription {
    private:
        /// @brief Hidden/different to allow for a working parameter deduction of handler.
        Subscription(Handler&& handler, Event& event)
            : handlers_(&event.handlers())
            , handler_(handlers_->insert(handlers_->end(), std::move(handler)))
        {}

    public:
        /// @brief Subscriptions can be empty.
        Subscription() = default;

        /// @brief Subscribes to an event using any invocable.
        template <typename THandler>
        Subscription(Event& event, THandler&& handler)
            : Subscription(makeHandler(std::forward<THandler>(handler)), event)
        {}

        /// @brief Subscribes to an event using a member function.
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, T& instance, void (T::*method)(TOtherArgs...))
            : Subscription(makeHandler(instance, method), event)
        {}

        /// @brief Subscribes to an event using a member function.
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, const T& instance, void (T::*method)(TOtherArgs...) const)
            : Subscription(makeHandler(instance, method), event)
        {}

        /// @brief Subscribes to an event using a member function.
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, T* instance, void (T::*method)(TOtherArgs...))
            : Subscription(makeHandler(*instance, method), event)
        {}

        /// @brief Subscribes to an event using a member function.
        template <typename T, typename... TOtherArgs>
        Subscription(Event& event, const T* instance, void (T::*method)(TOtherArgs...) const)
            : Subscription(makeHandler(*instance, method), event)
        {}

        /// @brief Automatically unsubscribes the handler from the event.
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

        friend void swap(Subscription& lhs, Subscription& rhs) { lhs.swap(rhs); }

        /// @brief Whether the subscription is currently subscribed to an event.
        explicit operator bool() const { return handlers_; }

        /// @brief Removes an existing subscription prematurely, if there is one.
        void remove() { *this = {}; }

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

    /// @brief Utility to create subscriptions.
    template <typename THandler>
    [[nodiscard]] Subscription subscribe(THandler&& handler)
    {
        return Subscription(*this, std::forward<THandler>(handler));
    }

    /// @brief Utility to create subscriptions.
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(T& instance, void (T::*method)(TOtherArgs...))
    {
        return Subscription(*this, instance, method);
    }

    /// @brief Utility to create subscriptions.
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(const T& instance, void (T::*method)(TOtherArgs...) const)
    {
        return Subscription(*this, instance, method);
    }

    /// @brief Utility to create subscriptions.
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(T* instance, void (T::*method)(TOtherArgs...))
    {
        return Subscription(*this, instance, method);
    }

    /// @brief Utility to create subscriptions.
    template <typename T, typename... TOtherArgs>
    [[nodiscard]] Subscription subscribe(const T* instance, void (T::*method)(TOtherArgs...) const)
    {
        return Subscription(*this, instance, method);
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename THandler>
    void append(THandler&& handler)
    {
        handlers().emplace_back(makeHandler(std::forward<THandler>(handler)));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(T& instance, void (T::*method)(TOtherArgs...))
    {
        handlers().emplace_back(makeHandler(instance, method));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(const T& instance, void (T::*method)(TOtherArgs...) const)
    {
        handlers().emplace_back(makeHandler(instance, method));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(T* instance, void (T::*method)(TOtherArgs...))
    {
        handlers().emplace_back(makeHandler(*instance, method));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(const T* instance, void (T::*method)(TOtherArgs...) const)
    {
        handlers().emplace_back(makeHandler(*instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename THandler>
    void prepend(THandler&& handler)
    {
        handlers().emplace_front(makeHandler(std::forward<THandler>(handler)));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(T& instance, void (T::*method)(TOtherArgs...))
    {
        handlers().emplace_front(makeHandler(instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(const T& instance, void (T::*method)(TOtherArgs...) const)
    {
        handlers().emplace_front(makeHandler(instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(T* instance, void (T::*method)(TOtherArgs...))
    {
        handlers().emplace_front(makeHandler(*instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(const T* instance, void (T::*method)(TOtherArgs...) const)
    {
        handlers().emplace_front(makeHandler(*instance, method));
    }

    /// @brief Returns true, if the event has at least one handler.
    explicit operator bool() const { return handlers_ && !handlers_->empty(); }

    /// @brief Triggers the event with the given parameters, notifying all subscribers.
    void operator()(const TArgs&... args) const
    {
        if (!handlers_)
            return;
        for (auto& handler : *handlers_)
            handler(args...);
    }

private:
    /// @brief Returns a reference to the handler list, creating it, if it didn't exist yet.
    std::list<Handler>& handlers()
    {
        return handlers_ ? *handlers_ : *(handlers_ = std::make_unique<std::list<Handler>>());
    }

    /// @brief Allows for handlers to have less arguments than the event.
    template <typename... TOtherArgs>
    static Handler makeHandler(std::function<void(TOtherArgs...)>&& handler)
    {
        return detail::bind_n<sizeof...(TOtherArgs)>(std::move(handler));
    }

    /// @brief Allows for handlers to have less arguments than the event.
    template <typename THandler>
    static Handler makeHandler(THandler&& handler)
    {
        return makeHandler(std::function(std::forward<THandler>(handler)));
    }

    /// @brief Allows for handlers to have less arguments than the event.
    template <typename T, typename... TOtherArgs>
    static Handler makeHandler(T& instance, void (T::*method)(TOtherArgs...))
    {
        return detail::bind_n<sizeof...(TOtherArgs)>(method, &instance);
    }

    /// @brief Allows for handlers to have less arguments than the event.
    template <typename T, typename... TOtherArgs>
    static Handler makeHandler(const T& instance, void (T::*method)(TOtherArgs...) const)
    {
        return detail::bind_n<sizeof...(TOtherArgs)>(method, &instance);
    }

    // Wrapping the list in a unique_ptr has the advantages of:
    // - Only taking up 1 pointer when no handlers are required, which can be quite common
    // - Allows events to be movable without having to worry about subscriptions
    std::unique_ptr<std::list<Handler>> handlers_;
};

} // namespace dang::utils

namespace std {

template <int v_index>
class is_placeholder<dang::utils::detail::Placeholder<v_index>> : public integral_constant<int, v_index> {};

} // namespace std

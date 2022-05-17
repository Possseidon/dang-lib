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

namespace detail {

/// @brief How an event stores its handlers internally.
template <typename... TArgs>
using EventHandler = std::function<void(const TArgs&...)>;

template <typename... TArgs>
struct EventData {
    using Handler = EventHandler<TArgs...>;
    using Handlers = std::list<Handler>;

    Handlers handlers;
};

} // namespace detail

template <typename... TArgs>
class EventSubscription;

/// @brief Represents an event, for which handlers can be registered.
/// @remark
/// Handlers can be added by:
/// - By simply appending/prepending a handler, which cannot be undone.
/// - By subscribing, which is automatically undone, when the subscription goes out of scope.
/// @remark To create an event, simply declare a public member variable, e.g. on_window_resize.
/// @remark To subscribe to an event, declare a member variable of type EventSubscription.
/// @remark Events are movable without having to worry about subscriptions.
/// @remark Copying however, will simply do nothing, as this would be hard to "get right" in regards to subscriptions.
template <typename... TArgs>
class Event {
public:
    friend class EventSubscription<TArgs...>;
    using Subscription = EventSubscription<TArgs...>;

private:
    using Data = detail::EventData<TArgs...>;
    using Handler = detail::EventHandler<TArgs...>;

public:
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
        ensureData()->handlers.emplace_back(makeHandler(std::forward<THandler>(handler)));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(T& instance, void (T::*method)(TOtherArgs...))
    {
        ensureData()->handlers.emplace_back(makeHandler(instance, method));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(const T& instance, void (T::*method)(TOtherArgs...) const)
    {
        ensureData()->handlers.emplace_back(makeHandler(instance, method));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(T* instance, void (T::*method)(TOtherArgs...))
    {
        ensureData()->handlers.emplace_back(makeHandler(*instance, method));
    }

    /// @brief Appends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void append(const T* instance, void (T::*method)(TOtherArgs...) const)
    {
        ensureData()->handlers.emplace_back(makeHandler(*instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename THandler>
    void prepend(THandler&& handler)
    {
        ensureData()->handlers.emplace_front(makeHandler(std::forward<THandler>(handler)));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(T& instance, void (T::*method)(TOtherArgs...))
    {
        ensureData()->handlers.emplace_front(makeHandler(instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(const T& instance, void (T::*method)(TOtherArgs...) const)
    {
        ensureData()->handlers.emplace_front(makeHandler(instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(T* instance, void (T::*method)(TOtherArgs...))
    {
        ensureData()->handlers.emplace_front(makeHandler(*instance, method));
    }

    /// @brief Prepends an event handler which cannot be removed.
    template <typename T, typename... TOtherArgs>
    void prepend(const T* instance, void (T::*method)(TOtherArgs...) const)
    {
        ensureData()->handlers.emplace_front(makeHandler(*instance, method));
    }

    /// @brief Returns true, if the event has at least one handler.
    explicit operator bool() const { return data_ && !data_->handlers.empty(); }

    /// @brief Triggers the event with the given parameters, notifying all subscribers.
    void operator()(const TArgs&... args) const
    {
        if (!data_)
            return;
        for (auto& handler : data_->handlers)
            handler(args...);
    }

private:
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

    /// @brief Returns a reference to the event data, creating it, if it didn't exist yet.
    const std::shared_ptr<Data>& ensureData() { return data_ ? data_ : (data_ = std::make_shared<Data>()); }

    std::shared_ptr<Data> data_;
};

/// @brief Allows subscribing to events using either a lambda or a pointer to a member function.
/// @remark The subscription is automatically removed from the event, once the subscription object itself is
/// destroyed. Likewise the subscription is cleared automatically when the event itself is destroyed.
template <typename... TArgs>
class EventSubscription {
public:
    friend class Event<TArgs...>;
    using Event = dang::utils::Event<TArgs...>;

private:
    using Handler = detail::EventHandler<TArgs...>;
    using EventData = detail::EventData<TArgs...>;

    /// @brief Hidden/different to allow for a working parameter deduction of handler.
    EventSubscription(Handler&& handler, Event& event)
        : event_data_(event.ensureData())
        , handler_(event.data_->handlers.insert(event.data_->handlers.end(), std::move(handler)))
    {}

public:
    /// @brief Subscriptions can be empty.
    EventSubscription() = default;

    /// @brief Subscribes to an event using any invocable.
    template <typename THandler>
    EventSubscription(Event& event, THandler&& handler)
        : EventSubscription(Event::makeHandler(std::forward<THandler>(handler)), event)
    {}

    /// @brief Subscribes to an event using a member function.
    template <typename T, typename... TOtherArgs>
    EventSubscription(Event& event, T& instance, void (T::*method)(TOtherArgs...))
        : EventSubscription(Event::makeHandler(instance, method), event)
    {}

    /// @brief Subscribes to an event using a member function.
    template <typename T, typename... TOtherArgs>
    EventSubscription(Event& event, const T& instance, void (T::*method)(TOtherArgs...) const)
        : EventSubscription(Event::makeHandler(instance, method), event)
    {}

    /// @brief Subscribes to an event using a member function.
    template <typename T, typename... TOtherArgs>
    EventSubscription(Event& event, T* instance, void (T::*method)(TOtherArgs...))
        : EventSubscription(Event::makeHandler(*instance, method), event)
    {}

    /// @brief Subscribes to an event using a member function.
    template <typename T, typename... TOtherArgs>
    EventSubscription(Event& event, const T* instance, void (T::*method)(TOtherArgs...) const)
        : EventSubscription(Event::makeHandler(*instance, method), event)
    {}

    /// @brief Automatically unsubscribes the handler from the event.
    ~EventSubscription() { remove(); }

    EventSubscription(const EventSubscription&) = delete;
    EventSubscription(EventSubscription&&) = default;
    EventSubscription& operator=(const EventSubscription&) = delete;
    EventSubscription& operator=(EventSubscription&&) = default;

    void swap(EventSubscription& other) noexcept
    {
        using std::swap;
        swap(event_data_, other.event_data_);
        swap(handler_, other.handler_);
    }

    friend void swap(EventSubscription& lhs, EventSubscription& rhs) { lhs.swap(rhs); }

    /// @brief Whether the subscription is currently subscribed to an event.
    explicit operator bool() const { return !event_data_.expired(); }

    /// @brief Removes an existing subscription prematurely, if there is one.
    void remove()
    {
        if (auto event_data = event_data_.lock()) {
            event_data->handlers.erase(handler_);
            event_data_.reset();
        }
    }

private:
    std::weak_ptr<EventData> event_data_;
    typename EventData::Handlers::iterator handler_;
};

} // namespace dang::utils

namespace std {

template <int v_index>
class is_placeholder<dang::utils::detail::Placeholder<v_index>> : public integral_constant<int, v_index> {};

} // namespace std

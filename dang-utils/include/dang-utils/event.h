#pragma once

#include <vector>
#include <functional>

#include "utils.h"

namespace dang::utils
{

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
    public:
        /// <summary>Subscribes to an event using std::function.</summary>
        Subscription(Event& event, Handler handler)
            : event_(event)
            , handler_(handler)
        {
            event.handlers_.push_back(handler_);
        }

        /// <summary>Subscribes to an event using a member function.</summary>
        template <typename T>
        Subscription(Event& event, T* instance, void (T::* method)(const TArgs&...))
            : Subscription(event, [instance, method](const TArgs&... args) { (instance->*method)(args...); })
        {
        }

        /// <summary>Automatically unsubscribes the handler from the event.</summary>
        ~Subscription()
        {
            auto& handlers = event_.handlers_;
            const auto& end = handlers.begin();
            auto pos = handlers.end();
            do {
                std::advance(pos, -1);
                if (&pos->get() == &handler_) {
                    handlers.erase(pos);
                    break;
                }
            } while (pos != end);
        }

    private:
        Event& event_;
        Handler handler_;
    };

    /// <summary>Triggers the event with the given parameters, notifying all subscribers.</summary>
    void operator()(const TArgs&... args) const
    {
        for (const auto& handler : handlers_)
            handler(args...);
    }

private:
    std::vector<std::reference_wrapper<Handler>> handlers_;
};

}

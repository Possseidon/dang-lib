#pragma once

#include <vector>
#include <functional>
#include <memory>

template <typename... TArgs>
struct Event {
    using Handler = std::function<void(const TArgs &...)>;

    struct Subscription {
        Subscription(Event& event, Handler handler)
            : event_(event)
            , handler_(handler)
        {
            event.handlers_.push_back(handler_);
        }

        template <typename T>
        Subscription(Event& event, T* instance, void (T::* method)(const TArgs&...))
            : Subscription(event, [instance, method](const TArgs&... args) { (instance->*method)(args...); })
        {
        }

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

    void operator()(const TArgs&... args) const
    {
        for (const auto& handler : handlers_)
            handler(args...);
    }

private:
    std::vector<std::reference_wrapper<Handler>> handlers_;
};

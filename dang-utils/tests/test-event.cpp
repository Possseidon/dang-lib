#include "dang-utils/event.h"

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"

namespace dutils = dang::utils;

TEST_CASE("Events can have handlers that get called once triggered.", "[event]")
{
    using Event = dutils::Event<>;
    using Values = std::vector<int>;

    Event event;
    Values values;
    auto add_one = [&] { values.push_back(1); };
    auto add_two = [&] { values.push_back(2); };

    SECTION("Triggering an event without handlers does nothing.")
    {
        event();
        CHECK(values.empty());
    }

    SECTION("Appending a handler does not immediately call the event.")
    {
        event.append(add_one);
        CHECK(values.empty());
    }
    SECTION("Appending a handler and triggering the event calls the handler.")
    {
        event.append(add_one);
        event();
        CHECK(values == Values{1});
    }

    SECTION("Prepending a handler does not immediately call the event.")
    {
        event.prepend(add_one);
        CHECK(values.empty());
    }
    SECTION("Prepending a handler and triggering the event calls the handler.")
    {
        event.prepend(add_one);
        event();
        CHECK(values == Values{1});
    }

    SECTION("Subscribing does not immediately call the handler.")
    {
        auto subscription = event.subscribe(add_one);
        CHECK(values.empty());
    }
    SECTION("Subscribing and triggering the event calls the handler.")
    {
        auto subscription = event.subscribe(add_one);
        event();
        CHECK(values == Values{1});
    }
    SECTION("Subscribing and removing the subscription before triggering the event does not call the handler.")
    {
        auto subscription = event.subscribe(add_one);
        subscription.remove();
        CHECK(values.empty());
        event();
        CHECK(values.empty());
    }
    SECTION("Subscribing and destroying the subscription before triggering the event does not call the handler.")
    {
        {
            auto subscription = event.subscribe(add_one);
        }
        CHECK(values.empty());
        event();
        CHECK(values.empty());
    }

    SECTION("Appending a handler and...")
    {
        event.append(add_one);
        SECTION("Appending a second handler calls the second handler after the first one.")
        {
            event.append(add_two);
            event();
            CHECK(values == Values{1, 2});
        }
        SECTION("Prepending a second handler calls the second handler before the first one.")
        {
            event.prepend(add_two);
            event();
            CHECK(values == Values{2, 1});
        }
        SECTION("Subscribing a second handler calls the second handler after the first one.")
        {
            event.append(add_two);
            event();
            CHECK(values == Values{1, 2});
        }
    }

    SECTION("Prepending a handler and...")
    {
        event.prepend(add_one);
        SECTION("Appending a second handler calls the second handler after the first one.")
        {
            event.append(add_two);
            event();
            CHECK(values == Values{1, 2});
        }
        SECTION("Prepending a second handler calls the second handler before the first one.")
        {
            event.prepend(add_two);
            event();
            CHECK(values == Values{2, 1});
        }
        SECTION("Subscribing a second handler calls the second handler after the first one.")
        {
            event.append(add_two);
            event();
            CHECK(values == Values{1, 2});
        }
    }

    SECTION("Subscribing a handler and...")
    {
        auto subscription = event.subscribe(add_one);
        SECTION("Appending a second handler calls the second handler after the first one.")
        {
            event.append(add_two);
            event();
            CHECK(values == Values{1, 2});
        }
        SECTION("Prepending a second handler calls the second handler before the first one.")
        {
            event.prepend(add_two);
            event();
            CHECK(values == Values{2, 1});
        }
        SECTION("Subscribing a second handler calls the second handler after the first one.")
        {
            auto subscription2 = event.subscribe(add_two);
            event();
            CHECK(values == Values{1, 2});
        }
    }

    SECTION("Subscribing two handlers...")
    {
        auto subscription1 = event.subscribe(add_one);
        auto subscription2 = event.subscribe(add_two);
        SECTION("And removing the first one calls only the second handler.")
        {
            subscription1.remove();
            event();
            CHECK(values == Values{2});
        }
        SECTION("And removing the second one calls only the first handler.")
        {
            subscription2.remove();
            event();
            CHECK(values == Values{1});
        }
    }
}

TEST_CASE("Events that are destroyed reset any subscriptions to it.")
{
    using Event = dutils::Event<>;

    Event::Subscription subscription;
    {
        Event event;
        CHECK(!subscription);
        subscription = event.subscribe([] {});
        CHECK(subscription);
    }
    CHECK(!subscription);
}

TEST_CASE("Event handlers can have less parameters than the event itself.", "[event]")
{
    using Event = dutils::Event<int, int, int>;
    using Values = std::vector<std::vector<int>>;

    Event event;
    Values values;

    event.append([&] { values.push_back({}); });
    event.append([&](int x) { values.push_back({x}); });
    event.append([&](int x, int y) { values.push_back({x, y}); });
    event.append([&](int x, int y, int z) { values.push_back({x, y, z}); });

    event(1, 2, 3);
    CHECK(values == Values{{}, {1}, {1, 2}, {1, 2, 3}});
}

#include "NotificationEventPublisher.h"

#include <algorithm>
#include <exception>

void NotificationEventPublisher::subscribe(const std::shared_ptr<INotificationObserver>& observer)
{
    if (!observer)
    {
        return;
    }
    const bool alreadySubscribed = std::any_of(observers_.begin(), observers_.end(),
        [&](const std::weak_ptr<INotificationObserver>& w) { return w.lock() == observer; });
    if (!alreadySubscribed)
    {
        observers_.push_back(observer);
    }
}

void NotificationEventPublisher::unsubscribe(const std::shared_ptr<INotificationObserver>& observer)
{
    std::erase_if(observers_, [&](const std::weak_ptr<INotificationObserver>& w)
    {
        auto locked = w.lock();
        return !locked || locked == observer;   // also removes expired subscribers
    });
}

void NotificationEventPublisher::publish(const NotificationEvent& event)
{
    // Copy: a subscriber may subscribe or unsubscribe while handling the event.
    const auto snapshot = observers_;
    for (const auto& weak : snapshot)
    {
        if (auto observer = weak.lock())
        {
            try
            {
                observer->onEvent(event);
            }
            catch (const std::exception&)
            {
                // A failing subscriber must not prevent other subscribers from being notified.
                ++failedDeliveries_;
            }
        }
    }
    std::erase_if(observers_, [](const std::weak_ptr<INotificationObserver>& w) { return w.expired(); });
}

std::size_t NotificationEventPublisher::subscriberCount() const
{
    return static_cast<std::size_t>(std::count_if(observers_.begin(), observers_.end(),
        [](const std::weak_ptr<INotificationObserver>& w) { return !w.expired(); }));
}

#pragma once

#include <memory>
#include <vector>

#include "NotificationEvents.h"

// Observer (pattern): subscriber interface.
class INotificationObserver
{
public:
    virtual ~INotificationObserver() = default;
    virtual void onEvent(const NotificationEvent& event) = 0;
};

// Subject (pattern): the publisher does not know concrete subscriber classes.
// Subscribers are stored as weak_ptr, so a destroyed subscriber never causes a dangling call
// and the publisher does not extend the lifetime of subscribers.
class NotificationEventPublisher
{
public:
    void subscribe(const std::shared_ptr<INotificationObserver>& observer);
    void unsubscribe(const std::shared_ptr<INotificationObserver>& observer);
    void publish(const NotificationEvent& event);

    std::size_t subscriberCount() const;      // subscribers that are still alive
    int failedDeliveries() const { return failedDeliveries_; }

private:
    std::vector<std::weak_ptr<INotificationObserver>> observers_;
    int failedDeliveries_ = 0;
};

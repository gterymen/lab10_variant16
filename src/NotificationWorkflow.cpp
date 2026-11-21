#include "NotificationWorkflow.h"

#include <ctime>
#include <stdexcept>

NotificationWorkflow::NotificationWorkflow(Storage& storage,
                                           NotificationDao& notifications,
                                           NotificationHistoryDao& history,
                                           NotificationEventPublisher& publisher,
                                           Clock clock)
    : storage_(storage), notifications_(notifications), history_(history),
      publisher_(publisher), clock_(std::move(clock))
{
}

std::string NotificationWorkflow::currentTime()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
    return buffer;
}

int NotificationWorkflow::createNotification(Notification item)
{
    item.id = 0;
    item.status = NotificationStatus::Created;

    int newId = 0;
    // The notification and its first history record are stored in one transaction:
    // if any operation fails, nothing remains in the database.
    storage_.transaction([&]()
    {
        newId = notifications_.add(item);
        history_.add({0, newId, "", NotificationStatus::Created, clock_()});
        return true;
    });

    publisher_.publish(NotificationCreated{newId, item.categoryId});
    return newId;
}

std::string NotificationWorkflow::transition(int notificationId, void (NotificationContext::*operation)())
{
    auto item = notifications_.findById(notificationId);
    if (!item)
    {
        throw std::out_of_range("Notification not found: " + std::to_string(notificationId));
    }

    NotificationContext context(makeState(item->status));
    const std::string before = context.stateName();
    (context.*operation)();                        // throws InvalidTransitionError if not allowed
    const std::string after = context.stateName();

    // Change of the status and the history record are atomic.
    // An exception inside the function rolls the transaction back.
    storage_.transaction([&]()
    {
        notifications_.updateStatus(notificationId, after);
        history_.add({0, notificationId, before, after, clock_()});
        return true;
    });
    return after;
}

void NotificationWorkflow::send(int notificationId)
{
    transition(notificationId, &NotificationContext::send);
}

void NotificationWorkflow::deliver(int notificationId)
{
    transition(notificationId, &NotificationContext::deliver);
    publisher_.publish(DeliveryCompleted{notificationId, true});
}

void NotificationWorkflow::fail(int notificationId)
{
    transition(notificationId, &NotificationContext::fail);
    publisher_.publish(DeliveryCompleted{notificationId, false});
}

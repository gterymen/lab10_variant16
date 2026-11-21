#pragma once

#include <functional>
#include <string>

#include "Database.h"
#include "NotificationDao.h"
#include "NotificationEventPublisher.h"
#include "NotificationHistoryDao.h"
#include "NotificationState.h"

// Business logic of the notification life cycle. Integrates State (transitions), the repositories
// (storage of the status and its history in one transaction) and Observer (events).
class NotificationWorkflow
{
public:
    using Clock = std::function<std::string()>;

    NotificationWorkflow(Storage& storage,
                         NotificationDao& notifications,
                         NotificationHistoryDao& history,
                         NotificationEventPublisher& publisher,
                         Clock clock = &NotificationWorkflow::currentTime);

    // Saves a new notification (state Created) together with the first history record.
    int createNotification(Notification item);

    // Operations of the State pattern. They throw InvalidTransitionError for an
    // invalid transition (nothing is changed) and std::out_of_range for an unknown notification.
    void send(int notificationId);
    void deliver(int notificationId);
    void fail(int notificationId);

    static std::string currentTime();

private:
    // Performs the operation on the state object and stores the result atomically.
    std::string transition(int notificationId, void (NotificationContext::*operation)());

    Storage& storage_;
    NotificationDao& notifications_;
    NotificationHistoryDao& history_;
    NotificationEventPublisher& publisher_;
    Clock clock_;
};

#pragma once

#include <vector>

#include "Database.h"

// Data access component for the history of notification states.
// Methods are virtual so that tests can substitute a failing implementation.
class NotificationHistoryDao
{
public:
    explicit NotificationHistoryDao(Storage& storage);
    virtual ~NotificationHistoryDao() = default;

    virtual int add(const NotificationStatusHistory& item);
    std::vector<NotificationStatusHistory> findByNotification(int notificationId);
    int countAll();

private:
    Storage& storage_;
};

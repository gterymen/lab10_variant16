#pragma once

#include <string>

// History of state changes of a notification (Lab 10, State pattern).
struct NotificationStatusHistory
{
    int id = 0;
    int notificationId = 0;
    std::string fromStatus;
    std::string toStatus;
    std::string changedAt;
};

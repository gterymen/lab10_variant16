#pragma once

#include <variant>

// Event types of the Observer pattern.
struct NotificationCreated
{
    int notificationId = 0;
    int categoryId = 0;
};

struct DeliveryCompleted
{
    int notificationId = 0;
    bool success = false;   // true: delivered, false: delivery error
};

using NotificationEvent = std::variant<NotificationCreated, DeliveryCompleted>;

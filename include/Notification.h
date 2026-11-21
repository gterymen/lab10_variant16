#pragma once

#include <string>

// Dependent (child) entity: notification that belongs to a category.
// categoryId is a foreign key that refers to NotificationCategory::id.
// Lab 10: importance, urgency and waitingMinutes are the input of the priority strategies.
struct Notification
{
    int id = 0;
    int categoryId = 0;
    std::string createdAt;
    std::string message;
    std::string status = "Created";
    double deliveryMs = 0.0;
    int importance = 3;        // 1..5
    int urgency = 3;           // 1..5
    int waitingMinutes = 0;    // time in the queue
};

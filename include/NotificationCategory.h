#pragma once

#include <string>

// Main (parent) entity: category of notifications (email, SMS or push channel)
struct NotificationCategory
{
    int id = 0;
    std::string name;
    std::string channel;
    std::string description;
    std::string status;
};

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Database.h"

// Data access component for the dependent entity (Lab 8), extended in Lab 10
// with the operations needed by State (status) and Strategy (priority input).
class NotificationDao
{
public:
    explicit NotificationDao(Storage& storage);

    int add(const Notification& item);
    std::optional<Notification> findById(int id);
    std::vector<Notification> findByNotificationCategory(int categoryId);
    int countByNotificationCategory(int categoryId);
    bool remove(int id);

    // Lab 10
    bool updateStatus(int id, const std::string& status);
    std::vector<Notification> findByStatus(const std::string& status);

private:
    Storage& storage_;
};

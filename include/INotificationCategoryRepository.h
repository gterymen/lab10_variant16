#pragma once

#include <optional>
#include <vector>

#include "NotificationCategory.h"

// Repository interface: contains no SQLite or sqlite_orm types.
class INotificationCategoryRepository
{
public:
    virtual ~INotificationCategoryRepository() = default;

    // Create
    virtual int add(const NotificationCategory& item) = 0;

    // Read
    virtual std::optional<NotificationCategory> findById(int id) = 0;
    virtual std::vector<NotificationCategory> findAll() = 0;

    // Update: false means that the record does not exist
    virtual bool update(const NotificationCategory& item) = 0;

    // Delete: false means that the record does not exist
    virtual bool remove(int id) = 0;
};

#include "SQLiteNotificationCategoryRepository.h"

SQLiteNotificationCategoryRepository::SQLiteNotificationCategoryRepository(Storage& storage)
    : storage_(storage)
{
}

int SQLiteNotificationCategoryRepository::add(const NotificationCategory& item)
{
    return storage_.insert(item);
}

std::optional<NotificationCategory> SQLiteNotificationCategoryRepository::findById(int id)
{
    return storage_.get_optional<NotificationCategory>(id);
}

std::vector<NotificationCategory> SQLiteNotificationCategoryRepository::findAll()
{
    return storage_.get_all<NotificationCategory>();
}

bool SQLiteNotificationCategoryRepository::update(const NotificationCategory& item)
{
    if (!storage_.get_optional<NotificationCategory>(item.id))
    {
        return false;
    }
    storage_.update(item);
    return true;
}

bool SQLiteNotificationCategoryRepository::remove(int id)
{
    if (!storage_.get_optional<NotificationCategory>(id))
    {
        return false;
    }
    storage_.remove<NotificationCategory>(id);
    return true;
}

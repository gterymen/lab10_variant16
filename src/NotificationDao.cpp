#include "NotificationDao.h"

NotificationDao::NotificationDao(Storage& storage)
    : storage_(storage)
{
}

int NotificationDao::add(const Notification& item)
{
    return storage_.insert(item);
}

std::optional<Notification> NotificationDao::findById(int id)
{
    return storage_.get_optional<Notification>(id);
}

std::vector<Notification> NotificationDao::findByNotificationCategory(int categoryId)
{
    using namespace sqlite_orm;
    return storage_.get_all<Notification>(
        where(c(&Notification::categoryId) == categoryId),
        order_by(&Notification::id));
}

int NotificationDao::countByNotificationCategory(int categoryId)
{
    using namespace sqlite_orm;
    return storage_.count<Notification>(where(c(&Notification::categoryId) == categoryId));
}

bool NotificationDao::remove(int id)
{
    if (!storage_.get_optional<Notification>(id))
    {
        return false;
    }
    storage_.remove<Notification>(id);
    return true;
}

bool NotificationDao::updateStatus(int id, const std::string& status)
{
    auto item = storage_.get_optional<Notification>(id);
    if (!item)
    {
        return false;
    }
    item->status = status;
    storage_.update(*item);
    return true;
}

std::vector<Notification> NotificationDao::findByStatus(const std::string& status)
{
    using namespace sqlite_orm;
    return storage_.get_all<Notification>(
        where(c(&Notification::status) == status),
        order_by(&Notification::id));
}

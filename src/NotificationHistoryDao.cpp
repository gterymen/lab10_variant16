#include "NotificationHistoryDao.h"

NotificationHistoryDao::NotificationHistoryDao(Storage& storage)
    : storage_(storage)
{
}

int NotificationHistoryDao::add(const NotificationStatusHistory& item)
{
    return storage_.insert(item);
}

std::vector<NotificationStatusHistory> NotificationHistoryDao::findByNotification(int notificationId)
{
    using namespace sqlite_orm;
    return storage_.get_all<NotificationStatusHistory>(
        where(c(&NotificationStatusHistory::notificationId) == notificationId),
        order_by(&NotificationStatusHistory::id));
}

int NotificationHistoryDao::countAll()
{
    return storage_.count<NotificationStatusHistory>();
}

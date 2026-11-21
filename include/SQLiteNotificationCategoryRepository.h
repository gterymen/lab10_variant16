#pragma once

#include "Database.h"
#include "INotificationCategoryRepository.h"

class SQLiteNotificationCategoryRepository : public INotificationCategoryRepository
{
public:
    explicit SQLiteNotificationCategoryRepository(Storage& storage);

    int add(const NotificationCategory& item) override;
    std::optional<NotificationCategory> findById(int id) override;
    std::vector<NotificationCategory> findAll() override;
    bool update(const NotificationCategory& item) override;
    bool remove(int id) override;

private:
    Storage& storage_;
};

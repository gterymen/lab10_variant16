#pragma once

#include <stdexcept>
#include <string>

#include <sqlite_orm/sqlite_orm.h>

#include "NotificationCategory.h"
#include "Notification.h"
#include "NotificationStatusHistory.h"

// ORM mapping: structures -> tables, fields -> columns.
inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,

        make_table(
            "NotificationCategory",
            make_column("id", &NotificationCategory::id, primary_key().autoincrement()),
            make_column("name", &NotificationCategory::name, not_null()),
            make_column("channel", &NotificationCategory::channel, not_null()),
            make_column("description", &NotificationCategory::description, not_null()),
            make_column("status", &NotificationCategory::status, not_null())
        ),

        make_table(
            "Notification",
            make_column("id", &Notification::id, primary_key().autoincrement()),
            make_column("categoryId", &Notification::categoryId, not_null()),
            make_column("createdAt", &Notification::createdAt, not_null()),
            make_column("message", &Notification::message, not_null()),
            make_column("status", &Notification::status, not_null()),
            make_column("deliveryMs", &Notification::deliveryMs, not_null()),
            make_column("importance", &Notification::importance, not_null()),
            make_column("urgency", &Notification::urgency, not_null()),
            make_column("waitingMinutes", &Notification::waitingMinutes, not_null()),
            foreign_key(&Notification::categoryId).references(&NotificationCategory::id)
        ),

        make_table(
            "NotificationStatusHistory",
            make_column("id", &NotificationStatusHistory::id, primary_key().autoincrement()),
            make_column("notificationId", &NotificationStatusHistory::notificationId, not_null()),
            make_column("fromStatus", &NotificationStatusHistory::fromStatus, not_null()),
            make_column("toStatus", &NotificationStatusHistory::toStatus, not_null()),
            make_column("changedAt", &NotificationStatusHistory::changedAt, not_null()),
            foreign_key(&NotificationStatusHistory::notificationId).references(&Notification::id)
        )
    );
}

using Storage = decltype(createStorage(""));

// Enables foreign key checks for the connection, keeps the connection open
// and synchronizes the schema (creates the tables if they do not exist).
inline void configureStorage(Storage& storage)
{
    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign key constraints");
        }
    };
    storage.open_forever();
    storage.sync_schema();
}

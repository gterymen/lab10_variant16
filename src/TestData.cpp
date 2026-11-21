#include "TestData.h"

#include <filesystem>
#include <iostream>
#include <vector>

#include "Database.h"

void prepareTestData(const std::string& databaseFile)
{
    // Only the separate demonstration database is removed, so the demo can be re-run.
    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);
    configureStorage(storage);

    const std::vector<NotificationCategory> items = {
        {0, "Security alerts", "email", "Account security events", "active"},
        {0, "Order updates", "push", "Order status changes", "active"},
        {0, "Promotions", "email", "Marketing campaigns", "active"},
        {0, "System maintenance", "sms", "Planned downtime notices", "active"},
        {0, "Reminders", "push", "Calendar reminders", "disabled"}
    };

    std::vector<int> ids;
    for (const auto& item : items)
    {
        ids.push_back(storage.insert(item));
    }

    const std::vector<Notification> records = {
        {0, ids[0], "2026-11-01", "Login from a new device", "Delivered", 420.0},
        {0, ids[0], "2026-11-01", "Password was changed", "Delivered", 380.0},
        {0, ids[0], "2026-11-02", "Suspicious activity detected", "DeliveryError", 1500.0},
        {0, ids[1], "2026-11-01", "Order 1001 was shipped", "Delivered", 650.0},
        {0, ids[1], "2026-11-02", "Order 1002 was delivered", "Delivered", 700.0},
        {0, ids[1], "2026-11-03", "Order 1003 was cancelled", "Created", 0.0},
        {0, ids[2], "2026-11-01", "Autumn sale starts today", "Delivered", 1200.0},
        {0, ids[2], "2026-11-02", "Your discount expires soon", "DeliveryError", 1800.0},
        {0, ids[2], "2026-11-03", "New collection is available", "Delivered", 950.0},
        {0, ids[3], "2026-11-01", "Maintenance on Saturday", "Delivered", 300.0},
        {0, ids[3], "2026-11-02", "Service is back online", "Delivered", 280.0},
        {0, ids[3], "2026-11-03", "Planned update tonight", "Created", 0.0},
        {0, ids[4], "2026-11-01", "Meeting at 10:00", "Delivered", 150.0},
        {0, ids[4], "2026-11-02", "Dentist appointment", "DeliveryError", 2100.0},
        {0, ids[4], "2026-11-03", "Pay the invoice", "Delivered", 210.0}
    };

    for (const auto& record : records)
    {
        storage.insert(record);
    }

    std::cout << "Test database prepared: " << databaseFile << '\n';
    std::cout << "NotificationCategory records: " << storage.count<NotificationCategory>() << '\n';
    std::cout << "Notification records: " << storage.count<Notification>() << '\n';
}

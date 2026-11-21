#pragma once

#include <string>

// Recreates the database file and fills it with the controlled test data set:
// 5 records of NotificationCategory and 15 records of Notification.
void prepareTestData(const std::string& databaseFile);

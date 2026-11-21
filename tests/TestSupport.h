#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Commands.h"
#include "Database.h"
#include "NotificationDao.h"
#include "NotificationEventPublisher.h"
#include "NotificationHistoryDao.h"
#include "NotificationWorkflow.h"

// In-memory database, one category and a counter instead of the real clock,
// so that the tests are fast and deterministic.
struct WorkflowFixture : ::testing::Test
{
    WorkflowFixture()
        : storage(createStorage(":memory:")),
          notifications((configureStorage(storage), storage)),
          history(storage),
          workflow(storage, notifications, history, publisher, [this] { return "t" + std::to_string(++tick); })
    {
        storage.insert(NotificationCategory{0, "Security alerts", "email", "Account security events", "active"});
    }

    int newItem(int importance = 3, int urgency = 3, int waiting = 0)
    {
        return workflow.createNotification({0, 1, "2026-11-01", "Message", "", 0.0, importance, urgency, waiting});
    }

    std::string statusOf(int id) { return notifications.findById(id)->status; }

    Storage storage;
    NotificationDao notifications;
    NotificationHistoryDao history;
    NotificationEventPublisher publisher;
    NotificationWorkflow workflow;
    int tick = 0;
};

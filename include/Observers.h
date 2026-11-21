#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "NotificationEventPublisher.h"

// Subscriber 1: notification journal. Keeps a text record of every event.
class NotificationJournal : public INotificationObserver
{
public:
    explicit NotificationJournal(std::ostream* out = nullptr);

    void onEvent(const NotificationEvent& event) override;
    const std::vector<std::string>& entries() const { return entries_; }

private:
    std::ostream* out_;
    std::vector<std::string> entries_;
};

// Subscriber 2: delivery statistics. Counts events independently of the journal.
class DeliveryStatistics : public INotificationObserver
{
public:
    void onEvent(const NotificationEvent& event) override;

    int createdCount() const { return created_; }
    int deliveredCount() const { return delivered_; }
    int failedCount() const { return failed_; }
    std::string summary() const;

private:
    int created_ = 0;
    int delivered_ = 0;
    int failed_ = 0;
};

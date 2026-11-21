#include <iomanip>
#include <iostream>
#include <memory>

#include "Commands.h"
#include "Database.h"
#include "NotificationDao.h"
#include "NotificationHistoryDao.h"
#include "NotificationWorkflow.h"
#include "Observers.h"
#include "PriorityService.h"
#include "TestData.h"

namespace
{
    void printHistory(NotificationHistoryDao& history, int id)
    {
        std::cout << "History of notification #" << id << ":\n";
        for (const auto& record : history.findByNotification(id))
        {
            std::cout << "  " << (record.fromStatus.empty() ? "(none)" : record.fromStatus)
                      << " -> " << record.toStatus << "  at " << record.changedAt << '\n';
        }
    }

    void printLog(const CommandInvoker& invoker)
    {
        for (const auto& item : invoker.log())
        {
            std::cout << "  " << item.description << ": "
                      << (item.success ? "OK" : "ERROR - " + item.error) << '\n';
        }
    }
}

int main()
{
    const std::string file = "notifications_lab10.db";
    prepareTestData(file);

    auto storage = createStorage(file);
    configureStorage(storage);

    NotificationDao notifications(storage);
    NotificationHistoryDao history(storage);
    NotificationEventPublisher publisher;
    NotificationWorkflow workflow(storage, notifications, history, publisher);

    std::cout << "\n=== 1. Observer: two independent subscribers ===\n";
    auto journal = std::make_shared<NotificationJournal>(&std::cout);
    auto statistics = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(journal);
    publisher.subscribe(statistics);
    std::cout << "Active subscribers: " << publisher.subscriberCount() << '\n';

    // importance, urgency, waiting minutes
    const int low = workflow.createNotification({0, 1, "2026-11-04", "Newsletter", "", 0.0, 1, 2, 5});
    const int high = workflow.createNotification({0, 1, "2026-11-04", "Account blocked", "", 0.0, 5, 5, 0});
    const int old = workflow.createNotification({0, 2, "2026-11-04", "Order reminder", "", 0.0, 2, 1, 90});

    std::cout << "\n=== 2. Strategy: priority of the new notifications ===\n";
    PriorityService priority(notifications, std::make_unique<MaxPriority>());
    std::cout << std::fixed << std::setprecision(3);
    for (int id : {low, high, old})
    {
        std::cout << "Notification #" << id << ":";
        priority.setStrategy(std::make_unique<MaxPriority>());
        std::cout << "  " << priority.strategyName() << " = " << priority.priorityOf(id);
        priority.setStrategy(std::make_unique<WeightedSumPriority>());
        std::cout << " | " << priority.strategyName() << " = " << priority.priorityOf(id);
        priority.setStrategy(std::make_unique<NormalizedPriority>());
        std::cout << " | " << priority.strategyName() << " = " << priority.priorityOf(id) << '\n';
    }
    std::cout << "Queue of Created notifications (" << priority.strategyName() << "):\n";
    for (const auto& [id, value] : priority.rankCreated())
    {
        std::cout << "  #" << id << "  " << value << '\n';
    }

    std::cout << "\n=== 3. Command: queue of operations ===\n";
    CommandInvoker invoker;
    for (const auto& [id, value] : priority.rankCreated())
    {
        invoker.submit(std::make_unique<SendCommand>(workflow, id));
        (void)value;
    }
    std::cout << "Queued commands: " << invoker.pendingCount() << '\n';
    int ok = invoker.runAll();
    std::cout << "Executed successfully: " << ok << '\n';
    printLog(invoker);

    std::cout << "\n=== 4. Delivery results, retry after an error ===\n";
    CommandInvoker results;
    results.submit(std::make_unique<DeliverCommand>(workflow, high));
    results.submit(std::make_unique<FailCommand>(workflow, low));
    results.submit(std::make_unique<DeliverCommand>(workflow, old));
    results.submit(std::make_unique<SendCommand>(workflow, low));      // retry
    results.submit(std::make_unique<DeliverCommand>(workflow, low));
    results.runAll();
    printLog(results);
    printHistory(history, low);

    std::cout << "\n=== 5. Invalid transitions are rejected ===\n";
    CommandInvoker wrong;
    wrong.submit(std::make_unique<DeliverCommand>(workflow, high));   // already Delivered
    wrong.submit(std::make_unique<SendCommand>(workflow, high));      // already Delivered
    wrong.submit(std::make_unique<FailCommand>(workflow, 999));       // unknown notification
    wrong.submit(std::make_unique<DeliverCommand>(workflow, 6));      // allowed, runs after errors
    const int wrongOk = wrong.runAll();
    std::cout << "Executed successfully: " << wrongOk << " of 4\n";
    printLog(wrong);

    std::cout << "\n=== 6. Observer: unsubscribe ===\n";
    publisher.unsubscribe(statistics);
    std::cout << "Active subscribers: " << publisher.subscriberCount() << '\n';
    workflow.deliver(12);     // only the journal receives this event

    std::cout << "\n=== 7. Subscribers' state ===\n";
    std::cout << "Journal entries: " << journal->entries().size() << '\n';
    std::cout << "Statistics: " << statistics->summary() << '\n';
    std::cout << "Total history records: " << history.countAll() << '\n';
    return 0;
}

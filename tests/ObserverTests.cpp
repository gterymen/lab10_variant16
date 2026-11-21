#include "TestSupport.h"

#include "Observers.h"

namespace {

TEST_F(WorkflowFixture, BothSubscribersReceiveTheCreationEvent) {
    auto journal = std::make_shared<NotificationJournal>();
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(journal);
    publisher.subscribe(stats);
    newItem();
    ASSERT_EQ(1u, journal->entries().size());
    EXPECT_EQ("NotificationCreated: notification #1, category #1", journal->entries()[0]);
    EXPECT_EQ(1, stats->createdCount());
}

TEST_F(WorkflowFixture, DeliveryEventsReportSuccessAndError) {
    auto journal = std::make_shared<NotificationJournal>();
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(journal);
    publisher.subscribe(stats);
    int a = newItem();
    int b = newItem();
    workflow.send(a);
    workflow.send(b);
    workflow.deliver(a);
    workflow.fail(b);
    EXPECT_EQ(1, stats->deliveredCount());
    EXPECT_EQ(1, stats->failedCount());
    EXPECT_EQ("DeliveryCompleted: notification #1 delivered", journal->entries()[2]);
    EXPECT_EQ("DeliveryCompleted: notification #2 delivery error", journal->entries()[3]);
}

TEST_F(WorkflowFixture, SendingDoesNotPublishDeliveryEvents) {
    auto stats = std::make_shared<DeliveryStatistics>();
    int id = newItem();
    publisher.subscribe(stats);
    workflow.send(id);
    EXPECT_EQ(0, stats->deliveredCount() + stats->failedCount());
}

TEST_F(WorkflowFixture, UnsubscribedObserverStopsReceivingEvents) {
    auto journal = std::make_shared<NotificationJournal>();
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(journal);
    publisher.subscribe(stats);
    newItem();
    publisher.unsubscribe(stats);
    newItem();
    EXPECT_EQ(2u, journal->entries().size());
    EXPECT_EQ(1, stats->createdCount());
    EXPECT_EQ(1u, publisher.subscriberCount());
}

TEST_F(WorkflowFixture, SubscriptionCanBeRestoredAfterUnsubscribing) {
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(stats);
    publisher.unsubscribe(stats);
    publisher.subscribe(stats);
    newItem();
    EXPECT_EQ(1, stats->createdCount());
}

TEST_F(WorkflowFixture, SubscribingTwiceDoesNotDuplicateNotifications) {
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(stats);
    publisher.subscribe(stats);
    newItem();
    EXPECT_EQ(1, stats->createdCount());
    EXPECT_EQ(1u, publisher.subscriberCount());
}

TEST_F(WorkflowFixture, DestroyedSubscriberIsRemovedAutomatically) {
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(stats);
    EXPECT_EQ(1u, publisher.subscriberCount());
    stats.reset();
    EXPECT_EQ(0u, publisher.subscriberCount());
    EXPECT_NO_THROW(newItem());
}

class ThrowingObserver : public INotificationObserver {
public:
    void onEvent(const NotificationEvent&) override { throw std::runtime_error("observer failed"); }
};

TEST_F(WorkflowFixture, FailingObserverDoesNotBlockOthers) {
    auto bad = std::make_shared<ThrowingObserver>();
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(bad);
    publisher.subscribe(stats);
    EXPECT_NO_THROW(newItem());
    EXPECT_EQ(1, stats->createdCount());
    EXPECT_EQ(1, publisher.failedDeliveries());
}

TEST_F(WorkflowFixture, NoEventIsPublishedForAnInvalidTransition) {
    auto stats = std::make_shared<DeliveryStatistics>();
    int id = newItem();
    publisher.subscribe(stats);
    EXPECT_THROW(workflow.deliver(id), InvalidTransitionError);
    EXPECT_EQ(0, stats->deliveredCount() + stats->failedCount());
}

TEST_F(WorkflowFixture, ObserverCanReceiveEventsDirectlyWithoutWorkflow) {
    auto journal = std::make_shared<NotificationJournal>();
    publisher.subscribe(journal);
    publisher.publish(DeliveryCompleted{7, true});
    ASSERT_EQ(1u, journal->entries().size());
    EXPECT_EQ("DeliveryCompleted: notification #7 delivered", journal->entries()[0]);
}

}  // namespace

#include "TestSupport.h"

namespace {

TEST(NotificationContextTest, FollowsTheMainPathWithoutDatabase) {
    NotificationContext context(std::make_unique<CreatedState>());
    EXPECT_EQ("Created", context.stateName());
    context.send();
    EXPECT_EQ("Sent", context.stateName());
    context.deliver();
    EXPECT_EQ("Delivered", context.stateName());
}

TEST(NotificationContextTest, NullInitialStateIsRejected) {
    EXPECT_THROW(NotificationContext(nullptr), std::invalid_argument);
}

TEST(NotificationContextTest, ErrorAndRetryWithoutDatabase) {
    NotificationContext context(std::make_unique<SentState>());
    context.fail();
    EXPECT_EQ("DeliveryError", context.stateName());
    context.send();
    EXPECT_EQ("Sent", context.stateName());
}

TEST(NotificationContextTest, CreatedAllowsOnlySending) {
    NotificationContext context(std::make_unique<CreatedState>());
    EXPECT_THROW(context.deliver(), InvalidTransitionError);
    EXPECT_THROW(context.fail(), InvalidTransitionError);
    EXPECT_EQ("Created", context.stateName());
}

TEST(NotificationContextTest, DeliveredIsFinal) {
    NotificationContext context(std::make_unique<DeliveredState>());
    EXPECT_THROW(context.send(), InvalidTransitionError);
    EXPECT_THROW(context.deliver(), InvalidTransitionError);
    EXPECT_THROW(context.fail(), InvalidTransitionError);
}

TEST(NotificationContextTest, MakeStateKnowsAllStatusesAndRejectsUnknown) {
    for (const char* name : {"Created", "Sent", "Delivered", "DeliveryError"}) {
        EXPECT_EQ(name, makeState(name)->name());
    }
    EXPECT_THROW(makeState("Archived"), std::invalid_argument);
}

TEST_F(WorkflowFixture, NewNotificationGetsStatusAndFirstHistoryRecord) {
    int id = newItem();
    EXPECT_EQ("Created", statusOf(id));
    auto records = history.findByNotification(id);
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ("", records[0].fromStatus);
    EXPECT_EQ("Created", records[0].toStatus);
}

TEST_F(WorkflowFixture, MainPathIsStoredWithHistory) {
    int id = newItem();
    workflow.send(id);
    workflow.deliver(id);
    EXPECT_EQ("Delivered", statusOf(id));
    auto records = history.findByNotification(id);
    ASSERT_EQ(3u, records.size());
    EXPECT_EQ("Created", records[1].fromStatus);
    EXPECT_EQ("Sent", records[1].toStatus);
    EXPECT_EQ("Sent", records[2].fromStatus);
    EXPECT_EQ("Delivered", records[2].toStatus);
}

TEST_F(WorkflowFixture, DeliveryErrorCanBeRetried) {
    int id = newItem();
    workflow.send(id);
    workflow.fail(id);
    EXPECT_EQ("DeliveryError", statusOf(id));
    workflow.send(id);
    workflow.deliver(id);
    EXPECT_EQ("Delivered", statusOf(id));
    EXPECT_EQ(5u, history.findByNotification(id).size());
}

TEST_F(WorkflowFixture, DeliveredNotificationCannotBeChanged) {
    int id = newItem();
    workflow.send(id);
    workflow.deliver(id);
    EXPECT_THROW(workflow.send(id), InvalidTransitionError);
    EXPECT_THROW(workflow.deliver(id), InvalidTransitionError);
    EXPECT_THROW(workflow.fail(id), InvalidTransitionError);
    EXPECT_EQ("Delivered", statusOf(id));
}

TEST_F(WorkflowFixture, DeliveringACreatedNotificationChangesNothing) {
    int id = newItem();
    int before = history.countAll();
    EXPECT_THROW(workflow.deliver(id), InvalidTransitionError);
    EXPECT_EQ("Created", statusOf(id));
    EXPECT_EQ(before, history.countAll());
}

TEST_F(WorkflowFixture, SendingTwiceIsNotAllowed) {
    int id = newItem();
    workflow.send(id);
    EXPECT_THROW(workflow.send(id), InvalidTransitionError);
    EXPECT_EQ("Sent", statusOf(id));
}

TEST_F(WorkflowFixture, UnknownNotificationIsReported) {
    EXPECT_THROW(workflow.send(999), std::out_of_range);
}

TEST_F(WorkflowFixture, UnknownStoredStatusStopsTheOperation) {
    int id = newItem();
    notifications.updateStatus(id, "Archived");
    EXPECT_THROW(workflow.send(id), std::invalid_argument);
    EXPECT_EQ("Archived", statusOf(id));
}

TEST_F(WorkflowFixture, HistoryUsesTheInjectedClock) {
    int id = newItem();
    workflow.send(id);
    auto records = history.findByNotification(id);
    EXPECT_EQ("t1", records[0].changedAt);
    EXPECT_EQ("t2", records[1].changedAt);
}

// Atomicity: the history write fails after the status was updated inside the transaction.
class FailingHistoryDao : public NotificationHistoryDao {
public:
    using NotificationHistoryDao::NotificationHistoryDao;
    int add(const NotificationStatusHistory&) override { throw std::runtime_error("history is unavailable"); }
};

TEST_F(WorkflowFixture, FailedHistoryWriteRollsBackTheStatusChange) {
    int id = newItem();
    FailingHistoryDao failing(storage);
    NotificationWorkflow broken(storage, notifications, failing, publisher);
    EXPECT_THROW(broken.send(id), std::runtime_error);
    EXPECT_EQ("Created", statusOf(id));
    EXPECT_EQ(1u, history.findByNotification(id).size());
}

TEST_F(WorkflowFixture, FailedCreationLeavesNoNotificationInTheDatabase) {
    FailingHistoryDao failing(storage);
    NotificationWorkflow broken(storage, notifications, failing, publisher);
    int before = storage.count<Notification>();
    EXPECT_THROW(broken.createNotification({0, 1, "2026-11-01", "x", "", 0.0, 3, 3, 0}), std::runtime_error);
    EXPECT_EQ(before, storage.count<Notification>());
}

TEST_F(WorkflowFixture, HistoryOfOneNotificationIsNotMixedWithOthers) {
    int first = newItem();
    int second = newItem();
    workflow.send(first);
    EXPECT_EQ(2u, history.findByNotification(first).size());
    EXPECT_EQ(1u, history.findByNotification(second).size());
}

}  // namespace

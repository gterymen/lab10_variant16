#include "TestSupport.h"

#include "Observers.h"

namespace {

TEST_F(WorkflowFixture, CommandsAreNotExecutedUntilTheInvokerRuns) {
    int id = newItem();
    CommandInvoker invoker;
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    EXPECT_EQ(1u, invoker.pendingCount());
    EXPECT_EQ("Created", statusOf(id));
    EXPECT_EQ(1, invoker.runAll());
    EXPECT_EQ("Sent", statusOf(id));
    EXPECT_EQ(0u, invoker.pendingCount());
}

TEST_F(WorkflowFixture, CommandsRunInTheSubmittedOrder) {
    int id = newItem();
    CommandInvoker invoker;
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    invoker.submit(std::make_unique<DeliverCommand>(workflow, id));
    EXPECT_EQ(2, invoker.runAll());
    EXPECT_EQ("Delivered", statusOf(id));
    ASSERT_EQ(2u, invoker.log().size());
    EXPECT_EQ("send #1", invoker.log()[0].description);
    EXPECT_EQ("deliver #1", invoker.log()[1].description);
}

TEST_F(WorkflowFixture, WrongOrderIsReportedAndDoesNotStopOtherCommands) {
    int id = newItem();
    CommandInvoker invoker;
    invoker.submit(std::make_unique<DeliverCommand>(workflow, id));   // not allowed in Created
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    EXPECT_EQ(1, invoker.runAll());
    ASSERT_EQ(2u, invoker.log().size());
    EXPECT_FALSE(invoker.log()[0].success);
    EXPECT_NE(std::string::npos, invoker.log()[0].error.find("Created"));
    EXPECT_TRUE(invoker.log()[1].success);
    EXPECT_EQ("Sent", statusOf(id));
}

TEST_F(WorkflowFixture, CommandForAnUnknownNotificationFailsWithMessage) {
    CommandInvoker invoker;
    invoker.submit(std::make_unique<FailCommand>(workflow, 999));
    EXPECT_EQ(0, invoker.runAll());
    EXPECT_EQ("Notification not found: 999", invoker.log()[0].error);
}

TEST_F(WorkflowFixture, ExecutedCommandsAreNotRepeated) {
    int id = newItem();
    CommandInvoker invoker;
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    invoker.runAll();
    EXPECT_EQ(0, invoker.runAll());
    EXPECT_EQ(1u, invoker.log().size());
}

TEST_F(WorkflowFixture, QueueCanBeClearedBeforeExecution) {
    int id = newItem();
    CommandInvoker invoker;
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    invoker.clearQueue();
    EXPECT_EQ(0, invoker.runAll());
    EXPECT_EQ("Created", statusOf(id));
}

TEST_F(WorkflowFixture, NullCommandIsRejected) {
    CommandInvoker invoker;
    EXPECT_THROW(invoker.submit(nullptr), std::invalid_argument);
}

TEST_F(WorkflowFixture, RetryAfterErrorIsJustAnotherCommand) {
    int id = newItem();
    CommandInvoker invoker;
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    invoker.submit(std::make_unique<FailCommand>(workflow, id));
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    invoker.submit(std::make_unique<DeliverCommand>(workflow, id));
    EXPECT_EQ(4, invoker.runAll());
    EXPECT_EQ("Delivered", statusOf(id));
}

class CountingCommand : public Command {
public:
    explicit CountingCommand(int& counter) : counter_(counter) {}
    void execute() override { ++counter_; }
    std::string description() const override { return "count"; }
private:
    int& counter_;
};

TEST(CommandPattern, InvokerWorksWithAnyCommandImplementation) {
    int counter = 0;
    CommandInvoker invoker;
    invoker.submit(std::make_unique<CountingCommand>(counter));
    invoker.submit(std::make_unique<CountingCommand>(counter));
    EXPECT_EQ(2, invoker.runAll());
    EXPECT_EQ(2, counter);
}

TEST_F(WorkflowFixture, CommandsPublishEventsThroughTheWorkflow) {
    auto stats = std::make_shared<DeliveryStatistics>();
    publisher.subscribe(stats);
    int id = newItem();
    CommandInvoker invoker;
    invoker.submit(std::make_unique<SendCommand>(workflow, id));
    invoker.submit(std::make_unique<DeliverCommand>(workflow, id));
    invoker.runAll();
    EXPECT_EQ(1, stats->createdCount());
    EXPECT_EQ(1, stats->deliveredCount());
}

}  // namespace

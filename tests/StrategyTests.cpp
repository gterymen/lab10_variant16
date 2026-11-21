#include "TestSupport.h"

#include "PriorityService.h"
#include "PriorityStrategy.h"

namespace {

TEST(PriorityStrategies, MaximumTakesTheLargestScore) {
    MaxPriority strategy;
    EXPECT_DOUBLE_EQ(4.0, strategy.calculate({4, 2, 0}));
    EXPECT_DOUBLE_EQ(5.0, strategy.calculate({1, 5, 0}));
    EXPECT_DOUBLE_EQ(5.0, strategy.calculate({1, 1, 60}));     // waiting 60 min = score 5
}

TEST(PriorityStrategies, WeightedSumUsesTheWeights) {
    WeightedSumPriority strategy;
    // 0.5*4 + 0.3*2 + 0.2*(30/12)
    EXPECT_NEAR(2.0 + 0.6 + 0.5, strategy.calculate({4, 2, 30}), 1e-12);
}

TEST(PriorityStrategies, NormalizedIsWeightedSumDividedByFive) {
    WeightedSumPriority weighted;
    NormalizedPriority normalized;
    PriorityInput input{4, 2, 30};
    EXPECT_NEAR(weighted.calculate(input) / 5.0, normalized.calculate(input), 1e-12);
    EXPECT_NEAR(1.0, normalized.calculate({5, 5, 60}), 1e-12);
}

TEST(PriorityStrategies, WaitingTimeIsCapped) {
    WeightedSumPriority strategy;
    EXPECT_DOUBLE_EQ(strategy.calculate({3, 3, 60}), strategy.calculate({3, 3, 600}));
}

TEST(PriorityStrategies, AlgorithmsGiveDifferentResults) {
    PriorityInput input{5, 1, 0};
    EXPECT_NE(MaxPriority().calculate(input), WeightedSumPriority().calculate(input));
    EXPECT_NE(WeightedSumPriority().calculate(input), NormalizedPriority().calculate(input));
}

TEST(PriorityStrategies, InvalidInputIsRejectedByEveryAlgorithm) {
    std::vector<std::unique_ptr<PriorityStrategy>> all;
    all.push_back(std::make_unique<MaxPriority>());
    all.push_back(std::make_unique<WeightedSumPriority>());
    all.push_back(std::make_unique<NormalizedPriority>());
    for (const auto& strategy : all) {
        EXPECT_THROW(strategy->calculate({0, 3, 0}), std::invalid_argument) << strategy->name();
        EXPECT_THROW(strategy->calculate({3, 6, 0}), std::invalid_argument) << strategy->name();
        EXPECT_THROW(strategy->calculate({3, 3, -1}), std::invalid_argument) << strategy->name();
    }
}

TEST_F(WorkflowFixture, ServiceReadsTheDataThroughTheRepository) {
    int id = newItem(4, 2, 30);
    PriorityService service(notifications, std::make_unique<WeightedSumPriority>());
    EXPECT_NEAR(3.1, service.priorityOf(id), 1e-12);
    EXPECT_THROW(service.priorityOf(999), std::out_of_range);
}

class ConstantPriority : public PriorityStrategy {
public:
    double calculate(const PriorityInput&) const override { return 0.5; }
    std::string name() const override { return "Constant"; }
};

TEST_F(WorkflowFixture, StrategyCanBeReplacedAtRunTime) {
    int id = newItem(5, 5, 0);
    PriorityService service(notifications, std::make_unique<MaxPriority>());
    EXPECT_EQ("Maximum", service.strategyName());
    EXPECT_DOUBLE_EQ(5.0, service.priorityOf(id));
    service.setStrategy(std::make_unique<NormalizedPriority>());
    EXPECT_EQ("Normalized weighted sum", service.strategyName());
    EXPECT_NEAR(0.8, service.priorityOf(id), 1e-12);
    service.setStrategy(std::make_unique<ConstantPriority>());
    EXPECT_DOUBLE_EQ(0.5, service.priorityOf(id));
}

TEST_F(WorkflowFixture, NullStrategyIsRejected) {
    EXPECT_THROW(PriorityService(notifications, nullptr), std::invalid_argument);
    PriorityService service(notifications, std::make_unique<MaxPriority>());
    EXPECT_THROW(service.setStrategy(nullptr), std::invalid_argument);
}

TEST_F(WorkflowFixture, RankingUsesOnlyCreatedNotificationsAndSortsByPriority) {
    int low = newItem(1, 1, 0);
    int high = newItem(5, 5, 0);
    int middle = newItem(3, 3, 0);
    int sent = newItem(5, 5, 60);
    workflow.send(sent);
    PriorityService service(notifications, std::make_unique<WeightedSumPriority>());
    auto ranking = service.rankCreated();
    ASSERT_EQ(3u, ranking.size());
    EXPECT_EQ(high, ranking[0].first);
    EXPECT_EQ(middle, ranking[1].first);
    EXPECT_EQ(low, ranking[2].first);
}

TEST_F(WorkflowFixture, RankingCanChangeWhenTheStrategyChanges) {
    int a = newItem(5, 1, 0);     // max 5, weighted 2.8
    int b = newItem(3, 3, 60);    // max 3, weighted 3.1 ... waiting score 5
    PriorityService service(notifications, std::make_unique<MaxPriority>());
    EXPECT_EQ(a, service.rankCreated()[0].first);       // 5 vs 5 -> stable order, a first
    service.setStrategy(std::make_unique<WeightedSumPriority>());
    EXPECT_EQ(b, service.rankCreated()[0].first);
}

}  // namespace

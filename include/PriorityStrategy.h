#pragma once

#include <string>

// Input of the priority algorithms.
struct PriorityInput
{
    int importance = 3;        // 1..5
    int urgency = 3;           // 1..5
    int waitingMinutes = 0;    // >= 0, values above kMaxWaitingMinutes do not increase the score
};

constexpr int kMaxWaitingMinutes = 60;

// Strategy (pattern): common interface of the interchangeable algorithms.
class PriorityStrategy
{
public:
    virtual ~PriorityStrategy() = default;
    virtual double calculate(const PriorityInput& input) const = 0;
    virtual std::string name() const = 0;
};

// All algorithms work with three scores in the range 0..5:
// importance, urgency and waiting time (waitingMinutes / 12, at most 5).

// 1. Maximum of the three scores (range 0..5).
class MaxPriority : public PriorityStrategy
{
public:
    double calculate(const PriorityInput& input) const override;
    std::string name() const override { return "Maximum"; }
};

// 2. Weighted sum 0.5*importance + 0.3*urgency + 0.2*waiting (range 0..5).
class WeightedSumPriority : public PriorityStrategy
{
public:
    double calculate(const PriorityInput& input) const override;
    std::string name() const override { return "Weighted sum"; }
};

// 3. Weighted sum divided by 5 (range 0..1).
class NormalizedPriority : public PriorityStrategy
{
public:
    double calculate(const PriorityInput& input) const override;
    std::string name() const override { return "Normalized weighted sum"; }
};

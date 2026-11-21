#include "PriorityStrategy.h"

#include <algorithm>
#include <stdexcept>

namespace
{
    struct Scores
    {
        double importance;
        double urgency;
        double waiting;
    };

    Scores scoresOf(const PriorityInput& input)
    {
        if (input.importance < 1 || input.importance > 5)
            throw std::invalid_argument("Importance must be in the range 1..5");
        if (input.urgency < 1 || input.urgency > 5)
            throw std::invalid_argument("Urgency must be in the range 1..5");
        if (input.waitingMinutes < 0)
            throw std::invalid_argument("Waiting time must not be negative");

        const int waiting = std::min(input.waitingMinutes, kMaxWaitingMinutes);
        return {static_cast<double>(input.importance),
                static_cast<double>(input.urgency),
                waiting / 12.0};
    }

    double weightedSum(const Scores& s)
    {
        return 0.5 * s.importance + 0.3 * s.urgency + 0.2 * s.waiting;
    }
}

double MaxPriority::calculate(const PriorityInput& input) const
{
    const Scores s = scoresOf(input);
    return std::max({s.importance, s.urgency, s.waiting});
}

double WeightedSumPriority::calculate(const PriorityInput& input) const
{
    return weightedSum(scoresOf(input));
}

double NormalizedPriority::calculate(const PriorityInput& input) const
{
    return weightedSum(scoresOf(input)) / 5.0;
}

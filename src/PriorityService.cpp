#include "PriorityService.h"

#include <algorithm>
#include <stdexcept>

#include "NotificationState.h"

namespace
{
    PriorityInput inputOf(const Notification& n)
    {
        return {n.importance, n.urgency, n.waitingMinutes};
    }
}

PriorityService::PriorityService(NotificationDao& notifications, std::unique_ptr<PriorityStrategy> strategy)
    : notifications_(notifications), strategy_(std::move(strategy))
{
    if (!strategy_)
    {
        throw std::invalid_argument("Strategy must not be null");
    }
}

void PriorityService::setStrategy(std::unique_ptr<PriorityStrategy> strategy)
{
    if (!strategy)
    {
        throw std::invalid_argument("Strategy must not be null");
    }
    strategy_ = std::move(strategy);
}

std::string PriorityService::strategyName() const
{
    return strategy_->name();
}

double PriorityService::priorityOf(int notificationId)
{
    auto item = notifications_.findById(notificationId);
    if (!item)
    {
        throw std::out_of_range("Notification not found: " + std::to_string(notificationId));
    }
    return strategy_->calculate(inputOf(*item));
}

std::vector<std::pair<int, double>> PriorityService::rankCreated()
{
    std::vector<std::pair<int, double>> result;
    for (const auto& item : notifications_.findByStatus(NotificationStatus::Created))
    {
        result.emplace_back(item.id, strategy_->calculate(inputOf(item)));
    }
    std::stable_sort(result.begin(), result.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    return result;
}

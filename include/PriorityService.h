#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "NotificationDao.h"
#include "PriorityStrategy.h"

// Context of the Strategy pattern. The input data is read through the repository (DAO),
// the algorithm can be replaced at run time.
class PriorityService
{
public:
    PriorityService(NotificationDao& notifications, std::unique_ptr<PriorityStrategy> strategy);

    void setStrategy(std::unique_ptr<PriorityStrategy> strategy);
    std::string strategyName() const;

    // Priority of one notification (std::out_of_range if it does not exist).
    double priorityOf(int notificationId);

    // Notifications waiting for sending (Created) ordered by priority, the highest first.
    std::vector<std::pair<int, double>> rankCreated();

private:
    NotificationDao& notifications_;
    std::unique_ptr<PriorityStrategy> strategy_;
};

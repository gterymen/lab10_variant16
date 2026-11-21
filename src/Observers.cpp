#include "Observers.h"

#include <ostream>
#include <type_traits>

NotificationJournal::NotificationJournal(std::ostream* out)
    : out_(out)
{
}

void NotificationJournal::onEvent(const NotificationEvent& event)
{
    std::string line = std::visit([](const auto& e) -> std::string
    {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, NotificationCreated>)
        {
            return "NotificationCreated: notification #" + std::to_string(e.notificationId) +
                   ", category #" + std::to_string(e.categoryId);
        }
        else
        {
            return "DeliveryCompleted: notification #" + std::to_string(e.notificationId) +
                   (e.success ? " delivered" : " delivery error");
        }
    }, event);

    entries_.push_back(line);
    if (out_)
    {
        *out_ << "[journal] " << line << '\n';
    }
}

void DeliveryStatistics::onEvent(const NotificationEvent& event)
{
    if (std::holds_alternative<NotificationCreated>(event))
    {
        ++created_;
    }
    else if (const auto* completed = std::get_if<DeliveryCompleted>(&event))
    {
        completed->success ? ++delivered_ : ++failed_;
    }
}

std::string DeliveryStatistics::summary() const
{
    return "created: " + std::to_string(created_) +
           ", delivered: " + std::to_string(delivered_) +
           ", delivery errors: " + std::to_string(failed_);
}

#include "NotificationState.h"

namespace
{
    [[noreturn]] void notAllowed(const char* operation, const NotificationState& state)
    {
        throw InvalidTransitionError(
            std::string("Cannot ") + operation + " a notification in state " + state.name());
    }
}

NotificationContext::NotificationContext(std::unique_ptr<NotificationState> initialState)
    : state_(std::move(initialState))
{
    if (!state_)
    {
        throw std::invalid_argument("Initial state must not be null");
    }
}

void NotificationContext::send()
{
    state_->send(*this);
    applyPendingState();
}

void NotificationContext::deliver()
{
    state_->deliver(*this);
    applyPendingState();
}

void NotificationContext::fail()
{
    state_->fail(*this);
    applyPendingState();
}

void NotificationContext::setState(std::unique_ptr<NotificationState> newState)
{
    pending_ = std::move(newState);
}

std::string NotificationContext::stateName() const
{
    return state_->name();
}

void NotificationContext::applyPendingState()
{
    if (pending_)
    {
        state_ = std::move(pending_);
    }
}

std::unique_ptr<NotificationState> makeState(const std::string& status)
{
    if (status == NotificationStatus::Created) return std::make_unique<CreatedState>();
    if (status == NotificationStatus::Sent) return std::make_unique<SentState>();
    if (status == NotificationStatus::Delivered) return std::make_unique<DeliveredState>();
    if (status == NotificationStatus::DeliveryError) return std::make_unique<DeliveryErrorState>();
    throw std::invalid_argument("Unknown notification status: " + status);
}

// Created: may only be sent.
void CreatedState::send(NotificationContext& context) const { context.setState(std::make_unique<SentState>()); }
void CreatedState::deliver(NotificationContext&) const { notAllowed("deliver", *this); }
void CreatedState::fail(NotificationContext&) const { notAllowed("fail", *this); }

// Sent: delivery ends with success or with an error.
void SentState::send(NotificationContext&) const { notAllowed("send", *this); }
void SentState::deliver(NotificationContext& context) const { context.setState(std::make_unique<DeliveredState>()); }
void SentState::fail(NotificationContext& context) const { context.setState(std::make_unique<DeliveryErrorState>()); }

// Delivered is a final state.
void DeliveredState::send(NotificationContext&) const { notAllowed("send", *this); }
void DeliveredState::deliver(NotificationContext&) const { notAllowed("deliver", *this); }
void DeliveredState::fail(NotificationContext&) const { notAllowed("fail", *this); }

// DeliveryError: only a repeated sending is allowed.
void DeliveryErrorState::send(NotificationContext& context) const { context.setState(std::make_unique<SentState>()); }
void DeliveryErrorState::deliver(NotificationContext&) const { notAllowed("deliver", *this); }
void DeliveryErrorState::fail(NotificationContext&) const { notAllowed("fail", *this); }

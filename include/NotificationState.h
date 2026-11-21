#pragma once

#include <memory>
#include <stdexcept>
#include <string>

// Names of the states of a notification (stored in Notification::status and in the history).
namespace NotificationStatus
{
    inline constexpr const char* Created = "Created";
    inline constexpr const char* Sent = "Sent";
    inline constexpr const char* Delivered = "Delivered";
    inline constexpr const char* DeliveryError = "DeliveryError";
}

// Thrown when an operation is not allowed in the current state.
class InvalidTransitionError : public std::logic_error
{
public:
    using std::logic_error::logic_error;
};

class NotificationContext;

// State (pattern): common interface of all states.
class NotificationState
{
public:
    virtual ~NotificationState() = default;

    virtual void send(NotificationContext& context) const = 0;     // Created / DeliveryError -> Sent
    virtual void deliver(NotificationContext& context) const = 0;  // Sent -> Delivered
    virtual void fail(NotificationContext& context) const = 0;     // Sent -> DeliveryError
    virtual std::string name() const = 0;
};

// Context (pattern): keeps the current state and delegates operations to it.
// A new state is applied after the operation of the old state has finished.
class NotificationContext
{
public:
    explicit NotificationContext(std::unique_ptr<NotificationState> initialState);

    void send();
    void deliver();
    void fail();

    void setState(std::unique_ptr<NotificationState> newState);
    std::string stateName() const;

private:
    void applyPendingState();

    std::unique_ptr<NotificationState> state_;
    std::unique_ptr<NotificationState> pending_;
};

// Creates the state object for a stored status name (std::invalid_argument if unknown).
std::unique_ptr<NotificationState> makeState(const std::string& status);

class CreatedState : public NotificationState
{
public:
    void send(NotificationContext& context) const override;
    void deliver(NotificationContext& context) const override;
    void fail(NotificationContext& context) const override;
    std::string name() const override { return NotificationStatus::Created; }
};

class SentState : public NotificationState
{
public:
    void send(NotificationContext& context) const override;
    void deliver(NotificationContext& context) const override;
    void fail(NotificationContext& context) const override;
    std::string name() const override { return NotificationStatus::Sent; }
};

class DeliveredState : public NotificationState
{
public:
    void send(NotificationContext& context) const override;
    void deliver(NotificationContext& context) const override;
    void fail(NotificationContext& context) const override;
    std::string name() const override { return NotificationStatus::Delivered; }
};

// After a delivery error the notification can be sent again (retry).
class DeliveryErrorState : public NotificationState
{
public:
    void send(NotificationContext& context) const override;
    void deliver(NotificationContext& context) const override;
    void fail(NotificationContext& context) const override;
    std::string name() const override { return NotificationStatus::DeliveryError; }
};

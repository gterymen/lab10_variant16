#include "Commands.h"

#include <exception>
#include <stdexcept>

SendCommand::SendCommand(NotificationWorkflow& workflow, int notificationId)
    : workflow_(workflow), notificationId_(notificationId) {}
void SendCommand::execute() { workflow_.send(notificationId_); }
std::string SendCommand::description() const { return "send #" + std::to_string(notificationId_); }

DeliverCommand::DeliverCommand(NotificationWorkflow& workflow, int notificationId)
    : workflow_(workflow), notificationId_(notificationId) {}
void DeliverCommand::execute() { workflow_.deliver(notificationId_); }
std::string DeliverCommand::description() const { return "deliver #" + std::to_string(notificationId_); }

FailCommand::FailCommand(NotificationWorkflow& workflow, int notificationId)
    : workflow_(workflow), notificationId_(notificationId) {}
void FailCommand::execute() { workflow_.fail(notificationId_); }
std::string FailCommand::description() const { return "fail #" + std::to_string(notificationId_); }

void CommandInvoker::submit(std::unique_ptr<Command> command)
{
    if (!command)
    {
        throw std::invalid_argument("Command must not be null");
    }
    queue_.push_back(std::move(command));
}

int CommandInvoker::runAll()
{
    // The queue is taken over first, so commands are never executed twice.
    auto current = std::move(queue_);
    queue_.clear();

    int succeeded = 0;
    for (auto& command : current)
    {
        CommandResult result;
        result.description = command->description();
        try
        {
            command->execute();
            result.success = true;
            ++succeeded;
        }
        catch (const std::exception& error)
        {
            result.error = error.what();
        }
        log_.push_back(std::move(result));
    }
    return succeeded;
}

void CommandInvoker::clearQueue()
{
    queue_.clear();
}

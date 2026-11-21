#pragma once

#include <memory>
#include <string>
#include <vector>

#include "NotificationWorkflow.h"

// Command (pattern): an operation of the workflow packed into an object.
class Command
{
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual std::string description() const = 0;
};

// Concrete commands. The receiver is the workflow; the command stores the target id.
class SendCommand : public Command
{
public:
    SendCommand(NotificationWorkflow& workflow, int notificationId);
    void execute() override;
    std::string description() const override;

private:
    NotificationWorkflow& workflow_;
    int notificationId_;
};

class DeliverCommand : public Command
{
public:
    DeliverCommand(NotificationWorkflow& workflow, int notificationId);
    void execute() override;
    std::string description() const override;

private:
    NotificationWorkflow& workflow_;
    int notificationId_;
};

class FailCommand : public Command
{
public:
    FailCommand(NotificationWorkflow& workflow, int notificationId);
    void execute() override;
    std::string description() const override;

private:
    NotificationWorkflow& workflow_;
    int notificationId_;
};

// Result of one executed command.
struct CommandResult
{
    std::string description;
    bool success = false;
    std::string error;   // empty if success
};

// Invoker: keeps a queue of commands, executes them in order and keeps the log of results.
// An error of one command does not stop the others.
class CommandInvoker
{
public:
    void submit(std::unique_ptr<Command> command);
    std::size_t pendingCount() const { return queue_.size(); }

    // Executes all queued commands; returns the number of successful ones.
    int runAll();

    // Drops the queued commands without executing them.
    void clearQueue();

    const std::vector<CommandResult>& log() const { return log_; }

private:
    std::vector<std::unique_ptr<Command>> queue_;
    std::vector<CommandResult> log_;
};

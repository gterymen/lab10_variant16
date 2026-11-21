# Lab 10 - Behavioral design patterns (variant 16, notification processing)

Extension of the Lab 8 project (sqlite_orm + Repository). Added:

- State - life cycle of a notification: Created -> Sent -> Delivered, plus DeliveryError (after an error the notification can be sent again); invalid transitions are rejected; every change is stored in the NotificationStatusHistory table in one SQLite transaction with the status update.
- Strategy - three interchangeable algorithms for the priority of a notification (maximum, weighted sum, normalized weighted sum of importance, urgency and waiting time); the algorithm is replaced at run time; the data comes from the repository.
- Observer - events NotificationCreated and DeliveryCompleted, two independent subscribers (notification journal, delivery statistics), subscribe/unsubscribe, weak_ptr, no new/delete.
- Command (extra pattern) - send, deliver and fail operations are command objects; CommandInvoker executes the queue and keeps a log of results.

## Build and run (x64 Native Tools Command Prompt)

```
set VCPKG_ROOT=C:\vcpkg
cmake --preset default
cmake --build build --config Release
ctest --test-dir build -C Release
build\Release\NotificationPatterns.exe
```

49 unit tests (GoogleTest) cover state transitions, atomicity (rollback), strategies, observers and commands.

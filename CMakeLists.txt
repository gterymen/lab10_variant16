cmake_minimum_required(VERSION 3.21)
project(NotificationPatterns LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/utf-8 /bigobj /Zc:__cplusplus)
endif()

enable_testing()

find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_library(NotificationCore STATIC
  src/SQLiteNotificationCategoryRepository.cpp
  src/NotificationDao.cpp
  src/NotificationHistoryDao.cpp
  src/TestData.cpp
  src/NotificationState.cpp
  src/NotificationEventPublisher.cpp
  src/Observers.cpp
  src/PriorityStrategy.cpp
  src/PriorityService.cpp
  src/NotificationWorkflow.cpp
  src/Commands.cpp
)
target_include_directories(NotificationCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NotificationCore PUBLIC sqlite_orm::sqlite_orm unofficial::sqlite3::sqlite3)

add_executable(NotificationPatterns src/main.cpp)
target_link_libraries(NotificationPatterns PRIVATE NotificationCore)

add_executable(NotificationTests
  tests/StateTests.cpp
  tests/StrategyTests.cpp
  tests/ObserverTests.cpp
  tests/CommandTests.cpp
)
target_link_libraries(NotificationTests PRIVATE NotificationCore GTest::gtest GTest::gtest_main)
gtest_discover_tests(NotificationTests)

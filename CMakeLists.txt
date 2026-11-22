cmake_minimum_required(VERSION 3.21)
project(CashRegisterLab10_V23 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_library(CashRegisterCore STATIC
    src/SQLiteCashRegisterRepository.cpp
    src/ReceiptRepository.cpp
    src/TestData.cpp
    src/ReceiptService.cpp
    src/CashDeskFacade.cpp
)
target_include_directories(CashRegisterCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(CashRegisterCore PUBLIC sqlite_orm::sqlite_orm unofficial::sqlite3::sqlite3)

add_executable(CashRegisterLab10_V23 src/main.cpp)
target_link_libraries(CashRegisterLab10_V23 PRIVATE CashRegisterCore)

add_executable(CashRegisterLab10Tests tests/Tests.cpp)
target_link_libraries(CashRegisterLab10Tests PRIVATE CashRegisterCore)

enable_testing()
add_test(NAME CashRegisterLab10Tests COMMAND CashRegisterLab10Tests)

if(MSVC)
    target_compile_options(CashRegisterCore PRIVATE /W4 /utf-8)
    target_compile_options(CashRegisterLab10_V23 PRIVATE /W4 /utf-8)
    target_compile_options(CashRegisterLab10Tests PRIVATE /W4 /utf-8)
else()
    target_compile_options(CashRegisterCore PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(CashRegisterLab10_V23 PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(CashRegisterLab10Tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

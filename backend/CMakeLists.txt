cmake_minimum_required(VERSION 3.20)
project(crm_backend VERSION 2.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -flto)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

# Dependencies
find_package(Crow REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(hiredis QUIET)
find_library(BCRYPT_LIBRARY bcrypt crypt REQUIRED)
find_path(PQXX_INCLUDE_DIR pqxx/pqxx REQUIRED)
find_library(PQXX_LIBRARY pqxx REQUIRED)
find_library(PQ_LIBRARY pq REQUIRED)

# Sources
set(SOURCES
    src/main.cpp
    src/core/config.cpp
    src/core/database.cpp
    src/core/jwt.cpp
    src/core/password.cpp
    src/core/middleware.cpp
    src/core/rate_limiter.cpp
    src/utils/audit.cpp
    src/utils/slugify.cpp
    src/api/auth_handler.cpp
    src/api/clients_handler.cpp
    src/api/tasks_handler.cpp
    src/api/finance_handler.cpp
    src/api/users_handler.cpp
    src/cache/redis_cache.cpp
    src/notifications/websocket.cpp
    src/email/smtp_client.cpp
    src/notifications/telegram.cpp
    src/metrics/prometheus.cpp
    src/export/csv_exporter.cpp
)

add_executable(crm_backend ${SOURCES})

target_include_directories(crm_backend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIR}
)

target_link_libraries(crm_backend PRIVATE
    Crow::Crow
    ${PQXX_LIBRARY}
    ${PQ_LIBRARY}
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    fmt::fmt
    ${BCRYPT_LIBRARY}
    pthread
)

if(hiredis_FOUND)
    target_link_libraries(crm_backend PRIVATE hiredis::hiredis)
    target_compile_definitions(crm_backend PRIVATE HAVE_REDIS=1)
endif()

enable_testing()
add_subdirectory(tests)

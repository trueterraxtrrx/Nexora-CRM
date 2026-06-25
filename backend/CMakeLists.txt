cmake_minimum_required(VERSION 3.20)
project(crm_backend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Release flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -flto)
endif()

# Warnings
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

# ─── Dependencies via find_package ───────────────────────────────────────────

# Crow (header-only HTTP framework)
find_package(Crow REQUIRED)

# libpqxx (PostgreSQL C++ client)
find_package(libpqxx REQUIRED)

# OpenSSL (JWT signing)
find_package(OpenSSL REQUIRED)

# nlohmann/json (header-only)
find_package(nlohmann_json REQUIRED)

# bcrypt
find_package(BCrypt QUIET)

# spdlog (structured logging)
find_package(spdlog REQUIRED)

# fmt
find_package(fmt REQUIRED)

# ─── Sources ─────────────────────────────────────────────────────────────────

set(SOURCES
    src/main.cpp
    src/core/config.cpp
    src/core/database.cpp
    src/core/jwt.cpp
    src/core/password.cpp
    src/core/middleware.cpp
    src/api/auth_handler.cpp
    src/api/clients_handler.cpp
    src/api/tasks_handler.cpp
    src/api/finance_handler.cpp
    src/api/users_handler.cpp
    src/models/user.cpp
    src/models/client.cpp
    src/models/task.cpp
    src/models/finance.cpp
    src/utils/slugify.cpp
    src/utils/audit.cpp
    src/utils/response.cpp
)

add_executable(crm_backend ${SOURCES})

target_include_directories(crm_backend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(crm_backend PRIVATE
    Crow::Crow
    libpqxx::pqxx
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    fmt::fmt
    pthread
)

# ─── Tests ───────────────────────────────────────────────────────────────────

enable_testing()
add_subdirectory(tests)

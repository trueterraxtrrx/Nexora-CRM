find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "GTest not found, skipping tests")
    return()
endif()

find_library(BCRYPT_LIBRARY bcrypt crypt REQUIRED)

add_executable(crm_tests
    test_jwt.cpp
    test_password.cpp
    test_slugify.cpp
)

target_include_directories(crm_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(crm_tests PRIVATE
    GTest::gtest_main
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog
    fmt::fmt
    ${BCRYPT_LIBRARY}
)

# Линкуем только нужные .cpp (без main.cpp и handlers)
target_sources(crm_tests PRIVATE
    ../src/core/jwt.cpp
    ../src/core/password.cpp
    ../src/utils/slugify.cpp
)

include(GoogleTest)
gtest_discover_tests(crm_tests)

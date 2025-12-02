cmake_minimum_required(VERSION 3.14)

# Download gtest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# # Windows
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# Include gtest module
include(GoogleTest)

# Unit tests
file(GLOB UNIT_TEST_SOURCES unit/*.cpp)
if(UNIT_TEST_SOURCES)
    add_executable(unit_tests ${UNIT_TEST_SOURCES})
    target_link_libraries(unit_tests 
        gtest_main
        gmock_main
    )
    target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    gtest_discover_tests(unit_tests)
endif()

# Integration tests
file(GLOB INTEGRATION_TEST_SOURCES integration/*.cpp)
if(INTEGRATION_TEST_SOURCES)
    add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
    target_link_libraries(integration_tests 
        gtest_main
    )
    target_include_directories(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    gtest_discover_tests(integration_tests)
endif()
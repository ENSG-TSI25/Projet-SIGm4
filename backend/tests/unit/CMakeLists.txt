cmake_minimum_required(VERSION 3.14)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Unit tests - link against the backend library
add_executable(unit_tests
    test_geometry4d.cpp
    test_datamanager.cpp
    test_geopackagereader.cpp
)

# Link against sigm4_backend 
target_link_libraries(unit_tests
    gtest_main
    sigm4_backend 
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

gtest_discover_tests(unit_tests)
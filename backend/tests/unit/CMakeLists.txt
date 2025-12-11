# Google Test
cmake_minimum_required(VERSION 3.28)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Determine project root
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# Find GDAL when configuring tests standalone so we link OGR symbols
find_package(GDAL REQUIRED)

# Source files to test
file(GLOB_RECURSE TEST_SOURCES 
    "${PROJECT_ROOT}/src/core/*.cpp"
)

# Testing executables
include(GoogleTest)

# Unit tests - link against the backend library
add_executable(unit_tests
    test_geometry4d.cpp
    test_datamanager.cpp
    test_geopackagereader.cpp
    #test_project_class.cpp
    test_proj.cpp
    test_transformationengine.cpp
    ${TEST_SOURCES}
)

# Link against sigm4_backend 
target_link_libraries(unit_tests
    gtest_main
    sigm4_backend 
)

target_include_directories(unit_tests PRIVATE
    ${PROJECT_ROOT}/include
    ${GDAL_INCLUDE_DIRS}
)

gtest_discover_tests(unit_tests)
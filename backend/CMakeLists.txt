cmake_minimum_required(VERSION 3.10)
project(BackendApp)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/**/*.cpp"
)

add_executable(backend_app ${SOURCES})

/* TESTS */

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

/* Google tests */

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)


/* Unit Tests */

file(GLOB UNIT_TEST_SOURCES unit/*.cpp)
add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests 
    gtest_main
    gmock_main
)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

/* Integration Tests */

file(GLOB INTEGRATION_TEST_SOURCES integration/*.cpp)
add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(integration_tests 
    gtest_main
)
target_include_directories(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(unit_tests)
gtest_discover_tests(integration_tests)

/* FIN TESTS */


find_package(GDAL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

target_link_libraries(backend_app 
    ${GDAL_LIBRARIES}
    ${PQXX_LIBRARIES}
)

target_include_directories(backend_app PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIRS}
)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Fichiers sources à tester
file(GLOB_RECURSE TEST_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
)

# Exécutable de test
add_executable(unit_tests
    test_geometry4d.cpp
    test_datamanager.cpp
    test_geopackagereader.cpp
    ${TEST_SOURCES}
)

target_link_libraries(unit_tests
    gtest_main
    ${GDAL_LIBRARIES}
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME AllUnitTests COMMAND unit_tests)

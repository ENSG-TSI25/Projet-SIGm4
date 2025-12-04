cmake_minimum_required(VERSION 3.10)
project(BackendLib)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(GDAL REQUIRED gdal)
pkg_check_modules(PROJ REQUIRED proj)


include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all backend sources EXCEPT main.cpp
# main.cpp will be used only for standalone testing
file(GLOB_RECURSE LIB_SOURCES 
    "src/core/*.cpp"
    "src/db/*.cpp"
    "src/Application.cpp"
)

# Create static library that frontend can link against
# This generates libsigm4_backend.a
add_library(sigm4_backend STATIC ${LIB_SOURCES})

find_package(GDAL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Link libraries as PUBLIC so frontend inherits these dependencies
# Frontend won't need to find GDAL/PQXX again
target_link_libraries(sigm4_backend PUBLIC
    ${GDAL_LIBRARIES}
    ${PQXX_LIBRARIES}
)

# Expose include directories as PUBLIC
# Frontend can directly include <core/DataManager.hpp> etc.
target_include_directories(sigm4_backend PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIRS}
    ${GDAL_INCLUDE_DIRS}
)

# This uses main.cpp and links against our library
add_executable(backend_app src/main.cpp)
target_link_libraries(backend_app sigm4_backend)

# Enable unit tests
enable_testing()
add_subdirectory(tests/unit)

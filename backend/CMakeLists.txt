cmake_minimum_required(VERSION 3.10)
project(BackendLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(GDAL REQUIRED gdal)
pkg_check_modules(PROJ REQUIRED proj)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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

find_package(PROJ REQUIRED)
find_package(GDAL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Link libraries as PUBLIC so frontend inherits these dependencies
# Frontend won't need to find GDAL/PQXX again
target_link_libraries(sigm4_backend PUBLIC
    ${GDAL_LIBRARIES}
    ${PQXX_LIBRARIES}
    PROJ::proj
)

# Expose include directories as PUBLIC
# Frontend can directly include <core/DataManager.hpp> etc.
target_include_directories(sigm4_backend PUBLIC 
<<<<<<< HEAD
    ${CMAKE_SOURCE_DIR}/include
=======
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIRS}
>>>>>>> origin/dev
    ${GDAL_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIRS}
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_executable(backend_app src/main.cpp)
    target_link_libraries(backend_app sigm4_backend)
    
    enable_testing()
    add_subdirectory(tests/unit)
endif()
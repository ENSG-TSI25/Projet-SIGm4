cmake_minimum_required(VERSION 3.10)
project(BackendLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

# Detect external libs via pkg-config (Ubuntu 22.04 compatible)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(GDAL REQUIRED gdal)
pkg_check_modules(PROJ REQUIRED proj)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Collect all backend sources except main.cpp
file(GLOB_RECURSE LIB_SOURCES 
    "src/core/*.cpp"
    "src/db/*.cpp"
    "src/Application.cpp"
)

# Create static backend library
add_library(sigm4_backend STATIC ${LIB_SOURCES})

# Link external libs (PUBLIC = frontend inherits)
target_link_libraries(sigm4_backend PUBLIC
    ${GDAL_LIBRARIES}
    ${PQXX_LIBRARIES}
    ${PROJ_LIBRARIES}
)

# Public include folders
target_include_directories(sigm4_backend PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GDAL_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIRS}
    ${PROJ_INCLUDE_DIRS}
    )

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_executable(backend_app src/main.cpp)
    target_link_libraries(backend_app sigm4_backend)
    
    enable_testing()
    add_subdirectory(tests/unit)
endif()
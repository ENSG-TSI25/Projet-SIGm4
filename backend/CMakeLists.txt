cmake_minimum_required(VERSION 3.10)
project(BackendApp)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/**/*.cpp"
)

add_executable(backend_app ${SOURCES})

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

add_subdirectory(tests/unit)
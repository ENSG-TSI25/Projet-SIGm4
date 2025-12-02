cmake_minimum_required(VERSION 3.10)
project(BackendApp)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(GDAL REQUIRED gdal)
pkg_check_modules(PROJ REQUIRED proj)


include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/**/*.cpp"

)


add_executable(backend_app ${SOURCES})


target_include_directories(backend_app PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${PQXX_INCLUDE_DIRS}
    ${GDAL_INCLUDE_DIRS}
    ${PROJ_INCLUDE_DIRS}
)


target_link_directories(backend_app PRIVATE
    ${PQXX_LIBRARY_DIRS}
    ${GDAL_LIBRARY_DIRS}
    ${PROJ_LIBRARY_DIRS}
)


target_link_libraries(backend_app
    ${PQXX_LIBRARIES}
    ${GDAL_LIBRARIES}
    ${PROJ_LIBRARIES}
)


target_compile_options(backend_app PRIVATE ${PQXX_CFLAGS_OTHER})

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

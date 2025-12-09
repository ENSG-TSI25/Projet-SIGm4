cmake_minimum_required(VERSION 3.5)

project(frontend VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt5
find_package(Qt5 COMPONENTS Widgets Gui Core Xml Svg REQUIRED)

# QGIS headers
include_directories(
    /usr/include/qgis
    /usr/include/qgis/qgis
    /usr/include/qgis/gui
)

set(BACKEND_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../backend)

add_subdirectory(${BACKEND_SOURCE_DIR} ${CMAKE_BINARY_DIR}/backend)

# Sources
set(PROJECT_SOURCES
    main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/Carte.cpp
    src/Carte.h
    src/dialogLayerManagement.cpp
    src/dialogLayerManagement.h
    src/dialogLayerManagement.ui
    resources.qrc
)

# Exécutable (nom correct)
add_executable(frontend
    ${PROJECT_SOURCES}
)

# Lien Qt5 + QGIS
target_link_libraries(frontend PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Xml
    Qt5::Svg
    qgis_core
    qgis_gui
    sigm4_backend    
)

# Propriétés
set_target_properties(frontend PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
cmake_minimum_required(VERSION 3.16)
project(DesktopGeekDashboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

# FetchContent for libgit2 (will be fully integrated in Block 4)
include(FetchContent)
FetchContent_Declare(
    libgit2
    GIT_REPOSITORY https://github.com/libgit2/libgit2.git
    GIT_TAG v1.8.1
)
# Disable building libgit2 apps/tests to speed up
set(BUILD_CLI OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libgit2)

qt_add_executable(DesktopGeekDashboard
    src/main.cpp
    src/window_manager.h
    src/window_manager.cpp
    src/widget_config_model.h
    src/widget_config_model.cpp
)

target_include_directories(DesktopGeekDashboard PRIVATE src)

qt_add_qml_module(DesktopGeekDashboard
    URI GeekDashboard
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/GridLayoutEngine.qml
)

target_link_libraries(DesktopGeekDashboard PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    libgit2package
)

# Set app icon if needed later
# if(WIN32)
#     set_target_properties(DesktopGeekDashboard PROPERTIES WIN32_EXECUTABLE TRUE)
# endif()
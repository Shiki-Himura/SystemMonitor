cmake_minimum_required(VERSION 3.16)
project(SystemMonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 finden
find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

# Executable erstellen
add_executable(SystemMonitor 
    main.cpp
    qcustomplot.cpp
)

# Qt6 linken
target_link_libraries(SystemMonitor
    Qt6::Core
    Qt6::Widgets
    Qt6::PrintSupport
    psapi  # Windows API für Prozess-Info
)
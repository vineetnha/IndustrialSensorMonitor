cmake_minimum_required(VERSION 3.16)
project(IndustrialSensorMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Charts)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/sensorworker.cpp
    src/sensormodel.cpp
    src/sensordashboard.cpp
    src/sensorchartwidget.cpp
    src/alertmanager.cpp
    src/datalogger.cpp
)

set(HEADERS
    include/mainwindow.h
    include/sensorworker.h
    include/sensormodel.h
    include/sensordashboard.h
    include/sensorchartwidget.h
    include/alertmanager.h
    include/datalogger.h
    include/sensordata.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} resources/resources.qrc)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Charts
)

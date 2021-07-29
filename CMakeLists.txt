cmake_minimum_required(VERSION 3.14)

project(GPSLogger VERSION 0.5 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core SerialPort WebSockets REQUIRED)

set(SOURCES
    main.cpp
    clogger.cpp
    myserver.cpp
    gpsreader.cpp
)

set(HEADERS
    clogger.h
    myserver.h
    gpsreader.h
)

add_executable(GPSLogger
  ${SOURCES} ${HEADERS}
)
target_link_libraries(GPSLogger Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::SerialPort Qt${QT_VERSION_MAJOR}::WebSockets)

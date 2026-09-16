cmake_minimum_required(VERSION 3.16)
project(qtmocap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)
find_package(OpenCV REQUIRED COMPONENTS core imgproc objdetect videoio)

add_executable(qtmocap
    main.cpp
    mocap.cpp
    mocap.h
)

target_link_libraries(qtmocap PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core opencv_core opencv_imgproc opencv_objdetect opencv_videoio)

cmake_minimum_required(VERSION 3.16)
project(cto-keygen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(cto-keygen main.cpp)

target_link_libraries(cto-keygen PRIVATE Qt6::Core)

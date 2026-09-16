cmake_minimum_required(VERSION 3.16)
project(webaudit LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)
add_executable(webaudit WIN32 main.cpp runner.cpp runner.h)
target_link_libraries(webaudit PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)

target_sources(webaudit PRIVATE ../licensing/license.cpp ../licensing/license.h)
target_include_directories(webaudit PRIVATE ../licensing)

target_sources(webaudit PRIVATE ../licensing/ctf.cpp ../licensing/ctf.h)

cmake_minimum_required(VERSION 3.15)

project(tgminify LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(rapidjson REQUIRED)
add_executable(tgminify src/main.cpp)
target_link_libraries(tgminify PRIVATE rapidjson)

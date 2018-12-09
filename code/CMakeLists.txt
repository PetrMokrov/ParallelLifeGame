cmake_minimum_required(VERSION 3.5)

project(LifeGame)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_FILES main.cpp life_game.hpp executor.hpp barrier.hpp)
add_executable(LifeGame ${SOURCE_FILES})
target_link_libraries(LifeGame pthread)
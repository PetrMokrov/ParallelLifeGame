cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_COMPILER mpic++)
set(CMAKE_C_COMPILER mpicc)

project(MpiLifeGame)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")
set(SOURCE_FILES main.cpp life_game_mpi.hpp life_game_slaver.hpp life_game_master.hpp)

add_executable(MpiLifeGame ${SOURCE_FILES})
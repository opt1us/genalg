cmake_minimum_required(VERSION 3.10)

project(GenAlgProject)

add_subdirectory(matplotplusplus)

add_executable(genalg main.cpp)
target_link_libraries(genalg PUBLIC matplot)
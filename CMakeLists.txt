cmake_minimum_required(VERSION 3.26)
project(qps2)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_executable(qps2
	src/main.cpp
	src/bus.cpp
	src/cpu.cpp
)
target_link_libraries(qps2 PRIVATE SDL2::SDL2)

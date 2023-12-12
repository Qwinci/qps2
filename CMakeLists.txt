cmake_minimum_required(VERSION 3.26)
project(qps2)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_executable(qps2
	src/main.cpp
	src/bus.cpp
	src/timer.cpp

	src/ee/cpu.cpp
	src/ee/inst_cop0.cpp
	src/ee/inst_special.cpp
	src/ee/inst_normal.cpp
	src/ee/inst_regimm.cpp
	src/ee/inst_mmi.cpp
	src/ee/inst_cop2.cpp
	src/ee/inst_cop1.cpp

	src/iop/cpu.cpp
)
target_include_directories(qps2 PRIVATE src)
target_link_libraries(qps2 PRIVATE SDL2::SDL2)

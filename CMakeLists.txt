cmake_minimum_required(VERSION 3.26)
project(qps2)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

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
	src/dmac.cpp
	src/gif.cpp
	src/gs.cpp

	src/iop/iop_bus.cpp
	src/iop/cpu.cpp
	src/iop/inst_normal.cpp
	src/iop/inst_cop0.cpp
	src/iop/inst_special.cpp
	src/iop/inst_regimm.cpp
	src/iop/dma.cpp
	src/iop/cdvd.cpp

	src/scheduler.cpp
)
target_include_directories(qps2 PRIVATE src)
target_link_libraries(qps2 PRIVATE SDL2::SDL2)
target_compile_options(qps2 PRIVATE -march=native)
#target_compile_options(qps2 PRIVATE -fprofile-generate)
#target_compile_options(qps2 PRIVATE -fprofile-use -fprofile-correction)
#target_link_options(qps2 PRIVATE -fprofile-generate)
#target_link_options(qps2 PRIVATE -fprofile-use -fprofile-correction)

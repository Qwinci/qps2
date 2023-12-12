#pragma once
#include <cstdint>

struct Bus;

struct Ipu {
	Bus& bus;
	uint32_t cmd;
	uint32_t ctrl;
	uint64_t fifo[2];
};

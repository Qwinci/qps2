#pragma once
#include <array>
#include "utils.hpp"

struct Bus;

struct Gif {
	Bus& bus;
	uint64_t fifo[2];
	uint32_t stat;
	uint32_t ctrl;
	uint64_t regs;
	uint16_t data_remaining;
	uint8_t nregs;
	uint8_t regs_remaining;
	uint8_t fmt;

	void fifo_write(Uint128 packet);
};

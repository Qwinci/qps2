#pragma once
#include <cstdint>

struct Bus;

struct Vif {
	Bus& bus;
	uint32_t stat;
	uint32_t fbrst;
	uint32_t err;
	uint32_t mark;
};

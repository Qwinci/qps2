#pragma once

struct Bus;

struct Gif {
	Bus& bus;
	uint64_t fifo[2];
	uint32_t stat;
	uint32_t ctrl;
};

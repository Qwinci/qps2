#pragma once
#include <cstdint>

struct Bus;

struct Timer {
	Bus& bus;
	uint16_t counter {};
	uint16_t mode {};
	uint16_t compare {};
	uint16_t hold {};

	void clock();
	uint16_t cycle {};
};

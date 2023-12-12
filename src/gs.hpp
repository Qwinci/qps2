#pragma once
#include <cstdint>

struct Bus;

struct Gs {
	Bus& bus;
	uint32_t csr;
};

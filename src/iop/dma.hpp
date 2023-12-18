#pragma once
#include <cstdint>
#include <array>

struct Bus;

struct IopDma {
	Bus& bus;
	uint32_t dpcr;
	uint32_t dpcr2;
	uint32_t dicr;
	uint32_t dicr2;
	uint32_t dmacen;

	struct Channel {
		uint32_t madr;
		uint32_t bcr;
		uint32_t chcr;
		uint32_t tadr;
		uint32_t words_to_transfer;
	};

	std::array<Channel, 13> channels;

	void write(uint32_t addr, uint32_t value);

	void clock_sif();
};

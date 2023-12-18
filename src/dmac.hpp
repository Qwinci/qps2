#pragma once
#include <cstdint>

struct Bus;

struct Dmac {
	Bus& bus;
	uint32_t ctrl {};
	uint32_t stat {};
	uint32_t pcr {};
	uint32_t sqwc {};
	uint32_t rbsr {};
	uint32_t rbor {};
	uint32_t enabler {0x1201};
	uint32_t enablew {};

	struct Channel {
		uint32_t chcr;
		uint32_t madr;
		uint32_t qwc;
		uint32_t tadr;
		uint32_t asr0;
		uint32_t asr1;
		uint32_t sadr;
		bool tag_end;
	};
	Channel channels[10] {};

	void write(uint32_t addr, uint32_t value);
	uint32_t read(uint32_t addr);

	void clock_sif();
};

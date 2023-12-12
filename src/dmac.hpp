#pragma once

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
	};
	Channel vif0 {};
	Channel vif1 {};
	Channel gif {};
	Channel ipu_from {};
	Channel ipu_to {};
	Channel sif0 {};
	Channel sif1 {};
	Channel sif2 {};
	Channel spr_from {};
	Channel spr_to {};
};

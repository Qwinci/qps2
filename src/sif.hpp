#pragma once

struct Bus;

struct Sif {
	Bus& bus;
	uint32_t mscom;
	uint32_t msflg;
	uint32_t smflg;
	uint32_t ctrl;
};

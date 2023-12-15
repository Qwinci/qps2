#include "dma.hpp"
#include "bus.hpp"
#include <iostream>
#include <cassert>

void IopDma::write(uint32_t addr, uint32_t value) {
	uint32_t base = addr & 0xFFFFFFF0;
	Channel* channel;
	if (base == 0x1F801080) {
		channel = &channels[0];
	}
	else if (base == 0x1F801090) {
		channel = &channels[1];
	}
	else if (base == 0x1F8010A0) {
		channel = &channels[2];
	}
	else if (base == 0x1F8010B0) {
		channel = &channels[3];
	}
	else if (base == 0x1F8010C0) {
		channel = &channels[4];
	}
	else if (base == 0x1F8010D0) {
		channel = &channels[5];
	}
	else if (base == 0x1F8010E0) {
		channel = &channels[6];
	}
	else if (base == 0x1F801500) {
		channel = &channels[7];
	}
	else if (base == 0x1F801510) {
		channel = &channels[8];
	}
	else if (base == 0x1F801520) {
		channel = &channels[9];
	}
	else if (base == 0x1F801530) {
		channel = &channels[10];
	}
	else if (base == 0x1F801540) {
		channel = &channels[11];
	}
	else if (base == 0x1F801550) {
		channel = &channels[12];
	}
	else {
		std::cerr << "invalid iop dma write to "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}

	uint8_t reg = addr & 0xF;
	if (reg == 0) {
		channel->madr = value;
	}
	else if (reg == 4) {
		channel->bcr = value;
	}
	else if (reg == 8) {
		channel->chcr = value;
		if (!(value & 1U << 24) && !(value & 1U << 28)) {
			return;
		}
		channel->chcr |= 1U << 24;

		uint8_t mode = value >> 9 & 0b11;
		// SIF0 to EE
		if (base == 0x1F801520) {
			assert(mode == 1);
		}
		// SIF1 from EE
		else if (base == 0x1F801530) {
			assert(mode == 1);
		}
		else {
			assert(false && "unimplemented dma channel started");
		}
	}
	else if (reg == 0xC) {
		channel->tadr = value;
	}
}

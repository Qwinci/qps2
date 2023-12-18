#include "dma.hpp"
#include "bus.hpp"
#include <iostream>
#include <cassert>

#define D_STR (1U << 24)
#define D_MODE(chcr) ((chcr) >> 9 & 0b11)
#define D_INC(chcr) ((chcr) >> 1 & 1)

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
		if (!(value & D_STR) && !(value & 1U << 28)) {
			return;
		}
		channel->chcr |= D_STR;
		auto block_size = channel->bcr & 0xFFFF;
		auto block_count = channel->bcr >> 16;
		if (block_size == 0) {
			block_size = 0x10000;
		}
		if (block_count == 0) {
			block_count = 0x10000;
		}
		channel->words_to_transfer = block_count * block_size;

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
	else {
		assert(false);
	}
}

void IopDma::clock_sif() {
	if (!dmacen) {
		return;
	}

	auto& sif0 = channels[9];
	// IOP->EE
	if (sif0.chcr & D_STR) {
		auto mode = D_MODE(sif0.chcr);
		assert(false);
	}

	// EE->IOP
	auto& sif1 = channels[10];
	if (sif1.chcr & D_STR) {
		auto mode = D_MODE(sif1.chcr);
		assert(mode == 1);

		int inc = D_INC(sif1.chcr) ? -4 : 4;
		if (bus.sif.sif1_fifo_size && sif1.words_to_transfer >= 2) {
			auto& iop_bus = bus.iop_cpu.iop_bus;
			auto value = bus.sif.sif1_fifo[bus.sif.sif1_fifo_iop_ptr];
			bus.sif.sif1_fifo_iop_ptr = (bus.sif.sif1_fifo_iop_ptr + 1) % 16;
			bus.sif.sif1_fifo_size -= 1;
			iop_bus.write32(sif1.madr, value);
			iop_bus.write32(sif1.madr + 4, value >> 32);
			sif1.madr += inc * 2;
			sif1.words_to_transfer -= 2;
		}
	}
}

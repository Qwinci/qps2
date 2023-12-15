#include "dmac.hpp"
#include "bus.hpp"
#include "utils.hpp"
#include <iostream>
#include <cassert>

#define D_CTRL_ENABLE (1 << 0)

#define D_CHCR_DIR(chcr) ((chcr) & 1)
#define D_DIR_TO_MEM 0
#define D_DIR_FROM_MEM 1
#define D_CHCR_MOD(chcr) ((chcr) >> 2 & 0b11)
#define D_CHCR_ASP(chcr) ((chcr) >> 4 & 0b11)
#define D_CHCR_TTE (1U << 6)
#define D_CHCR_TIE (1U << 7)
#define D_CHCR_STR (1U << 8)
#define D_CHCR_TAG(tag) ((static_cast<uint32_t>(tag)) << 16)
#define D_CHCR_TAG_MASK 0xFFFF0000

void Dmac::write(uint32_t addr, uint32_t value) {
	auto base = addr & 0xFFFFFF00;
	uint8_t reg = addr & 0xFF;
	Channel* channel;
	if (base == 0x10008000) {
		channel = &channels[0];
	}
	else if (base == 0x10009000) {
		channel = &channels[1];
	}
	else if (base == 0x1000A000) {
		channel = &channels[2];
	}
	else if (base == 0x1000B000) {
		channel = &channels[3];
	}
	else if (base == 0x1000B400) {
		channel = &channels[4];
	}
	else if (base == 0x1000C000) {
		channel = &channels[5];
	}
	else if (base == 0x1000C400) {
		channel = &channels[6];
	}
	else if (base == 0x1000C800) {
		channel = &channels[7];
	}
	else if (base == 0x1000D000) {
		channel = &channels[8];
	}
	else if (base == 0x1000D400) {
		channel = &channels[9];
	}
	else {
		std::cerr << "unimplemented dmac write to "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}

	if (reg == 0x00) {
		channel->chcr = value;
		if (!(value & D_CHCR_STR) || !(ctrl & D_CTRL_ENABLE)) {
			return;
		}

		uint8_t mode = D_CHCR_MOD(value);
		// SIF0 from IOP
		if (base == 0x1000C000) {
			assert(mode == 1);
			assert(!channel->qwc && "not implemented");
		}
		// SIF1 to IOP
		else if (base == 0x1000C400) {
			assert(mode == 1);
			assert(!channel->qwc && "not implemented");
			assert(false);
		}
		// GIF
		else if (base == 0x1000A000) {
			if (mode == 0) {
				while (channel->qwc) {
					Uint128 packet {
						bus.read64(channel->madr),
						bus.read64(channel->madr + 8)
					};
					channel->madr += 16;
					bus.gif.fifo_write(packet);
					channel->qwc -= 1;
				}

				channel->chcr &= ~D_CHCR_STR;
			}
			else if (mode == 1) {
				bool irq = false;
				bool tag_end = false;
				while (true) {
					while (channel->qwc) {
						Uint128 packet {
							bus.read64(channel->madr),
							bus.read64(channel->madr + 8)
						};
						channel->madr += 16;
						bus.gif.fifo_write(packet);
						channel->qwc -= 1;
					}

					if (((channel->chcr & D_CHCR_TIE) && irq) || tag_end) {
						break;
					}

					uint64_t tag[2];
					tag[0] = bus.read64(channel->tadr);
					tag[1] = bus.read64(channel->tadr + 8);
					irq = tag[0] >> 31;

					uint8_t id = tag[0] >> 28 & 0b111;
					if (id == 0) {
						assert(!(tag[0] >> 63));
						channel->madr = tag[0] >> 32;
						channel->tadr += 16;
						tag_end = true;
					}
					else if (id == 1) {
						auto old_madr = channel->madr;
						channel->madr = channel->tadr + 16;
						channel->tadr = old_madr;
						assert(!(channel->chcr & D_CHCR_TTE));
					}
					else {
						assert(false);
					}
				}

				channel->chcr &= ~D_CHCR_STR;
			}
			else {
				assert(false);
			}
		}
		else {
			assert(false && "unimplemented dma channel started");
		}
	}
	else if (reg == 0x10) {
		channel->madr = value;
	}
	else if (reg == 0x20) {
		channel->qwc = value;
	}
	else if (reg == 0x30) {
		channel->tadr = value;
	}
	else if (reg == 0x40) {
		channel->asr0 = value;
	}
	else if (reg == 0x50) {
		channel->asr1 = value;
	}
	else if (reg == 0x80) {
		channel->sadr = value;
	}
}

uint32_t Dmac::read(uint32_t addr) {
	auto base = addr & 0xFFFFFF00;
	uint8_t reg = addr & 0xFF;
	Channel* channel;
	if (base == 0x10008000) {
		channel = &channels[0];
	}
	else if (base == 0x10009000) {
		channel = &channels[1];
	}
	else if (base == 0x1000A000) {
		channel = &channels[2];
	}
	else if (base == 0x1000B000) {
		channel = &channels[3];
	}
	else if (base == 0x1000B400) {
		channel = &channels[4];
	}
	else if (base == 0x1000C000) {
		channel = &channels[5];
	}
	else if (base == 0x1000C400) {
		channel = &channels[6];
	}
	else if (base == 0x1000C800) {
		channel = &channels[7];
	}
	else if (base == 0x1000D000) {
		channel = &channels[8];
	}
	else if (base == 0x1000D400) {
		channel = &channels[9];
	}
	else {
		std::cerr << "unimplemented dmac read from "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}

	if (reg == 0x00) {
		return channel->chcr;
	}
	else if (reg == 0x10) {
		return channel->madr;
	}
	else if (reg == 0x20) {
		return channel->qwc;
	}
	else if (reg == 0x30) {
		return channel->tadr;
	}
	else if (reg == 0x40) {
		return channel->asr0;
	}
	else if (reg == 0x50) {
		return channel->asr1;
	}
	else if (reg == 0x80) {
		return channel->sadr;
	}
	else {
		return 0;
	}
}

#include <cassert>
#include "gif.hpp"
#include "bus.hpp"

#define GIF_TAG_NLOOP(packet) ((packet).low & 0x7FFF)
#define GIF_TAG_EOP(packet) ((packet).low & 1U << 15)
#define GIF_TAG_PRIM_EN(packet) ((packet).low & 1ULL << 46)
#define GIF_TAG_PRIM(packet) ((packet).low >> 47 & 0x7FF)
#define GIF_TAG_DATA_FMT(packet) ((packet).low >> 58 & 0b11)
#define GIF_TAG_NREGS(packet) ((packet).low >> 60 & 0b1111)
#define GIF_TAG_REG(packet, num) ((packet).high >> ((num) * 4))

void Gif::fifo_write(Uint128 packet) {
	if (data_remaining == 0) {
		uint16_t nloop = GIF_TAG_NLOOP(packet);
		if (nloop == 0) {
			return;
		}

		bool eop = GIF_TAG_EOP(packet);
		bool prim_en = GIF_TAG_PRIM_EN(packet);
		uint16_t prim = GIF_TAG_PRIM(packet);
		fmt = GIF_TAG_DATA_FMT(packet);
		nregs = GIF_TAG_NREGS(packet);
		if (nregs == 0) {
			nregs = 16;
		}

		if (prim_en) {
			bus.gs.prim = prim;
		}

		regs_remaining = nregs;
		data_remaining = nloop;
		regs = packet.high;
	}
	else {
		// PACKED
		if (fmt == 0) {
			uint8_t reg = regs >> ((nregs - regs_remaining) * 4) & 0b1111;
			--regs_remaining;
			if (regs_remaining == 0) {
				regs_remaining = nregs;
				--data_remaining;
			}

			if (reg == 0) {
				bus.gs.prim = packet.low & 0x7FF;
			}
			else if (reg == 1) {
				bus.gs.rgbaq.red = packet.low & 0xFF;
				bus.gs.rgbaq.green = packet.low >> 32 & 0xFF;
				bus.gs.rgbaq.blue = packet.high & 0xFF;
				bus.gs.rgbaq.alpha = packet.high >> 32 & 0xFF;
			}
			else if (reg == 4) {
				// x
				uint64_t data = packet.low & 0xFFFF;
				// y
				data |= (packet.low >> 32 & 0xFFFF) << 16;
				// z
				data |= (packet.high >> 4 & 0xFFFFFF) << 32;
				// f
				data |= (packet.high >> 36 & 0xFF) << 56;
				// disable drawing
				if (packet.high & 1ULL << 47) {
					bus.gs.write_reg(0xC, data);
				}
				else {
					bus.gs.write_reg(0x4, data);
				}
			}
			else if (reg == 0xE) {
				reg = packet.high & 0xFF;
				bus.gs.write_reg(reg, packet.low);
			}
			else if (reg == 2 || reg == 3 || reg == 5 || reg == 0xA ||
				reg == 0xF) {
				assert(false && "unimplemented gs reg accessed");
			}
			else {
				bus.gs.write_reg(reg, packet.low);
			}
		}
		// IMAGE
		else if (fmt == 2 || fmt == 3) {
			bus.gs.write_hw_reg(packet.low);
			bus.gs.write_hw_reg(packet.high);
			--data_remaining;
		}
		else {
			assert(false);
		}
	}
}

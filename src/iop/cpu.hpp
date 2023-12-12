#pragma once
#include "cpu_shared.hpp"
#include <cstdint>

struct Bus;

struct IopCpu {
	Bus& bus;
	uint32_t regs[32];
	uint32_t pc;
	uint32_t hi;
	uint32_t lo;

	inline constexpr uint32_t& get_reg(Reg reg) {
		return regs[static_cast<int>(reg)];
	}

	inline void write_reg(uint8_t reg, uint32_t value) {
		if (reg == 0) {
			return;
		}
		regs[reg] = value;
	}

	void clock();
};

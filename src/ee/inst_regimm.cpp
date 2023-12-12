#include <iostream>
#include <cassert>
#include "cpu.hpp"

void EeCpu::inst_regimm(uint32_t byte) {
	uint8_t func = byte >> 16 & 0b11111;
	// BLTZ
	if (func == 0b00000) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (static_cast<int64_t>(regs[rs].low) < 0) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// BGEZ
	else if (func == 0b00001) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (static_cast<int64_t>(regs[rs].low) >= 0) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	else {
		std::cerr << "unimplemented regimm func "
		          << std::hex << std::uppercase << static_cast<unsigned int>(func)
		          << std::dec << '\n';
		abort();
	}
}

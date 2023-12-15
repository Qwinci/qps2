#include <iostream>
#include "cpu.hpp"

void EeCpu::inst_mmi(uint32_t byte) {
	uint8_t func = byte & 0b111111;

	// PLZCW
	if (func == 0b000100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		auto value = regs[rs].low;
		uint32_t low = value;
		uint32_t high = value >> 32;

		uint8_t low_bit = low >> 31;
		uint8_t high_bit = high >> 31;

		uint32_t low_res = 0;
		uint32_t high_res = 0;
		for (uint8_t i = 0; i < 32; ++i) {
			if ((low >> (31 - i) & 1) != low_bit) {
				break;
			}
			++low_res;
		}
		for (uint8_t i = 0; i < 32; ++i) {
			if ((high >> (31 - i) & 1) != high_bit) {
				break;
			}
			++high_res;
		}

		--low_res;
		--high_res;
		write_reg_low(rd, static_cast<uint64_t>(high_res) << 32 | low_res);
	}
	// MMI0
	else if (func == 0b001000) {
		func = byte >> 6 & 0b11111;
		// PEXTLW
		if (func == 0b10010) {
			uint8_t rs = byte >> 21 & 0b11111;
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;

			auto a = regs[rs].low;
			auto b = regs[rt].low;
			write_reg_low(rd, (b & 0xFFFFFFFF) | ((a & 0xFFFFFFFF) << 32));
			write_reg_high(rd, b >> 32 | (a & 0xFFFFFFFF00000000));
		}
		else {
			std::cerr << "unimplemented mmi0 func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
			          << std::dec << '\n';
			abort();
		}
	}
	// MMI2
	else if (func == 0b001001) {
		func = byte >> 6 & 0b11111;
		// PMFHI
		if (func == 0b01000) {
			uint8_t rd = byte >> 11 & 0b11111;
			write_reg_low(rd, hi_lo);
			write_reg_high(rd, 0);
		}
		// PMFLO
		else if (func == 0b01001) {
			uint8_t rd = byte >> 11 & 0b11111;
			write_reg_low(rd, hi_lo >> 32);
			write_reg_high(rd, 0);
		}
		// PCPYLD
		else if (func == 0b01110) {
			uint8_t rs = byte >> 21 & 0b11111;
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;
			write_reg_low(rd, regs[rt].low);
			write_reg_high(rd, regs[rs].low);
		}
		else {
			std::cerr << "unimplemented mmi2 func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
			          << std::dec << '\n';
			abort();
		}
	}
	// MFHI1
	else if (func == 0b010000) {
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, hi1_lo1 >> 32);
	}
	// MFLO1
	else if (func == 0b010010) {
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, hi1_lo1 & 0xFFFFFFFF);
	}
	// MULT1
	else if (func == 0b011000) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		auto a = static_cast<int32_t>(regs[rs].low);
		auto b = static_cast<int32_t>(regs[rt].low);
		int64_t res = a * b;
		hi1_lo1 = res;
		auto low = static_cast<int32_t>(res);
		write_reg_low(rd, static_cast<int64_t>(low));
	}
	// DIV1
	else if (func == 0b011010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto a = static_cast<int32_t>(regs[rs].low);
		auto b = static_cast<int32_t>(regs[rt].low);
		int32_t res = a / b;
		int32_t mod = a % b;
		hi1_lo1 = static_cast<uint64_t>(mod) << 32 | res;
	}
	// DIVU1
	else if (func == 0b011011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto a = static_cast<uint32_t>(regs[rs].low);
		auto b = static_cast<uint32_t>(regs[rt].low);
		uint32_t res = 0;
		uint32_t mod = 0;
		if (b != 0) {
			res = a / b;
			mod = a % b;
		}
		hi1_lo1 = static_cast<uint64_t>(mod) << 32 | res;
	}
	// MMI1
	else if (func == 0b101000) {
		func = byte >> 6 & 0b11111;
		// PADDUW
		if (func == 0b10000) {
			uint8_t rs = byte >> 21 & 0b11111;
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;

			auto rs_split = regs[rs].split32();
			auto rt_split = regs[rt].split32();

			for (int i = 0; i < 4; ++i) {
				uint64_t res = rs_split[i] + rt_split[i];
				if (res > 0xFFFFFFFF) {
					res = 0xFFFFFFFF;
				}
				rt_split[i] = res;
			}
			if (rd != 0) {
				regs[rd].store32(rt_split);
			}
		}
		else {
			std::cerr << "unimplemented mmi1 func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
			          << std::dec << '\n';
			abort();
		}
	}
	// MMI3
	else if (func == 0b101001) {
		func = byte >> 6 & 0b11111;
		// POR
		if (func == 0b10010) {
			uint8_t rs = byte >> 21 & 0b11111;
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;

			write_reg_low(rd, regs[rs].low | regs[rt].low);
			write_reg_high(rd, regs[rs].high | regs[rt].high);
		}
		// PCPYUD
		else if (func == 0b01110) {
			uint8_t rs = byte >> 21 & 0b11111;
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;
			write_reg_low(rd, regs[rs].high);
			write_reg_high(rd, regs[rt].high);
		}
		else {
			std::cerr << "unimplemented mmi3 func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
			          << std::dec << '\n';
			abort();
		}
	}
	else {
		std::cerr << "unimplemented mmi func "
		          << std::hex << std::uppercase << static_cast<unsigned int>(func)
		          << std::dec << '\n';
		abort();
	}
}

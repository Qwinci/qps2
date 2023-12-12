#include "cpu.hpp"
#include <iostream>
#include <cstdlib>
#include <cassert>

void EeCpu::inst_special(uint32_t byte) {
	uint8_t func = byte & 0b111111;
	// SLL
	if (func == 0b000000) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;

		auto res = static_cast<int32_t>(regs[rt].low) << l_sa;
		write_reg_low(rd, static_cast<int64_t>(res));
	}
	// SRL
	else if (func == 0b000010) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;

		auto res = static_cast<uint32_t>(regs[rt].low) >> l_sa;
		write_reg_low(rd, static_cast<int64_t>(static_cast<int32_t>(res)));
	}
	// SRA
	else if (func == 0b000011) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;

		auto res = static_cast<int32_t>(regs[rt].low) >> l_sa;
		write_reg_low(rd, static_cast<int64_t>(res));
	}
	// SLLV
	else if (func == 0b000100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		uint8_t shift = regs[rs].low & 0b11111;
		auto res = static_cast<uint32_t>(regs[rt].low) << shift;
		write_reg_low(rd, static_cast<int64_t>(static_cast<int32_t>(res)));
	}
	// SRLV
	else if (func == 0b000110) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		uint8_t shift = regs[rs].low & 0b11111;
		auto res = static_cast<uint32_t>(regs[rt].low) >> shift;
		write_reg_low(rd, static_cast<int64_t>(static_cast<int32_t>(res)));
	}
	// SRAV
	else if (func == 0b000111) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		uint8_t shift = regs[rs].low & 0b11111;
		auto res = static_cast<int32_t>(regs[rt].low) >> shift;
		write_reg_low(rd, static_cast<int64_t>(res));
	}
	// JR
	else if (func == 0b001000) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		in_branch_delay = true;
		new_pc = regs[rs].low;
	}
	// JALR
	else if (func == 0b001001) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		write_reg_low(rd, pc + 4);
		in_branch_delay = true;
		new_pc = regs[rs].low;
	}
	// MOVZ
	else if (func == 0b001010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		if (regs[rt].low == 0) {
			write_reg_low(rd, regs[rs].low);
		}
	}
	// MOVN
	else if (func == 0b001011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		if (regs[rt].low != 0) {
			write_reg_low(rd, regs[rs].low);
		}
	}
	// BREAK
	else if (func == 0b001101) {
		raise_level2_exception(0x80000100, 0x9);
	}
	// SYNC
	else if (func == 0b001111) {
	// todo implement
	}
	// MFHI
	else if (func == 0b010000) {
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, hi_lo >> 32);
	}
	// MFLO
	else if (func == 0b010010) {
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, hi_lo & 0xFFFFFFFF);
	}
	// DSLLV
	else if (func == 0b010100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t shift = regs[rs].low & 0b111111;
		write_reg_low(rd, regs[rt].low << shift);
	}
	// DSRAV
	else if (func == 0b010111) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t shift = regs[rs].low & 0b111111;
		write_reg_low(rd, static_cast<int64_t>(regs[rt].low) >> shift);
	}
	// MULT
	else if (func == 0b011000) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto a = static_cast<int32_t>(regs[rs].low);
		auto b = static_cast<int32_t>(regs[rt].low);
		int64_t res = a * b;
		hi_lo = res;
	}
	// DIV
	else if (func == 0b011010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto a = static_cast<int32_t>(regs[rs].low);
		auto b = static_cast<int32_t>(regs[rt].low);
		int32_t res = a / b;
		int32_t mod = a % b;
		hi_lo = static_cast<uint64_t>(mod) << 32 | res;
	}
	// DIVU
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
		hi_lo = static_cast<uint64_t>(mod) << 32 | res;
	}
	// ADDU
	else if (func == 0b100001) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		auto res = static_cast<int32_t>(regs[rs].low + regs[rt].low);
		write_reg_low(rd, static_cast<int64_t>(res));
	}
	// SUBU
	else if (func == 0b100011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		auto res = static_cast<int32_t>(regs[rs].low - regs[rt].low);
		write_reg_low(rd, static_cast<int64_t>(res));
	}
	// AND
	else if (func == 0b100100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, regs[rs].low & regs[rt].low);
	}
	// OR
	else if (func == 0b100101) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, regs[rs].low | regs[rt].low);
	}
	// NOR
	else if (func == 0b100111) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, ~(regs[rs].low | regs[rt].low));
	}
	// SLT
	else if (func == 0b101010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, static_cast<int64_t>(regs[rs].low) < static_cast<int64_t>(regs[rt].low));
	}
	// SLTU
	else if (func == 0b101011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, regs[rs].low < regs[rt].low);
	}
	// DADDU
	else if (func == 0b101101) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg_low(rd, regs[rs].low + regs[rt].low);
	}
	// DSLL
	else if (func == 0b111000) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;
		write_reg_low(rd, regs[rt].low << l_sa);
	}
	// DSRL
	else if (func == 0b111010) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;
		write_reg_low(rd, regs[rt].low >> l_sa);
	}
	// DSLL32
	else if (func == 0b111100) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;
		write_reg_low(rd, regs[rt].low << (l_sa + 32));
	}
	// DSRL32
	else if (func == 0b111110) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;
		write_reg_low(rd, regs[rt].low >> (l_sa + 32));
	}
	// DSRA32
	else if (func == 0b111111) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;
		write_reg_low(rd, static_cast<int64_t>(regs[rt].low) >> (l_sa + 32));
	}
	else {
		std::cerr << "unimplemented special func "
		          << std::hex << std::uppercase << static_cast<unsigned int>(func)
		          << std::dec << '\n';
		abort();
	}
}

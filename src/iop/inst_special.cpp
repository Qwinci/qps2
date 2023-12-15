#include "cpu.hpp"
#include <iostream>
#include <cassert>

void IopCpu::inst_special(uint32_t byte) {
	uint8_t func = byte & 0b111111;

	// SLL
	if (func == 0b000000) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;

		auto res = regs[rt] << l_sa;
		write_reg(rd, res);
	}
	// SRL
	else if (func == 0b000010) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;

		auto res = regs[rt] >> l_sa;
		write_reg(rd, res);
	}
	// SRA
	else if (func == 0b000011) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		uint8_t l_sa = byte >> 6 & 0b11111;

		auto res = static_cast<int32_t>(regs[rt]) >> l_sa;
		write_reg(rd, res);
	}
	// SLLV
	else if (func == 0b000100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		uint8_t shift = regs[rs] & 0b11111;
		auto res = regs[rt] << shift;
		write_reg(rd, static_cast<int32_t>(res));
	}
	// SRLV
	else if (func == 0b000110) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		uint8_t shift = regs[rs] & 0b11111;
		auto res = regs[rt] >> shift;
		write_reg(rd, static_cast<int32_t>(res));
	}
	// JR
	else if (func == 0b001000) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		in_branch_delay = true;
		new_pc = regs[rs];
	}
	// JALR
	else if (func == 0b001001) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		write_reg(rd, pc + 4);
		in_branch_delay = true;
		new_pc = regs[rs];
	}
	// SYSCALL
	else if (func == 0b001100) {
		raise_level1_exception(0x80000080, 0x8);
	}
	// MFHI
	else if (func == 0b010000) {
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, hi);
	}
	// MTHI
	else if (func == 0b010001) {
		uint8_t rs = byte >> 21 & 0b11111;
		hi = regs[rs];
	}
	// MFLO
	else if (func == 0b010010) {
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, lo);
	}
	// MTLO
	else if (func == 0b010011) {
		uint8_t rs = byte >> 21 & 0b11111;
		lo = regs[rs];
	}
	// MULT
	else if (func == 0b011000) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto a = static_cast<int32_t>(regs[rs]);
		auto b = static_cast<int32_t>(regs[rt]);
		int64_t res = a * b;
		lo = res;
		hi = res >> 32;
	}
	// MULTU
	else if (func == 0b011001) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto a = regs[rs];
		auto b = regs[rt];
		uint64_t res = a * b;
		lo = res;
		hi = res >> 32;
	}
	// DIVU
	else if (func == 0b011011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto a = regs[rs];
		auto b = regs[rt];
		uint32_t res = 0;
		uint32_t mod = 0;
		if (b != 0) {
			res = a / b;
			mod = a % b;
		}
		hi = mod;
		lo = res;
	}
	// ADD
	else if (func == 0b100000) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		auto res = static_cast<int32_t>(regs[rs] + regs[rt]);
		write_reg(rd, res);
	}
	// ADDU
	else if (func == 0b100001) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		auto res = static_cast<int32_t>(regs[rs] + regs[rt]);
		write_reg(rd, res);
	}
	// SUBU
	else if (func == 0b100011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		auto res = static_cast<int32_t>(regs[rs] - regs[rt]);
		write_reg(rd, res);
	}
	// AND
	else if (func == 0b100100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, regs[rs] & regs[rt]);
	}
	// OR
	else if (func == 0b100101) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, regs[rs] | regs[rt]);
	}
	// XOR
	else if (func == 0b100110) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, regs[rs] ^ regs[rt]);
	}
	// NOR
	else if (func == 0b100111) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, ~(regs[rs] | regs[rt]));
	}
	// SLT
	else if (func == 0b101010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, static_cast<int32_t>(regs[rs]) < static_cast<int32_t>(regs[rt]));
	}
	// SLTU
	else if (func == 0b101011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;
		write_reg(rd, regs[rs] < regs[rt]);
	}
	else {
		std::cerr << "unimplemented iop special func "
		          << std::hex << std::uppercase << static_cast<unsigned int>(func)
		          << std::dec << '\n';
		abort();
	}
}

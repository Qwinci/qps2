#include "cpu.hpp"
#include <iostream>
#include <cassert>

void IopCpu::inst_normal(uint32_t byte) {
	uint8_t op = byte >> 26;

	// SPECIAL
	if (op == 0b000000) {
		inst_special(byte);
	}
	// REGIMM
	else if (op == 0b000001) {
		inst_regimm(byte);
	}
	// J
	else if (op == 0b000010) {
		assert(!in_branch_delay);

		uint32_t instr_index = byte << 6 >> 6;
		uint32_t addr = pc;
		addr &= 0xF0000000;
		addr |= instr_index << 2;

		in_branch_delay = true;
		new_pc = addr;
	}
	// JAL
	else if (op == 0b000011) {
		assert(!in_branch_delay);

		uint32_t instr_index = byte << 6 >> 6;
		uint32_t addr = pc;
		addr &= 0xF0000000;
		addr |= instr_index << 2;
		get_reg(Reg::Ra) = pc + 4;

		in_branch_delay = true;
		new_pc = addr;
	}
	// BEQ
	else if (op == 0b000100) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (regs[rs] == regs[rt]) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// BNE
	else if (op == 0b000101) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (regs[rs] != regs[rt]) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// BLEZ
	else if (op == 0b000110) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (static_cast<int32_t>(regs[rs]) <= 0) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// BGTZ
	else if (op == 0b000111) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (static_cast<int32_t>(regs[rs]) > 0) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// ADDI
	else if (op == 0b001000) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		auto res = static_cast<int32_t>(regs[rs]) + imm;
		write_reg(rt, res);
	}
	// ADDIU
	else if (op == 0b001001) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		auto res = static_cast<int32_t>(regs[rs]) + imm;
		write_reg(rt, res);
	}
	// SLTI
	else if (op == 0b001010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg(rt, static_cast<int32_t>(regs[rs]) < imm);
	}
	// SLTIU
	else if (op == 0b001011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg(rt, regs[rs] < imm);
	}
	// ANDI
	else if (op == 0b001100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint16_t imm = byte & 0xFFFF;
		write_reg(rt, regs[rs] & imm);
	}
	// ORI
	else if (op == 0b001101) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint16_t imm = byte & 0xFFFF;
		write_reg(rt, regs[rs] | imm);
	}
	// LUI
	else if (op == 0b001111) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint32_t imm = (byte & 0xFFFF) << 16;
		write_reg(rt, imm);
	}
	// COP0
	else if (op == 0b010000) {
		inst_cop0(byte);
	}
	// LB
	else if (op == 0b100000) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write_reg(rt, static_cast<int32_t>(static_cast<int8_t>(read8(addr))));
	}
	// LH
	else if (op == 0b100001) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write_reg(rt, static_cast<int32_t>(static_cast<int16_t>(read16(addr))));
	}
	// LW
	else if (op == 0b100011) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write_reg(rt, read32(addr));
	}
	// LBU
	else if (op == 0b100100) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write_reg(rt, read8(addr));
	}
	// LHU
	else if (op == 0b100101) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write_reg(rt, read16(addr));
	}
	// SB
	else if (op == 0b101000) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write8(addr, regs[rt]);
	}
	// SH
	else if (op == 0b101001) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write16(addr, regs[rt]);
	}
	// SW
	else if (op == 0b101011) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base] + offset;
		write32(addr, regs[rt]);
	}
	else {
		std::cerr << "unimplemented iop op "
		          << std::hex << std::uppercase << byte << std::dec << '\n';
		abort();
	}
}

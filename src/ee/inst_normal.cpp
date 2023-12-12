#include <cassert>
#include <iostream>
#include "cpu.hpp"

void EeCpu::inst_normal(uint32_t byte) {
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
		get_reg(Reg::Ra).low = pc + 4;

		in_branch_delay = true;
		new_pc = addr;
	}
	// BEQ
	else if (op == 0b000100) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (regs[rs].low == regs[rt].low) {
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
		if (regs[rs].low != regs[rt].low) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// BLEZ
	else if (op == 0b000110) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (static_cast<int64_t>(regs[rs].low) <= 0) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// BGTZ
	else if (op == 0b000111) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (static_cast<int64_t>(regs[rs].low) > 0) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
	}
	// ADDI
	else if (op == 0b001000) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg_low(rt, static_cast<int64_t>(static_cast<int32_t>(regs[rs].low + imm)));
	}
	// ADDIU
	else if (op == 0b001001) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		auto res = static_cast<int32_t>(regs[rs].low) + imm;
		write_reg_low(rt, static_cast<int64_t>(res));
	}
	// SLTI
	else if (op == 0b001010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg_low(rt, static_cast<int64_t>(regs[rs].low) < imm);
	}
	// SLTIU
	else if (op == 0b001011) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg_low(rt, regs[rs].low < imm);
	}
	// ANDI
	else if (op == 0b001100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint16_t imm = byte & 0xFFFF;
		write_reg_low(rt, regs[rs].low & imm);
	}
	// ORI
	else if (op == 0b001101) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint16_t imm = byte & 0xFFFF;
		write_reg_low(rt, regs[rs].low | imm);
	}
	// XORI
	else if (op == 0b001110) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint16_t imm = byte & 0xFFFF;
		write_reg_low(rt, regs[rs].low ^ imm);
	}
	// LUI
	else if (op == 0b001111) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint32_t imm = (byte & 0xFFFF) << 16;
		write_reg_low(rt, static_cast<int64_t>(static_cast<int32_t>(imm)));
	}
	// COP0
	else if (op == 0b010000) {
		inst_cop0(byte);
	}
	// COP1
	else if (op == 0b010001) {
		inst_cop1(byte);
	}
	// COP2
	else if (op == 0b010010) {
		inst_cop2(byte);
	}
	// BEQL
	else if (op == 0b010100) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (regs[rs].low == regs[rt].low) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
		else {
			pc += 4;
		}
	}
	// BNEL
	else if (op == 0b010101) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int32_t>(static_cast<int16_t>((byte & 0xFFFF))) << 2;
		if (regs[rs].low != regs[rt].low) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
		else {
			pc += 4;
		}
	}
	// DADDIU
	else if (op == 0b011001) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg_low(rt, regs[rs].low + imm);
	}
	// LDL
	else if (op == 0b011010) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		uint64_t res = regs[rt].low;
		res &= 0xFFFFFFFF;
		res |= static_cast<uint64_t>(read32(addr)) << 32;
		write_reg_high(rt, res);
		// todo verify
	}
	// LDR
	else if (op == 0b011011) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		uint64_t res = regs[rt].low;
		res &= 0xFFFFFFFF00000000;
		res |= static_cast<uint64_t>(read32(addr));
		write_reg_high(rt, res);
		// todo verify
	}
	// MMI
	else if (op == 0b011100) {
		inst_mmi(byte);
	}
	// LQ
	else if (op == 0b011110) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		addr &= 0xFFFFFFF0;
		write_reg_low(rt, read64(addr));
		write_reg_high(rt, read64(addr + 8));
	}
	// SQ
	else if (op == 0b011111) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		addr &= 0xFFFFFFF0;
		write64(addr, regs[rt].low);
		write64(addr + 8, regs[rt].high);
	}
	// LB
	else if (op == 0b100000) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, static_cast<int64_t>(static_cast<int8_t>(read8(addr))));
	}
	// LH
	else if (op == 0b100001) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, static_cast<int64_t>(static_cast<int16_t>(read16(addr))));
	}
	// LW
	else if (op == 0b100011) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, static_cast<int64_t>(static_cast<int32_t>(read32(addr))));
	}
	// LBU
	else if (op == 0b100100) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read8(addr));
	}
	// LHU
	else if (op == 0b100101) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read16(addr));
	}
	// LWU
	else if (op == 0b100111) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read32(addr));
	}
	// SB
	else if (op == 0b101000) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write8(addr, regs[rt].low);
	}
	// SH
	else if (op == 0b101001) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write16(addr, regs[rt].low);
	}
	// SW
	else if (op == 0b101011) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write32(addr, regs[rt].low);
	}
	// SDL
	else if (op == 0b101100) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write32(addr, regs[rt].low >> 32);
	}
	// SDR
	else if (op == 0b101101) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write32(addr, regs[rt].low);
	}
	// CACHE
	else if (op == 0b101111) {
		// todo implement
	}
	// LD
	else if (op == 0b110111) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read64(addr));
	}
	// SWC1
	else if (op == 0b111001) {
		// todo floating point
	}
	// SD
	else if (op == 0b111111) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write64(addr, regs[rt].low);
	}
	else {
		std::cerr << "unimplemented op "
		          << std::hex << std::uppercase << byte << std::dec << '\n';
		abort();
	}
}

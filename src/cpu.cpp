#include "cpu.hpp"
#include "bus.hpp"
#include <iostream>
#include <cassert>

#define TODO(name) do { \
	std::cerr << "todo " << name << '\n'; \
	abort(); \
} while (0)

#define UNREACHABLE(msg) do { \
	std::cerr << "reached unreachable code: " << msg << '\n'; \
	abort(); \
} while (0)

Cpu::Cpu(Bus& bus) : bus {bus} {
	// EE
	co0.get_reg(Cop0Reg::PrId) = 0x59;
}

void Cpu::clock() {
	auto byte = read32(pc);
	pc += 4;

	uint8_t op = byte >> 26;
	// COP0
	if (op == 0b010000) {
		uint8_t fmt = byte >> 21 & 0b11111;
		// MFC0
		if (fmt == 0b00000) {
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;

			write_reg_low(rt, co0.regs[rd]);
		}
		// MTC0
		else if (fmt == 0b00100) {
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;

			// todo use the value
			co0.regs[rd] = regs[rt].low;
		}
		// BC0
		else if (fmt == 0b01000) {
			TODO("BC0");
		}
		// TLB
		else if (fmt == 0b10000) {
			fmt = byte & 0b111111;
			if (fmt == 0b000001) {
				TODO("TLBR");
			}
			// TLBWI
			else if (fmt == 0b000010) {
				auto index = co0.get_reg(Cop0Reg::Index);

				auto page_mask = co0.get_reg(Cop0Reg::PageMask);
				auto entry_hi = co0.get_reg(Cop0Reg::EntryHi);
				auto entry_lo0 = co0.get_reg(Cop0Reg::EntryLo0);
				auto entry_lo1 = co0.get_reg(Cop0Reg::EntryLo1);
				auto vpn2 = entry_hi >> 13;
				auto vpn = vpn2 * 2;

				auto page_size = page_mask == 0 ? 0x1000 : 0;
				vpn *= page_size;
				// todo implement
			}
			else if (fmt == 0b000110) {
				TODO("TLBWR");
			}
			else if (fmt == 0b001000) {
				TODO("TLBP");
			}
			else if (fmt == 0b011000) {
				TODO("ERET");
			}
			else if (fmt == 0b111000) {
				TODO("EI");
			}
			else if (fmt == 0b111001) {
				TODO("DI");
			}
			else {
				UNREACHABLE("invalid TLB instruction");
			}
		}
		else {
			UNREACHABLE("invalid COP0 instruction");
		}
	}
	// SPECIAL
	else if (op == 0b000000) {
		uint8_t func = byte & 0b111111;
		// SLL
		if (func == 0b000000) {
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;
			uint8_t l_sa = byte >> 6 & 0b11111;

			write_reg_low(rd, static_cast<int64_t>(regs[rt].low << l_sa));
		}
		// SRA
		else if (func == 0b000011) {
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;
			uint8_t l_sa = byte >> 6 & 0b11111;

			write_reg_low(rd, static_cast<int64_t>(regs[rt].low >> l_sa));
		}
		// JR
		else if (func == 0b001000) {
			assert(!in_branch_delay);

			uint8_t rs = byte >> 21 & 0b11111;
			in_branch_delay = true;
			new_pc = regs[rs].low;
			return;
		}
		// JALR
		else if (func == 0b001001) {
			assert(!in_branch_delay);

			uint8_t rs = byte >> 21 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;

			write_reg_low(rd, pc + 4);
			in_branch_delay = true;
			new_pc = regs[rs].low;
			return;
		}
		// SYNC
		else if (func == 0b001111) {
			// todo implement
		}
		// ADDU
		else if (func == 0b100001) {
			uint8_t rs = byte >> 21 & 0b11111;
			uint8_t rt = byte >> 16 & 0b11111;
			uint8_t rd = byte >> 11 & 0b11111;
			write_reg_low(rd, regs[rs].low + regs[rt].low);
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
		else {
			std::cerr << "unimplemented special func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
					  << std::dec << '\n';
			exit(1);
		}
	}
	// SLTI
	else if (op == 0b001010) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg_low(rt, static_cast<int64_t>(regs[rs].low) < imm);
	}
	// BNE
	else if (op == 0b000101) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int32_t>((byte & 0xFFFF) << 2);
		if (regs[rs].low != regs[rt].low) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
		return;
	}
	// JAL
	else if (op == 0b000011) {
		assert(!in_branch_delay);

		uint32_t instr_index = byte << 6 >> 6;
		uint32_t addr = pc;
		addr &= ~0b1111111111111111111111111111;
		addr |= instr_index << 2;
		get_reg(Reg::Ra).low = pc + 4;

		in_branch_delay = true;
		new_pc = addr;
		return;
	}
	// BEQ
	else if (op == 0b000100) {
		assert(!in_branch_delay);

		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int32_t>((byte & 0xFFFF) << 2);
		if (regs[rs].low == regs[rt].low) {
			in_branch_delay = true;
			new_pc = pc + imm;
		}
		return;
	}
	// LUI
	else if (op == 0b001111) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint32_t imm = (byte & 0xFFFF) << 16;
		write_reg_low(rt, static_cast<int64_t>(imm));
	}
	// ORI
	else if (op == 0b001101) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint16_t imm = byte & 0xFFFF;
		write_reg_low(rt, regs[rs].low | imm);
	}
	// ADDIU
	else if (op == 0b001001) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg_low(rt, regs[rs].low + imm);
	}
	// LW
	else if (op == 0b100011) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, static_cast<int64_t>(read32(addr)));
	}
	// ADDI
	else if (op == 0b001000) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto imm = static_cast<int16_t>(byte & 0xFFFF);
		write_reg_low(rt, regs[rs].low + imm);
	}
	// ANDI
	else if (op == 0b001100) {
		uint8_t rs = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		uint16_t imm = byte & 0xFFFF;
		write_reg_low(rt, regs[rs].low & imm);
	}
	// SW
	else if (op == 0b101011) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write32(addr, regs[rt].low);
	}
	// LBU
	else if (op == 0b100100) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read8(addr));
	}
	// LWU
	else if (op == 0b100111) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read32(addr));
	}
	// LHU
	else if (op == 0b100101) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read16(addr));
	}
	// LD
	else if (op == 0b110111) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write_reg_low(rt, read64(addr));
	}
	// MMI
	else if (op == 0b011100) {
		uint8_t func = byte & 0b111111;

		// MMI1
		if (func == 0b101000) {
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
				exit(1);
			}
		}
		else {
			std::cerr << "unimplemented mmi func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
			          << std::dec << '\n';
			exit(1);
		}
	}
	// SD
	else if (op == 0b111111) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write64(addr, regs[rt].low);
	}
	// REGIMM
	else if (op == 0b000001) {
		uint8_t func = byte >> 16 & 0b11111;
		// BGEZ
		if (func == 0b00001) {
			uint8_t rs = byte >> 21 & 0b11111;
			auto imm = static_cast<int32_t>((byte & 0xFFFF) << 2);
			if (static_cast<int64_t>(regs[rs].low) >= 0) {
				in_branch_delay = true;
				new_pc = pc + imm;
			}
			return;
		}
		else {
			std::cerr << "unimplemented regimm func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
			          << std::dec << '\n';
			exit(1);
		}
	}
	// SH
	else if (op == 0b101001) {
		uint8_t base = byte >> 21 & 0b11111;
		uint8_t rt = byte >> 16 & 0b11111;
		auto offset = static_cast<int16_t>(byte & 0xFFFF);
		uint32_t addr = regs[base].low + offset;
		write16(addr, regs[rt].low);
	}
	else {
		std::cerr << "unimplemented op "
		          << std::hex << std::uppercase << byte << std::dec << '\n';
		exit(1);
	}

	if (in_branch_delay) {
		in_branch_delay = false;
		pc = new_pc;
	}

	// (0x14200005).toString(2).padStart(32, '0')
}

uint8_t Cpu::read8(uint32_t addr) {
	return bus.read8(addr);
}

void Cpu::write8(uint32_t addr, uint8_t value) {
	bus.write8(addr, value);
}

uint16_t Cpu::read16(uint32_t addr) {
	return read8(addr) | read8(addr + 1) << 8;
}

uint32_t Cpu::read32(uint32_t addr) {
	return read16(addr) | read16(addr + 2) << 16;
}

uint64_t Cpu::read64(uint32_t addr) {
	return read32(addr) | static_cast<uint64_t>(read32(addr + 4)) << 32;
}

void Cpu::write16(uint32_t addr, uint16_t value) {
	write8(addr, value);
	write8(addr + 1, value >> 8);
}

void Cpu::write32(uint32_t addr, uint32_t value) {
	write16(addr, value);
	write16(addr + 2, value >> 16);
}

void Cpu::write64(uint32_t addr, uint64_t value) {
	write32(addr, value);
	write32(addr + 4, value >> 32);
}

#include "cpu.hpp"
#include "utils.hpp"

void EeCpu::inst_cop0(uint32_t byte) {
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
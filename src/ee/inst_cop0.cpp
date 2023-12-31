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

			auto entry_hi = co0.get_reg(Cop0Reg::EntryHi);
			auto entry_lo0 = co0.get_reg(Cop0Reg::EntryLo0);
			auto entry_lo1 = co0.get_reg(Cop0Reg::EntryLo1);
			auto mask = co0.get_reg(Cop0Reg::PageMask);

			auto& entry = tlb[index];
			entry.global = entry_lo0 & entry_lo1 & 1;

			entry.even_page_valid = entry_lo0 & 1U << 1;
			entry.even_page_dirty = entry_lo0 & 1U << 2;
			entry.even_cache_mode = entry_lo0 >> 3 & 0b111;
			entry.even_pfn = (entry_lo0 & ~(1U << 31)) >> 6;
			entry.scratchpad = entry_lo0 & 1U << 31;

			entry.odd_page_valid = entry_lo1 & 1U << 1;
			entry.odd_page_dirty = entry_lo1 & 1U << 2;
			entry.odd_cache_mode = entry_lo1 >> 3 & 0b111;
			entry.odd_pfn = (entry_lo1 & ~(1U << 31)) >> 6;
			entry.scratchpad = entry_lo1 & 1U << 31;

			entry.asid = entry_hi & 0xFF;
			entry.vpn2 = entry_hi >> 13;
			entry.mask = mask;
		}
		else if (fmt == 0b000110) {
			TODO("TLBWR");
		}
		else if (fmt == 0b001000) {
			TODO("TLBP");
		}
		// ERET
		else if (fmt == 0b011000) {
			auto status = co0.get_reg(Cop0Reg::Status);

			// ERL
			if (status & 1 << 2) {
				pc = co0.get_reg(Cop0Reg::ErrorEpc);
				co0.get_reg(Cop0Reg::Status) = status & ~(1 << 2);
			}
			else {
				pc = co0.get_reg(Cop0Reg::Epc);
				// disable EXL
				co0.get_reg(Cop0Reg::Status) = status & ~(1 << 1);
			}
		}
		else if (fmt == 0b111000) {
			auto status = co0.get_reg(Cop0Reg::Status);
			// EXL/ERL, KSU in kernel or EDI enabled
			if ((status & 1 << 1) || (status & 1 << 2) ||
			    (status >> 3 & 0b11) == 0 || (status & 1 << 17)) {
				// enable EIE
				co0.get_reg(Cop0Reg::Status) = status | 1 << 16;
			}
		}
		// DI
		else if (fmt == 0b111001) {
			auto status = co0.get_reg(Cop0Reg::Status);
			// EXL/ERL, KSU in kernel or EDI enabled
			if ((status & 1 << 1) || (status & 1 << 2) ||
				(status >> 3 & 0b11) == 0 || (status & 1 << 17)) {
				// disable EIE
				co0.get_reg(Cop0Reg::Status) = status & ~(1 << 16);
			}
		}
		else {
			UNREACHABLE("invalid TLB instruction");
		}
	}
	else {
		UNREACHABLE("invalid COP0 instruction");
	}
}

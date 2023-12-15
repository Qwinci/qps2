#include "cpu.hpp"
#include "utils.hpp"
#include <iostream>

void IopCpu::inst_cop0(uint32_t byte) {
	uint8_t fmt = byte >> 21 & 0b11111;
	// MFC0
	if (fmt == 0b00000) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		write_reg(rt, co0.regs[rd]);
	}
	// MTC0
	else if (fmt == 0b00100) {
		uint8_t rt = byte >> 16 & 0b11111;
		uint8_t rd = byte >> 11 & 0b11111;

		// todo use the value
		co0.regs[rd] = regs[rt];
	}
	// TLB
	else if (fmt == 0b10000) {
		fmt = byte & 0b111111;
		if (fmt == 0b000001) {
			TODO("TLBR");
		}
		// TLBWI
		else if (fmt == 0b000010) {

		}
		else if (fmt == 0b000110) {
			TODO("TLBWR");
		}
		else if (fmt == 0b001000) {
			TODO("TLBP");
		}
		// RFE
		else if (fmt == 0b010000) {
			auto& sr = co0.get_reg(IopCop0Reg::Sr);
			sr &= ~0b11;
			sr |= sr >> 2 & 0b11;
			sr &= ~0b1100;
			sr |= (sr >> 4 & 0b11) << 2;
		}
		else {
			UNREACHABLE("invalid TLB instruction");
		}
	}
	else {
		std::cerr << "unimplemented iop cop0 func "
		          << std::hex << std::uppercase << static_cast<uint32_t>(fmt) << std::dec << '\n';
		abort();
	}
}

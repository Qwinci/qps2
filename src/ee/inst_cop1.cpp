#include "cpu.hpp"
#include <iostream>

void EeCpu::inst_cop1(uint32_t byte) {
	uint8_t func = byte >> 21 & 0b11111;
	// MTC1
	if (func == 0b00100) {
		// todo
	}
	// CTC1
	else if (func == 0b00110) {
		// todo
	}
	// FPU.S
	else if (func == 0b10000) {
		func = byte & 0b111111;
		// ADDA.S
		if (func == 0b011000) {
			// todo
		}
		else {
			std::cerr << "unimplemented cpu1 FPU.S func "
			          << std::hex << std::uppercase << static_cast<unsigned int>(func)
			          << std::dec << '\n';
			abort();
		}
	}
	else {
		std::cerr << "unimplemented cop1 func "
		          << std::hex << std::uppercase << static_cast<unsigned int>(func)
		          << std::dec << '\n';
		abort();
	}
}

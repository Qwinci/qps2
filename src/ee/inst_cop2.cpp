#include "cpu.hpp"
#include <iostream>

void EeCpu::inst_cop2(uint32_t byte) {
	uint8_t func = byte >> 21 & 0b11111;
	// QMFC2
	if (func == 0b00001) {
		// todo
	}
	// CFC2
	else if (func == 0b00010) {
		// todo
	}
	// QMTC2
	else if (func == 0b00101) {
		// todo
	}
	// CTC2
	else if (func == 0b00110) {
		// todo
	}
	// SPECIAL
	else if (func >= 0b10000) {
		// todo
	}
	else {
		std::cerr << "unimplemented cop2 func "
		          << std::hex << std::uppercase << static_cast<unsigned int>(func)
		          << std::dec << '\n';
		abort();
	}
}

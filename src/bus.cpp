#include "bus.hpp"
#include "cpu.hpp"
#include <fstream>
#include <cassert>
#include <iostream>

Bus::Bus(const std::string& bios_name) {
	bios.resize(1024 * 1024 * 4);
	main_ram.resize(1024 * 1024 * 32);
	std::ifstream file {bios_name, std::ios::binary};
	file.read(reinterpret_cast<char*>(bios.data()), 1024 * 1024 * 4);
}

uint8_t Bus::read8(uint32_t addr) {
	if (addr < 0x2000000) {
		return main_ram[addr];
	}
	else if (addr >= 0x10000000 && addr <= 0x10010000) {
		std::cerr << "unimplemented register read from "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
	else if (addr >= 0x1FC00000 && addr < 0x20000000) {
		return bios[addr - 0x1FC00000];
	}
	// unknown
	else if (addr == 0x1F803204) {
		return 0;
	}
	else {
		std::cerr << "unimplemented read from "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
}

uint16_t Bus::read16(uint32_t addr) {
	return read8(addr) | read8(addr + 1) << 8;
}

uint32_t Bus::read32(uint32_t addr) {
	if (addr == 0x10000000) {
		return timers[0].counter;
	}

	return read16(addr) | read16(addr + 2) << 16;
}

uint64_t Bus::read64(uint32_t addr) {
	return read32(addr) | static_cast<uint64_t>(read32(addr + 4)) << 32;
}

void Bus::write8(uint32_t addr, uint8_t value) {
	if (addr < 0x2000000) {
		main_ram[addr] = value;
	}
	// unknown registers
	else if ((addr >= 0x1000F500 && addr < 0x1000F508) ||
		(addr >= 0x1F801470 && addr < 0x1F801478)) {
		return;
	}
	else if (addr >= 0x10000000 && addr <= 0x10010000) {
		std::cerr << "unimplemented register write to "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
	// SMODE1, SMODE2, SRFSH, SYNCH1, SYNCH2, SYNCV
	else if ((addr >= 0x12000010 && addr < 0x12000018) ||
		(addr >= 0x12000020 && addr < 0x12000028) ||
		(addr >= 0x12000030 && addr < 0x12000038) ||
		(addr >= 0x12000040 && addr < 0x12000048) ||
		(addr >= 0x12000050 && addr < 0x12000058) ||
		(addr >= 0x12000060 && addr < 0x12000068)) {
		return;
	}
	else {
		std::cerr << "unimplemented write to "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
}

void Bus::write16(uint32_t addr, uint16_t value) {
	write8(addr, value);
	write8(addr + 1, value >> 8);
}

void Bus::write32(uint32_t addr, uint32_t value) {
	if (addr == 0x10000000) {
		timers[0].counter = value;
		return;
	}
	else if (addr == 0x10000010) {
		timers[0].mode = value;
		return;
	}

	write16(addr, value);
	write16(addr + 2, value >> 16);
}

void Bus::write64(uint32_t addr, uint64_t value) {
	write32(addr, value);
	write32(addr + 4, value >> 32);
}

void Bus::clock() {
	for (auto& timer : timers) {
		timer.clock();
	}
}

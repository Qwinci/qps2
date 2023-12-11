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
	// todo move to cpu
	if (addr < 0x2000000) {
		return main_ram[addr];
	}
	else if (addr >= 0x20000000 && addr < 0x20000000 + 0x2000000) {
		return main_ram[addr - 0x20000000];
	}
	else if (addr >= 0x70000000 && addr < 0x70004000) {
		return scratchpad_ram[addr - 0x70000000];
	}
	// todo figure out what this is (end is not correct)
	else if (addr >= 0xB0000000 && addr <= 0xB1000000) {
		return 0;
	}
	else if (addr >= 0xBF801000 && addr <= 0xBF801420) {
		return 0;
	}
	else if (addr >= 0x9FC00000 && addr < 0xA0000000) {
		return bios[addr - 0x9FC00000];
	}
	else if (addr >= 0xBFC00000 && addr < 0xC0000000) {
		return bios[addr - 0xBFC00000];
	}
	else {
		std::cerr << "unimplemented read from "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		exit(1);
	}
}

uint16_t Bus::read16(uint32_t addr) {
	return read8(addr) | read8(addr + 1) << 8;
}

uint32_t Bus::read32(uint32_t addr) {
	uint32_t value = read8(addr);
	value |= read8(addr + 1) << 8;
	value |= read8(addr + 2) << 16;
	value |= read8(addr + 3) << 24;
	return value;
}

uint64_t Bus::read64(uint32_t addr) {
	return read32(addr) | static_cast<uint64_t>(read32(addr + 4)) << 32;
}

void Bus::write8(uint32_t addr, uint8_t value) {
	// todo move to cpu

	if (addr < 0x2000000) {
		main_ram[addr] = value;
	}
	else if (addr >= 0x20000000 && addr < 0x20000000 + 0x2000000) {
		main_ram[addr - 0x20000000] = value;
	}
	else if (addr >= 0x70000000 && addr < 0x70004000) {
		scratchpad_ram[addr - 0x70000000] = value;
	}
	// todo figure out what this is (end is not correct)
	else if (addr >= 0xB0000000 && addr <= 0xB3000000) {
		return;
	}
	else if (addr >= 0xBF801000 && addr <= 0xBF801420) {
		return;
	}
	else if (addr >= 0xBFC00000 && addr < 0xC0000000) {
		return;
	}
	else {
		std::cerr << "unimplemented write to "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		exit(1);
	}
}

void Bus::write16(uint32_t addr, uint16_t value) {
	write8(addr, value);
	write8(addr + 1, value >> 8);
}

void Bus::write32(uint32_t addr, uint32_t value) {
	write8(addr, value);
	write8(addr + 1, value >> 8);
	write8(addr + 2, value >> 16);
	write8(addr + 3, value >> 24);
}

void Bus::write64(uint32_t addr, uint64_t value) {
	write32(addr, value);
	write8(addr + 4, value >> 32);
}

#pragma once
#include "cpu.hpp"
#include "timer.hpp"
#include <cstdint>
#include <vector>
#include <string>

struct Bus {
	explicit Bus(const std::string& bios_name);
	uint8_t read8(uint32_t addr);
	uint16_t read16(uint32_t addr);
	uint32_t read32(uint32_t addr);
	uint64_t read64(uint32_t addr);
	void write8(uint32_t addr, uint8_t value);
	void write16(uint32_t addr, uint16_t value);
	void write32(uint32_t addr, uint32_t value);
	void write64(uint32_t addr, uint64_t value);

	void clock();

	Cpu cpu {*this};
	std::vector<uint8_t> bios;
	std::vector<uint8_t> main_ram;
	Timer timers[4] {{*this}, {*this}, {*this}, {*this}};
};

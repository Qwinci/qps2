#pragma once
#include "ee/cpu.hpp"
#include "timer.hpp"
#include "gif.hpp"
#include "gs.hpp"
#include "dmac.hpp"
#include "vif.hpp"
#include "ipu.hpp"
#include "sif.hpp"
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

	EeCpu ee_cpu {*this};
	std::vector<uint8_t> bios;
	std::vector<uint8_t> main_ram;
	std::vector<uint8_t> iop_ram;
	std::vector<uint8_t> vu0_code;
	std::vector<uint8_t> vu0_data;
	std::vector<uint8_t> vu1_code;
	std::vector<uint8_t> vu1_data;
	Timer timers[4] {{*this}, {*this}, {*this}, {*this}};
	Gif gif {*this};
	Gs gs {*this};
	Dmac dmac {*this};
	Vif vif0 {*this};
	Vif vif1 {*this};
	Ipu ipu {*this};
	Sif sif {*this};
	uint32_t mch_ricm {};
	uint32_t mch_drd {};
	uint8_t rdram_dev_id {};
};

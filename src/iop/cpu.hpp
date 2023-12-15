#pragma once
#include "cpu_shared.hpp"
#include <cstdint>
#include "iop_bus.hpp"

struct Bus;

enum class IopCop0Reg {
	Bpc = 3,
	Bda = 5,
	JumpDest = 6,
	Dcic = 7,
	BadVaddr = 8,
	BDam = 9,
	BpcM = 11,
	Sr = 12,
	Cause = 13,
	Epc = 14,
	PrId = 15
};

struct IopCpu {
	explicit IopCpu(Bus& bus);

	Bus& bus;
	IopBus iop_bus;
	uint32_t regs[32] {};
	uint32_t pc {0xBFC00000};
	uint32_t hi {};
	uint32_t lo {};
	uint32_t new_pc {};
	bool in_branch_delay {};

	inline constexpr uint32_t& get_reg(Reg reg) {
		return regs[static_cast<int>(reg)];
	}

	inline void write_reg(uint8_t reg, uint32_t value) {
		if (reg == 0) {
			return;
		}
		regs[reg] = value;
	}

	struct Coprocessor {
		uint32_t regs[64];

		constexpr uint32_t& get_reg(IopCop0Reg reg) {
			return regs[static_cast<int>(reg)];
		}
	};
	Coprocessor co0 {};

	void clock();

	uint8_t read8(uint32_t addr);
	uint16_t read16(uint32_t addr);
	uint32_t read32(uint32_t addr);
	void write8(uint32_t addr, uint8_t value);
	void write16(uint32_t addr, uint16_t value);
	void write32(uint32_t addr, uint32_t value);

	uint32_t virt_to_phys(uint32_t virt);;

	void raise_level1_exception(uint32_t vector, uint8_t cause);

	void inst_normal(uint32_t byte);
	void inst_cop0(uint32_t byte);
	void inst_special(uint32_t byte);
	void inst_regimm(uint32_t byte);
};

#pragma once
#include <cstdint>
#include <array>

struct Bus;

enum class Reg {
	Zero,
	At,
	V0,
	V1,
	A0,
	A1,
	A2,
	A3,
	T0,
	T1,
	T2,
	T3,
	T4,
	T5,
	T6,
	T7,
	T8,
	T9,
	S0,
	S1,
	S2,
	S3,
	S4,
	S5,
	S6,
	S7,
	K0,
	K1,
	Gp,
	Sp,
	Fp,
	Ra
};

struct Register {
	uint64_t low;
	uint64_t high;

	inline void store32(const std::array<uint32_t, 4>& splits) {
		low = splits[0] | static_cast<uint64_t>(splits[1]) << 32;
		high = splits[2] | static_cast<uint64_t>(splits[3]) << 32;
	}

	constexpr std::array<uint32_t, 4> split32() {
		uint32_t x0 = low;
		uint32_t x1 = low >> 32;
		uint32_t x2 = high;
		uint32_t x3 = high >> 32;
		return {x0, x1, x2, x3};
	}
};

enum class Cop0Reg {
	Index,
	Random,
	EntryLo0,
	EntryLo1,
	Context,
	PageMask,
	Wired,
	BadVAddr = 8,
	Count,
	EntryHi,
	Compare,
	Status,
	Cause,
	Epc,
	PrId,
	Config,
	BadPAddr,
	Debug,
	Perf,
	TagLo,
	TagHi,
	ErrorEpc
};

struct EeCpu {
	explicit EeCpu(Bus& bus);
	Bus& bus;
	bool clock_bus {true};

	void clock();
	inline constexpr Register& get_reg(Reg reg) {
		return regs[static_cast<int>(reg)];
	}

	inline void write_reg_low(uint8_t reg, uint64_t value) {
		if (reg == 0) {
			return;
		}
		regs[reg].low = value;
	}

	inline void write_reg_high(uint8_t reg, uint64_t value) {
		if (reg == 0) {
			return;
		}
		regs[reg].high = value;
	}

	Register regs[32] {};
	uint32_t pc {0xBFC00000};
	uint64_t hi_lo {};
	uint64_t hi1_lo1 {};
	uint32_t sa {};

	struct Coprocessor {
		uint32_t regs[31];

		inline constexpr uint32_t& get_reg(Cop0Reg reg) {
			return regs[static_cast<int>(reg)];
		}
	};

	struct TlbEntry {
		bool even_page_valid : 1;
		bool even_page_dirty : 1;
		uint8_t even_cache_mode : 3;
		uint32_t even_pfn : 20;
		bool odd_page_valid : 1;
		bool odd_page_dirty : 1;
		uint8_t odd_cache_mode : 3;
		uint32_t odd_pfn : 20;
		bool scratchpad : 1;
		uint8_t asid;
		bool global : 1;
		uint32_t vpn2 : 19;
		uint16_t mask : 12;
	};
	TlbEntry tlb[48] {};

	Coprocessor co0 {};
	uint8_t scratchpad_ram[1024 * 16] {};
	bool in_branch_delay {};
	uint32_t new_pc {};
	uint32_t intc_stat {};
	uint32_t intc_mask {};

	uint32_t virt_to_phys(uint32_t virt);

	uint8_t read8(uint32_t addr);
	uint16_t read16(uint32_t addr);
	uint32_t read32(uint32_t addr);
	uint64_t read64(uint32_t addr);
	void write8(uint32_t addr, uint8_t value);
	void write16(uint32_t addr, uint16_t value);
	void write32(uint32_t addr, uint32_t value);
	void write64(uint32_t addr, uint64_t value);

	void raise_level2_exception(uint32_t vector, uint8_t cause);

	void inst_cop0(uint32_t byte);
	void inst_special(uint32_t byte);
	void inst_normal(uint32_t byte);
	void inst_regimm(uint32_t byte);
	void inst_mmi(uint32_t byte);
	void inst_cop2(uint32_t byte);
	void inst_cop1(uint32_t byte);
};

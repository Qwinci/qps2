#include <cassert>
#include "cpu.hpp"
#include "../bus.hpp"

EeCpu::EeCpu(Bus& bus) : bus {bus} {
	// EE
	co0.get_reg(Cop0Reg::PrId) = 0x59;
}

void EeCpu::clock() {
	assert(pc != 0x82000 && "kernel finished");

	co0.get_reg(Cop0Reg::Count) += 1;

	auto cur_clock = clock_counter++;
	// bus is clocked twice as slow as the cpu
	if ((cur_clock & 1) == 0) {
		bus.clock();
	}
	// iop is clocked 8x slow as the cpu
	if ((cur_clock & 7) == 0) {
		bus.iop_cpu.clock();
	}

	auto byte = read32(pc);
	pc += 4;

	if (in_branch_delay) {
		inst_normal(byte);
		in_branch_delay = false;
		pc = new_pc;
	}
	else {
		inst_normal(byte);
	}

	// (0x14200005).toString(2).padStart(32, '0')
}

uint32_t EeCpu::virt_to_phys(uint32_t virt) {
	return virt & 0x1FFFFFFF;
}

uint8_t EeCpu::read8(uint32_t addr) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		return scratchpad_ram[addr - 0x70000000];
	}
	else {
		return bus.read8(virt_to_phys(addr));
	}
}

uint16_t EeCpu::read16(uint32_t addr) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		return scratchpad_ram[addr - 0x70000000] | scratchpad_ram[addr - 0x70000000] << 8;
	}
	else {
		return bus.read16(virt_to_phys(addr));
	}
}

uint32_t EeCpu::read32(uint32_t addr) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		return scratchpad_ram[addr - 0x70000000] | scratchpad_ram[addr - 0x70000000 + 1] << 8 |
			scratchpad_ram[addr - 0x70000000 + 2] << 16 | scratchpad_ram[addr - 0x70000000 + 3] << 24;
	}
	else {
		return bus.read32(virt_to_phys(addr));
	}
}

uint64_t EeCpu::read64(uint32_t addr) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		return scratchpad_ram[addr - 0x70000000] |
			scratchpad_ram[addr - 0x70000000 + 1] << 8 |
			scratchpad_ram[addr - 0x70000000 + 2] << 16 |
			scratchpad_ram[addr - 0x70000000 + 3] << 24 |
			static_cast<uint64_t>(scratchpad_ram[addr - 0x70000000 + 4]) << 32 |
			static_cast<uint64_t>(scratchpad_ram[addr - 0x70000000 + 5]) << 40 |
			static_cast<uint64_t>(scratchpad_ram[addr - 0x70000000 + 6]) << 48 |
			static_cast<uint64_t>(scratchpad_ram[addr - 0x70000000 + 7]) << 56;
	}
	else {
		return bus.read64(virt_to_phys(addr));
	}
}

void EeCpu::write8(uint32_t addr, uint8_t value) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		scratchpad_ram[addr - 0x70000000] = value;
	}
	else {
		bus.write8(virt_to_phys(addr), value);
	}
}

void EeCpu::write16(uint32_t addr, uint16_t value) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		scratchpad_ram[addr - 0x70000000] = value;
		scratchpad_ram[addr - 0x70000000 + 1] = value >> 8;
	}
	else {
		bus.write16(virt_to_phys(addr), value);
	}
}

void EeCpu::write32(uint32_t addr, uint32_t value) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		scratchpad_ram[addr - 0x70000000] = value;
		scratchpad_ram[addr - 0x70000000 + 1] = value >> 8;
		scratchpad_ram[addr - 0x70000000 + 2] = value >> 16;
		scratchpad_ram[addr - 0x70000000 + 3] = value >> 24;
	}
	else {
		bus.write32(virt_to_phys(addr), value);
	}
}

void EeCpu::write64(uint32_t addr, uint64_t value) {
	if (addr >= 0x70000000 && addr < 0x70004000) {
		scratchpad_ram[addr - 0x70000000] = value;
		scratchpad_ram[addr - 0x70000000 + 1] = value >> 8;
		scratchpad_ram[addr - 0x70000000 + 2] = value >> 16;
		scratchpad_ram[addr - 0x70000000 + 3] = value >> 24;
		scratchpad_ram[addr - 0x70000000 + 4] = value >> 32;
		scratchpad_ram[addr - 0x70000000 + 5] = value >> 40;
		scratchpad_ram[addr - 0x70000000 + 6] = value >> 48;
		scratchpad_ram[addr - 0x70000000 + 7] = value >> 56;
	}
	else {
		bus.write64(virt_to_phys(addr), value);
	}
}

void EeCpu::raise_level2_exception(uint32_t vector, uint8_t cause) {
	// DEV, use bootstrap vectors
	if (co0.get_reg(Cop0Reg::Status) & 1 << 23) {
		if (vector != 0xBFC00000) {
			vector &= 0xFFF;
			vector |= 0xBFC00200;
		}
	}

	auto& cause_reg = co0.get_reg(Cop0Reg::Cause);
	cause_reg |= cause << 2;
	if (in_branch_delay) {
		co0.get_reg(Cop0Reg::ErrorEpc) = pc - 4;
		// BD2
		cause_reg |= 1U << 30;
	}
	else {
		co0.get_reg(Cop0Reg::ErrorEpc) = pc;
	}
	// ERL
	co0.get_reg(Cop0Reg::Status) |= 1 << 2;
	pc = vector;
}

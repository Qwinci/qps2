#include "cpu.hpp"
#include "../bus.hpp"
#include <fstream>
#include <filesystem>
#include <vector>
#include <elf.h>

EeCpu::EeCpu(Bus& bus) : bus {bus} {
	// EE
	co0.get_reg(Cop0Reg::PrId) = 0x59;
}

void EeCpu::clock() {
	if (pc == 0x82000) {
		std::ifstream file {"../roms/3stars.elf", std::ios::binary};
		auto size = std::filesystem::file_size("../roms/3stars.elf");
		std::vector<uint8_t> data(size);
		file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));

		auto* ehdr = reinterpret_cast<Elf32_Ehdr*>(data.data());
		for (uint16_t i = 0; i < ehdr->e_phnum; ++i) {
			auto* phdr = reinterpret_cast<Elf32_Phdr*>(data.data() + ehdr->e_phoff + i * ehdr->e_phentsize);
			if (phdr->p_type != PT_LOAD) {
				continue;
			}

			auto* file_data = data.data() + phdr->p_offset;
			for (uint32_t off = 0; off < phdr->p_filesz; ++off) {
				write8(phdr->p_vaddr + off, file_data[off]);
			}
			for (uint32_t off = phdr->p_filesz; off < phdr->p_memsz; ++off) {
				write8(phdr->p_vaddr + off, 0);
			}
		}
		pc = ehdr->e_entry;
	}

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
		return *(uint64_t*) &scratchpad_ram[addr - 0x70000000];
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

void EeCpu::raise_level1_exception(uint32_t vector, uint8_t cause) {
	// BEV, use bootstrap vectors
	if (co0.get_reg(Cop0Reg::Status) & 1 << 22) {
		if (vector != 0xBFC00000) {
			vector &= 0xFFF;
			vector |= 0xBFC00200;
		}
	}

	auto& cause_reg = co0.get_reg(Cop0Reg::Cause);
	cause_reg &= ~(0b11111 << 2);
	cause_reg |= cause << 2;
	if (in_branch_delay) {
		co0.get_reg(Cop0Reg::Epc) = pc - 8;
		// BD
		cause_reg |= 1U << 31;
		in_branch_delay = false;
	}
	else {
		co0.get_reg(Cop0Reg::Epc) = pc - 4;
		cause_reg &= ~(1U << 31);
	}
	// EXL
	co0.get_reg(Cop0Reg::Status) |= 1 << 1;
	pc = vector;
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
	cause_reg &= ~(0b11111 << 2);
	cause_reg |= cause << 2;
	if (in_branch_delay) {
		co0.get_reg(Cop0Reg::ErrorEpc) = pc - 8;
		// BD2
		cause_reg |= 1U << 30;
		in_branch_delay = false;
	}
	else {
		co0.get_reg(Cop0Reg::ErrorEpc) = pc - 4;
		cause_reg &= ~(1U << 30);
	}
	// ERL
	co0.get_reg(Cop0Reg::Status) |= 1 << 2;
	pc = vector;
}

void EeCpu::raise_int0(uint8_t irq) {
	intc_stat |= 1U << irq;
	auto status = co0.get_reg(Cop0Reg::Status);
	if (!(intc_stat & intc_mask) || !(status & 1U << 10)) {
		// INTC interrupt pending
		co0.get_reg(Cop0Reg::Cause) |= 1U << 10;
		return;
	}

	raise_level1_exception(0x80000200, 0);
}
